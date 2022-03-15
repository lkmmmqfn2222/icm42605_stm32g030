/*
*********************************************************************************************************
*
*	模块名称 : cpu内部falsh操作模块
*	文件名称 : bsp_cpu_flash.c
*	版    本 : V1.0
*	说    明 : 提供读写CPU内部Flash的函数
*	修改记录 :
*
*	Copyright (C), 2013-2021
*
*********************************************************************************************************
*/

#include "bsp_cpu_flash.h"
#include "stm32g0xx_hal.h"


/*
*********************************************************************************************************
*	函 数 名: bsp_GetPage
*	功能说明: 根据地址计算页地址
*	形    参：无
*	返 回 值: 页地址
*********************************************************************************************************
*/
uint32_t bsp_GetPage(uint32_t _ulWrAddr)
{
    return ((_ulWrAddr - FLASH_BASE) / FLASH_PAGE_SIZE);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ReadCpuFlash
*	功能说明: 读取CPU Flash的内容
*	形    参：_ucpDst : 目标缓冲区
*			 _ulFlashAddr : 起始地址
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值: 0=成功，1=失败
*********************************************************************************************************
*/
uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t* _ucpDst, uint32_t _ulSize)
{
    uint32_t i;

    /* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
    if((_ulFlashAddr + (_ulSize / FLASH_PAGE_SIZE)) > (FLASH_BASE + FLASH_SIZE))
    {
        return 1;
    }

    /* 长度为0时不继续操作,否则起始地址为奇地址会出错 */
    if(_ulSize == 0)
    {
        return 1;
    }

    for(i = 0; i < _ulSize; i++)
    {
        *_ucpDst++ = *(__IO uint8_t*)_ulFlashAddr++;
    }

    return 0;
}

//uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint32_t* _ucpDst, uint32_t _ulSize)
//{
//    uint32_t i;

//    /* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
//    if((_ulFlashAddr + (_ulSize / FLASH_PAGE_SIZE)) > (FLASH_BASE + FLASH_SIZE))
//    {
//        return 1;
//    }

//    /* 长度为0时不继续操作,否则起始地址为奇地址会出错 */
//    if(_ulSize == 0)
//    {
//        return 1;
//    }

//    for(i = 0; i < _ulSize; i++)
//    {
//        *_ucpDst++ = *(__IO uint32_t*)_ulFlashAddr;
//        _ulFlashAddr += 4;
//    }

//    return 0;
//}


/*
*********************************************************************************************************
*	函 数 名: bsp_CmpCpuFlash
*	功能说明: 比较Flash指定地址的数据.
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpBuf : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值:
*			FLASH_IS_EQU		0   Flash内容和待写入的数据相等，不需要擦除和写操作
*			FLASH_REQ_WRITE		1	Flash不需要擦除，直接写
*			FLASH_REQ_ERASE		2	Flash需要先擦除,再写
*			FLASH_PARAM_ERR		3	函数参数错误
*********************************************************************************************************
*/
uint8_t bsp_CmpCpuFlash(uint32_t _ulFlashAddr, uint8_t* _ucpBuf, uint32_t _ulSize)
{
    uint32_t i;
    uint8_t ucIsEqu;	/* 相等标志 */
    uint8_t ucByte;

    /* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
    if((_ulFlashAddr + (_ulSize / FLASH_PAGE_SIZE)) > (FLASH_BASE + FLASH_SIZE))
    {
        return FLASH_PARAM_ERR;		/*　函数参数错误　*/
    }

    /* 长度为0时返回正确 */
    if(_ulSize == 0)
    {
        return FLASH_IS_EQU;		/* Flash内容和待写入的数据相等 */
    }

    ucIsEqu = 1;			/* 先假设所有字节和待写入的数据相等，如果遇到任何一个不相等，则设置为 0 */
    for(i = 0; i < _ulSize; i++)
    {
        ucByte = *(__IO uint8_t*)_ulFlashAddr;

        if(ucByte != *_ucpBuf)
        {
            if(ucByte != 0xFF)
            {
                return FLASH_REQ_ERASE;		/* 需要擦除后再写 */
            }
            else
            {
                ucIsEqu = 0;	/* 不相等，需要写 */
            }
        }

        _ulFlashAddr++;
        _ucpBuf++;
    }

    if(ucIsEqu == 1)
    {
        return FLASH_IS_EQU;	/* Flash内容和待写入的数据相等，不需要擦除和写操作 */
    }
    else
    {
        return FLASH_REQ_WRITE;	/* Flash不需要擦除，直接写 */
    }
}

/*
*********************************************************************************************************
*	函 数 名: bsp_WriteCpuFlash
*	功能说明: 写数据到CPU 内部Flash。
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpSrc : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）应当为8的整数倍
*	返 回 值: 0-成功，1-数据长度或地址溢出，2-写Flash出错(估计Flash寿命到)
*********************************************************************************************************
*/
uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint8_t* _ucpSrc, uint32_t _ulSize)
{
    uint32_t i = 0, PageError = 0;
    uint8_t  ucRet;
    uint64_t usTemp;
    FLASH_EraseInitTypeDef EraseInitStruct = {0};

    HAL_StatusTypeDef status = HAL_ERROR;

    /* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
    if((_ulFlashAddr + (_ulSize / FLASH_PAGE_SIZE)) > (FLASH_BASE + FLASH_SIZE))
    {
        return 1;
    }
    /* 长度为0 时不继续操作  */
    if(_ulSize == 0)
    {
        return 0;
    }
    /* 写入的字节数不是8的整数倍则不继续操作  */
    if((_ulSize % 8) != 0)
    {
        return 1;
    }

    ucRet = bsp_CmpCpuFlash(_ulFlashAddr, _ucpSrc, _ulSize);
    if(ucRet == FLASH_IS_EQU)
    {
        return 0;
    }

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();
    
    /* 清楚错误标志位*/
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ECCC | FLASH_FLAG_ECCD);
    __HAL_FLASH_CLEAR_FLAG(FLASH_SR_CLEAR);
    
    /* 需要擦除 */
    if(ucRet == FLASH_REQ_ERASE)
    {
        EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.Page        = bsp_GetPage(_ulFlashAddr);
        EraseInitStruct.NbPages     = bsp_GetPage(FLASH_USER_END_ADDR) - bsp_GetPage(_ulFlashAddr) + 1;
        status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
        if(status != HAL_OK)
        {
            return 2;
        }
    }

    /* 按字节模式编程（为提高效率，可以按字编程，一次写入8字节） */
    for(i = 0; i < _ulSize / 8; i++)
    {
        usTemp  = _ucpSrc[8 * i];
        usTemp |= ((uint64_t)(_ucpSrc[8 * i + 1]) << 8);
        usTemp |= ((uint64_t)(_ucpSrc[8 * i + 2]) << 16);
        usTemp |= ((uint64_t)(_ucpSrc[8 * i + 3]) << 24);
        usTemp |= ((uint64_t)(_ucpSrc[8 * i + 4]) << 32);
        usTemp |= ((uint64_t)(_ucpSrc[8 * i + 5]) << 40);
        usTemp |= ((uint64_t)(_ucpSrc[8 * i + 6]) << 48);
        usTemp |= ((uint64_t)(_ucpSrc[8 * i + 7]) << 56);
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, _ulFlashAddr, usTemp);
        if(status != HAL_OK)
        {
            break;
        }
        _ulFlashAddr += 8;
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    if(status == HAL_OK)
    {
        return 0;
    }
    return 2;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_FlashEnableReadProtection
*	功能说明: flash读保护。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_FlashEnableReadProtection(void)
{
 
  FLASH_OBProgramInitTypeDef OBInit;
  
  __HAL_FLASH_PREFETCH_BUFFER_DISABLE();
  
  HAL_FLASHEx_OBGetConfig(&OBInit);
  if(OBInit.RDPLevel == OB_RDP_LEVEL_0)
  {
    OBInit.OptionType = OPTIONBYTE_RDP;
    OBInit.RDPLevel   = OB_RDP_LEVEL_1;
    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();
    HAL_FLASHEx_OBProgram(&OBInit);
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
  }
  __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
 
}

/*
*********************************************************************************************************
*	函 数 名: bsp_FlashDisableReadProtection
*	功能说明: flash去除读保护。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/ 
void bsp_FlashDisableReadProtection(void)
{
 
  FLASH_OBProgramInitTypeDef OBInit;
  
  __HAL_FLASH_PREFETCH_BUFFER_DISABLE();
  
  HAL_FLASHEx_OBGetConfig(&OBInit);
  if(OBInit.RDPLevel == OB_RDP_LEVEL_1)
  {
    OBInit.OptionType = OPTIONBYTE_RDP;
    OBInit.RDPLevel   = OB_RDP_LEVEL_0;
    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();
    HAL_FLASHEx_OBProgram(&OBInit);
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
  }
  __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
 
}
  

/***************************** (END OF FILE) *********************************/
