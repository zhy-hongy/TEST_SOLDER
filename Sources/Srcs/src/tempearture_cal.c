#include "temperature_cal.h"
#include "solder.h"
static volatile uint8_t	index;
static volatile float 	temp;

static volatile float 	real_temp;

temp_cal_data_t temp_210_data;
temp_cal_data_t temp_245_data;

temp_cal_data_t temp_calibration_115_data;
temp_cal_data_t temp_calibration_210_data;
temp_cal_data_t temp_calibration_245_data;

float mv_tab_210[6]		= {  0, 1.61, 2.08, 2.56, 2.98, 3.45};
float temp_tab_210[6]	= { 30,  250,  300,  350,  400,  450};

//float mv_tab_210[6]		= {   0, 1.36, 1.7,   2.05, 2.45, 2.88};
//float temp_tab_210[6]	= {  30,  250,  300,  350,  400,  450};

float mv_tab_245[6]		= {   0,  4.7,  5.8,  6.9,  7.8,  8.8};
float temp_tab_245[6]	= {  20,  250,  300,  350,  400,  450};


float goal_temp_tab_115[5]	= {   0,  100,  200,  300,  400};
float real_temp_tab_115[5]	= {  25,  100,  200,  300,  400};

float goal_temp_tab_210[5]	= {   0,  100,  200,  300,  400};
float real_temp_tab_210[5]	= {  25,  100,  200,  300,  400};

float goal_temp_tab_245[5]	= {   0,  100,  200,  300,  400};
float real_temp_tab_245[5]	= {  25,  50,  150,  250,  350};

//求6组一元一次方程的k和b，首尾相连
void init_temp_cal_tab(temp_cal_data_t* temp_cal_data, float* mv_tab, float* temp_tab, uint8_t length)
{
	temp_cal_data->length 	=	length;
	temp_cal_data->mv 		=	mv_tab;
	temp_cal_data->temp 	=	temp_tab;

	for(index = 0; index < temp_cal_data->length-1; index++){
			temp_cal_data->k[index] = 
			( temp_cal_data->temp[index+1] - temp_cal_data->temp[index] ) / ( temp_cal_data->mv[index+1] - temp_cal_data->mv[index] );
		
			temp_cal_data->b[index] = temp_cal_data->temp[index] - temp_cal_data->k[index] * temp_cal_data->mv[index];				
	}
	
	#if DEBUG_CMD_CTROL == 1 && DEBUG_TEMP_INIT_INFO == 1
	sprintf(debug_string_out_0, "temp init length: %d\nmv\t:", temp_cal_data->length);
	for(index = 0; index < temp_cal_data->length; index++){
			sprintf(debug_string_out_1, "%8.2f", temp_cal_data->mv[index]);
			strcat(debug_string_out_0, debug_string_out_1);
	}
	strcat(debug_string_out_0, "\ntemp_tab:");
	for(index = 0; index < temp_cal_data->length; index++){
			sprintf(debug_string_out_1, "%8.2f", temp_cal_data->temp[index]);
			strcat(debug_string_out_0, debug_string_out_1);
	}
	strcat(debug_string_out_0, "\nk\t:");
	for(index = 0; index < temp_cal_data->length; index++){
			sprintf(debug_string_out_1, "%8.2f", temp_cal_data->k[index]);
			strcat(debug_string_out_0, debug_string_out_1);
	}
	strcat(debug_string_out_0, "\nb\t:");
	for(index = 0; index < temp_cal_data->length; index++){
			sprintf(debug_string_out_1, "%8.2f", temp_cal_data->b[index]);
			strcat(debug_string_out_0, debug_string_out_1);
	}
	strcat(debug_string_out_0, "\n");
	log_info(debug_string_out_0);
	#endif
}


void init_temp_Calibration_tab(temp_cal_data_t* temp_cal_data, float* mv_tab, float* temp_tab, uint8_t length)
{
	temp_cal_data->length 	=	length;
	temp_cal_data->mv 		=	mv_tab;
	temp_cal_data->temp 	=	temp_tab;

	for(index = 0; index < temp_cal_data->length-1; index++){
			temp_cal_data->k[index] = 
			( temp_cal_data->temp[index+1] - temp_cal_data->temp[index] ) / ( temp_cal_data->mv[index+1] - temp_cal_data->mv[index] );
		
			temp_cal_data->b[index] = temp_cal_data->temp[index] - temp_cal_data->k[index] * temp_cal_data->mv[index];				
	}

}
//初始化所有温度校准表
void init_all_temp_tab(void)
{
	init_temp_cal_tab(&temp_210_data, mv_tab_210, temp_tab_210, 6);
	init_temp_cal_tab(&temp_245_data, mv_tab_245, temp_tab_245, 6);
	
	solder_param_get_from_array((void*)&solder_channel.cal_115_flag, param_temp_cal_115_flag);
	solder_param_get_from_array((void*)&solder_channel.cal_210_flag, param_temp_cal_210_flag);
	solder_param_get_from_array((void*)&solder_channel.cal_245_flag, param_temp_cal_245_flag);
	
	if(solder_channel.cal_115_flag==1)
	{
		solder_param_get_from_array((void*)&real_temp_tab_115[1], param_temp_cal_115_100);
		solder_param_get_from_array((void*)&real_temp_tab_115[2], param_temp_cal_115_200);
		solder_param_get_from_array((void*)&real_temp_tab_115[3], param_temp_cal_115_300);
		solder_param_get_from_array((void*)&real_temp_tab_115[4], param_temp_cal_115_400);
	}
	if(solder_channel.cal_210_flag==1)
	{
		solder_param_get_from_array((void*)&real_temp_tab_210[1], param_temp_cal_210_100);
		solder_param_get_from_array((void*)&real_temp_tab_210[2], param_temp_cal_210_200);
		solder_param_get_from_array((void*)&real_temp_tab_210[3], param_temp_cal_210_300);
		solder_param_get_from_array((void*)&real_temp_tab_210[4], param_temp_cal_210_400);
	}
	if(solder_channel.cal_245_flag==1)
	{
		solder_param_get_from_array((void*)&real_temp_tab_245[1], param_temp_cal_245_100);
		solder_param_get_from_array((void*)&real_temp_tab_245[2], param_temp_cal_245_200);
		solder_param_get_from_array((void*)&real_temp_tab_245[3], param_temp_cal_245_300);
		solder_param_get_from_array((void*)&real_temp_tab_245[4], param_temp_cal_245_400);
//		log_info("cal_245_flag");
	}
//	sprintf((char*)debug_string_out_0, "real_temp_tab_245:%f\n",real_temp_tab_245[0]);
//	log_info((char*)debug_string_out_0);
//	sprintf((char*)debug_string_out_0, "real_temp_tab_245:%f\n", real_temp_tab_245[1]);
//	log_info((char*)debug_string_out_0);
//	sprintf((char*)debug_string_out_0, "real_temp_tab_245:%f\n", real_temp_tab_245[2]);
//	log_info((char*)debug_string_out_0);
//	sprintf((char*)debug_string_out_0, "real_temp_tab_245:%f\n", real_temp_tab_245[3]);
//	log_info((char*)debug_string_out_0);
//	sprintf((char*)debug_string_out_0, "real_temp_tab_245:%f\n",real_temp_tab_245[4]);
//	log_info((char*)debug_string_out_0);
//	
	init_temp_Calibration_tab(&temp_calibration_115_data,goal_temp_tab_115,real_temp_tab_115,5);
	init_temp_Calibration_tab(&temp_calibration_210_data,goal_temp_tab_210,real_temp_tab_210,5);
	init_temp_Calibration_tab(&temp_calibration_245_data,goal_temp_tab_245,real_temp_tab_245,5);
	
}

float Calibration_temp(temp_cal_data_t* temp_cal_data, float goal_temp)
{
	float tmp_temp = goal_temp * 0.75f;
	
	if( tmp_temp < (temp_cal_data->mv[0]+0.001f) ){
		real_temp = temp_cal_data->temp[0];
	}else
	if( tmp_temp > (temp_cal_data->mv[temp_cal_data->length-1] - 0.001f)){
		real_temp = temp_cal_data->k[temp_cal_data->length-2] * tmp_temp + temp_cal_data->b[temp_cal_data->length-2];
	}else{
		for(index = 0; index < (temp_cal_data->length-1); index++)
		{
				if(tmp_temp > temp_cal_data->mv[index] && tmp_temp < temp_cal_data->mv[index+1])
				{
						real_temp = temp_cal_data->k[index] * tmp_temp + temp_cal_data->b[index];
						break;
				}
		}
	}
	
	real_temp = real_temp / 0.75f;
	
//	sprintf((char*)debug_string_out_0, "temp:%f, %f\n",goal_temp, real_temp);

//	log_info((char*)debug_string_out_0);
	
	return real_temp;
}


/**
 * @brief 根据输入的毫伏值使用线性插值法计算对应的温度值
 * 
 * @param temp_cal_data 温度校准数据结构体指针，包含校准点数据
 * @param mv 输入的毫伏值
 * @return float 计算得到的温度值
 * 
 * 函数功能说明：
 * 1. 该函数使用分段线性插值方法，根据输入的毫伏值计算对应的温度
 * 2. 温度校准数据结构体中包含多个校准点，每个点有对应的毫伏值和温度值
 * 3. 函数预先计算了每段线性方程的斜率(k)和截距(b)，存储在校准数据结构体中
 * 4. 根据输入毫伏值所在的区间，选择对应的线性方程进行计算
 */
float cal_temp(temp_cal_data_t* temp_cal_data, float mv)
{
	// 处理输入毫伏值低于最低校准点的情况
	// 如果输入值小于第一个校准点的毫伏值(加上0.001mV的容差)
	if( mv < (temp_cal_data->mv[0]+0.001f) ){
		// 直接返回第一个校准点的温度值
		temp = temp_cal_data->temp[0];
	}
	// 处理输入毫伏值高于最高校准点的情况
	// 如果输入值大于最后一个校准点的毫伏值(减去0.001mV的容差)
	else if( mv > (temp_cal_data->mv[temp_cal_data->length-1] - 0.001f)){
		// 使用最后一段线性方程外推计算温度值
		// 使用倒数第二段的斜率和截距，因为最后一段没有下一个点可以计算
		temp = temp_cal_data->k[temp_cal_data->length-2] * mv + temp_cal_data->b[temp_cal_data->length-2];
	}
	// 处理输入毫伏值在正常范围内的情况
	else{
		// 遍历所有校准点区间，找到输入值所在的区间
		for(index = 0; index < (temp_cal_data->length-1); index++)
		{
			// 检查输入毫伏值是否在当前区间内
			// 即大于当前点的毫伏值且小于下一个点的毫伏值
			if(mv > temp_cal_data->mv[index] && mv < temp_cal_data->mv[index+1])
			{
				// 使用当前区间的线性方程计算温度值
				// 温度 = 斜率 * 毫伏值 + 截距
				temp = temp_cal_data->k[index] * mv + temp_cal_data->b[index];
				// 找到对应区间后跳出循环
				break;
			}
		}
	}
	// 返回计算得到的温度值
	return temp;
}

/**
 * @brief 根据ADC值和工具类型计算实际温度
 * @param adc_val ADC采样值
 * @param tool_type 工具类型（115/210/245）
 * @return float 计算得到的实际温度值，如果出错返回TEMP_EXCEED
 * 
 * 函数功能说明：
 * 1. 根据工具类型选择对应的运放校正参数和温度数据表
 * 2. 将ADC值转换为电压，再通过运放校正得到毫伏值
 * 3. 使用线性插值方法计算基础温度
 * 4. 根据标定标志位决定是否进行温度校准
 * 5. 返回最终的温度计算结果
 */
float get_temp(float adc_val, uint8_t tool_type)
{
	float mv_tmp;           // 临时存储转换后的毫伏值
	float temporary_temp;   // 临时存储计算的基础温度值
	
	// 根据工具类型选择对应的温度计算参数
	if(tool_type == 115 || tool_type == 210)
	{
		// 对于115和210型工具，使用相同的温度计算参数
		// 1. adc_ret_vol将ADC值转换为电压值
		// 2. opa_vol_cal进行运放电压校正
		// 3. *1000转换为毫伏值
		// 4. cal_temp使用线性插值计算基础温度
		mv_tmp 	= opa_vol_cal( (void*)&temp_210_opa_correct, adc_ret_vol(adc_val) ) * 1000;
		temporary_temp=cal_temp(&temp_210_data, mv_tmp);
	}
	else if(tool_type == 245)
	{
		// 对于245型工具，使用专用的温度计算参数
		mv_tmp 	= opa_vol_cal( (void*)&temp_245_opa_correct, adc_ret_vol(adc_val) ) * 1000;
		temporary_temp=cal_temp(&temp_245_data, mv_tmp);
	}     
//		#ifdef DEBUG_CMD_CTROL
//	DELAY_EXECUTE(1000,{
//		sprintf(debug_string_out_0, "temporary_temp: %2f\r\n", temporary_temp);
//		log_info(debug_string_out_0);});
//		 #endif
	// 判断是否需要进行温度校准的条件：
	// 1. 当前处于设置模式(ch_set) - 显示原始温度值
	// 2. 对应工具类型的标定标志位为0 - 未进行标定，使用基础温度
	if(	solder_channel.ch_stage==ch_set  || 
			(tool_type == 115 && solder_channel.cal_115_flag == 0) ||
		  (tool_type == 210 && solder_channel.cal_210_flag == 0 )||
			(tool_type == 245 && solder_channel.cal_245_flag == 0 )	)
	{
		// 不进行校准，直接返回基础温度值
//		log_info("ch_set\n");	
		return temporary_temp;
	}
	else{
		// 根据工具类型和标定标志位进行温度校准
		if(tool_type == 115 && solder_channel.cal_115_flag==1){
			// 115型工具已标定，使用校准数据表进行温度修正
			return Calibration_temp((void*)&temp_calibration_115_data,temporary_temp);
		}
		if(tool_type == 210 && solder_channel.cal_210_flag==1){
			// 210型工具已标定，使用校准数据表进行温度修正
			return Calibration_temp((void*)&temp_calibration_210_data,temporary_temp);		
		}
		if(tool_type == 245 && solder_channel.cal_245_flag==1){
			// 245型工具已标定，使用校准数据表进行温度修正
			//log_info("cal_245_flag\n");	
			return Calibration_temp((void*)&temp_calibration_245_data,temporary_temp);
		}
	}                                
	// 如果所有条件都不满足，返回温度超限错误
	return TEMP_EXCEED;
}
