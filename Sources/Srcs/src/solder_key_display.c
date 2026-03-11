#include "solder.h"
#include "solder_error.h"
extern volatile key_ntf_t key_ntf_cmd;
static uint32_t systemtime_last_calibration = 0; // 上一次校准模式完成时间
static uint32_t systemtime_last_set_key = 0;	 // 校验模式步进时间间隔控制

static void enter_set_mode(ch_set_mode_t mode);
extern float calibration_target_temp_transfer; // 进入校准模式，临时存储目标温度

extern float min_target_temp_transfer; // 进入睡眠最低温度模式，临时存储目标温度

extern float temp_target_temp_transfer; // 临时存储睡眠温度的改变量

void screen_display()
{
	uint16_t temp_tmp;

	//	temp_tmp = (int)solder_channel.temp_target;
	//	display_temp_set(temp_tmp);
	if (solder_channel.ch_stage == ch_set)
	{
		if (solder_channel.ch_set_mode == ch_set_mode_up_down_inc ||
			solder_channel.ch_set_mode == ch_set_mode_min_temp ||
			solder_channel.ch_set_mode == ch_set_mode_ch1 ||
			solder_channel.ch_set_mode == ch_set_mode_ch2 ||
			solder_channel.ch_set_mode == ch_set_mode_ch3)
		{
			temp_tmp = (int)solder_channel.ch_set_temp_disp;
			display_temp_show(temp_tmp);
			temp_tmp = (int)solder_channel.ch_set_set_disp;
			display_temp_set(temp_tmp);
		}

		if (solder_channel.ch_set_mode == ch_set_mode_calibration)
		{
			temp_tmp = (int)solder_channel.ch_set_temp_disp;

			if (temp_tmp <= 0)
			{
				temp_tmp = (int)solder_channel.temp_show;
			}

			display_temp_show(temp_tmp);
			temp_tmp = (int)solder_channel.ch_set_set_disp;
			display_temp_set(temp_tmp);
			// display_unitF_flag(1);
			display_unitC_flag(1);
		}
	}
	else
	{
		temp_tmp = (int)solder_channel.temp_show; // solder_channel.temp_show;

		if (solder_channel.ch_stage == ch_working ||
			solder_channel.ch_stage == ch_sleep ||
			(solder_channel.ch_stage == ch_err &&
			 solder_channel.ch_err_flag == ERROR_TEMP_ERROR))
		{
			//		if(solder_channel.temp_show < 100){
			//			display_temp_show(100);
			//		}else{
			display_temp_show(temp_tmp);
			//		}
		}
		else
		{
			display_temp_show(-200);
		}

		if (solder_channel.ch_stage == ch_sleep && solder_channel.min_temp_enable_flag == 1 && solder_channel.ch_sleep_stage_frstin_flag == 1)
		{
			temp_tmp = (int)(min_target_temp_transfer + temp_target_temp_transfer);
		}
		else
		{
			temp_tmp = (int)solder_channel.temp_target;
		}

		if (solder_channel.ch_stage == ch_working ||
			(solder_channel.ch_stage == ch_err &&
			 solder_channel.ch_err_flag == ERROR_TEMP_ERROR))
		{
			display_temp_set(temp_tmp);
		}
		else
		{
			if (solder.disp_maintain_state > 0)
			{
				display_temp_set(temp_tmp);
				solder.disp_flicker_cnt++;

				if (solder.disp_flicker_cnt > 10)
				{
					solder.disp_flicker_cnt = 0;
					solder.disp_maintain_state = 0;
				}
			}
			else
			{
				if (solder.disp_flicker_cnt > 6)
				{
					solder.disp_flicker_cnt = 0;

					if (solder.disp_flicker_state > 0)
					{
						display_temp_set((int16_t)-200);
						solder.disp_flicker_state = 0;
					}
					else
					{
						display_temp_set(temp_tmp);
						solder.disp_flicker_state = 1;
					}
				}
				else
				{
					solder.disp_flicker_cnt++;
				}
			}
		}
	}

	display_power_gear(solder_channel.power_gear);

	if ((solder.err_flag == ERROR_NONE &&
		 solder_channel.ch_err_flag == ERROR_NONE) ||
		solder_channel.ch_err_flag == ERROR_TEMP_ERROR)
	{
		display_error_flag(0);
	}
	else
	{
		display_error_flag(1);
	}

	if (solder_channel.tool_in_state == tool_out)
	{
		display_no_tool_flag(1);
	}
	else
	{
		display_no_tool_flag(0);
	}

	if (solder_channel.power_high_flag > 0)
	{
		display_unitH_flag(1);
	}
	else
	{
		display_unitH_flag(0);
	}

	display_fix_flag(1);
	display_unitC_flag(1);
	lcd_scan();
}

void screen_display_test()
{
	display_temp_show(888);
	display_temp_set(888);
	display_error_flag(1);
	display_no_tool_flag(1);
	display_unitH_flag(1);
	display_error_flag(1);
	display_fix_flag(1);
	display_unitC_flag(1);
	display_power_gear(8);
	lcd_scan();
}

static void handle_calibration_mode(void)
{
	solder_channel.ch_set_temp_disp =
		solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step];

	switch (solder_channel.ch_set_step)
	{
	case calibration_step_100:
		solder_channel.ch_set_set_disp = 100;
		solder_channel.temp_target = 100;
		break;
	case calibration_step_200:
		solder_channel.ch_set_set_disp = 200;
		solder_channel.temp_target = 200;
		break;
	case calibration_step_300:
		solder_channel.ch_set_set_disp = 300;
		solder_channel.temp_target = 300;
		break;
	case calibration_step_400:
		solder_channel.ch_set_set_disp = 400;
		solder_channel.temp_target = 400;
		break;
	default:
		break;
	}

	if (solder_channel.ch_frstin_set_temp_flag == 1)
	{
		key_ntf_cmd.temp_target_chg_ntf = 1;
		solder_channel.ch_frstin_set_temp_flag = 0;
	}
}
// 处理设置键在校准模式、步进模式或睡眠最低温度模式下的操作
static void handle_set_key_in_set_mode(void)
{
	if (solder_channel.ch_set_mode == ch_set_mode_up_down_inc ||
		solder_channel.ch_set_mode == ch_set_mode_ch1 ||
		solder_channel.ch_set_mode == ch_set_mode_ch2 ||
		solder_channel.ch_set_mode == ch_set_mode_ch3)
	{
		solder_channel.set_complete_flag = 1;
	}

	if (solder_channel.ch_set_mode == ch_set_mode_min_temp)
	{
		solder_channel.set_complete_flag = 1;
		if (solder_channel.ch_set_temp_disp > 25)
		{
			solder_channel.min_temp_enable_flag = 1;
		}
		else
		{
			solder_channel.min_temp_enable_flag = 0;

			// 退出睡眠最低温度模式下，恢复目标温度加上改变量
			solder_channel.temp_target = min_target_temp_transfer + temp_target_temp_transfer;
			key_ntf_cmd.temp_target_chg_ntf = 1;
		}
	}
	if (solder_channel.ch_set_mode == ch_set_mode_calibration &&
		timeDifference(getSystemTime(), systemtime_last_set_key) > 500)
	{
		solder_channel.ch_frstin_set_temp_flag = 1;

		systemtime_last_set_key = getSystemTime(); // 更新校验模式步进时间间隔控制

		if (solder_channel.ch_set_step == 3)
		{
			// 校准参数验证
			if (solder_channel.ch_set_calibration_temp[0] > 150 ||
				solder_channel.ch_set_calibration_temp[0] < 50 ||
				solder_channel.ch_set_calibration_temp[1] > 250 ||
				solder_channel.ch_set_calibration_temp[1] < 150 ||
				solder_channel.ch_set_calibration_temp[2] > 350 ||
				solder_channel.ch_set_calibration_temp[2] < 250 ||
				solder_channel.ch_set_calibration_temp[3] > 450 ||
				solder_channel.ch_set_calibration_temp[3] < 350 ||
				solder_channel.ch_set_calibration_temp[0] > solder_channel.ch_set_calibration_temp[1] ||
				solder_channel.ch_set_calibration_temp[1] > solder_channel.ch_set_calibration_temp[2] ||
				solder_channel.ch_set_calibration_temp[2] > solder_channel.ch_set_calibration_temp[3])
			{
				solder_channel.ch_set_step = 0;
			}
			else
			{
				solder_channel.set_complete_flag = 1;
				clear_key_third_flag(KEY_SET);
				systemtime_last_calibration = getSystemTime(); // 更新上一次校准模式完成时间
				calibration_param_to_array(solder_channel.tool_type);
				init_all_temp_tab();
				solder_channel.temp_target = calibration_target_temp_transfer;
				key_ntf_cmd.temp_target_chg_ntf = 1;
				solder_channel.ch_temp_cal_param_udpate_ntf = 1;
			}
		}
		solder_channel.ch_set_step++;
	}
}
// 处理设置模式下SET键的确认操作，包括校准参数验证和模式切换。
static void handle_add_key_in_set_mode(void)
{
	switch (solder_channel.ch_set_mode)
	{
	case ch_set_mode_up_down_inc:
		solder_channel.ch_set_temp_disp += 1;
		if (solder_channel.ch_set_temp_disp > 10)
		{
			solder_channel.ch_set_temp_disp = 1;
		}
		break;
	case ch_set_mode_min_temp:
		solder_channel.ch_set_temp_disp += solder_channel.temp_target_inc;
		if (solder_channel.ch_set_temp_disp > 300)
			solder_channel.ch_set_temp_disp = 300;
		break;
	case ch_set_mode_ch1:
	case ch_set_mode_ch2:
	case ch_set_mode_ch3:
		solder_channel.ch_set_temp_disp += solder_channel.temp_target_inc;
		if (solder_channel.ch_set_temp_disp > 500)
			solder_channel.ch_set_temp_disp = 500;
		break;
	case ch_set_mode_calibration:
		solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step]++;
		if (solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step] > 450)
		{
			solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step] = 450;
		}
		solder_channel.ch_set_temp_disp =
			solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step];
		break;
	default:
		break;
	}
	clear_key_notify(KEY_ADD);
}
// 处理设置键在设置模式下的操作
static void handle_sub_key_in_set_mode(void)
{
	switch (solder_channel.ch_set_mode)
	{
	case ch_set_mode_up_down_inc:
		solder_channel.ch_set_temp_disp = (solder_channel.ch_set_temp_disp == 1) ? 10 : (solder_channel.ch_set_temp_disp - 1);
		break;
	case ch_set_mode_min_temp:
		solder_channel.ch_set_temp_disp -= solder_channel.temp_target_inc;
		if (solder_channel.ch_set_temp_disp < 0)
			solder_channel.ch_set_temp_disp = 0;
		break;
	case ch_set_mode_ch1:
	case ch_set_mode_ch2:
	case ch_set_mode_ch3:
		solder_channel.ch_set_temp_disp -= solder_channel.temp_target_inc;
		if (solder_channel.ch_set_temp_disp < 100)
			solder_channel.ch_set_temp_disp = 100;
		break;
	case ch_set_mode_calibration:
		solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step]--;
		if (solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step] < 50)
		{
			solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step] = 50;
		}
		solder_channel.ch_set_temp_disp = solder_channel.ch_set_calibration_temp[solder_channel.ch_set_step];
		break;
	default:
		break;
	}
	clear_key_notify(KEY_SUB);
}
// 处理设置键在正常模式下的操作
static void handle_set_key_in_normal_mode(void)
{
	solder.disp_maintain_state = 1;

	if (is_key_pressed_up_third(KEY_SET) &&
		solder_channel.tool_type != notool &&
		timeDifference(getSystemTime(), systemtime_last_calibration) > 1000)
	{
		// systemtime_last_calibration = getSystemTime();
		// 进入校准模式
		solder_channel.ch_last_stage = solder_channel.ch_stage;
		solder_channel.ch_stage = ch_set;
		solder_channel.ch_stage_frstin_flag = 1;
		solder_channel.ch_set_stage = ch_set_stage_ideal;
		solder_channel.ch_set_mode = ch_set_mode_calibration;
		solder_channel.ch_set_step = calibration_step_100;
		systemtime_last_set_key = getSystemTime(); // 更新校验模式步进时间间隔控制
		solder_channel.ch_set_timeout_max = 12000; // 设置2分钟超时

		calibration_param_to_calibration_buf(
			solder_channel.tool_type, solder_channel.ch_set_calibration_temp);

#if DEBUG_SET_MODE_TEST == 1
		sprintf((char *)debug_string_out_0, "^&^100:%d\n",
				(uint32_t)solder_channel.ch_set_calibration_temp[0]);
		log_info((char *)debug_string_out_0);
		sprintf((char *)debug_string_out_0, "^&^200:%d\n",
				(uint32_t)solder_channel.ch_set_calibration_temp[1]);
		log_info((char *)debug_string_out_0);
		sprintf((char *)debug_string_out_0, "^&^300:%d\n",
				(uint32_t)solder_channel.ch_set_calibration_temp[2]);
		log_info((char *)debug_string_out_0);
		sprintf((char *)debug_string_out_0, "^&^400:%d\n",
				(uint32_t)solder_channel.ch_set_calibration_temp[3]);
		log_info((char *)debug_string_out_0);
		sprintf((char *)debug_string_out_0, "flag:%d\n",
				solder_channel.cal_245_flag);
		log_info((char *)debug_string_out_0);
#endif
		solder_param_get_from_array((void *)&calibration_target_temp_transfer,
									param_target_temp);
		solder_channel.ch_frstin_set_temp_flag = 1;
		solder_channel.heat_mode = heat_calibration;
		solder_channel.ch_set_step = 0;
		clear_key_notify(KEY_SET);
	}
	else if (get_key_notify(KEY_SET) == 3)
	{ // 长按set建// 进入高功率模式
		solder_channel.ch_last_stage = solder_channel.ch_stage;
		solder_channel.ch_stage = ch_set;
		solder_channel.ch_stage_frstin_flag = 1;
		solder_channel.ch_set_stage = ch_set_stage_ideal;
		solder_channel.ch_set_mode = ch_set_mode_power_high;
		solder_channel.set_complete_flag = 1;
		clear_key_notify(KEY_SET);
	}
	else if (is_double_key_pressed(KEY_SET, KEY_ADD))
	{
		// 进入温度增量设置模式
		enter_set_mode(ch_set_mode_up_down_inc);
		clear_key_notify(KEY_ADD);
		clear_key_notify(KEY_SET);
	}

	else if (is_double_key_pressed(KEY_SET, KEY_SUB))
	{
		// 进入休眠温度设置模式
		enter_set_mode(ch_set_mode_min_temp);
		clear_key_notify(KEY_SUB);
		clear_key_notify(KEY_SET);
	}

	// else if (is_double_key_pressed(KEY_SET, KEY_SUB))
	// {
	// 	// 进入温度增量设置模式
	// 	enter_set_mode(ch_set_mode_up_down_inc);
	// 	clear_key_notify(KEY_SUB);
	// 	clear_key_notify(KEY_SET);
	// }

#ifdef THT_M120
	if (is_double_key_pressed(KEY_SET, KEY_CH1))
	{
		enter_set_mode(ch_set_mode_ch1);
		clear_key_notify(KEY_CH1);
		clear_key_notify(KEY_SET);
	}
	if (is_double_key_pressed(KEY_SET, KEY_CH2))
	{
		enter_set_mode(ch_set_mode_ch2);
		clear_key_notify(KEY_CH2);
		clear_key_notify(KEY_SET);
	}
	if (is_double_key_pressed(KEY_SET, KEY_CH3))
	{
		enter_set_mode(ch_set_mode_ch3);
		clear_key_notify(KEY_CH3);
		clear_key_notify(KEY_SET);
	}
#endif
}
// 进入设置模式并初始化相关参数，根据模式类型加载对应的温度参数。
static void enter_set_mode(ch_set_mode_t mode)
{
	solder_channel.ch_last_stage = solder_channel.ch_stage;
	solder_channel.ch_stage = ch_set;
	solder_channel.ch_stage_frstin_flag = 1;
	solder_channel.ch_set_stage = ch_set_stage_ideal;
	solder_channel.ch_set_mode = mode;
	solder_channel.ch_set_timeout_max = 1000;
	switch (mode)
	{
	case ch_set_mode_up_down_inc:
		solder_param_get_from_array((void *)&solder_channel.ch_set_temp_disp,
									param_target_temp_inc);
		break;
	case ch_set_mode_ch1:
		solder_param_get_from_array((void *)&solder_channel.ch_set_temp_disp,
									param_target_temp_ch1);
		break;
	case ch_set_mode_ch2:
		solder_param_get_from_array((void *)&solder_channel.ch_set_temp_disp,
									param_target_temp_ch2);
		break;
	case ch_set_mode_ch3:
		solder_param_get_from_array((void *)&solder_channel.ch_set_temp_disp,
									param_target_temp_ch3);
		break;
	case ch_set_mode_min_temp:
		solder_param_get_from_array((void *)&solder_channel.ch_set_temp_disp,
									param_min_temp);
		break;
	default:
		break;
	}
	solder_channel.ch_set_set_disp = solder_channel.ch_set_temp_disp;
}
// 处理加键在正常模式下的操作
static void handle_add_key_in_normal_mode(void)
{
	solder.disp_maintain_state = 1;
	if (solder_channel.ch_stage == ch_sleep && solder_channel.min_temp_enable_flag == 1)
	{
		temp_target_temp_transfer += solder_channel.temp_target_inc;
		if (min_target_temp_transfer + temp_target_temp_transfer > 450)
		{
			temp_target_temp_transfer = 450 - min_target_temp_transfer;
		}
	}
	else
	{
		solder_channel.temp_target += solder_channel.temp_target_inc;
		if (solder_channel.temp_target > 450)
			solder_channel.temp_target = 450;
		key_ntf_cmd.temp_target_chg_ntf = 1;
	}
	clear_key_notify(KEY_ADD);
}

// 处理减键在正常模式下的操作
static void handle_sub_key_in_normal_mode(void)
{
	solder.disp_maintain_state = 1;

	if (solder_channel.ch_stage == ch_sleep && solder_channel.min_temp_enable_flag == 1)
	{
		temp_target_temp_transfer -= solder_channel.temp_target_inc;
		if (min_target_temp_transfer + temp_target_temp_transfer < 100)
		{
			temp_target_temp_transfer = 100 - min_target_temp_transfer;
		}
	}
	else
	{
		solder_channel.temp_target -= solder_channel.temp_target_inc;
		if (solder_channel.temp_target < 100)
			solder_channel.temp_target = 100;
		key_ntf_cmd.temp_target_chg_ntf = 1;
	}
	clear_key_notify(KEY_SUB);
}

// 处理通道选择键（CH1~CH3）在正常模式下的操作 
static void handle_channel_keys_in_normal_mode(void) 
{ 
#ifdef THT_M120 
    float ch_temp_before_offset = 0;
    const key_num_t channel_keys[] = {KEY_CH1, KEY_CH2, KEY_CH3};
    const param_name_t temp_params[] = {param_target_temp_ch1, param_target_temp_ch2, param_target_temp_ch3};
    const uint8_t channel_count = sizeof(channel_keys) / sizeof(channel_keys[0]);
    
    for (uint8_t i = 0; i < channel_count; i++) {
        if (get_key_notify(channel_keys[i]) == 1) {
            solder.disp_maintain_state = 1;
            
            if (solder_channel.ch_stage == ch_sleep && solder_channel.min_temp_enable_flag == 1) {
                solder_param_get_from_array((void *)&ch_temp_before_offset, temp_params[i]);
                temp_target_temp_transfer = (ch_temp_before_offset - min_target_temp_transfer);
            } else {
                solder_param_get_from_array((void *)&solder_channel.temp_target, temp_params[i]);
                key_ntf_cmd.temp_target_chg_ntf = 1;
            }
            
            clear_key_notify(channel_keys[i]);
        }
    }
#endif 
}
//
static void clear_all_key_notifications(void)
{
	if (get_key_notify(KEY_SET) < 0)
	{
		if (solder_channel.ch_set_mode == ch_set_mode_power_high)
		{
			solder_channel.set_complete_flag = 1;
		}
		clear_key_notify(KEY_SET);
	}
	if (get_key_notify(KEY_ADD) < 0)
		clear_key_notify(KEY_ADD);
	if (get_key_notify(KEY_SUB) < 0)
		clear_key_notify(KEY_SUB);

#ifdef THT_M120
	if (get_key_notify(KEY_CH1) < 0)
		clear_key_notify(KEY_CH1);
	if (get_key_notify(KEY_CH2) < 0)
		clear_key_notify(KEY_CH2);
	if (get_key_notify(KEY_CH3) < 0)
		clear_key_notify(KEY_CH3);
#endif
}

// 主按键处理函数
void key_process(void)
{
	// if (timeDifference(getSystemTime(), systemtime_last_calibration) < 2000)
	// {
	// 	clear_key_notify(KEY_ADD);
	// 	clear_key_notify(KEY_SUB);
	// }

	// 处理校准模式
	if (solder_channel.ch_set_mode == ch_set_mode_calibration)
	{
		handle_calibration_mode();
	}

	// 根据当前状态处理按键
	if (solder_channel.ch_stage == ch_set)
	{
		// 设置模式下的按键处理
		if (get_key_notify(KEY_SET) == 1 && get_key_notify(KEY_ADD) <= 0 &&
			get_key_notify(KEY_SUB) <= 0
#ifdef THT_M120
			&&get_key_notify(KEY_CH1) <= 0 && get_key_notify(KEY_CH2) <= 0 &&
				get_key_notify(KEY_CH3) <= 0
#endif
		)
			{
				handle_set_key_in_set_mode();
				solder_channel.ch_set_timeout_cnt = 0; // 重置计数器
				clear_key_notify(KEY_SET);
			}

		if (is_double_key_pressed(KEY_SUB,
								  KEY_ADD))
		{										   // 同时检查到加减键按下，清除通知
			solder_channel.ch_set_timeout_cnt = 0; // 重置计数器
			clear_key_notify(KEY_SUB);
			clear_key_notify(KEY_ADD);
		}

		if (get_key_notify(KEY_ADD) > 0)
		{
			handle_add_key_in_set_mode();
			solder_channel.ch_set_timeout_cnt = 0; // 重置计数器
		}

		if (get_key_notify(KEY_SUB) > 0)
		{
			handle_sub_key_in_set_mode();
			solder_channel.ch_set_timeout_cnt = 0; // 重置计数器
		}
	}
	else
	{
		// 正常模式下的按键处理
		if (get_key_notify(KEY_SET) > 0)
		{
			handle_set_key_in_normal_mode();
		}
		else
		{
			if (is_double_key_pressed(KEY_SUB, KEY_ADD))
			{ // 同时检查到加减键按下，清除通知
				clear_key_notify(KEY_SUB);
				clear_key_notify(KEY_ADD);
			}
			if (solder_channel.ch_sleep_stage_frstin_flag == 0 
				&& (get_key_notify(KEY_ADD) > 0 
				|| get_key_notify(KEY_SUB) > 0)
				#ifdef THT_M120
				||get_key_notify(KEY_CH1) > 0 
				|| get_key_notify(KEY_CH2) > 0 
				|| get_key_notify(KEY_CH3) > 0
				#endif
			)
			{
				// temp_target_temp_transfer = min_target_temp_transfer;
				// key_ntf_cmd.temp_target_chg_ntf = 1;
				solder_channel.ch_sleep_stage_frstin_flag = 1;
				// solder_channel.ch_set_timeout_max = 600;
			}
			if (get_key_notify(KEY_ADD) > 0)
			{
				// solder_channel.ch_sleep_timeout_cnt = 0; // 重置计数器
				handle_add_key_in_normal_mode();
			}

			if (get_key_notify(KEY_SUB) > 0)
			{
				// solder_channel.ch_sleep_timeout_cnt = 0; // 重置计数器
				handle_sub_key_in_normal_mode();
			}

			handle_channel_keys_in_normal_mode();
		}
	}

	// 处理温度变化通知
	if (key_ntf_cmd.temp_target_chg_ntf == 1)
	{
		key_ntf_cmd.temp_target_chg_ntf = 0;
		ctrl_params_update_from_target_temp(&solder_channel.controller,
											solder_channel.temp_target);

		if (solder_channel.ch_set_mode != ch_set_mode_calibration)
		{
			solder_channel.ch_temp_param_update_ntf = 1;
			solder_param_write_to_array((void *)&solder_channel.temp_target,
										param_target_temp);
		}
		solder.disp_maintain_state = 1;
	}

	// 清除所有按键通知
	clear_all_key_notifications();
}
