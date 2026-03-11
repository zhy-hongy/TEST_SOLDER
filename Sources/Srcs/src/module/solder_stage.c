#include "solder_stage.h"
#include "solder_tool.h"
#include "solder_param.h"
#include "solder_error.h"
static uint16_t ch_err_wait_cnt = 0;
extern volatile key_ntf_t key_ntf_cmd;
extern float calibration_target_temp_transfer; // 进入校准模式，临时存储目标温度

extern float min_target_temp_transfer; // 进入睡眠最低温度模式，临时存储目标温度

extern float temp_target_temp_transfer; // 临时存储睡眠温度的改变量
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
