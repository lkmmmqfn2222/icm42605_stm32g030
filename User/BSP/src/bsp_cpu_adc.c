
#include "bsp_cpu_adc.h"
#include "stm32g0xx_hal.h"


ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

__IO uint16_t adcCovValueBuff[ADC_CHANNEL_TIMES][ADC_CHANNEL_NUMBER] = {0};
uint16_t adcAverageBuff[ADC_CHANNEL_NUMBER] = {0};


void Adc_Error_Handler(void);

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

    /* USER CODE BEGIN ADC1_Init 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();
    /* USER CODE END ADC1_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */
    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.LowPowerAutoPowerOff = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = ADC_CHANNEL_NUMBER;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_12CYCLES_5;
    hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_12CYCLES_5;
    hadc1.Init.OversamplingMode = DISABLE;
    hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
    if(HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Adc_Error_Handler();
    }
    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_7;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Adc_Error_Handler();
    }
    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_2;
    if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Adc_Error_Handler();
    }
    /* USER CODE BEGIN ADC1_Init 2 */

    /* USER CODE END ADC1_Init 2 */

}

void Adc_Error_Handler(void)
{

}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**ADC1 GPIO Configuration
    PA7     ------> ADC1_IN7
    PB0     ------> ADC1_IN8
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

static void MX_DMA_Init(void)
{

    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    //HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    //HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if(HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
        Adc_Error_Handler();
    }

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

}


void bsp_CpuAdcInit(void)
{
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();

}

void bsp_CpuAdcStart(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcCovValueBuff, (ADC_CHANNEL_TIMES * ADC_CHANNEL_NUMBER));
}

void bsp_CpuAdcStop(void)
{
    HAL_ADC_Stop_DMA(&hadc1);
}

void bsp_CpuAdcGetAverageValue(void)
{
    uint8_t count;
    uint8_t i;
    uint32_t sum = 0;

    for(i = 0; i < ADC_CHANNEL_NUMBER; i++)
    {
        for(count = 0; count < ADC_CHANNEL_TIMES; count++)
        {
            sum += adcCovValueBuff[count][i];
        }
        adcAverageBuff[i] = sum / ADC_CHANNEL_TIMES;                                                                                                                                       ;
        sum = 0;
    }
}





/*****************************  (END OF FILE) *********************************/
