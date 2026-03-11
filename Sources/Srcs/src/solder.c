#define CONFIG_DEBUG_ON
#include "solder.h"

#include "temperature_cal.h"
#include "solder_error.h"
#include "solder_param.h"
#include "solder_power.h"
#include "solder_tool.h"
#include "solder_stage.h"
#include "solder_temp.h"


// #define PARAM_WRITE
solder_param_info_t solder_param_info[512];

uint8_t solder_param[512];

volatile uint32_t zero_up_irq_tim_record = 0, zero_down_irq_tim_record = 0;

volatile key_ntf_t key_ntf_cmd;

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
