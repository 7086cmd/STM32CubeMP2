/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Socket_RTOS/Src/httpserver_socket.c
  * @author  MCD Application Team
  * @brief   Basic http server implementation using LwIP socket API  
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/apps/fs.h"
#include "string.h"
#include "httpserver_socket.h"
#include "cmsis_os.h"

#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define WEBSERVER_THREAD_PRIO    ( osPriorityAboveNormal )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void http_server_serve(int conn);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief serve tcp connection  
  * @param conn: connection socket 
  * @retval None
  */
void http_server_serve(int conn) 
{
  int buflen = 1500;
  int ret;
  struct fs_file file;
  unsigned char recv_buffer[1500];
				
  /* Read in the request */
  ret = read(conn, recv_buffer, buflen); 
  if(ret < 0) return;

  /* Check if request to get /img/sics.gif */
  if (strncmp((char *)recv_buffer,"GET /img/sics.gif", 17) == 0)
  {
    if (ERR_OK == fs_open(&file, "/img/sics.gif"))
    {
			write(conn, (const unsigned char*)(file.data), (size_t)file.len);
			fs_close(&file);
    }
    else
    {
    	if (ERR_OK == fs_open(&file, "/404.html"))
    	{
  			write(conn, (const unsigned char*)(file.data), (size_t)file.len);
  			fs_close(&file);
    	}
    }
  }
  /* Check if request to get index.html */
  else if ((strncmp((char *)recv_buffer,"GET /index.html", 15) == 0) || (strncmp((char *)recv_buffer, "GET / ", 6) == 0))
  {
    if (ERR_OK == fs_open(&file, "/index.html"))
    {
			write(conn, (const unsigned char*)(file.data), (size_t)file.len);
			fs_close(&file);
    }
    else
    {
    	if (ERR_OK == fs_open(&file, "/404.html"))
    	{
  			write(conn, (const unsigned char*)(file.data), (size_t)file.len);
  			fs_close(&file);
    	}
    }
  }
  else
  {
    /* Load 404 page */
  	if (ERR_OK == fs_open(&file, "/404.html"))
  	{
			write(conn, (const unsigned char*)(file.data), (size_t)file.len);
			fs_close(&file);
  	}
  }
  /* Close connection socket */
  close(conn);
}

/**
  * @brief  http server thread 
  * @param arg: pointer on argument(not used here) 
  * @retval None
  */
static void http_server_socket_thread(void *arg)
{
  int sock, newconn, size;
  struct sockaddr_in address, remotehost;

 /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
  {
    return;
  }
  
  /* bind to port 80 at any interface */
  address.sin_family = AF_INET;
  address.sin_port = htons(80);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
  {
    return;
  }
  
  /* listen for incoming connections (TCP listen backlog = 5) */
  listen(sock, 5);
  
  size = sizeof(remotehost);
  
  while (1) 
  {
    newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    http_server_serve(newconn);
  }
}

/**
  * @brief  Initialize the HTTP server (start its thread) 
  * @param  none
  * @retval None
  */
void http_server_socket_init()
{
  sys_thread_new("HTTP", http_server_socket_thread, NULL, DEFAULT_THREAD_STACKSIZE * 20, WEBSERVER_THREAD_PRIO);
}
