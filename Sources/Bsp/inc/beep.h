#ifndef __BEEP_H__
#define __BEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"
#include "bsp_time.h"

#define BEEP_CLK   		RCC_APB2_PERIPH_GPIOA
#define BEEP_PIN   		GPIO_PIN_15
#define BEEP_PORT  		GPIOA

#define BEEP_TIME_RCC 	RCC_APB1_PERIPH_TIM2	
#define BEEP_TIME      	TIM2
	
#define BEEP_ON 		TIM_Enable(BEEP_TIME, ENABLE)
#define BEEP_OFF 		TIM_Enable(BEEP_TIME, DISABLE)

extern volatile uint8_t beep_once_ntf;
	
void beep_init();
void beep_pwm_set(uint16_t duty);
void beep_poll(void);

#ifdef __cplusplus
}
#endif

#endif


