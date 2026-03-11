#include "tool_io.h"

void tool_in_det_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	GPIO_InitStruct(&GPIO_InitStructure);
	
	GPIO_InitStructure.Pin        	= TOOL_IN_PIN;
	GPIO_InitStructure.GPIO_Pull  	= GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode  	= GPIO_Mode_Input;

	GPIO_InitPeripheral(TOOL_IN_PORT, &GPIO_InitStructure);
}

void tool_type_det_init(void)
{
	GPIO_InitType GPIO_InitStructure;
	
	GPIO_InitStruct(&GPIO_InitStructure);
	
	GPIO_InitStructure.Pin        = TOOL_TYPE_1_DET_PIN;
	GPIO_InitStructure.GPIO_Pull  = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Input;
	
	GPIO_InitPeripheral(TOOL_TYPE_1_DET_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin        = TOOL_TYPE_2_DET_PIN;
	GPIO_InitStructure.GPIO_Pull  = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Input;
	
	GPIO_InitPeripheral(TOOL_TYPE_2_DET_PORT, &GPIO_InitStructure);
}

void tool_sleep_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	GPIO_InitStruct(&GPIO_InitStructure);
	
	GPIO_InitStructure.Pin        = SLEEP_HANDLE_PIN;
	GPIO_InitStructure.GPIO_Pull  = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Input;
//	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitPeripheral(SLEEP_HANDLE_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin        = SLEEP_TOOL_PIN;
	GPIO_InitStructure.GPIO_Pull  = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Input;
//	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitPeripheral(SLEEP_TOOL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin        = SLEEP_TEST_OUT_PIN;
	GPIO_InitStructure.GPIO_Pull  = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitPeripheral(SLEEP_TEST_OUT_PORT, &GPIO_InitStructure);


	GPIO_WriteBit(SLEEP_TEST_OUT_PORT, SLEEP_TEST_OUT_PIN, Bit_SET);
}



void tool_det_exti_init(void)
{
    tool_in_det_init();
    tool_type_det_init();
    tool_sleep_init();
}


