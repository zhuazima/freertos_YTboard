#include "stm32f10x.h"
#include "hal_led.h"


void hal_ledConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE); 						 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE); 	


	GPIO_InitStructure.GPIO_Pin = LED1_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; ; 
	GPIO_Init(LED1_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(LED1_PORT,LED1_PIN);
	
	GPIO_InitStructure.GPIO_Pin = LED2_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; ; 
	GPIO_Init(LED2_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(LED2_PORT,LED2_PIN);	
	
}


void hal_Led1Drive(unsigned char sta)
{
	if(sta)
	{
		GPIO_SetBits(LED1_PORT,LED1_PIN);
	}else
	{
		GPIO_ResetBits(LED1_PORT,LED1_PIN);
	}
}


void hal_Led2Drive(unsigned char sta)
{
	if(sta)
	{
		GPIO_SetBits(LED2_PORT,LED2_PIN);
	}else
	{
		GPIO_ResetBits(LED2_PORT,LED2_PIN);
	}
}


void hal_Led1Turn(void)
{
	GPIO_WriteBit(LED1_PORT,LED1_PIN,(BitAction)(1-GPIO_ReadOutputDataBit(LED1_PORT,LED1_PIN)));
}


void hal_Led2Turn(void)
{
	GPIO_WriteBit(LED2_PORT,LED2_PIN,(BitAction)(1-GPIO_ReadOutputDataBit(LED2_PORT,LED2_PIN)));
}
