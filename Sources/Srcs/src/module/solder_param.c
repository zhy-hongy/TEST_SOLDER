#include "solder_param.h"

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