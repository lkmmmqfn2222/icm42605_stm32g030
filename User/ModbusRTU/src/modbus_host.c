/*******************************************************************************
*   Filename:       modbus_host.c
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2016-06-29
*   Revision:       v1.0
*   Writer:	        Roger-WY
*
*   Description:    Modbus通信模块. 主站模式
*
*   Notes:
*
*
*******************************************************************************/

#include "modbus_host.h"

#if (MODBUS_CFG_HOST_EN == 1)
#include <string.h>
#include "bsp_user_lib.h"
#include "bsp.h"
//#include "app_var.h"


#define MODBUS_HOST_TIMEOUT		    100		/* 接收命令超时时间, 单位ms  波特率比较低时，可能需要将该值增大 */
#define MODBUS_HOST_RETRYCOUNT		1		/* 循环发送次数 */

uint8_t g_SlaveAddr = 1;                    /* 定义需要查询的Modbus从机地址 */


/* 保存每个从机的计数器值 */
MODH_T  g_tModH;
uint8_t g_modh_timeout = 0;   /* 读取超时标志位 */

HOST_VAR_T g_tHostVar;

//----------------------------------------------------------------------------//
static void MODH_RxTimeOut(void);
static void MODH_AnalyzeApp(void);

static void MODH_Read_01H(void);
static void MODH_Read_02H(void);
static void MODH_Read_03H(void);
static void MODH_Read_04H(void);
static void MODH_Read_05H(void);
static void MODH_Read_06H(void);
static void MODH_Read_10H(void);


//============================================================================//

/*******************************************************************************
* 名    称： MODH_SendPacket
* 功    能： 发送数据包
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注： 根据宏定义MODBUS_HOST_UARTX 确定使用端口
*******************************************************************************/
void MODH_SendPacket(uint8_t *_buf, uint16_t _len)
{
	//RS485_SendBuf(_buf, _len);
    comSendBuf(MODBUS_HOST_UARTX, _buf, _len);   /* 调用串口底层发送函数发送数据帧 */
}

/*******************************************************************************
* 名    称： MODH_SendAckWithCRC
* 功    能： 发送应答,自动加CRC.
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODH_SendAckWithCRC(void)
{
	uint16_t crc;

	crc = CRC16_Modbus(g_tModH.TxBuf, g_tModH.TxCount);
	g_tModH.TxBuf[g_tModH.TxCount++] = crc >> 8;
	g_tModH.TxBuf[g_tModH.TxCount++] = crc;
	MODH_SendPacket(g_tModH.TxBuf, g_tModH.TxCount);

#if ( MODBUS_HOST_DEBUG ==1 )	/* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Txlen = g_tModH.TxCount;
	memcpy(g_tPrint.TxBuf, g_tModH.TxBuf, g_tModH.TxCount);
#endif
}

/*******************************************************************************
* 名    称： MODH_AnalyzeApp
* 功    能： 分析应用层协议。处理应答。
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODH_AnalyzeApp(void)
{
	switch (g_tModH.RxBuf[1])			/* 第2个字节 功能码 */
	{
		case 0x01:	/* 读取线圈状态 */
			MODH_Read_01H();
			break;

		case 0x02:	/* 读取输入状态 */
			MODH_Read_02H();
			break;

		case 0x03:	/* 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值 */
			MODH_Read_03H();
			break;

		case 0x04:	/* 读取输入寄存器 */
			MODH_Read_04H();
			break;

		case 0x05:	/* 强制单线圈 */
			MODH_Read_05H();
			break;

		case 0x06:	/* 写单个寄存器 */
			MODH_Read_06H();
			break;

		case 0x10:	/* 写多个寄存器 */
			MODH_Read_10H();
			break;

		default:
			break;
	}
}

/*******************************************************************************
* 名    称： MODH_Send01H
* 功    能： 发送01H指令，查询1个或多个保持寄存器
* 入口参数：  _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Send01H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x01;		/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* 寄存器个数 低字节 */

	MODH_SendAckWithCRC();		/* 发送数据，自动加CRC */
	g_tModH.fAck01H = 0;		/* 清接收标志 */
	g_tModH.RegNum = _num;		/* 寄存器个数 */
	g_tModH.Reg01H = _reg;		/* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

/*******************************************************************************
* 名    称： MODH_Send02H
* 功    能： 发送02H指令，读离散输入寄存器
* 入口参数： _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Send02H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x02;		/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* 寄存器个数 低字节 */

	MODH_SendAckWithCRC();		/* 发送数据，自动加CRC */
	g_tModH.fAck02H = 0;		/* 清接收标志 */
	g_tModH.RegNum = _num;		/* 寄存器个数 */
	g_tModH.Reg02H = _reg;		/* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

/*******************************************************************************
* 名    称： MODH_Send03H
* 功    能： 发送03H指令，查询1个或多个保持寄存器
* 入口参数：  _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Send03H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x03;		/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* 寄存器个数 低字节 */

	MODH_SendAckWithCRC();		/* 发送数据，自动加CRC */
	g_tModH.fAck03H = 0;		/* 清接收标志 */
	g_tModH.RegNum = _num;		/* 寄存器个数 */
	g_tModH.Reg03H = _reg;		/* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

/*******************************************************************************
* 名    称： MODH_Send04H
* 功    能： 发送04H指令，读输入寄存器
* 入口参数： _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Send04H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x04;		/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* 寄存器个数 低字节 */

	MODH_SendAckWithCRC();		/* 发送数据，自动加CRC */
	g_tModH.fAck04H = 0;		/* 清接收标志 */
	g_tModH.RegNum = _num;		/* 寄存器个数 */
	g_tModH.Reg04H = _reg;		/* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

/*******************************************************************************
* 名    称： MODH_Send05H
* 功    能： 发送05H指令，写强置单线圈
* 入口参数： _addr : 从站地址
*			 _reg : 寄存器编号
*			 _value : 寄存器值,2字节
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Send05H(uint8_t _addr, uint16_t _reg, uint16_t _value)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;			/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x05;			/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;		/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;			/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value >> 8;		/* 寄存器值 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value;			/* 寄存器值 低字节 */

	MODH_SendAckWithCRC();		/* 发送数据，自动加CRC */

	g_tModH.fAck05H = 0;		/* 如果收到从机的应答，则这个标志会设为1 */
}

/*******************************************************************************
* 名    称： MODH_Send06H
* 功    能： 发送06H指令，写1个保持寄存器
* 入口参数： _addr : 从站地址
*			 _reg : 寄存器编号
*			 _value : 寄存器值,2字节
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Send06H(uint8_t _addr, uint16_t _reg, uint16_t _value)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;			/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x06;			/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;		/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;			/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value >> 8;		/* 寄存器值 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value;			/* 寄存器值 低字节 */

	MODH_SendAckWithCRC();		/* 发送数据，自动加CRC */

	g_tModH.fAck06H = 0;		/* 如果收到从机的应答，则这个标志会设为1 */
}

/*******************************************************************************
* 名    称： MODH_Send10H
* 功    能： 发送10H指令，连续写多个保持寄存器. 最多一次支持23个寄存器。
* 入口参数： _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数n (每个寄存器2个字节) 值域
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Send10H(uint8_t _addr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	uint16_t i;

	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x10;		/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8;	/* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;		/* 寄存器个数 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 2 * _num;	/* 数据字节数 */

	for (i = 0; i < 2 * _num; i++) {
		if (g_tModH.TxCount > H_RX_BUF_SIZE - 3) {
			return;		/* 数据超过缓冲区超度，直接丢弃不发送 */
		}
		g_tModH.TxBuf[g_tModH.TxCount++] = _buf[i];		/* 后面的数据长度 */
	}

	MODH_SendAckWithCRC();	/* 发送数据，自动加CRC */
}

/*******************************************************************************
* 名    称： MODH_ReciveNew
* 功    能： 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注： 需要在使用主机的串口接受中断中调用此函数
*******************************************************************************/
void MODH_ReciveNew(uint8_t _data)
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

	g_modh_timeout = 0;

	timeout = 35000000 / MODBUS_HOST_BAUD;		/* 计算超时时间，单位us 35000000*/

	/* 硬件定时中断，定时精度us 用于MODBUS主机*/
	bsp_StartHardTimer(2, timeout, (void *)MODH_RxTimeOut);

	if (g_tModH.RxCount < H_RX_BUF_SIZE) {
		g_tModH.RxBuf[g_tModH.RxCount++] = _data;
	}
}

/*******************************************************************************
* 名    称： MODH_RxTimeOut
* 功    能： 超过3.5个字符时间后执行本函数。 设置全局变量 g_mods_timeout = 1; 通知主程序开始解码。
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODH_RxTimeOut(void)
{
	g_modh_timeout = 1;
}

/*******************************************************************************
* 名    称： MODH_Poll
* 功    能： 接收控制器指令. 1ms 响应时间。
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Poll(void)
{
	uint16_t crc1;

	if (g_modh_timeout == 0)	/* 超过3.5个字符时间后执行MODH_RxTimeOut()函数。全局变量 g_rtu_timeout = 1 */
	{
		/* 没有超时，继续接收。不要清零 g_tModH.RxCount */
		return ;
	}

	/* 收到命令
		05 06 00 88 04 57 3B70 (8 字节)
			05    :  数码管屏的号站，
			06    :  指令
			00 88 :  数码管屏的显示寄存器
			04 57 :  数据,,,转换成 10 进制是 1111.高位在前,
			3B70  :  二个字节 CRC 码	从05到 57的校验
	*/
	g_modh_timeout = 0;

	if (g_tModH.RxCount < 4) {
		goto err_ret;
	}

	/* 计算CRC校验和 */
	crc1 = CRC16_Modbus(g_tModH.RxBuf, g_tModH.RxCount);
	if (crc1 != 0) {
		goto err_ret;
	}

	/* 分析应用层协议 */
	MODH_AnalyzeApp();

err_ret:
#if ( MODBUS_HOST_DEBUG ==1 )	/* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Rxlen = g_tModH.RxCount;
	memcpy(g_tPrint.RxBuf, g_tModH.RxBuf, g_tModH.RxCount);
#endif

	g_tModH.RxCount = 0;	/* 必须清零计数器，方便下次帧同步 */
}

/*******************************************************************************
* 名    称： MODH_Read_01H
* 功    能： 分析01H指令的应答数据
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODH_Read_01H(void)
{
	uint8_t bytes;
	uint8_t *p;

	if (g_tModH.RxCount > 0) {
		bytes = g_tModH.RxBuf[2];	/* 数据长度 字节数 */
		switch (g_tModH.Reg01H) {
			case REG_D01:
				if (bytes == 8) {
					p = &g_tModH.RxBuf[3];

					g_tHostVar.D01 = BEBufToUint16(p); p += 2;	/* 寄存器 */
					g_tHostVar.D02 = BEBufToUint16(p); p += 2;	/* 寄存器 */
					g_tHostVar.D03 = BEBufToUint16(p); p += 2;	/* 寄存器 */
					g_tHostVar.D04 = BEBufToUint16(p); p += 2;	/* 寄存器 */

					g_tModH.fAck01H = 1;
				}
				break;
		}
	}
}

/*******************************************************************************
* 名    称： MODH_Read_02H
* 功    能： 分析02H指令的应答数据
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODH_Read_02H(void)
{
	uint8_t bytes;
	uint8_t *p;

	if (g_tModH.RxCount > 0) {
		bytes = g_tModH.RxBuf[2];	/* 数据长度 字节数 */
		switch (g_tModH.Reg02H) {
			case REG_T01:
				if (bytes == 6) {
					p = &g_tModH.RxBuf[3];

					g_tHostVar.T01 = BEBufToUint16(p); p += 2;	/* 寄存器 */
					g_tHostVar.T02 = BEBufToUint16(p); p += 2;	/* 寄存器 */
					g_tHostVar.T03 = BEBufToUint16(p); p += 2;	/* 寄存器 */

					g_tModH.fAck02H = 1;
				}
				break;
		}
	}
}

/*******************************************************************************
* 名    称： MODH_Read_04H
* 功    能： 分析04H指令的应答数据
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODH_Read_04H(void)
{
	uint8_t bytes;
	uint8_t *p;

	if (g_tModH.RxCount > 0) {
		bytes = g_tModH.RxBuf[2];	/* 数据长度 字节数 */
		switch (g_tModH.Reg04H) {
			case REG_T01:
				if (bytes == 2) {
					p = &g_tModH.RxBuf[3];

					g_tHostVar.A01 = BEBufToUint16(p); p += 2;	/* 寄存器 */

					g_tModH.fAck04H = 1;
				}
				break;
		}
	}
}

/*******************************************************************************
* 名    称： MODH_Read_05H
* 功    能： 分析05H指令的应答数据
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODH_Read_05H(void)
{
	if (g_tModH.RxCount > 0) {
		if (g_tModH.RxBuf[0] == g_SlaveAddr) {
			g_tModH.fAck05H = 1;		/* 接收到应答 */
		}
	};
}

/*******************************************************************************
* 名    称： MODH_Read_06H
* 功    能： 分析06H指令的应答数据
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void MODH_Read_06H(void)
{
	if (g_tModH.RxCount > 0) {
		if (g_tModH.RxBuf[0] == g_SlaveAddr) {
			g_tModH.fAck06H = 1;		/* 接收到应答 */
		}
	}
}

/*******************************************************************************
* 名    称： MODH_Read_03H
* 功    能： 分析03H指令的应答数据
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Read_03H(void)
{
	uint8_t bytes;
	uint8_t *p;

	if (g_tModH.RxCount > 0) {
		bytes = g_tModH.RxBuf[2];	/* 数据长度 字节数 */
		switch (g_tModH.Reg03H) {
			case REG_P01:
				if (bytes == 4) {
					p = &g_tModH.RxBuf[3];

					g_tHostVar.P01 = BEBufToUint16(p); p += 2;	/* 寄存器 */
					g_tHostVar.P02 = BEBufToUint16(p); p += 2;	/* 寄存器 */

					g_tModH.fAck03H = 1;
				}
				break;
		}
	}
}

/*******************************************************************************
* 名    称： MODH_Read_10H
* 功    能： 分析10H指令的应答数据
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MODH_Read_10H(void)
{
	/*
		10H指令的应答:
			从机地址                11
			功能码                  10
			寄存器起始地址高字节	00
			寄存器起始地址低字节    01
			寄存器数量高字节        00
			寄存器数量低字节        02
			CRC校验高字节           12
			CRC校验低字节           98
	*/
	if (g_tModH.RxCount > 0) {
		if (g_tModH.RxBuf[0] == g_SlaveAddr) {
			g_tModH.fAck10H = 1;		/* 接收到应答 */
		}
	}
}

/*******************************************************************************
* 名    称： MODH_WriteParam_01H
* 功    能： 写单线圈. 通过发送01H指令实现，发送之后，等待从机应答。循环MODBUS_HOST_RETRYCOUNT次写命令
* 入口参数： _reg：寄存器地址  _num：写入的数量
* 出口参数： 1 表示成功。0 表示失败（通信超时或被拒绝）
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t MODH_ReadParam_01H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < MODBUS_HOST_RETRYCOUNT; i++) {
		MODH_Send01H (g_SlaveAddr, _reg, _num);	 /* 发送命令 */
		time1 = bsp_GetRunTime();	             /* 记录命令发送的时刻 */

		while (1) {				/* 等待应答,超时或接收到应答则break  */
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > MODBUS_HOST_TIMEOUT) {
				break;		/* 通信超时了 */
			}

			if (g_tModH.fAck01H > 0) {
				break;		/* 接收到应答 */
			}
		}

		if (g_tModH.fAck01H > 0) {
			break;			/* 循环NUM次，如果接收到命令则break循环 */
		}
	}

	if (g_tModH.fAck01H == 0) {
		return 0;
	} else  {
		return 1;	/* 01H 读成功 */
	}
}

/*******************************************************************************
* 名    称： MODH_WriteParam_02H
* 功    能： 单个参数. 通过发送02H指令实现，发送之后，等待从机应答。循环MODBUS_HOST_RETRYCOUNT次写命令
* 入口参数： _reg：寄存器地址  _num：写入的数量
* 出口参数： 1 表示成功。0 表示失败（通信超时或被拒绝）
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t MODH_ReadParam_02H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < MODBUS_HOST_RETRYCOUNT; i++) {
		MODH_Send02H (g_SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */

		while (1) {
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > MODBUS_HOST_TIMEOUT) {
				break;		/* 通信超时了 */
			}

			if (g_tModH.fAck02H > 0) {
				break;
			}
		}

		if (g_tModH.fAck02H > 0) {
			break;
		}
	}

	if (g_tModH.fAck02H == 0) {
		return 0;
	} else  {
		return 1;	/* 02H 读成功 */
	}
}

/*******************************************************************************
* 名    称： MODH_WriteParam_03H
* 功    能： 读寄存器. 通过发送03H指令实现，发送之后，等待从机应答。循环MODBUS_HOST_RETRYCOUNT次写命令
* 入口参数： _reg：寄存器地址  _num：读取的数量
* 出口参数： 1 表示成功。0 表示失败（通信超时或被拒绝）
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t MODH_ReadParam_03H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < MODBUS_HOST_RETRYCOUNT; i++) {
		MODH_Send03H (g_SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */

		while (1) {
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > MODBUS_HOST_TIMEOUT) {
				break;		/* 通信超时了 */
			}

			if (g_tModH.fAck03H > 0) {
				break;
			}
		}

		if (g_tModH.fAck03H > 0) {
			break;
		}
	}

	if (g_tModH.fAck03H == 0) {
		return 0;	/* 通信超时了 */
	} else  {
		return 1;	/* 写入03H参数成功 */
	}
}

/*******************************************************************************
* 名    称： MODH_WriteParam_04H
* 功    能： 单个参数. 通过发送04H指令实现，发送之后，等待从机应答。循环MODBUS_HOST_RETRYCOUNT次写命令
* 入口参数： _reg：寄存器地址  _num：写入的数量
* 出口参数： 1 表示成功。0 表示失败（通信超时或被拒绝）
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t MODH_ReadParam_04H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < MODBUS_HOST_RETRYCOUNT; i++) {
		MODH_Send04H (g_SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */

		while (1) {
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > MODBUS_HOST_TIMEOUT)	 {
				break;		/* 通信超时了 */
			}

			if (g_tModH.fAck04H > 0) {
				break;
			}
		}

		if (g_tModH.fAck04H > 0) {
			break;
		}
	}

	if (g_tModH.fAck04H == 0) {
		return 0;	/* 通信超时了 */
	} else  {
		return 1;	/* 04H 读成功 */
	}
}
/*******************************************************************************
* 名    称： MODH_WriteParam_05H
* 功    能： 单个参数. 通过发送05H指令实现，发送之后，等待从机应答。循环MODBUS_HOST_RETRYCOUNT次写命令
* 入口参数： _reg：寄存器地址  _num：写入的数量  *_buf：写入的寄存器的值
* 出口参数： 1 表示成功。0 表示失败（通信超时或被拒绝）
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t MODH_WriteParam_05H(uint16_t _reg, uint16_t _value)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < MODBUS_HOST_RETRYCOUNT; i++) {
		MODH_Send05H (g_SlaveAddr, _reg, _value);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */

		while (1) {
			bsp_Idle();

			/* 超时大于 TIMEOUT，则认为异常 */
			if (bsp_CheckRunTime(time1) > MODBUS_HOST_TIMEOUT) {
				break;	/* 通信超时了 */
			}

			if (g_tModH.fAck05H > 0) {
				break;
			}
		}

		if (g_tModH.fAck05H > 0) {
			break;
		}
	}

	if (g_tModH.fAck05H == 0) {
		return 0;	/* 通信超时了 */
	} else {
		return 1;	/* 05H 写成功 */
	}
}

/*******************************************************************************
* 名    称： MODH_WriteParam_06H
* 功    能： 单个参数. 通过发送06H指令实现，发送之后，等待从机应答。循环MODBUS_HOST_RETRYCOUNT次写命令
* 入口参数： _reg：寄存器地址  _num：写入的数量  *_buf：写入的寄存器的值
* 出口参数： 1 表示成功。0 表示失败（通信超时或被拒绝）
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t MODH_WriteParam_06H(uint16_t _reg, uint16_t _value)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < MODBUS_HOST_RETRYCOUNT; i++) {
		MODH_Send06H (g_SlaveAddr, _reg, _value);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */

		while (1) {
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > MODBUS_HOST_TIMEOUT)	 {
				break;
			}

			if (g_tModH.fAck06H > 0) {
				break;
			}
		}

		if (g_tModH.fAck06H > 0) {
			break;
		}
	}

	if (g_tModH.fAck06H == 0) {
		return 0;	/* 通信超时了 */
	} else {
		return 1;	/* 写入06H参数成功 */
	}
}

/*******************************************************************************
* 名    称： MODH_WriteParam_10H
* 功    能： 单个参数. 通过发送10H指令实现，发送之后，等待从机应答。循环MODBUS_HOST_RETRYCOUNT次写命令
* 入口参数： _reg：寄存器地址  _num：写入的数量  *_buf：写入的寄存器的值
* 出口参数： 1 表示成功。0 表示失败（通信超时或被拒绝）
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t MODH_WriteParam_10H(uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < MODBUS_HOST_RETRYCOUNT; i++) {
		MODH_Send10H(g_SlaveAddr, _reg, _num, _buf);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */

		while (1) {
			bsp_Idle();

			if (bsp_CheckRunTime(time1) > MODBUS_HOST_TIMEOUT) {
				break;
			}

			if (g_tModH.fAck10H > 0) {
				break;
			}
		}

		if (g_tModH.fAck10H > 0) {
			break;
		}
	}

	if (g_tModH.fAck10H == 0) {
		return 0;	/* 通信超时了 */
	} else {
		return 1;	/* 写入10H参数成功 */
	}
}

#endif
/*****************************  (END OF FILE) *********************************/

