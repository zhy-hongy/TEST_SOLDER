#include "controller.h"

// 卡尔曼滤波器参数配置（存储在Flash中）
const kalman_2d_init_params_t kf_params_115 = {
	.A = {{0.98, 0.02}, // 状态转移矩阵
		  {0.56, 0.325}},
	.H = {1.0, 0.0},  // 观测矩阵
	.Q = {{0.3, 0.0}, // 过程噪声协方差
		  {0.0, 1.0}},
	.R = 0.5, // 测量噪声协方差
	.P_init = {{1.0, 0.0},
			   {0.0, 1.0}}};
const kalman_2d_init_params_t kf_params_210 = {
	.A = {{0.98, 0.02}, // 状态转移矩阵
		  {0.59, 0.30}},
	.H = {1.0, 0.0},  // 观测矩阵
	.Q = {{0.3, 0.0}, // 过程噪声协方差
		  {0.0, 1.0}},
	.R = 0.5, // 测量噪声协方差
	.P_init = {{1.0, 0.0},
			   {0.0, 1.0}}};
const kalman_2d_init_params_t kf_params_245 = {
	.A = {{0.95, 0.05}, // 状态转移矩阵
		  {0.50, 0.375}},
	.H = {1.0, 0.0},  // 观测矩阵
	.Q = {{0.3, 0.0}, // 过程噪声协方差
		  {0.0, 1.0}},
	.R = 0.5, // 测量噪声协方差
	.P_init = {{1.0, 0.0},
			   {0.0, 1.0}}};
// 标准温度点数组，用于插值计算
// 温度点: 50, 100, 150, 200, 250, 300, 350, 400, 450
float heat_normal_temp[THRESHOLD_BUF_SIZE] = {50, 100, 150, 200, 250, 300, 350, 400, 450};

float heat_gapcnts_threshold_115[THRESHOLD_BUF_SIZE] = {200, 150, 80, 50, 35, 25, 18, 15, 12};
float heat_slope_thresh_hold_115[THRESHOLD_BUF_SIZE] = {-10, -10, -10, -10, -10, -10, -10, -10, -10};

float heat_gapcnts_threshold_210[THRESHOLD_BUF_SIZE] = {200, 150, 80, 50, 35, 25, 18, 15, 12};
float heat_slope_thresh_hold_210[THRESHOLD_BUF_SIZE] = {-0.1, -0.14, -0.17, -0.19, -0.25, -0.33, -0.39, -0.5, -0.7};

float heat_gapcnts_threshold_245[THRESHOLD_BUF_SIZE] = {200, 150, 100, 60, 40, 30, 25, 20, 18};
float heat_slope_thresh_hold_245[THRESHOLD_BUF_SIZE] = {-0.05, -0.1, -0.1, -0.12, -0.15, -0.22, -0.27, -0.35, -0.50};
/***
 * @brief 控制器初始化
 * @param ctrl 控制器结构体指针
 * @param tool_type 工具类型
 */
void controller_init(volatile temp_controller_t *ctrl, tool_type_t tool_type)
{
	switch (tool_type)
	{
	case tool_115:
		kalman_init(&ctrl->kf, &kf_params_115, 0);
		ctrl->heat_gapcnts_threshold = heat_gapcnts_threshold_115;
		ctrl->heat_slope_thresh_hold = heat_slope_thresh_hold_115;
		break;
	case tool_210:
		kalman_init(&ctrl->kf, &kf_params_210, 0);
		ctrl->heat_gapcnts_threshold = heat_gapcnts_threshold_210;
		ctrl->heat_slope_thresh_hold = heat_slope_thresh_hold_210;
		break;
	case tool_245:
		kalman_init(&ctrl->kf, &kf_params_245, 0);
		ctrl->heat_gapcnts_threshold = heat_gapcnts_threshold_245;
		ctrl->heat_slope_thresh_hold = heat_slope_thresh_hold_245;

		break;
	default:
		kalman_init(&ctrl->kf, &kf_params_210, 0);
		ctrl->heat_gapcnts_threshold = heat_gapcnts_threshold_210;
		ctrl->heat_slope_thresh_hold = heat_slope_thresh_hold_210;
		break;
	}
	slope_slide_window_init(&ctrl->slope_slide_window);
	ctrl->gap_mode = gap_once;
}
void controller_reset(volatile temp_controller_t *ctrl)
{
	ctrl->temp_target = 220;
	ctrl->heat_gap_cnts_threshold = 0;
	ctrl->fict_heat_gap_cnts_threshold = 0;
	ctrl->heat_urgent_times = 0;
	ctrl->heat_urgent_gap_times = 0;
}
/**
 * @brief 插值计算
 * @param x_buf 输入数据
 * @param y_buf 输出数据
 * @param target_x 目标x值
 * @param size 数据大小
 * @return 插值结果
 */
float interp(const float *x_buf, const float *y_buf, float target_x, uint8_t size)
{
	uint8_t i, j;
	float result = 0.0f, term;

	for (i = 0; i < size; ++i)
	{
		term = y_buf[i];
		for (j = 0; j < size; ++j)
		{
			if (i != j)
			{
				term *= (target_x - x_buf[j]) / (x_buf[i] - x_buf[j]);
			}
		}
		result += term;
	}
	return result;
}

/**
 * @brief 根据目标温度更新温度控制器参数
 * @param ctrl 温度控制器结构体指针
 * @param temp_target 新的目标温度值
 *
 * 函数功能说明：
 * 1. 更新控制器的目标温度值
 * 2. 设置目标温度变化稳定计数器
 * 3. 根据目标温度插值计算加热间隔阈值和斜率阈值
 * 4. 设置触摸检测计数器
 * 5. 更新虚拟控制参数（用于仿真或预测）
 * 6. 根据目标温度与估计温度的关系设置加热标志
 */
void ctrl_params_update_from_target_temp(volatile temp_controller_t *ctrl, float temp_target)
{
	ctrl->temp_target = temp_target;
	ctrl->target_temp_update_stable_cnts = TARGET_TEMP_CHG_STABLE_CNTS;
	ctrl->heat_gap_cnts_threshold = interp(heat_normal_temp, ctrl->heat_gapcnts_threshold, temp_target, THRESHOLD_BUF_SIZE);
	ctrl->slope_threshold = interp(heat_normal_temp, ctrl->heat_slope_thresh_hold, temp_target, THRESHOLD_BUF_SIZE);

	ctrl->slope_touch_det_cnt = ctrl->heat_gap_cnts_threshold * 1.5;

	ctrl->fict_heat_gap_cnts_threshold = ctrl->heat_gap_cnts_threshold;
	ctrl->fict_slope_threshold = ctrl->slope_threshold;

	if (ctrl->temp_target > ctrl->temp_ested)
	{
		ctrl->target_heat_flag = 1;
	}
}
/***
 * @brief 根据虚拟目标温度更新温度控制器参数
 * @param ctrl 温度控制器结构体指针
 *
 * 函数功能说明：
 * 1. 更新虚拟控制参数（用于仿真或预测）
 * 2. 根据虚拟目标温度插值计算加热间隔阈值和斜率阈值
 * 3. 设置触摸检测计数器
 */
void ctrl_params_update_from_target_temp_fict(volatile temp_controller_t *ctrl)
{
	if (ctrl->touch_state == touch_none)
	{
		//		ctrl->heat_gap_cnts_threshold		= interp(heat_normal_temp, ctrl->heat_gapcnts_threshold, ctrl->temp_target, THRESHOLD_BUF_SIZE);
		//		ctrl->slope_threshold 				= interp(heat_normal_temp, ctrl->heat_slope_thresh_hold, ctrl->temp_target, THRESHOLD_BUF_SIZE);

		ctrl->fict_heat_gap_cnts_threshold = ctrl->heat_gap_cnts_threshold;
		ctrl->fict_slope_threshold = ctrl->slope_threshold;
	}
	else
	{
		ctrl->fict_heat_gap_cnts_threshold = interp(heat_normal_temp, ctrl->heat_gapcnts_threshold, ctrl->temp_target_fict, THRESHOLD_BUF_SIZE);
		ctrl->fict_slope_threshold = interp(heat_normal_temp, ctrl->heat_slope_thresh_hold, ctrl->temp_target_fict, THRESHOLD_BUF_SIZE);
	}
	ctrl->slope_touch_det_cnt = ctrl->heat_gap_cnts_threshold * 1.5;
}

void touch_state_update_from_slope(volatile temp_controller_t *ctrl)
{
	if (ctrl->slope_stable < ctrl->fict_slope_threshold * 1.2)
	{
		ctrl->touch_state = touch_little;
		if (ctrl->touch_state_last != ctrl->touch_state)
		{
			ctrl->touch_heat_flag = 1;
		}
	}
}

void touch_state_update_from_pre_heat(volatile temp_controller_t *ctrl)
{
	//	if(ctrl->heat_gap_cnts < ctrl->fict_heat_gap_cnts_threshold || ctrl->slope_stable < ctrl->fict_slope_threshold){
	//		ctrl->touch_state = touched;
	//		if(ctrl->touch_state_last != ctrl->touch_state){
	//			ctrl->touch_heat_flag = 1;
	//		}
	////	}else if( (ctrl->heat_gap_cnts - ctrl->fict_heat_gap_cnts_threshold) < 0.3*ctrl->fict_heat_gap_cnts_threshold
	////				&& ctrl->slope_stable  < ctrl->fict_slope_threshold ){
	////		ctrl->touch_state = touch_little;
	////		if(ctrl->touch_state_last == touch_none){
	////			ctrl->touch_heat_flag = 1;
	////		}
	//	}else{
	ctrl->touch_state = touch_none;
	//	}
}

void temp_target_fict_params_update(volatile temp_controller_t *ctrl)
{
	switch (ctrl->touch_state)
	{
	case touch_none:
		ctrl->temp_target_fict = -1;
		break;
		//		case touch_little:
		//			ctrl->temp_target_fict	=	1.05*ctrl->temp_target;
		//			break;
	case touched:
		ctrl->temp_target_fict = 1.20 * ctrl->temp_target;
		break;
	default:

		break;
	}

	ctrl_params_update_from_target_temp_fict(ctrl);
}

void cal_urgent_heat(volatile temp_controller_t *ctrl)
{
	float tmp;

	if (ctrl->target_heat_flag)
	{
		tmp = (ctrl->temp_target - ctrl->temp_ested) * ctrl->temp_target / 3000.0f;
		if (tmp > 10.0f)
		{
			tmp = 10.0f;
		}
		else if (tmp < 2.0f)
		{
			tmp = 2.0f;
		}

		ctrl->heat_urgent_times = (int16_t)tmp;
		if (ctrl->heat_urgent_times < 15)
		{
			ctrl->heat_urgent_gap_times = (int16_t)tmp;
		}
		else
		{
			ctrl->heat_urgent_gap_times = 10;
		}
		ctrl->heat_target_unstable_cnt = TOUCH_DETECT_SLOPE_COUNT;
		ctrl->target_heat_flag = 0;
	}
	else if (ctrl->touch_heat_flag)
	{
		if (ctrl->temp_target > 300)
		{
			ctrl->heat_urgent_times = 4;
			ctrl->heat_urgent_gap_times = 5;
		}
		else if (ctrl->temp_target > 200)
		{
			ctrl->heat_urgent_times = 4;
			ctrl->heat_urgent_gap_times = 4;
		}
		else if (ctrl->temp_target > 100)
		{
			ctrl->heat_urgent_times = 3;
			ctrl->heat_urgent_gap_times = 2;
		}

		ctrl->touch_heat_flag = 0;
	}
}

void cal_heat_flag(volatile temp_controller_t *ctrl)
{
	if (ctrl->temp_current > 600)
	{
		ctrl->ctrl_heat_flag = ctrl_heat_off;
	}
	else if (ctrl->temp_ested < ctrl->temp_target_slope_climb)
	{
		if (ctrl->touch_state_last == touched && (ctrl->touch_state == touch_none || ctrl->touch_state_last == touch_little))
		{
			ctrl->ctrl_heat_flag = ctrl_heat_off;
		}
		else
		{
			ctrl->ctrl_heat_flag = ctrl_heat_on;
		}
	}
	else
	{
		ctrl->ctrl_heat_flag = ctrl_heat_off;
		if (ctrl->heat_urgent_times > 0)
		{
			if (ctrl->heat_urgent_times > ctrl->heat_urgent_gap_times)
			{
				ctrl->ctrl_heat_flag = ctrl_heat_on;
				ctrl->heat_urgent_times--;
			}
			else
			{
				ctrl->ctrl_heat_flag = ctrl_heat_off;
				ctrl->heat_urgent_gap_times--;
			}
		}
	}

	//	switch(ctrl->gap_mode){
	//		case gap_none:
	//			break;
	//		case gap_once:
	//			if(ctrl->heat_gap_cnts < 1){
	//				ctrl->ctrl_heat_flag = ctrl_heat_off;
	//			}
	//			break;
	//		case gap_twice:
	//			if(ctrl->heat_gap_cnts < 2){
	//				ctrl->ctrl_heat_flag = ctrl_heat_off;
	//			}
	//			break;
	//		default:
	//			break;
	//	}
}

void controller_temp_est(volatile temp_controller_t *ctrl, float temp_current, uint8_t heat_flag_last_cycle)
{
	float k_tmp;
	ctrl->temp_current = temp_current;

	k_tmp = 450.0 - ctrl->temp_target;
	k_tmp = k_tmp / 2600;

	if (ctrl->touch_state == touched)
	{
		//			ctrl->B_est[0] = -k_tmp;
		ctrl->B_est[0] = 0;
		ctrl->B_est[1] = -k_tmp;
	}
	else if (ctrl->touch_state == touch_little)
	{
		//			ctrl->B_est[0] = -k_tmp/2;
		ctrl->B_est[0] = 0;
		ctrl->B_est[1] = -k_tmp / 2;
	}
	else
	{
		ctrl->B_est[0] = 0;
		ctrl->B_est[1] = 0;
	}
	ctrl->Bu[0] = ctrl->B_est[0] * ctrl->kf.x[1];
	ctrl->Bu[1] = ctrl->B_est[1] * ctrl->kf.x[1];

	if (heat_flag_last_cycle)
	{
		ctrl->Bu[0] += 5;
		ctrl->Bu[1] += 3;
	}

	//	ctrl->Bu[0] = 0;
	//	ctrl->Bu[1] = 0;

	kalman_update(&ctrl->kf, temp_current, (const float *)ctrl->Bu);

	ctrl->temp_filted = ctrl->kf.x[0];
	ctrl->temp_ested = ctrl->kf.x[1];
}

void reset_controller_touch_params(volatile temp_controller_t *ctrl)
{
	ctrl->heat_gap_cnts = 0;
	slope_slide_window_init(&ctrl->slope_slide_window);
}

void controller_state_update(volatile temp_controller_t *ctrl, uint8_t work_state_flag, uint8_t heat_flag_last)
{
	if (heat_flag_last > 0)
	{
		ctrl->heat_gap_cnts = 0;
	}
	ctrl->heat_gap_cnts++;

	if (work_state_flag == 0)
	{
		ctrl->temp_target_slope_climb = ctrl->temp_ested;
	}
	else
	{
		if (ctrl->temp_target_slope_climb < ctrl->temp_target)
		{
			ctrl->temp_target_slope_climb = ctrl->temp_target_slope_climb + TARGET_TEMP_CLIMB_STEP_LEN;
		}
		if (ctrl->temp_target_slope_climb >= ctrl->temp_target)
		{
			ctrl->temp_target_slope_climb = ctrl->temp_target;
		}
	}

	// 斜率计算
	slope_slide_window_push(&ctrl->slope_slide_window, heat_flag_last, ctrl->temp_ested);
	slope_slide_window_cal(&ctrl->slope_slide_window);

	ctrl->slope_stable = ctrl->slope_slide_window.slope;

	if (ctrl->temp_ested < ctrl->temp_target)
	{
		ctrl->pre_heat_flag = ctrl_heat_on;
	}
	else
	{
		ctrl->pre_heat_flag = ctrl_heat_off;
	}

	ctrl->touch_state_last = ctrl->touch_state;

	// 计算是否接触到物体
	//	if(		ctrl->pre_heat_flag == 0
	//		&& 	ctrl->heat_gap_cnts > ctrl->slope_touch_det_cnt
	//		&& 	ctrl->slope_slide_window.slope_stable_flag > 0 ){
	////		touch_state_update_from_slope(ctrl);
	//	}else
	if (ctrl->pre_heat_flag == ctrl_heat_on)
	{
		touch_state_update_from_pre_heat(ctrl);
	}

	temp_target_fict_params_update(ctrl);

	cal_urgent_heat(ctrl);

	cal_heat_flag(ctrl);
}
