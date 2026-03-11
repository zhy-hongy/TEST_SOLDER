#include "beep.h"

uint16_t beep_once_cnt, beep_gap_cnt;
volatile uint8_t beep_once_ntf;

uint8_t beep_times;
uint8_t long_short_cnt;


// ���������duty:333; rang 0-665
//void beep_init(uint16_t CCR1_Val)
//{
//	uint16_t PrescalerValue = 0;

//    /* GPIO Configuration */
//    GPIO_InitType GPIO_InitStructure;
//	
//	TIM_TimeBaseInitType TIM_TimeBaseStructure;
//    OCInitType TIM_OCInitStructure;
//    /* BEEP_TIME clock enable */
//    RCC_EnableAPB1PeriphClk(BEEP_TIME_CLK, ENABLE);

//    GPIO_InitStruct(&GPIO_InitStructure);
//    /* GPIOA Configuration:BEEP_TIME Channel1, alternate function push-pull */
//    GPIO_InitStructure.Pin        = BEEP_PIN;
//    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
//    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_TIM2;
//    GPIO_InitPeripheral(BEEP_PORT, &GPIO_InitStructure);

//    /* Compute the prescaler value */
//    PrescalerValue = (uint16_t)(SystemCoreClock / 1000000) - 1;
//    /* Time base configuration */
//    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);    
//    TIM_TimeBaseStructure.Period    = 665;
//    TIM_TimeBaseStructure.Prescaler = PrescalerValue;
//    TIM_TimeBaseStructure.ClkDiv    = 0;
//    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

//    TIM_InitTimeBase(BEEP_TIME, &TIM_TimeBaseStructure);

//    /* PWM1 Mode configuration: Channel1 */
//    TIM_InitOcStruct(&TIM_OCInitStructure);    
//    TIM_OCInitStructure.OcMode      = TIM_OCMODE_PWM1;
//    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
//    TIM_OCInitStructure.Pulse       = CCR1_Val;
//    TIM_OCInitStructure.OcPolarity  = TIM_OC_POLARITY_HIGH;

//    TIM_InitOc1(BEEP_TIME, &TIM_OCInitStructure);

//    TIM_ConfigOc1Preload(BEEP_TIME, TIM_OC_PRE_LOAD_ENABLE);

//    TIM_ConfigArPreload(BEEP_TIME, ENABLE);

//    /* BEEP_TIME enable counter */
//    TIM_Enable(BEEP_TIME, ENABLE);

//}

void beep_gpio_init(void)	
{
    /* GPIO Configuration */
    GPIO_InitType GPIO_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);
    /* GPIOA Configuration:BEEP_TIME Channel1, alternate function push-pull */
    GPIO_InitStructure.Pin        		= BEEP_PIN;
    GPIO_InitStructure.GPIO_Mode  		= GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Current 	= GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Alternate 	= GPIO_AF5_TIM2;
    GPIO_InitPeripheral(BEEP_PORT, &GPIO_InitStructure);
}

void beep_init(void)
{
	beep_gpio_init();
	bsp_tim_pwm_output_init(BEEP_TIME, BEEP_TIME_RCC, 1, 1000, 36);
	BEEP_OFF;
	
	beep_once_cnt 	= 0;
	beep_gap_cnt 	= 0;
	beep_once_ntf 	= 1;
}

void beep_pwm_set(uint16_t duty)
{
	if(duty == 0)
	{
		BEEP_OFF;
	}
	else
	{
		TIM_SetCmp1(BEEP_TIME, 100);
	}
	
}

void beep_poll(void)
{
	if(beep_once_ntf > 0){
		beep_times = (beep_once_ntf&0x0F);		
		if((beep_once_ntf&0x10) > 0){
			long_short_cnt = 40;
		}else{
			long_short_cnt = 15;
		}
		beep_once_cnt = 0;
		
		beep_once_ntf = 0;
	}
	
	if(beep_times > 0){
		BEEP_ON;
		beep_once_cnt++;
		if(beep_once_cnt > long_short_cnt){
			BEEP_OFF;
			beep_times--;
			beep_once_cnt = 0;
		}
	}
}
