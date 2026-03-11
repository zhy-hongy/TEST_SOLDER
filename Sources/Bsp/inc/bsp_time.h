#ifndef __BSP_TIME_H__
#define __BSP_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"


#ifdef THT_L075
#define POWER_CTRL_PWM_GPIO_PORT			GPIOB
#define POWER_CTRL_PWM_GPIO_PIN				GPIO_PIN_8

#define POWER_CTRL_PWM_TIM						TIM4
#define POWER_CTRL_PWM_TIM_RCC				RCC_APB1_PERIPH_TIM4
	
#define CH_CTRL_PWM_TIM								TIM5
#define CH_CTRL_PWM_TIM_RCC						RCC_APB1_PERIPH_TIM5
	
	
#define CH_CTRL_START_TIM_FREQ_100xHz		TIM5
#define CH_CTRL_START_TIM_RCC						RCC_APB1_PERIPH_TIM5
#define CH_CTRL_START_TIM_IRQn					TIM5_IRQn
#define CH_CTRL_START_TIM_IRQHandler		TIM5_IRQHandler
	
#endif

	
#define CH_CTRL_TIMER										TIM6
#define CH_CTRL_TIM_RCC									RCC_APB1_PERIPH_TIM6
#define CH_CTRL_TIM_IRQn								TIM6_IRQn
#define CH_CTRL_TIM_IRQHandler					TIM6_IRQHandler

	
void bsp_ctrl_tim_nvic_init(void);
void bsp_ctrl_tim_init(void);

void bsp_ctrl_tim_start_once(uint16_t us);
void bsp_ctrl_tim_stop(void);

void bsp_ctrl_start_tim_start(uint16_t us);
void bsp_tim_pwm_output_init(TIM_Module *TIMx, uint32_t tim_clk, uint8_t oc_ch, uint16_t arr, uint16_t psc);

#ifdef __cplusplus
}
#endif

#endif


