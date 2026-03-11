#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_all.h"

void bsp_init(void)
{	
	bsp_rcc_init();
	bsp_nvic_init();
	
	systick_init();
	
	delay_ms(10);
	
	uart_usb_init();
	uart_usb_queue_init();
	
	i2c_init();

//	debug_us_counter_timer_init();
	
	work_led_init();
	key_gpio_init();
	lcd_init();
	
	tool_det_exti_init();
	contrl_gpio_init();	
	
	mux_gpio_init();
	
	adc_gpio_init();
	adc_temperature_init();
	adc_tim_drv_init();
	adc_monitor_init();
	adc_enable_init();
	
	bsp_ctrl_tim_init();
	
	zero_signal_init();	
	
	 // 初始化看门狗
    // IWDG_Init();

	 // 根据复位原因设置LED状态
    // if (IWDG_CheckResetFlag())
    // {
    //     log_info("IWDG reset detected!");
    // }

	beep_init();
		
	opa_init(&temp_210_opa_correct, 470.0f/27.0f*36.0f, 0);
	opa_init(&temp_245_opa_correct, 470.0f/68.0f*36.0f, 0);
	/**
	 * 更新原理图后没有偏置电压
	 */
	#if 1		//老版子
	opa_init(&ileak_opa_correct, 	25,			DEFAULT_BASE_VOLTAGE);
	opa_init(&voltage_opa_correct, 	1.0f,		DEFAULT_BASE_VOLTAGE);
	opa_init(&current_opa_correct, 	(470.0f/27.0f), 		DEFAULT_BASE_VOLTAGE);
	#else
	opa_init(&ileak_opa_correct, 	25,			DEFAULT_BASE_VOLTAGE);
	opa_init(&voltage_opa_correct, 	1.0f,		0);
	opa_init(&current_opa_correct, 	(470.0f/27.0f), 		0);
	#endif
}

void bsp_rcc_init(void)
{
	/* System clocks configuration ---------------------------------------------*/
	rcc_hsi_cfg();

	/* Enable peripheral clocks ------------------------------------------------*/
	/* Enable DMA clocks */
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);

	/* Enable ADC clocks */
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
	
	/* Enable RS232 clocks */
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM2 | RS232_PERIPH, ENABLE);	
	

	/* Enable GPIOA GPIOB AFIO clocks */
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | 
							RCC_APB2_PERIPH_GPIOB |
							RCC_APB2_PERIPH_GPIOC |
							RCC_APB2_PERIPH_GPIOD |
							RCC_APB2_PERIPH_AFIO, 
							ENABLE);
	
		/* Enable USB LINK UART clock*/
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_TIM1 | USB_LINK_UART_PERIPH, ENABLE);
}

void bsp_nvic_init(void)
{
	/* 3 bit for pre-emption priority, 1 bits for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	
}


void bsp_periph_init(void)
{

}

void bsp_dma_init(void)
{

}

#ifdef __cplusplus
}
#endif


