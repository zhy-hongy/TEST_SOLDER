#include "debug.h"

#if DEBUG_ON == 1

volatile bsp_dbg_t ch_bsp_debug;
volatile logic_dbg_t ch_logic_debug;
volatile queue_status_t queue_status;

char cmd_string_in[200];

char debug_string_out_0[1000];
char debug_string_out_1[200];

char debug_string_ctrl_0[1000];
char debug_string_ctrl_1[200];

char debug_string_int_0[1000];
char debug_string_int_1[200];

volatile float debug_detail_temp_mv[50];
volatile float debug_detail_temp[50];

#define DEBUG_TEMP_BUFFER_SIZE 100

volatile circle_queue_t debug_mv_queue;
volatile circle_queue_t debug_temperature_queue;
volatile circle_queue_t debug_filted_temp_x1_queue;
volatile circle_queue_t debug_filted_temp_x2_queue;
volatile circle_queue_t debug_filted_temp_show_queue;
volatile circle_queue_t debug_cnts_queue_1;
volatile circle_queue_t debug_cnts_queue_2;
volatile circle_queue_t debug_state_queue;

volatile float debug_mv_buffer[100];
volatile float debug_temperature_buffer[100];
volatile float debug_filted_temp_x1_buffer[100];
volatile float debug_filted_temp_x2_buffer[100];
volatile float debug_filted_temp_show_buffer[100];
volatile uint16_t debug_cnts_buffer_1[100];
volatile uint16_t debug_cnts_buffer_2[100];
volatile uint8_t debug_heat_state_buffer[100];

int debug_i;

int debug_temp_target;

int debug_ctrl_ch1_num = 0,
	debug_ctrl_ch1_contnu_cnts = 0,
	debug_ctrl_ch1_gap_cnts = 0,
	debug_heat_mode_1 = 0,
	debug_heat_tool_type_1 = 0,
	debug_temp_zero_time_1 = 0,
	debug_temp_wait_time_1 = 0,
	debug_measure_time_1 = 0,
	debug_ctrl_frst_time_1 = 0,
	debug_ctrl_botm_time_1 = 0,

	debug_ctrl_ch2_num = 0,
	debug_ctrl_ch2_contnu_cnts = 0,
	debug_ctrl_ch2_gap_cnts = 0,
	debug_heat_mode_2 = 0,
	debug_heat_tool_type_2 = 0,
	debug_temp_zero_time_2 = 0,
	debug_temp_wait_time_2 = 0,
	debug_measure_time_2 = 0,
	debug_ctrl_frst_time_2 = 0,
	debug_ctrl_botm_time_2 = 0;

void print_adc_data(void);

void uart_usb_deal_rcv_debug(void)
{
	uint32_t tmp_len;
	char str_test[100];

	__disable_irq();
	if (usb_rx_queue.data_len > 0)
	{
		tmp_len = queue_pop_all((void *)&usb_rx_queue, (uint8_t *)str_test);
		uart_usb_send_to_txbuffer((char *)str_test, tmp_len);
	}
	__enable_irq();
}

void debug_cmd_string_pop(volatile circle_queue_t *cmd_queue, char *cmd_string)
{
	uint32_t index_enter, cmd_len = 0;
	char tmp_char;

	__disable_irq();
	index_enter = 0;
	while (index_enter != cmd_queue->data_len)
	{
		queue_read_data(cmd_queue, &tmp_char, index_enter);
		if (tmp_char != '\r')
		{
			index_enter++;
			cmd_len++;
		}
		else
		{
			if (cmd_len + 1 <= sizeof(cmd_string_in))
			{
				queue_pop_array(cmd_queue, (uint8_t *)cmd_string, cmd_len + 1);
			}
			break;
		}
	}
	__enable_irq();
}

void bsp_debug_cmd_process(void)
{

	/***************  debug us test  ***************/
	if (strstr((char *)cmd_string_in, "debug us timer begin") != NULL)
	{
		ch_bsp_debug.timer_us_flag = 1;
		log_info("cmd ok\n");
	}

	if (strstr((char *)cmd_string_in, "debug us timer over") != NULL)
	{
		ch_bsp_debug.timer_us_flag = 0;
		log_info("cmd ok\n");
	}

	/***************  zero test  ***************/
	if (strstr((char *)cmd_string_in, "control timer test begin") != NULL)
	{
		ch_bsp_debug.zero_int_flag = 1;
		log_info("control timer cmd ok\n");
	}

	if (strstr((char *)cmd_string_in, "control timer test stop") != NULL)
	{
		ch_bsp_debug.zero_int_flag = 0;
		log_info("control timer cmd ok\n");
	}

	/***************  adc timer test  ***************/
	if (strstr((char *)cmd_string_in, "adc regular dma test begin") != NULL)
	{
		ch_bsp_debug.adc_regular_dma_flag = 1;
		log_info("adc regular dma test started\n");
	}
	if (strstr((char *)cmd_string_in, "adc regular dma test stop") != NULL)
	{
		ch_bsp_debug.adc_regular_dma_flag = 0;
		log_info("adc regular dma test stopped\n");
	}

	if (strstr((char *)cmd_string_in, "adc injection val print begin") != NULL)
	{
		ch_bsp_debug.adc_injection_flag = 1;
		log_info("adc injection val print started\n");
	}
	if (strstr((char *)cmd_string_in, "adc injection val print stop") != NULL)
	{
		ch_bsp_debug.adc_injection_flag = 0;
		log_info("adc injection val print stopped\n");
	}

	if (strstr((char *)cmd_string_in, "adc after calculation print begin") != NULL)
	{
		ch_bsp_debug.adc_after_calculation_flag = 1;
		log_info("adc after calculation print started\n");
	}
	if (strstr((char *)cmd_string_in, "adc after calculation print stop") != NULL)
	{
		ch_bsp_debug.adc_after_calculation_flag = 0;
		log_info("adc after calculation val print stopped\n");
	}

	if (strstr((char *)cmd_string_in, "min_temp_flag_enable_begin") != NULL)
	{
		ch_bsp_debug.min_temp_flag_enable = 1;
		log_info("min_temp_flag_enable_begin\n");
	}

	if (strstr((char *)cmd_string_in, "min_temp_flag_enable_stop") != NULL)
	{
		ch_bsp_debug.min_temp_flag_enable = 0;
		log_info("min_temp_flag_enable_stop\n");
	}

	if (strstr((char *)cmd_string_in, "temp_test_num_add") != NULL)
	{
		ch_bsp_debug.temp_test_num += 1;
		if (ch_bsp_debug.temp_test_num > 50)
			ch_bsp_debug.temp_test_num = 50;

		sprintf(debug_string_out_0,"temp_test_num=%d\n", ch_bsp_debug.temp_test_num);
		log_info(debug_string_out_0);
	}

	if (strstr((char *)cmd_string_in, "temp_test_num_sub") != NULL)
	{
		ch_bsp_debug.temp_test_num -= 1;
		if (ch_bsp_debug.temp_test_num <= 0)
		{
			ch_bsp_debug.temp_test_num = 0;
		}
		sprintf(debug_string_out_0,"temp_test_num=%d\n", ch_bsp_debug.temp_test_num);
		log_info(debug_string_out_0);
	}

	if (strstr((char *)cmd_string_in, "pwm_test_add") != NULL)
	{
		ch_bsp_debug.pwm_test_add_flag += 10;
		if (ch_bsp_debug.pwm_test_add_flag > 100)
			ch_bsp_debug.pwm_test_add_flag = 100;

		sprintf(debug_string_out_0,"pwm_test_add_flag=%d\n", ch_bsp_debug.pwm_test_add_flag);
		log_info(debug_string_out_0);
	}

	if (strstr((char *)cmd_string_in, "pwm_test_sub") != NULL)
	{
		ch_bsp_debug.pwm_test_add_flag -= 10;
		if (ch_bsp_debug.pwm_test_add_flag <= 0)
		{
			ch_bsp_debug.pwm_test_add_flag = 0;
		}

		sprintf(debug_string_out_0,"pwm_test_add_flag=%d\n", ch_bsp_debug.pwm_test_add_flag);
		log_info(debug_string_out_0);
	}

	if (strstr((char *)cmd_string_in, "mux sw1 set") != NULL)
	{
		ch_bsp_debug.mux_sw1_set_flag = 1;
		MUX1_SET;
		log_info("mux_sw1=1\n");
	}

	if (strstr((char *)cmd_string_in, "mux sw1 reset") != NULL)
	{
		ch_bsp_debug.mux_sw1_set_flag = 0;
		MUX1_RESET;
		log_info("mux_sw1=0\n");
	}

	if (strstr((char *)cmd_string_in, "mux sw2 set") != NULL)
	{
		ch_bsp_debug.mux_sw2_set_flag = 1;
		MUX2_SET;
		log_info("mux_sw2=1\n");
	}

	if (strstr((char *)cmd_string_in, "mux sw2 reset") != NULL)
	{
		ch_bsp_debug.mux_sw2_set_flag = 0;
		MUX2_RESET;
		log_info("mux_sw2=0\n");
	}

	if (strstr((char *)cmd_string_in, "heat once") != NULL)
	{
		ch_bsp_debug.heat_once_flag = 1;
		//				log_info("receive cmd heat once\n");
	}
}

void logic_debug_cmd_process(void)
{
	/***************  stage test  ***************/
	if (strstr((char *)cmd_string_in, "stage test start") != NULL)
	{
		ch_logic_debug.stage_flag = 1;
		if (strstr((char *)cmd_string_in, "channel 2") != NULL)
			ch_logic_debug.stage_flag = 1;

		sprintf(debug_string_out_0, "start stage ouput channel %d\n", ch_logic_debug.stage_flag);
		uart_usb_send_to_txbuffer((void *)debug_string_out_0, strlen(debug_string_out_0));
	}

	if (strstr((char *)cmd_string_in, "stage test stop") != NULL)
	{
		ch_logic_debug.stage_flag = 0;
		strcpy(debug_string_out_0, "stop stage output\n");
	}

	/***************  control process test  **************/
	if (strstr((char *)cmd_string_in, "control process begin") != NULL)
	{
		ch_logic_debug.ctrl_process_flag = 1;
		log_info("\ncontrol process begin");
	}
	if (strstr((char *)cmd_string_in, "control process stop") != NULL)
	{
		ch_logic_debug.ctrl_process_flag = 0;
		log_info("\ncontrol process stop");
	}

	/***************  temperature detail test  **************/
	if (strstr((char *)cmd_string_in, "detail temperature test begin") != NULL)
	{
		ch_logic_debug.adc_temp_detail_flag = 1;

		log_info("temperature test started\n");
	}

	if (strstr((char *)cmd_string_in, "detail temperature test stop") != NULL)
	{
		ch_logic_debug.adc_temp_detail_flag = 0;
		log_info("adc dma flag test stopped\n");
	}

	/***************  temperature  test  **************/
	if (strstr((char *)cmd_string_in, "temperature array output begin") != NULL)
	{
		ch_logic_debug.temp_filt_array_flag = 1;
		queue_init(&debug_mv_queue, (void *)debug_mv_buffer, 100, 4);
		queue_init(&debug_temperature_queue, (void *)debug_temperature_buffer, 100, 4);
		queue_init(&debug_filted_temp_x1_queue, (void *)debug_filted_temp_x1_buffer, 100, 4);
		queue_init(&debug_filted_temp_x2_queue, (void *)debug_filted_temp_x2_buffer, 100, 4);
		queue_init(&debug_filted_temp_show_queue, (void *)debug_filted_temp_show_buffer, 100, 4);
		queue_init(&debug_cnts_queue_1, (void *)&debug_cnts_buffer_1, 100, 2);
		queue_init(&debug_cnts_queue_2, (void *)&debug_cnts_buffer_2, 100, 2);
		queue_init(&debug_state_queue, (void *)&debug_heat_state_buffer, 100, 1);

		//			log_info("temperature array output started\n");
	}

	if (strstr((char *)cmd_string_in, "temperature array output stop") != NULL)
	{
		ch_logic_debug.temp_filt_array_flag = 0;
		log_info("temperature array output stopped\n");
	}
	/***************  voltage and current test  **************/
	if (strstr((char *)cmd_string_in, "vol cur test begin") != NULL)
	{
		ch_logic_debug.voltage_current_flag = 1;
		log_info("vol cur output started\n");
	}
	if (strstr((char *)cmd_string_in, "vol cur test stop") != NULL)
	{
		ch_logic_debug.voltage_current_flag = 0;
		log_info("vol cur output stopped\n");
	}

	/***************  error logic test  **************/
	if (strstr((char *)cmd_string_in, "error test start") != NULL)
	{
		ch_logic_debug.error_flag = 1;
		log_info("error test ouput\n");
	}

	if (strstr((char *)cmd_string_in, "error test stop") != NULL)
	{
		ch_logic_debug.error_flag = 0;
		log_info("stop error test\n");
	}

	if (strstr((char *)cmd_string_in, "target temp") != NULL)
	{
		ch_logic_debug.temp_target_flag = 1;
		sscanf((char *)cmd_string_in, "%*s%*s%d", &debug_temp_target);

		//			sprintf(debug_string_out_0, "target %d\n", debug_temp_target);
		//			log_info(debug_string_out_0);
	}

	/***************  logic control test  **************/
	if (strstr((char *)cmd_string_in, "test ctrl start") != NULL)
	{
		sscanf((char *)cmd_string_in, "%*s%*s%*s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d  %d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			   &debug_ctrl_ch1_num,
			   &debug_heat_mode_1,
			   &debug_heat_tool_type_1,
			   &debug_ctrl_ch1_contnu_cnts,
			   &debug_ctrl_ch1_gap_cnts,
			   &debug_temp_zero_time_1,
			   &debug_temp_wait_time_1,
			   &debug_measure_time_1,
			   &debug_ctrl_frst_time_1,
			   &debug_ctrl_botm_time_1,

			   &debug_ctrl_ch2_num,
			   &debug_heat_mode_2,
			   &debug_heat_tool_type_2,
			   &debug_ctrl_ch2_contnu_cnts,
			   &debug_ctrl_ch2_gap_cnts,
			   &debug_temp_zero_time_2,
			   &debug_temp_wait_time_2,
			   &debug_measure_time_2,
			   &debug_ctrl_frst_time_2,
			   &debug_ctrl_botm_time_2);
		if (debug_heat_mode_1 < 4 || debug_heat_mode_1 > 5 ||
			debug_heat_mode_2 < 4 || debug_heat_mode_2 > 5 ||
			debug_ctrl_ch1_num > 100 || debug_ctrl_ch2_num > 100 ||
			debug_temp_wait_time_1 > 1000 || debug_temp_wait_time_2 > 1000 ||
			debug_measure_time_1 > 1000 || debug_measure_time_2 > 1000)
		{
			sprintf(debug_string_out_0, "test ctrl parameter error %d %d %d %d\n",
					debug_ctrl_ch1_num,
					debug_ctrl_ch2_num,
					debug_heat_mode_1,
					debug_heat_mode_2);
		}
		else
		{
			//						debug_ctrl_normal_flag = 0;
			//						debug_ctrl_ch1_flag = debug_ctrl_ch2_flag = 1;
			sprintf(debug_string_out_0, "test ctrl start, parameters: %d %d %d, %d %d %d\n",
					debug_ctrl_ch1_num, debug_heat_mode_1, debug_heat_tool_type_1,
					debug_ctrl_ch2_num, debug_heat_mode_2, debug_heat_tool_type_2);


			//						solder_ch1.heat_mode = (heat_mode_t)debug_heat_mode_1;
			//						solder_ch2.heat_mode = (heat_mode_t)debug_heat_mode_2;
			//
			//						solder_ch1.heat_test_cnts = debug_ctrl_ch1_num + 1;
			//						solder_ch2.heat_test_cnts = debug_ctrl_ch2_num + 1;

			//						solder_ch1.tool_type 			= (tool_type_t)debug_heat_tool_type_1;
			//						solder_ch2.tool_type 			= (tool_type_t)debug_heat_tool_type_2;

			//						solder_ch1.heat_contnu_max_cnts 			= 0;
			//						solder_ch2.heat_contnu_max_cnts 			= 0;
			//						solder_ch1.heat_gap_cnts				 			= 0;
			//						solder_ch2.heat_gap_cnts				 			= 0;

			//						solder_state.zero_wait_time				= debug_temp_zero_time_1;
			//						solder_state.temp_wait_time 			=	debug_temp_wait_time_1;
			//						solder_state.measure_time 				=	debug_measure_time_1;
			//						solder_state.heat_time_frst_half 	=	debug_ctrl_frst_time_1;
			//						solder_state.heat_time_botm_half 	=	debug_ctrl_botm_time_1;

			//						solder_ch1.zero_wait_time				= debug_temp_zero_time_1;
			//						solder_ch1.temp_wait_time 			=	debug_temp_wait_time_1;
			//						solder_ch1.measure_time 				=	debug_measure_time_1;
			//						solder_ch1.heat_time_frst_half 	=	debug_ctrl_frst_time_1;
			//						solder_ch1.heat_time_botm_half 	=	debug_ctrl_botm_time_1;

			//						solder_ch2.zero_wait_time 			=	debug_temp_zero_time_2;
			//						solder_ch2.temp_wait_time 			=	debug_temp_wait_time_2;
			//						solder_ch2.measure_time					=	debug_measure_time_2;
			//						solder_ch2.heat_time_frst_half 	=	debug_ctrl_frst_time_2;
			//						solder_ch2.heat_time_botm_half	= debug_ctrl_botm_time_2;

			//						if(debug_heat_tool_type_1 == tool_210){
			//							mux_select(1, mux_210_temp);
			//						}else
			//						if(debug_heat_tool_type_1 == tool_245){
			//							mux_select(1, mux_245_temp);
			//						}
			//
			//						if(debug_heat_tool_type_2 == tool_210){
			//							mux_select(2, mux_210_temp);
			//						}else
			//						if(debug_heat_tool_type_2 == tool_245){
			//							mux_select(2, mux_245_temp);
			//						}
		}

		//					if (packet_sent == 1)
		//						CDC_Send_DATA ((unsigned char*)debug_string_out_0, strlen(debug_string_out_0));
		//					Receive_length = 0;
	}

	if (strstr((char *)cmd_string_in, "test ctrl stop") != NULL)
	{
		strcpy(debug_string_out_0, "test ctrl stopped");
	}
}

void debug_print(void)
{
	uint16_t i, data_length;
	float temp_data;
	uint8_t state_flag_tmp;
	uint16_t cnt_tmp;

#if DEBUG_CMD_CTROL == 1 && DEBUG_TEMP_ARRAY_OUTPUT == 1
	if (ch_logic_debug.temp_filt_array_flag == 1)
	{
		data_length = 20;
		if (debug_filted_temp_x2_queue.data_len > data_length)
		{
			sprintf(debug_string_out_0, "length: %d, temp_cur:\t", data_length);
			for (int i = 0; i < data_length; i++)
			{
				queue_pop(&debug_temperature_queue, &temp_data);
				sprintf(debug_string_out_1, " %5.4f", (float)temp_data);
				strcat(debug_string_out_0, debug_string_out_1);
			}
			strcat(debug_string_out_0, "\n");
			log_info(debug_string_out_0);

			sprintf(debug_string_out_0, "length: %d, temp_filt:\t", data_length);
			for (int i = 0; i < data_length; i++)
			{
				queue_pop(&debug_filted_temp_x1_queue, &temp_data);
				sprintf(debug_string_out_1, " %5.4f", (float)temp_data);
				strcat(debug_string_out_0, debug_string_out_1);
			}
			strcat(debug_string_out_0, "\n");
			log_info(debug_string_out_0);

			sprintf(debug_string_out_0, "length: %d, temp_est:\t", data_length);
			for (int i = 0; i < data_length; i++)
			{
				queue_pop(&debug_filted_temp_x2_queue, &temp_data);
				sprintf(debug_string_out_1, " %5.4f", (float)temp_data);
				strcat(debug_string_out_0, debug_string_out_1);
			}
			strcat(debug_string_out_0, "\n");
			log_info(debug_string_out_0);

			//					sprintf(debug_string_out_0, "length: %d, temp_show:\t", data_length);
			//					for(int i = 0; i < data_length; i++){
			//						queue_pop(&debug_filted_temp_show_queue, &temp_data);
			//						sprintf(debug_string_out_1, " %5.2f", (float)temp_data);
			//						strcat(debug_string_out_0, debug_string_out_1);
			//					}
			//					strcat(debug_string_out_0, "\n");
			//					log_info(debug_string_out_0);

			sprintf(debug_string_out_0, "length: %d, heat_state:\t", data_length);
			for (int i = 0; i < data_length; i++)
			{
				queue_pop(&debug_state_queue, &state_flag_tmp);
				sprintf(debug_string_out_1, " %6d", state_flag_tmp);
				strcat(debug_string_out_0, debug_string_out_1);
			}
			strcat(debug_string_out_0, "\n");
			log_info(debug_string_out_0);

			//					sprintf(debug_string_out_0, "length: %d, heat_state:\t", data_length);
			//					for(int i = 0; i < data_length; i++){
			//						queue_pop(&debug_cnts_queue_1, &cnt_tmp);
			//						sprintf(debug_string_out_1, " %5d", cnt_tmp);
			//						strcat(debug_string_out_0, debug_string_out_1);
			//					}
			//					strcat(debug_string_out_0, "\n");
			//					log_info(debug_string_out_0);

			//					sprintf(debug_string_out_0, "length: %d, heat_state:\t", data_length);
			//					for(int i = 0; i < data_length; i++){
			//						queue_pop(&debug_cnts_queue_2, &cnt_tmp);
			//						sprintf(debug_string_out_1, " %5d", cnt_tmp);
			//						strcat(debug_string_out_0, debug_string_out_1);
			//					}
			//					strcat(debug_string_out_0, "\n");
			//					log_info(debug_string_out_0);
		}
	}
#endif
}

void debug_task(void)
{
#if DEBUG_CMD_CTROL == 1
	{
		debug_cmd_string_pop((void *)&usb_rx_queue, cmd_string_in);

		bsp_debug_cmd_process();
		logic_debug_cmd_process();

		debug_print();
		memset(cmd_string_in, 0, sizeof(cmd_string_in));
	}
#endif
}

#endif