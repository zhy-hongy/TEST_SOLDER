#ifndef __HEAT_CTRL_H__
#define __HEAT_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"


#define POWER_CTRL_CLK       RCC_APB2_PERIPH_GPIOA
#define POWER_CTRL_PORT      GPIOA
#define POWER_CTRL_PIN       GPIO_PIN_10

#ifdef THT_M120
#define CH_HeatOn		  GPIO_WriteBit(POWER_CTRL_PORT, POWER_CTRL_PIN, Bit_SET)
#define CH_HeatOff		GPIO_WriteBit(POWER_CTRL_PORT, POWER_CTRL_PIN, Bit_RESET)
#endif
	
#ifdef THT_L075
#define CH_HeatOn	  		GPIO_WriteBit(POWER_CTRL_PORT, POWER_CTRL_PIN, Bit_SET)
#define CH_HeatOff			GPIO_WriteBit(POWER_CTRL_PORT, POWER_CTRL_PIN, Bit_RESET)
#endif
	
void contrl_gpio_init(void);


#ifdef __cplusplus
}
#endif

#endif


