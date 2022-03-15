/*
*********************************************************************************************************
*
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2013-2014, 
*
*********************************************************************************************************
*/

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include <stdint.h>


/* 供外部调用的函数声明 */
void bsp_LedInit(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
uint8_t bsp_IsLedOn(uint8_t _no);

#endif

/***************************** (END OF FILE) *********************************/
