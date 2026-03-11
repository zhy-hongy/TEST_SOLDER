#ifndef __IWDG_CONFIG_H__
#define __IWDG_CONFIG_H__

#include "n32g43x.h"

// 看门狗配置宏定义
#define IWDG_TIMEOUT_MS          1000     // 看门狗超时时间(ms)
#define IWDG_PRESCALER           IWDG_PRESCALER_DIV32  // 预分频器
#define IWDG_CLOCK_SOURCE_FREQ   40000   // LSI频率(Hz)，可根据实际测量调整

// 函数声明
void IWDG_Init(void);
void IWDG_Feed(void);
uint8_t IWDG_CheckResetFlag(void);
void IWDG_ClearResetFlag(void);

#endif /* __IWDG_CONFIG_H__ */