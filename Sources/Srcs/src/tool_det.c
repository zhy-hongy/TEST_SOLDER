#include "tool_io.h"
#include "tool_det.h"
#include "solder.h"
volatile tool_detect_data_t tool_detect_data = {

	.tool_in_chk_sig_bits_cnt = {0},
	.tool_in_chk_update_cnt = 0,
	.tool_in_disable_in_heat = 0,

	.tool_type_1_sig_bits_cnt = {0},
	.tool_type_1_update_cnt = 0,

	.tool_type_2_sig_bits_cnt = {0},
	.tool_type_2_update_cnt = 0,

	.sleep_sig_bits = sleep_sig_err,
	.sleep_sig_bits_cnt = {0},
	.sleep_update_cnt = 0,

	.tool_in_state = tool_out,
	.tool_type_1 = notool,
	.tool_type_2 = notool,
	.tool_type_3 = notool,
	.sleep_state = sleep_err};

// 在加热状态下不检测工具是否在位
void tool_detect_disable_in_heat_set(volatile tool_detect_data_t *tool_det)
{
	tool_det->tool_in_disable_in_heat = solder_channel.heat_ctrl_flag;
}
void tool_in_check_sig_update(volatile tool_detect_data_t *tool_det, uint8_t update_sw)
{
	uint8_t tmp;
	tool_detect_disable_in_heat_set(tool_det);
	if ((update_sw & CH_STATE_SW_TOOL_IN_ON) != CH_STATE_SW_OFF)
	{
		if (tool_det->tool_in_disable_in_heat == tool_heat_on)
		{
			return;
		}
		

		tmp = GPIO_ReadInputDataBit(TOOL_IN_PORT, TOOL_IN_PIN);

		if (tmp == RESET)
		{
			tool_det->tool_in_chk_sig_bits_cnt[0]++;
		}
		else
		{
			tool_det->tool_in_chk_sig_bits_cnt[1]++;
		}

		tool_det->tool_in_chk_update_cnt++;
		if (tool_det->tool_in_chk_update_cnt > SIGNAL_JITTER_FILTER_CNT_SYSTICK)
		{
			tool_det->tool_in_update_flag = 1;

			if (tool_det->tool_in_chk_sig_bits_cnt[0] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->tool_in_state = tool_in;
			}
			else
			{
				tool_det->tool_in_state = tool_out;
			}
			if (tool_det->tool_in_chk_sig_bits_cnt[1] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->tool_in_state = tool_out;
			}

			tool_det->tool_in_chk_sig_bits_cnt[0] = 0;
			tool_det->tool_in_chk_sig_bits_cnt[1] = 0;
			tool_det->tool_in_chk_update_cnt = 0;
		}
	}
	else
	{

		tool_det->tool_in_chk_sig_bits_cnt[0] = 0;
		tool_det->tool_in_chk_sig_bits_cnt[1] = 0;
		tool_det->tool_in_chk_update_cnt = 0;
		tool_det->tool_in_update_flag = 0;
	}
}

/**
 * @brief 工具类型1检测更新函数
 * @param tool_det 工具检测数据结构体指针
 * @param update_sw 更新开关，用于控制是否进行检测
 *
 * 功能说明：
 * - 检测焊台工具类型1（210℃和245℃工具）
 * - 通过GPIO读取工具类型检测引脚状态
 * - 使用抖动滤波算法消除信号抖动
 * - 根据检测结果确定工具类型（tool_115_210或tool_245）
 *
 * 检测逻辑：
 * - 当update_sw包含CH_STATE_SW_TOOL_TYPE_1_ON时进行检测
 * - 读取GPIO引脚状态，统计高低电平计数
 * - 达到滤波计数阈值后判断工具类型
 * - 低电平计数多时判定为tool_115_210（210℃工具）
 * - 高电平计数多时判定为tool_245（245℃工具）
 */
void tool_type_1_update(volatile tool_detect_data_t *tool_det, uint8_t update_sw)
{	
	uint8_t tmp;
  
	if( ( update_sw & CH_STATE_SW_TOOL_TYPE_1_ON) != CH_STATE_SW_OFF ){	

		tmp = GPIO_ReadInputDataBit(TOOL_TYPE_1_DET_PORT, TOOL_TYPE_1_DET_PIN);
		tool_det->tool_type_1_update_cnt++;

		if(tmp == RESET){
			tool_det->tool_type_1_sig_bits_cnt[0]++;
		} else{
			tool_det->tool_type_1_sig_bits_cnt[1]++;
		}

		if (tool_det->tool_type_1_update_cnt > SIGNAL_JITTER_FILTER_CNT_SYSTICK)
		{
			tool_det->tool_type_1_update_flag = 1;

			if (tool_det->tool_type_1_sig_bits_cnt[0] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->tool_type_1 = tool_115_210;
			}
			if (tool_det->tool_type_1_sig_bits_cnt[1] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->tool_type_1 = tool_245;
			}

			tool_det->tool_type_1_update_cnt = 0;
			tool_det->tool_type_1_sig_bits_cnt[0] = 0;
			tool_det->tool_type_1_sig_bits_cnt[1] = 0;
		}
	}else{
		tool_det->tool_type_1_update_cnt = 0;
		tool_det->tool_type_1_sig_bits_cnt[0] = 0;
		tool_det->tool_type_1_sig_bits_cnt[1] = 0;
		tool_det->tool_type_1_update_flag = 0;
	}
}

// 加热中检测烙铁头类型
// AC
// 不加热		高
// 加热 210		高
// 加热 245		方波		方波周期 50Hz
// DC
// 不加热		高
// 加热 210		高
// 加热 245		低
void tool_type_2_update(volatile tool_detect_data_t *tool_det, uint8_t update_sw)
{	
	uint8_t tmp;
  if( (update_sw & CH_STATE_SW_TOOL_TYPE_2_ON) != CH_STATE_SW_OFF )
	{	
		tmp = GPIO_ReadInputDataBit(TOOL_TYPE_2_DET_PORT, TOOL_TYPE_2_DET_PIN);

		if(tmp == RESET){
			tool_det->tool_type_2_sig_bits_cnt[0]++;
		} else{
			tool_det->tool_type_2_sig_bits_cnt[1]++;
		}

		tool_det->tool_type_2_update_cnt++;

		if (tool_det->tool_type_2_update_cnt >= SIGNAL_JITTER_FILTER_CNT_SYSTICK)
		{
			tool_det->tool_type_2_update_flag = 1;

			if (tool_det->tool_type_2_sig_bits_cnt[1] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->tool_type_2 = tool_115_210;
			}
			else
#ifdef THT_M120
				if ((tool_det->tool_type_2_sig_bits_cnt[0] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_FAIL_RATE &&
					 tool_det->tool_type_2_sig_bits_cnt[0] < SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE) ||
					(tool_det->tool_type_2_sig_bits_cnt[1] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_FAIL_RATE &&
					 tool_det->tool_type_2_sig_bits_cnt[1] < SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE))
			{
				tool_det->tool_type_2 = tool_245;
			}
#endif

#ifdef THT_L075
			//			if(  tool_det->tool_type_2_sig_bits_cnt[0] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE) {
			//				tool_det->tool_type_2 = tool_245;
			//			}
			if (tool_det->tool_type_2_sig_bits_cnt[0] > SIGNAL_JITTER_FILTER_HALF_RATE)
			{
				tool_det->tool_type_2 = tool_245;
			}
#endif

#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1
			if (ch_logic_debug.stage_flag == 1)
			{
				sprintf((char *)debug_string_out_0, "tool_type_2_update function: tool_type_1: %d,tool_type_2: %d, tool_type_3: %d\n",
						tool_det->tool_type_1, tool_det->tool_type_2, tool_det->tool_type_3);
				log_info((char *)debug_string_out_0);

				sprintf((char *)debug_string_out_0, "tool_type_2_update function:  tool type 2 update_cnts\t%d, %d, %d\n",
						tool_det->tool_type_2_update_cnt,
						tool_det->tool_type_2_sig_bits_cnt[0],
						tool_det->tool_type_2_sig_bits_cnt[1]);
				log_info((char *)debug_string_out_0);
			}
#endif

			tool_det->tool_type_2_update_cnt = 0;
			tool_det->tool_type_2_sig_bits_cnt[0] = 0;
			tool_det->tool_type_2_sig_bits_cnt[1] = 0;
		}
	}else{
		tool_det->tool_type_2_update_cnt = 0;
		tool_det->tool_type_2_sig_bits_cnt[0] = 0;
		tool_det->tool_type_2_sig_bits_cnt[1] = 0;
		tool_det->tool_type_2_update_flag = 0;
	}
}

/**
 * @brief 睡眠状态更新函数
 * @param tool_det 工具检测数据结构体指针，存储睡眠状态相关信息
 * @param update_sw 更新开关标志，控制是否执行状态更新
 * 
 * 功能说明：
 * - 检测焊台工具的睡眠状态，包括睡眠基座状态、工具状态和工作状态
 * - 通过GPIO读取睡眠基座和工具的状态信号
 * - 使用信号防抖算法消除信号抖动，提高状态检测的可靠性
 * - 根据信号组合判断具体的睡眠状态
 * 
 * 状态说明：
 * - sleep_err: 睡眠错误状态
 * - sleep_base: 睡眠基座状态
 * - sleep_tool: 睡眠工具状态
 * - on_work: 工作状态
 */

void sleep_state_update(volatile tool_detect_data_t *tool_det, uint8_t update_sw)
{
	//	#if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1
	//	sprintf((char*)debug_output_buffer, "sleep signal update\tsw:%d\tupdate_cnt:%d bits_cnt:%d %d %d %d\n",
	//								update_sw,
	//								tool_det->sleep_update_cnt,
	//								tool_det->sleep_sig_bits_cnt[0],
	//								tool_det->sleep_sig_bits_cnt[1],
	//								tool_det->sleep_sig_bits_cnt[2],
	//								tool_det->sleep_sig_bits_cnt[3]	);
	//	log_info((char*)debug_output_buffer);
	//	#endif
	tool_detect_disable_in_heat_set(tool_det);
	if( (update_sw & CH_STATE_SW_SLEEP_ON) != CH_STATE_SW_OFF){
		
		if(tool_det->tool_in_disable_in_heat == tool_heat_on){
			return;
		}
		if( RESET == GPIO_ReadInputDataBit(SLEEP_HANDLE_PORT, SLEEP_HANDLE_PIN) )
		{
			tool_det->sleep_sig_bits_cnt[0]++;
		}else{
			tool_det->sleep_sig_bits_cnt[1]++;
		}
		if( RESET == GPIO_ReadInputDataBit(SLEEP_TOOL_PORT, SLEEP_TOOL_PIN) )
		{
			tool_det->sleep_sig_bits_cnt[2]++;
		}else{
			tool_det->sleep_sig_bits_cnt[3]++;
		}

		tool_det->sleep_update_cnt++;

		if (tool_det->sleep_update_cnt > SIGNAL_JITTER_FILTER_CNT_SYSTICK)
		{
			// #if DEBUG_CMD_CTROL == 1 && DEBUG_STAGE_TEST == 1
			// sprintf((char*)debug_output_buffer, "sleep signal update\t\tupdate_cnt:%d bits_cnt:%d %d %d %d\n",
			// 								tool_det->sleep_update_cnt,
			// 								tool_det->sleep_sig_bits_cnt[0],
			// 								tool_det->sleep_sig_bits_cnt[1],
			// 								tool_det->sleep_sig_bits_cnt[2],
			// 								tool_det->sleep_sig_bits_cnt[3]	);
			// log_info((char*)debug_output_buffer);
			// #endif
			tool_det->sleep_state_update_flag = 1;
			// sleep base
			if (tool_det->sleep_sig_bits_cnt[0] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->sleep_sig_bits = 0x08;
			}
			else if (tool_det->sleep_sig_bits_cnt[1] < SIGNAL_JITTER_FILTER_HALF_RATE)
			{
				tool_det->sleep_sig_bits = 0x00;
			}
			else if (tool_det->sleep_sig_bits_cnt[1] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->sleep_sig_bits = 0x01;
			}

			if (tool_det->sleep_sig_bits_cnt[2] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->sleep_sig_bits |= 0x80;
			}
			else if (tool_det->sleep_sig_bits_cnt[3] < SIGNAL_JITTER_FILTER_HALF_RATE)
			{
				tool_det->sleep_sig_bits |= 0x00;
			}
			else if (tool_det->sleep_sig_bits_cnt[3] > SIGNAL_JITTER_FILTER_CNT_SYSTICK_SUCSS_RATE)
			{
				tool_det->sleep_sig_bits |= 0x10;
			}

			if (tool_det->sleep_sig_bits == 0x88)
			{
				tool_det->sleep_state = sleep_err;
			}
			else if (tool_det->sleep_sig_bits == 0x01 || tool_det->sleep_sig_bits == 0x81)
			{
				tool_det->sleep_state = sleep_base;
			}
			else if (tool_det->sleep_sig_bits == 0x10 || tool_det->sleep_sig_bits == 0x18)
			{
				tool_det->sleep_state = sleep_tool;
			}
			else if (tool_det->sleep_sig_bits == 0x11)
			{
				tool_det->sleep_state = on_work;
			}
			// if(tool_det->sleep_sig_bits == 0x11){
			// 	tool_det->sleep_state = on_work;
			// }else {
			// 	tool_det->sleep_state = sleep_tool;
			// }

			tool_det->sleep_update_cnt = 0;
			tool_det->sleep_sig_bits_cnt[0] = 0;
			tool_det->sleep_sig_bits_cnt[1] = 0;
			tool_det->sleep_sig_bits_cnt[2] = 0;
			tool_det->sleep_sig_bits_cnt[3] = 0;
		}
	}else{
		tool_det->sleep_update_cnt = 0;
		tool_det->sleep_sig_bits_cnt[0] = 0;
		tool_det->sleep_sig_bits_cnt[1] = 0;
		tool_det->sleep_sig_bits_cnt[2] = 0;
		tool_det->sleep_sig_bits_cnt[3] = 0;
		tool_det->sleep_state_update_flag = 0;
	}
}

void tool_detect_signal_reset(volatile tool_detect_data_t *tool_det)
{
	tool_det->tool_in_update_flag = 0;
	tool_det->tool_type_1_update_flag = 0;
	tool_det->tool_type_2_update_flag = 0;
	tool_det->sleep_state_update_flag = 0;

	tool_det->tool_in_state = tool_out;
	tool_det->tool_type_1 = notool;
	tool_det->tool_type_2 = notool;
	tool_det->sleep_state = sleep_err;

	tool_det->tool_in_chk_sig_bits_cnt[0] = 0;
	tool_det->tool_in_chk_sig_bits_cnt[1] = 0;
	tool_det->tool_in_chk_update_cnt = 0;

	tool_det->tool_type_1_update_cnt = 0;
	tool_det->tool_type_1_sig_bits_cnt[0] = 0;
	tool_det->tool_type_1_sig_bits_cnt[1] = 0;

	tool_det->tool_type_2_update_cnt = 0;
	tool_det->tool_type_2_sig_bits_cnt[0] = 0;
	tool_det->tool_type_2_sig_bits_cnt[1] = 0;

	tool_det->sleep_update_cnt = 0;
	tool_det->sleep_sig_bits_cnt[0] = 0;
	tool_det->sleep_sig_bits_cnt[1] = 0;
	tool_det->sleep_sig_bits_cnt[2] = 0;
	tool_det->sleep_sig_bits_cnt[3] = 0;
}

//void SOLDER_CH_TOOL_IN_IRQHandler(void)
//{
//	if (RESET != EXTI_GetITStatus(TOOL_IN_EXTI_LINE))
//	{
//		if(solder_channel.state_update_sw & CH_STATE_SW_TOOL_IN_ON){
//			solder_channel.tool_in_chk_sig_bits_cnt[2]++;
//		}
//		EXTI_ClrITPendBit(TOOL_IN_EXTI_LINE);
//	}
//}

