#ifndef __TOOL_DET_H__
#define __TOOL_DET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"
#include "debug.h"

#define	SIGNAL_JITTER_FILTER_CNT_SYSTICK						25
#define SIGNAL_JITTER_FILTER_HALF_RATE							10
#define SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE	22
#define SIGNAL_JITTER_FILTER_CNT_SYSTICK_FAIL_RATE	01

#define CH_STATE_SW_OFF								0x00
#define CH_STATE_SW_TOOL_IN_ON				0x01
#define CH_STATE_SW_TOOL_TYPE_1_ON		0x02
#define CH_STATE_SW_TOOL_TYPE_2_ON		0x04
#define CH_STATE_SW_SLEEP_ON					0x08

typedef enum{
	notool = 0, tool_115 = 115, tool_210 = 210, tool_245 = 245, tool_115_210 = 115+210,tool_210_245 = 210+245
} tool_type_t;

typedef enum{
	tool_in = 0, tool_out
} tool_in_state_t;

typedef enum{
	tool_heat_off = 0, tool_heat_on
} tool_heat_state_t;

typedef enum{
	sleep_err = 0x00, sleep_base = 0x01, sleep_tool = 0x10, on_work = 0x11, sleep_trembling = 0x20
} sleep_state_t;

typedef enum{
	tool_type_sig_210 = 210, tool_type_sig_245 = 245
} tool_type_sig_t;

//typedef enum{
//	sig_reset = 0, sig_pulse = 0x01, sig_high = 0x10
//} tool_sig_chk_t;

typedef enum{
	sleep_sig_err = 0x00, sleep_sig_base = 0x01, 
	sleep_sig_tool = 0x10, sig_on_work = 0x11
} sleep_sig_t;

typedef struct{
	uint8_t					tool_in_update_flag;
	uint8_t					tool_type_1_update_flag;
	uint8_t					tool_type_2_update_flag;
	uint8_t					sleep_state_update_flag;
	
	uint8_t					tool_in_disable_in_heat;//在加热状态下不检测工具是否在位

	tool_in_state_t			tool_in_state;
	tool_type_t				tool_type_1;
	tool_type_t				tool_type_2;
	tool_type_t				tool_type_3;
	sleep_state_t			sleep_state;
	
	uint16_t				tool_in_chk_sig_bits_cnt[2];
	uint16_t				tool_in_chk_update_cnt;
	
	uint16_t				tool_type_1_sig_bits_cnt[2];
	uint16_t				tool_type_2_sig_bits_cnt[2];
	uint16_t				tool_type_1_update_cnt;
	uint16_t				tool_type_2_update_cnt;

	uint8_t					sleep_sig_bits;
	uint16_t				sleep_sig_bits_cnt[4];
	uint16_t				sleep_update_cnt;
} tool_detect_data_t;

extern volatile tool_detect_data_t tool_detect_data;

void tool_in_check_sig_update(volatile tool_detect_data_t* ch, uint8_t update_sw);
void tool_type_1_update(volatile tool_detect_data_t* ch, uint8_t update_sw);
void tool_type_2_update(volatile tool_detect_data_t* ch, uint8_t update_sw);
void sleep_state_update(volatile tool_detect_data_t* ch, uint8_t update_sw);
//void sleep_state_update(volatile tool_detect_data_t* ch, uint8_t update_sw, uint8_t heat_state);
void tool_detect_disable_in_heat_set(volatile tool_detect_data_t* tool_det);
void tool_detect_signal_reset(volatile tool_detect_data_t* ch);

#ifdef __cplusplus
}
#endif

#endif


