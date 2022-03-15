/*******************************************************************************
*   Filename:       bsp_cpu_i2c.c
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2020-06-29
*   Revision:       v1.0
*   Writer:	        Roger-WY
*
*   Description:    使用hal库操作I2C总线驱动模块
*                   
*
*   Notes:          
*
*
*******************************************************************************/

#include "bsp_cpu_i2c2.h"
#include "bsp.h"




//============================================================================//
I2C_HandleTypeDef halI2c2;			
static void I2Cx_Error(void);

extern void Error_Handler(void);

 /*******************************************************************************
* 名    称： hal_I2c2Master_Init
* 功    能： 配置I2C总线的GPIO
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2016-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void hal_I2c2Master_Init(void) 
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能I2Cx时钟 */
	SENSORS_I2C_RCC_CLK_ENABLE();

	/* 使能I2C GPIO 时钟 */
	SENSORS_I2C_SCL_GPIO_CLK_ENABLE();
	SENSORS_I2C_SDA_GPIO_CLK_ENABLE();

	/* 配置I2Cx引脚: SCL ----------------------------------------*/
	GPIO_InitStructure.Pin   = SENSORS_I2C_SCL_GPIO_PIN; 
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = SENSORS_I2C_SCL_GPIO_AF; 
	HAL_GPIO_Init(SENSORS_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

	/* 配置I2Cx引脚: SDA ----------------------------------------*/
	GPIO_InitStructure.Pin = SENSORS_I2C_SDA_GPIO_PIN; 
	GPIO_InitStructure.Alternate = SENSORS_I2C_SDA_GPIO_AF;  
	HAL_GPIO_Init(SENSORS_I2C_SDA_GPIO_PORT, &GPIO_InitStructure); 
	
  halI2c2.Instance = I2C2;
  /* 
  100KHZ: 0x10707DBC
  400KHZ: 0x00602173 */
  halI2c2.Init.Timing = 0x10707DBC; //设置频率为100Khz
  halI2c2.Init.OwnAddress1 = 0;
  halI2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  halI2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  halI2c2.Init.OwnAddress2 = 0;
  halI2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  halI2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  halI2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&halI2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&halI2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    I2Cx_Error();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&halI2c2, 0) != HAL_OK)
  {
    I2Cx_Error();
  }

}
/**
  * @brief  Manages error callback by re-initializing I2C.
  * @param  Addr: I2C Address
  * @retval None
  */
static void I2Cx_Error(void)
{
	/* 恢复I2C寄存器为默认值 */
	HAL_I2C_DeInit(&halI2c2); 
    
}
/**
  * @brief  写寄存器，这是提供给上层的接口
    * @param  slave_addr: 从机地址
    * @param 	reg_addr:寄存器地址
    * @param len：写入的长度
    *	@param data_ptr:指向要写入的数据
  * @retval 正常为0，不正常为非0
  */
int Sensors_I2C2_WriteRegister( unsigned char slave_addr,
								unsigned char reg_addr,
								unsigned short len, 
								unsigned char *data_ptr)
{
	HAL_StatusTypeDef status = HAL_OK;
	status = HAL_I2C_Mem_Write(&halI2c2, slave_addr, reg_addr, I2C_MEMADD_SIZE_8BIT,data_ptr, len,I2Cx_FLAG_TIMEOUT); 
	/* 检查通讯状态 */
	if(status != HAL_OK)
	{
		/* 总线出错处理 */
		I2Cx_Error();
	}
	while (HAL_I2C_GetState(&halI2c2) != HAL_I2C_STATE_READY)
	{
		
	}
	/* 检查SENSOR是否就绪进行下一次读写操作 */
	while (HAL_I2C_IsDeviceReady(&halI2c2, slave_addr, I2Cx_FLAG_TIMEOUT, I2Cx_FLAG_TIMEOUT) == HAL_TIMEOUT);
	/* 等待传输结束 */
	while (HAL_I2C_GetState(&halI2c2) != HAL_I2C_STATE_READY)
	{
		
	}
	return status;
}

/**
  * @brief  读寄存器，这是提供给上层的接口
	* @param  slave_addr: 从机地址
	* @param 	reg_addr:寄存器地址
	* @param len：要读取的长度
	*	@param data_ptr:指向要存储数据的指针
  * @retval 正常为0，不正常为非0
  */
int Sensors_I2C2_ReadRegister(unsigned char slave_addr,
							unsigned char reg_addr,
							unsigned short len, 
							unsigned char *data_ptr)
{
	HAL_StatusTypeDef status = HAL_OK;
	status = HAL_I2C_Mem_Read(&halI2c2,slave_addr,reg_addr,I2C_MEMADD_SIZE_8BIT,data_ptr,len,I2Cx_LONG_TIMEOUT);    
	/* 检查通讯状态 */
	if(status != HAL_OK)
	{
		/* 总线出错处理 */
		I2Cx_Error();
	}
	while (HAL_I2C_GetState(&halI2c2) != HAL_I2C_STATE_READY)
	{
		
	}
	/* 检查SENSOR是否就绪进行下一次读写操作 */
	while (HAL_I2C_IsDeviceReady(&halI2c2, slave_addr+1, I2Cx_FLAG_TIMEOUT, I2Cx_FLAG_TIMEOUT) == HAL_TIMEOUT);
	/* 等待传输结束 */
	while (HAL_I2C_GetState(&halI2c2) != HAL_I2C_STATE_READY)
	{
		
	}
    return status;
}



/*****************************  (END OF FILE) *********************************/
