#ifndef __BSP_CPU_SPI1_H__
#define __BSP_CPU_SPI1_H__

#include "stm32g0xx_hal.h"




void hal_Spi1Master_Init(void);
uint8_t hal_Spi1_ReadWriteByte(uint8_t txdata);


extern SPI_HandleTypeDef hspi1;

#endif