/*******************************************************************************
*   Filename:       modbus_slave.c
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2016-06-29
*   Revision:       v1.0
*   Writer:	        Roger-WY
*
*   Description:     Modbus通信模块. 从站模式
*
*   Notes:
*
*
*******************************************************************************/
#include "modbus_slave.h"
#if (MODBUS_CFG_SLAVE_EN == 1)
#include <string.h>
#include "bsp_user_lib.h"
#include "bsp_timer.h"
#include "bsp_uart_fifo.h"
//#include "app_var.h"


//----------------------------------------------------------------------------//
static void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen);
static void MODS_SendAckOk(void);
static void MODS_SendAckErr(uint8_t _ucErrCode);

static void MODS_AnalyzeApp(void);

static void MODS_RxTimeOut(void);

static void MODS_01H(void);
static void MODS_02H(void);
static void MODS_03H(void);
static void MODS_04H(void);
static void MODS_05H(void);
static void MODS_06H(void);
static void MODS_10H(void);

static uint8_t MODS_ReadRegValue(uint16_t reg_addr, uint8_t *reg_value);
static uint8_t MODS_WriteRegValue(uint16_t reg_addr, uint16_t reg_value);

static uint8_t g_mods_timeout = 0;
void MODS_ReciveNew(uint8_t _byte);

MODS_T       g_tModS;
SLAVE_VAR_T  g_tVar;

//============================================================================//

/*******************************************************************************
* 名    称： MODS_Poll
* 功    能： 解析数据包. 在主程序中轮流调用。
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODS_Poll(void)
{
	uint16_t addr;
	uint16_t crc1;
	/* 超过3.5个字符时间后执行MODH_RxTimeOut()函数。全局变量 g_rtu_timeout = 1; 通知主程序开始解码 */
	if (g_mods_timeout == 0) {
		return;								/* 没有超时，继续接收。不要清零 g_tModS.RxCount */
	}

	g_mods_timeout = 0;	 					/* 清标志 */

	if (g_tModS.RxCount < 4) {				/* 接收到的数据小于4个字节就认为错误 */
		goto err_ret;
	}

	/* 计算CRC校验和 */
	crc1 = CRC16_Modbus(g_tModS.RxBuf, g_tModS.RxCount);
	if (crc1 != 0) {
		goto err_ret;
	}

	/* 站地址 (1字节） */
	addr = g_tModS.RxBuf[0];				/* 第1字节 站号 */
	if (addr != MODBUS_SLAVE_ADDR) {		/* 判断主机发送的命令地址是否符合 */
		goto err_ret;
	}

	/* 分析应用层协议 */
	MODS_AnalyzeApp();

err_ret:
#if (MODBUS_SLAVE_DEBUG == 1)			   /* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Rxlen = g_tModS.RxCount;
	memcpy(g_tPrint.RxBuf, g_tModS.RxBuf, g_tModS.RxCount);
#endif

	g_tModS.RxCount = 0;					/* 必须清零计数器，方便下次帧同步 */
}

/*******************************************************************************
* 名    称： MODS_ReciveNew
* 功    能： 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODS_ReciveNew(uint8_t _byte)
{
	/*
		3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
		两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
		所以就是3.5个字符的时间，波特率高，这个时间间隔就小，波特率低，这个时间间隔相应就大

		4800  = 7.297ms
		9600  = 3.646ms
		19200  = 1.771ms
		38400  = 0.885ms
	*/
	uint32_t timeout;

	g_mods_timeout = 0;

	timeout = 35000000 / MODBUS_SLAVE_BAUD;	/* 计算超时时间，单位us 35000000*/

	/* 硬件定时中断，定时精度us  用于Modbus从机 */
	bsp_StartHardTimer(1, timeout, (void *)MODS_RxTimeOut);

	if ( g_tModS.RxCount < S_RX_BUF_SIZE ){
		g_tModS.RxBuf[g_tModS.RxCount++] = _byte;
	}
}

/*******************************************************************************
* 名    称： MODS_RxTimeOut
* 功    能： 超过3.5个字符时间后执行本函数。 设置全局变量 g_mods_timeout = 1; 通知主程序开始解码。
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_RxTimeOut(void)
{
	g_mods_timeout = 1;
}

/*******************************************************************************
* 名    称： MODS_SendWithCRC
* 功    能： 发送一串数据, 自动追加2字节CRC
* 入口参数： _pBuf 数据；
*			  _ucLen 数据长度（不带CRC）
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen)
{
	uint16_t crc;
	uint8_t buf[S_TX_BUF_SIZE];

	memcpy(buf, _pBuf, _ucLen);
	crc = CRC16_Modbus(_pBuf, _ucLen);
	buf[_ucLen++] = crc >> 8;
	buf[_ucLen++] = crc;

	//RS485_SendBuf(buf, _ucLen);
    comSendBuf(MODBUS_SLAVE_UARTX, buf, _ucLen); /* 调用串口发送程序发送数据*/

#if (MODBUS_SLAVE_DEBUG == 1)			   /* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Txlen = _ucLen;
	memcpy(g_tPrint.TxBuf, buf, _ucLen);
#endif
}

/*******************************************************************************
* 名    称： MODS_SendAckErr
* 功    能： 发送错误应答
* 入口参数： _ucErrCode : 错误代码
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_SendAckErr(uint8_t _ucErrCode)
{
	uint8_t txbuf[3];

	txbuf[0] = g_tModS.RxBuf[0];					/* 485地址 */
	txbuf[1] = g_tModS.RxBuf[1] | 0x80;				/* 异常的功能码 */
	txbuf[2] = _ucErrCode;							/* 错误代码(01,02,03,04) */

	MODS_SendWithCRC(txbuf, 3);
}

/*******************************************************************************
* 名    称： MODS_SendAckOk
* 功    能： 发送正确的应答
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_SendAckOk(void)
{
	uint8_t txbuf[6];
	uint8_t i;

	for (i = 0; i < 6; i++) {
		txbuf[i] = g_tModS.RxBuf[i];
	}
	MODS_SendWithCRC(txbuf, 6);
}

/*******************************************************************************
* 名    称： MODS_AnalyzeApp
* 功    能： 分析应用层协议
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_AnalyzeApp(void)
{
	switch (g_tModS.RxBuf[1]) {				/* 第2个字节 功能码 */
		case 0x01:							/* 读取线圈状态（此例程用led代替）*/
			MODS_01H();
			break;

		case 0x02:							/* 读取输入状态（按键状态）*/
			MODS_02H();
			break;

		case 0x03:							/* 读取保持寄存器（此例程存在g_tVar中）*/
			MODS_03H();
			break;

		case 0x04:							/* 读取输入寄存器（ADC的值）*/
			MODS_04H();
			break;

		case 0x05:							/* 强制单线圈（设置led）*/
			MODS_05H();
			break;

		case 0x06:							/* 写单个保存寄存器（此例程改写g_tVar中的参数）*/
			MODS_06H();
			break;

		case 0x10:							/* 写多个保存寄存器（此例程存在g_tVar中的参数）*/
			MODS_10H();
			break;

		default:
			g_tModS.RspCode = RSP_ERR_CMD;
			MODS_SendAckErr(g_tModS.RspCode);	/* 告诉主机命令错误 */
			break;
	}
}

/*******************************************************************************
* 名    称： MODS_01H
* 功    能： 读取线圈状态（对应远程开关D01/D02/D03）
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
/* 说明:这里用LED代替继电器,便于观察现象 */
static void MODS_01H(void)
{
	/*
	 举例：
		主机发送:
			11 从机地址
			01 功能码
			00 寄存器起始地址高字节
			13 寄存器起始地址低字节
			00 寄存器数量高字节
			25 寄存器数量低字节
			0E CRC校验高字节
			84 CRC校验低字节

		从机应答: 	1代表ON，0代表OFF。若返回的线圈数不为8的倍数，则在最后数据字节未尾使用0代替. BIT0对应第1个
			11 从机地址
			01 功能码
			05 返回字节数
			CD 数据1(线圈0013H-线圈001AH)
			6B 数据2(线圈001BH-线圈0022H)
			B2 数据3(线圈0023H-线圈002AH)
			0E 数据4(线圈0032H-线圈002BH)
			1B 数据5(线圈0037H-线圈0033H)
			45 CRC校验高字节
			E6 CRC校验低字节

		例子:
			01 01 10 01 00 03   29 0B	--- 查询D01开始的3个继电器状态
			01 01 10 03 00 01   09 0A   --- 查询D03继电器的状态
	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t m;
	uint8_t status[10];

	g_tModS.RspCode = RSP_OK;

	/* 没有外部继电器，直接应答错误 */
	if (g_tModS.RxCount != 8) {
		g_tModS.RspCode = RSP_ERR_VALUE;				/* 数据值域错误 */
		return;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 			/* 寄存器号 */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);				/* 寄存器个数 */

	m = (num + 7) / 8;

	if ((reg >= REG_D01) && (num > 0) && (reg + num <= REG_DXX + 1)) {
		for (i = 0; i < m; i++) {
			status[i] = 0;
		}
		for (i = 0; i < num; i++) {
//			if (Bsp_IsLedOn(i + 1 + reg - REG_D01))	{	/* 读LED的状态，写入状态寄存器的每一位 */
//				status[i / 8] |= (1 << (i % 8));
//			}
		}
	} else {
		g_tModS.RspCode = RSP_ERR_REG_ADDR;				/* 寄存器地址错误 */
	}

	if (g_tModS.RspCode == RSP_OK) {					/* 正确应答 */
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
		g_tModS.TxBuf[g_tModS.TxCount++] = m;			/* 返回字节数 */

		for (i = 0; i < m; i++) {
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i];	/* 继电器状态 */
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	} else {
		MODS_SendAckErr(g_tModS.RspCode);				/* 告诉主机命令错误 */
	}
}

/*******************************************************************************
* 名    称： MODS_02H
* 功    能： 读取输入状态（对应K01～K03）
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_02H(void)
{
	/*
		主机发送:
			11 从机地址
			02 功能码
			00 寄存器地址高字节
			C4 寄存器地址低字节
			00 寄存器数量高字节
			16 寄存器数量低字节
			BA CRC校验高字节
			A9 CRC校验低字节

		从机应答:  响应各离散输入寄存器状态，分别对应数据区中的每位值，1 代表ON；0 代表OFF。
		           第一个数据字节的LSB(最低字节)为查询的寻址地址，其他输入口按顺序在该字节中由低字节
		           向高字节排列，直到填充满8位。下一个字节中的8个输入位也是从低字节到高字节排列。
		           若返回的输入位数不是8的倍数，则在最后的数据字节中的剩余位至该字节的最高位使用0填充。
			11 从机地址
			02 功能码
			03 返回字节数
			AC 数据1(00C4H-00CBH)
			DB 数据2(00CCH-00D3H)
			35 数据3(00D4H-00D9H)
			20 CRC校验高字节
			18 CRC校验低字节

		例子:
		01 02 20 01 00 08  23CC  ---- 读取T01-08的状态
		01 02 20 04 00 02  B3CA  ---- 读取T04-05的状态
		01 02 20 01 00 12  A207   ---- 读 T01-18
	*/

	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t m;
	uint8_t status[10];

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8) {
		g_tModS.RspCode = RSP_ERR_VALUE;				/* 数据值域错误 */
		return;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 			/* 寄存器号 */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);				/* 寄存器个数 */

	m = (num + 7) / 8;
	if ((reg >= REG_T01) && (num > 0) && (reg + num <= REG_TXX + 1)) {
		for (i = 0; i < m; i++) {
			status[i] = 0;
		}
		for (i = 0; i < num; i++) {
           /* 读取按键状态 */
//			if (Bsp_GetKeyState((KEY_ID_E)(KID_K1 + reg - REG_T01 + i))) {
//				status[i / 8] |= (1 << (i % 8));
//			}
		}
	} else {
		g_tModS.RspCode = RSP_ERR_REG_ADDR;				/* 寄存器地址错误 */
	}

	if (g_tModS.RspCode == RSP_OK){					    /* 正确应答 */
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
		g_tModS.TxBuf[g_tModS.TxCount++] = m;			/* 返回字节数 */

		for (i = 0; i < m; i++) {
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i];/* T01-02状态 */
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	} else {
		MODS_SendAckErr(g_tModS.RspCode);				/* 告诉主机命令错误 */
	}
}

/*******************************************************************************
* 名    称： MODS_ReadRegValue
* 功    能： 读取保持寄存器的值
* 入口参数： reg_addr 寄存器地址
*			  reg_value 存放寄存器结果
* 出口参数： 1表示OK 0表示错误
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static uint8_t MODS_ReadRegValue(uint16_t reg_addr, uint8_t *reg_value)
{
	uint16_t value = 0;

	switch (reg_addr) {									/* 判断寄存器地址 */
		case SLAVE_REG_P01:
			value =	g_tVar.P01;
			break;

		case SLAVE_REG_P02:
			value =	g_tVar.P02;							/* 将寄存器值读出 */
			break;

		default:
			return 0;									/* 参数异常，返回 0 */
	}

//    if (reg_addr < MB_USEINGREG_NUM) {
//        value = g_tCtrl.buf2[reg_addr];  /*应该读取使用Modbus缓存数组*/

//    } else {
//        return 0;									    /* 参数异常，返回 0 */
//    }
    
    /* 装载寄存器的值 */
	reg_value[0] = value >> 8;
	reg_value[1] = value;

	return 1;											/* 读取成功 */
}

/*******************************************************************************
* 名    称： MODS_WriteRegValue
* 功    能： 写保持寄存器的值
* 入口参数： reg_addr 寄存器地址
*			 reg_value 寄存器值
* 出口参数： 1表示OK 0表示错误
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static uint8_t MODS_WriteRegValue(uint16_t reg_addr, uint16_t reg_value)
{
	switch (reg_addr) {							/* 判断寄存器地址 */
		case SLAVE_REG_P01:
			g_tVar.P01 = reg_value;				/* 将值写入保存寄存器 */
			break;

		case SLAVE_REG_P02:
			g_tVar.P02 = reg_value;				/* 将值写入保存寄存器 */
			break;

		default:
			return 0;		/* 参数异常，返回 0 */
	}
//    if (reg_addr < MB_USEINGREG_NUM) {
//        g_tCtrl.buf2[reg_addr] = reg_value;  /* 应该往Modbus缓存数组写入数据 */
//        if(reg_addr == (15)) {
//           ee_WriteBytes((uint8_t *)&g_tCtrl.dat.etEepromPar,0,sizeof(g_tCtrl.dat.etEepromPar));
//        }
//    } else {
//        return 0;									/* 参数异常，返回 0 */
//    }
	return 1;		                                /* 读取成功 */
}

/*******************************************************************************
* 名    称： MODS_03H
* 功    能： 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_03H(void)
{
	/*
		从机地址为11H。保持寄存器的起始地址为006BH，结束地址为006DH。该次查询总共访问3个保持寄存器。

		主机发送:
			11 从机地址
			03 功能码
			00 寄存器地址高字节
			6B 寄存器地址低字节
			00 寄存器数量高字节
			03 寄存器数量低字节
			76 CRC高字节
			87 CRC低字节

		从机应答: 	保持寄存器的长度为2个字节。对于单个保持寄存器而言，寄存器高字节数据先被传输，
					低字节数据后被传输。保持寄存器之间，低地址寄存器先被传输，高地址寄存器后被传输。
			11 从机地址
			03 功能码
			06 字节数
			00 数据1高字节(006BH)
			6B 数据1低字节(006BH)
			00 数据2高字节(006CH)
			13 数据2 低字节(006CH)
			00 数据3高字节(006DH)
			00 数据3低字节(006DH)
			38 CRC高字节
			B9 CRC低字节

		例子:
			01 03 30 06 00 01  6B0B      ---- 读 3006H, 触发电流
			01 03 4000 0010 51C6         ---- 读 4000H 倒数第1条浪涌记录 32字节
			01 03 4001 0010 0006         ---- 读 4001H 倒数第1条浪涌记录 32字节

			01 03 F000 0008 770C         ---- 读 F000H 倒数第1条告警记录 16字节
			01 03 F001 0008 26CC         ---- 读 F001H 倒数第2条告警记录 16字节

			01 03 7000 0020 5ED2         ---- 读 7000H 倒数第1条波形记录第1段 64字节
			01 03 7001 0020 0F12         ---- 读 7001H 倒数第1条波形记录第2段 64字节

			01 03 7040 0020 5F06         ---- 读 7040H 倒数第2条波形记录第1段 64字节
	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint8_t reg_value[32] = {0};

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8) {							    /* 03H命令必须是8个字节 */
		g_tModS.RspCode = RSP_ERR_VALUE;					/* 数据值域错误 */
		goto err_ret;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 				/* 寄存器号 */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);					/* 寄存器个数 */
	if (num > sizeof(reg_value) / 2) {
		g_tModS.RspCode = RSP_ERR_VALUE;					/* 数据值域错误 */
		goto err_ret;
	}

	for (i = 0; i < num; i++) {
		if (MODS_ReadRegValue(reg, &reg_value[2 * i]) == 0) {   /* 读出寄存器值放入reg_value */
			g_tModS.RspCode = RSP_ERR_REG_ADDR;				    /* 寄存器地址错误 */
			break;
		}
		reg++;
	}

err_ret:
	if (g_tModS.RspCode == RSP_OK) {						/* 正确应答 */
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
		g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;			/* 返回字节数 */

		for (i = 0; i < num; i++) {
			g_tModS.TxBuf[g_tModS.TxCount++] = reg_value[2*i];
			g_tModS.TxBuf[g_tModS.TxCount++] = reg_value[2*i+1];
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);	/* 发送正确应答 */
	} else {
		MODS_SendAckErr(g_tModS.RspCode);					/* 发送错误应答 */
	}
}

/*******************************************************************************
* 名    称： MODS_04H
* 功    能： 读取输入寄存器（对应A01/A02） SMA
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_04H(void)
{
	/*
		主机发送:
			11 从机地址
			04 功能码
			00 寄存器起始地址高字节
			08 寄存器起始地址低字节
			00 寄存器个数高字节
			02 寄存器个数低字节
			F2 CRC高字节
			99 CRC低字节

		从机应答:  输入寄存器长度为2个字节。对于单个输入寄存器而言，寄存器高字节数据先被传输，
				低字节数据后被传输。输入寄存器之间，低地址寄存器先被传输，高地址寄存器后被传输。
			11 从机地址
			04 功能码
			04 字节数
			00 数据1高字节(0008H)
			0A 数据1低字节(0008H)
			00 数据2高字节(0009H)
			0B 数据2低字节(0009H)
			8B CRC高字节
			80 CRC低字节

		例子:

			01 04 2201 0006 2BB0  --- 读 2201H A01通道模拟量 开始的6个数据
			01 04 2201 0001 6A72  --- 读 2201H

	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t status[10];

	memset(status, 0, 10);

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8) {
		g_tModS.RspCode = RSP_ERR_VALUE;	/* 数据值域错误 */
		goto err_ret;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* 寄存器号 */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);	/* 寄存器个数 */

	if ((reg >= REG_A01) && (num > 0) && (reg + num <= REG_AXX + 1)) {
		for (i = 0; i < num; i++) {
			switch (reg) {
				/* 测试参数 */
				case REG_A01:
					status[i] = g_tVar.A01;
					break;

				default:
					status[i] = 0;
					break;
			}
			reg++;
		}
	} else {
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* 寄存器地址错误 */
	}

err_ret:
	if (g_tModS.RspCode == RSP_OK) {		/* 正确应答 */
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
		g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;			/* 返回字节数 */

		for (i = 0; i < num; i++) {
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i] >> 8;
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i] & 0xFF;
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	} else {
		MODS_SendAckErr(g_tModS.RspCode);	/* 告诉主机命令错误 */
	}
}

/*******************************************************************************
* 名    称： MODS_05H
* 功    能： 强制单线圈（对应D01/D02/D03）
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_05H(void)
{
	/*
		主机发送: 写单个线圈寄存器。FF00H值请求线圈处于ON状态，0000H值请求线圈处于OFF状态
		。05H指令设置单个线圈的状态，15H指令可以设置多个线圈的状态。
			11 从机地址
			05 功能码
			00 寄存器地址高字节
			AC 寄存器地址低字节
			FF 数据1高字节
			00 数据2低字节
			4E CRC校验高字节
			8B CRC校验低字节

		从机应答:
			11 从机地址
			05 功能码
			00 寄存器地址高字节
			AC 寄存器地址低字节
			FF 寄存器1高字节
			00 寄存器1低字节
			4E CRC校验高字节
			8B CRC校验低字节

		例子:
		01 05 10 01 FF 00   D93A   -- D01打开
		01 05 10 01 00 00   98CA   -- D01关闭

		01 05 10 02 FF 00   293A   -- D02打开
		01 05 10 02 00 00   68CA   -- D02关闭

		01 05 10 03 FF 00   78FA   -- D03打开
		01 05 10 03 00 00   390A   -- D03关闭
	*/
	uint16_t reg;
	uint16_t value;

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8) {
		g_tModS.RspCode = RSP_ERR_VALUE;		/* 数据值域错误 */
		goto err_ret;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* 寄存器号 */
	value = BEBufToUint16(&g_tModS.RxBuf[4]);	/* 数据 */

	if (value != 0 && value != 1) {
		g_tModS.RspCode = RSP_ERR_VALUE;		/* 数据值域错误 */
		goto err_ret;
	}

	if (reg == REG_D01) {
		g_tVar.D01 = value;
	} else if (reg == REG_D02) {
		g_tVar.D02 = value;
	} else if (reg == REG_D03) {
		g_tVar.D03 = value;
	} else if (reg == REG_D04) {
		g_tVar.D04 = value;
	} else {
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* 寄存器地址错误 */
	}
err_ret:
	if (g_tModS.RspCode == RSP_OK){			    /* 正确应答 */
		MODS_SendAckOk();
	} else {
		MODS_SendAckErr(g_tModS.RspCode);		/* 告诉主机命令错误 */
	}
}

/*******************************************************************************
* 名    称： MODS_06H
* 功    能： 写单个寄存器
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_06H(void)
{

	/*
		写保持寄存器。注意06指令只能操作单个保持寄存器，16指令可以设置单个或多个保持寄存器

		主机发送:
			11 从机地址
			06 功能码
			00 寄存器地址高字节
			01 寄存器地址低字节
			00 数据1高字节
			01 数据1低字节
			9A CRC校验高字节
			9B CRC校验低字节

		从机响应:
			11 从机地址
			06 功能码
			00 寄存器地址高字节
			01 寄存器地址低字节
			00 数据1高字节
			01 数据1低字节
			1B CRC校验高字节
			5A	CRC校验低字节

		例子:
			01 06 30 06 00 25  A710    ---- 触发电流设置为 2.5
			01 06 30 06 00 10  6707    ---- 触发电流设置为 1.0


			01 06 30 1B 00 00  F6CD    ---- SMA 滤波系数 = 0 关闭滤波
			01 06 30 1B 00 01  370D    ---- SMA 滤波系数 = 1
			01 06 30 1B 00 02  770C    ---- SMA 滤波系数 = 2
			01 06 30 1B 00 05  36CE    ---- SMA 滤波系数 = 5

			01 06 30 07 00 01  F6CB    ---- 测试模式修改为 T1
			01 06 30 07 00 02  B6CA    ---- 测试模式修改为 T2

			01 06 31 00 00 00  8736    ---- 擦除浪涌记录区
			01 06 31 01 00 00  D6F6    ---- 擦除告警记录区

*/

	uint16_t reg;
	uint16_t value;

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8) {
		g_tModS.RspCode = RSP_ERR_VALUE;		/* 数据值域错误 */
		goto err_ret;
	}

	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* 寄存器号 */
	value = BEBufToUint16(&g_tModS.RxBuf[4]);	/* 寄存器值 */

	if (MODS_WriteRegValue(reg, value) == 1) {	/* 该函数会把写入的值存入寄存器 */
		;
	} else {
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* 寄存器地址错误 */
	}

err_ret:
	if (g_tModS.RspCode == RSP_OK) {			/* 正确应答 */
		MODS_SendAckOk();
	} else {
		MODS_SendAckErr(g_tModS.RspCode);		/* 告诉主机命令错误 */
	}
}

/*******************************************************************************
* 名    称： MODS_10H
* 功    能： 连续写多个寄存器.  进用于改写时钟
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODS_10H(void)
{
	/*
		从机地址为11H。保持寄存器的其实地址为0001H，寄存器的结束地址为0002H。总共访问2个寄存器。
		保持寄存器0001H的内容为000AH，保持寄存器0002H的内容为0102H。

		主机发送:
			11 从机地址
			10 功能码
			00 寄存器起始地址高字节
			01 寄存器起始地址低字节
			00 寄存器数量高字节
			02 寄存器数量低字节
			04 字节数
			00 数据1高字节
			0A 数据1低字节
			01 数据2高字节
			02 数据2低字节
			C6 CRC校验高字节
			F0 CRC校验低字节

		从机响应:
			11 从机地址
			06 功能码
			00 寄存器地址高字节
			01 寄存器地址低字节
			00 数据1高字节
			01 数据1低字节
			1B CRC校验高字节
			5A	CRC校验低字节

		例子:
			01 10 30 00 00 06 0C  07 DE  00 0A  00 01  00 08  00 0C  00 00     389A    ---- 写时钟 2014-10-01 08:12:00
			01 10 30 00 00 06 0C  07 DF  00 01  00 1F  00 17  00 3B  00 39     5549    ---- 写时钟 2015-01-31 23:59:57

	*/
	uint16_t reg_addr;
	uint16_t reg_num;
	uint8_t byte_num;
	uint8_t i;
	uint16_t value;

	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount < 11) {
		g_tModS.RspCode = RSP_ERR_VALUE;			/* 数据值域错误 */
		goto err_ret;
	}

	reg_addr = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* 寄存器号 */
	reg_num = BEBufToUint16(&g_tModS.RxBuf[4]);		/* 寄存器个数 */
	byte_num = g_tModS.RxBuf[6];					/* 后面的数据体字节数 */

	if (byte_num != 2 * reg_num) {
		;
	}

	for (i = 0; i < reg_num; i++) {
		value = BEBufToUint16(&g_tModS.RxBuf[7 + 2 * i]);	/* 寄存器值 */

		if (MODS_WriteRegValue(reg_addr + i, value) == 1) {
			;
		} else {
			g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* 寄存器地址错误 */
			break;
		}
	}

err_ret:
	if (g_tModS.RspCode == RSP_OK) {				/* 正确应答 */
		MODS_SendAckOk();
	} else {
		MODS_SendAckErr(g_tModS.RspCode);			/* 告诉主机命令错误 */
	}
}

/*****************************  (END OF FILE) *********************************/
#endif