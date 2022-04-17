

#include "hal_relay.h"
#include "stm32f10x.h"

void RELAY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //APB2外设时钟使能      
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//启动AFIO重映射功能时钟    
    GPIO_InitStructure.GPIO_Pin = RELAY1 | RELAY2; //选择端口号（0~15或all）                        
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //选择IO接口工作方式       
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //设置IO接口速度（2/10/50MHz）    
	GPIO_Init(RELAYPORT, &GPIO_InitStructure);

	//必须将禁用JTAG功能才能做GPIO使用
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);// 改变指定管脚的映射,完全禁用JTAG+SW-DP
	GPIO_ResetBits(RELAYPORT,RELAY1 | RELAY2); //都为低电平（0） 初始为关继电器							
}


void RELAY_1(unsigned char c){ //继电器的控制程序（c=0继电器放开，c=1继电器吸合）
	GPIO_WriteBit(RELAYPORT,RELAY1,(BitAction)(c));//通过参数值写入接口
}

void RELAY_2(unsigned char c){ //继电器的控制程序（c=0继电器放开，c=1继电器吸合）
	GPIO_WriteBit(RELAYPORT,RELAY2,(BitAction)(c));//通过参数值写入接口
}

