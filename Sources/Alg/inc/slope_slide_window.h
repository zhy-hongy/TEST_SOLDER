#ifndef __SLOPE_SLIDE_WINDOW_H__
#define __SLOPE_SLIDE_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define MAX_SLIDE_WIDTH				200
#define SLIDE_WINDOW_WIDTH			25
#define SLOPE_STABLE_WAIT_CNT		10	
#define SLOPE_STABLE_CAL_CNT_MIN	5

#include "stdint.h"

typedef struct{
	uint8_t 	tail;
	uint8_t 	head;	
	int16_t 	flag_gap_cnt;
	int16_t 	slope_cnt;
	
	int16_t 	stable_cnt;
	
	float 		x_buf[SLIDE_WINDOW_WIDTH];
	float 		y_buf[SLIDE_WINDOW_WIDTH];
	float 		xy_buf[SLIDE_WINDOW_WIDTH];
	float 		xx_buf[SLIDE_WINDOW_WIDTH];
	
	float 		slope;
	float 		intercept;
	
	uint8_t 	slope_stable_flag;
} slope_slide_window_t;

void slope_slide_window_init(volatile slope_slide_window_t *slide_window);
void slope_slide_window_reset(volatile slope_slide_window_t *slide_window);

void slope_slide_window_push(volatile slope_slide_window_t *slide_window, uint8_t flag_last, float data);
void slope_slide_window_cal(volatile slope_slide_window_t *slide_window);

#ifdef __cplusplus
}
#endif

#endif