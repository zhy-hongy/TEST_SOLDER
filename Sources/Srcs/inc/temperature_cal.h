#ifndef _TEMPERATURE_CAL_H_
#define _TEMPERATURE_CAL_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include <stdint.h>
#include <string.h>
#include "debug.h"
#include "adc_aq.h"
	
#define CAL_LEN			20

#define MV_MIN			0
#define MV_MAX			5	

#define TEMP_MIN		0
#define TEMP_MAX		500
#define TEMP_EXCEED 1000
	

typedef struct{
		uint8_t length;
		float *mv;
		float *temp;
		float k[CAL_LEN];
		float b[CAL_LEN];
}	temp_cal_data_t;
	
void init_all_temp_tab(void);
float cal_temp_210(float mv);
float cal_temp_245(float mv);

float get_temp(float adc_val, uint8_t tool_type);
#ifdef __cplusplus
}
#endif

#endif
