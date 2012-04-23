/**
  ******************************************************************************
  * @file    STM32F4-Discovery FreeRTOS demo\main.c
  * @author  T.O.M.A.S. Team
  * @version V1.1.0
  * @date    14-October-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "hw_config.h"
#include "stm32f4_discovery.h"

#include "config.h"
#include "LCD.h"
#include "TIM3_PWM.h"

#include <stdio.h>

/** @addtogroup STM32F4-Discovery_Demo
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DELAY 1000     /* msec */
#define queueSIZE	6

/* Private macro -------------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Task functions declarations */
static void vLEDTask( void *pvParameters );
static void vSWITCHTask( void *pvParameters );

/* handlers to tasks to better control them */
xTaskHandle xLED_Tasks[4];
xTaskHandle xMEMS_Task, xBALANCE_Task;

/* variables used by tasks */

/* initial arguments for vLEDTask task (which LED and what is the delay) */
static const int LEDS[4][2] = {{LED3,DELAY*1},
							   {LED4,DELAY*5},
							   {LED5,DELAY*10},
							   {LED6,DELAY*20}};

/* semaphores, queues declarations */
xSemaphoreHandle xSemaphoreSW  = NULL;
xQueueHandle xQueue;

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{ 
	/* create a pipe for MEMS->TIM4 data exchange */
	xQueue=xQueueCreate(1,queueSIZE*sizeof(uint8_t));

	/* create semaphores... */
	vSemaphoreCreateBinary( xSemaphoreSW );

	/* ...and clean them up */
	if(xSemaphoreTake(xSemaphoreSW, ( portTickType ) 0) == pdTRUE)
	{
	}

	/* initialize hardware... */
	prvSetupHardware();
	LCD_Initializtion();

	LCD_Clear(Red);
	GUI_Text(0,0,"Open207Z",White,Red);
	//LCD_Clear(Red);
	//GUI_Text(76,120,"Development Board V1.0",White,Red);

	/* Start the tasks defined within this file/specific to this demo. */
	xTaskCreate( vLEDTask, ( signed portCHAR * ) "LED3", configMINIMAL_STACK_SIZE, (void *)LEDS[0],tskIDLE_PRIORITY, &xLED_Tasks[0] );
	xTaskCreate( vLEDTask, ( signed portCHAR * ) "LED4", configMINIMAL_STACK_SIZE, (void *)LEDS[1],tskIDLE_PRIORITY, &xLED_Tasks[1] );
	xTaskCreate( vLEDTask, ( signed portCHAR * ) "LED5", configMINIMAL_STACK_SIZE, (void *)LEDS[2],tskIDLE_PRIORITY, &xLED_Tasks[2] );
	xTaskCreate( vLEDTask, ( signed portCHAR * ) "LED6", configMINIMAL_STACK_SIZE, (void *)LEDS[3],tskIDLE_PRIORITY, &xLED_Tasks[3] );
	xTaskCreate( vSWITCHTask, ( signed portCHAR * ) "SWITCH", configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY, NULL );

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was not enough heap space to create the idle task. */
	return 0;  
}

/*-----------------------------------------------------------*/

void vLEDTask( void *pvParameters )
{
    volatile int *LED;
    LED = (int *) pvParameters;

	for( ;; )
	{
		STM_EVAL_LEDToggle((Led_TypeDef)LED[0]);
	    vTaskDelay(LED[1]/portTICK_RATE_MS);
	}
}

/*-----------------------------------------------------------*/

void vSWITCHTask( void *pvParameters )
{
	static int i=0;
	for( ;; )
	{
		if(xSemaphoreTake(xSemaphoreSW,( portTickType ) 0) == pdTRUE)
		{
			i^=1;		//just switch the state if semaphore was given

			if(i==0)	//LED3..LD6 tasks ready, BALANCE, MEMS suspended
			{
				TIM_Cmd(TIM4, DISABLE);
				prvLED_Config(GPIO);
				vTaskResume(xLED_Tasks[0]);
				vTaskResume(xLED_Tasks[1]);
				vTaskResume(xLED_Tasks[2]);
				vTaskResume(xLED_Tasks[3]);
			}
			else		//MEMS and BALANCE ready, LED tasks suspended
			{
				vTaskSuspend(xLED_Tasks[0]);
				vTaskSuspend(xLED_Tasks[1]);
				vTaskSuspend(xLED_Tasks[2]);
				vTaskSuspend(xLED_Tasks[3]);
				prvLED_Config(TIMER);
				TIM_Cmd(TIM4, ENABLE);
			}
		}
		taskYIELD(); 	//task is going to ready state to allow next one to run
	}
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that 
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software 
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
