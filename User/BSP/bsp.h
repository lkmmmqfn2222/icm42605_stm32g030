/*
*********************************************************************************************************
*
*	模块名称 : BSP模块(For STM32G0)
*	文件名称 : bsp.h
*	版    本 : V1.0
*	说    明 : 这是硬件底层驱动程序的主文件。每个c文件可以 #include "bsp.h" 来包含所有的外设驱动模块。
*			   bsp = Borad surport packet 板级支持包
*
*	Copyright (C), 2018-2030
*
*********************************************************************************************************
*/

#ifndef __BSP_H__
#define __BSP_H__

  
#include "stm32g0xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "SEGGER_RTT.h"


/* CPU空闲时执行的函数 */
//#define CPU_IDLE()		bsp_Idle()

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */

/* 这个宏仅用于调试阶段排错 */
#define BSP_Printf		printf
//#define BSP_Printf(...)


#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

/* 定义优先级分组 */
#define NVIC_PREEMPT_PRIORITY	4

/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */
#include "bsp_timer.h"
#include "bsp_beep.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_uart_fifo.h"
#include "bsp_tim_pwm.h"
#include "bsp_iwdg.h"


#if defined(MODBUS_RTU_ENABLE)
#include "modbus_rtu.h"
#endif

/* 提供给其他C文件调用的函数 */
void bsp_Init(void);
void bsp_Idle(void);

void User_Error_Handler(char *file, uint32_t line);
extern void SystemClock_Config(void);

#endif

/*****************************  (END OF FILE) *********************************/
