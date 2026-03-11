#include "led.h"

#ifdef __cplusplus
extern "C" {
#endif

static volatile uint8_t work_led_state;

void work_led_init(void)
{
    GPIO_InitType GPIO_InitStructure;
		
	GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin          = WORK_LED_PIN;
    GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Pull    = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(WORK_LED_PORT, &GPIO_InitStructure);
	
	WORK_LED_OFF;
}

void work_led_blink(void)
{
	if(work_led_state == 0){
		WORK_LED_ON;
		work_led_state = 1;
	}else{
		WORK_LED_OFF;
		work_led_state = 0;
	}
}

#ifdef __cplusplus
}
#endif