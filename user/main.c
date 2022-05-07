/* Lib */
#include "stdio.h "

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"


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
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void vTaskKeyScan(void *pvParameters);
static void AppTaskCreate (void);



void KeyScan_handler(KEY_VALUE_TYPEDEF keys);

/*
**********************************************************************************************************
变量声明
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static TaskHandle_t xHandleTaskKeyScan = NULL;
static TaskHandle_t xHandleTaskStart = NULL;


/*
*********************************************************************************************************
* 函 数 名: main
* 功能说明: 标准 c 程序入口。
* 形 参: 无 * 返 回 值: 无
*********************************************************************************************************
*/


void KeyScan_handler(KEY_VALUE_TYPEDEF keys)
{   
    KEY_VALUE_TYPEDEF KeyValue;
    KeyValue = keys;

    if(KeyValue == KEY1_CLICK_RELEASE)
    {
        printf("This is key1 release ....");
    }

}




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


    hal_KeyScanCBSRegister(KeyScan_handler);

    // printf("who are you ");
    // printf("are you ");
    // printf("you ");

    /* 创建任务 */
    AppTaskCreate();
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
    while(1)
    {
        hal_KeyProc();
        vTaskDelay(10);
    } 
 }

/*
*********************************************************************************************************
* 函 数 名: vTaskMsgPro
* 功能说明: 信息处理，这里是用作 LED 闪烁
* 形 参: pvParameters 是在创建该任务时传递的形参
* 返 回 值: 无
* 优 先 级: 3 
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
    while(1)
    {
        hal_Led1Drive(1);
        vTaskDelay(1000);
        hal_Led1Drive(0);
        vTaskDelay(1000);
    } 
 }
/*
*********************************************************************************************************
* 函 数 名: vTaskStart
* 功能说明: 启动任务，也就是最高优先级任务，这里用作 LED 闪烁
* 形 参: pvParameters 是在创建该任务时传递的形参
* 返 回 值: 无
* 优 先 级: 4 
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
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
* 函 数 名: AppTaskCreate
* 功能说明: 创建应用任务
* 形 参: 无 * 返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	BaseType_t x = 0;
    x = xTaskCreate( vTaskStart, /* 任务函数 */
                "vTaskStart", /* 任务名 */
                512, /* 任务栈大小，单位 word，也就是 4 字节 */
                NULL, /* 任务参数 */
                2, /* 任务优先级*/
                &xHandleTaskStart ); /* 任务句柄 */

    x = xTaskCreate( vTaskMsgPro, /* 任务函数 */
                "vTaskMsgPro", /* 任务名 */
                512, /* 任务栈大小，单位 word，也就是 4 字节 */
                NULL, /* 任务参数 */
                2, /* 任务优先级*/
                &xHandleTaskMsgPro ); /* 任务句柄 */

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
                10, /* 任务优先级*/
                &xHandleTaskKeyScan ); /* 任务句柄 */				
								
	x = x;
}








