/*
*********************************************************************************************************
*
*	模块名称 : cpu内部falsh操作模块
*	文件名称 : bsp_cpu_flash.h
*	版    本 : V1.0
*
*	Copyright (C), 2013-2021
*
*********************************************************************************************************
*/

#ifndef __BSP_CPU_FLASH_H__
#define __BSP_CPU_FLASH_H__

#include <stdint.h>
#include "stm32g0xx_hal.h"

#define FLASH_PAGE_NUM      (((*((uint32_t *)FLASHSIZE_BASE)) & (0x007FU)) / 2)

#define FLASH_USER_START_ADDR   (FLASH_BASE + ((FLASH_PAGE_NUM - 1) * 2048))   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (FLASH_BASE + ((FLASH_PAGE_NUM - 1) * 2048))   /* End @ of user Flash area */


#define FLASH_IS_EQU		0   /* Flash内容和待写入的数据相等，不需要擦除和写操作 */
#define FLASH_REQ_WRITE		1	/* Flash不需要擦除，直接写 */
#define FLASH_REQ_ERASE		2	/* Flash需要先擦除,再写 */
#define FLASH_PARAM_ERR		3	/* 函数参数错误 */

//uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint32_t* _ucpDst, uint32_t _ulSize);
uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpDst, uint32_t _ulSize);
uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpSrc, uint32_t _ulSize);
uint8_t bsp_CmpCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpBuf, uint32_t _ulSize);

void bsp_FlashEnableReadProtection(void);
void bsp_FlashDisableReadProtection(void);

#endif


/***************************** (END OF FILE) *********************************/

