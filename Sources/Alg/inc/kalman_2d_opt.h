#ifndef KALMAN_2D_OPT_H
#define KALMAN_2D_OPT_H

#include <stdint.h>

#define KF_SUCCESS       0
#define KF_ERR_NULL      1
#define KF_ERR_INVALID   2

typedef struct {
    // 核心参数指针（只读）
    const float A[2][2];    // 状态转移矩阵
    const float B[2][2];    // 控制输入矩阵
    const float H[2][2];    // 观测矩阵（行向量）
    const float Q[2][2];    // 过程噪声协方差
    const float P_init[2][2]; // 初始协方差
    float R;                // 观测噪声
} kalman_2d_init_params_t;

typedef struct {
    // 动态状态
    float x[2];          // 状态向量 [pos, vel]
    float P[2][2];       // 误差协方差
    
    // 固定参数（指针引用）
    const float (*A)[2];
    const float (*B)[2];
    const float (*H)[2];
    const float (*Q)[2];
    float R;
    
    // 临时计算缓存
    struct {
        float x_pred[2];
        float P_pred[2][2];
        float K[2];
        float S;
    } cache;
} kalman_filter_2d_t;

/**
 * @brief 初始化卡尔曼滤波器（指针引用模式）
 * @param kf 滤波器实例指针
 * @param params 初始化参数结构体
 * @param init_state 初始状态值
 * @return 状态码 0=成功
 */
int kalman_init(volatile kalman_filter_2d_t* kf, const kalman_2d_init_params_t* params, float init_state);

/**
 * @brief 带控制输入的卡尔曼更新
 * @param kf 滤波器实例指针
 * @param z 观测值
 * @param u 控制输入向量（2维）
 * @return 状态码
 */
int kalman_update(volatile kalman_filter_2d_t* kf, float z, const float Bu[2]);

#endif // KALMAN_FILTER_PTR_H