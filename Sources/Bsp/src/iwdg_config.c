#include "iwdg_config.h"

/**
 * @brief 初始化独立看门狗
 * @param 无
 * @retval 无
 */
void IWDG_Init(void)
{
    // 检查是否由看门狗复位
    if (IWDG_CheckResetFlag())
    {
        // 看门狗复位标志已设置，可以在此处处理复位后的操作
        IWDG_ClearResetFlag();  // 清除复位标志
    }
    
    // 允许写入看门狗配置寄存器
    IWDG_WriteConfig(IWDG_WRITE_ENABLE);
    
    // 设置预分频器
    IWDG_SetPrescalerDiv(IWDG_PRESCALER);

    uint32_t f_counter = IWDG_CLOCK_SOURCE_FREQ / 32;
     uint32_t reload_value = f_counter;
    // 设置重载值
    IWDG_CntReload(reload_value);
    // IWDG_CntReload(IWDG_CLOCK_SOURCE_FREQ/128);
    
    // 重载看门狗计数器
    IWDG_ReloadKey();
    
    // 启用看门狗
    IWDG_Enable();
}

/**
 * @brief 喂狗操作
 * @param 无
 * @retval 无
 */
void IWDG_Feed(void)
{
    IWDG_ReloadKey();
}

/**
 * @brief 检查看门狗复位标志
 * @param 无
 * @retval 1: 由看门狗复位, 0: 非看门狗复位
 */
uint8_t IWDG_CheckResetFlag(void)
{
    return (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_IWDGRSTF) != RESET);
}

/**
 * @brief 清除看门狗复位标志
 * @param 无
 * @retval 无
 */
void IWDG_ClearResetFlag(void)
{
    RCC_ClrFlag();
}