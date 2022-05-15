/* Lib */
#include "stdio.h "

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"


/* hal */
#include "hal_led.h"
#include "hal_time.h"
#include "hal_relay.h"
#include "hal_Uart.h"
#include "hal_key.h"




/*
**********************************************************************************************************
函数声明
**********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED1(void *pvParameters);
static void vTaskLED2(void *pvParameters);
static void vTaskKeyScan(void *pvParameters);
static void AppTaskCreate (void);
static void AppObjCreate (void);
static void vTimerCallback(xTimerHandle pxTimer);




/*
**********************************************************************************************************
变量声明
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleTaskLED1 = NULL;
static TaskHandle_t xHandleTaskKeyScan = NULL;
static TaskHandle_t xHandleTaskLED2 = NULL;


/*
**********************************************************************************************************
任务间通信变量的声明
**********************************************************************************************************
*/

/*事件标志组*/
#define BIT_0 (1 << 0)
#define BIT_1 (1 << 1)
#define BIT_ALL (BIT_0 | BIT_1)
static EventGroupHandle_t xCreatedEventGroup = NULL;

// /*消息队列*/
// static QueueHandle_t xQueue1 = NULL;
// static QueueHandle_t xQueue2 = NULL;


/*
*********************************************************************************************************
* 函 数 名: main
* 功能说明: 标准 c 程序入口。
* 形 参: 无 * 返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
    /* 
    在启动调度前，为了防止初始化 STM32 外设时有中断服务程序执行，这里禁止全局中断(除了 NMI 和 HardFault)。
    这样做的好处是：
    1. 防止执行的中断服务程序中有 FreeRTOS 的 API 函数。
    2. 保证系统正常启动，不受别的中断影响。
    3. 关于是否关闭全局中断，大家根据自己的实际情况设置即可。
    在移植文件 port.c 中的函数 prvStartFirstTask 中会重新开启全局中断。通过指令 cpsie i 开启，__set_PRIMASK(1)
    和 cpsie i 是等效的。
    */
    // __set_PRIMASK(1); 

    __disable_irq();

    /* 硬件初始化 */
    hal_ledConfig();

//    RELAY_Init();
    hal_TimeInit();
    hal_KeyInit();
    hal_Uart_Init();

    /* 创建任务 */
    AppTaskCreate();

    /* 创建任务通信机制 */
    AppObjCreate();

    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();

    /* 
    如果系统正常启动是不会运行到这里的，运行到这里极有可能是用于定时器任务或者空闲任务的
    heap 空间不足造成创建失败，此要加大 FreeRTOSConfig.h 文件中定义的 heap 大小：
    #define configTOTAL_HEAP_SIZE ( ( size_t ) ( 17 * 1024 ) )
    */
    while(1);
}




/*
*********************************************************************************************************
* 函 数 名: AppObjCreate
* 功能说明: 创建任务通信机制武汉安富莱电子有限公司
WWW.ARMFLY.COM 安富莱 STM32-V6 开发板 FreeRTOS 教程
2016年06月30日 版本： 1.0 第 453 页 共 1177 页
* 形 参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
static TimerHandle_t xTimers[2] = {NULL,NULL};


static void AppObjCreate (void)
{
	uint8_t i;
	
	const TickType_t xTimerPer[2] = {1000, 1000};
	
    /* 创建事件标志组 */
    xCreatedEventGroup = xEventGroupCreate();
    if(xCreatedEventGroup == NULL)
    {
     /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }


    
    
    /*
    1. 创建定时器，如果在 RTOS 调度开始前初始化定时器，那么系统启动后才会执行。
    2. 统一初始化两个定时器，他们使用共同的回调函数，在回调函数中通过定时器 ID 来区分
    是那个定时器的时间到。当然，使用不同的回调函数也是没问题的。
    */
    for(i = 0; i < 2; i++)
    {
        xTimers[i] = xTimerCreate("Timer",  /* 定时器名字 */
        xTimerPer[i],                       /* 定时器周期,单位时钟节拍 */
        pdTRUE,                             /* 周期性 */
        (void *) i,                         /* 定时器 ID */
        vTimerCallback);                    /* 定时器回调函数 */

        if(xTimers[i] == NULL)
        {
            /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
        }
        else
        {
            /* 启动定时器，系统启动后才开始工作 */
            if(xTimerStart(xTimers[i], 100) != pdPASS)
            {
                /* 定时器还没有进入激活状态 */
            }
        }
    }


    // /* 创建 10 个 uint8_t 型消息队列 */
    // xQueue1 = xQueueCreate(10, sizeof(uint8_t));
    // if( xQueue1 == 0 )
    // {
    //     /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    // }

    // /* 创建 10 个存储指针变量的消息队列，由于 CM3/CM4 内核是 32 位机，一个指针变量占用 4 个字节 */
    // xQueue2 = xQueueCreate(10, sizeof(struct Msg *));
    // if( xQueue2 == 0 )
    // {
    //     /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    // }




}

/*
*********************************************************************************************************
* 函 数 名: AppTaskCreate
* 功能说明: 创建应用任务
* 形 参: 无 * 返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	BaseType_t x = 0;
    // x = xTaskCreate( vTaskLED2, /* 任务函数 */
    //             "vTaskLED2", /* 任务名 */
    //             512, /* 任务栈大小，单位 word，也就是 4 字节 */
    //             NULL, /* 任务参数 */
    //             2, /* 任务优先级*/
    //             &xHandleTaskLED2 ); /* 任务句柄 */

    // x = xTaskCreate( vTaskLED1, /* 任务函数 */
    //             "vTaskLED1", /* 任务名 */
    //             512, /* 任务栈大小，单位 word，也就是 4 字节 */
    //             NULL, /* 任务参数 */
    //             2, /* 任务优先级*/
    //             &xHandleTaskLED1 ); /* 任务句柄 */

    x = xTaskCreate( vTaskTaskUserIF, /* 任务函数 */
                "vTaskTaskUserIF", /* 任务名 */
                512, /* 任务栈大小，单位 word，也就是 4 字节 */
                NULL, /* 任务参数 */
                1, /* 任务优先级*/
                &xHandleTaskUserIF ); /* 任务句柄 */

    x = xTaskCreate( vTaskKeyScan, /* 任务函数 */
                "vTaskKeyScan", /* 任务名 */
                512, /* 任务栈大小，单位 word，也就是 4 字节 */
                NULL, /* 任务参数 */
                9, /* 任务优先级*/
                &xHandleTaskKeyScan ); /* 任务句柄 */				
								
	x = x;
}



/*
*********************************************************************************************************
*   函 数 名 : vTaskTaskUserIF
*	功能说明 : 接口消息处理
*	形 参    : pvParameters 是在创建该任务时传递的形参
*	返 回 值 : 
*   优 先 级 : 1  (数值越小优先级越低，这个跟uCOS相反)
*********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters)
{
//	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];

    while(1)
    {
		// ucKeyCode = bsp_GetKey();
		
		// if (ucKeyCode != KEY_NONE)
		// {
		// 	switch (ucKeyCode)
			{
				/* K1键 打印任务执行情况 */
				// case KEY_DOWN_K1:			 
					printf("=================================================\r\n");
					printf("TaskName      State   Priority  Remain  TaskID\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
				
					printf("\r\nTaskName        Count          Ration\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
				// 	break;
				
				// /* 其他的键值不处理 */
				// default:                     
				// 	break;
			}
		// }
		
		vTaskDelay(2000);
	}
}
/*
*********************************************************************************************************
* 函 数 名: vTaskKeyScan
* 功能说明: 按键扫描
* 形 参: pvParameters 是在创建该任务时传递的形参
* 返 回 值: 无
* 优 先 级: 3
*********************************************************************************************************
*/
static void vTaskKeyScan(void *pvParameters)
{ 
    EventBits_t uxBits;
    unsigned char KeyValue = 0;


    while(1)
    {
        KeyValue = hal_KeyProc();
        if(KeyValue== KEY1_CLICK_RELEASE)
        {
            /* 设置事件标志组的 bit0 */
            uxBits = xEventGroupSetBits(xCreatedEventGroup, BIT_0);
            if((uxBits & BIT_0) != 0)
            {
                printf("K1 pressed , bit 0 has been set .\r\n");
            }
            else
            {
                printf("K1 pressed  , bit 0 has been clearred .\r\n");
            }

            printf("key1 release ....\r\n ");
        }

        vTaskDelay(10);
    } 

 }

/*
*********************************************************************************************************
* 函 数 名: vTaskLED1
* 功能说明: 信息处理，这里是用作 LED 闪烁
* 形 参: pvParameters 是在创建该任务时传递的形参
* 返 回 值: 无
* 优 先 级: 3 
*********************************************************************************************************
*/
static void vTaskLED1(void *pvParameters)
{
    EventBits_t uxBits;
    const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* 最大延迟 100ms */


    while(1)
    {
        // /* 测试任务调度锁 */
        // vTaskSuspendAll();
        // printf("task LED1 is running ... \r\n ");
        // xTaskResumeAll();
        // if(!xTaskResumeAll()) /* 关闭调度锁，如果需要任务切换，此函数返回 pdTRUE，否则返回 pdFALSE */
        // {
        //     taskYIELD ();
        // }



        /* 等 K2 按键按下设置 bit0 和 K3 按键按下设置 bit1 */
        uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* 事件标志组句柄 */
                                    BIT_0,  /* 等待 bit0 和 bit1 被设置 */
                                    pdTRUE, /* 退出前 bit0 和 bit1 被清除，这里是 bit0 和 bit1都被设置才表示“退出” */
                                    pdFALSE, /* 设置为 pdTRUE 表示等待 bit1 和 bit0 都被设置*/
                                    xTicksToWait); /* 等待延迟时间 */
        if((uxBits & BIT_0) == BIT_0)
        {
            /* 接收到 bit1 和 bit0 都被设置的消息 */
            printf("Receive  bit 0 has been set ..\r\n");

            /* 超时，另外注意仅接收到一个按键按下的消息时，变量 uxBits 的相应 bit 也是被设置的 */
            hal_Led1Turn();
        }

    } 
 }
/*
*********************************************************************************************************
* 函 数 名: vTaskLED2
* 功能说明: 启动任务，也就是最高优先级任务，这里用作 LED 闪烁
* 形 参: pvParameters 是在创建该任务时传递的形参
* 返 回 值: 无
* 优 先 级: 4 
*********************************************************************************************************
*/
static void vTaskLED2(void *pvParameters)
{
    while(1)
    {
        hal_Led2Drive(1);
        vTaskDelay(500);
        hal_Led2Drive(0);
        vTaskDelay(500);
    } 
}






/*
*********************************************************************************************************
* 函 数 名: vTimerCallback
* 功能说明: 定时器回调函数
* 形 参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
static void vTimerCallback(xTimerHandle pxTimer)
{
    uint32_t ulTimerID;
    configASSERT(pxTimer);

    /* 获取那个定时器时间到 */
    ulTimerID = (uint32_t)pvTimerGetTimerID(pxTimer);

    /* 处理定时器 0 任务 */
    if(ulTimerID == 0)
    {
        hal_Led1Turn();     
    }
    /* 处理定时器 1 任务 */
    if(ulTimerID == 1)
    {
        hal_Led2Turn();
    }
}





