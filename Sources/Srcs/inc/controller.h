#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"
#include "debug.h"
#include "kalman_2d_opt.h"
#include "slope_slide_window.h"
#include "tool_det.h"

#define HEAT_ONCE_CNT_MAX			200
	
#define THRESHOLD_BUF_SIZE 			9
	
#define TARGET_TEMP_CHG_STABLE_CNTS	100
	
#define TOUCH_DETECT_SLOPE_COUNT	200
	
#define TARGET_TEMP_CLIMB_STEP_LEN	15
	
typedef enum{
	temp_sw, temp_stable, touch
} control_state_t;
	
typedef enum{
	touch_none, touch_little, touched
} touch_state_t;

typedef enum{
	whole = 0, frst_half, botm_half
} ctrl_heat_time_t;

typedef enum{
	gap_none = 0, gap_once, gap_twice, gap_third
} ctrl_heat_gap_mode_t;

typedef enum: uint8_t{
	ctrl_heat_off = 0, ctrl_heat_on = 1
} ctrl_heat_flag_t;

typedef struct{
	/* 目标温度相关参数 */
    float                   temp_target;                    /**< 目标温度设定值 (°C) */
    float                   temp_target_slope_climb;        /**< 斜坡升温过程中的临时目标温度 */
    float                   temp_target_fict;               /**< 虚拟目标温度，用于控制算法 */
    int16_t                 target_temp_update_stable_cnts; /**< 目标温度更新稳定计数器 */
	
	/* 当前温度与滤波相关 */
    float                   temp_current;                   /**< 当前测量的温度值 (°C) */
    kalman_filter_2d_t      kf;                             /**< 卡尔曼滤波器实例，用于温度状态估计 */
    float                   Bu[2];                          /**< 控制输入向量 B*u */
    float                   B_est[2];                       /**< 估计的系统矩阵B */
    float                   temp_filted;                    /**< 滤波后的温度值 */
    float                   temp_ested;                     /**< 估计的温度值 */
	
	/* 加热控制标志 */
    ctrl_heat_flag_t        pre_heat_flag;                  /**< 预热阶段加热标志 */
    ctrl_heat_flag_t        ctrl_heat_flag;                 /**< 主控制加热标志 */
	
	/* 功率控制 */
    float                   power_percent;                  /**< 当前功率百分比 (0-100%) */
    
    /* 斜率检测相关 */
    slope_slide_window_t    slope_slide_window;             /**< 滑动窗口斜率检测器 */
    float                   slope_stable;                   /**< 稳定状态下的斜率值 */
    float                   slope_fix_20;                   /**< 固定20点的斜率值 */
    float                   slope_threshold;                /**< 斜率检测阈值 */
    float                   fict_slope_threshold;           /**< 虚拟斜率阈值 */
    uint16_t                slope_buffer_index;             /**< 斜率缓冲区索引 */
	
	/* 加热状态标志 */
    uint8_t                 target_heat_flag;               /**< 目标温度加热标志 */
    uint8_t                 touch_heat_flag;                /**< 触摸加热标志 */
    int16_t                 heat_target_unstable_cnt;       /**< 加热目标不稳定计数器 */
    
    /* 加热间隙模式 */
    ctrl_heat_gap_mode_t    gap_mode;                       /**< 加热间隙模式 (无间隙/单次/双次/三次) */
    
    /* 加热间隙控制 */
    int16_t                 heat_gap_cnts;                  /**< 当前加热间隙计数 */
    int16_t                 heat_gap_cnts_threshold;        /**< 加热间隙计数阈值 */
    int16_t                 fict_heat_gap_cnts_threshold;   /**< 虚拟加热间隙计数阈值 */
    float                   *heat_gapcnts_threshold;        /**< 指向加热间隙阈值数组的指针 */
    float                   *heat_slope_thresh_hold;        /**< 指向加热斜率阈值数组的指针 */
        
    /* 触摸检测 */
    int16_t                 slope_touch_det_cnt;            /**< 斜率触摸检测计数器 */
    
    /* 触摸状态 */
    touch_state_t           touch_state;                    /**< 当前触摸状态 (无触摸/轻微触摸/已触摸) */
    touch_state_t           touch_state_last;               /**< 上一次触摸状态 */
    
    /* 紧急加热控制 */
    int16_t                 heat_urgent_times;              /**< 紧急加热次数 */
    int16_t                 heat_urgent_gap_times;          /**< 紧急加热间隙次数 */
	
} temp_controller_t;

void controller_init(volatile temp_controller_t* ctrl, tool_type_t tool_type);
void controller_reset(volatile temp_controller_t* ctrl);

void ctrl_params_update_from_target_temp(volatile temp_controller_t* ctrl, float target_temp);
void ctrl_params_update_from_target_temp_fict(volatile temp_controller_t* ctrl);

void controller_temp_est(volatile temp_controller_t* ctrl, float temp_current, uint8_t heat_flag_last_cycle);
void controller_state_update(volatile temp_controller_t* ctrl,  uint8_t work_state_flag, uint8_t heat_flag);

#ifdef __cplusplus
}
#endif

#endif