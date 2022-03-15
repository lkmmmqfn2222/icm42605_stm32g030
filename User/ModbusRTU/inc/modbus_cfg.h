/*******************************************************************************
*   Filename:       modbus_cfg.h
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2016-06-29
*   Revision:       v1.0
*   Writer:	        Roger-WY
*
*   Description:     Modbus通信配置头文件
*******************************************************************************/
#ifndef __MODBUS_CFG_H__
#define __MODBUS_CFG_H__

#include <stdint.h>

/*
Modbus-rtu通过时间间隔确定分帧，所以源码中使用到了硬件定时器，需要在源码中配置定时器
*/
/***********************************************
* 描述： Modbus主机或者从机是否使能
*/
#define MODBUS_CFG_HOST_EN     0
#define MODBUS_CFG_SLAVE_EN    1

#if (MODBUS_CFG_HOST_EN == 1)
/***********************************************
* 描述： Modbus主机设备 宏定义配置
*/
#define MODBUS_HOST_DEBUG   0           /* 主机Modbus调试信息输出 */

#define MODBUS_HOST_UARTX   COM1        /* 主机通讯使用的串口号 */
#define MODBUS_HOST_BAUD    UART1_BAUD  /* 主机通讯使用的波特率 */

/***********************************************
* 描述： Modbus使用的串口缓存区
*/
#define H_RX_BUF_SIZE		64
#define H_TX_BUF_SIZE      	128

#endif

#if (MODBUS_CFG_SLAVE_EN == 1)
/***********************************************
* 描述： Modbus从机设备 宏定义配置
*/
#define MODBUS_SLAVE_UARTX  COM1           /* 从机通讯使用的串口号 */
#define MODBUS_SLAVE_BAUD	UART1_BAUD     /* 从机通讯使用的波特率 */

#define MODBUS_SLAVE_ADDR	1              /* 定义Modbus从站地址 */

#define MODBUS_SLAVE_DEBUG  0              /* 是否输出Modbus调试信息 */

/***********************************************
* 描述： Modbus使用的串口缓存区
*/
#define S_RX_BUF_SIZE		128
#define S_TX_BUF_SIZE		64

#endif







#endif