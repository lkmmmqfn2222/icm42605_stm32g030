/*
*********************************************************************************************************
*
*	模块名称 : 串口中断+FIFO驱动模块
*	文件名称 : bsp_uart_fifo.c
*	版    本 : V1.8
*	说    明 : 采用串口中断+FIFO模式实现多个串口的同时访问
*	修改记录 :
*
*	Copyright (C), 2015-2020,
*
*********************************************************************************************************
*/

#include <stdio.h>
#include "bsp_uart_fifo.h"
#include "bsp.h"
#include "main.h"

/* 串口1的GPIO  PB6, PB7 */
#define USART1_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()

#define USART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART1_TX_GPIO_PORT              GPIOB
#define USART1_TX_PIN                    GPIO_PIN_6
#define USART1_TX_AF                     GPIO_AF0_USART1    

#define USART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART1_RX_GPIO_PORT              GPIOB
#define USART1_RX_PIN                    GPIO_PIN_7
#define USART1_RX_AF                     GPIO_AF0_USART1    
/* 串口2的GPIO --- PA2 PA3 */
#define USART2_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()

#define USART2_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_TX_GPIO_PORT              GPIOA
#define USART2_TX_PIN                    GPIO_PIN_2
#define USART2_TX_AF                     GPIO_AF1_USART2

#define USART2_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_RX_GPIO_PORT              GPIOA
#define USART2_RX_PIN                    GPIO_PIN_3
#define USART2_RX_AF                     GPIO_AF1_USART2
/* 串口3的GPIO --- PB10 PB11  */
#define USART3_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE()

#define USART3_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART3_TX_GPIO_PORT              GPIOB
#define USART3_TX_PIN                    GPIO_PIN_10
#define USART3_TX_AF                     GPIO_AF7_USART3

#define USART3_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART3_RX_GPIO_PORT              GPIOB
#define USART3_RX_PIN                    GPIO_PIN_11
#define USART3_RX_AF                     GPIO_AF7_USART3
/* 串口4的GPIO --- PC10 PC11  */
#define UART4_CLK_ENABLE()              __HAL_RCC_UART4_CLK_ENABLE()

#define UART4_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART4_TX_GPIO_PORT              GPIOC
#define UART4_TX_PIN                    GPIO_PIN_10
#define UART4_TX_AF                     GPIO_AF8_UART4

#define UART4_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART4_RX_GPIO_PORT              GPIOC
#define UART4_RX_PIN                    GPIO_PIN_11
#define UART4_RX_AF                     GPIO_AF8_UART4
/* 串口5的GPIO --- PC12/UART5_TX PD2/UART5_RX  */
#define UART5_CLK_ENABLE()              __HAL_RCC_UART5_CLK_ENABLE()

#define UART5_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART5_TX_GPIO_PORT              GPIOC
#define UART5_TX_PIN                    GPIO_PIN_12
#define UART5_TX_AF                     GPIO_AF8_UART5

#define UART5_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define UART5_RX_GPIO_PORT              GPIOD
#define UART5_RX_PIN                    GPIO_PIN_2
#define UART5_RX_AF                     GPIO_AF8_UART5
/* 串口6的GPIO --- PG14 PC7  */
#define USART6_CLK_ENABLE()              __HAL_RCC_USART6_CLK_ENABLE()

#define USART6_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define USART6_TX_GPIO_PORT              GPIOG
#define USART6_TX_PIN                    GPIO_PIN_14
#define USART6_TX_AF                     GPIO_AF7_USART6

#define USART6_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define USART6_RX_GPIO_PORT              GPIOC
#define USART6_RX_PIN                    GPIO_PIN_7
#define USART6_RX_AF                     GPIO_AF7_USART6
/* 串口7的GPIO --- PB4/UART7_TX, PB3/UART7_RX  */
#define UART7_CLK_ENABLE()              __HAL_RCC_UART7_CLK_ENABLE()

#define UART7_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define UART7_TX_GPIO_PORT              GPIOB
#define UART7_TX_PIN                    GPIO_PIN_4
#define UART7_TX_AF                     GPIO_AF11_UART7

#define UART7_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define UART7_RX_GPIO_PORT              GPIOB
#define UART7_RX_PIN                    GPIO_PIN_3
#define UART7_RX_AF                     GPIO_AF11_UART7
/* 串口8的GPIO --- PJ8/UART8_TX, PJ9/UART8_RX */
#define UART8_CLK_ENABLE()              __HAL_RCC_UART8_CLK_ENABLE()

#define UART8_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOJ_CLK_ENABLE()
#define UART8_TX_GPIO_PORT              GPIOJ
#define UART8_TX_PIN                    GPIO_PIN_8
#define UART8_TX_AF                     GPIO_AF8_UART8

#define UART8_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOJ_CLK_ENABLE()
#define UART8_RX_GPIO_PORT              GPIOJ
#define UART8_RX_PIN                    GPIO_PIN_9
#define UART8_RX_AF                     GPIO_AF8_UART8

/* 定义每个串口结构体变量 */
#if UART1_FIFO_EN == 1

	static UART_T g_tUart1;
	static uint8_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART2_FIFO_EN == 1
	static UART_T g_tUart2;
	static uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART3_FIFO_EN == 1
	static UART_T g_tUart3;
	static uint8_t g_TxBuf3[UART3_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART4_FIFO_EN == 1
	static UART_T g_tUart4;
	static uint8_t g_TxBuf4[UART4_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART5_FIFO_EN == 1
	static UART_T g_tUart5;
	static uint8_t g_TxBuf5[UART5_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART6_FIFO_EN == 1
	static UART_T g_tUart6;
	static uint8_t g_TxBuf6[UART6_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf6[UART6_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART7_FIFO_EN == 1
	static UART_T g_tUart7;
	static uint8_t g_TxBuf7[UART7_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf7[UART7_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART8_FIFO_EN == 1
	static UART_T g_tUart8;
	static uint8_t g_TxBuf8[UART8_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_RxBuf8[UART8_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif
		
static void UartVarInit(void);

static void InitHardUart(void);
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);
static void UartIRQ(UART_T *_pUart);

void RS485_InitTXE(void);

/*
*********************************************************************************************************
*	函 数 名: bsp_UartInit
*	功能说明: 初始化串口硬件，并对全局变量赋初值.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_UartInit(void)
{
	
	UartVarInit();		/* 必须先初始化全局变量,再配置硬件 */

	InitHardUart();		/* 配置串口的硬件参数(波特率等) */
    
#if defined(ENABLE_RS584_TXEN)
	RS485_InitTXE();	/* 配置RS485芯片的发送使能硬件，配置为推挽输出 */
    #endif
}

/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为UART指针
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: uart指针
*********************************************************************************************************
*/
UART_T *ComToUart(COM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
		#if UART1_FIFO_EN == 1
			return &g_tUart1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM2)
	{
		#if UART2_FIFO_EN == 1
			return &g_tUart2;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM3)
	{
		#if UART3_FIFO_EN == 1
			return &g_tUart3;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM4)
	{
		#if UART4_FIFO_EN == 1
			return &g_tUart4;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM5)
	{
		#if UART5_FIFO_EN == 1
			return &g_tUart5;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM6)
	{
		#if UART6_FIFO_EN == 1
			return &g_tUart6;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM7)
	{
		#if UART7_FIFO_EN == 1
			return &g_tUart7;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM8)
	{
		#if UART8_FIFO_EN == 1
			return &g_tUart8;
		#else
			return 0;
		#endif
	}	
	else
	{
		Error_Handler();
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为 USART_TypeDef* USARTx
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: USART_TypeDef*,  USART1, USART2, USART3, UART4, UART5，USART6，UART7，UART8。
*********************************************************************************************************
*/
USART_TypeDef *ComToUSARTx(COM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
		#if UART1_FIFO_EN == 1
			return USART1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM2)
	{
		#if UART2_FIFO_EN == 1
			return USART2;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM3)
	{
		#if UART3_FIFO_EN == 1
			return USART3;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM4)
	{
		#if UART4_FIFO_EN == 1
			return UART4;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM5)
	{
		#if UART5_FIFO_EN == 1
			return UART5;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM6)
	{
		#if UART6_FIFO_EN == 1
			return USART6;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM7)
	{
		#if UART7_FIFO_EN == 1
			return UART7;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM8)
	{
		#if UART8_FIFO_EN == 1
			return UART8;
		#else
			return 0;
		#endif
	}	
	
	else
	{
		/* 不做任何处理 */
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: comSendBuf
*	功能说明: 向串口发送一组数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _ucaBuf: 待发送的数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	if (pUart->SendBefor != 0)
	{
		pUart->SendBefor();		/* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
	}

	UartSend(pUart, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	函 数 名: comSendChar
*	功能说明: 向串口发送1个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _ucByte: 待发送的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte)
{
	comSendBuf(_ucPort, &_ucByte, 1);
}

/*
*********************************************************************************************************
*	函 数 名: comGetChar
*	功能说明: 从接收缓冲区读取1字节，非阻塞。无论有无数据均立即返回。
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _pByte: 接收到的数据存放在这个地址
*	返 回 值: 0 表示无数据, 1 表示读取到有效字节
*********************************************************************************************************
*/
uint8_t comGetChar(COM_PORT_E _ucPort, uint8_t *_pByte)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return 0;
	}

	return UartGetChar(pUart, _pByte);
}

/*
*********************************************************************************************************
*	函 数 名: comClearTxFifo
*	功能说明: 清零串口发送缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearTxFifo(COM_PORT_E _ucPort)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usTxWrite = 0;
	pUart->usTxRead = 0;
	pUart->usTxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: comClearRxFifo
*	功能说明: 清零串口接收缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearRxFifo(COM_PORT_E _ucPort)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usRxWrite = 0;
	pUart->usRxRead = 0;
	pUart->usRxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: comSetBaud
*	功能说明: 设置串口的波特率. 本函数固定设置为无校验，收发都使能模式
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _BaudRate: 波特率，8倍过采样  波特率.0-12.5Mbps
*                                16倍过采样 波特率.0-6.25Mbps
*	返 回 值: 无
*********************************************************************************************************
*/
void comSetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate)
{
	USART_TypeDef* USARTx;
	
	USARTx = ComToUSARTx(_ucPort);
	if (USARTx == 0)
	{
		return;
	}
	
	bsp_SetUartParam(USARTx,  _BaudRate, UART_PARITY_NONE, UART_MODE_TX_RX);
}

/* 如果是RS485通信，请按如下格式编写函数， 我们仅举了 USART3作为RS485的例子 */
#if defined(ENABLE_RS584_TXEN)
/*
*********************************************************************************************************
*	函 数 名: RS485_InitTXE
*	功能说明: 配置RS485发送使能口线 TXE
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_InitTXE(void)
{
	GPIO_InitTypeDef gpio_init;
	
	/* 打开GPIO时钟 */
	RS485_TXEN_GPIO_CLK_ENABLE();
	
	/* 配置引脚为推挽输出 */
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	gpio_init.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	/* GPIO速度等级 */
	gpio_init.Pin = RS485_TXEN_PIN;
	HAL_GPIO_Init(RS485_TXEN_GPIO_PORT, &gpio_init);	
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendBefor
*	功能说明: 发送数据前的准备工作。对于RS485通信，请设置RS485芯片为发送状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendBefor = RS485_SendBefor
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendBefor(void)
{
	RS485_TX_EN();	/* 切换RS485收发芯片为发送模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendOver
*	功能说明: 发送一串数据结束后的善后处理。对于RS485通信，请设置RS485芯片为接收状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendOver = RS485_SendOver
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendOver(void)
{
	RS485_RX_EN();	/* 切换RS485收发芯片为接收模式 */
}

#endif

/*******************************************************************************
 * 名    称： ModbusSlave_Uart_ReciveNew
 * 功    能： Modbus从机接收到新的数据
 * 入口参数： _byte 接收到的新数据
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2016-07-19
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
#if  (MODBUS_CFG_SLAVE_EN == 1)
extern void MODS_ReciveNew(uint8_t _byte);
void ModbusSlave_Uart_ReciveNew(uint8_t _byte)
{
	MODS_ReciveNew(_byte);
}
#endif
/*******************************************************************************
 * 名    称： ModbusHost_Uart_ReciveNew
 * 功    能： Modbus主机接收到新的数据
 * 入口参数： _byte 接收到的新数据
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2016-07-19
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
#if  (MODBUS_CFG_HOST_EN == 1)
extern void MODH_ReciveNew(uint8_t _data);
void ModbusHost_Uart_ReciveNew(uint8_t _byte)
{
    MODH_ReciveNew(_byte);
}
#endif


/*
*********************************************************************************************************
*	函 数 名: UartVarInit
*	功能说明: 初始化串口相关的变量
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartVarInit(void)
{
#if UART1_FIFO_EN == 1
	g_tUart1.uart = USART1;						/* STM32 串口设备 */
	g_tUart1.pTxBuf = g_TxBuf1;					/* 发送缓冲区指针 */
	g_tUart1.pRxBuf = g_RxBuf1;					/* 接收缓冲区指针 */
	g_tUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart1.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart1.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart1.usRxRead  = 0;						/* 接收FIFO读索引 */
	g_tUart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart1.usTxCount = 0;						/* 待发送的数据个数 */
    #if defined(ENABLE_RS584_TXEN)    
	g_tUart1.SendBefor = RS485_SendBefor;		/* 发送数据前的回调函数 */
	g_tUart1.SendOver  = RS485_SendOver;			/* 发送完毕后的回调函数 */
    #else
	g_tUart1.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart1.SendOver  = 0;						/* 发送完毕后的回调函数 */
    #endif
    #if ((MODBUS_CFG_SLAVE_EN == 1) && (MODBUS_SLAVE_UARTX == COM1))
    g_tUart1.ReciveNew = ModbusSlave_Uart_ReciveNew;	    
    #elif ((MODBUS_CFG_HOST_EN == 1) && (MODBUS_HOST_UARTX == COM1))
    g_tUart1.ReciveNew = ModbusHost_Uart_ReciveNew;	
    #else
	g_tUart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
    #endif
	g_tUart1.Sending = 0;						/* 正在发送中标志 */
#endif

#if UART2_FIFO_EN == 1
	g_tUart2.uart = USART2;						/* STM32 串口设备 */
	g_tUart2.pTxBuf = g_TxBuf2;					/* 发送缓冲区指针 */
	g_tUart2.pRxBuf = g_RxBuf2;					/* 接收缓冲区指针 */
	g_tUart2.usTxBufSize = UART2_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart2.usRxBufSize = UART2_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart2.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart2.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart2.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart2.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart2.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart2.usTxCount = 0;						/* 待发送的数据个数 */
    #if defined(ENABLE_RS584_TXEN)    
	g_tUart2.SendBefor = RS485_SendBefor;		/* 发送数据前的回调函数 */
	g_tUart2.SendOver  = RS485_SendOver;			/* 发送完毕后的回调函数 */
    #else
	g_tUart2.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart2.SendOver  = 0;						/* 发送完毕后的回调函数 */
    #endif
    #if ((MODBUS_CFG_SLAVE_EN == 1) && (MODBUS_SLAVE_UARTX == COM2))
    g_tUart2.ReciveNew = ModbusSlave_Uart_ReciveNew;	    
    #elif ((MODBUS_CFG_HOST_EN == 1) && (MODBUS_HOST_UARTX == COM2))
    g_tUart2.ReciveNew = ModbusHost_Uart_ReciveNew;	
    #else
	g_tUart2.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
    #endif
	g_tUart2.Sending = 0;						/* 正在发送中标志 */
#endif

#if UART3_FIFO_EN == 1
	g_tUart3.uart = USART3;						/* STM32 串口设备 */
	g_tUart3.pTxBuf = g_TxBuf3;					/* 发送缓冲区指针 */
	g_tUart3.pRxBuf = g_RxBuf3;					/* 接收缓冲区指针 */
	g_tUart3.usTxBufSize = UART3_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart3.usRxBufSize = UART3_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart3.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart3.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart3.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart3.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart3.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart3.usTxCount = 0;						/* 待发送的数据个数 */
    #if defined(ENABLE_RS584_TXEN)    
	g_tUart3.SendBefor = RS485_SendBefor;		/* 发送数据前的回调函数 */
	g_tUart3.SendOver  = RS485_SendOver;			/* 发送完毕后的回调函数 */
    #else
	g_tUart3.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart3.SendOver  = 0;						/* 发送完毕后的回调函数 */
    #endif
    #if ((MODBUS_CFG_SLAVE_EN == 1) && (MODBUS_SLAVE_UARTX == COM3))
    g_tUart3.ReciveNew = ModbusSlave_Uart_ReciveNew;	    
    #elif ((MODBUS_CFG_HOST_EN == 1) && (MODBUS_HOST_UARTX == COM3))
    g_tUart3.ReciveNew = ModbusHost_Uart_ReciveNew;	
    #else
	g_tUart3.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
    #endif
	g_tUart3.Sending = 0;						/* 正在发送中标志 */
#endif

#if UART4_FIFO_EN == 1
	g_tUart4.uart = UART4;						/* STM32 串口设备 */
	g_tUart4.pTxBuf = g_TxBuf4;					/* 发送缓冲区指针 */
	g_tUart4.pRxBuf = g_RxBuf4;					/* 接收缓冲区指针 */
	g_tUart4.usTxBufSize = UART4_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart4.usRxBufSize = UART4_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart4.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart4.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart4.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart4.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart4.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart4.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart4.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart4.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart4.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart4.Sending = 0;						/* 正在发送中标志 */
#endif

#if UART5_FIFO_EN == 1
	g_tUart5.uart = UART5;						/* STM32 串口设备 */
	g_tUart5.pTxBuf = g_TxBuf5;					/* 发送缓冲区指针 */
	g_tUart5.pRxBuf = g_RxBuf5;					/* 接收缓冲区指针 */
	g_tUart5.usTxBufSize = UART5_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart5.usRxBufSize = UART5_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart5.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart5.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart5.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart5.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart5.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart5.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart5.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart5.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart5.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart5.Sending = 0;						/* 正在发送中标志 */
#endif


#if UART6_FIFO_EN == 1
	g_tUart6.uart = USART6;						/* STM32 串口设备 */
	g_tUart6.pTxBuf = g_TxBuf6;					/* 发送缓冲区指针 */
	g_tUart6.pRxBuf = g_RxBuf6;					/* 接收缓冲区指针 */
	g_tUart6.usTxBufSize = UART6_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart6.usRxBufSize = UART6_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart6.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart6.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart6.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart6.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart6.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart6.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart6.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart6.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart6.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart6.Sending = 0;						/* 正在发送中标志 */
#endif

#if UART7_FIFO_EN == 1
	g_tUart7.uart = UART7;						/* STM32 串口设备 */
	g_tUart7.pTxBuf = g_TxBuf7;					/* 发送缓冲区指针 */
	g_tUart7.pRxBuf = g_RxBuf7;					/* 接收缓冲区指针 */
	g_tUart7.usTxBufSize = UART7_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart7.usRxBufSize = UART7_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart7.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart7.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart7.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart7.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart7.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart7.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart7.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart7.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart7.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart7.Sending = 0;						/* 正在发送中标志 */
#endif

#if UART8_FIFO_EN == 1
	g_tUart8.uart = UART8;						/* STM32 串口设备 */
	g_tUart8.pTxBuf = g_TxBuf8;					/* 发送缓冲区指针 */
	g_tUart8.pRxBuf = g_RxBuf8;					/* 接收缓冲区指针 */
	g_tUart8.usTxBufSize = UART8_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart8.usRxBufSize = UART8_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart8.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart8.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart8.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart8.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart8.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart8.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart8.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart8.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart8.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart8.Sending = 0;						/* 正在发送中标志 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetUartParam
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32- H7开发板
*	形    参: Instance   USART_TypeDef类型结构体
*             BaudRate   波特率
*             Parity     校验类型，奇校验或者偶校验
*             Mode       发送和接收模式使能
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode)
{
  UART_HandleTypeDef UartHandle = {0};	
	
	/*##-1- 配置串口硬件参数 ######################################*/
	/* 异步串口模式 (UART Mode) */
	/* 配置如下:
	  - 字长    = 8 位
	  - 停止位  = 1 个停止位
	  - 校验    = 参数Parity
	  - 波特率  = 参数BaudRate
	  - 硬件流控制关闭 (RTS and CTS signals) */

	UartHandle.Instance        = Instance;

	UartHandle.Init.BaudRate   = BaudRate;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = Parity;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = Mode;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	UartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
	if (HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		Error_Handler();
	}
}

/*
*********************************************************************************************************
*	函 数 名: InitHardUart
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitHardUart(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

#if UART1_FIFO_EN == 1		/* 串口1 */
	/* 使能 GPIO TX/RX 时钟 */
	USART1_TX_GPIO_CLK_ENABLE();
	USART1_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	USART1_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = USART1_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = USART1_TX_AF;
	HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = USART1_RX_PIN;
	GPIO_InitStruct.Alternate = USART1_RX_AF;
	HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
  
	/* 配置波特率、奇偶校验 */
	bsp_SetUartParam(USART1,UART1_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	SET_BIT(USART1->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(USART1->RQR, USART_RQR_RXFRQ);  /* 清除RXNE接收标志 */
	// USART_CR1_PEIE | USART_CR1_RXNEIE
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE_RXFNEIE);	/* 使能PE. RX接受中断 */
#endif

#if UART2_FIFO_EN == 1		/* 串口2 */
	/* 使能 GPIO TX/RX 时钟 */
	USART2_TX_GPIO_CLK_ENABLE();
	USART2_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	USART2_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = USART2_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = USART2_TX_AF;
	HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = USART2_RX_PIN;
	GPIO_InitStruct.Alternate = USART2_RX_AF;
	HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
  
	/* 配置波特率、奇偶校验 */
	bsp_SetUartParam(USART2,  UART2_BAUD, UART_PARITY_NONE, UART_MODE_RX);	// UART_MODE_TX_RX

	SET_BIT(USART2->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(USART2->RQR, USART_RQR_RXFRQ);	/* 清除RXNE接收标志 */
	SET_BIT(USART2->CR1, USART_CR1_RXNEIE_RXFNEIE);	/* 使能PE. RX接受中断 */
#endif

#if UART3_FIFO_EN == 1			/* 串口3 */
	/* 使能 GPIO TX/RX 时钟 */
	USART3_TX_GPIO_CLK_ENABLE();
	USART3_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	USART3_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = USART3_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = USART3_TX_AF;
	HAL_GPIO_Init(USART3_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = USART3_RX_PIN;
	GPIO_InitStruct.Alternate = USART3_RX_AF;
	HAL_GPIO_Init(USART3_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(USART3_IRQn, 0, 3);
	HAL_NVIC_EnableIRQ(USART3_IRQn);
  
	/* 配置波特率、奇偶校验 */
	bsp_SetUartParam(USART3,  UART3_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	SET_BIT(USART3->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(USART3->RQR, USART_RQR_RXFRQ);/* 清除RXNE接收标志 */
	SET_BIT(USART3->CR1, USART_CR1_RXNEIE);	/* 使能PE. RX接受中断 */
#endif

#if UART4_FIFO_EN == 1			/* 串口4 TX = PC10   RX = PC11 */
	/* 使能 GPIO TX/RX 时钟 */
	UART4_TX_GPIO_CLK_ENABLE();
	UART4_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	UART4_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = UART4_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = UART4_TX_AF;
	HAL_GPIO_Init(UART4_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = UART4_RX_PIN;
	GPIO_InitStruct.Alternate = UART4_RX_AF;
	HAL_GPIO_Init(UART4_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(UART4_IRQn, 0, 4);
	HAL_NVIC_EnableIRQ(UART4_IRQn);
  
	/* 配置波特率、奇偶校验 */
	bsp_SetUartParam(UART4,  UART4_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	SET_BIT(UART4->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(UART4->RQR, USART_RQR_RXFRQ);/* 清除RXNE接收标志 */
	SET_BIT(UART4->CR1, USART_CR1_RXNEIE);	/* 使能RX接受中断 */
#endif

#if UART5_FIFO_EN == 1			/* 串口5 TX = PC12   RX = PD2 */
	/* 使能 GPIO TX/RX 时钟 */
	UART5_TX_GPIO_CLK_ENABLE();
	UART5_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	UART5_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = UART5_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = UART5_TX_AF;
	HAL_GPIO_Init(UART5_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = UART5_RX_PIN;
	GPIO_InitStruct.Alternate = UART5_RX_AF;
	HAL_GPIO_Init(UART5_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(UART5_IRQn, 0, 5);
	HAL_NVIC_EnableIRQ(UART5_IRQn);
  
	/* 配置波特率、奇偶校验 */
	bsp_SetUartParam(UART5,  UART5_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	SET_BIT(UART5->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(UART5->RQR, USART_RQR_RXFRQ);/* 清除RXNE接收标志 */
	SET_BIT(UART5->CR1, USART_CR1_RXNEIE);	/* 使能RX接受中断 */
#endif

#if UART6_FIFO_EN == 1			/* USART6 */
	/* 使能 GPIO TX/RX 时钟 */
	USART6_TX_GPIO_CLK_ENABLE();
	USART6_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	USART6_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = USART6_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = USART6_TX_AF;
	HAL_GPIO_Init(USART6_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = USART6_RX_PIN;
	GPIO_InitStruct.Alternate = USART6_RX_AF;
	HAL_GPIO_Init(USART6_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(USART6_IRQn, 0, 6);
	HAL_NVIC_EnableIRQ(USART6_IRQn);
	
	/* 配置波特率、奇偶校验 */
	bsp_SetUartParam(USART6,  UART6_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	SET_BIT(USART6->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(USART6->RQR, USART_RQR_RXFRQ);/* 清除RXNE接收标志 */
	SET_BIT(USART6->CR1, USART_CR1_RXNEIE);	/* 使能PE. RX接受中断 */
#endif

#if UART7_FIFO_EN == 1			/* UART7 */
	/* 使能 GPIO TX/RX 时钟 */
	UART7_TX_GPIO_CLK_ENABLE();
	UART7_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	UART7_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = UART7_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = UART7_TX_AF;
	HAL_GPIO_Init(UART7_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = UART7_RX_PIN;
	GPIO_InitStruct.Alternate = UART7_RX_AF;
	HAL_GPIO_Init(UART7_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(UART7_IRQn, 0, 6);
	HAL_NVIC_EnableIRQ(UART7_IRQn);
	
	/* 配置波特率、奇偶校验 */
	bsp_SetUartParam(UART7,  UART7_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	SET_BIT(UART7->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(UART7->RQR, USART_RQR_RXFRQ);	/* 清除RXNE接收标志 */
	SET_BIT(UART7->CR1, USART_CR1_RXNEIE);	/* 使能PE. RX接受中断 */
#endif

#if UART8_FIFO_EN == 1			/* UART8 */
	/* 使能 GPIO TX/RX 时钟 */
	UART8_TX_GPIO_CLK_ENABLE();
	UART7_RX_GPIO_CLK_ENABLE();
	
	/* 使能 USARTx 时钟 */
	UART8_CLK_ENABLE();	

	/* 配置TX引脚 */
	GPIO_InitStruct.Pin       = UART8_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = UART8_TX_AF;
	HAL_GPIO_Init(UART8_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* 配置RX引脚 */
	GPIO_InitStruct.Pin = UART8_RX_PIN;
	GPIO_InitStruct.Alternate = UART8_RX_AF;
	HAL_GPIO_Init(UART8_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(UART8_IRQn, 0, 6);
	HAL_NVIC_EnableIRQ(UART8_IRQn);
	
	/* 配置波特率、奇偶校验 */
	bsp_SetUartParam(UART8,  UART8_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);
	
	SET_BIT(UART8->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(UART8->RQR, USART_RQR_RXFRQ);	/* 清除RXNE接收标志 */
	SET_BIT(UART8->CR1, USART_CR1_RXNEIE);	/* 使能PE. RX接受中断 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: UartSend
*	功能说明: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t i;

	for (i = 0; i < _usLen; i++)
	{
		/* 如果发送缓冲区已经满了，则等待缓冲区空 */
		while (1)
		{
			__IO uint16_t usCount;

			DISABLE_INT();
			usCount = _pUart->usTxCount;
			ENABLE_INT();

			if (usCount < _pUart->usTxBufSize)
			{
				break;
			}
			else if(usCount == _pUart->usTxBufSize)/* 数据已填满缓冲区 */
			{
				if((_pUart->uart->CR1 & USART_CR1_TXEIE_TXFNFIE) == 0)
				{
					SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE_TXFNFIE);
				}  
			}
		}

		/* 将新数据填入发送缓冲区 */
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		_pUart->usTxCount++;
		ENABLE_INT();
	}

	SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE_TXFNFIE);	/* 使能发送中断（缓冲区空） */
}

/*
*********************************************************************************************************
*	函 数 名: UartGetChar
*	功能说明: 从串口接收缓冲区读取1字节数据 （用于主程序调用）
*	形    参: _pUart : 串口设备
*			  _pByte : 存放读取数据的指针
*	返 回 值: 0 表示无数据  1表示读取到数据
*********************************************************************************************************
*/
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte)
{
	uint16_t usCount;

	/* usRxWrite 变量在中断函数中被改写，主程序读取该变量时，必须进行临界区保护 */
	DISABLE_INT();
	usCount = _pUart->usRxCount;
	ENABLE_INT();

	/* 如果读和写索引相同，则返回0 */
	//if (_pUart->usRxRead == usRxWrite)
	if (usCount == 0)	/* 已经没有数据 */
	{
		return 0;
	}
	else
	{
		*_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* 从串口接收FIFO取1个数据 */

		/* 改写FIFO读索引 */
		DISABLE_INT();
		if (++_pUart->usRxRead >= _pUart->usRxBufSize)
		{
			_pUart->usRxRead = 0;
		}
		_pUart->usRxCount--;
		ENABLE_INT();
		return 1;
	}
}

/*
*********************************************************************************************************
*   函 数 名: UartTxEmpty
*   功能说明: 判断发送缓冲区是否为空。
*   形    参:  _pUart : 串口设备
*   返 回 值: 1为空。0为不空。
*********************************************************************************************************
*/
uint8_t UartTxEmpty(COM_PORT_E _ucPort)
{
   UART_T *pUart;
   uint8_t Sending;
   
   pUart = ComToUart(_ucPort);
   if (pUart == 0)
   {
      return 0;
   }

   Sending = pUart->Sending;

   if (Sending != 0)
   {
      return 0;
   }
   return 1;
}

/*
*********************************************************************************************************
*	函 数 名: UartIRQ
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: _pUart : 串口设备
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{
	uint32_t isrflags   = READ_REG(_pUart->uart->ISR);
	uint32_t cr1its     = READ_REG(_pUart->uart->CR1);
	uint32_t cr3its     = READ_REG(_pUart->uart->CR3);
	
	/* 处理接收中断  */
	if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET)
	{
		/* 从串口接收数据寄存器读取数据存放到接收FIFO */
		uint8_t ch;

		ch = READ_REG(_pUart->uart->RDR);
		_pUart->pRxBuf[_pUart->usRxWrite] = ch;
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}

		/* 回调函数,通知应用程序收到新数据,一般是发送1个消息或者设置一个标记 */
		//if (_pUart->usRxWrite == _pUart->usRxRead)
		//if (_pUart->usRxCount == 1)
		{
			if (_pUart->ReciveNew)
			{
				_pUart->ReciveNew(ch); /* 比如，交给MODBUS解码程序处理字节流 */
			}
		}
	}

	/* 处理发送缓冲区空中断 */
	if ( ((isrflags & USART_ISR_TXE_TXFNF) != RESET) && (cr1its & USART_CR1_TXEIE_TXFNFIE) != RESET)//USART_CR1_TXEIE
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
			//USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1,USART_CR1_TXEIE_TXFNFIE  );//USART_CR1_TXEIE

			/* 使能数据发送完毕中断 */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
			SET_BIT(_pUart->uart->CR1, USART_CR1_TCIE);
		}
		else
		{
			_pUart->Sending = 1;
			
			/* 从发送FIFO取1个字节写入串口发送数据寄存器 */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}

	}
	/* 数据bit位全部发送完毕的中断 */
	if (((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TCIE);

			/* 回调函数, 一般用来处理RS485通信，将RS485芯片设置为接收模式，避免抢占总线 */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
			
			_pUart->Sending = 0;
		}
		else
		{
			/* 正常情况下，不会进入此分支 */

			/* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
	
	/* 清除中断标志 */
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_PEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_FEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_NEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_OREF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_IDLEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_TCF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_LBDF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CTSF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CMF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_WUF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_TXFECF);
	
//	  *            @arg UART_CLEAR_PEF: Parity Error Clear Flag
//  *            @arg UART_CLEAR_FEF: Framing Error Clear Flag
//  *            @arg UART_CLEAR_NEF: Noise detected Clear Flag
//  *            @arg UART_CLEAR_OREF: OverRun Error Clear Flag
//  *            @arg UART_CLEAR_IDLEF: IDLE line detected Clear Flag
//  *            @arg UART_CLEAR_TCF: Transmission Complete Clear Flag
//  *            @arg UART_CLEAR_LBDF: LIN Break Detection Clear Flag
//  *            @arg UART_CLEAR_CTSF: CTS Interrupt Clear Flag
//  *            @arg UART_CLEAR_RTOF: Receiver Time Out Clear Flag
//  *            @arg UART_CLEAR_CMF: Character Match Clear Flag
//  *            @arg.UART_CLEAR_WUF:  Wake Up from stop mode Clear Flag
//  *            @arg UART_CLEAR_TXFECF: TXFIFO empty Clear Flag	
}

/*
*********************************************************************************************************
*	函 数 名: USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler等
*	功能说明: USART中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1
void USART1_IRQHandler(void)
{
	UartIRQ(&g_tUart1);
}
#endif

#if UART2_FIFO_EN == 1
void USART2_IRQHandler(void)
{
	UartIRQ(&g_tUart2);
}
#endif

#if UART3_FIFO_EN == 1
void USART3_IRQHandler(void)
{
	UartIRQ(&g_tUart3);
}
#endif

#if UART4_FIFO_EN == 1
void UART4_IRQHandler(void)
{
	UartIRQ(&g_tUart4);
}
#endif

#if UART5_FIFO_EN == 1
void UART5_IRQHandler(void)
{
	UartIRQ(&g_tUart5);
}
#endif

#if UART6_FIFO_EN == 1
void USART6_IRQHandler(void)
{
	UartIRQ(&g_tUart6);
}
#endif

#if UART7_FIFO_EN == 1
void UART7_IRQHandler(void)
{
	UartIRQ(&g_tUart7);
}
#endif

#if UART8_FIFO_EN == 1
void UART8_IRQHandler(void)
{
	UartIRQ(&g_tUart8);
}
#endif

/*
*********************************************************************************************************
*	函 数 名: fputc
*	功能说明: 重定义putc函数，这样可以使用printf函数从串口1打印输出
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
#if 1	/* 将需要printf的字符通过串口中断FIFO发送出去，printf函数会立即返回 */
	comSendChar(COM1, ch);
	
	return ch;
#else	/* 采用阻塞方式发送每个字符,等待数据发送完毕 */
	/* 写一个字节到USART1 */
	USART1->TDR = ch;
	
	/* 等待发送结束 */
	while((USART1->ISR & USART_ISR_TC) == 0)
	{}
	
	return ch;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: fgetc
*	功能说明: 重定义getc函数，这样可以使用getchar函数从串口1输入数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fgetc(FILE *f)
{

#if 1	/* 从串口接收FIFO中取1个数据, 只有取到数据才返回 */
	uint8_t ucData;

	while(comGetChar(COM1, &ucData) == 0);

	return ucData;
#else
	/* 等待接收到数据 */
	while((USART1->ISR & USART_ISR_RXNE) == 0)
	{}

	return (int)USART1->RDR;
#endif
}

/*****************************  (END OF FILE) *********************************/
