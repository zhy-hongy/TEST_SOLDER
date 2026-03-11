#include "kalman_2d_opt.h"
#include <string.h>
#include <math.h>

// 参数校验宏
#define CHECK_NULL(ptr) if(!(ptr)) return KF_ERR_NULL
#define CHECK_MATRIX(ptr) if(!(ptr) || isnan(*(const float*)(ptr))) return KF_ERR_INVALID

int kalman_init(volatile kalman_filter_2d_t* kf, const kalman_2d_init_params_t* params, float init_state) {
    // 一级指针检查
    CHECK_NULL(kf);
    CHECK_NULL(params);
    
    // 二级指针有效性校验
    CHECK_MATRIX(params->A);
    CHECK_MATRIX(params->B); 
    CHECK_MATRIX(params->H);
    CHECK_MATRIX(params->Q);
    CHECK_MATRIX(params->P_init);
    
    // 参数指针绑定
    kf->A = params->A;
    kf->B = params->B;
    kf->H = params->H;
    kf->Q = params->Q;
    kf->R = params->R;
    
    // 状态初始化
    kf->x[0] = init_state;
    kf->x[1] = 0.0f;
    
    // 协方差初始化
    memcpy((void*)kf->P, params->P_init, sizeof(kf->P)); 
    
    // 缓存清零
    memset((void*)&kf->cache, 0, sizeof(kf->cache));
    
    return KF_SUCCESS;
}

int kalman_update(volatile kalman_filter_2d_t* kf, float z, const float Bu[2]) {
    // 输入校验
    CHECK_NULL(kf);
    CHECK_NULL(Bu);
    CHECK_MATRIX(kf->A); // 运行时参数校验
    
    // 使用宏定义简化矩阵访问
    #define A(row,col) (kf->A[row][col])
    #define B(row,col) (kf->B[row][col])
    #define H(col)     (*(*kf->H + col)) // 行向量访问
    
    /*----- 预测阶段 -----*/
    // 状态预测：x_pred = A*x + B*u
//    kf->cache.x_pred[0] = A(0,0)*kf->x[0] + A(0,1)*kf->x[1];
//    kf->cache.x_pred[1] = A(1,0)*kf->x[0] + A(1,1)*kf->x[1];
	
	kf->cache.x_pred[0] = A(0,0)*kf->x[0] + A(0,1)*kf->x[1] + Bu[0];
    kf->cache.x_pred[1] = A(1,0)*kf->x[0] + A(1,1)*kf->x[1] + Bu[1];
                        
    // 协方差预测：P_pred = A*P*A^T + Q
    for(int i=0; i<2; i++) {
        for(int j=0; j<2; j++) {
            kf->cache.P_pred[i][j] = A(i,0)*(A(j,0)*kf->P[0][0] + A(j,1)*kf->P[1][0])
                                    + A(i,1)*(A(j,0)*kf->P[0][1] + A(j,1)*kf->P[1][1])
                                    + kf->Q[i][j];
        }
    }
    
    /*----- 更新阶段 -----*/
    // 新息协方差：S = H*P_pred*H^T + R
    float HP[2] = {
        H(0)*kf->cache.P_pred[0][0] + H(1)*kf->cache.P_pred[1][0],
        H(0)*kf->cache.P_pred[0][1] + H(1)*kf->cache.P_pred[1][1]
    };
    kf->cache.S = H(0)*HP[0] + H(1)*HP[1] + kf->R;
    
    if(fabsf(kf->cache.S) < 1e-6f) return KF_ERR_INVALID;
    float S_inv = 1.0f / kf->cache.S;
    
    // 卡尔曼增益：K = P_pred * H^T * S_inv
    kf->cache.K[0] = HP[0] * S_inv;
    kf->cache.K[1] = HP[1] * S_inv;
    
    // 状态更新
    float y = z - (H(0)*kf->cache.x_pred[0] + H(1)*kf->cache.x_pred[1]);
    kf->x[0] = kf->cache.x_pred[0] + kf->cache.K[0] * y;
    kf->x[1] = kf->cache.x_pred[1] + kf->cache.K[1] * y;
    
    // 协方差更新：P = (I - K*H) * P_pred
    float KH[2][2] = {
        {kf->cache.K[0]*H(0), kf->cache.K[0]*H(1)},
        {kf->cache.K[1]*H(0), kf->cache.K[1]*H(1)}
    };
    kf->P[0][0] = (1 - KH[0][0])*kf->cache.P_pred[0][0] - KH[0][1]*kf->cache.P_pred[1][0];
    kf->P[0][1] = (1 - KH[0][0])*kf->cache.P_pred[0][1] - KH[0][1]*kf->cache.P_pred[1][1];
    kf->P[1][0] = kf->P[0][1]; // 保持对称
    kf->P[1][1] = (1 - KH[1][1])*kf->cache.P_pred[1][1] - KH[1][0]*kf->cache.P_pred[0][1];
    
    return KF_SUCCESS;
    
    #undef A
    #undef B
    #undef H
}