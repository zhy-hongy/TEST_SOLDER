#include "solder_error.h"
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

