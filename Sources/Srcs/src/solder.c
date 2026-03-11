#define CONFIG_DEBUG_ON
#include "solder.h"

#include "temperature_cal.h"

// #define PARAM_WRITE
solder_param_info_t solder_param_info[512];

uint8_t solder_param[512];
static uint16_t ch_err_wait_cnt = 0;
volatile uint32_t zero_up_irq_tim_record = 0, zero_down_irq_tim_record = 0;

volatile key_ntf_t key_ntf_cmd;

void heat_temperature_error(volatile solder_ch_state_t *ch);
static void zero_cross_irq_handler(void);
// static uint32_t systemtime_last = 0;

uint32_t zero_cross_times[10]; // 记录十次过零检测时间计数的间隔值
volatile uint8_t zero_times = 0;

float calibration_target_temp_transfer = 0; // 进入校准模式，临时存储目标温度
float min_target_temp_transfer = 0;			// 进入睡眠最低温度模式，临时存储目标温度
float temp_target_temp_transfer = 0;		// 临时存储最低温度睡眠，目标温度的改变量

uint8_t key_state;
uint8_t enc_key_old[32] = {0};
uint8_t enc_key[32] = {00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
					   00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
					   00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
					   00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
uint8_t enc_rand[32] = {1, 1, 1, 1};


volatile solder_state_t solder = {
	.init_over = 0,

	.err_flag = ERROR_NONE,

	.disp_maintain_state = 0,
	.disp_flicker_cnt = 20,

	.ctrl_state = ctrl_ideal,

	.zero_wait_time = 680,
	.temp_wait_time = CH_WAIT_TIME,
	.measure_time = CH_MEASURE_TIME

#ifdef THT_L075
	,
	.heat_pwm_ctrl_total_cycle_cnt = 0,
	.heat_pwm_ctrl_low_period = CH_CTRL_PWM_OUTPUT_TIM_LOAD,
	.heat_pwm_ctrl_on_off_flag = 0,
	.voltage_duty_now = 50,
#endif
};

volatile solder_ch_state_t solder_channel = {
	.channel = 1,
	.ch_temp_cal_param_udpate_ntf = 0,
	.ch_stage = ch_ideal_notool,
	.ch_last_stage = ch_ideal_notool,

	.ch_stage_frstin_flag = 1,
	.tool_state_update_sw = CH_STATE_SW_OFF,

	.ch_set_timeout_max = 3000, // 设置模式超时最大值，默认30秒（3000个10ms周期）
	.type_test_avg_res = 0,

	.ch_err_flag = ERROR_NONE,

	.ch_temp_error_continue_cnt = 0,
	.ch_temp_error_continue_gap_cnt = 0,

	.ctrl_temp_ok_first_ntf = 0,
	.temp_reset_ntf = 0,

	.ctrl_state = ctrl_ideal,
	.temp_current = 0,
	.temp_target = 300,
	.temp_show = 0,

	.heat_state = heat_off,

	.heat_mode = heat_none,
	.heat_time_frst_half = 0,
	.heat_time_botm_half = 0,
	.power_AC_HZ_flag = 1,
	.power_buf_index = 0,
	.power_sum = 0};
	

/************* 基本初始化的方法 ***************/
void solder_dev_init()
{
	uint8_t i;
	///	ch_logic_debug.stage_flag = 1;
	bsp_init();

	for (i = 0; i < KEY_MAX; i++)
	{
		clear_key_notify(i);
	}

	delay_ms(500);
	mux_select(mux_reset);
#ifdef PARAM_WRITE

	if (get_key_status(&key_state))
	{
		if (key_state == 00)
		{
			//			log_info("key state debug mode\n");
		}
		else
		{
			//			log_info("key state safe mode\n");
		}
	}
	else
	{
		//		log_info("key_state error\n");
	}

	if (Set_Key(0, enc_key, enc_key_old))
	{
		//		log_info("set key ok\n");
	}
	else
	{
		//		log_info("set key error\n");
	}

#endif
	// solder_param_load_temp_calibration_from_rom();
	screen_display_test();
	frequency_check();
	memset(enc_rand, 1, 32);
	if (Authenticate(0, enc_key, enc_rand) == SUCCESS_WORK)
	{
#if DEBUG_STAGE_TEST == 1
		log_info("entryption ok\n");
#endif
		solder_param_init();
		init_all_temp_tab();
		solder.dev_init_over = 1;
	}
	else
	{
#if DEBUG_STAGE_TEST == 1
		log_info("entryption error\n");
#endif
	}

	solder.init_over = 1;
}

void solder_param_init()
{
	float tmp_in = 320;
	float tmp_out = 0;
	solder_param_info[param_target_temp].param_loc = 0;
	solder_param_info[param_target_temp].param_length = 4; // 目标温度
	solder_param_info[param_target_temp_inc].param_loc = 4;
	solder_param_info[param_target_temp_inc].param_length = 4; // 加减温度
	solder_param_info[param_target_temp_ch1].param_loc = 8;
	solder_param_info[param_target_temp_ch1].param_length = 4;
	solder_param_info[param_target_temp_ch2].param_loc = 12;
	solder_param_info[param_target_temp_ch2].param_length = 4;
	solder_param_info[param_target_temp_ch3].param_loc = 16;
	solder_param_info[param_target_temp_ch3].param_length = 4;
	solder_param_info[param_control_power_high].param_loc = 20;
	solder_param_info[param_control_power_high].param_length = 1; // 高功率模式
	solder_param_info[param_temp_cal_115_flag].param_loc = 22;
	solder_param_info[param_temp_cal_115_flag].param_length = 2; // 校准模式
	solder_param_info[param_temp_cal_115_100].param_loc = 24;
	solder_param_info[param_temp_cal_115_100].param_length = 4;
	solder_param_info[param_temp_cal_115_200].param_loc = 28;
	solder_param_info[param_temp_cal_115_200].param_length = 4;
	solder_param_info[param_temp_cal_115_300].param_loc = 32;
	solder_param_info[param_temp_cal_115_300].param_length = 4;
	solder_param_info[param_temp_cal_115_400].param_loc = 36;
	solder_param_info[param_temp_cal_115_400].param_length = 4;
	solder_param_info[param_temp_cal_210_flag].param_loc = 40;
	solder_param_info[param_temp_cal_210_flag].param_length = 2;
	solder_param_info[param_temp_cal_210_100].param_loc = 42;
	solder_param_info[param_temp_cal_210_100].param_length = 4;
	solder_param_info[param_temp_cal_210_200].param_loc = 46;
	solder_param_info[param_temp_cal_210_200].param_length = 4;
	solder_param_info[param_temp_cal_210_300].param_loc = 50;
	solder_param_info[param_temp_cal_210_300].param_length = 4;
	solder_param_info[param_temp_cal_210_400].param_loc = 54;
	solder_param_info[param_temp_cal_210_400].param_length = 4;
	solder_param_info[param_temp_cal_245_flag].param_loc = 58;
	solder_param_info[param_temp_cal_245_flag].param_length = 2;
	solder_param_info[param_temp_cal_245_100].param_loc = 60;
	solder_param_info[param_temp_cal_245_100].param_length = 4;
	solder_param_info[param_temp_cal_245_200].param_loc = 64;
	solder_param_info[param_temp_cal_245_200].param_length = 4;
	solder_param_info[param_temp_cal_245_300].param_loc = 68;
	solder_param_info[param_temp_cal_245_300].param_length = 4;
	solder_param_info[param_temp_cal_245_400].param_loc = 72;
	solder_param_info[param_temp_cal_245_400].param_length = 4;
	solder_param_info[param_temp_cal_470_flag].param_loc = 78;
	solder_param_info[param_temp_cal_470_flag].param_length = 2;
	solder_param_info[param_temp_cal_470_100].param_loc = 80;
	solder_param_info[param_temp_cal_470_100].param_length = 4;
	solder_param_info[param_temp_cal_470_200].param_loc = 84;
	solder_param_info[param_temp_cal_470_200].param_length = 4;
	solder_param_info[param_temp_cal_470_300].param_loc = 88;
	solder_param_info[param_temp_cal_470_300].param_length = 4;
	solder_param_info[param_temp_cal_470_400].param_loc = 92;
	solder_param_info[param_temp_cal_470_400].param_length = 4;// 休眠最低温度和设置flag
	solder_param_info[param_min_temp].param_loc = 96;
	solder_param_info[param_min_temp].param_length = 4;
	solder_param_info[param_min_temp_flag].param_loc = 100;
	solder_param_info[param_min_temp_flag].param_length = 2;

#ifdef PARAM_WRITE
	tmp_in = 320;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_target_temp);
	solder_param_write_to_rom(param_target_temp);
	tmp_in = 1;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_target_temp_inc);
	solder_param_write_to_rom(param_target_temp_inc);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_min_temp_flag);
	solder_param_write_to_rom(param_min_temp_flag);
	tmp_in = 300;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_target_temp_ch1);
	solder_param_write_to_rom(param_target_temp_ch1);
	tmp_in = 350;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_target_temp_ch2);
	solder_param_write_to_rom(param_target_temp_ch2);
	tmp_in = 400;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_target_temp_ch3);
	solder_param_write_to_rom(param_target_temp_ch3);
	tmp_in = 0;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_115_flag);
	solder_param_write_to_rom(param_temp_cal_115_flag);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_210_flag);
	solder_param_write_to_rom(param_temp_cal_210_flag);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_245_flag);
	solder_param_write_to_rom(param_temp_cal_245_flag);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_470_flag);
	solder_param_write_to_rom(param_temp_cal_470_flag);
	tmp_in = 100;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_115_100);
	solder_param_write_to_rom(param_temp_cal_115_100);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_210_100);
	solder_param_write_to_rom(param_temp_cal_210_100);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_245_100);
	solder_param_write_to_rom(param_temp_cal_245_100);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_470_100);
	solder_param_write_to_rom(param_temp_cal_470_100);
	tmp_in = 200;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_115_200);
	solder_param_write_to_rom(param_temp_cal_115_200);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_210_200);
	solder_param_write_to_rom(param_temp_cal_210_200);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_245_200);
	solder_param_write_to_rom(param_temp_cal_245_200);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_470_200);
	solder_param_write_to_rom(param_temp_cal_470_200);
	tmp_in = 300;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_115_300);
	solder_param_write_to_rom(param_temp_cal_115_300);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_210_300);
	solder_param_write_to_rom(param_temp_cal_210_300);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_245_300);
	solder_param_write_to_rom(param_temp_cal_245_300);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_470_300);
	solder_param_write_to_rom(param_temp_cal_470_300);
	tmp_in = 400;
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_115_400);
	solder_param_write_to_rom(param_temp_cal_115_400);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_210_400);
	solder_param_write_to_rom(param_temp_cal_210_400);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_245_400);
	solder_param_write_to_rom(param_temp_cal_245_400);
	solder_param_write_to_array((uint8_t *)&tmp_in, param_temp_cal_470_400);
	solder_param_write_to_rom(param_temp_cal_470_400);
#endif
#ifdef DEBUG_PARAM_OUTPUT
	memset(debug_string_out_0, 0, 300);
	log_info("param init ok\n");

	for (uint8_t i = 0; i < 100; i++)
	{
		sprintf(debug_string_out_1, "%02X ", solder_param[i]);
		strcat(debug_string_out_0, debug_string_out_1);
	}

	strcat(debug_string_out_0, "\n");
	log_info(debug_string_out_0);
#endif
	solder_param_load_all_from_rom();
	solder_param_get_from_array((void *)&solder_channel.temp_target,
								param_target_temp);
	ctrl_params_update_from_target_temp(&solder_channel.controller,
										solder_channel.temp_target);
	solder_param_get_from_array((void *)&solder_channel.temp_target_inc,
								param_target_temp_inc);
	solder_param_get_from_array((void *)&solder_channel.power_high_flag,
								param_control_power_high);
	solder_param_get_from_array((void *)&solder_channel.min_temp,
								param_min_temp);
	solder_param_get_from_array((void *)&solder_channel.min_temp_enable_flag,
								param_min_temp_flag);

}
#ifdef THT_L075

void adjust_power_voltage(float voltage)
{
	float duty = (0.5f - 1.0f) / (16 - 10) * (voltage - 10) + 1.0f;
	solder.voltage_duty_goal = duty * 100;
}
void adjust_power_voltage_poll()
{
	// uint16_t duty_CC_cnt;

	if (solder.voltage_duty_goal != solder.voltage_duty_now)
	{
		if (solder.voltage_duty_goal > solder.voltage_duty_now)
		{
			solder.voltage_duty_now++;
		}   
		else
		{
			solder.voltage_duty_now--;
		}

		TIM_SetCmp3(POWER_CTRL_PWM_TIM, solder.voltage_duty_now);
		
		// TIM_ConfigOc3Preload(POWER_CTRL_PWM_TIM, duty_CC_cnt);
	}
#ifdef DEBUG_CMD_CTROL

	if (ch_bsp_debug.pwm_test_add_flag > 0)
	{
		TIM_SetCmp3(POWER_CTRL_PWM_TIM, ch_bsp_debug.pwm_test_add_flag);
	}

#endif
}
#endif
/*
根据过零中断里面存储的每次过零时间间隔计算频率
根据频率的不同设置不同的加热方式和不同的过零等待时间
*/
void frequency_check(void)
{
	//	delay_ms(100);
	uint8_t zero_freq_stable = 0;
	int32_t sub_val_tmp;

	while (1)
	{
		if (zero_times == 9)
		{
			zero_freq_stable = 1;

			for (uint8_t i = 0; i < 9; i++)
			{
				sub_val_tmp = zero_cross_times[i + 1] - zero_cross_times[i];

				if (sub_val_tmp > 1 || sub_val_tmp < -1)
				{
					zero_freq_stable = 0;
				}
			}
		}
		if (zero_times == 9 && zero_freq_stable == 1)
		{
			float sum = 0;

			for (uint8_t i = 0; i < 10; i++)
			{
				sum += zero_cross_times[i];
			}

			sum = sum / 10;
#if DEBUG_ON == 1
			memset(debug_string_out_0, 0, 100);

			for (uint8_t i = 0; i < 10; i++)
			{
				sprintf(debug_string_out_1, "%d ", zero_cross_times[i]);
				strcat(debug_string_out_0, debug_string_out_1);
			}

			log_info("frequency debug, zero_cross:\n");
			log_info(debug_string_out_0);
			log_info("\n");
			sprintf(debug_string_out_0, "interval: %f\n", sum);
			log_info(debug_string_out_0);
#endif
			solder.zero_wait_time = 600;

			if (sum > 9 && sum < 11)
			{
				solder_channel.power_AC_HZ_flag = 1;
				solder.zero_wait_time = 680;
#if DEBUG_ON == 1
				log_info("^^^^^^^^50HZ\r\n");
#endif
			}

			if (sum > 7 && sum < 9)
			{
				solder_channel.power_AC_HZ_flag = 0;
				solder.zero_wait_time = 480;
#if DEBUG_ON == 1
				log_info("^^^^^^60HZ\r\n");
#endif
			}

			break;
		}
	}
}
void solder_param_load_all_from_rom()
{
	Read_Data_Protected(enc_rand, enc_key, 0, 102, solder_param);
#ifdef DEBUG_PARAM_OUTPUT
	log_info("param load from rom:\n");

	for (uint8_t i = 0; i < 100; i++)
	{
		sprintf(debug_string_out_1, "%02X ", solder_param[i]);
		strcat(debug_string_out_0, debug_string_out_1);
	}

	strcat(debug_string_out_0, "\n");
	log_info(debug_string_out_0);
#endif
}
void solder_param_load_dev_init_from_rom()
{
	solder_param_load_from_rom(param_target_temp);
	solder_param_load_from_rom(param_target_temp_inc);
	solder_param_load_from_rom(param_target_temp_ch1);
	solder_param_load_from_rom(param_target_temp_ch2);
	solder_param_load_from_rom(param_target_temp_ch3);
	solder_param_load_from_rom(param_control_power_high);
}
void solder_param_load_temp_calibration_from_rom()
{
	solder_param_load_from_rom(param_temp_cal_115_flag);
	solder_param_load_from_rom(param_temp_cal_115_100);
	solder_param_load_from_rom(param_temp_cal_115_200);
	solder_param_load_from_rom(param_temp_cal_115_300);
	solder_param_load_from_rom(param_temp_cal_115_400);
	solder_param_load_from_rom(param_temp_cal_210_flag);
	solder_param_load_from_rom(param_temp_cal_210_100);
	solder_param_load_from_rom(param_temp_cal_210_200);
	solder_param_load_from_rom(param_temp_cal_210_300);
	solder_param_load_from_rom(param_temp_cal_210_400);
	solder_param_load_from_rom(param_temp_cal_245_flag);
	solder_param_load_from_rom(param_temp_cal_245_100);
	solder_param_load_from_rom(param_temp_cal_245_200);
	solder_param_load_from_rom(param_temp_cal_245_300);
	solder_param_load_from_rom(param_temp_cal_245_400);
	//
	//	solder_param_load_from_rom(param_temp_cal_470_flag);
	//	solder_param_load_from_rom(param_temp_cal_470_100);
	//	solder_param_load_from_rom(param_temp_cal_470_200);
	//	solder_param_load_from_rom(param_temp_cal_470_300);
	//	solder_param_load_from_rom(param_temp_cal_470_400);
#ifdef DEBUG_PARAM_OUTPUT
	log_info("param load from rom:\n");

	for (uint8_t i = 0; i < 100; i++)
	{
		sprintf(debug_string_out_1, "%02X ", solder_param[i]);
		strcat(debug_string_out_0, debug_string_out_1);
	}

	strcat(debug_string_out_0, "\n");
	log_info(debug_string_out_0);
#endif
}


inline void solder_param_write_to_rom(param_name_t name)
{
	Set_Parameter(solder_param_info[name].param_loc,
				  solder_param_info[name].param_length,
				  solder_param + solder_param_info[name].param_loc, enc_key);
#ifdef DEBUG_PARAM_OUTPUT
	sprintf(debug_string_out_0, "write success %f\n",
			*(float *)(solder_param + solder_param_info[name].param_loc));
	log_info(debug_string_out_0);
#endif
}

inline void solder_param_load_from_rom(param_name_t name)
{
	if (Read_Data_Protected(enc_key, enc_key, solder_param_info[name].param_loc,
							solder_param_info[name].param_length,
							solder_param + solder_param_info[name].param_loc))
	{
#ifdef DEBUG_PARAM_OUTPUT
		log_info("read success\n");
#endif
	}
}
void solder_param_save(void *params, param_name_t name)
{
	uint16_t i;
	uint8_t *p = (uint8_t *)params;

	for (i = 0; i < solder_param_info[name].param_length; i++)
	{
		solder_param[solder_param_info[name].param_loc + i] = p[i];
	}

	Set_Parameter(solder_param_info[name].param_loc,
				  solder_param_info[name].param_length,
				  solder_param + solder_param_info[name].param_loc, enc_key);
}

void solder_param_get(void *params, param_name_t name)
{
	uint16_t i;
	uint8_t *p = (uint8_t *)params;
	Read_Data_Protected(enc_key, enc_key, solder_param_info[name].param_loc,
						solder_param_info[name].param_length,
						solder_param + solder_param_info[name].param_loc);

	for (i = 0; i < solder_param_info[name].param_length; i++)
	{
		p[i] = solder_param[solder_param_info[name].param_loc + i];
	}
}

void solder_param_write_to_array(void *params, param_name_t name)
{
	uint16_t i;
	uint8_t *p = (uint8_t *)params;

	for (i = 0; i < solder_param_info[name].param_length; i++)
	{
		solder_param[solder_param_info[name].param_loc + i] = p[i];
	}

	// memcpy(param, solder_param + solder_param_info[name].param_loc,
	// solder_param_info[name].param_length);
}

void solder_param_get_from_array(void *params, param_name_t name)
{
	uint16_t i;
	uint8_t *p = (uint8_t *)params;

	for (i = 0; i < solder_param_info[name].param_length; i++)
	{
		p[i] = solder_param[solder_param_info[name].param_loc + i];
	}

	// memcpy(param, solder_param + solder_param_info[name].param_loc,
	// solder_param_info[name].param_length);
}

void solder_parameters_update_to_rom()
{
	if (solder_channel.ch_temp_param_update_ntf)
	{
		solder_param_write_to_rom(param_target_temp);
		solder_param_write_to_rom(param_target_temp_inc);
		solder_param_write_to_rom(param_target_temp_ch1);
		solder_param_write_to_rom(param_target_temp_ch2);
		solder_param_write_to_rom(param_target_temp_ch3);
		solder_param_write_to_rom(param_min_temp);
		solder_param_write_to_rom(param_min_temp_flag);

		solder_channel.ch_temp_param_update_ntf = 0;
#ifdef DEBUG_PARAM_OUTPUT
		log_info("solder temp param save\n");
#endif
	}
	if (solder_channel.ch_power_param_udpate_ntf)
	{
		solder_param_write_to_rom(param_control_power_high);
		solder_channel.ch_power_param_udpate_ntf = 0;
#ifdef DEBUG_PARAM_OUTPUT
		log_info("power high flag write to rom");
#endif
	}

	if (solder_channel.ch_temp_cal_param_udpate_ntf)
	{
		calibration_param_to_rom(solder_channel.tool_type); // 把参数存储到rom里面
		solder_channel.ch_temp_cal_param_udpate_ntf = 0;
	}
}

/**
 * @brief 保存最小温度设置并启用最小温度功能
 * @param ch 焊台通道状态结构体指针
 * @note 该函数将设置的温度值保存到参数数组中，并启用最小温度功能
 */
void save_min_temp_settings(volatile solder_ch_state_t *ch)
{
	// 将设置的温度值保存到参数数组
	solder_param_write_to_array((void *)&ch->ch_set_temp_disp, param_min_temp);
	// 从参数数组中读取最小温度值
	solder_param_get_from_array((void *)&ch->min_temp, param_min_temp);
	// 保存最小温度启用标志到参数数组
	solder_param_write_to_array((void *)&ch->min_temp_enable_flag, param_min_temp_flag);
	// 从参数数组中读取最小温度启用标志
	solder_param_get_from_array((void *)&ch->min_temp_enable_flag, param_min_temp_flag);
}
void solder_state_init(solder_state_t *solder) {}

void solder_channel_state_init(solder_ch_state_t *solder_channel) {}
void power_gear_cal(volatile solder_ch_state_t *ch, uint8_t heat_flag) {}

void power_cal(volatile solder_ch_state_t *ch, uint8_t heat_flag)
{
	if (heat_flag > 0 && ch->power_sum < POWER_SUM_LEN)
	{
		ch->power_sum++;
	}

	if (ch->power_rec_buf[ch->power_buf_index] > 0 && ch->power_sum > 0)
	{
		ch->power_sum--;
	}

	//	DELAY_EXECUTE(500, {
	//		sprintf(debug_string_out_0, "power_sum: %2d\r\n", ch->power_sum);
	//		log_info(debug_string_out_0); });
	ch->power_rec_buf[ch->power_buf_index] = heat_flag;
	ch->power_buf_index++;

	if (ch->power_buf_index >= POWER_SUM_LEN)
	{
		ch->power_buf_index = 0;
	}

	if (ch->power_sum > 0)
	{
		if (ch->power_sum < 2)
		{
			ch->power_gear = 1;
		}
		else
		{
			ch->power_gear = ch->power_sum / 3;
		}

		if (ch->power_gear > 8)
		{
			ch->power_gear = 8;
		}
	}
	else
	{
		ch->power_gear = 0;
	}

	if (ch->power_sum > 30)
	{
		ch->power_exceed_cnts++;
	}
	else
	{
		ch->power_exceed_gap_cnts++;
	}

	if (ch->power_exceed_cnts > 10000 || ch->power_exceed_gap_cnts > 10000)
	{
		if (ch->power_exceed_gap_cnts < 600)
		{
			// 2 min 停止加热
			ch->ch_err_flag |= ERROR_POWER_EXCEED;
		}

		ch->power_exceed_cnts = 0;
		ch->power_exceed_gap_cnts = 0;
	}
}

void check_cur_vol_res_error(volatile solder_ch_state_t *ch, volatile vol_cur_data_t *vol_cur)
{
	uint8_t cur_test_i;
	uint8_t first_error_discard_flag = 1;
	ch->ch_err_cnt[3] = 0;
	ch->ch_err_cnt[4] = 0;
	ch->ch_err_cnt[5] = 0;
	ch->ch_err_cnt[6] = 0;

	for (cur_test_i = 0; cur_test_i < vol_cur->length; cur_test_i++)
	{
		if (vol_cur->heat_state[cur_test_i] == heat_on)
		{
			if (first_error_discard_flag == 1)
			{
				first_error_discard_flag = 0;
			}
			else
			{
				// if( fabs(vol_cur->resist[cur_test_i]) < RES_MIN ){
				// 	ch->ch_err_cnt[3]++;
				// }
				if( fabs(vol_cur->resist[cur_test_i]) > RES_MAX || fabs(vol_cur->current[cur_test_i]) < CUR_HEAT_ON_MIN){
					ch->ch_err_cnt[4]++;
				}
			}
		}
		else
		{
			// if( fabs(vol_cur->current[cur_test_i]) >  CUR_HEAT_OFF_MAX){ 
			//		ch->ch_err_cnt[3]++;
			// }
		}

		if (vol_cur->pcb_temp[cur_test_i] > PCB_TEMP_MAX_ERR)
		{
			ch->ch_err_cnt[5]++;
		}

		if (fabs(vol_cur->ileak[cur_test_i]) > ILEAK_MAX_ERR)
		{
			ch->ch_err_cnt[6]++;
		}
	}
#if DEBUG_ERROR == 1
	// DELAY_EXECUTE(1000,{
	// 	sprintf(debug_string_out_0, "vol_cur->pcb_temp: %2f, vol_cur->pcb_temp[cur_test_i] + ch_bsp_debug.temp_test_num: %2f \r\n", vol_cur->pcb_temp[0], vol_cur->pcb_temp[cur_test_i] + ch_bsp_debug.temp_test_num);
	// 	log_info(debug_string_out_0);});
	// DELAY_EXECUTE(1000, {
	// 	sprintf(debug_string_out_0, "ileak: %2f, voltage: %2f, pcb_temp: %2f\r\n",
	// 		solder.voltage_current.ileak[0], solder.voltage_current.voltage[0], solder.voltage_current.pcb_temp[0]);
	// 	log_info(debug_string_out_0); });
#endif

	if (ch->ch_err_cnt[3] > 2)
	{
		ch->ch_err_flag |= ERROR_CUR_SHORT_ERROR;
	}

	if (ch->ch_err_cnt[4] > 2)
	{
		ch->ch_err_flag |= ERROR_CUR_CUT_ERROR;
	}

	if (ch->ch_err_cnt[5] > 5)
	{
		ch->ch_err_flag |= CH_ERR_PCB_TEMP_ERROR;
	}

	if (ch->ch_err_cnt[6] > 3)
	{
		ch->ch_err_flag |= CH_ERR_ILEAK_ERROR;
	}

#if DEBUG_ERROR == 1

	if (ch->ch_err_flag != ERROR_NONE)
		if (ch_logic_debug.error_flag == 1)
		{
			ch_logic_debug.error_flag = 0;
			sprintf(debug_string_out_0, "error ch_err_flag:%d\n",
					solder_channel.ch_err_flag);
			log_info(debug_string_out_0);

			if (solder_channel.ch_err_flag != ERROR_TEMP_ERROR &&
				solder_channel.ch_err_flag != ERROR_TEMP_TOOL_ERROR)
			{
				if (solder_channel.ch_err_flag)
				{
					sprintf(debug_string_out_0, "%2.2f", vol_cur->voltage[0]);

					for (int vol_cur_i = 0; vol_cur_i < vol_cur->length; vol_cur_i++)
					{
						sprintf(debug_string_out_1, ", %2.2f", vol_cur->voltage[vol_cur_i]);
						strcat(debug_string_out_0, debug_string_out_1);
					}

					strcat(debug_string_out_0, "\n");
					log_info(debug_string_out_0);
					//
					sprintf(debug_string_out_0, "%2.2f", vol_cur->current[0]);

					for (int vol_cur_i = 0; vol_cur_i < vol_cur->length; vol_cur_i++)
					{
						sprintf(debug_string_out_1, ", %2.2f", vol_cur->current[vol_cur_i]);
						strcat(debug_string_out_0, debug_string_out_1);
					}

					strcat(debug_string_out_0, "\n");
					log_info(debug_string_out_0);
					sprintf(debug_string_out_0, "%2.2f", vol_cur->resist[0]);

					for (int vol_cur_i = 0; vol_cur_i < vol_cur->length; vol_cur_i++)
					{
						sprintf(debug_string_out_1, ", %2.2f", vol_cur->resist[vol_cur_i]);
						strcat(debug_string_out_0, debug_string_out_1);
					}

					strcat(debug_string_out_0, "\n");
					log_info(debug_string_out_0);
					sprintf(debug_string_out_0, "%2d", vol_cur->heat_state[0]);

					for (int vol_cur_i = 0; vol_cur_i < vol_cur->length; vol_cur_i++)
					{
						sprintf(debug_string_out_1, ", %2d", vol_cur->heat_state[vol_cur_i]);
						strcat(debug_string_out_0, debug_string_out_1);
					}

					strcat(debug_string_out_0, "\n");
					log_info(debug_string_out_0);
				}
			}
		}
		#endif
}

void check_solder_channel_tool_type_from_res(volatile solder_ch_state_t *ch,
											 volatile vol_cur_data_t *vol_cur)
{
	uint8_t cur_test_i;

	if (ch->type_test_avg_res < RES_210_MIN)
	{
		ch->tool_type_3 = notool;
	}
	else if (ch->type_test_avg_res > RES_210_MIN &&
			 ch->type_test_avg_res < RES_245_MAX)
	{
		ch->tool_type_3 = tool_210_245;
	}
	else if (ch->type_test_avg_res > RES_115_MIN &&
			 ch->type_test_avg_res < RES_115_MAX)
	{
		ch->tool_type_3 = tool_115;
	}
	else if (ch->type_test_avg_res > RES_115_MAX)
	{
		ch->tool_type_3 = notool; // err
	}
}

void ch_temp_show_filt_once(volatile solder_ch_state_t *ch)
{
	if (ch->ch_stage == ch_working)
	{
		ch->temp_show = 0.96f * ch->temp_show + 0.04f * ch->controller.temp_ested;
	}
	else
	{
		ch->temp_show = 0.99f * ch->temp_show + 0.01f * ch->controller.temp_ested;
	}

	if (ch->ch_stage == ch_working || (ch->ch_stage == ch_sleep && ch->min_temp_enable_flag == 1))
		if (((ch->temp_show - ch->temp_target > 0.0f) &&
			 (ch->temp_show - ch->temp_target < 20.0f)) ||
			((ch->temp_target - ch->temp_show < 3.0f) &&
			 (ch->temp_target - ch->temp_show > 0.0f)))
		{
			ch->temp_show = ch->temp_target;
		}
}

/************ channel stage process *************/
void solder_ch_stage_process(volatile solder_ch_state_t *ch)
{
	// static uint8_t tool_out_cnt = 0;
	// DELAY_EXECUTE(1000,{
	// 	sprintf(debug_string_out_0, "ch_stage: %d, ch_set_timeout_cnt: %d, tool_in_update_flag: %d\r\n", ch->ch_stage, ch->ch_set_timeout_cnt, tool_detect_data.tool_in_update_flag);
	// 	log_info(debug_string_out_0);});
	switch (ch->ch_stage)
	{
	// 进入空闲状态
	case ch_ideal_notool:
		ch->heat_mode = heat_none;

		if (ch->ch_stage_frstin_flag == 1)
		{
			ch->ch_stage_frstin_flag = 0;
			ch->tool_in_state = tool_out;
			ch->tool_type = notool;
			ch->sleep_state = sleep_err;
			mux_select(mux_reset);
			ch->tool_state_update_sw = CH_STATE_SW_TOOL_IN_ON;
			tool_detect_signal_reset(&tool_detect_data);
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

			if (ch_logic_debug.stage_flag == 1)
			{
				sprintf((char *)debug_string_out_0, "stage: ch_ideal_notool\t\t%d\n",
						getSystemTime());
				log_info((char *)debug_string_out_0);
			}

#endif
		}
		else
		{
			//  处理进入下一阶段
			if (ch->ch_err_flag != ERROR_NONE)
			{
				ch->ch_last_stage = ch->ch_stage;
				ch->ch_stage = ch_err;
				ch->ch_stage_frstin_flag = 1;
			}
			else
			{
				if (tool_detect_data.tool_in_update_flag > 0 &&
					tool_detect_data.tool_in_state == tool_in)
				{
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_tool_in_test;
					ch->ch_stage_frstin_flag = 1;
				}
			}
		}

		break;
	// 为检测到插入的烙铁头后，开始判断
	case ch_tool_in_test:
		ch->heat_mode = heat_none;

		if (ch->ch_stage_frstin_flag == 1)
		{
			ch->ch_stage_frstin_flag = 0;
			ch->tool_in_state = tool_out;
			ch->tool_type = notool;
			ch->tool_type_1 = notool;
			ch->tool_type_2 = notool;
			ch->tool_type_3 = notool;
			ch->sleep_state = sleep_err;
			ch->tool_in_wait_cnt = 0;
			ch->tool_state_update_sw = CH_STATE_SW_OFF;
			tool_detect_signal_reset(&tool_detect_data);
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

			if (ch_logic_debug.stage_flag == 1)
			{
				sprintf((char *)debug_string_out_0, "stage: ch_tool_in_test\t\t%d\n",
						getSystemTime());
				log_info((char *)debug_string_out_0);
			}

#endif
		}
		else
		{
			if (ch->ch_err_flag != ERROR_NONE)
			{
				ch->ch_last_stage = ch->ch_stage;
				ch->ch_stage = ch_err;
				ch->ch_stage_frstin_flag = 1;
			}
			else
			{
				ch->tool_in_wait_cnt++;

				if (ch->tool_in_wait_cnt > 20)
				{
					ch->tool_state_update_sw =
						CH_STATE_SW_TOOL_IN_ON | CH_STATE_SW_TOOL_TYPE_1_ON;
				}

				if ((tool_detect_data.tool_in_update_flag > 0) &&
					(tool_detect_data.tool_type_1_update_flag > 0) &&
					(tool_detect_data.tool_in_state == tool_in) &&
					(tool_detect_data.tool_type_1 != notool))
				{
					ch->tool_type_1 = tool_detect_data.tool_type_1;
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_tool_quality_test;
					ch->ch_stage_frstin_flag = 1;
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

					if (ch_logic_debug.stage_flag == 1)
					{
						sprintf((char *)debug_string_out_0,
								"stage switch : tool_type_1\t\t%d\n", ch->tool_type_1);
						log_info((char *)debug_string_out_0);
					}

#endif
				}
				else if (tool_detect_data.tool_in_update_flag > 0 &&
						 tool_detect_data.tool_in_state == tool_out)
				{
					ch->tool_in_state = tool_out;
					ch->tool_type = notool;
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_ideal_notool;
					ch->ch_stage_frstin_flag = 1;
				}
			}
		}

		break;

	// 检测工具好坏
	case ch_tool_quality_test:
		if (ch->ch_stage_frstin_flag == 1)
		{
			ch->ch_stage_frstin_flag = 0;
			ch->heat_test_cnts = 0;
			ch->ch_tool_quality_test_cnt = 0;
			ch->heat_test_max_cnts = 2;
			ch->tool_state_update_sw = CH_STATE_SW_TOOL_TYPE_2_ON | CH_STATE_SW_TOOL_IN_ON;
			tool_detect_signal_reset(&tool_detect_data);
			ch->heat_mode = heat_test;
			ch->ch_stage_frstin_flag = 0;
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

			if (ch_logic_debug.stage_flag == 1)
			{
				sprintf((char *)debug_string_out_0,
						"stage: ch_tool_quality_test\t\t%d\n", getSystemTime());
				log_info((char *)debug_string_out_0);
			}

#endif
		}
		else
		{
			if (ch->ch_err_flag != ERROR_NONE)
			{
				ch->ch_last_stage = ch->ch_stage;
				ch->ch_stage = ch_err;
				ch->ch_stage_frstin_flag = 1;
			}
			else
			{
				if (ch->heat_test_cnts == ch->heat_test_max_cnts)
				{
					if (tool_detect_data.tool_type_2_update_flag > 0)
					{
						if (tool_detect_data.tool_type_2 != notool)
						{
							ch->tool_type_2 = tool_detect_data.tool_type_2;
							ch->tool_state_update_sw &= ~CH_STATE_SW_TOOL_TYPE_2_ON;
							check_solder_channel_tool_type_from_res(
								&solder_channel, &solder.voltage_current);
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

							if (ch->type_test_avg_res < 0)
							{
								sprintf(debug_string_out_0, "%2.2f",
										solder.voltage_current.voltage[0]);

								for (int vol_cur_i = 0;
									 vol_cur_i < solder.voltage_current.length; vol_cur_i++)
								{
									sprintf(debug_string_out_1, ", %2.2f",
											solder.voltage_current.voltage[vol_cur_i]);
									strcat(debug_string_out_0, debug_string_out_1);
								}

								strcat(debug_string_out_0, "\n");
								log_info(debug_string_out_0);
								//
								sprintf(debug_string_out_0, "%2.2f",
										solder.voltage_current.current[0]);

								for (int vol_cur_i = 0;
									 vol_cur_i < solder.voltage_current.length; vol_cur_i++)
								{
									sprintf(debug_string_out_1, ", %2.2f",
											solder.voltage_current.current[vol_cur_i]);
									strcat(debug_string_out_0, debug_string_out_1);
								}

								strcat(debug_string_out_0, "\n");
								log_info(debug_string_out_0);
								sprintf(debug_string_out_0, "%2.2f",
										solder.voltage_current.resist[0]);

								for (int vol_cur_i = 0;
									 vol_cur_i < solder.voltage_current.length; vol_cur_i++)
								{
									sprintf(debug_string_out_1, ", %2.2f",
											solder.voltage_current.resist[vol_cur_i]);
									strcat(debug_string_out_0, debug_string_out_1);
								}

								strcat(debug_string_out_0, "\n");
								log_info(debug_string_out_0);
								sprintf(debug_string_out_0, "%2d",
										solder.voltage_current.heat_state[0]);

								for (int vol_cur_i = 0;
									 vol_cur_i < solder.voltage_current.length; vol_cur_i++)
								{
									sprintf(debug_string_out_1, ", %2d",
											solder.voltage_current.heat_state[vol_cur_i]);
									strcat(debug_string_out_0, debug_string_out_1);
								}

								strcat(debug_string_out_0, "\n");
								log_info(debug_string_out_0);
							}

#endif
							// 重新检查 tool type 1 的情况
							//						ch->tool_state_update_sw
							//= CH_STATE_SW_TOOL_IN_ON | CH_STATE_SW_TOOL_TYPE_1_ON;
							//						tool_detect_signal_reset(&tool_detect_data);
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

							if (ch_logic_debug.stage_flag == 1)
							{
								sprintf((char *)debug_string_out_0,
										"stage tool_quality switch tool_type_1: "
										"%d,tool_type_2: %d, tool_type_3: %d, res = %.2f\n",
										ch->tool_type_1, ch->tool_type_2, ch->tool_type_3,
										ch->type_test_avg_res);
								log_info((char *)debug_string_out_0);
								sprintf((char *)debug_string_out_0,
										"stage tool_quality debug: heat_test_cnts:\t%d\n",
										ch->heat_test_cnts);
								log_info((char *)debug_string_out_0);
							}

#endif
						}
					}
				}

				//					if(
				// tool_detect_data.tool_type_1_update_flag > 0
				//						&&
				// tool_detect_data.tool_type_1 != notool){
				// ch->tool_type_1 = tool_detect_data.tool_type_1;
				//
				//						#if DEBUG_CMD_CTROL == 1
				//&& DEBUG_STAGE_TEST == 1
				// if(ch_logic_debug.stage_flag == 1){
				// sprintf((char*)debug_string_out_0, "stage tool_quality switch
				// tool_type_1:%d\n",
				// tool_detect_data.tool_type_1);
				// log_info((char*)debug_string_out_0);
				//						}
				//						#endif
				//					}

				if (tool_detect_data.tool_in_update_flag > 0 &&
					tool_detect_data.tool_in_state == tool_out)
				{
					ch->tool_in_state = tool_out;
					ch->tool_type = notool;
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_ideal_notool;
					ch->ch_stage_frstin_flag = 1;
				}

				if (ch->tool_type_1 == notool || ch->tool_type_2 == notool ||
					ch->tool_type_3 == notool)
				{
					// 工具损坏
					ch->ch_tool_quality_test_cnt++;
					if(ch->ch_tool_quality_test_cnt >= 300){
						// 工具损坏
						ch->ch_last_stage = ch->ch_stage;
						ch->ch_stage = ch_ideal_notool;
						ch->ch_stage_frstin_flag = 1;
					}
				}else
				{
					if (ch->tool_type_1 == ch->tool_type_2)
					{
						// 处理电流电阻
						if (ch->tool_type_1 == tool_115_210 &&
							ch->tool_type_3 == tool_115)
						{
							ch->tool_type = tool_115;
						}
						else if (ch->tool_type_1 == tool_115_210 &&
								 ch->tool_type_3 == tool_210_245)
						{
							ch->tool_type = tool_210;
						}
						else if (ch->tool_type_1 == tool_245 &&
								 ch->tool_type_3 == tool_210_245)
						{
							ch->tool_type = tool_245;
						}
						else
						{
							ch->tool_type = notool;
						}

						if (ch->tool_type != notool)
						{
							switch (ch->tool_type)
							{
							case tool_115:
								mux_select(mux_115_temp);
								break;

							case tool_210:
								mux_select(mux_210_temp);
								break;

							case tool_245:
								mux_select(mux_245_temp);
								break;

							default:
								mux_select(mux_reset);
								break;
							}

							// 根据频率和功率模式设置加热参数
							if (ch->power_AC_HZ_flag == 1)
							{
								if (ch->power_high_flag == 0)
								{
									// log_info("%%%%%% 50Hz_L\r\n");
									// 50Hz 低功率配置
									switch (ch->tool_type)
									{
									case tool_115:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_115_50Hz_L;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_115_50Hz_L;
										break;

									case tool_210:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_210_50Hz_L; // 87w
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_210_50Hz_L;
										break;

									case tool_245:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_245_50Hz_L;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_245_50Hz_L;
										break;

									default:
										break;
									}
								}
								else
								{
									// log_info("%%%%%% 50Hz_H\r\n");
									// 50Hz 高功率配置
									switch (ch->tool_type)
									{
									case tool_115:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_115_50Hz_H;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_115_50Hz_H;
										break;

									case tool_210:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_210_50Hz_H; // 122w
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_210_50Hz_H;
										break;

									case tool_245:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_245_50Hz_H;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_245_50Hz_H;
										break;

									default:
										break;
									}
								}
							}
							else
							{
								// 60Hz 低功率配置
								if (ch->power_high_flag == 0)
								{
									// log_info("%%%%%%60Hz_L\r\n");
									switch (ch->tool_type)
									{
									case tool_115:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_115_60Hz_L;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_115_60Hz_L;
										break;

									case tool_210:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_210_60Hz_L; // 80w
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_210_60Hz_L;
										break;

									case tool_245:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_245_60Hz_L;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_245_60Hz_L;
										break;

									default:
										break;
									}
								}
								else
								{
									//								log_info("%%%%%% 60Hz_H\r\n");
									// 60Hz 高功率配置
									switch (ch->tool_type)
									{
									case tool_115:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_115_60Hz_H;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_115_60Hz_H;
										break;

									case tool_210:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_210_60Hz_H;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_210_60Hz_H;
										break;

									case tool_245:
										solder_channel.heat_time_frst_half =
											CH_FRST_HALF_TIM_245_60Hz_H;
										solder_channel.heat_time_botm_half =
											CH_BOTM_HALF_TIM_245_60Hz_H;
										break;

									default:
										break;
									}
								}
							}

#ifdef THT_L075

							switch (ch->tool_type)
							{
							case tool_115:
								adjust_power_voltage(10);
								break;

							case tool_210:
								adjust_power_voltage(12);
								break;

							case tool_245:
								adjust_power_voltage(16);
								break;

							default:
								break;
							}

#endif
							ch->tool_in_state = tool_in;
							ch->ch_last_stage = ch->ch_stage;
							ch->ch_stage = ch_sleep;
							ch->ch_stage_frstin_flag = 1;
							controller_init(&solder_channel.controller,
											solder_channel.tool_type);
							ctrl_params_update_from_target_temp(&solder_channel.controller,
																solder_channel.temp_target);
						}
						else
						{
							ch->ch_last_stage = ch->ch_stage;
							ch->ch_stage = ch_ideal_notool;
							ch->ch_stage_frstin_flag = 1;
						}

#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

						if (ch_logic_debug.stage_flag == 1)
						{
							sprintf(
								(char *)debug_string_out_0,
								"stage tool_quality tool_type detected ok:%d, type_1:%d, "
								"type_2:%d, type_3:%d\n",
								ch->tool_type, ch->tool_type_1, ch->tool_type_2,
								ch->tool_type_3);
							log_info((char *)debug_string_out_0);
						}

#endif
					}
					else
					{
						//	time out deal
						ch->ch_last_stage = ch->ch_stage;
						ch->ch_stage = ch_ideal_notool;
						ch->ch_stage_frstin_flag = 1;
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

						if (ch_logic_debug.stage_flag == 1)
						{
							sprintf(debug_string_out_1,
									"error tool test tool_type type1: %d  type2: %d  "
									"type3: %d\n",
									ch->tool_type_1, ch->tool_type_2, ch->tool_type_3);
							log_info(debug_string_out_1);
						}

#endif
					}
				}
			}
		}

		break;

	// 正常工作状态，开始休眠
	case ch_sleep:
		if (ch->min_temp_enable_flag == 1)
		{
			ch->heat_mode = heat_min_sleep;
		}
		else
		{
			ch->heat_mode = heat_none;
		}
		static float last_temp_target_temp_transfer = 0;

		if (ch->ch_stage_frstin_flag == 1)
		{
			ch->ch_stage_frstin_flag = 0;
			// ch->ch_sleep_timeout_cnt = 0; // 重置计数器
			ch->ch_sleep_stage_frstin_flag = 0;
			tool_detect_signal_reset(&tool_detect_data);
			ch->tool_state_update_sw = CH_STATE_SW_TOOL_IN_ON | CH_STATE_SW_SLEEP_ON;

			if (ch->min_temp_enable_flag == 1)
			{
				ch->heat_mode = heat_min_sleep;
				solder_param_get_from_array((void *)&min_target_temp_transfer, param_target_temp);
				if (ch->ch_last_stage != ch_set)
				{
					temp_target_temp_transfer = 0;
				}
				last_temp_target_temp_transfer = 0;
				ch->temp_target = ch->min_temp;
			}


			ch->temp_reset_ntf = 1;
			ctrl_params_update_from_target_temp(&solder_channel.controller, solder_channel.temp_target);
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

			if (ch_logic_debug.stage_flag == 1)
			{
				sprintf((char *)debug_string_out_0, "stage: ch_sleep\t\t%d\n",
						getSystemTime());
				log_info((char *)debug_string_out_0);
			}

#endif
		}

		// ch->ch_sleep_timeout_cnt++;
		uint8_t timeout_flag = 0;
		// if (ch->ch_sleep_timeout_cnt >= ch->ch_set_timeout_max)
		if (solder.disp_maintain_state == 0)
		{
			// ch->ch_sleep_timeout_cnt = 0;
			ch->ch_sleep_stage_frstin_flag = 0;
			if (temp_target_temp_transfer != last_temp_target_temp_transfer)
			{
				float temp_target = min_target_temp_transfer + temp_target_temp_transfer;
				solder_param_write_to_array((void *)&temp_target, param_target_temp);
				last_temp_target_temp_transfer = temp_target_temp_transfer;
				solder_channel.ch_temp_param_update_ntf = 1;
			}
		}

		// DELAY_EXECUTE(1000,{
		// sprintf(debug_string_out_0, "heat_mode: %2d, heat_state: %2d, heat_ctrl_flag: %2d\r\n", ch->heat_mode, ch->heat_state, ch->heat_ctrl_flag);
		// log_info(debug_string_out_0);});

		if (ch->ch_err_flag != ERROR_NONE)
		{
			ch->ch_last_stage = ch->ch_stage;
			ch->ch_stage = ch_err;
			ch->ch_stage_frstin_flag = 1;
		}
		else
		{
			if (tool_detect_data.tool_in_update_flag > 0 &&
				tool_detect_data.tool_in_state == tool_out)
			{
				ch->tool_in_state = tool_out;
				ch->ch_last_stage = ch->ch_stage;
				ch->ch_stage = ch_ideal_notool;
				ch->ch_stage_frstin_flag = 1;

				if (ch->min_temp_enable_flag == 1)
				{
					solder_channel.temp_target = min_target_temp_transfer + temp_target_temp_transfer;
					key_ntf_cmd.temp_target_chg_ntf = 1;
				}
			}

			if (tool_detect_data.sleep_state_update_flag > 0 &&
				tool_detect_data.sleep_state == on_work)
			{
				if (ch->min_temp_enable_flag == 1)
				{
					solder_channel.temp_target = min_target_temp_transfer + temp_target_temp_transfer;
					key_ntf_cmd.temp_target_chg_ntf = 1;
				}

				ch->sleep_state = on_work;
				ch->ch_last_stage = ch->ch_stage;
				ch->ch_stage = ch_working;
				ch->ch_stage_frstin_flag = 1;
				ch->heat_mode = heat_norm_botm_half;
			}
		}

		break;

	case ch_working:
		if (ch->ch_stage_frstin_flag == 1)
		{
			// tool_out_cnt = 0;
			ch->ch_stage_frstin_flag = 0;
			ch->tool_state_update_sw =
				CH_STATE_SW_TOOL_IN_ON | CH_STATE_SW_SLEEP_ON;
			// ch->tool_state_update_sw 	|= CH_STATE_SW_TOOL_IN_ON;
			tool_detect_signal_reset(&tool_detect_data);

			if (ch->ch_last_stage == ch_err)
			{
				ctrl_params_update_from_target_temp(&ch->controller, ch->temp_target);
				ch->sleep_state = on_work;
			}

			ch->heat_mode = heat_norm_botm_half;
			ch->ctrl_temp_ok_first_ntf = 0;
			ch->work_to_sleep_wait_cnt = 100;
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

			if (ch_logic_debug.stage_flag == 1)
			{
				sprintf((char *)debug_string_out_0, "stage: ch_working\t\t%d\n",
						getSystemTime());
				log_info((char *)debug_string_out_0);
			}
#endif
		}

		if (ch->ch_err_flag != ERROR_NONE)
		{
			ch->ch_last_stage = ch->ch_stage;
			ch->ch_stage = ch_err;
			ch->ch_stage_frstin_flag = 1;
			ch->heat_mode = heat_none;
		}
		else
		{
			if (tool_detect_data.tool_in_update_flag > 0 &&
				tool_detect_data.tool_in_state == tool_out)
			{
				ch->tool_in_state = tool_out;
				ch->tool_type = notool;
				ch->ch_last_stage = ch->ch_stage;
				ch->ch_stage = ch_ideal_notool;
				ch->ch_stage_frstin_flag = 1;
				ch->heat_mode = heat_none;
			}

			if (ch->ch_last_stage == ch_sleep && ch->ctrl_temp_ok_first_ntf == 0 &&
				ch->temp_show > ch->temp_target - 1.0f)
			{
				ch->ctrl_temp_ok_first_ntf = 1;
				beep_once_ntf = 1;
			}


			if (tool_detect_data.sleep_state_update_flag > 0 &&
				tool_detect_data.sleep_state != on_work)
			{
				if (ch->work_to_sleep_wait_cnt > 0)
				{
					ch->work_to_sleep_wait_cnt--;
				}
				else
				{
					ch->sleep_state = tool_detect_data.sleep_state;
					ch->ch_stage = ch_sleep;
					ch->ch_stage_frstin_flag = 1;
					ch->heat_mode = heat_none;
					beep_once_ntf = 0x11;
				}
			}
		}

		break;

	case ch_err:
		if (ch->ch_stage_frstin_flag == 1)
		{
			ch->ch_stage_frstin_flag = 0;
			ch->heat_mode = heat_none;
			// ch->ch_err_wait_cnt = 0;
			ch_err_wait_cnt = 0;
			ch->tool_state_update_sw = CH_STATE_SW_OFF;
			tool_detect_signal_reset(&tool_detect_data);

			if ((ch->ch_err_flag & ERROR_CUR_SHORT_ERROR) != ERROR_NONE)
			{
				ch->tool_state_update_sw =
					CH_STATE_SW_SLEEP_ON | CH_STATE_SW_TOOL_IN_ON;
			}

			if (ch->ch_last_stage == ch_working)
			{
				if ((ch->ch_err_flag & ERROR_POWER_EXCEED) != ERROR_NONE)
				{
					ch->tool_state_update_sw = CH_STATE_SW_SLEEP_ON;
				}

				if ((ch->ch_err_flag & ERROR_TEMP_TOOL_ERROR) != ERROR_NONE)
				{
					ch->tool_state_update_sw = CH_STATE_SW_TOOL_IN_ON;
				}
			}

			//				beep_once_ntf = 3;
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

			if (ch_logic_debug.stage_flag == 1)
			{
				sprintf((char *)debug_string_out_0, "stage: ch_err\t%d\t%d\n",
						getSystemTime(), solder_channel.ch_err_flag);
				log_info((char *)debug_string_out_0);
			}

#endif
		}
		else
		{
			if ((ch->ch_err_flag & ERROR_CUR_CUT_ERROR) != ERROR_NONE)
			{
				// ch->ch_err_wait_cnt += 1;
				ch_err_wait_cnt++;

				if (ch_err_wait_cnt > TYPE_AGAIN_TEST_TIM)
				{
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_ideal_notool;
					ch->ch_stage_frstin_flag = 1;
					ch->ch_err_flag &= ~ERROR_CUR_CUT_ERROR;
				}
			}
			else if ((ch->ch_err_flag & ERROR_CUR_SHORT_ERROR) != ERROR_NONE)
			{
				if (tool_detect_data.tool_in_update_flag > 0 &&
					tool_detect_data.tool_in_state == tool_out)
				{
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_ideal_notool;
					ch->ch_stage_frstin_flag = 1;
					ch->ch_err_flag &= ~ERROR_CUR_SHORT_ERROR;
				}
			}


			//				if(ch->ch_last_stage == ch_working){
			if ((solder_channel.ch_err_flag & ERROR_TEMP_ERROR) != ERROR_NONE &&
				(solder_channel.ch_err_flag & (~ERROR_TEMP_ERROR)) == ERROR_NONE)
			{
				// ch->ch_err_wait_cnt += 1;
				ch_err_wait_cnt++;

				if (ch_err_wait_cnt > 6)
				{
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_working;
					ch->heat_mode = heat_norm_botm_half;
					ch->ch_stage_frstin_flag = 1;
					ch->ch_err_flag &= (~ERROR_TEMP_ERROR);
				}
			}

			if ((solder_channel.ch_err_flag & CH_ERR_PCB_TEMP_ERROR) != ERROR_NONE)
			{
				// ch->ch_err_wait_cnt += 1;
				ch_err_wait_cnt++;

				if (ch_err_wait_cnt > 300 || ch->pcb_temp_avg < PCB_TEMP_MAX_ERR - 5)
				{
					if (tool_detect_data.sleep_state_update_flag > 0 &&
						tool_detect_data.sleep_state != on_work &&
						tool_detect_data.sleep_state != sleep_err)
					{
						ch->ch_last_stage = ch->ch_stage;
						ch->ch_stage = ch_sleep;
						ch->ch_stage_frstin_flag = 1;
						ch->ch_err_flag &= (~CH_ERR_PCB_TEMP_ERROR);
					}
				}
			}

			if ((solder_channel.ch_err_flag & CH_ERR_ILEAK_ERROR) != ERROR_NONE)
			{
				// ch->ch_err_wait_cnt += 1;
				ch_err_wait_cnt++;

				if (ch_err_wait_cnt > 2000)
				{
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_ideal_notool;
					ch->ch_stage_frstin_flag = 1;
					ch->ch_err_flag &= (~CH_ERR_ILEAK_ERROR);
				}
			}

			if ((solder_channel.ch_err_flag & ERROR_TEMP_TOOL_ERROR) !=
				ERROR_NONE)
			{
				if (tool_detect_data.tool_in_update_flag > 0 &&
					tool_detect_data.tool_in_state == tool_out)
				{
					ch->ch_last_stage = ch->ch_stage;
					ch->ch_stage = ch_ideal_notool;
					ch->ch_stage_frstin_flag = 1;
					ch->ch_err_flag &= ~ERROR_TEMP_TOOL_ERROR;
				}
			}

			if ((solder_channel.ch_err_flag & ERROR_POWER_EXCEED) != ERROR_NONE &&
				(solder_channel.ch_err_flag & (~ERROR_POWER_EXCEED)) ==
					ERROR_NONE)
			{
				// ch->ch_err_wait_cnt += 1;
				ch_err_wait_cnt++;

				if (ch_err_wait_cnt > 100)
				{
					if (tool_detect_data.sleep_state_update_flag > 0 &&
						tool_detect_data.sleep_state != on_work &&
						tool_detect_data.sleep_state != sleep_err)
					{
						ch->ch_last_stage = ch->ch_stage;
						ch->ch_stage = ch_sleep;
						ch->ch_stage_frstin_flag = 1;
						ch->ch_err_flag &= (~ERROR_POWER_EXCEED);
					}
				}
			}
		}

		//			}
		break;

	case ch_set:
		if (ch->ch_stage_frstin_flag)
		{
#if DEBUG_SET_MODE_TEST == 1
			sprintf((char *)debug_string_out_0, "stage: ch_set/*/*\t\t%d\n",
					getSystemTime());
			log_info((char *)debug_string_out_0);
#endif

			if (solder_channel.ch_set_mode == ch_set_mode_calibration)
			{
				//						if(
				// ch->ctrl_temp_ok_first_ntf == 0 &&
				// ch->temp_show > ch->temp_target
				//- 1.0f )
				//							{
				//									ch->ctrl_temp_ok_first_ntf
				//= 1;
				// beep_once_ntf = 1;
				//							}
			}
			else
			{
				ch->heat_mode = heat_none; // 非校准模式关闭加热
			}

			ch->ch_stage_frstin_flag = 0;
			ch->set_complete_flag = 0;
			ch->ch_set_timeout_cnt = 0; // 设置模式超时，重置超时计数器

			if (ch->ch_set_mode == ch_set_mode_power_high)
			{
				if (ch->power_high_flag > 0)
				{
					ch->power_high_flag = 0;
				}
				else
				{
					ch->power_high_flag = 1;
				}

				solder_param_write_to_array((void *)&ch->power_high_flag,
											param_control_power_high);
				ch->ch_power_param_udpate_ntf = 1;

				//					switch(ch->tool_type)
				//					{
				//						case tool_115:
				// mux_select(mux_115_temp); break;
				// case tool_210: mux_select(mux_210_temp); break;
				// case tool_245: mux_select(mux_245_temp); break;
				// default: mux_select(mux_reset); break;
				//					}
				// 根据频率和功率模式设置加热参数
				if (ch->power_AC_HZ_flag == 1)
				{
					if (ch->power_high_flag == 0)
					{
						// log_info("****** 50Hz_L\r\n");
						// 50Hz 低功率配置
						switch (ch->tool_type)
						{
						case tool_115:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_115_50Hz_L;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_115_50Hz_L;
							break;

						case tool_210:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_210_50Hz_L;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_210_50Hz_L;
							break;

						case tool_245:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_245_50Hz_L;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_245_50Hz_L;
							break;

						default:
							break;
						}
					}
					else
					{
						//								log_info("****** 50Hz_H\r\n");
						// 50Hz 高功率配置
						switch (ch->tool_type)
						{
						case tool_115:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_115_50Hz_H;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_115_50Hz_H;
							break;

						case tool_210:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_210_50Hz_H;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_210_50Hz_H;
							break;

						case tool_245:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_245_50Hz_H;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_245_50Hz_H;
							break;

						default:
							break;
						}
					}
				}
				else
				{
					// 60Hz 配置
					if (ch->power_high_flag == 0)
					{
						//							log_info("****** 60Hz_L\r\n");
						switch (ch->tool_type)
						{
						case tool_115:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_115_60Hz_L;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_115_60Hz_L;
							break;

						case tool_210:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_210_60Hz_L;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_210_60Hz_L;
							break;

						case tool_245:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_245_60Hz_L;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_245_60Hz_L;
							break;

						default:
							break;
						}
					}
					else
					{
						//							log_info("****** 60Hz_H\r\n");
						// 60Hz 高功率配置
						switch (ch->tool_type)
						{
						case tool_115:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_115_60Hz_H;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_115_60Hz_H;
							break;

						case tool_210:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_210_60Hz_H;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_210_60Hz_H;
							break;

						case tool_245:
							solder_channel.heat_time_frst_half =
								CH_FRST_HALF_TIM_245_60Hz_H;
							solder_channel.heat_time_botm_half =
								CH_BOTM_HALF_TIM_245_60Hz_H; // 118w
							break;

						default:
							break;
						}
					}
				}
			}

			ch->tool_state_update_sw = CH_STATE_SW_TOOL_IN_ON;
			tool_detect_signal_reset(&tool_detect_data);
		}
		else
		{
			if (solder_channel.ch_set_mode == ch_set_mode_calibration)
			{
				static float last_temp_target = -1.0f;

				// ch->heat_mode				=
				// heat_norm_botm_half;
				if (ch->ctrl_temp_ok_first_ntf == 0 &&
					ch->temp_show > ch->temp_target - 1.0f)
				{
					ch->ctrl_temp_ok_first_ntf = 1;
					beep_once_ntf = 1;
				}
				else if (ch->temp_target != last_temp_target)
				{
					last_temp_target = ch->temp_target;
					ch->ctrl_temp_ok_first_ntf = 0;
				}

				if (tool_detect_data.tool_in_update_flag > 0 &&
					tool_detect_data.tool_in_state == tool_out)
				{
					// ch->ch_last_stage 			=	ch->ch_stage;
					ch->ch_stage = ch_ideal_notool;
					solder_channel.temp_target = calibration_target_temp_transfer; // 复原目标温度，退出校验模式
					solder_channel.ch_set_step = 4;								   // exit calibration
					ch->ch_stage_frstin_flag = 1;
				}
			}

			ch->ch_set_timeout_cnt++;
			uint8_t timeout_flag = 0;

			// 设置模式超时判断
			if (ch->ch_set_timeout_cnt >= ch->ch_set_timeout_max)
			{
				ch->ch_set_timeout_cnt = 0; // 设置模式超时，重置超时计数器
				timeout_flag = 1;

				if (solder_channel.ch_set_mode == ch_set_mode_calibration)
				{
					solder_channel.temp_target = calibration_target_temp_transfer;
					calibration_param_to_array(solder_channel.tool_type);
					init_all_temp_tab();
					solder_channel.ch_set_step = 4; // exit calibration
					key_ntf_cmd.temp_target_chg_ntf = 1;
					solder_channel.ch_temp_cal_param_udpate_ntf = 1;
				}

				// 蜂鸣器提示校验模式超时
				beep_once_ntf = 1;
			}

			if ((ch->set_complete_flag || timeout_flag) &&
				tool_detect_data.tool_in_update_flag > 0)
			{
				if (ch->ch_set_mode == ch_set_mode_up_down_inc)
				{
					solder_param_write_to_array((void *)&ch->ch_set_temp_disp,
												param_target_temp_inc);
					solder_param_get_from_array((void *)&ch->temp_target_inc,
												param_target_temp_inc);
					ch->ch_temp_param_update_ntf = 1;
				}
				else if (ch->ch_set_mode == ch_set_mode_ch1 ||
						 ch->ch_set_mode == ch_set_mode_ch2 ||
						 ch->ch_set_mode == ch_set_mode_ch3)
				{
					if (solder_channel.ch_set_mode == ch_set_mode_ch1)
					{
						solder_param_write_to_array((void *)&ch->ch_set_temp_disp,
													param_target_temp_ch1);
					}

					if (solder_channel.ch_set_mode == ch_set_mode_ch2)
					{
						solder_param_write_to_array((void *)&ch->ch_set_temp_disp,
													param_target_temp_ch2);
					}

					if (solder_channel.ch_set_mode == ch_set_mode_ch3)
					{
						solder_param_write_to_array((void *)&ch->ch_set_temp_disp,
													param_target_temp_ch3);
					}

					ch->temp_target = ch->ch_set_temp_disp;
					ctrl_params_update_from_target_temp(&ch->controller,
														ch->temp_target);
					solder_param_write_to_array((void *)&ch->temp_target,
												param_target_temp);
					ch->ch_temp_param_update_ntf = 1;
				}
				else if (solder_channel.ch_set_mode == ch_set_mode_calibration)
				{
					solder_channel.ch_set_mode = ch_set_mode_none;
				}
				else if (solder_channel.ch_set_mode == ch_set_mode_min_temp)
				{
					save_min_temp_settings(ch);
					ch->ch_temp_param_update_ntf = 1;
				}


				if ((ch->ch_last_stage == ch_sleep ||
					 ch->ch_last_stage == ch_working) &&
					tool_detect_data.tool_in_state == tool_in)
				{
					ch->ch_stage = ch->ch_last_stage;
					ch->ch_last_stage = ch_set;
					ch->ch_stage_frstin_flag = 1;
				}
				else if (ch->ch_last_stage == ch_err)
				{
					ch->ch_stage = ch->ch_last_stage;
				}
				else
				{
					ch->ch_stage = ch_ideal_notool;
					ch->ch_stage_frstin_flag = 1;
				}
			}
		}

		break;

	default:
		break;
	}
}

#ifdef THT_M120
/********** 过零中断开始处理本次控制 ************/
void ZERO_CHECK_IRQHandler(void)
{
    if (RESET != EXTI_GetITStatus(ZERO_CHECK_LINE)) {
        EXTI_ClrITPendBit(ZERO_CHECK_LINE);
        zero_cross_irq_handler();
    }
}
#endif

#ifdef THT_L075
void CH_CTRL_START_TIM_IRQHandler(void)
{
    if (TIM_GetIntStatus(CH_CTRL_START_TIM_FREQ_100xHz, TIM_INT_UPDATE) != RESET) {
        TIM_ClrIntPendingBit(CH_CTRL_START_TIM_FREQ_100xHz, TIM_INT_UPDATE);
        zero_cross_irq_handler();
    }
}
#endif

/**
 * @brief 零交叉中断处理核心逻辑（公共部分）
 * @note 提取两个平台的公共处理逻辑，避免代码重复
 */
static void zero_cross_irq_handler(void)
{
    /* 停止加热 */
    solder_channel.heat_state = heat_off;
    CH_HeatOff;

    /* 计算真实系统时间（处理时间溢出） */
    uint32_t system_time_real = (system_time < zero_up_irq_tim_record) 
                              ? system_time + SYSTEM_TIME_MAX 
                              : system_time;

    /* 检查时间间隔是否有效（5ms以上） */
    if (system_time_real - zero_up_irq_tim_record <= 5) {
        return;
    }

    uint8_t interval = system_time_real - zero_up_irq_tim_record;
    
    /* 记录有效的时间间隔（5-13ms范围内） */
    if (interval >= 5 && interval <= 13) {
        zero_cross_times[zero_times] = interval;
        zero_times = (zero_times + 1) % 10;
        zero_up_irq_tim_record = system_time;
    }

	// 频率检测阶段需要 zero_cross_times/zero_times，但不需要控制循环
	if (solder.dev_init_over == 0) {
		return;   // 不启动 CH_CTRL_TIMER，不改 ctrl_state
	}

    /* 重置定时器和状态 */
    bsp_ctrl_tim_stop();
    
    /* 保存上一周期的状态 */
    solder_channel.heat_flag_last_cycle = solder_channel.heat_ctrl_flag;
    solder_channel.temp_est_last_cycle = solder_channel.controller.temp_ested;
    solder_channel.temp_current_last_cycle = solder_channel.temp_current;

    /* 启动等待定时器（如果有等待时间） */
    if (solder.zero_wait_time > 0) {
        bsp_ctrl_tim_start_once(solder.zero_wait_time);
    }

    /* 重置控制状态 */
    solder.ctrl_state = ctrl_wait_zero;
    solder_channel.ctrl_state = ctrl_wait_zero;
}

void CH_CTRL_TIM_IRQHandler()
{
	if (TIM_GetIntStatus(CH_CTRL_TIMER, TIM_INT_UPDATE) != RESET)
	{
		TIM_ClrIntPendingBit(CH_CTRL_TIMER, TIM_INT_UPDATE);

		 // 初始化未完成时，立即停表退出，避免乱跑
		 if (solder.dev_init_over == 0) {
			bsp_ctrl_tim_stop();
			return;
		}

		bsp_ctrl_tim_stop();

		switch (solder.ctrl_state)
		{
		case ctrl_wait_zero:
			power_cal(&solder_channel, solder_channel.heat_flag_last_cycle);
			/*  更新电压电流数据  */
			vol_cur_gather_stop();
			get_voltage_current_array();
			//  电压电流错误处理
			check_cur_vol_res_error(&solder_channel, &solder.voltage_current);
			//  重新启动电压电流采集
			vol_cur_gather_start();
			/*  处理按键  */
			key_process();
			solder_ch_stage_process(&solder_channel);
#if DEBUG_CMD_CTROL == 1 && DEBUG_CMD_TARGET_TEMP == 1

			if (ch_logic_debug.temp_target_flag == 1)
			{
				solder_channel.temp_target = debug_temp_target;
				ctrl_params_update_from_target_temp(&solder_channel.controller,
													solder_channel.temp_target);
				ch_logic_debug.temp_target_flag = 0;
			}

#endif
#if DEBUG_CMD_CTROL == 1 && DEBUG_VOL_CUR_OUTPUT == 1

			if (solder_channel.heat_flag_last_cycle == ctrl_heat_on)
				if (ch_logic_debug.voltage_current_flag == 1)
				{
					sprintf(debug_string_ctrl_0,
							"***** voltage and current length: %d *****",
							solder.voltage_current.length);
					sprintf(debug_string_ctrl_1, "\nileadk: ");
					strcat(debug_string_ctrl_0, debug_string_ctrl_1);

					for (debug_i = 0; debug_i < solder.voltage_current.length;
						 debug_i++)
					{
						sprintf(debug_string_ctrl_1, " %4.2f",
								solder.voltage_current.ileak[debug_i]);
						strcat(debug_string_ctrl_0, debug_string_ctrl_1);
					}

					sprintf(debug_string_ctrl_1, "\nvoltage: ");
					strcat(debug_string_ctrl_0, debug_string_ctrl_1);

					for (debug_i = 0; debug_i < solder.voltage_current.length;
						 debug_i++)
					{
						sprintf(debug_string_ctrl_1, " %4.2f",
								solder.voltage_current.voltage[debug_i]);
						strcat(debug_string_ctrl_0, debug_string_ctrl_1);
					}

					sprintf(debug_string_ctrl_1, "\ncurrent: ");
					strcat(debug_string_ctrl_0, debug_string_ctrl_1);

					for (debug_i = 0; debug_i < solder.voltage_current.length;
						 debug_i++)
					{
						sprintf(debug_string_ctrl_1, " %4.4f",
								solder.voltage_current.current[debug_i]);
						strcat(debug_string_ctrl_0, debug_string_ctrl_1);
					}

					sprintf(debug_string_ctrl_1, "\nresist: ");
					strcat(debug_string_ctrl_0, debug_string_ctrl_1);

					for (debug_i = 0; debug_i < solder.voltage_current.length;
						 debug_i++)
					{
						sprintf(debug_string_ctrl_1, " %4.4f",
								solder.voltage_current.resist[debug_i]);
						strcat(debug_string_ctrl_0, debug_string_ctrl_1);
					}

					sprintf(debug_string_ctrl_1, "\nheat: ");
					strcat(debug_string_ctrl_0, debug_string_ctrl_1);

					for (debug_i = 0; debug_i < solder.voltage_current.length;
						 debug_i++)
					{
						sprintf(debug_string_ctrl_1, " %4d",
								solder.voltage_current.heat_state[debug_i]);
						strcat(debug_string_ctrl_0, debug_string_ctrl_1);
					}

					sprintf(debug_string_ctrl_1, "\n");
					strcat(debug_string_ctrl_0, debug_string_ctrl_1);
					log_info(debug_string_ctrl_0);
					//
					//						sprintf(dbg_test,"avg_res:%4.2f\n",avg_res);
					//                        log_info(dbg_test);
					//
					//						sprintf(dbg_test1,"actual_res:%4.2f\n",actual_res);
					//						log_info(dbg_test1);
					//
					//						sprintf(dbg_test1,"avg_cur:%4.2f\n",avg_cur);
					//						log_info(dbg_test1);
				}

#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_LED == 1
			WORK_LED_OFF;
#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_STRING == 1

			if (ch_logic_debug.ctrl_process_flag == 1)
			{
				log_info("***********************\nzero IRQ: ctrl wait_zero\n");
			}

#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_LED == 1
			WORK_LED_ON;
#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_STRING == 1

			if (ch_logic_debug.ctrl_process_flag == 1)
			{
				log_info("ch_tim: ctrl wait_zero\n");
			}

#endif
			bsp_ctrl_tim_start_once(solder.temp_wait_time);
			solder.ctrl_state = ctrl_wait_measure;
			break;

		case ctrl_wait_measure:
#if DEBUG_CTROL_PROCESS_OUTPUT_LED == 1
			WORK_LED_OFF;
#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_STRING == 1

			if (ch_logic_debug.ctrl_process_flag == 1)
			{
				log_info("ch_tim: ctrl wait temperature measure\n");
			}

#endif

			/**************  温度获取开始   **************/
			if (solder_channel.ch_stage == ch_sleep ||
				solder_channel.ch_stage == ch_working ||
				solder_channel.ch_set_mode == ch_set_mode_calibration)
			{
				temp_base_gather_start();
			}
			//				WORK_LED_OFF;
			bsp_ctrl_tim_start_once(solder.measure_time); // 重新开启定时器定时，等待测温时间 CH_MEASURE_TIME 300us
			solder.ctrl_state = ctrl_measure;
			break;

		case ctrl_measure:
#if DEBUG_CTROL_PROCESS_OUTPUT_LED == 1
			WORK_LED_OFF;
#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_STRING == 1

			if (ch_logic_debug.ctrl_process_flag == 1)
			{
				log_info("ch_tim: ctrl measure\n");
			}

#endif

			/**************  温度获取，温度滤波   **************
			 ************** temperature calculate *************/

			if (solder_channel.ch_stage == ch_sleep ||
				solder_channel.ch_stage == ch_working ||
				solder_channel.ch_set_mode == ch_set_mode_calibration)
			{
				temp_base_gather_stop();

				if (adc_regular_dma_state.dma_working_flag == 0)
				{
					solder_channel.temp_current =
						get_temp(adc_regular_dma_state.adc_average,
								 solder_channel.tool_type);
				}
				controller_temp_est(&solder_channel.controller,
									solder_channel.temp_current,
									solder_channel.heat_flag_last_cycle);
				// 显示温度的滤波
				ch_temp_show_filt_once(&solder_channel);

				if (solder_channel.temp_reset_ntf == 1)
				{
					solder_channel.temp_show = solder_channel.controller.temp_ested;
					solder_channel.temp_reset_ntf = 0;
				}
			}

			/**************** 在不同模式下计算是否加热 *******************/
			if (solder_channel.heat_mode == heat_none)
			{
				if (solder_channel.ch_err_flag != ERROR_TEMP_ERROR &&
					solder_channel.ch_err_flag != ERROR_TEMP_TOOL_ERROR)
				{
					controller_state_update(&solder_channel.controller, 0,
											solder_channel.heat_ctrl_flag);
				}

				solder_channel.heat_ctrl_flag = ctrl_heat_off;
				// log_info("heat_none\n");
			}
			else if (solder_channel.heat_mode == heat_test)
			{
				if (solder_channel.heat_test_cnts <
					solder_channel.heat_test_max_cnts)
				{
					solder_channel.heat_test_cnts++;
					solder_channel.heat_ctrl_flag = ctrl_heat_on;
					solder_channel.heat_state = heat_on;
					CH_HeatOn;
				}
				else
				{
					solder_channel.heat_ctrl_flag = ctrl_heat_off;
					solder_channel.heat_state = heat_off;
					CH_HeatOff;
				}

				//					log_info("heat_test\n");
			}
			else if (solder_channel.heat_mode == heat_norm ||
					 solder_channel.heat_mode == heat_norm_frst_half ||
					 solder_channel.heat_mode == heat_norm_botm_half ||
					 solder_channel.heat_mode == heat_min_sleep ||
					 solder_channel.heat_mode == heat_calibration)
			{
				heat_temperature_error(&solder_channel);
				controller_state_update(&solder_channel.controller, 1,
										solder_channel.heat_ctrl_flag);
				solder_channel.heat_ctrl_flag =
					solder_channel.controller.ctrl_heat_flag;
				// log_info("heat_norm\n");
			}
			else
#if DEBUG_CMD_CTROL == 1 && DEBUG_HEAT_IO_TEST == 1
				if (solder_channel.heat_mode == heat_debug_once &&
					ch_bsp_debug.heat_once_flag == 1)
			{
				solder_channel.heat_ctrl_flag = ctrl_heat_on;
				ch_bsp_debug.heat_once_flag = 0;
				solder_channel.heat_state = heat_on;
				CH_HeatOn;
				log_info("heat once debug: heat on \n");
			}
			else
#endif
#if DEBUG_CMD_CTROL == 1 && DEBUG_LOGIC_HEAT_TEST == 1
				if (solder_channel.heat_mode == heat_debug_frst_half ||
					solder_channel.heat_mode == heat_debug_botm_half)
			{
			}
			else
#endif
			{
				solder_channel.heat_ctrl_flag = ctrl_heat_off;
				solder_channel.heat_state = heat_off;
				CH_HeatOff;
			}

			// 实际执行
			if (solder_channel.heat_mode == heat_test)
			{
			}
			else if ((solder_channel.heat_mode == heat_norm ||
					solder_channel.heat_mode == heat_norm_frst_half) &&
					solder_channel.heat_ctrl_flag == ctrl_heat_on)
			{
				solder_channel.heat_state = heat_on;
				CH_HeatOn;
			}
			else
			{
				solder_channel.heat_state = heat_off;
				CH_HeatOff;
			}

			if (solder_channel.heat_time_frst_half > 0)
			{
				bsp_ctrl_tim_start_once(solder_channel.heat_time_frst_half);
				solder.ctrl_state = ctrl_frst_half;
				solder_channel.ctrl_state = ctrl_frst_half;
			}

			/*****************  debug code  *****************/
#if DEBUG_CMD_CTROL == 1 && DEBUG_TEMP_DETAILE_OUTPUT == 1

			if (ch_logic_debug.adc_temp_detail_flag == 1)
			{
				sprintf(debug_string_ctrl_0, "temp_detail:");

				for (debug_i = 0; debug_i < CH_TEMP_DMA_LENGTH; debug_i++)
				{
					//							debug_detail_temp_mv[debug_i]
					//=
					// get_temp_mv((float)adc_regular_dma_state.adc_val[debug_i]);
					debug_detail_temp[debug_i] =
						get_temp((float)adc_regular_dma_state.adc_val[debug_i],
								 solder_channel.tool_type);
					sprintf(debug_string_ctrl_1, "%6.2f ",
							debug_detail_temp[debug_i]);
					strcat(debug_string_ctrl_0, debug_string_ctrl_1);
				}

				sprintf(debug_string_ctrl_1, "*%6.2f  %6.2f\n",
						solder_channel.temp_current,
						adc_regular_dma_state.adc_average);
				strcat(debug_string_ctrl_0, debug_string_ctrl_1);
				log_info(debug_string_ctrl_0);
			}

#endif
#if DEBUG_CMD_CTROL == 1 && DEBUG_TEMP_ARRAY_OUTPUT == 1

			if (ch_logic_debug.temp_filt_array_flag == 1)
			{
				//						solder_channel.temp_show
				//=	get_temp_mv(adc_regular_dma_state.adc_average);
				//						solder_channel.temp_current
				//=	get_temp(adc_regular_dma_state.adc_average);
				//						queue_status =
				// queue_push(&debug_mv_queue,
				// (void*)&solder_channel.temp_safe);
				//						if(queue_status
				//== QUEUE_OVERLOAD)
				// log_info("queue overload\n");
				queue_status = queue_push(
					&debug_temperature_queue,
					(void *)&solder_channel.controller.temp_target_slope_climb);

				if (queue_status == QUEUE_OVERLOAD)
				{
					log_info("queue overload\n");
				}

				queue_status =
					queue_push(&debug_filted_temp_x1_queue,
							   (void *)&solder_channel.controller.temp_current);

				if (queue_status == QUEUE_OVERLOAD)
				{
					log_info("queue overload\n");
				}

				queue_status =
					queue_push(&debug_filted_temp_x2_queue,
							   (void *)&solder_channel.controller.temp_ested);

				if (queue_status == QUEUE_OVERLOAD)
				{
					log_info("queue overload\n");
				}

				//								queue_status
				//= queue_push(&debug_filted_temp_show_queue,
				//(void*)&solder_channel.temp_show);
				// if(queue_status == QUEUE_OVERLOAD)
				// log_info("queue overload\n");
				queue_status =
					queue_push(&debug_state_queue,
							   (void *)solder_channel.heat_flag_last_cycle);

				if (queue_status == QUEUE_OVERLOAD)
				{
					log_info("queue overload\n");
				}

				//					queue_status =
				// queue_push(&debug_cnts_queue_1,
				//(void*)&solder_channel.heat_ctrl_flag);
				// if(queue_status == QUEUE_OVERLOAD)
				// log_info("queue overload\n");
				// queue_status = queue_push(&debug_cnts_queue_2,
				//(void*)&controller_210.heat_urgent_gap_times);
				// if(queue_status
				//== QUEUE_OVERLOAD)
				// log_info("queue overload\n");
				//						sprintf(debug_string_ctrl_0,
				//"push temp data: %4.2f   length: %d\n",
				//																				solder_channel.temp_avrage,
				// debug_temperature_queue.data_len);
				//						log_info(debug_string_ctrl_0);
			}

#endif
			break;

		default:
			switch (solder_channel.ctrl_state)
			{
			case ctrl_frst_half:
#if DEBUG_CMD_CTROL == 1
#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_LED == 1
#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_STRING == 1
				if (ch_logic_debug.ctrl_process_flag == 1)
				{
					log_info("ch_tim: ctrl first half\n");
				}

#endif

				if (solder_channel.heat_mode == heat_norm_frst_half)
				{
					CH_HeatOff; // 关闭加热
					solder_channel.heat_state = heat_off;
				}
				else if ((solder_channel.heat_mode == heat_norm_botm_half ||
						  solder_channel.heat_mode == heat_calibration ||
						  solder_channel.heat_mode == heat_min_sleep) &&
						 solder_channel.heat_ctrl_flag == ctrl_heat_on)
				{
					CH_HeatOn;
					solder_channel.heat_state = heat_on;
				}

				if (solder_channel.heat_time_botm_half > 0)
				{
					bsp_ctrl_tim_start_once(solder_channel.heat_time_botm_half);
					solder.ctrl_state = ctrl_botm_half;
					solder_channel.ctrl_state = ctrl_botm_half;
				}

				break;

			case ctrl_botm_half:
#if DEBUG_CTROL_PROCESS_OUTPUT_LED == 1
				WORK_LED_ON;
#endif
#if DEBUG_CTROL_PROCESS_OUTPUT_STRING == 1

				if (ch_logic_debug.ctrl_process_flag == 1)
				{
					log_info("ch_tim: ctrl bottom half\n");
				}

#endif
				CH_HeatOff;
				solder_channel.heat_state = heat_off;
				solder.ctrl_state = ctrl_ideal;
				solder_channel.ctrl_state = ctrl_ideal;
				break;

			default:
				break;
			}

			break;
		}
	}
}
void heat_temperature_error(volatile solder_ch_state_t *ch)
{
	if (ch->heat_flag_last_cycle == ctrl_heat_on)
	{
		switch (ch->tool_type)
		{
		case tool_115:
			if (ch->controller.temp_ested > 200)
			{
				if (ch->temp_current_last_cycle + 1.0f > ch->temp_current)
				{
					ch->ch_err_flag |= ERROR_TEMP_ERROR;
					ch->heat_mode = heat_none;
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

					if (ch_logic_debug.stage_flag == 1)
					{
						log_info("temperature error 115!\n");
					}

#endif
					ch->ch_temp_error_continue_cnt++;
					ch->ch_temp_error_continue_gap_cnt = 0;

					if (ch->ch_temp_error_continue_cnt > TEMP_ERROR_CONT_MAX)
					{
						ch->ch_err_flag |= ERROR_TEMP_TOOL_ERROR;
					}
				}
				else
				{
					ch->ch_temp_error_continue_gap_cnt++;

					if (ch->ch_temp_error_continue_gap_cnt > 0)
					{
						ch->ch_temp_error_continue_cnt = 0;
					}
				}
			}

			break;

		case tool_210:
			if (ch->controller.temp_ested > 300)
			{
				if (ch->temp_current_last_cycle > ch->temp_current)
				{
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

					if (ch_logic_debug.stage_flag == 1)
					{
						log_info("temperature error 210!\n");
					}

#endif
					ch->ch_temp_error_continue_cnt++;
					ch->ch_temp_error_continue_gap_cnt = 0;

					if (ch->ch_temp_error_continue_cnt > 2)
					{
						ch->ch_err_flag |= ERROR_TEMP_ERROR;
						ch->heat_mode = heat_none;
					}

					if (ch->ch_temp_error_continue_cnt > TEMP_ERROR_CONT_MAX)
					{
						ch->ch_err_flag |= ERROR_TEMP_TOOL_ERROR;
						ch->heat_mode = heat_none;
					}
				}
				else
				{
					ch->ch_temp_error_continue_gap_cnt++;

					if (ch->ch_temp_error_continue_gap_cnt > 0)
					{
						ch->ch_temp_error_continue_cnt = 0;
					}
				}
			}

			break;

		case tool_245:
			if (ch->controller.temp_ested > 480)
			{
				if ((ch->temp_current_last_cycle - 1.0f) > ch->temp_current)
				{
#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1

					if (ch_logic_debug.stage_flag == 1)
					{
						log_info("temperature error 245!\n");
					}

#endif
					ch->ch_temp_error_continue_cnt++;
					ch->ch_temp_error_continue_gap_cnt = 0;

					if (ch->ch_temp_error_continue_cnt > 5)
					{
						ch->ch_err_flag |= ERROR_TEMP_ERROR;
						ch->heat_mode = heat_none;
					}

					if (ch->ch_temp_error_continue_cnt > TEMP_ERROR_CONT_MAX)
					{
						ch->ch_err_flag |= ERROR_TEMP_TOOL_ERROR;
					}
				}
				else
				{
					ch->ch_temp_error_continue_gap_cnt++;

					if (ch->ch_temp_error_continue_gap_cnt > 0)
					{
						ch->ch_temp_error_continue_cnt = 0;
					}
				}
			}

			break;

		default:
			break;
		}
	}
	else
	{
		ch->ch_temp_error_continue_gap_cnt++;

		if (ch->ch_temp_error_continue_gap_cnt > 3)
		{
			ch->ch_temp_error_continue_cnt = 0;
			ch->ch_temp_error_continue_gap_cnt = 0;
		}
	}
}
/*
根据不同的工具把校准温度存储进calibration_buf数组
*/
void calibration_param_to_calibration_buf(tool_type_t tool, volatile float *calibration_temp_buf)
{
	if (tool == tool_115)
	{
		solder_param_get_from_array((void *)&calibration_temp_buf[0],
									param_temp_cal_115_100);
		solder_param_get_from_array((void *)&calibration_temp_buf[1],
									param_temp_cal_115_200);
		solder_param_get_from_array((void *)&calibration_temp_buf[2],
									param_temp_cal_115_300);
		solder_param_get_from_array((void *)&calibration_temp_buf[3],
									param_temp_cal_115_400);
		solder_param_get_from_array((void *)&solder_channel.cal_115_flag,
									param_temp_cal_115_flag);
	}
	else if (tool == tool_210)
	{
		solder_param_get_from_array((void *)&calibration_temp_buf[0],
									param_temp_cal_210_100);
		solder_param_get_from_array((void *)&calibration_temp_buf[1],
									param_temp_cal_210_200);
		solder_param_get_from_array((void *)&calibration_temp_buf[2],
									param_temp_cal_210_300);
		solder_param_get_from_array((void *)&calibration_temp_buf[3],
									param_temp_cal_210_400);
		solder_param_get_from_array((void *)&solder_channel.cal_210_flag,
									param_temp_cal_210_flag);
	}
	else if (tool == tool_245)
	{
		solder_param_get_from_array((void *)&calibration_temp_buf[0],
									param_temp_cal_245_100);
		solder_param_get_from_array((void *)&calibration_temp_buf[1],
									param_temp_cal_245_200);
		solder_param_get_from_array((void *)&calibration_temp_buf[2],
									param_temp_cal_245_300);
		solder_param_get_from_array((void *)&calibration_temp_buf[3],
									param_temp_cal_245_400);
		solder_param_get_from_array((void *)&solder_channel.cal_245_flag,
									param_temp_cal_245_flag);
	}
}
/*
根据不同的工具把校准温度读校准温度数组里面
*/
void calibration_param_to_array(tool_type_t tool)
{
	if (tool == tool_115)
	{
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[0],
			param_temp_cal_115_100);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[1],
			param_temp_cal_115_200);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[2],
			param_temp_cal_115_300);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[3],
			param_temp_cal_115_400);
		solder_channel.cal_115_flag = 1;
		solder_param_write_to_array((uint8_t *)&solder_channel.cal_115_flag,
									param_temp_cal_115_flag);
	}
	else if (tool == tool_210)
	{
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[0],
			param_temp_cal_210_100);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[1],
			param_temp_cal_210_200);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[2],
			param_temp_cal_210_300);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[3],
			param_temp_cal_210_400);
		solder_channel.cal_210_flag = 1;
		solder_param_write_to_array((uint8_t *)&solder_channel.cal_210_flag,
									param_temp_cal_210_flag);
	}
	else if (tool == tool_245)
	{
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[0],
			param_temp_cal_245_100);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[1],
			param_temp_cal_245_200);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[2],
			param_temp_cal_245_300);
		solder_param_write_to_array(
			(uint8_t *)&solder_channel.ch_set_calibration_temp[3],
			param_temp_cal_245_400);
		solder_channel.cal_245_flag = 1;
		solder_param_write_to_array((uint8_t *)&solder_channel.cal_245_flag,
									param_temp_cal_245_flag);
	}
}
void calibration_param_to_rom(tool_type_t tool)
{
	if (tool == tool_115)
	{
		solder_param_write_to_rom(param_temp_cal_115_100);
		solder_param_write_to_rom(param_temp_cal_115_200);
		solder_param_write_to_rom(param_temp_cal_115_300);
		solder_param_write_to_rom(param_temp_cal_115_400);
		solder_channel.cal_115_flag = 1;
		solder_param_write_to_rom(param_temp_cal_115_flag);
	}
	else if (tool == tool_210)
	{
		solder_param_write_to_rom(param_temp_cal_210_100);
		solder_param_write_to_rom(param_temp_cal_210_200);
		solder_param_write_to_rom(param_temp_cal_210_300);
		solder_param_write_to_rom(param_temp_cal_210_400);
		solder_channel.cal_210_flag = 1;
		solder_param_write_to_rom(param_temp_cal_210_flag);
	}
	else if (tool == tool_245)
	{
		solder_param_write_to_rom(param_temp_cal_245_100);
		solder_param_write_to_rom(param_temp_cal_245_200);
		solder_param_write_to_rom(param_temp_cal_245_300);
		solder_param_write_to_rom(param_temp_cal_245_400);
		solder_channel.cal_245_flag = 1;
		solder_param_write_to_rom(param_temp_cal_245_flag);
	}
}
