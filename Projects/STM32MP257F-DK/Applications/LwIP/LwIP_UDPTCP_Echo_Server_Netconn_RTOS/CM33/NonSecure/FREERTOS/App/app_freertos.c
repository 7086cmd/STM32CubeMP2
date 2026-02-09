/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_azure_rtos.c
  * @author  MCD Application Team
  * @brief   app_azure_rtos application implementation file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"
#include "cmsis_os2.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
#include "lwip/sio.h"
#endif /* MDK ARM Compiler */
#include "ethernetif.h"
#include "tcpecho.h"
#include "udpecho.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
void DHCP_Thread(void * argument);
void ethernet_link_thread(void* argument);
static void Netif_Config(void);
/* USER CODE END Includes */
extern struct netif gnetif;
extern ip4_addr_t ipaddr;
extern ip4_addr_t netmask;
extern ip4_addr_t gw;
extern uint8_t IP_ADDRESS[4];
extern uint8_t NETMASK_ADDRESS[4];
extern uint8_t GATEWAY_ADDRESS[4];

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static osThreadId_t StartThreadHandle;
static const osThreadAttr_t StartThread_attributes = {
  .name = "Thread One",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 2
};

static osThreadId_t ethernet_link_thread_handle;
static const osThreadAttr_t ethernet_link_thread_attributes = {
  .name = "Ethernet Link Thread",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 2
};

static osThreadId_t DHCPThreadHandle;
static const osThreadAttr_t DHCPThread_attributes = {
  .name = "DHCP Thread",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 2
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void StartThread(void *argument);
/* USER CODE END PFP */

  /**
  * @brief  Define the initial system.
  * @param  first_unused_memory : Pointer to the first unused memory
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of Task */
  StartThreadHandle = osThreadNew(StartThread, NULL, &StartThread_attributes);
  if (NULL == StartThreadHandle)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/**
  * @brief  Start Thread
  * @param  argument not used
  * @retval None
  */
static void StartThread(void *argument)
{
  /* Create tcp_ip stack thread */
  tcpip_init(NULL, NULL);

  /* Initialize the LwIP stack */
  Netif_Config();

  /* Initialize the TCP echo server thread */
  tcpecho_init();
  /* Initialize the UDP echo server thread */
  udpecho_init();

  for( ;; )
  {
    /* Delete the Init Thread */
    osThreadTerminate(NULL);
  }
}

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
/*Static IP ADDRESS*/
#define IP_ADDR0   ((uint8_t)10U)
#define IP_ADDR1   ((uint8_t)199U)
#define IP_ADDR2   ((uint8_t)84U)
#define IP_ADDR3   ((uint8_t)230U)

/*NETMASK*/
#define NETMASK_ADDR0   ((uint8_t)255U)
#define NETMASK_ADDR1   ((uint8_t)255U)
#define NETMASK_ADDR2   ((uint8_t)255U)
#define NETMASK_ADDR3   ((uint8_t)0U)

/*Gateway Address*/
#define GW_ADDR0   ((uint8_t)10U)
#define GW_ADDR1   ((uint8_t)199U)
#define GW_ADDR2   ((uint8_t)0U)
#define GW_ADDR3   ((uint8_t)1U)

static void Netif_Config(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;

#if LWIP_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else
  IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* LWIP_DHCP */

  /* add the network interface */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /*  Registers the default network interface. */
  netif_set_default(&gnetif);

  ethernet_link_status_updated(&gnetif);

#if LWIP_NETIF_LINK_CALLBACK
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);

  ethernet_link_thread_handle = osThreadNew(ethernet_link_thread, &gnetif, &ethernet_link_thread_attributes);
  if (NULL == ethernet_link_thread_handle)
  {
    Error_Handler();
  }
#endif

#if LWIP_DHCP
  /* Start DHCPClient */
  DHCPThreadHandle = osThreadNew(DHCP_Thread, &gnetif, &DHCPThread_attributes);
  if (NULL == DHCPThreadHandle)
  {
    Error_Handler();
  }
#endif
}

