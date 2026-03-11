#ifndef __TOOL_IO_H__
#define __TOOL_IO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"
#include "debug.h"

#ifdef THT_M120
//#define TOOL_IN_PORT                           	GPIOB
//#define TOOL_IN_PIN                            	GPIO_PIN_13
#define TOOL_IN_PORT                           	GPIOA
#define TOOL_IN_PIN                            	GPIO_PIN_8
#endif
	
#ifdef THT_L075
#define TOOL_IN_PORT                           	GPIOA
#define TOOL_IN_PIN                            	GPIO_PIN_8
#endif
//#define TOOL_IN_EXTI_LINE                      	EXTI_LINE13
//#define TOOL_IN_PORT_SOURCE                    	GPIOB_PORT_SOURCE
//#define TOOL_IN_PIN_SOURCE                     	GPIO_PIN_SOURCE13
//#define TOOL_IN_IRQn                           	EXTI15_10_IRQn
//#define SOLDER_CH_TOOL_IN_IRQHandler        		EXTI15_10_IRQHandler
//#define NVIC_PREEM_PRIO_CH_TOOL_IN_EXTI	       	4
//#define NVIC_SUB_PRIO_CH_TOOL_IN_EXTI		       	0

#define TOOL_TYPE_1_DET_PORT                    GPIOA
#define TOOL_TYPE_1_DET_PIN                     GPIO_PIN_11
//#define TOOL_TYPE_1_DET_EXTI_LINE               EXTI_LINE11
//#define TOOL_TYPE_1_DET_PORT_SOURCE             GPIOA_PORT_SOURCE
//#define TOOL_TYPE_1_DET_SOURCE                  GPIO_PIN_SOURCE11
//#define TOOL_TYPE_1_DET_IRQn                    EXTI15_10_IRQn
//#define NVIC_PREEM_PRIO_TOOL_TYPE_1_DET_EXTI		5
//#define NVIC_SUB_PRIO_TOOL_TYPE_1_DET_EXTI			0

#ifdef THT_M120
//#define TOOL_TYPE_2_DET_PORT                    GPIOB
//#define TOOL_TYPE_2_DET_PIN                     GPIO_PIN_12
#define TOOL_TYPE_2_DET_PORT                    GPIOA
#define TOOL_TYPE_2_DET_PIN                     GPIO_PIN_9
#endif

#ifdef THT_L075
#define TOOL_TYPE_2_DET_PORT                    GPIOA
#define TOOL_TYPE_2_DET_PIN                     GPIO_PIN_9
#endif
//#define TOOL_TYPE_2_DET_EXTI_LINE               EXTI_LINE12
//#define TOOL_TYPE_2_DET_PORT_SOURCE             GPIOB_PORT_SOURCE
//#define TOOL_TYPE_2_DET_SOURCE                  GPIO_PIN_SOURCE12
//#define TOOL_TYPE_2_DET_IRQn                    EXTI15_10_IRQn
//#define NVIC_PREEM_PRIO_TOOL_TYPE_2_DET_EXTI		5
//#define NVIC_SUB_PRIO_TOOL_TYPE_2_DET_EXTI			1

#define SLEEP_HANDLE_PORT        	GPIOB
#define SLEEP_HANDLE_PIN         	GPIO_PIN_2

#define SLEEP_TEST_OUT_PORT			GPIOB
#define SLEEP_TEST_OUT_PIN			GPIO_PIN_10

#define SLEEP_TOOL_PORT        		GPIOB
#define SLEEP_TOOL_PIN         		GPIO_PIN_11




void tool_det_exti_init(void);

#ifdef __cplusplus
}
#endif

#endif


