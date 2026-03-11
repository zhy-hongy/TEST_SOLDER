#include "solder.h"

uint16_t temp_table[] = {
	58354, // -55
	55464, // -54
	52698, // -53
	50048, // -52
	47515, // -51
	45097, // -50
	42789, // -49
	40589, // -48
	38492, // -47
	36496, // -46
	34597, // -45
	32791, // -44
	31075, // -43
	29444, // -42
	27896, // -41
	26427, // -40
	25034, // -39
	23713, // -38
	22460, // -37
	21273, // -36
	20148, // -35
	19083, // -34
	18075, // -33
	17120, // -32
	16216, // -31
	15361, // -30
	14551, // -29
	13785, // -28
	13061, // -27
	12376, // -26
	11728, // -25
	11114, // -24
	10535, // -23
	9986,  // -22
	9468,  // -21
	8977,  // -20
	8513,  // -19
	8075,  // -18
	7660,  // -17
	7267,  // -16
	6896,  // -15
	6545,  // -14
	6212,  // -13
	5898,  // -12
	5601,  // -11
	5319,  // -10
	5053,  // -9
	4801,  // -8
	4562,  // -7
	4336,  // -6
	4122,  // -5
	3920,  // -4
	3728,  // -3
	3546,  // -2
	3374,  // -1
	3211,  // 0
	3057,  // 1
	2910,  // 2
	2771,  // 3
	2639,  // 4
	2515,  // 5
	2396,  // 6
	2284,  // 7
	2177,  // 8
	2076,  // 9
	1978,  // 10
	1889,  // 11
	1802,  // 12
	1720,  // 13
	1642,  // 14
	1568,  // 15
	1497,  // 16
	1430,  // 17
	1366,  // 18
	1306,  // 19
	1248,  // 20
	1193,  // 21
	1141,  // 22
	1092,  // 23
	1044,  // 24
	1000,  // 25
	957,   // 26
	916,   // 27
	877,   // 28
	840,   // 29
	805,   // 30
	771,   // 31
	739,   // 32
	709,   // 33
	679,   // 34
	652,   // 35
	625,   // 36
	600,   // 37
	576,   // 38
	552,   // 39
	530,   // 40
	509,   // 41
	489,   // 42
	470,   // 43
	452,   // 44
	434,   // 45
	417,   // 46
	401,   // 47
	386,   // 48
	371,   // 49
	358,   // 50
	344,   // 51
	331,   // 52
	318,   // 53
	306,   // 54
	295,   // 55
	284,   // 56
	274,   // 57
	264,   // 58
	254,   // 59
	245,   // 60
	236,   // 61
	228,   // 62
	220,   // 63
	212,   // 64
	205,   // 65
	198,   // 66
	191,   // 67
	184,   // 68
	178,   // 69
	172,   // 70
	166,   // 71
	160,   // 72
	155,   // 73
	150,   // 74
	145,   // 75
	140,   // 76
	135,   // 77
	131,   // 78
	126,   // 79
	122,   // 80
	118,   // 81
	115,   // 82
	111,   // 83
	107,   // 84
	104,   // 85
	101,   // 86
	97,	   // 87
	94,	   // 88
	91,	   // 89
	89,	   // 90
	86,	   // 91
	83,	   // 92
	81,	   // 93
	78,	   // 94
	76,	   // 95
	74,	   // 96
	71,	   // 97
	69,	   // 98
	67,	   // 99
	65,	   // 100
	63,	   // 101
	61,	   // 102
	60,	   // 103
	58,	   // 104
	56,	   // 105
	55,	   // 106
	53,	   // 107
	52,	   // 108
	50,	   // 109
	49,	   // 110
	47,	   // 111
	46,	   // 112
	45,	   // 113
	43,	   // 114
	42,	   // 115
	41,	   // 116
	40,	   // 117
	39,	   // 118
	38,	   // 119
	37,	   // 120
	36,	   // 121
	35,	   // 122
	34,	   // 123
	33,	   // 124
	32,	   // 125
};

float get_solder_ileak(float adc_val)
{
	float v_in;
	v_in = opa_vol_cal((void *)&ileak_opa_correct, adc_ret_vol(adc_val));
	return v_in / 0.050; // 50毫欧
}

float get_solder_voltage(float adc_val)
{
#ifdef THT_L075
	return opa_vol_cal((void *)&voltage_opa_correct, adc_ret_vol(adc_val)) * 10; // 母线电压分压9:1::
#else
	return opa_vol_cal((void *)&voltage_opa_correct, adc_ret_vol(adc_val)) * 51 / 1.5;// 母线电压分压34:1:
#endif
}

float get_solder_current(float adc_val)
{
	float v_in;
	v_in = opa_vol_cal((void *)&current_opa_correct, adc_ret_vol(adc_val));
	return v_in * 1000 / 5; // 5毫欧
}
static int search_temp(float rst_Rx10)
{
	int i, min_index = 0;
	// 计算数组长度
	int len = sizeof(temp_table) / sizeof(u16);
	// 记录最小差值
	float min_diff = fabs(rst_Rx10 - temp_table[0]);

	for (i = 1; i < len; i++)
	{
		// 计算数组里每一个阻值和rst_Rx10的差值
		float diff = fabs(rst_Rx10 - temp_table[i]);

		// 得到差值最小元素对应的索引i
		if (diff < min_diff)
		{
			// 如果有更小的差值，赋值
			min_diff = diff;
			min_index = i;
		}
	}

#ifdef DEBUG_CMD_CTROL
	// DELAY_EXECUTE(1000,{
	// 	sprintf(debug_string_out_0, "len: %d R: %.2f min_diff: %.2f min_index: %d \n", len, rst_Rx10, min_diff, min_index);
	// 	log_info(debug_string_out_0);});
#endif
	return min_index;
}

float get_solder_pcb_temp(uint16_t adc_value)
{
	float rst_V;
	float rst_R;
	int rst_T;
	// 计算电压
	rst_V = adc_ret_vol(adc_value);
	// 计算电阻值
	rst_R = rst_V * 10 / (3.3 - rst_V);
	// 将阻值兑换成温度
	rst_T = search_temp(rst_R * 100) - 55;
#ifdef DEBUG_CMD_CTROL
	// DELAY_EXECUTE(1000,{
	// 	sprintf(debug_string_out_0, "rst_V: %.2f, rst_R: %.2f, rst_T: %2d, adc_value: %2d\r\n", rst_V, rst_R, rst_T, adc_value);
	// 	log_info(debug_string_out_0);});
#endif
	return rst_T;
}


uint8_t get_voltage_current_array(void)
{
	float sum_res = 0, sum_cur = 0, sum_pcb_temp = 0;
	uint8_t vol_cur_i, sum_cnts = 0;
	uint8_t first_error_discard_flag = 1;
	// int sum_temp = 0;
	solder.voltage_current.length = adc_inject_irq_state.cnts;

	for (vol_cur_i = 0; vol_cur_i < solder.voltage_current.length; vol_cur_i++)
	{
		solder.voltage_current.ileak[vol_cur_i] =
			get_solder_ileak(adc_inject_irq_state.adc_val[0][vol_cur_i]);
		solder.voltage_current.voltage[vol_cur_i] =
			get_solder_voltage(adc_inject_irq_state.adc_val[1][vol_cur_i]);
		solder.voltage_current.current[vol_cur_i] =
			get_solder_current(adc_inject_irq_state.adc_val[2][vol_cur_i]);
		solder.voltage_current.pcb_temp[vol_cur_i] =
			get_solder_pcb_temp(adc_inject_irq_state.adc_val[3][vol_cur_i]);
		// sum_temp += solder.voltage_current.pcb_temp[vol_cur_i];
		solder.voltage_current.resist[vol_cur_i] = solder.voltage_current.voltage[vol_cur_i] /
												   solder.voltage_current.current[vol_cur_i];
		sum_pcb_temp += solder.voltage_current.pcb_temp[vol_cur_i];

		if (solder.voltage_current.heat_state[vol_cur_i] == heat_on)
		{
			if (first_error_discard_flag == 1)
			{
				first_error_discard_flag = 0;
			}
			else
			{
				sum_res += solder.voltage_current.resist[vol_cur_i];
				sum_cnts++;
			}
		}
		#ifdef DEBUG_CMD_CTROL
        if (ch_bsp_debug.adc_after_calculation_flag == 1)
		{
			DELAY_EXECUTE(1000, {
				sprintf(debug_string_int_0, "ileak: %4.3f,voltage: %4.3f,current: %4.3f,pcb_temp: %4.3f\n",
						solder.voltage_current.ileak[vol_cur_i],
						solder.voltage_current.voltage[vol_cur_i],
						solder.voltage_current.current[vol_cur_i],
						solder.voltage_current.pcb_temp[vol_cur_i]);
				log_info(debug_string_int_0);
			});
		}
		#endif
	}

#ifdef DEBUG_CMD_CTROL

	// if (solder_channel.ch_stage == ch_tool_quality_test) {
	//     sprintf(debug_string_out_0,
	//             "pcb_temp: %2f %2f,  current:%2f, %2f, voltage:%2f, %2f, solder_channel.type_test_avg_res: %2f\r\n",
	//             solder.voltage_current.pcb_temp[0], solder.voltage_current.pcb_temp[1],
	//             solder.voltage_current.current[5], solder.voltage_current.current[6],
	//             solder.voltage_current.voltage[5], solder.voltage_current.voltage[6], solder_channel.type_test_avg_res);
	//     log_info(debug_string_out_0);
	// }

	//	DELAY_EXECUTE(1000, {
	//		sprintf(debug_string_out_0, "voltage: %2f ,%2f ,%2f ,%2f ,%2f ,%2f ,%2f ,%2f ,%2f ,%2f , current: %2f ,%2f ,%2f ,%2f ,%2f ,%2f ,%2f ,%2f ,%2f ,%2f \r\n",
	//			solder.voltage_current.voltage[0],
	//				solder.voltage_current.voltage[1],
	//				solder.voltage_current.voltage[2],
	//				solder.voltage_current.voltage[3],
	//				solder.voltage_current.voltage[4],
	//				solder.voltage_current.voltage[5],
	//				solder.voltage_current.voltage[6],
	//				solder.voltage_current.voltage[7],
	//				solder.voltage_current.voltage[8],
	//				solder.voltage_current.voltage[9],
	//				solder.voltage_current.current[0],
	//				solder.voltage_current.current[1],
	//				solder.voltage_current.current[2],
	//				solder.voltage_current.current[3],
	//				solder.voltage_current.current[4],
	//				solder.voltage_current.current[5],
	//				solder.voltage_current.current[6],
	//				solder.voltage_current.current[7],
	//				solder.voltage_current.current[8],
	//				solder.voltage_current.current[9]);
	//		log_info(debug_string_out_0); });
#endif

	if (sum_cnts > 3)
	{
		solder_channel.type_test_avg_res = sum_res / sum_cnts;
	}
	else if (sum_cnts == 0)
	{
	}
	else
	{
		solder_channel.type_test_avg_res = -100.0f;
	}

	solder_channel.pcb_temp_avg = sum_pcb_temp / VOL_CUR_DATA_LENGTH;
	return 0;
}


void ADC_IRQHandler(void)
{
	if (ADC_GetIntStatus(ADC, ADC_INT_JENDC) != RESET)
	{
		ADC_ClearIntPendingBit(ADC, ADC_INT_JENDC);
		// 漏电流   电压   电流 PCB温度
		adc_inject_irq_state.adc_val[0][adc_inject_irq_state.cnts] =
			ADC_GetInjectedConversionDat(ADC, ADC_INJ_CH_1);
		adc_inject_irq_state.adc_val[1][adc_inject_irq_state.cnts] =
			ADC_GetInjectedConversionDat(ADC, ADC_INJ_CH_2);
		adc_inject_irq_state.adc_val[2][adc_inject_irq_state.cnts] =
			ADC_GetInjectedConversionDat(ADC, ADC_INJ_CH_3);
		adc_inject_irq_state.adc_val[3][adc_inject_irq_state.cnts] =
			ADC_GetInjectedConversionDat(ADC, ADC_INJ_CH_4);
		solder.voltage_current.heat_state[adc_inject_irq_state.cnts] = solder_channel.heat_state;
#if DEBUG_CMD_CTROL == 1 && DEBUG_ADC_INJ_INT == 1

		if (ch_bsp_debug.adc_injection_flag == 1)
		{
			DELAY_EXECUTE(1000, {
				sprintf(debug_string_int_0, "adc_val-> ileak: %4d,voltage: %4d,current: %4d,pcb_temp: %4d\n",
						adc_inject_irq_state.adc_val[0][adc_inject_irq_state.cnts],
						adc_inject_irq_state.adc_val[1][adc_inject_irq_state.cnts],
						adc_inject_irq_state.adc_val[2][adc_inject_irq_state.cnts],
						adc_inject_irq_state.adc_val[3][adc_inject_irq_state.cnts]);
				log_info(debug_string_int_0);
			});
		}

#endif
		adc_inject_irq_state.cnts++;

		if (adc_inject_irq_state.cnts == VOL_CUR_DATA_LENGTH)
		{
			adc_inject_irq_state.cnts = 0;
		}
	}
}