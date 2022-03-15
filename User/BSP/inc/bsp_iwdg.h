#ifndef __BSP_IWDG_H__
#define __BSP_IWDG_H__

#include <stdint.h>

typedef enum __iwdgmode
{
    FORBID_IWDG = 0,    //禁止使用独立看门狗
    ENABLE_CPU_IWDG,    //使用内部独立看门狗
    ENABLE_EXTERN_IWDG, //使用外置独立看门狗
    ENABLE_BOTH_IWDG,   //同时使用内部和外置独立看门狗
}IwdgModeTypeDef_T;



uint8_t bsp_IwdgInit(IwdgModeTypeDef_T mode);
void  bsp_IwdgFeed(void);
IwdgModeTypeDef_T  bsp_IwdgGetMode(void);



#endif
