#include "bsp_all.h"
#include "counter_ms_us.h"

volatile uint32_t debug_unit_counter = 0;
counter_ms_us_t debug_time_record_ch[20];

// ÅäÖÃtimer¼ÆÊı1Îª1us
void init_channels(uint32_t unit_base)
{
	int i;
	for(i = 0; i < 20; ++i){
		debug_time_record_ch[i].work_flag = 0;
		debug_time_record_ch[i].timer_unit_base = 1000;
		debug_time_record_ch[i].unit_count_max = DEBUG_UNIT_COUNTER_MAX;
	}
}

void record_start(counter_ms_us_t* ch)
{
	ch->start_tim_counter = TIM_GetCnt(DEBUG_US_COUNTER_TIM);
	ch->start_unit_counter = debug_unit_counter;
	ch->work_flag = 1;
}

void cal_record_time(counter_ms_us_t* ch)
{
	uint32_t tmp_cnt;
	
	if(ch->end_tim_counter > ch->start_tim_counter)
	{
		ch->us = ch->end_tim_counter - ch->start_tim_counter;
	}else{
		ch->us = ch->timer_unit_base 
					 + ch->end_tim_counter - ch->start_tim_counter;
	}
	
	if(ch->end_unit_counter - ch->start_unit_counter){
		tmp_cnt = ch->end_unit_counter - ch->start_unit_counter;
	}else{
		tmp_cnt = ch->unit_count_max + ch->end_tim_counter - ch->start_unit_counter;
	}
	
	ch->ms = tmp_cnt * (ch->timer_unit_base / 1000) 
				+ ch->us / ch->timer_unit_base;

	
	ch->s  = ch->ms / 1000;
	ch->ms = ch->ms % 1000;
	ch->us = ch->us % 1000;
}

void record_end(counter_ms_us_t* ch)
{
	ch->end_tim_counter = TIM_GetCnt(DEBUG_US_COUNTER_TIM);
	ch->end_unit_counter = debug_unit_counter;
	cal_record_time(ch);
}

