#ifndef __ZERO_CHECK_H__
#define __ZERO_CHECK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"


#define ZERO_CHECK_PIN				GPIO_PIN_9
#define ZERO_CHECK_PORT				GPIOB
#define ZERO_CHECK_PORT_SOURCE		GPIOB_PORT_SOURCE
#define ZERO_CHECK_PIN_SOURCE		GPIO_PIN_SOURCE9
#define ZERO_CHECK_LINE   			EXTI_LINE9
#define ZERO_CHECK_IRQn				EXTI9_5_IRQn
#define ZERO_CHECK_IRQHandler		EXTI9_5_IRQHandler

#define Get_ZERO_Level				GPIO_ReadInputDataBit(ZERO_CHECK_PORT, ZERO_CHECK_PIN)

void zero_signal_init(void);
uint8_t zero_check_status(void);

#ifdef __cplusplus
}
#endif

#endif


