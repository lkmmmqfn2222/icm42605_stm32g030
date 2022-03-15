
#ifndef __BSP_CPU_ADC_H__
#define __BSP_CPU_ADC_H__

#include <stdint.h>

#define ADC_CHANNEL_TIMES  30   //每个通到采集30次
#define ADC_CHANNEL_NUMBER 2    //2个通道 


extern uint16_t adcAverageBuff[ADC_CHANNEL_NUMBER] ;


void bsp_CpuAdcInit(void);
void bsp_CpuAdcStart(void);
void bsp_CpuAdcStop(void);
void bsp_CpuAdcGetAverageValue(void);


#endif


/***************************** (END OF FILE) *********************************/

