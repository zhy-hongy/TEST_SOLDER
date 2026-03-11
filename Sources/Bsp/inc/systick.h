#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"

#define SYSTEM_TIME_MAX		1000000
	
extern volatile uint32_t system_time;
extern volatile uint32_t system_time_real;
extern volatile uint32_t system_time_s;

void systick_init(void);
void updateSystemTime(void);
uint32_t getSystemTime(void);
int32_t timeDifference(uint32_t current, uint32_t previous); 
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);


#ifdef __cplusplus
}
#endif

#endif


