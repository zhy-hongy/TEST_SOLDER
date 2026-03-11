#include "bsp_all.h"


volatile uint32_t system_time 			= 0;
volatile uint32_t system_time_real 	=	0;
volatile uint32_t system_time_s 		= 0;

//系统定时器设置
void systick_init(void)
{
	NVIC_InitType NVIC_InitStructure;

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);	//select system clock	
	SysTick->LOAD = (72 * 1000 -1);
	SysTick->VAL =0x00;          											//clear timer value
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;          	//Start countdown	 
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;	        //enable it	 

	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_SYSTICK;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SUB_PRIO_SYSTICK;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void updateSystemTime(void) {
	system_time++;
	if(system_time > SYSTEM_TIME_MAX){
		system_time = 0;
	}
	system_time_s = system_time / 1000;
}

uint32_t getSystemTime(void) { return system_time; }

int32_t timeDifference(uint32_t current, uint32_t previous) {
    // 自动处理系统时间回绕问题
    if (current >= previous) {
        return current - previous;
    } else {
        // 处理回绕：SYSTEM_TIME_MAX + 1 是总时间范围
        return (SYSTEM_TIME_MAX + 1 - previous) + current;
    }
}

void delay_us(uint32_t us)
{
	uint32_t i = 0;
	for(i = 0; i < us; i++)
	{
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();
	}
}

void delay_ms(uint32_t ms)
{
	int32_t time, system_time_tmp;
	
	time = system_time;
	system_time_tmp = system_time;
	
	while((system_time_tmp - time) < ms){
		system_time_tmp = system_time;
		if(system_time_tmp < time){
			system_time_tmp += SYSTEM_TIME_MAX;
		}
	};
}




