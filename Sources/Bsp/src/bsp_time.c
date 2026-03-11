#include "bsp_all.h"
#include "debug.h"

uint32_t index_t6_cnt = 0;

void bsp_tim_interrupt_init(TIM_Module *TIMx,uint32_t tim_clk ,uint16_t arr,uint16_t psc)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;

	  /* TIM5 clock enable */
    RCC_EnableAPB1PeriphClk(tim_clk, ENABLE);

    TIM_DeInit(TIMx);
	
	  /* Time base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);    
		TIM_TimeBaseStructure.Prescaler = psc;  
		TIM_TimeBaseStructure.Period    = arr;    
    TIM_TimeBaseStructure.ClkDiv    = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIMx, &TIM_TimeBaseStructure);

    TIM_ConfigPrescaler(TIMx, psc, TIM_PSC_RELOAD_MODE_IMMEDIATE);

    /* TIMx enable update irq */
    TIM_ConfigInt(TIMx, TIM_INT_UPDATE, ENABLE);

    /* TIMx enable counter */
    TIM_Enable(TIMx, ENABLE);
}

#ifdef THT_L075

void bsp_tim_pwm_output_gpio_init(void)
{
    GPIO_InitType GPIO_InitStructure;
		
		GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin          	= POWER_CTRL_PWM_GPIO_PIN;
    GPIO_InitStructure.GPIO_Current 	= GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Pull    	= GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode    	= GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Alternate = GPIO_AF2_TIM4;
    GPIO_InitPeripheral(POWER_CTRL_PWM_GPIO_PORT, &GPIO_InitStructure);
}

#endif

void bsp_tim_pwm_output_init(TIM_Module *TIMx, uint32_t tim_clk, uint8_t oc_ch, uint16_t arr, uint16_t psc)
{
	
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	OCInitType TIM_OCInitStructure;
	
	uint16_t CCR_Val       	= arr * 40 / 100;
	
	RCC_EnableAPB1PeriphClk(tim_clk, ENABLE);
	
    TIM_DeInit(TIMx);
    /* Time base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);    
    TIM_TimeBaseStructure.Period    = arr;
    TIM_TimeBaseStructure.Prescaler = psc;
    TIM_TimeBaseStructure.ClkDiv    = 0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIMx, &TIM_TimeBaseStructure);

    TIM_InitOcStruct(&TIM_OCInitStructure);    
    TIM_OCInitStructure.OcMode      = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse       = CCR_Val;
    TIM_OCInitStructure.OcPolarity  = TIM_OC_POLARITY_HIGH;

    /* PWM1 Mode configuration: Channel1 */
	if( (oc_ch & 0x01) > 0 ){
		TIM_InitOc1(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc1Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
    
	/* PWM1 Mode configuration: Channel2 */
	if( (oc_ch & 0x02) > 0 ){
		TIM_InitOc2(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc2Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	
    /* PWM1 Mode configuration: Channel3 */
	if( (oc_ch & 0x04) > 0 ){
		TIM_InitOc3(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc3Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	
    /* PWM1 Mode configuration: Channel4 */
	if( (oc_ch & 0x08) > 0 ){
		TIM_InitOc4(TIMx, &TIM_OCInitStructure);
		TIM_ConfigOc4Preload(TIMx, TIM_OC_PRE_LOAD_ENABLE);
	}
	
    TIM_ConfigArPreload(TIMx, ENABLE);

    /* TIMx enable counter */
    TIM_Enable(TIMx, ENABLE);
	
}

void bsp_tim_once(TIM_Module* timx, uint16_t time)
{
	timx->AR = time;
    TIM_Enable(timx, ENABLE);
}

void bsp_tim_stop(TIM_Module* timx)
{
	TIM_Enable(timx, DISABLE);
//		TIM_GenerateEvent(timx, TIM_EVT_SRC_UPDATE);
}

#ifdef THT_L075
void bsp_ctrl_start_tim_nvic_init(void)
{
	NVIC_InitType NVIC_InitStructure;
	    /* Enable the TIM2 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = CH_CTRL_START_TIM_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_CTRL_START_TIM;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_CTRL_START_TIM;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void bsp_ctrl_pwm_tim_nvic_init(void)
{
	NVIC_InitType NVIC_InitStructure;
	    /* Enable the TIM2 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = CH_CTRL_START_TIM_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_CTRL_START_TIM;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_CTRL_START_TIM;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}
#endif

void bsp_ctrl_tim_nvic_init(void)
{
	NVIC_InitType NVIC_InitStructure;
	    /* Enable the TIM2 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = CH_CTRL_TIM_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_CTRL_TIM;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_CTRL_TIM;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void bsp_ctrl_tim_init(void)
{
	#ifdef THT_L075
	bsp_tim_pwm_output_gpio_init();
	bsp_tim_pwm_output_init(POWER_CTRL_PWM_TIM, POWER_CTRL_PWM_TIM_RCC, 0x04, 100, 50);
	
	// 设置为100Hz触发，触发后计算是否需要温度采集和控制计算
	bsp_tim_interrupt_init(CH_CTRL_START_TIM_FREQ_100xHz, CH_CTRL_START_TIM_RCC, 10000, 35);
	
//	bsp_tim_interrupt_init(CH_CTRL_TIMER, CH_CTRL_TIM_RCC, 1000, 35);//初始化定时器
//	
	bsp_ctrl_start_tim_nvic_init();
	
	
	#endif
	
	bsp_tim_interrupt_init(CH_CTRL_TIMER, CH_CTRL_TIM_RCC, 1000, 35);//初始化定时器
	bsp_ctrl_tim_nvic_init();
	bsp_ctrl_tim_stop();
}

#ifdef THT_L075
void bsp_ctrl_start_tim_start(uint16_t us)
{
	TIM_SetAutoReload(CH_CTRL_START_TIM_FREQ_100xHz, us);
	TIM_Enable(CH_CTRL_START_TIM_FREQ_100xHz, ENABLE);
}
#endif

void bsp_ctrl_tim_start_once(uint16_t us)
{
	TIM_SetAutoReload(CH_CTRL_TIMER, us);
	TIM_Enable(CH_CTRL_TIMER, ENABLE);
}

void bsp_ctrl_tim_stop(void)
{
	TIM_Enable(CH_CTRL_TIMER, DISABLE);
	TIM_SetCnt(CH_CTRL_TIMER, 0);
}


