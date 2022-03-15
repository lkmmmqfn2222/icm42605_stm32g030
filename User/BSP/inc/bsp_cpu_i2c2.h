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

#ifndef _BSP_CPU_I2C2_H__
#define _BSP_CPU_I2C2_H__


#include "stm32g0xx_hal.h"
/*等待超时时间*/

#define I2Cx_WAIT_TIMEOUT             ((uint32_t) 300) //0x1100
#define I2Cx_FLAG_TIMEOUT             ((uint32_t) 1000) //0x1100
#define I2Cx_LONG_TIMEOUT             ((uint32_t) (300 * I2Cx_FLAG_TIMEOUT)) //was300
/**************************I2C参数定义，I2C1或I2C2********************************/
#define I2C_OWN_ADDRESS                     0x00


#define SENSORS_I2C_RCC_CLK_ENABLE()   	    __HAL_RCC_I2C2_CLK_ENABLE()
#define SENSORS_I2C_FORCE_RESET()    		__HAL_RCC_I2C2_FORCE_RESET()
#define SENSORS_I2C_RELEASE_RESET()  		__HAL_RCC_I2C2_RELEASE_RESET()

/*引脚定义*/ 
#define SENSORS_I2C_SCL_GPIO_PORT           GPIOA
#define SENSORS_I2C_SCL_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define SENSORS_I2C_SCL_GPIO_PIN         	GPIO_PIN_11
#define SENSORS_I2C_SCL_GPIO_AF         	GPIO_AF6_I2C2
 
#define SENSORS_I2C_SDA_GPIO_PORT           GPIOA
#define SENSORS_I2C_SDA_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define SENSORS_I2C_SDA_GPIO_PIN            GPIO_PIN_12
#define SENSORS_I2C_SDA_GPIO_AF             GPIO_AF6_I2C2




																			 
void hal_I2c2Master_Init(void);
																			 
int  Sensors_I2C2_ReadRegister( unsigned char slave_addr,
                                unsigned char reg_addr,
                                unsigned short len, 
                                unsigned char *data_ptr);
int  Sensors_I2C2_WriteRegister(unsigned char slave_addr,
                                unsigned char reg_addr,
                                unsigned short len, 
                                unsigned char *data_ptr);
extern I2C_HandleTypeDef halI2c2;	

#endif
