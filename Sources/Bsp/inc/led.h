#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"

#define WORK_LED_CLK	RCC_APB2_PERIPH_GPIOB
#define WORK_LED_PORT	GPIOB
#define WORK_LED_PIN	GPIO_PIN_3

#define WORK_LED_OFF	GPIO_WriteBit(WORK_LED_PORT, WORK_LED_PIN, Bit_SET)
#define WORK_LED_ON		GPIO_WriteBit(WORK_LED_PORT, WORK_LED_PIN, Bit_RESET)
	
void work_led_init(void);
void work_led_blink(void); 
	
#ifdef __cplusplus
}
#endif

#endif


