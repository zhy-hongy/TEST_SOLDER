#include "bsp_all.h"
#include "debug.h"

void zero_signal_init(void)
{
	GPIO_InitType GPIO_InitStructure;
	EXTI_InitType EXTI_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	
	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin        = ZERO_CHECK_PIN;
	GPIO_InitStructure.GPIO_Pull  = GPIO_Pull_Up;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Input;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitPeripheral(ZERO_CHECK_PORT, &GPIO_InitStructure);
	
	/*Configure key EXTI Line to key input Pin*/
	GPIO_ConfigEXTILine(ZERO_CHECK_PORT_SOURCE, ZERO_CHECK_PIN_SOURCE);
	
  /*Configure EXTI line*/
	EXTI_InitStructure.EXTI_Line    = ZERO_CHECK_LINE;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitPeripheral(&EXTI_InitStructure);

	/*Set input interrupt priority*/
	NVIC_InitStructure.NVIC_IRQChannel                   = ZERO_CHECK_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_ZERO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_ZERO;
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


uint8_t zero_check_status(void)
{
	return GPIO_ReadInputDataBit(ZERO_CHECK_PORT, ZERO_CHECK_PIN);
}


