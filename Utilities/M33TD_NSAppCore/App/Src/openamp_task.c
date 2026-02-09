/**
  ******************************************************************************
  * @file    openamp_task.c
  * @author  MCD Application Team
  * @brief   OpenAMP task implementation.
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

/* Includes ------------------------------------------------------------------*/
#include "openamp_task.h"
#include "openamp_driver.h"
#include "rsc_table.h"
#include <stdio.h>
#include <string.h>
#include "metal/sys.h"
#include "metal/device.h"
#include "openamp.h"
#include "openamp_log.h"
#if ENABLE_DISPLAY_TASK
#include "display_task.h"
#endif
#if ENABLE_SCMI_MGR_TASK
#include "scmi_mgr_task.h"
#endif /* ENABLE_SCMI_MGR_TASK */
#if ENABLE_BTN_MONITOR_TASK
#include "btn_monitor_task.h"
#endif /* ENABLE_BTN_MONITOR_TASK */

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define OPENAMP_POWER_ENDPOINT_ADDR        (0x58U)

#if ENABLE_DISPLAY_TASK
#define OPENAMP_DISPLAY_ENDPOINT_ADDR      (0x59U)
#endif

/* Private function prototypes ----------------------------------------------*/
static void OpenampTask(void *argument);

static int OpenampTask_PowerEndpointCallback(struct rpmsg_endpoint *ept, void *data, size_t len,
                                             uint32_t src, void *priv);
static int OpenampTask_PowerSendCommand(const char *cmd);

#if ENABLE_DISPLAY_TASK
static void OpenampTask_DisplayEndpointRegister(void);
#endif

static void OpenampTask_ScmiPowerEventListener(ScmiPowerEvent_t event, void *context);

#if defined(ENABLE_BTN_MONITOR_TASK)
static void OpenAMP_RebootButtonHandler(ButtonEventType_t event, void *context);
#endif /* ENABLE_BTN_MONITOR_TASK */

/* Private variables ---------------------------------------------------------*/
extern IPCC_HandleTypeDef hipcc;

typedef enum
{
  OPENAMP_TASK_STATE_STOPPED = 0,
  OPENAMP_TASK_STATE_READY,
} OpenampTaskState_t;

/**
  * @brief OpenAMP task handle.
  */
static osThreadId_t OpenampTaskHandle;

static osMessageQueueId_t OpenampTaskEventQueue;
static const osMessageQueueAttr_t OpenampTaskEventQueueAttr =
{
  .name = "OpenampTaskEventQueue",
};

static OpenampTaskState_t OpenampTaskState = OPENAMP_TASK_STATE_STOPPED;

/**
  * @brief OpenAMP task thread attributes.
  */
static const osThreadAttr_t OpenampTask_Attributes =
{
  .name       = "OpenampTask",
  .priority   = (osPriority_t)OPENAMP_TASK_PRIORITY,
  .stack_size = OPENAMP_TASK_STACK_SIZE
};

/**
  * @brief RPMsg power endpoint handle.
  */
static struct rpmsg_endpoint power_ept;

#if ENABLE_DISPLAY_TASK
/**
  * @brief RPMsg display endpoint handle.
  */
static struct rpmsg_endpoint display_ept;
#endif

/* Private functions ---------------------------------------------------------*/

bool OpenampTask_PostCommand(const OpenampTaskCommand_t *cmd)
{
  if ((OpenampTaskEventQueue == NULL) || (cmd == NULL))
  {
    return false;
  }

  return (osMessageQueuePut(OpenampTaskEventQueue, cmd, 0U, 0U) == osOK);
}

static void OpenampTask_MboxRxCallback(uint32_t vring_id)
{
  OpenampTaskCommand_t cmd = { .type = OPENAMP_CMD_RX_EVENT, .param = vring_id };
  (void)OpenampTask_PostCommand(&cmd);
}

/**
  * @brief  Function implementing the ThreadOpenamp thread.
  * @param  argument: Not used.
  * @retval None
  */
static void OpenampTask(void *argument)
{
  (void)argument;

  OpenampTaskCommand_t cmd;

  for (;;)
  {
    if (osMessageQueueGet(OpenampTaskEventQueue, &cmd, NULL, osWaitForever) == osOK)
    {
      switch (cmd.type)
      {
        case OPENAMP_CMD_INIT:
        {
          if (OpenampTaskState == OPENAMP_TASK_STATE_READY)
          {
            break;
          }

          if (MX_OPENAMP_Init(RPMSG_REMOTE, NULL, &hipcc) != HAL_OK)
          {
            APP_LOG_ERR("OpenAMP", "OpenAMP init failed");
            OpenampTaskState = OPENAMP_TASK_STATE_STOPPED;
            break;
          }

#if ENABLE_SCMI_MGR_TASK
          ScmiMgrTask_RegisterListener(OpenampTask_ScmiPowerEventListener, NULL);
#endif /* ENABLE_SCMI_MGR_TASK */

#if defined(ENABLE_BTN_MONITOR_TASK)
          BtnMonitorTask_RegisterListener(BUTTON_EVENT_VERY_LONG_PRESS, OpenAMP_RebootButtonHandler, NULL);
#endif /* ENABLE_BTN_MONITOR_TASK */

#if ENABLE_DISPLAY_TASK
          OpenampTask_DisplayEndpointRegister();
#endif

          OpenampTaskState = OPENAMP_TASK_STATE_READY;
          break;
        }

        case OPENAMP_CMD_RX_EVENT:
        {
          if (OpenampTaskState != OPENAMP_TASK_STATE_READY)
          {
            break;
          }

          (void)cmd.param;
          (void)OPENAMP_check_for_message();
          break;
        }

        case OPENAMP_CMD_REINIT:
        {
          APP_LOG_INF("OpenAMP", "OpenAMP reinit requested");

          if (OpenampTaskState == OPENAMP_TASK_STATE_READY)
          {
#if defined(ENABLE_SCMI_MGR_TASK)
            (void)ScmiMgrTask_UnregisterListener(OpenampTask_ScmiPowerEventListener);
#endif /* ENABLE_SCMI_MGR_TASK */

#if defined(ENABLE_BTN_MONITOR_TASK)
            BtnMonitorTask_UnregisterListener(BUTTON_EVENT_VERY_LONG_PRESS);
#endif /* ENABLE_BTN_MONITOR_TASK */

#if ENABLE_DISPLAY_TASK
            if (display_ept.addr == OPENAMP_DISPLAY_ENDPOINT_ADDR)
            {
              display_ept.addr = 0;
            }
#endif

            if (power_ept.addr == OPENAMP_POWER_ENDPOINT_ADDR)
            {
              power_ept.addr = 0;
            }

            OPENAMP_DeInit(&hipcc);
          }

          OpenampTaskState = OPENAMP_TASK_STATE_STOPPED;
          osDelay(100U);
          OpenampTaskCommand_t init_cmd = { .type = OPENAMP_CMD_INIT, .param = 0U };
          (void)OpenampTask_PostCommand(&init_cmd);
          break;
        }

        case OPENAMP_CMD_SHUTDOWN:
        {
          if (OpenampTaskState != OPENAMP_TASK_STATE_READY)
          {
            APP_LOG_ERR("OpenAMP", "shutdown requested while OpenAMP not ready");
            break;
          }

          (void)OpenampTask_PowerSendCommand("shutdown\n");
          break;
        }

        case OPENAMP_CMD_REBOOT:
        {
          if (OpenampTaskState != OPENAMP_TASK_STATE_READY)
          {
            APP_LOG_ERR("OpenAMP", "reboot requested while OpenAMP not ready");
            break;
          }

          (void)OpenampTask_PowerSendCommand("reboot\n");
          break;
        }

        default:
          break;
      }
    }
  }
}

#if ENABLE_DISPLAY_TASK
/**
 * @brief  Register the Display RPMsg endpoint.
 * @note   Safe to call multiple times.
 */
static void OpenampTask_DisplayEndpointRegister(void)
{
  int status;

  if (display_ept.addr == OPENAMP_DISPLAY_ENDPOINT_ADDR)
  {
    return;
  }

  APP_LOG_DBG("OpenAMP", "Enable RPMSG for Display");
  status = OPENAMP_create_fixed_endpoint(&display_ept,
                                        "display",
                                        OPENAMP_DISPLAY_ENDPOINT_ADDR,
                                        OPENAMP_DISPLAY_ENDPOINT_ADDR,
                                        DisplayTask_RpmsgCallback,
                                        NULL);
  if (status != 0)
  {
    APP_LOG_ERR("OpenAMP", "Display endpoint creation failed (%d)", status);
    return;
  }
}
#endif

/**
 * @brief  RPMsg callback for the power endpoint.
 *         This endpoint is TX-only; inbound messages are unexpected.
 * @param  ept   Local RPMsg endpoint
 * @param  data  Pointer to received message buffer
 * @param  len   Length of received message
 * @param  src   Remote endpoint address
 * @param  priv  User private context (unused)
 * @retval -1 Always returns error as inbound messages are not supported
 */
static int OpenampTask_PowerEndpointCallback(struct rpmsg_endpoint *ept, void *data, size_t len,
                                      uint32_t src, void *priv)
{
  (void)ept;
  (void)data;
  (void)len;
  (void)src;
  (void)priv;

  APP_LOG_ERR("OpenAMP", "Power endpoint: unexpected inbound message");
  return -1;
}

/**
 * @brief  Send a power management command over RPMsg.
 *         Supported commands: "shutdown\n", "reboot\n".
 * @param  cmd Command string including line terminator (e.g. "shutdown\n")
 * @retval 0 on success, negative error code otherwise
 */
static int OpenampTask_PowerSendCommand(const char *cmd)
{
  int status;
  const char *action;

  if (cmd == NULL)
  {
    APP_LOG_ERR("OpenAMP", "Invalid power command: (null)");
    return RPMSG_ERR_PARAM;
  }

  if (strcmp(cmd, "shutdown\n") == 0)
  {
    action = "shutdown";
  }
  else if (strcmp(cmd, "reboot\n") == 0)
  {
    action = "reboot";
  }
  else
  {
    APP_LOG_ERR("OpenAMP", "Invalid power command: %s", cmd);
    return RPMSG_ERR_PARAM;
  }

  APP_LOG_INF("OpenAMP", "request remote processor %s", action);

  if (power_ept.addr != OPENAMP_POWER_ENDPOINT_ADDR)
  {
    status = OPENAMP_create_fixed_endpoint(&power_ept,
                                           "power",
                                           OPENAMP_POWER_ENDPOINT_ADDR,
                                           OPENAMP_POWER_ENDPOINT_ADDR,
                                           OpenampTask_PowerEndpointCallback,
                                           NULL);
    if (status != 0)
    {
      APP_LOG_ERR("OpenAMP", "power endpoint creation failed (%d)", status);
      return status;
    }
  }

  return rpmsg_trysend(&power_ept, cmd, strlen(cmd) + 1U);
}

/**
 * @brief  SCMI power event listener.
 * @param  event   SCMI power event.
 * @param  context User context pointer (unused).
 * @retval None
 */
static void OpenampTask_ScmiPowerEventListener(ScmiPowerEvent_t event, void *context)
{
  (void)context;

  if (event == SCMI_POWER_EVENT_WARM_RESET)
  {
    OpenampTaskCommand_t cmd = { .type = OPENAMP_CMD_REINIT, .param = 0U };
    (void)OpenampTask_PostCommand(&cmd);
  }
}

#if defined(ENABLE_BTN_MONITOR_TASK)
/**
 * @brief  Button event handler to trigger a reboot.
 * @retval None
 */
static void OpenAMP_RebootButtonHandler(ButtonEventType_t event, void *context)
{
  (void)event;
  (void)context;

  OpenampTaskCommand_t cmd = { .type = OPENAMP_CMD_REBOOT, .param = 0U };
  (void)OpenampTask_PostCommand(&cmd);
}
#endif /* ENABLE_BTN_MONITOR_TASK */

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Application Thread Initialization.
  * @retval None
  */
void OpenampTask_Init(void)
{
  if (OpenampTaskHandle != NULL)
  {
    return;
  }

  if (OpenampTaskEventQueue == NULL)
  {
    OpenampTaskEventQueue = osMessageQueueNew(20U, sizeof(OpenampTaskCommand_t), &OpenampTaskEventQueueAttr);
    if (OpenampTaskEventQueue == NULL)
    {
      NSAppCore_ErrorHandler();
    }
  }

  OPENAMP_RegisterMBOXRxCallback(OpenampTask_MboxRxCallback);

  if (openampDriver.init() != 0)
  {
    APP_LOG_ERR("OpenAMP", "OpenAMP platform init failed");
    NSAppCore_ErrorHandler();
  }

  OpenampTaskHandle = osThreadNew(OpenampTask, NULL, &OpenampTask_Attributes);
  if (OpenampTaskHandle == NULL)
  {
    NSAppCore_ErrorHandler();
  }

  OpenampTaskCommand_t cmd = { .type = OPENAMP_CMD_INIT, .param = 0U };
  (void)OpenampTask_PostCommand(&cmd);
}

/**
  * @brief  Application Thread DeInitialization.
  * @retval None
  */
void OpenampTask_DeInit(void)
{
  if (OpenampTaskHandle != NULL)
  {
    (void)osThreadTerminate(OpenampTaskHandle);
    OpenampTaskHandle = NULL;
  }

#if ENABLE_DISPLAY_TASK
  if (display_ept.addr == OPENAMP_DISPLAY_ENDPOINT_ADDR)
  {
    display_ept.addr = 0;
  }
#endif

  if (power_ept.addr == OPENAMP_POWER_ENDPOINT_ADDR)
  {
    power_ept.addr = 0;
  }

  OPENAMP_DeInit(&hipcc);

#if defined(ENABLE_SCMI_MGR_TASK)
  (void)ScmiMgrTask_UnregisterListener(OpenampTask_ScmiPowerEventListener);
#endif /* ENABLE_SCMI_MGR_TASK */

#if defined(ENABLE_BTN_MONITOR_TASK)
  BtnMonitorTask_UnregisterListener(BUTTON_EVENT_VERY_LONG_PRESS);
#endif /* ENABLE_BTN_MONITOR_TASK */

  openampDriver.deinit();

  if (OpenampTaskEventQueue != NULL)
  {
    (void)osMessageQueueDelete(OpenampTaskEventQueue);
    OpenampTaskEventQueue = NULL;
  }

  OpenampTaskState = OPENAMP_TASK_STATE_STOPPED;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

