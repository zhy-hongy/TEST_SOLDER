#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include "uart_usb.h"
#include "queue.h"
#include "mux_ctrl.h"
	

#define DEBUG_ON										1

#if DEBUG_ON == 1
	
#define log_info(CHAR) uart_usb_send_to_txbuffer(CHAR, strlen(CHAR))
	
#ifdef CONFIG_DEBUG_ON
    #define DBG_I(fmt, ...) do { \
        sprintf(debug_string_out_0, fmt, ##__VA_ARGS__); \
        log_info(debug_string_out_0); \
    } while(0)
#else
#define DBG_I(fmt, ...)
#endif

#define DEBUG_CMD_CTROL							1

// Logic debug
#define DEBUG_STAGE_TEST						1
	
#define DEBUG_CTROL_PROCESS_OUTPUT_STRING 			0
#define DEBUG_CTROL_PROCESS_OUTPUT_LED		 			0

#define DEBUG_SET_MODE_TEST							1

#define DEBUG_LOGIC_HEAT_TEST							0
	
#define DEBUG_TEMP_DETAILE_OUTPUT						0
#define DEBUG_TEMP_ARRAY_OUTPUT							0
#define DEBUG_VOL_CUR_OUTPUT							1
#define	DEBUG_ERROR_TEST								0

#define DEBUG_CMD_TARGET_TEMP						0

#define DEBUG_ERROR                			1

#define DEBUG_PARAM_OUTPUT 						1

// BSP debug
#define DEBUG_TEMP_INIT_INFO							0
#define DEBUG_HEAT_IO_TEST								0
#define DEBUG_KEY_TEST									0
#define DEBUG_ADC_REG_DMA			 					0
#define DEBUG_ADC_INJ_INT			 					1



typedef struct
{
	uint8_t timer_us_flag;
	uint8_t zero_int_flag;
	uint8_t timer_ctrl_flag;

	uint8_t adc_regular_dma_flag;
	uint8_t adc_injection_flag;
	
	uint8_t adc_after_calculation_flag;

	uint8_t mux_sw1_set_flag;
	uint8_t mux_sw2_set_flag;
	uint8_t mux_sw1_reset_flag;
	uint8_t mux_sw2_reset_flag;

	uint8_t beep_on_flag;
	uint8_t beep_off_flag;

	uint8_t heat_once_flag;
	uint8_t min_temp_flag_enable;

	int8_t temp_test_num;

	int8_t pwm_test_add_flag;

}bsp_dbg_t;


typedef struct
{
	uint8_t	stage_flag;
	uint8_t ctrl_process_flag;

	uint8_t adc_temp_detail_flag;
	uint8_t temp_filt_array_flag;

	uint8_t voltage_current_flag;

	uint8_t error_flag;

	uint8_t temp_target_flag;
}logic_dbg_t;

extern volatile bsp_dbg_t 		ch_bsp_debug;
extern volatile logic_dbg_t  	ch_logic_debug;
extern volatile queue_status_t	queue_status;

extern char debug_string_out_0[];
extern char debug_string_out_1[];

extern char debug_string_ctrl_0[];
extern char debug_string_ctrl_1[];

extern char debug_string_int_0[];
extern char debug_string_int_1[];

extern volatile circle_queue_t 	debug_mv_queue;
extern volatile circle_queue_t 	debug_temperature_queue;
extern volatile circle_queue_t 	debug_filted_temp_x1_queue;
extern volatile circle_queue_t	debug_filted_temp_x2_queue;
extern volatile circle_queue_t 	debug_filted_temp_show_queue;
extern volatile circle_queue_t 	debug_cnts_queue_1;
extern volatile circle_queue_t 	debug_cnts_queue_2;
extern volatile circle_queue_t 	debug_state_queue;

extern volatile float 		debug_temperature_buffer[100];
extern volatile float 		debug_filted_temp_x1_buffer[100];
extern volatile float 		debug_filted_temp_x2_buffer[100];
extern volatile float 		debug_filted_temp_show_buffer[100];
extern volatile uint16_t 	debug_cnts_buffer[100];
extern volatile uint8_t		debug_heat_state_buffer[100];

extern volatile float debug_detail_temp_mv[50];
extern volatile float debug_detail_temp[50];


extern int debug_i;

extern int debug_temp_target;

extern int 	debug_ctrl_ch1_num,
						debug_ctrl_ch1_contnu_cnts,
						debug_ctrl_ch1_gap_cnts,
						debug_heat_mode_1, 
						debug_heat_tool_type_1,
						debug_temp_zero_time_1,
						debug_temp_wait_time_1,
						debug_measure_time_1, 
						debug_ctrl_frst_time_1, 
						debug_ctrl_botm_time_1,

						debug_ctrl_ch2_num,
						debug_ctrl_ch2_contnu_cnts,
						debug_ctrl_ch2_gap_cnts,
						debug_heat_mode_2, 
						debug_heat_tool_type_2,
						debug_temp_zero_time_2,
						debug_temp_wait_time_2,
						debug_measure_time_2, 
						debug_ctrl_frst_time_2, 
						debug_ctrl_botm_time_2;


void debug_cmd_string_pop(volatile circle_queue_t *cmd_queue, char* cmd_string);

void bsp_debug_cmd_process(void);

void uart_usb_deal_rcv_debug(void);

void debug_task(void);

void debug_us_counter_timer_init(void);


#endif

#ifdef __cplusplus
}
#endif

#endif

