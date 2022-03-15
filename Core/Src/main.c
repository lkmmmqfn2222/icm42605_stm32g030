/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp.h"
#include "bsp_icm42605.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"STM32G030_ICM42605"
#define EXAMPLE_DATE	"2021-05-22"
#define DEMO_VER		"1.0.0"
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int16_t iTemperature = 0;
icm42605RawData_t stAccData;
icm42605RawData_t stGyroData;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void app_SeggerRttInit(void);
static void app_ShowPowerOnInfo(void);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */

    /* USER CODE BEGIN 2 */
    bsp_Init();

    /* 打印相关信息 */
    app_SeggerRttInit();
	app_ShowPowerOnInfo();

    bsp_Icm42605Init();
    bsp_StartAutoTimer(SOFT_TMR_ID_ONESECOND, 1000);


    //bsp_SetTIMOutPWM(GPIOA,GPIO_PIN_8,TIM1,TIM_CHANNEL_1,2000,5000);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    for (;;)
    {
        /* 实现喂狗,Modbus轮询,刷新UI等操作，不可阻塞 */
        bsp_Idle();

        /* 1s 软定时器事件 */
        if(bsp_CheckTimer(SOFT_TMR_ID_ONESECOND))
        {
            bsp_IcmGetTemperature(&iTemperature);
            printf("1.Temp:%d\n",iTemperature);
            SEGGER_RTT_printf(0,"1.temp:%d\n",iTemperature);

            bsp_IcmGetAccelerometer(&stAccData);
            printf("2.AccX:%d-AccY:%d-AccZ:%d\n",stAccData.x,stAccData.y,stAccData.z);
            SEGGER_RTT_printf(0,"2.AccX:%d-AccY:%d-AccZ:%d\n",stAccData.x,stAccData.y,stAccData.z);

            bsp_IcmGetAccelerometer(&stAccData);
            printf("3.GyroX:%d-GyroY:%d-GyroZ:%d\n",stGyroData.x,stGyroData.y,stGyroData.z);
            SEGGER_RTT_printf(0,"3.GyroX:%d-GyroY:%d-GyroZ:%d\n",stGyroData.x,stGyroData.y,stGyroData.z);

            bsp_IcmGetRawData(&stAccData,&stGyroData);
            printf("4.AccX:%d-AccY:%d-AccZ:%d-GyroX:%d-GyroY:%d-GyroZ:%d\n",stAccData.x,stAccData.y,stAccData.z,stGyroData.x,stGyroData.y,stGyroData.z);
            SEGGER_RTT_printf(0,"4.AccX:%d-AccY:%d-AccZ:%d-GyroX:%d-GyroY:%d-GyroZ:%d\n",stAccData.x,stAccData.y,stAccData.z,stGyroData.x,stGyroData.y,stGyroData.z);
        }

        /* USER CODE END WHILE */


        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}


/* USER CODE BEGIN 4 */

void app_SeggerRttInit(void)
{
    /***********************************************
    * 描述： RTT使用配置
    */
    /* 配置通道0，上行配置*/
	SEGGER_RTT_ConfigUpBuffer(0, "RTTUP", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
	/* 配置通道0，下行配置*/
	SEGGER_RTT_ConfigDownBuffer(0, "RTTDOWN", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
}
/*******************************************************************************
 * 名    称： app_ShowPowerOnInfo
 * 功    能： 上电打印相关信息
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void app_ShowPowerOnInfo(void)
{
	/***********************************************
    * 描述： 通过RTT打印信息
    */
    SEGGER_RTT_SetTerminal(0);
    SEGGER_RTT_printf(0,"\n\r");
	SEGGER_RTT_printf(0,"*************************************************************\n\r");
	SEGGER_RTT_printf(0,"* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	SEGGER_RTT_printf(0,"* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	SEGGER_RTT_printf(0,"* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST库版本 可查看 stm32g0xx_hal.c 文件 */
	SEGGER_RTT_printf(0,"* Hal版本 :STM32G0xx_HAL_Driver:V1.4.1 \r\n");
	SEGGER_RTT_printf(0,"* \r\n");	/* 打印一行空格 */
	SEGGER_RTT_printf(0,"* Email : wangyao@zhiyigroup.cn \r\n");
	SEGGER_RTT_printf(0,"* 淘宝店: zhisheng.taobao.com\r\n");
	SEGGER_RTT_printf(0,"* 技术支持QQ群  : 539041646 \r\n");
	SEGGER_RTT_printf(0,"* Copyright www.zhiyigroup.cn 至一电子科技\r\n");
	SEGGER_RTT_printf(0,"*************************************************************\n\r");

	/***********************************************
    * 描述： 通过串口打印信息
    */
    printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

    /* 打印ST库版本 可查看 stm32g0xx_hal.c 文件 */
	printf("* Hal版本 :STM32G0xx_HAL_Driver:V1.4.1 \r\n");
	printf("* \r\n");	/* 打印一行空格 */
	printf("* Email : wangyao@zhiyigroup.cn \r\n");
	printf("* 淘宝店: zhisheng.taobao.com\r\n");
	printf("* 技术支持QQ群  : 539041646 \r\n");
	printf("* Copyright www.zhiyigroup.cn 至一电子科技\r\n");
	printf("*************************************************************\n\r");
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  User_Error_Handler(__FILE__, __LINE__);
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
