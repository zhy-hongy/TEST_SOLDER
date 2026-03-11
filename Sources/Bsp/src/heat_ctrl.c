#include "heat_ctrl.h"



void contrl_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	GPIO_InitStruct(&GPIO_InitStructure);
	
	GPIO_InitStructure.Pin          = POWER_CTRL_PIN;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Pull    = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(POWER_CTRL_PORT, &GPIO_InitStructure);

	GPIO_WriteBit(POWER_CTRL_PORT, POWER_CTRL_PIN, Bit_RESET);
}

