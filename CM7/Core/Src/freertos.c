/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "lwip/udp.h"
#include "tcp_server.h"
#include "inFlash.h"
/* USER CODE END Includes */

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
/* USER CODE BEGIN Variables */
int test1 = 0;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId ledTaskHandle;
osThreadId tcpTaskHandle;
osThreadId inFlashTaskHandle;
osSemaphoreId myBinarySem_lwipInitHandle;
osStaticSemaphoreDef_t myBinarySem_lwipInitControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartLedTask(void const * argument);
void StartTcpTask(void const * argument);
void StartInFlashTask(void const * argument);

extern void MX_LWIP_Init(void);
extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of myBinarySem_lwipInit */
  osSemaphoreStaticDef(myBinarySem_lwipInit, &myBinarySem_lwipInitControlBlock);
  myBinarySem_lwipInitHandle = osSemaphoreCreate(osSemaphore(myBinarySem_lwipInit), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of ledTask */
  osThreadDef(ledTask, StartLedTask, osPriorityIdle, 0, 512);
  ledTaskHandle = osThreadCreate(osThread(ledTask), NULL);

  /* definition and creation of tcpTask */
  osThreadDef(tcpTask, StartTcpTask, osPriorityIdle, 0, 512);
  tcpTaskHandle = osThreadCreate(osThread(tcpTask), NULL);

  /* definition and creation of inFlashTask */
  osThreadDef(inFlashTask, StartInFlashTask, osPriorityIdle, 0, 256);
  inFlashTaskHandle = osThreadCreate(osThread(inFlashTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();

  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  const char* message = "Hello UDP message!\n\r";

  osDelay(1000);
  printf("MX_LWIP_Init\r\n");
  osSemaphoreRelease(myBinarySem_lwipInitHandle);

  ip_addr_t PC_IPADDR;
  IP_ADDR4(&PC_IPADDR, 192, 168, 7, 2);

  struct udp_pcb* my_udp = udp_new();
  udp_connect(my_udp, &PC_IPADDR, 55151);
  struct pbuf* udp_buffer = NULL;

  /* Infinite loop */
  for (;;) {
    osDelay(1000);
    /* !! PBUF_RAM is critical for correct operation !! */
    udp_buffer = pbuf_alloc(PBUF_TRANSPORT, strlen(message), PBUF_RAM);

    if (udp_buffer != NULL) {
      memcpy(udp_buffer->payload, message, strlen(message));
      udp_send(my_udp, udp_buffer);
      pbuf_free(udp_buffer);
    }
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartLedTask */
/**
* @brief Function implementing the ledTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLedTask */
void StartLedTask(void const * argument)
{
  /* USER CODE BEGIN StartLedTask */
  uint8_t LD_STATE = 0;
  /* Infinite loop */
  for(;;)
  {
	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, (GPIO_PinState)LD_STATE);
	osDelay(500);
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, (GPIO_PinState)LD_STATE);
	osDelay(500);
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, (GPIO_PinState)LD_STATE);
	osDelay(500);
	LD_STATE ^= 1;
  }
  /* USER CODE END StartLedTask */
}

/* USER CODE BEGIN Header_StartTcpTask */
/**
* @brief Function implementing the tcpTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTcpTask */
void StartTcpTask(void const * argument)
{
  /* USER CODE BEGIN StartTcpTask */
  osSemaphoreWait(myBinarySem_lwipInitHandle, osWaitForever);
  printf("tcp_server_init\r\n");
  tcp_server_init();
  extern struct netif gnetif;
  printf("IP address: %s\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));
  
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
    test1++;
    if(test1 > 100) test1 = 0;
  }
  /* USER CODE END StartTcpTask */
}

/* USER CODE BEGIN Header_StartInFlashTask */
/**
* @brief Function implementing the inFlashTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartInFlashTask */
void StartInFlashTask(void const * argument)
{
  /* USER CODE BEGIN StartInFlashTask */
  int debounce = 0;
  int key = 0;
  /* Infinite loop */
  for(;;)
  {
	GPIO_PinState pin = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
	if(GPIO_PIN_SET == pin){
		debounce++;
		if(debounce > 5){
			debounce = 0;
			key = 1;
		}
	}

	if(key == 1){
#if 0
		uint32_t wData[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
		int size = sizeof(wData) / sizeof(wData[0]);
		uint32_t *rData = malloc(sizeof(wData));

		Internal_WriteFlash(0x80cff00, wData, size);
		Internal_ReadFlash(0x80cff00, rData, size);

		free(rData);
#endif

	}
	key = 0;

    osDelay(1);
  }
  /* USER CODE END StartInFlashTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
