/*
*********************************************************************************************************
*
*	模块名称 : BSP模块(For STM32G0)
*	文件名称 : bsp.c
*	版    本 : V1.0
*	说    明 : 这是硬件底层驱动程序的主文件。每个c文件可以 #include "bsp.h" 来包含所有的外设驱动模块。
*			   bsp = Borad surport packet 板级支持包
*	修改记录 :
*
*	Copyright (C), 2018-2030
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
*********************************************************************************************************
*	                                   函数声明
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*	函 数 名: bsp_Init
*	功能说明: 初始化所有的硬件设备。该函数配置CPU寄存器和外设的寄存器并初始化一些全局变量。只需要调用一次
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Init(void)
{
     
	//bsp_KeyInit();    	/* 按键初始化，要放在滴答定时器之前，因为按钮检测是通过滴答定时器扫描 */
	bsp_InitTimer();  	/* 初始化滴答定时器 */
    bsp_LedInit();    	/* 初始化LED */	
    //bsp_BeepInit();
   
    
    bsp_UartInit();
	
	/* 针对不同的应用程序，添加需要的底层驱动模块初始化函数 */
    
    /* 初始化完成后 再打开独立看门狗 */
    //bsp_IwdgInit(ENABLE_CPU_IWDG);
}


/*
*********************************************************************************************************
*	函 数 名: User_Error_Handler
*	形    参: file : 源代码文件名称。关键字 __FILE__ 表示源代码文件名。
*			  line ：代码行号。关键字 __LINE__ 表示源代码行号
*	返 回 值: 无
*		User_Error_Handler(__FILE__, __LINE__);
*********************************************************************************************************
*/
void User_Error_Handler(char *file, uint32_t line)
{
	/* 
		用户可以添加自己的代码报告源代码文件名和代码行号，比如将错误文件和行号打印到串口
		printf("Wrong parameters value: file %s on line %d\r\n", file, line) 
	*/
	
	/* 这是一个死循环，断言失败时程序会在此处死机，以便于用户查错 */
	if (line == 0)
	{
		return;
	}
	
	while(1)
	{
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer10ms
*	功能说明: 该函数每隔10ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些处理时间要求不严格的
*			任务可以放在此函数。比如：按键扫描、蜂鸣器鸣叫控制等。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer10ms(void)
{
	//bsp_KeyScan10ms();
	//bsp_BeepPro();

}

/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer1ms
*	功能说明: 该函数每隔1ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些需要周期性处理的事务
*			 可以放在此函数。比如：触摸坐标扫描。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer1ms(void)
{
	
}

/*
*********************************************************************************************************
*	函 数 名: bsp_Idle
*	功能说明: 空闲时执行的函数。一般主程序在for和while循环程序体中需要插入 CPU_IDLE() 宏来调用本函数。
*			 本函数缺省为空操作。用户可以添加喂狗、设置CPU进入休眠模式的功能。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Idle(void)
{
	/* --- 喂狗 */
    bsp_IwdgFeed();
	/* --- 让CPU进入休眠，由Systick定时中断唤醒或者其他中断唤醒 */

	/* 例如 emWin 图形库，可以插入图形库需要的轮询函数 */
	//GUI_Exec();

	/* 例如 uIP 协议，可以插入uip轮询函数 */
	//TOUCH_CapScan();
    
    /* ModbusRTU 轮询函数 */
#if ( MODBUS_CFG_SLAVE_EN == 1 )
    MODS_Poll();    /* Modbus从站消息处理函数 */
#endif
#if ( MODBUS_CFG_HOST_EN == 1 )
    MODH_Poll();    /* Modbus主站消息处理函数 */
#endif
}



/***************************** (END OF FILE) *********************************/
