/*
*********************************************************************************************************
*
*	模块名称 : 定时器模块
*	文件名称 : bsp_timer.c
*	版    本 : V1.0
*	说    明 : 配置systick定时器作为系统滴答定时器。缺省定时周期为1ms。
*
*				实现了多个软件定时器供主程序使用(精度1ms)， 可以通过修改 TMR_COUNT 增减定时器个数
*				实现了ms级别延迟函数（精度1ms） 和us级延迟函数
*				实现了系统运行时间函数（1ms单位）
*
*	修改记录 :
*
*	Copyright (C), 2015-2030
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "stm32g0xx_hal.h"

#define USE_HARDTIMERX

#if defined(USE_HARDTIMERX)

#define USE_HARDTIMER_TIM3
//#define USE_HARDTIMER_TIM17

#if defined (USE_HARDTIMER_TIM3)
#define TIM_HARD TIM3
#define RCC_TIM_HARD_CLK_ENABLE() __HAL_RCC_TIM3_CLK_ENABLE()
#define TIM_HARD_IRQn           TIM3_IRQn
#define TIM_HARD_IRQHandler     TIM3_IRQHandler
#endif

#if defined (USE_HARDTIMER_TIM17)
#define TIM_HARD TIM17
#define RCC_TIM_HARD_CLK_ENABLE() __HAL_RCC_TIM17_CLK_ENABLE()
#define TIM_HARD_IRQn           TIM17_IRQn
#define TIM_HARD_IRQHandler     TIM17_IRQHandler
#endif


/* 保存 TIM定时中断到后执行的回调函数指针 */
static void (*s_TIM_CallBack1)(void);
static void (*s_TIM_CallBack2)(void);
static void (*s_TIM_CallBack3)(void);
static void (*s_TIM_CallBack4)(void);

#endif


/* 这2个全局变量转用于 bsp_DelayMS() 函数 */
static volatile uint32_t s_uiDelayCount = 0;
static volatile uint8_t  s_ucTimeOutFlag = 0;

/* 定于软件定时器结构体变量 */
static SOFT_TMR s_tTmr[TMR_COUNT] = {0};

/*
	全局运行时间，单位1ms
	最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
*/
volatile int32_t g_iRunTime = 0;

static volatile uint8_t g_ucEnableSystickISR = 0;	/* 等待变量初始化 */

static void bsp_SoftTimerDec(SOFT_TMR *_tmr);


/*
*********************************************************************************************************
*	函 数 名: bsp_InitTimer
*	功能说明: 配置systick中断，并初始化软件定时器变量
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitTimer(void)
{
	uint8_t i;

	/* 清零所有的软件定时器 */
	for (i = 0; i < TMR_COUNT; i++)
	{
		s_tTmr[i].Count = 0;
		s_tTmr[i].PreLoad = 0;
		s_tTmr[i].Flag = 0;
		s_tTmr[i].Mode = TMR_ONCE_MODE;	/* 缺省是1次性工作模式 */
	}

	/*
		配置systic中断周期为1ms，并启动systick中断。

    	SystemCoreClock 是固件中定义的系统内核时钟，对于STM32H7,一般为400MHz

    	SysTick_Config() 函数的形参表示内核时钟多少个周期后触发一次Systick定时中断.
	    	-- SystemCoreClock / 1000  表示定时频率为 1000Hz， 也就是定时周期为  1ms
	    	-- SystemCoreClock / 500   表示定时频率为 500Hz，  也就是定时周期为  2ms
	    	-- SystemCoreClock / 2000  表示定时频率为 2000Hz， 也就是定时周期为  500us

    	对于常规的应用，我们一般取定时周期1ms。对于低速CPU或者低功耗应用，可以设置定时周期为 10ms
    */
	SysTick_Config(SystemCoreClock / 1000);
	
	g_ucEnableSystickISR = 1;		/* 1表示执行systick中断 */
    
    #if defined(USE_HARDTIMERX)
    bsp_InitHardTimer();
	#endif
}

/*
*********************************************************************************************************
*	函 数 名: SysTick_ISR
*	功能说明: SysTick中断服务程序，每隔1ms进入1次
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
extern void bsp_RunPer1ms(void);
extern void bsp_RunPer10ms(void);
void SysTick_ISR(void)
{
	static uint8_t s_count = 0;
	uint8_t i = 0;
    
    if (g_ucEnableSystickISR == 0) /* 做了一个变量标志， 调用了函数 bsp_InitTimer 才置位此变量 */
    {
        return;
    }
	
	/* 每隔1ms进来1次 （仅用于 bsp_DelayMS） */
	if (s_uiDelayCount > 0)
	{
		if (--s_uiDelayCount == 0)
		{
			s_ucTimeOutFlag = 1;
		}
	}

	/* 每隔1ms，对软件定时器的计数器进行减一操作 */
	for (i = 0; i < TMR_COUNT; i++)
	{
		bsp_SoftTimerDec(&s_tTmr[i]);
	}

	/* 全局运行时间每1ms增1 */
	g_iRunTime++;
	if (g_iRunTime == 0x7FFFFFFF)	/* 这个变量是 int32_t 类型，最大数为 0x7FFFFFFF */
	{
		g_iRunTime = 0;
	}

	bsp_RunPer1ms();		/* 每隔1ms调用一次此函数，此函数在 bsp.c */

	if (++s_count >= 10)
	{
		s_count = 0;

		bsp_RunPer10ms();	/* 每隔10ms调用一次此函数，此函数在 bsp.c */
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SoftTimerDec
*	功能说明: 每隔1ms对所有定时器变量减1。必须被SysTick_ISR周期性调用。
*	形    参:  _tmr : 定时器变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
	if (_tmr->Count > 0)
	{
		/* 如果定时器变量减到1则设置定时器到达标志 */
		if (--_tmr->Count == 0)
		{
			_tmr->Flag = 1;

			/* 如果是自动模式，则自动重装计数器 */
			if(_tmr->Mode == TMR_AUTO_MODE)
			{
				_tmr->Count = _tmr->PreLoad;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DelayMS
*	功能说明: ms级延迟，延迟精度为正负1ms
*	形    参:  n : 延迟长度，单位1 ms。 n 应大于2
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DelayMS(uint32_t n)
{
	if (n == 0)
	{
		return;
	}
	else if (n == 1)
	{
		n = 2;
	}

	DISABLE_INT();  			/* 关中断 */

	s_uiDelayCount = n;
	s_ucTimeOutFlag = 0;

	ENABLE_INT();  				/* 开中断 */

	while (1)
	{
		//bsp_Idle();				/* CPU空闲执行的操作， 见 bsp.c 和 bsp.h 文件 */

		/*
			等待延迟时间到
			注意：编译器认为 s_ucTimeOutFlag = 0，所以可能优化错误，因此 s_ucTimeOutFlag 变量必须申明为 volatile
		*/
		if (s_ucTimeOutFlag == 1)
		{
			break;
		}
	}
}

/*
*********************************************************************************************************
*    函 数 名: bsp_DelayUS
*    功能说明: us级延迟。 必须在systick定时器启动后才能调用此函数。
*    形    参:  n : 延迟长度，单位1 us
*    返 回 值: 无
*********************************************************************************************************
*/
void bsp_DelayUS(uint32_t n)
{
    uint32_t ticks;
    uint32_t told;
    uint32_t tnow;
    uint32_t tcnt = 0;
    uint32_t reload;
       
	reload = SysTick->LOAD;                
    ticks = n * (SystemCoreClock / 1000000);	 /* 需要的节拍数 */  
    
    tcnt = 0;
    told = SysTick->VAL;             /* 刚进入时的计数器值 */

    while (1)
    {
        tnow = SysTick->VAL;    
        if (tnow != told)
        {    
            /* SYSTICK是一个递减的计数器 */    
            if (tnow < told)
            {
                tcnt += told - tnow;    
            }
            /* 重新装载递减 */
            else
            {
                tcnt += reload - tnow + told;    
            }        
            told = tnow;

            /* 时间超过/等于要延迟的时间,则退出 */
            if (tcnt >= ticks)
            {
            	break;
            }
        }  
    }
} 

/*
*********************************************************************************************************
*	函 数 名: bsp_StartTimer
*	功能说明: 启动一个定时器，并设置定时周期。
*	形    参:  	_id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*				_period : 定时周期，单位1ms
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_StartTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* 打印出错的源代码文件名、函数名称 */
		//BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* 参数异常，死机等待看门狗复位 */
	}

	DISABLE_INT();  			/* 关中断 */

	s_tTmr[_id].Count = _period;		/* 实时计数器初值 */
	s_tTmr[_id].PreLoad = _period;		/* 计数器自动重装值，仅自动模式起作用 */
	s_tTmr[_id].Flag = 0;				/* 定时时间到标志 */
	s_tTmr[_id].Mode = TMR_ONCE_MODE;	/* 1次性工作模式 */

	ENABLE_INT();  				/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_StartAutoTimer
*	功能说明: 启动一个自动定时器，并设置定时周期。
*	形    参:  	_id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*				_period : 定时周期，单位10ms
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* 打印出错的源代码文件名、函数名称 */
		//BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* 参数异常，死机等待看门狗复位 */
	}

	DISABLE_INT();  		/* 关中断 */

	s_tTmr[_id].Count = _period;			/* 实时计数器初值 */
	s_tTmr[_id].PreLoad = _period;		/* 计数器自动重装值，仅自动模式起作用 */
	s_tTmr[_id].Flag = 0;				/* 定时时间到标志 */
	s_tTmr[_id].Mode = TMR_AUTO_MODE;	/* 自动工作模式 */

	ENABLE_INT();  			/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_StopTimer
*	功能说明: 停止一个定时器
*	形    参:  	_id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_StopTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		/* 打印出错的源代码文件名、函数名称 */
//		BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* 参数异常，死机等待看门狗复位 */
	}

	DISABLE_INT();  	/* 关中断 */

	s_tTmr[_id].Count = 0;				/* 实时计数器初值 */
	s_tTmr[_id].Flag = 0;				/* 定时时间到标志 */
	s_tTmr[_id].Mode = TMR_ONCE_MODE;	/* 自动工作模式 */

	ENABLE_INT();  		/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CheckTimer
*	功能说明: 检测定时器是否超时
*	形    参:  	_id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*				_period : 定时周期，单位1ms
*	返 回 值: 返回 0 表示定时未到， 1表示定时到
*********************************************************************************************************
*/
uint8_t bsp_CheckTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		return 0;
	}

	if (s_tTmr[_id].Flag == 1)
	{
		s_tTmr[_id].Flag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetRunTime
*	功能说明: 获取CPU运行时间，单位1ms。最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
*	形    参:  无
*	返 回 值: CPU运行时间，单位1ms
*********************************************************************************************************
*/
int32_t bsp_GetRunTime(void)
{
	int32_t runtime;

	DISABLE_INT();  	/* 关中断 */

	runtime = g_iRunTime;	/* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

	ENABLE_INT();  		/* 开中断 */

	return runtime;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CheckRunTime
*	功能说明: 计算当前运行时间和给定时刻之间的差值。处理了计数器循环。
*	形    参:  _LastTime 上个时刻
*	返 回 值: 当前时间和过去时间的差值，单位1ms
*********************************************************************************************************
*/
int32_t bsp_CheckRunTime(int32_t _LastTime)
{
	int32_t now_time;
	int32_t time_diff;

	DISABLE_INT();  	/* 关中断 */

	now_time = g_iRunTime;	/* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

	ENABLE_INT();  		/* 开中断 */
	
	if (now_time >= _LastTime)
	{
		time_diff = now_time - _LastTime;
	}
	else
	{
		time_diff = 0x7FFFFFFF - _LastTime + now_time;
	}

	return time_diff;
}

/*
*********************************************************************************************************
*    下面的代码使用一个TIM的4个捕获中断，实现4个硬件定时器
*********************************************************************************************************
*/

#if defined(USE_HARDTIMERX)

/*
*********************************************************************************************************
*    函 数 名: bsp_InitHardTimer
*    功能说明: 配置 TIMx，用于us级别硬件定时。TIMx将自由运行，永不停止.
*            TIMx可以用TIM2 - TIM5 之间的TIM, 这些TIM有4个通道, 挂在 APB1 上，输入时钟=SystemCoreClock / 2
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitHardTimer(void)
{
    TIM_HandleTypeDef  TimHandle = {0};
    uint32_t usPeriod;
    uint16_t usPrescaler;
    uint32_t uiTIMxCLK;
    TIM_TypeDef* TIMx = TIM_HARD;
    
    RCC_TIM_HARD_CLK_ENABLE();      /* 使能TIM时钟 */
    
    /*-----------------------------------------------------------------------
    APB 时钟 == 主时钟 64MHZ
    根据实际的APB时钟 得到正确的 uiTIMxCLK
    ----------------------------------------------------------------------- */
    uiTIMxCLK = SystemCoreClock;

    usPrescaler = uiTIMxCLK / 1000000 - 1;  /* 分频比 = 1 */
    
    usPeriod = 0xFFFF;      //都是16位通用定时器


    /* 
       设置分频为usPrescaler后，那么定时器计数器计1次就是1us
       而参数usPeriod的值是决定了最大计数：
       usPeriod = 0xFFFF 表示最大0xFFFF微秒。
       usPeriod = 0xFFFFFFFF 表示最大0xFFFFFFFF微秒。
    */
    TimHandle.Instance = TIMx;
    TimHandle.Init.Prescaler         = usPrescaler;
    TimHandle.Init.Period            = usPeriod;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;
    TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
        User_Error_Handler(__FILE__, __LINE__);
    }

    /* 配置定时器中断，给CC捕获比较中断使用 */
    {
        HAL_NVIC_SetPriority(TIM_HARD_IRQn, 0, 2);
        HAL_NVIC_EnableIRQ(TIM_HARD_IRQn);  
    }
    
    /* 启动定时器 */
    HAL_TIM_Base_Start(&TimHandle);
    
    /* 启动溢出中断，用于运行时间计数, us单位 */    
    TIMx->SR = (uint16_t)~TIM_IT_UPDATE;   /* 清除UPDATE中断标志 */
    TIMx->DIER |= TIM_IT_UPDATE;           /* 使能UPDATE中断 */    
}

/*
*********************************************************************************************************
*   函 数 名: bsp_StartHardTimer
*   功能说明: 使用TIM2-5做单次定时器使用, 定时时间到后执行回调函数。可以同时启动4个定时器通道，互不干扰。
*             定时精度正负1us （主要耗费在调用本函数的执行时间）
*   形    参: _CC : 捕获比较通道几，1，2，3, 4
*             _uiTimeOut : 超时时间, 单位 1us. 对于16位定时器，最大 65.5ms; 对于32位定时器，最大 4294秒
*             _pCallBack : 定时时间到后，被执行的函数
*   返 回 值: 无
*********************************************************************************************************
*/
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void *_pCallBack)
{
    uint32_t cnt_now;
    uint32_t cnt_tar;
    TIM_TypeDef* TIMx = TIM_HARD;
    
    cnt_now = TIMx->CNT; 
    cnt_tar = cnt_now + _uiTimeOut;         /* 计算捕获的计数器值 */
    if (_CC == 1)
    {
        s_TIM_CallBack1 = (void (*)(void))_pCallBack;

        TIMx->CCR1 = cnt_tar;               /* 设置捕获比较计数器CC1 */
        TIMx->SR = (uint16_t)~TIM_IT_CC1;   /* 清除CC1中断标志 */
        TIMx->DIER |= TIM_IT_CC1;           /* 使能CC1中断 */
    }
    else if (_CC == 2)
    {
        s_TIM_CallBack2 = (void (*)(void))_pCallBack;

        TIMx->CCR2 = cnt_tar;               /* 设置捕获比较计数器CC2 */
        TIMx->SR = (uint16_t)~TIM_IT_CC2;   /* 清除CC2中断标志 */
        TIMx->DIER |= TIM_IT_CC2;           /* 使能CC2中断 */
    }
    else if (_CC == 3)
    {
        s_TIM_CallBack3 = (void (*)(void))_pCallBack;

        TIMx->CCR3 = cnt_tar;               /* 设置捕获比较计数器CC3 */
        TIMx->SR = (uint16_t)~TIM_IT_CC3;   /* 清除CC3中断标志 */
        TIMx->DIER |= TIM_IT_CC3;           /* 使能CC3中断 */
    }
    else if (_CC == 4)
    {
        s_TIM_CallBack4 = (void (*)(void))_pCallBack;

        TIMx->CCR4 = cnt_tar;               /* 设置捕获比较计数器CC4 */
        TIMx->SR = (uint16_t)~TIM_IT_CC4;   /* 清除CC4中断标志 */
        TIMx->DIER |= TIM_IT_CC4;           /* 使能CC4中断 */
    }
    else
    {
        return;
    }
}

/*
*********************************************************************************************************
*    函 数 名: TIMx_IRQHandler
*    功能说明: TIM 中断服务程序
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void TIM_HARD_IRQHandler(void)
{
    uint32_t timesr;
    uint16_t itstatus = 0x0, itenable = 0x0;
    TIM_TypeDef *TIMx = TIM_HARD;

    timesr = TIMx->SR;
    
    /* 溢出中断，用于CPU运行时间计算. 65.535ms进入一次 */
    if (timesr & TIM_IT_UPDATE)
    {
        TIMx->SR = (uint16_t)~TIM_IT_UPDATE;
        

    }
    
    itstatus = timesr & TIM_IT_CC1;
    itenable = TIMx->DIER & TIM_IT_CC1;
    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
    {
        TIMx->SR = (uint16_t)~TIM_IT_CC1;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC1;    /* 禁能CC1中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack1();
    }

    itstatus = timesr & TIM_IT_CC2;
    itenable = TIMx->DIER & TIM_IT_CC2;
    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
    {
        TIMx->SR = (uint16_t)~TIM_IT_CC2;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC2; /* 禁能CC2中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack2();
    }

    itstatus = timesr & TIM_IT_CC3;
    itenable = TIMx->DIER & TIM_IT_CC3;
    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
    {
        TIMx->SR = (uint16_t)~TIM_IT_CC3;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC3; /* 禁能CC2中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack3();
    }

    itstatus = timesr & TIM_IT_CC4;
    itenable = TIMx->DIER & TIM_IT_CC4;
    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
    {
        TIMx->SR = (uint16_t)~TIM_IT_CC4;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC4; /* 禁能CC4中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack4();
    }
}

#endif


/***************************** (END OF FILE) *********************************/
