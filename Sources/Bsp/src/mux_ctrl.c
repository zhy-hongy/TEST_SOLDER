#include "mux_ctrl.h"


void mux_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin          = SIGMUX_SW1_PIN;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Pull    = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(SIGMUX_SW1_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin          = SIGMUX_SW2_PIN;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Pull    = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(SIGMUX_SW1_PORT, &GPIO_InitStructure);
}

void mux_select(mux_mode_t mode)
{
	switch(mode){
		case mux_115_temp: 
			MUX1_RESET;
			MUX2_RESET;
			break;
		
		case mux_210_temp: 
			MUX1_RESET;
			MUX2_RESET;
			break;
		
		case mux_245_temp:
			MUX1_SET;
			MUX2_RESET;
			break;
		
		case mux_reset:
			MUX1_SET;
			MUX2_SET;
			break;			

		default:
			MUX1_RESET;
			MUX2_RESET;
			break;
	}
}

