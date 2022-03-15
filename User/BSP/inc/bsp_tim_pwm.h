/*
*********************************************************************************************************
*
*    模块名称 : 利用STM32内部TIM输出PWM信号，顺便实现
*    文件名称 : bsp_tim_pwm.h
*    版    本 : V1.0
*    说    明 : 头文件
*
*
*********************************************************************************************************
*/
#ifndef __BSP_TIM_PWM_H__
#define __BSP_TIM_PWM_H__


#include "stm32g0xx_hal.h"

void bsp_SetTIMOutPWM(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_TypeDef* TIMx, uint8_t _ucChannel,
     uint32_t _ulFreq, uint32_t _ulDutyCycle);
      

void bsp_SetTIMforInt(TIM_TypeDef* TIMx, uint32_t _ulFreq, uint8_t _PreemptionPriority, uint8_t _SubPriority);


#endif

/***************************** (END OF FILE) *********************************/
