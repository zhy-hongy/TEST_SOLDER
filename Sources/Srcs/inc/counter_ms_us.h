#ifndef __COUNTER_MS_US__
#define __COUNTER_MS_US__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
	
#define DEBUG_UNIT_COUNTER_MAX  	1000000
#define DEBUG_UNIT_BASE						50000					// 设置多少us为一个unit

typedef struct US_CUNTER{
	uint32_t timer_unit_base;
	uint32_t unit_count_max;
	
	uint32_t start_tim_counter;
	uint32_t start_unit_counter;

	uint32_t end_tim_counter;
	uint32_t end_unit_counter;
	
	uint32_t s;
	uint16_t ms;
	uint16_t us;
	uint8_t  work_flag;
} counter_ms_us_t;
	

extern volatile uint32_t debug_unit_counter;
extern counter_ms_us_t debug_time_record_ch[20];

void record_start(counter_ms_us_t* ch);
void cal_record_time(counter_ms_us_t* ch);

#ifdef __cplusplus
}
#endif

#endif
