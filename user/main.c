#include "hal_led.h"

void delay(unsigned long DelayTime);

int main(void)
{
	hal_ledConfig();
	while(1)
	{
		hal_Led1Drive(0);
        delay(7200000);
        hal_Led1Drive(1);
        delay(7200000);
	}
}

void delay(unsigned long DelayTime)
{
    unsigned long i;
    for(i = 0;i < DelayTime;i++);
}
