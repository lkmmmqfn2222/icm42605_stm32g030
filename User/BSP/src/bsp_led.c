/*
*********************************************************************************************************
*
*	模块名称 : LED显示模块 
*	文件名称 : bsp_led.c
*	版    本 : V1.0
*	说    明 : 
*
*	修改记录 :
*
*	Copyright (C), 2016-2020
*
*********************************************************************************************************
*/

#include "bsp_led.h"
#include "stm32g0xx_hal.h"


#define GPIO_RCC_LED1()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define GPIO_PORT_LED1	  GPIOA
#define GPIO_PIN_LED1	  GPIO_PIN_0

#define GPIO_RCC_LED2()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define GPIO_PORT_LED2	  GPIOA
#define GPIO_PIN_LED2	  GPIO_PIN_0

/*
*********************************************************************************************************
*	函 数 名: bsp_LedInit
*	功能说明: LED初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/                                                                                                                                                                                                                                                                                                                                                                                      
void bsp_LedInit(void)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    GPIO_RCC_LED1();
    GPIO_RCC_LED2();

	bsp_LedOff(1);
	bsp_LedOff(2);

    /*Configure GPIO pins */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = GPIO_PIN_LED1;
    HAL_GPIO_Init(GPIO_PORT_LED1, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_LED2;
    HAL_GPIO_Init(GPIO_PORT_LED2, &GPIO_InitStruct);
    
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOn
*	功能说明: 指定LED亮
*	形    参: LED
*	返 回 值: 无
*********************************************************************************************************
*/    
void bsp_LedOn(uint8_t _no)
{
	if (_no == 1)
	{
		HAL_GPIO_WritePin(GPIO_PORT_LED1,GPIO_PIN_LED1,GPIO_PIN_RESET);	
	}
	else if (_no == 2)
	{
		HAL_GPIO_WritePin(GPIO_PORT_LED2,GPIO_PIN_LED2,GPIO_PIN_RESET);	
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOn
*	功能说明: 指定LED灭
*	形    参: LED
*	返 回 值: 无
*********************************************************************************************************
*/  
void bsp_LedOff(uint8_t _no)
{
    if (_no == 1)
	{
		HAL_GPIO_WritePin(GPIO_PORT_LED1,GPIO_PIN_LED1,GPIO_PIN_SET);	
	}
	else if (_no == 2)
	{
		HAL_GPIO_WritePin(GPIO_PORT_LED2,GPIO_PIN_LED2,GPIO_PIN_SET);	
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOn
*	功能说明: 指定LED翻转
*	形    参: LED
*	返 回 值: 无
*********************************************************************************************************
*/  
void bsp_LedToggle(uint8_t _no)
{
    if (_no == 1)
	{
		HAL_GPIO_TogglePin(GPIO_PORT_LED1,GPIO_PIN_LED1);	
	}
	else if (_no == 2)
	{
		HAL_GPIO_TogglePin(GPIO_PORT_LED2,GPIO_PIN_LED2);	
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOn
*	功能说明: 获取LED是否处于亮的状态
*	形    参: LED
*	返 回 值: 无
*********************************************************************************************************
*/  
uint8_t bsp_IsLedOn(uint8_t _no)
{
    
    return 0;
}

/*****************************  (END OF FILE) *********************************/
