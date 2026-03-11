#include "slope_slide_window.h"

void slope_slide_window_init(volatile slope_slide_window_t *slide_window)
{
	slide_window->head = 0;	
	slide_window->tail = 0;
	slide_window->flag_gap_cnt = 0;
	slide_window->stable_cnt = SLOPE_STABLE_WAIT_CNT;
}

void slope_slide_window_reset(volatile slope_slide_window_t *slide_window)
{

}

/**
 * @brief 滑动窗口数据推送函数
 * 
 * 将新的温度数据和加热标志推送到滑动窗口中，用于后续的斜率计算。
 * 该函数维护一个循环缓冲区，实现滑动窗口的数据管理。
 * 
 * @param slide_window 指向滑动窗口结构体的指针
 * @param heat_flag_last 上一周期的加热标志（0=未加热，>0=已加热）
 * @param data 当前周期的温度数据
 */
void slope_slide_window_push(volatile slope_slide_window_t *slide_window, uint8_t heat_flag_last, float data)
{
    // 如果上一周期有加热操作，重置窗口计数器和尾指针
    if(heat_flag_last > 0){
        slide_window->flag_gap_cnt = 0;  // 重置加热间隙计数器
        slide_window->tail = 0;          // 重置缓冲区尾指针，重新开始填充
    }
    
    // 如果加热间隙计数小于稳定等待计数，重置稳定计数器
    // 这表示窗口数据还不够稳定，需要继续收集数据
    if(slide_window->flag_gap_cnt <= SLOPE_STABLE_WAIT_CNT){
        slide_window->stable_cnt = 0;    // 重置稳定计数器
    }
    
    // 将数据存储到滑动窗口的各个缓冲区中
    slide_window->x_buf[slide_window->tail] = (float)slide_window->flag_gap_cnt;        // X轴：加热间隙计数（时间序列）
    slide_window->y_buf[slide_window->tail] = data;                                     // Y轴：温度数据
    slide_window->xy_buf[slide_window->tail] = (float)slide_window->flag_gap_cnt * data; // XY乘积：用于斜率计算
    slide_window->xx_buf[slide_window->tail] = (float)slide_window->flag_gap_cnt * (float)slide_window->flag_gap_cnt; // X平方：用于斜率计算
    
    // 更新指针和计数器
    slide_window->tail++;           // 尾指针后移，指向下一个存储位置
    slide_window->flag_gap_cnt++;   // 加热间隙计数递增（时间推进）
    slide_window->stable_cnt++;     // 稳定计数器递增
    
    // 循环缓冲区处理：当尾指针到达窗口宽度时，回绕到起始位置
    if(slide_window->tail == SLIDE_WINDOW_WIDTH){
        slide_window->tail = 0;     // 尾指针回绕，实现循环缓冲区
    }
}
/**
 * @brief 滑动窗口斜率计算函数
 * 
 * 使用最小二乘法线性回归计算滑动窗口中温度数据的斜率。
 * 该函数用于检测温度变化的趋势，判断烙铁头是否接触工件。
 * 
 * @param slide_window 指向滑动窗口结构体的指针
 */
void slope_slide_window_cal(volatile slope_slide_window_t *slide_window)
{
	volatile uint16_t i, buf_index = 0;
	volatile float sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;

	// 确定计算窗口的起始位置和有效数据点数
	if(slide_window->stable_cnt > SLIDE_WINDOW_WIDTH){
		// 数据已填满整个窗口，使用最新的SLIDE_WINDOW_WIDTH个点
		slide_window->head = slide_window->tail;        // 头指针指向当前尾指针位置
		slide_window->slope_cnt = SLIDE_WINDOW_WIDTH;   // 使用完整的窗口宽度进行计算
	}else{
		// 数据未填满窗口，从稳定等待期之后开始计算
		slide_window->head = SLOPE_STABLE_WAIT_CNT;     // 跳过前10个不稳定数据点
		slide_window->slope_cnt = slide_window->stable_cnt;  // 使用实际收集的数据点数
	}
	
	// 只有当有效数据点数大于最小计算阈值时才进行斜率计算
	if(slide_window->stable_cnt > SLOPE_STABLE_CAL_CNT_MIN){
		// 遍历滑动窗口中的数据点，计算各项累加和
		for (int i = slide_window->head; i < slide_window->slope_cnt + slide_window->head; i++) {
				// 处理循环缓冲区的索引回绕
				if(i < SLIDE_WINDOW_WIDTH){
						buf_index = i;  // 索引在窗口范围内，直接使用
				}else{
						buf_index = i - SLIDE_WINDOW_WIDTH;  // 索引超出范围，回绕到起始位置
				}
				
				// 累加各项统计量，用于最小二乘法计算
				sum_x  += slide_window->x_buf[buf_index];    // X值累加（时间序列）
				sum_y  += slide_window->y_buf[buf_index];    // Y值累加（温度值）
				sum_xy += slide_window->xy_buf[buf_index];   // X*Y乘积累加
				sum_xx += slide_window->xx_buf[buf_index];   // X平方累加
		}
		
		// 使用最小二乘法计算斜率：slope = (n*Σxy - Σx*Σy) / (n*Σx? - (Σx)?)
		slide_window->slope = (slide_window->slope_cnt * sum_xy - sum_x * sum_y) / 
							(slide_window->slope_cnt * sum_xx - sum_x * sum_x);
		
		// 计算截距：intercept = (Σy - slope*Σx) / n
		slide_window->intercept = (sum_y - (slide_window->slope) * sum_x) / slide_window->slope_cnt;		
		
		// 设置斜率稳定标志，表示计算有效
		slide_window->slope_stable_flag = 1;
	}else{
		// 数据点数不足，无法进行有效计算
		slide_window->slope = 0;            // 斜率设为0
		slide_window->intercept = 0;        // 截距设为0
		slide_window->slope_stable_flag = 0; // 设置斜率不稳定标志
	}
}
