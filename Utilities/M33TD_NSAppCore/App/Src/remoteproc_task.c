/**
  ******************************************************************************
  * @file    remoteproc_task.c
  * @author  MCD Application Team
  * @brief   Remote processor task implementation file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/**
  * @addtogroup RemoteProcTask
  * @{
  */

#include "remoteproc_task.h"
#include "remoteproc_driver.h"
#include "tfm_ioctl_cpu_api.h"
#include <string.h>
#include <stdio.h>

/**
  * @brief Maximum number of remote processor state callbacks.
  */
#define REMOTEPROC_MAX_CALLBACKS 1

/**
  * @brief Timeout and interval for asynchronous start.
  */
#define REMOTEPROC_START_TIMEOUT_MS         1000
#define REMOTEPROC_START_CHECK_INTERVAL_MS  10

/**
  * @brief Structure for remote processor event message.
  */
typedef struct {
    RemoteProcEvent event; /**< Event type. */
    uint32_t param;        /**< Event parameter. */
} RemoteProcEventMsg;

/**
  * @brief Structure for remote processor state callback entry.
  */
typedef struct {
    RemoteProcStateCallback cb; /**< Callback function pointer. */
    void *context;              /**< User context. */
} RemoteProcCallbackEntry;

/**
  * @brief List of registered remote processor state callbacks.
  */
static RemoteProcCallbackEntry remoteProcCallbackList[REMOTEPROC_MAX_CALLBACKS];

/**
  * @brief Handle for the event message queue.
  */
static osMessageQueueId_t remoteProcEventQueueHandle;

/**
  * @brief Handle for the remote processor task thread.
  */
static osThreadId_t remoteProcTaskHandle;

/**
  * @brief Structure holding coprocessor information.
  */
static struct cpu_info_res remoteProcCoproInfo;

/**
  * @brief Current state of the remote processor.
  */
static RemoteProcState remoteProcCurrentState = REMOTEPROC_STATE_UNKNOWN;

/**
  * @brief String representations of coprocessor CPU status.
  */
static const char *remoteProcCpuStatusStr[] = {
    "offline", "suspended", "started", "running", "crashed", "unknown"
};

/**
  * @brief Tick count for asynchronous start timeout.
  */
static uint32_t remoteProcStartTick = 0;

/**
  * @brief CPU ID to restart after crash.
  */
static uint32_t remoteProcCrashCpuId = 0;

/**
  * @brief Crash recovery flag.
  */
static bool remoteProcCrashRecovery = false;

/**
  * @brief Attributes for the event message queue.
  */
static const osMessageQueueAttr_t remoteProcEventQueueAttr = {
    .name = "RemoteProcEventQueue"
};

/**
  * @brief Attributes for the remote processor task thread.
  */
static const osThreadAttr_t remoteProcTaskAttr = {
    .name = "RemoteProcTask",
    .priority = (osPriority_t) REMOTEPROC_TASK_PRIORITY,
    .stack_size = REMOTEPROC_TASK_STACK_SIZE
};

/* Forward declarations */
static void RemoteProcTask(void *argument);
static void RemoteProcTask_NotifyCallbacks(RemoteProcState new_state);
static void RemoteProcTask_HandleEvent(RemoteProcEvent event, uint32_t param);
static void RemoteProcTask_HandleStart(uint32_t cpu_id);
static void RemoteProcTask_HandleStop(uint32_t cpu_id);
static void RemoteProcTask_CrashHandler(uint32_t cpu_id);

/**
  * @brief Register a callback for remote processor state changes.
  * @param  cb: Callback function pointer.
  * @param  context: User context pointer.
  * @retval HAL_OK if registered, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef RemoteProcTask_RegisterCallback(RemoteProcStateCallback cb, void *context)
{
  for (int i = 0; i < REMOTEPROC_MAX_CALLBACKS; ++i)
  {
    if (remoteProcCallbackList[i].cb == NULL)
    {
      remoteProcCallbackList[i].cb = cb;
      remoteProcCallbackList[i].context = context;
      return HAL_OK;
    }
  }
  return HAL_ERROR;
}

/**
  * @brief Unregister a previously registered callback.
  * @param  cb: Callback function pointer.
  * @retval HAL_OK if unregistered, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef RemoteProcTask_UnregisterCallback(RemoteProcStateCallback cb)
{
  for (int i = 0; i < REMOTEPROC_MAX_CALLBACKS; ++i)
  {
    if (remoteProcCallbackList[i].cb == cb)
    {
      remoteProcCallbackList[i].cb = NULL;
      remoteProcCallbackList[i].context = NULL;
      return HAL_OK;
    }
  }
  return HAL_ERROR;
}

/**
  * @brief Notify all registered callbacks of a state change.
  * @param  new_state: The new state to notify.
  */
static void RemoteProcTask_NotifyCallbacks(RemoteProcState new_state)
{
  for (int i = 0; i < REMOTEPROC_MAX_CALLBACKS; ++i)
  {
    if (remoteProcCallbackList[i].cb)
    {
      remoteProcCallbackList[i].cb(new_state, remoteProcCallbackList[i].context);
    }
  }
}

/**
  * @brief Post an event to the remote processor event queue.
  * @param  event: Event type.
  * @param  param: Event parameter.
  * @retval HAL_OK if the event was posted, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef RemoteProcTask_PostEvent(RemoteProcEvent event, uint32_t param)
{
  if (!remoteProcEventQueueHandle)
    return HAL_ERROR;
  RemoteProcEventMsg msg = { .event = event, .param = param };
  if (osMessageQueuePut(remoteProcEventQueueHandle, &msg, 0, 0) == osOK)
    return HAL_OK;
  return HAL_ERROR;
}

/**
  * @brief Get the current state of the remote processor.
  * @retval The current remote processor state.
  */
RemoteProcState RemoteProcTask_GetState(void)
{
  return remoteProcCurrentState;
}

/**
  * @brief Crash handler callback for the remote processor task.
  * @param  cpu_id: CPU ID that crashed.
  */
static void RemoteProcTask_CrashHandler(uint32_t cpu_id)
{
  RemoteProcTask_PostEvent(REMOTEPROC_EVENT_CRASH, cpu_id);
}

/**
  * @brief Initialize the remote processor task and its resources.
  *        Creates the event queue and the task thread, and initializes the driver.
  * @retval None
  */
void RemoteProcTask_Init(void)
{
  memset(remoteProcCallbackList, 0, sizeof(remoteProcCallbackList));
  remoteProcEventQueueHandle = osMessageQueueNew(8, sizeof(RemoteProcEventMsg), &remoteProcEventQueueAttr);
  if (!remoteProcEventQueueHandle)
    NSAppCore_ErrorHandler();

  remoteProcTaskHandle = osThreadNew(RemoteProcTask, NULL, &remoteProcTaskAttr);
  if (!remoteProcTaskHandle)
    NSAppCore_ErrorHandler();

  remoteproc_driver.init();
  remoteproc_driver.register_crash_callback(RemoteProcTask_CrashHandler);

  remoteProcCurrentState = REMOTEPROC_STATE_OFFLINE;
  RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);

  /* Auto-start the Cortex-A35. */
#if REMOTE_PROC_AUTO_START
  RemoteProcTask_PostEvent(REMOTEPROC_EVENT_START, 0);
#endif /* REMOTE_PROC_AUTO_START */
}

/**
  * @brief De-initialize the remote processor task and its resources.
  *        Deletes the event queue and terminates the task thread.
  * @retval None
  */
void RemoteProcTask_DeInit(void)
{
  remoteproc_driver.unregister_crash_callback(RemoteProcTask_CrashHandler);
  remoteproc_driver.deinit();

  if (remoteProcEventQueueHandle)
  {
    osMessageQueueDelete(remoteProcEventQueueHandle);
    remoteProcEventQueueHandle = NULL;
  }
  if (remoteProcTaskHandle)
  {
    osThreadTerminate(remoteProcTaskHandle);
    remoteProcTaskHandle = NULL;
  }
  remoteProcCurrentState = REMOTEPROC_STATE_UNKNOWN;
}

/**
  * @brief Task function to handle remote processor events.
  * @param  argument: Not used.
  * @retval None
  */
static void RemoteProcTask(void *argument)
{
  (void)argument;
  RemoteProcEventMsg msg;
  while (1)
  {
    if (osMessageQueueGet(remoteProcEventQueueHandle, &msg, NULL, osWaitForever) == osOK)
    {
      RemoteProcTask_HandleEvent(msg.event, msg.param);
    }
  }
}

/**
  * @brief Handle a remote processor event based on the current state.
  * @param  event: Event type.
  * @param  param: Event parameter.
  */
static void RemoteProcTask_HandleEvent(RemoteProcEvent event, uint32_t param)
{
  switch (remoteProcCurrentState)
  {
  case REMOTEPROC_STATE_OFFLINE:
    if (event == REMOTEPROC_EVENT_START)
    {
      remoteProcCurrentState = REMOTEPROC_STATE_STARTING;
      RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);
      RemoteProcTask_HandleStart(param);
    }
    break;
  case REMOTEPROC_STATE_STARTING:
    if (event == REMOTEPROC_EVENT_CHECK_START_STATUS)
    {
      int err = tfm_platform_cpu_info(param, &remoteProcCoproInfo);
      if (err != TFM_PLATFORM_ERR_SUCCESS)
      {
        APP_LOG_ERR("RemoteProc", "Failed to get copro info for cpu%lu (err: %d)", param, err);
        RemoteProcTask_PostEvent(REMOTEPROC_EVENT_ERROR, param);
        break;
      }
      if (remoteProcCoproInfo.status == CPU_RUNNING)
      {
        APP_LOG_INF("RemoteProc", "copro %s started.", remoteProcCoproInfo.name);
        RemoteProcTask_PostEvent(REMOTEPROC_EVENT_STARTED, param);
      }
      else if ((osKernelGetTickCount() - remoteProcStartTick) > REMOTEPROC_START_TIMEOUT_MS)
      {
        APP_LOG_ERR("RemoteProc", "Timeout waiting for copro %s to start. Last status: %s",
          remoteProcCoproInfo.name,
          remoteProcCpuStatusStr[
            (remoteProcCoproInfo.status >= 0 && remoteProcCoproInfo.status < CPU_LAST)
            ? remoteProcCoproInfo.status : CPU_LAST
          ]);
        RemoteProcTask_PostEvent(REMOTEPROC_EVENT_ERROR, param);
      }
      else
      {
        osDelay(REMOTEPROC_START_CHECK_INTERVAL_MS);
        RemoteProcTask_PostEvent(REMOTEPROC_EVENT_CHECK_START_STATUS, param);
      }
    }
    else if (event == REMOTEPROC_EVENT_STARTED)
    {
      remoteProcCurrentState = REMOTEPROC_STATE_RUNNING;
      RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);
    }
    else if (event == REMOTEPROC_EVENT_ERROR)
    {
      remoteProcCurrentState = REMOTEPROC_STATE_OFFLINE;
      RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);
    }
    break;
  case REMOTEPROC_STATE_RUNNING:
    if (event == REMOTEPROC_EVENT_STOP)
    {
      remoteProcCurrentState = REMOTEPROC_STATE_STOPPING;
      RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);
      RemoteProcTask_HandleStop(param);
    }
    else if (event == REMOTEPROC_EVENT_CRASH)
    {
      APP_LOG_INF("RemoteProc", "Crash detected on cpu%lu. Attempting recovery...", param);
      remoteProcCrashRecovery = true;
      remoteProcCrashCpuId = param;
      remoteProcCurrentState = REMOTEPROC_STATE_STOPPING;
      RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);
      RemoteProcTask_HandleStop(param); // Initiate stop as part of recovery
    }
    break;
  case REMOTEPROC_STATE_STOPPING:
    if (event == REMOTEPROC_EVENT_STOPPED)
    {
      if (remoteProcCrashRecovery)
      {
        APP_LOG_INF("RemoteProc", "Crash recovery: restarting cpu%lu", remoteProcCrashCpuId);
        remoteProcCrashRecovery = false;
        remoteProcCurrentState = REMOTEPROC_STATE_STARTING;
        RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);
        RemoteProcTask_HandleStart(remoteProcCrashCpuId);
      }
      else
      {
        remoteProcCurrentState = REMOTEPROC_STATE_OFFLINE;
        RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);
      }
    }
    else if (event == REMOTEPROC_EVENT_ERROR)
    {
      remoteProcCurrentState = REMOTEPROC_STATE_OFFLINE;
      remoteProcCrashRecovery = false;
      RemoteProcTask_NotifyCallbacks(remoteProcCurrentState);
    }
    break;
  default:
    break;
  }
}

/**
  * @brief Handle the start event for the remote processor (asynchronous).
  * @param  cpu_id: ID of the remote processor to start.
  */
static void RemoteProcTask_HandleStart(uint32_t cpu_id)
{
  int err = tfm_platform_cpu_info(cpu_id, &remoteProcCoproInfo);
  if (err != TFM_PLATFORM_ERR_SUCCESS)
  {
    APP_LOG_ERR("RemoteProc", "Failed to get copro info for cpu%lu (err: %d)", cpu_id, err);
    RemoteProcTask_PostEvent(REMOTEPROC_EVENT_ERROR, cpu_id);
    return;
  }

  if (remoteProcCoproInfo.status < 0 || remoteProcCoproInfo.status >= CPU_LAST)
  {
    APP_LOG_ERR("RemoteProc", "copro %s error %ld", remoteProcCoproInfo.name, remoteProcCoproInfo.status);
    RemoteProcTask_PostEvent(REMOTEPROC_EVENT_ERROR, cpu_id);
    return;
  }

  if (remoteProcCoproInfo.status == CPU_RUNNING)
  {
    APP_LOG_INF("RemoteProc", "copro %s already running.", remoteProcCoproInfo.name);
    RemoteProcTask_PostEvent(REMOTEPROC_EVENT_STARTED, cpu_id);
    return;
  }

  int start_err = tfm_platform_cpu_start(cpu_id, &remoteProcCoproInfo.status);
  if (start_err != TFM_PLATFORM_ERR_SUCCESS)
  {
    APP_LOG_ERR("RemoteProc", "Failed to start copro %s (err: %d)", remoteProcCoproInfo.name, start_err);
    RemoteProcTask_PostEvent(REMOTEPROC_EVENT_ERROR, cpu_id);
    return;
  }

  APP_LOG_INF("RemoteProc", "starting copro %s... (pending)", remoteProcCoproInfo.name);

  remoteProcStartTick = osKernelGetTickCount();
  RemoteProcTask_PostEvent(REMOTEPROC_EVENT_CHECK_START_STATUS, cpu_id);
}

/**
  * @brief Handle the stop event for the remote processor.
  * @param  cpu_id: ID of the remote processor to stop.
  */
static void RemoteProcTask_HandleStop(uint32_t cpu_id)
{
  int err = tfm_platform_cpu_stop(cpu_id, &remoteProcCoproInfo.status);
  if (err != TFM_PLATFORM_ERR_SUCCESS)
  {
    APP_LOG_ERR("RemoteProc", "Stop fail err: %d. Last status: %s",
      err,
      remoteProcCpuStatusStr[
        (remoteProcCoproInfo.status >= 0 && remoteProcCoproInfo.status < CPU_LAST)
        ? remoteProcCoproInfo.status : CPU_LAST
      ]);
    RemoteProcTask_PostEvent(REMOTEPROC_EVENT_ERROR, cpu_id);
    return;
  }
  APP_LOG_INF("RemoteProc", "copro %s stopped.", remoteProcCoproInfo.name);
  RemoteProcTask_PostEvent(REMOTEPROC_EVENT_STOPPED, cpu_id);
}

/**
  * @brief Get a copy of the remote processor information structure.
  * @retval A copy of the cpu_info_res structure.
  */
struct cpu_info_res RemoteProcTask_GetCoproInfo(void)
{
  return remoteProcCoproInfo;
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

