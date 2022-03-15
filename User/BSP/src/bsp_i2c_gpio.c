/*******************************************************************************
*   Filename:       bsp_i2c_gpio.c
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2016-06-29
*   Revision:       v1.0
*   Writer:	        Roger-WY
*
*   Description:    I2C总线驱动模块
*                   用gpio模拟i2c总线, 适用于STM32F1系列CPU。
*                   该模块不包括应用层命令帧，仅包括I2C总线基本操作函数。
*
*   Notes:          在访问I2C设备前，请先调用 i2c_CheckDevice() 检测I2C设备是否正常，该函数会配置GPIO
*
*
*******************************************************************************/

#include "bsp_i2c_gpio.h"
#include "stm32g0xx_hal.h"


/* 定义I2C总线连接的GPIO端口, 用户只需要修改下面6行代码即可任意改变SCL和SDA的引脚 */
#define GPIO_I2C_SCL_RCC()     	__HAL_RCC_GPIOA_CLK_ENABLE()
#define GPIO_I2C_SCL_PIN		GPIO_PIN_11			/* 连接到SCL时钟线的GPIO */
#define GPIO_I2C_SCL_PORT    	GPIOA

#define GPIO_I2C_SDA_RCC()     	__HAL_RCC_GPIOA_CLK_ENABLE()
#define GPIO_I2C_SDA_PIN		GPIO_PIN_12			/* 连接到SDA数据线的GPIO */
#define GPIO_I2C_SDA_PORT    	GPIOA


/* 定义读写SCL和SDA的宏 */
#define I2C_SCL_1()     HAL_GPIO_WritePin(GPIO_I2C_SCL_PORT,GPIO_I2C_SCL_PIN,GPIO_PIN_SET);		/* SCL = 1 */
#define I2C_SCL_0()     HAL_GPIO_WritePin(GPIO_I2C_SCL_PORT,GPIO_I2C_SCL_PIN,GPIO_PIN_RESET);	/* SCL = 0 */

#define I2C_SDA_1()     HAL_GPIO_WritePin(GPIO_I2C_SDA_PORT,GPIO_I2C_SDA_PIN,GPIO_PIN_SET);			/* SDA = 1 */
#define I2C_SDA_0()     HAL_GPIO_WritePin(GPIO_I2C_SDA_PORT,GPIO_I2C_SDA_PIN,GPIO_PIN_RESET);			/* SDA = 0 */

#define I2C_SDA_READ()  ((HAL_GPIO_ReadPin(GPIO_I2C_SDA_PORT, GPIO_I2C_SDA_PIN)) != GPIO_PIN_RESET)	/* 读SDA口线状态 */
#define I2C_SCL_READ()  ((HAL_GPIO_ReadPin(GPIO_I2C_SCL_PORT, GPIO_I2C_SCL_PIN)) != GPIO_PIN_RESET)	/* 读SCL口线状态 */

//============================================================================//


/*******************************************************************************
* 名    称： bsp_InitI2C
* 功    能： 配置I2C总线的GPIO，采用模拟IO的方式实现
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void bsp_InitI2C(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_I2C_SCL_RCC();
	GPIO_I2C_SDA_RCC();	/* 打开GPIO时钟 */

    /*Configure GPIO pins */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = GPIO_I2C_SCL_PIN;
    HAL_GPIO_Init(GPIO_I2C_SCL_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_I2C_SDA_PIN;
    HAL_GPIO_Init(GPIO_I2C_SDA_PORT, &GPIO_InitStruct);
	/* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
	bsp_i2c_Stop();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_Delay
*	功能说明: I2C总线位延迟，最快400KHz
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void i2c_Delay(void)
{
	__IO uint8_t i;

	/*　
		CPU主频168MHz时，在内部Flash运行, MDK工程不优化。用台式示波器观测波形。
		循环次数为5时，SCL频率 = 1.78MHz (读耗时: 92ms, 读写正常，但是用示波器探头碰上就读写失败。时序接近临界)
		循环次数为10时，SCL频率 = 1.1MHz (读耗时: 138ms, 读速度: 118724B/s)
		循环次数为30时，SCL频率 = 440KHz， SCL高电平时间1.0us，SCL低电平时间1.2us

		上拉电阻选择2.2K欧时，SCL上升沿时间约0.5us，如果选4.7K欧，则上升沿约1us

		实际应用选择400KHz左右的速率即可
	*/
	for (i = 0; i < 30; i++);
}

/*******************************************************************************
* 名    称： bsp_i2c_Start
* 功    能： CPU发起I2C总线启动信号
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：  开始时序
 *            SCL ˉˉˉˉˉˉ\____
 *            SDA ˉˉˉˉ\______
 *                  |   |
 *                  START
*******************************************************************************/
void bsp_i2c_Start(void)
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	I2C_SDA_1();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_0();
	i2c_Delay();

	I2C_SCL_0();
	i2c_Delay();
}

/*******************************************************************************
* 名    称： bsp_i2c_Stop
* 功    能： CPU发起I2C总线停止信号
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注： 停止时序
 *            SCL _____/ˉˉˉˉˉˉˉ
 *            SDA _________/ˉˉˉˉˉ
 *                       |   |
 *                       STOP
*******************************************************************************/
void bsp_i2c_Stop(void)
{
	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
	I2C_SDA_0();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_1();
	i2c_Delay();
}

/*******************************************************************************
* 名    称： bsp_i2c_SendByte
* 功    能： CPU向I2C总线设备发送8bit数据
* 入口参数：  _ucByte ： 等待发送的字节
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void bsp_i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++) {
		if (_ucByte & 0x80) {
			I2C_SDA_1();
		} else {
			I2C_SDA_0();
		}
		i2c_Delay();
		I2C_SCL_1();
		i2c_Delay();
		I2C_SCL_0();
		if (i == 7) {
			 I2C_SDA_1(); // 释放总线
		}
		_ucByte <<= 1;	/* 左移一个bit */
		i2c_Delay();
	}
}

/*******************************************************************************
* 名    称： bsp_i2c_ReadByte
* 功    能： CPU从I2C总线设备读取8bit数据
* 入口参数： 无
* 出口参数： 读到的数据
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t bsp_i2c_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	/* 读到第1个bit为数据的bit7 */
	value = 0;
	for (i = 0; i < 8; i++) {
		value <<= 1;
		I2C_SCL_1();
		i2c_Delay();
		if (I2C_SDA_READ()) {
			value++;
		}
		I2C_SCL_0();
		i2c_Delay();
	}
	return value;
}

/*******************************************************************************
* 名    称： bsp_i2c_WaitAck
* 功    能： CPU产生一个时钟，并读取器件的ACK应答信号
* 入口参数： 无
* 出口参数： 返回0表示正确应答，1表示无器件响应
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t bsp_i2c_WaitAck(void)
{
	uint8_t re;

	I2C_SDA_1();	/* CPU释放SDA总线 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	i2c_Delay();
	if (I2C_SDA_READ()) {	/* CPU读取SDA口线状态 */
		re = 1;
	} else {
		re = 0;
	}
	I2C_SCL_0();
	i2c_Delay();
	return re;
}

/*******************************************************************************
* 名    称： bsp_i2c_Ack
* 功    能： CPU产生一个ACK信号
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void bsp_i2c_Ack(void)
{
	I2C_SDA_0();	/* CPU驱动SDA = 0 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU产生1个时钟 */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
	I2C_SDA_1();	/* CPU释放SDA总线 */
}

/*******************************************************************************
* 名    称： bsp_i2c_NAck
* 功    能： CPU产生1个NACK信号。
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void bsp_i2c_NAck(void)
{
	I2C_SDA_1();	/* CPU驱动SDA = 1 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU产生1个时钟 */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
}

/*******************************************************************************
* 名    称： bsp_i2c_CheckDevice
* 功    能： 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
* 入口参数： _Address：设备的I2C总线地址
* 出口参数： 返回值 0 表示正确， 返回1表示未探测到
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t bsp_i2c_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;

	if (I2C_SDA_READ() && I2C_SCL_READ()) {
		bsp_i2c_Start();		/* 发送启动信号 */

		/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
		bsp_i2c_SendByte(_Address | 0x00);
		ucAck = bsp_i2c_WaitAck();	/* 检测设备的ACK应答 */

		bsp_i2c_Stop();			/* 发送停止信号 */

		return ucAck;
	}
	return 1;	/* I2C总线异常 */
}


/*****************************  (END OF FILE) *********************************/
