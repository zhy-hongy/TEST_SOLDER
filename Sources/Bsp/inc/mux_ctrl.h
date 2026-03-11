#ifndef __MUX_CTRL_H__
#define __MUX_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"

#define SIGMUX_SW1_CLK	  	RCC_APB2_PERIPH_GPIOC
#define SIGMUX_SW1_PORT	  	GPIOC
#define SIGMUX_SW1_PIN		GPIO_PIN_14
#define SIGMUX_SW2_CLK	  	RCC_APB2_PERIPH_GPIOC
#define SIGMUX_SW2_PORT	  	GPIOC
#define SIGMUX_SW2_PIN		GPIO_PIN_15

#define MUX1_SET			GPIO_WriteBit(SIGMUX_SW1_PORT, SIGMUX_SW1_PIN, Bit_SET)
#define MUX1_RESET			GPIO_WriteBit(SIGMUX_SW1_PORT, SIGMUX_SW1_PIN, Bit_RESET)
#define MUX2_SET			GPIO_WriteBit(SIGMUX_SW2_PORT, SIGMUX_SW2_PIN, Bit_SET)
#define MUX2_RESET			GPIO_WriteBit(SIGMUX_SW2_PORT, SIGMUX_SW2_PIN, Bit_RESET)

typedef enum{
	mux_115_temp, mux_210_temp, mux_245_temp, mux_reset
} mux_mode_t;

void mux_gpio_init(void);
void mux_select(mux_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif


