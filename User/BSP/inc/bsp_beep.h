/*
*********************************************************************************************************
*
*	模块名称 : 蜂鸣器模块
*	文件名称 : bsp_beep.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2012-2020
*
*********************************************************************************************************
*/

#ifndef __BSP_BEEP_H__
#define __BSP_BEEP_H__

#include <stdint.h>

typedef struct _BEEP_T
{
	uint8_t 	ucEnalbe;
	uint8_t 	ucState;
	uint16_t 	usBeepTime;
	uint16_t 	usStopTime;
	uint16_t 	usCycle;
	uint16_t 	usCount;
	uint16_t 	usCycleCount;
	uint8_t 	ucMute;		/* 1表示静音 */	
}BEEP_T;

/* 供外部调用的函数声明 */
void bsp_BeepInit(void);
void bsp_BeepStart(uint16_t _usBeepTime, uint16_t _usStopTime, uint16_t _usCycle);
void bsp_BeepStop(void);
void bsp_BeepPause(void);
void bsp_BeepResume(void);
void bsp_BeepKeyTone(void);

void bsp_BeepPro(void);

#endif

/***************************** (END OF FILE) *********************************/
