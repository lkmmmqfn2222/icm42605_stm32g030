/*
*********************************************************************************************************
*
*	ģ������ : I2C��������ģ��
*	�ļ����� : bsp_i2c_gpio.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ���
*
*	Copyright (C), 2012-2020
*
*********************************************************************************************************
*/

#ifndef __BSP_I2C_GPIO_H__
#define __BSP_I2C_GPIO_H__

#include <stdint.h>

#define GOIP_I2C_WR	0		/* д����bit */
#define GPIO_I2C_RD	1		/* ������bit */

// I2C acknowledge
typedef enum{
  ACK  = 0,
  NACK = 1,
}etI2cAck;


void bsp_InitI2C(void);
void bsp_i2c_Start(void);
void bsp_i2c_Stop(void);
void bsp_i2c_SendByte(uint8_t _ucByte);
uint8_t bsp_i2c_ReadByte(void);
uint8_t bsp_i2c_WaitAck(void);
void bsp_i2c_Ack(void);
void bsp_i2c_NAck(void);
uint8_t bsp_i2c_CheckDevice(uint8_t _Address);

#endif
