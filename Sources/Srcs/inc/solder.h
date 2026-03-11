#ifndef __SOLDER_H__
#define __SOLDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"
#include "bsp_all.h"
#include "controller.h"
#include "temperature_cal.h"
#include "math.h"
#include "encrypt_func.h"


#define CHANNEL_NUMS							1
#define CHANNEL_NUMS_MAX						10

/*********** SOLDER ERROR *************/
//#define SOLDER_ERR_NONE							0
	

/*********** TYPE TEST *************/

#define RES_MIN									0.8
#define RES_MAX									6
#define RES_115_MIN                             3.1
#define RES_115_MAX                             3.8
#define RES_210_MIN                             0.9
#define RES_210_MAX                             2.6
#define RES_245_MIN                             2.6
#define RES_245_MAX                             3.5

#define CUR_HEAT_ON_MAX													20
#define CUR_HEAT_ON_MIN													0.2
#define CUR_HEAT_OFF_MAX												3.0
//#define CUR_HEAT_OFF_MIN												3.0




#define PCB_TEMP_MAX_ERR							60 // 板温警报温度
#define ILEAK_MAX_ERR								2.0 // 电流泄漏警报值


#define CUR_TEST_MAX	                  	 	 15 
#define CUR_TEST_MIN                     	 	 (-15)
#define TYPE_AGAIN_TEST_TIM                     200
	
//#define CH_ZERO_WAIT_TIME						450  //100

#ifdef THT_M120
#define CH_WAIT_TIME						    400  //600  

#define CH_MEASURE_TIME							300  //280
#endif

#ifdef THT_L075
#define CH_WAIT_TIME						    10  //600
#define CH_MEASURE_TIME							400  //280
#endif

#ifdef THT_M120
#define CH_FRST_HALF_TIM_115_50Hz_L			6500
#define CH_BOTM_HALF_TIM_115_50Hz_L			3500

#define CH_FRST_HALF_TIM_210_50Hz_L			3500
#define CH_BOTM_HALF_TIM_210_50Hz_L			7200

#define CH_FRST_HALF_TIM_245_50Hz_L			1800
#define CH_BOTM_HALF_TIM_245_50Hz_L			8200


#define CH_FRST_HALF_TIM_115_50Hz_H			6000
#define CH_BOTM_HALF_TIM_115_50Hz_H			4000

#define CH_FRST_HALF_TIM_210_50Hz_H			1200
#define CH_BOTM_HALF_TIM_210_50Hz_H			9000

#define CH_FRST_HALF_TIM_245_50Hz_H			10
#define CH_BOTM_HALF_TIM_245_50Hz_H			10000


#define CH_FRST_HALF_TIM_115_60Hz_L			6000
#define CH_BOTM_HALF_TIM_115_60Hz_L			4000

#define CH_FRST_HALF_TIM_210_60Hz_L			2800
#define CH_BOTM_HALF_TIM_210_60Hz_L			7500

#define CH_FRST_HALF_TIM_245_60Hz_L			1500
#define CH_BOTM_HALF_TIM_245_60Hz_L			8500


#define CH_FRST_HALF_TIM_115_60Hz_H			5500
#define CH_BOTM_HALF_TIM_115_60Hz_H			4500

#define CH_FRST_HALF_TIM_210_60Hz_H			500
#define CH_BOTM_HALF_TIM_210_60Hz_H			9000

#define CH_FRST_HALF_TIM_245_60Hz_H			10
#define CH_BOTM_HALF_TIM_245_60Hz_H			10000
#endif


#ifdef THT_L075
#define CH_FRST_HALF_TIM_115_50Hz_L 		6500
#define CH_BOTM_HALF_TIM_115_50Hz_L			3500

#define CH_FRST_HALF_TIM_210_50Hz_L			3500
#define CH_BOTM_HALF_TIM_210_50Hz_L			7200

#define CH_FRST_HALF_TIM_245_50Hz_L			1800
#define CH_BOTM_HALF_TIM_245_50Hz_L			8200


#define CH_FRST_HALF_TIM_115_50Hz_H			6000
#define CH_BOTM_HALF_TIM_115_50Hz_H			4000

#define CH_FRST_HALF_TIM_210_50Hz_H			1200
#define CH_BOTM_HALF_TIM_210_50Hz_H			9000

#define CH_FRST_HALF_TIM_245_50Hz_H			10
#define CH_BOTM_HALF_TIM_245_50Hz_H			10000


#define CH_FRST_HALF_TIM_115_60Hz_L			6000
#define CH_BOTM_HALF_TIM_115_60Hz_L			4000

#define CH_FRST_HALF_TIM_210_60Hz_L			2800
#define CH_BOTM_HALF_TIM_210_60Hz_L			7500

#define CH_FRST_HALF_TIM_245_60Hz_L			1500
#define CH_BOTM_HALF_TIM_245_60Hz_L			8500


#define CH_FRST_HALF_TIM_115_60Hz_H			5500
#define CH_BOTM_HALF_TIM_115_60Hz_H			4500

#define CH_FRST_HALF_TIM_210_60Hz_H			500
#define CH_BOTM_HALF_TIM_210_60Hz_H			9000

#define CH_FRST_HALF_TIM_245_60Hz_H			10
#define CH_BOTM_HALF_TIM_245_60Hz_H			10000

// 控制输出PWM信号，50为50us，低电平时间50us,占空比为95%
#define CH_CTRL_PWM_OUTPUT_TIM_LOAD			50
#endif


#define CH_FRST_HALF_TIM						10
#define CH_BOTM_HALF_TIM						9500

#define CH_TEST_HEAT_TIME						1800


#define POWER_SUM_LEN							50

/**
 * @brief 延时执行代码块宏，每隔指定时间执行一次代码块
 * @param interval_ms 执行间隔（毫秒）
 * @param code_block 要执行的代码块，用 {} 包裹
 * 
 * 注意：system_time 最大值为 SYSTEM_TIME_MAX，会自动处理溢出
 */
#define DELAY_EXECUTE(interval_ms, code_block) \
    do { \
        static uint32_t last_time_##__LINE__ = 0; \
        uint32_t current_time = getSystemTime(); \
        uint32_t elapsed_time = 0; \
        \
        /* 处理时间溢出（当 current_time < last_time_时） */ \
        if (current_time >= last_time_##__LINE__) { \
            elapsed_time = current_time - last_time_##__LINE__; \
        } else { \
            /* 时间溢出，计算从 last_time_ 到 SYSTEM_TIME_MAX + 1 再到 current_time 的时间 */ \
            elapsed_time = (SYSTEM_TIME_MAX - last_time_##__LINE__ + 1) + current_time; \
        } \
        \
        if (elapsed_time > (interval_ms)) { \
            last_time_##__LINE__ = current_time; \
            code_block \
        } \
    } while(0)

// channel总工作状态
typedef enum{
	ch_ideal_notool = 0,  
	ch_tool_in_test, 	
	ch_tool_quality_test,
	ch_sleep, 
	ch_working, 
	ch_err, 
	ch_set,
	ch_debug
} ch_stage_t;

// 底层状态
typedef enum{
	heat_none = 0, 
	heat_norm, 
	heat_norm_frst_half, heat_norm_botm_half, 
	heat_calibration,
	heat_min_sleep,
	heat_test,
	heat_debug_once, 
	heat_debug_frst_half, heat_debug_botm_half
}	heat_mode_t;

typedef enum{
	ctrl_ideal = 0, 
	ctrl_wait_zero, 
	ctrl_wait_measure,  
	ctrl_measure,
	ctrl_frst_half, 
	ctrl_botm_half
} ctrl_state_t;

typedef enum : uint8_t{
	heat_off = 0, heat_on = 1
} heat_state_t;

typedef enum{
	zero_err, zero_norm, zero_none
} zero_state_t;

typedef struct{
	uint8_t temp_target_chg_ntf;
	uint8_t ch_set_ntf;
	uint8_t mode_set_ntf;
} key_ntf_t;

typedef struct{
	uint8_t 		length;
	heat_state_t	heat_state[VOL_CUR_DATA_LENGTH];
	float 			ileak[VOL_CUR_DATA_LENGTH];
	float			voltage[VOL_CUR_DATA_LENGTH];
	float			current[VOL_CUR_DATA_LENGTH];
	float			pcb_temp[VOL_CUR_DATA_LENGTH];
	float 			resist[VOL_CUR_DATA_LENGTH];
	uint8_t			vol_cur_error_array[VOL_CUR_DATA_LENGTH];
} vol_cur_data_t;

typedef enum{
	ch_set_stage_ideal, ch
} ch_set_stage_t;

typedef enum{
	ch_set_mode_none, ch_set_mode_up_down_inc, ch_set_mode_ch1, ch_set_mode_ch2, ch_set_mode_ch3,
	ch_set_mode_calibration, ch_set_mode_power_high, ch_set_mode_min_temp
} ch_set_mode_t;

typedef enum : uint16_t{
	param_target_temp = 0,
	param_target_temp_inc,
	param_target_temp_ch1,
	param_target_temp_ch2,
	param_target_temp_ch3,
	param_control_power_high,
	
	param_temp_cal_115_flag,
	param_temp_cal_115_100,
	param_temp_cal_115_200,
	param_temp_cal_115_300,
	param_temp_cal_115_400,
	
	param_temp_cal_210_flag,
	param_temp_cal_210_100,
	param_temp_cal_210_200,
	param_temp_cal_210_300,
	param_temp_cal_210_400,
	
	param_temp_cal_245_flag,
	param_temp_cal_245_100,
	param_temp_cal_245_200,
	param_temp_cal_245_300,
	param_temp_cal_245_400,

	
	param_temp_cal_470_flag,
	param_temp_cal_470_100,
	param_temp_cal_470_200,
	param_temp_cal_470_300,
	param_temp_cal_470_400,
    param_min_temp,
	param_min_temp_flag,
	param_num_max
} param_name_t;

typedef struct{
	uint16_t 		param_loc;
	uint16_t 		param_length;	
} solder_param_info_t;


typedef enum
{
	//calibration_step_idle,
	calibration_step_100 = 0,
	calibration_step_200,
	calibration_step_300,
	calibration_step_400
	
} ch_set_calibration_t;

typedef struct {	
    uint8_t                     channel;                     // 通道编号，标识不同的焊锡通道
    ch_stage_t                  ch_stage;                    // 当前通道工作阶段（如无工具、工作中、睡眠等）
    ch_set_stage_t              ch_set_stage;                // 设置阶段状态
    ch_set_mode_t               ch_set_mode;                 // 设置模式（如上下调节、校准模式等）
    ch_set_calibration_t        ch_set_step;                 // 校准步骤（如100°C、200°C等校准点）
    uint8_t                     set_complete_flag;           // 设置完成标志位
    float                       ch_set_calibration_temp[6];  // 校准温度缓冲区，存储不同校准点的温度值
    uint8_t                     ch_temp_param_update_ntf;    // 温度参数更新通知标志
    uint8_t                     ch_temp_cal_param_udpate_ntf;// 温度校准参数更新通知标志
    uint8_t                     ch_power_param_udpate_ntf;   // 功率参数更新通知标志
    uint8_t                     ch_param_update_ntf;         // 通道参数更新通知标志
    
    float                       ch_set_set_disp;             // 设置界面显示值
    float                       ch_set_temp_disp;            // 设置界面温度显示值
    
    uint8_t                     ch_last_stage;               // 上一周期的通道阶段，用于阶段转换检测
    uint8_t                     ch_stage_frstin_flag;        // 首次进入当前阶段标志，用于初始化操作
    uint8_t                     ch_sleep_stage_frstin_flag;  // 首次进入睡眠阶段标志
    uint8_t                     ch_frstin_set_temp_flag;     // 首次进入设置温度标志
    uint8_t                     tool_state_update_sw;        // 工具状态更新开关，控制工具状态更新逻辑

    uint16_t                    ch_err_flag;                 // 通道错误标志位，使用位掩码记录不同类型错误
    uint16_t                    ch_err_cnt[16];              // 错误计数数组，记录不同类型错误的发生次数
    uint16_t                    ch_err_wait_cnt;             // 错误等待计数，用于错误恢复计时
    uint16_t                    ch_temp_error_continue_cnt;  // 温度错误连续计数，用于检测连续温度异常
    uint16_t                    ch_temp_error_continue_gap_cnt; // 温度正常连续计数，用于重置温度错误连续计数
//  uint16_t                    ch_err_bits;
//  uint8_t                     ch_err_bits_cnts[16];
    
    ctrl_state_t                ctrl_state;                  // 通道控制状态，与系统控制状态同步
    temp_controller_t           controller;                  // 温度控制器实例，包含温度控制算法所需的所有参数和状态
    
    uint16_t                    ctrl_temp_ok_first_ntf;      // 温度达标首次通知标志，用于首次达到目标温度时的提示
    uint16_t                    temp_reset_ntf;              // 温度重置通知标志，用于重置温度显示
    
    float                       temp_target;                 // 目标温度值，用户设置的期望温度
    float                       temp_target_slope_climb;     // 目标温度爬升斜率，用于控制温度上升速度
    float                       temp_target_inc;             // 目标温度增量，用于逐步调整目标温度
    
    float                       temp_current;                // 当前实际温度值，通过ADC采样并转换得到
    float                       temp_current_last_cycle;     // 上一周期的实际温度值，用于温度变化率检测
    float                       temp_est_last_cycle;         // 上一周期的温度估计值，用于卡尔曼滤波参考
    float                       temp_show;                   // 显示温度值，经过滤波处理后展示给用户的温度
    
    tool_in_state_t             tool_in_state;               // 工具插入状态（如未插入、已插入等）
    tool_type_t                 tool_type, tool_type_1, tool_type_2, tool_type_3; // 工具类型标识，支持多种工具检测
    sleep_state_t               sleep_state;                 // 睡眠状态标识
    
    uint16_t                    tool_in_wait_cnt;            // 工具插入等待计数，用于工具插入检测的防抖
    uint16_t                    ch_tool_quality_test_cnt;    // 工具质量测试计数器，用于工具质量检测的循环次数
    uint16_t                    work_to_sleep_wait_cnt;      // 工作转睡眠等待计数，用于自动进入睡眠模式计时
    uint16_t                    tool_to_sleep_wait_cnt;      // 工具转睡眠等待计数，用于工具插入后的睡眠控制
    uint16_t                    ch_set_timeout_cnt;          // 设置模式超时计数器，用于自动回归上一次模式
    // uint16_t                    ch_sleep_timeout_cnt;       // 睡眠模式超时计数器
    uint16_t                    ch_set_timeout_max;          // 设置模式超时最大值，默认30秒（3000个10ms周期）
	
    float                       type_test_avg_res;           // 工具类型测试的平均电阻值，用于工具类型识别

    float                       pcb_temp_avg;           	// 板温的平均温度值，用于板温识别
    
    heat_mode_t                 heat_mode;                   // 加热模式（如无加热、正常加热、校准加热等）
    uint8_t                     heat_test_cnts;              // 加热测试计数器，用于测试模式的加热次数控制
    uint8_t                     heat_test_max_cnts;          // 加热测试最大次数，测试模式下的加热上限

    uint16_t                    heat_time_frst_half;         // 第一半周期加热时间，控制正半周的加热时长
    uint16_t                    heat_time_botm_half;         // 第二半周期加热时间，控制负半周的加热时长

    heat_state_t                heat_state;                  // 当前加热状态（开/关）

    ctrl_heat_flag_t            heat_ctrl_flag;              // 加热控制标志，控制加热输出
    ctrl_heat_flag_t            heat_flag_last_cycle;        // 上一周期的加热控制标志，用于功率计算和状态参考

    uint8_t                     power_high_flag;             // 高功率模式标志
    uint16_t                    cal_115_flag, cal_210_flag, cal_245_flag; // 不同工具类型的校准标志
    
    uint8_t                     power_AC_HZ_flag;            // 电源频率标志（1表示50Hz，0表示60Hz）
    uint16_t                    power_gear;                  // 功率档位，控制输出功率大小
    uint8_t                     power_rec_buf[POWER_SUM_LEN];// 功率记录缓冲区，用于功率统计
    uint8_t                     power_buf_index;             // 功率缓冲区索引
    uint16_t                    power_sum;                   // 功率总和，用于功率统计计算
    uint32_t                    power_exceed_cnts;           // 功率超限连续计数
    uint32_t                    power_exceed_gap_cnts;       // 功率正常连续计数，用于重置超限计数
    
    float                       min_temp;                   // 休眠最低温度值
    uint16_t                    min_temp_enable_flag;       // 休眠最低温度使能标志
} solder_ch_state_t;

 
typedef struct{
	
	solder_ch_state_t  *chs[CHANNEL_NUMS_MAX];
	
	uint16_t            init_over;               // 初始化完成标志，置1表示系统初始化完成
	uint16_t            dev_init_over;           // 设备初始化完成标志，置1表示设备初始化完成

	uint16_t            err_flag;                // 错误标志，使用位掩码记录不同类型的系统级错误
	uint16_t            err_cnt;                 // 错误计数，记录系统级错误发生的次数

	ctrl_state_t        ctrl_state;              // 系统控制状态，与通道控制状态同步

	vol_cur_data_t      voltage_current;         // 电压电流数据结构体，存储系统的电压和电流信息

	uint32_t            zero_up_irq_tim_record;  // 过零中断时间记录，用于计算过零间隔

	uint16_t            disp_maintain_state;     // 显示维持状态，控制显示的持续时间
	uint16_t            disp_flicker_cnt;        // 显示闪烁计数，用于控制显示闪烁频率
	uint16_t            disp_flicker_state;      // 显示闪烁状态，标记当前显示是否处于闪烁状态

	uint16_t            zero_wait_time;          // 过零等待时间，用于控制过零后的等待时长
	uint16_t            temp_wait_time;          // 温度等待时间，用于控制温度测量前的等待时长
	uint16_t            measure_time;            // 测量时间，用于控制温度测量的时长

	uint8_t             prior_channel;           // 优先通道，标记当前优先处理的通道
	uint8_t             output_channel;          // 输出通道，标记当前有输出的通道
	uint8_t             heat_ctrl_flag;          // 加热控制标志，控制系统级的加热使能/禁用

	uint8_t             active_channel;          // 活动通道，标记当前活跃的通道

	#ifdef THT_L075                               // 特定硬件平台条件编译
	uint16_t            heat_pwm_ctrl_total_cycle_cnt;  // 加热PWM控制总周期计数，用于PWM周期管理
	uint16_t            heat_pwm_ctrl_low_period;        // 加热PWM控制低电平周期，用于PWM占空比控制
	uint16_t 						heat_pwm_ctrl_on_off_flag;
	uint16_t            voltage_duty_now;                // 当前电压占空比，记录当前的PWM占空比
	uint16_t            voltage_duty_goal;               // 目标电压占空比，记录目标的PWM占空比
	#endif
} solder_state_t;

typedef struct {
	uint8_t 			unit_C_F;
	uint8_t 			display_error_flag;
	uint8_t				tool_state;
	uint8_t 			power_gear;
} manual_config_t;


extern volatile solder_state_t 		solder;
extern volatile solder_ch_state_t 	solder_channel;   

extern uint8_t enc_key[];
extern uint8_t enc_rand[];


extern uint8_t enc_key_old[];

void solder_dev_init(void);
void solder_param_init(void);

void solder_param_load_all_from_rom();
void solder_param_load_dev_init_from_rom();
void solder_param_load_temp_calibration_from_rom();
void solder_parameters_update_to_rom();



void solder_param_save(void* params, param_name_t name);
void solder_param_get(void* params, param_name_t name);

void solder_param_write_to_array(void* params, param_name_t name);
void solder_param_get_from_array(void* params, param_name_t name);

// void solder_ch_stage_process(volatile solder_ch_state_t* ch);

#ifdef THT_L075
void adjust_power_voltage(float voltage);
void adjust_power_voltage_poll();
#endif

uint8_t get_voltage_current_array(void);
void calibration_param_to_rom(tool_type_t tool);
void calibration_param_to_array(tool_type_t tool);
void calibration_param_to_calibration_buf(tool_type_t tool,volatile float *calibration_temp_buf );
void key_process();
void screen_display();
void screen_display_test();
void frequency_check(void);


#ifdef __cplusplus
}
#endif

#endif




