/*
*********************************************************************************************************
*
*	模块名称 : 独立看门狗模块
*	文件名称 : bsp_idwg.c
*	版    本 : V1.0
*	说    明 :
*
*	修改记录 :独立看门狗(IWDG)由内部的低速时钟(LSI 32KHZ)驱动；因此，即使主时钟发生故障它也仍然有效
*
*	Copyright (C), 2015-2030
*
*********************************************************************************************************
*/
#include "bsp_iwdg.h"
#include "stm32g0xx_hal.h"


static IwdgModeTypeDef_T sIwdgMode  = FORBID_IWDG; //默认禁用看门狗

//#define ENBALE_EXTERN_FEED_IWDG_FUN

static IWDG_HandleTypeDef hiwdg;

//----------------------------------------------------------------------------//
#if defined(ENBALE_EXTERN_FEED_IWDG_FUN)
#define EXTERN_IWDG_GPIO_RCC()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define EXTERN_IWDG_GPIO_PORT	 GPIOA
#define EXTERN_IWDG_GPIO_PIN	 GPIO_PIN_0
#endif


//----------------------------------------------------------------------------//

void Iwdg_Error_Handler(void);

//============================================================================//

/*******************************************************************************
 * 名    称： bsp_IwdgFeed
 * 功    能： 独立看门狗喂狗
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2021-01-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void  bsp_IwdgFeed(void)
{
    if(sIwdgMode == FORBID_IWDG)
        return;

    /***********************************************
    * 描述：STM内部看门狗喂狗
    */
    if((sIwdgMode == ENABLE_CPU_IWDG) || (sIwdgMode == ENABLE_BOTH_IWDG))
    {
        /* Refresh IWDG: reload counter */
        if(HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
        {
            /* Refresh Error */
            Iwdg_Error_Handler();
        }
    }
    /***********************************************
    * 描述：外部看门狗喂狗
    */
    if((sIwdgMode == ENABLE_EXTERN_IWDG) || (sIwdgMode == ENABLE_BOTH_IWDG))
    {
#if defined(ENBALE_EXTERN_FEED_IWDG_FUN)
        HAL_GPIO_WritePin(EXTERN_IWDG_GPIO_PORT, EXTERN_IWDG_GPIO_PIN, GPIO_PIN_SET);
        HAL_Delay(1);   //延时函数根据外部看门狗的要求设置
        HAL_GPIO_WritePin(EXTERN_IWDG_GPIO_PORT, EXTERN_IWDG_GPIO_PIN, GPIO_PIN_RESET);
#endif
    }
}

/*******************************************************************************
 * 名    称： Bsp_IwdgGetMode
 * 功    能： 获取用了上面狗（内部的？外部的？还是两个都用？）
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
IwdgModeTypeDef_T  bsp_IwdgGetMode(void)
{
    return sIwdgMode;
}

/*******************************************************************************
 * 名    称： bsp_IwdgInit
 * 功    能： 独立看门狗初始化
 * 入口参数： 0:禁止；1：内部看门狗 ；2：外部看门狗 ；3：同时使用内部和外部看门狗;
 * 出口参数： 0：初始化成功 1：初始化失败
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
uint8_t bsp_IwdgInit(IwdgModeTypeDef_T mode)
{
    sIwdgMode = mode;

    if(mode == FORBID_IWDG)     //禁止狗
        return 0;

    if((mode == ENABLE_CPU_IWDG) || (mode == ENABLE_BOTH_IWDG))         //使用内部狗或者内部外部狗一起用
    {
        /*##-1- Check if the system has resumed from IWDG reset ####################*/
        if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != 0x00u)
        {
            /* Clear reset flags in any cases */
            __HAL_RCC_CLEAR_RESET_FLAGS();
        }

        hiwdg.Instance        = IWDG;
        hiwdg.Init.Prescaler  = IWDG_PRESCALER_32;    //32kHZ / 32 = 1K
        hiwdg.Init.Window     = IWDG_WINDOW_DISABLE;
        hiwdg.Init.Reload     = 4000;                 // 4s
        if(HAL_IWDG_Init(&hiwdg) != HAL_OK)
        {
            Iwdg_Error_Handler();
        }

    }
    if((mode == ENABLE_BOTH_IWDG) || (mode == ENABLE_EXTERN_IWDG))         //使用外狗或者内狗外狗一起用
    {
#if defined(ENBALE_EXTERN_FEED_IWDG_FUN)
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        EXTERN_IWDG_GPIO_RCC();
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Pin = EXTERN_IWDG_GPIO_PIN;
        HAL_GPIO_Init(EXTERN_IWDG_GPIO_PORT, &GPIO_InitStruct);
#endif

    }

    return 1;
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Iwdg_Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */

    /* Infinite loop */
    while(1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}


/***************************** (END OF FILE) *********************************/
