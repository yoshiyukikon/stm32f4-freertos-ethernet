/* Library includes. */
#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "stm32f4xx.h"

extern GPIO_InitTypeDef GPIO_InitStructure;
extern NVIC_InitTypeDef NVIC_InitStructure;

/* TIM4 Autoreload and Capture Compare register values */
#define TIM_ARR                          ((uint16_t)1900)
#define TIM_CCR                          ((uint16_t)1000)

/* definitions used by prvLED_Config() to reconfigure LED pins GPIO<->TIM4 */
#define GPIO	0
#define TIMER	1

void prvSetupHardware( void );
void prvTIM4_Config(void);
void prvLED_Config(char state);

#endif /*HW_CONFIG_H*/
