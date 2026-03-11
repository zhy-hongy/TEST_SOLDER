#include "adc_aq.h"
#include "debug.h"
static uint16_t i;

volatile adc_regular_dma_t adc_regular_dma_state;
volatile adc_inject_irq_t adc_inject_irq_state;

volatile double opa_base_voltage;

volatile opa_correct_data temp_210_opa_correct;
volatile opa_correct_data temp_245_opa_correct;
volatile opa_correct_data ileak_opa_correct;
volatile opa_correct_data voltage_opa_correct;
volatile opa_correct_data current_opa_correct;

void adc_gpio_init(void)
{
	/* GPIO configuration ------------------------------------------------------*/
	GPIO_InitType GPIO_InitStructure;

	GPIO_InitStruct(&GPIO_InitStructure);
	/* Configure PA.01 (in1)as analog input -------------------------*/
	GPIO_InitStructure.Pin = BASE_1_2_ADC_CH1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = SOLDER_VOUT_CH3_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = SOLDER_CURRENT_CH4_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = ILEAK_ADC_CH2_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);


	GPIO_InitStructure.Pin = CH_TEMP_ADC_CH6_PIN_COM;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Pin       = 	CH_TEMP_ADC_CH5_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);


}

void adc_temperature_init(void)
{
	ADC_InitType ADC_InitStructure;
	DMA_InitType DMA_InitStructure;
	NVIC_InitType NVIC_InitStructure;

	/* RCC_ADCHCLK_DIV8*/
	ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV8);
	RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSI, RCC_ADC1MCLK_DIV8); // selsect HSI as RCC ADC1M CLK Source

	ADC_DeInit(ADC);
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.MultiChEn = ENABLE;
	ADC_InitStructure.ContinueConvEn = ENABLE;
	ADC_InitStructure.ExtTrigSelect = ADC_EXT_TRIGCONV_NONE;
	ADC_InitStructure.DatAlign = ADC_DAT_ALIGN_R;
	ADC_InitStructure.ChsNumber = 1;
	ADC_Init(ADC, &ADC_InitStructure);

	/* ADC regular channel configuration */
	//    ADC_ConfigRegularChannel(ADC, BASE_1_2_ADC_CH,   1, ADC_SAMP_TIME_71CYCLES5);
	ADC_ConfigRegularChannel(ADC, CH_TEMP_CH,        1, ADC_SAMP_TIME_71CYCLES5);


	/* Enable ADC DMA */
	ADC_EnableDMA(ADC, ENABLE);

	/* DMA channel1 configuration ----------------------------------------------*/
	DMA_DeInit(TEMP_ADC_DMA_CH);
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.PeriphAddr = (uint32_t)&ADC->DAT;
	DMA_InitStructure.MemAddr = (uint32_t)&adc_regular_dma_state.adc_val;
	DMA_InitStructure.Direction = DMA_DIR_PERIPH_SRC;
	DMA_InitStructure.BufSize = CH_TEMP_DMA_LENGTH;
	DMA_InitStructure.PeriphInc = DMA_PERIPH_INC_DISABLE;
	DMA_InitStructure.DMA_MemoryInc = DMA_MEM_INC_ENABLE;
	DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_HALFWORD;
	DMA_InitStructure.MemDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.CircularMode = DMA_MODE_CIRCULAR;
	DMA_InitStructure.Priority = DMA_PRIORITY_LOW;
	DMA_InitStructure.Mem2Mem = DMA_M2M_DISABLE;
	DMA_Init(TEMP_ADC_DMA_CH, &DMA_InitStructure);

	DMA_RequestRemap(DMA_REMAP_ADC1, DMA, TEMP_ADC_DMA_CH, ENABLE);
	/* Enable DMA channel */
	DMA_EnableChannel(TEMP_ADC_DMA_CH, DISABLE);

	/*开启传输完成中断*/
	DMA_ConfigInt(TEMP_ADC_DMA_CH, DMA_INT_TXC, ENABLE);

	/*配置dma通道一传输完成中断*/
	NVIC_InitStructure.NVIC_IRQChannel = TEMP_ADC_DMA_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_DMA_CH_ADC;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SUB_PRIO_DMA_CH_ADC;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Start ADC Software Conversion */
	//    ADC_EnableSoftwareStartConv(ADC, ENABLE);
	//		DMA_EnableChannel(TEMP_ADC_DMA_CH, ENABLE);
}
void adc_monitor_init(void)

{
	//	ADC_InitType ADC_InitStructure;
	NVIC_InitType NVIC_InitStructure;

	/* Set injected sequencer length */
	ADC_ConfigInjectedSequencerLength(ADC, 4);
	/* ADC1 injected channel Configuration */
	//		ADC_ConfigInjectedChannel(ADC, BASE_1_2_ADC_CH, 1, ADC_SAMP_TIME_71CYCLES5);
	/* ADC1 injected channel Configuration */
	ADC_ConfigInjectedChannel(ADC, ILEAK_ADC_CH, 1, ADC_SAMP_TIME_71CYCLES5);
	/* ADC1 injected channel Configuration */
	ADC_ConfigInjectedChannel(ADC, SOLDER_VOUT_CH, 2, ADC_SAMP_TIME_71CYCLES5);
	/* ADC1 injected channel Configuration */
	ADC_ConfigInjectedChannel(ADC, SOLDER_CURRENT_CH, 3, ADC_SAMP_TIME_71CYCLES5);

	ADC_ConfigInjectedChannel(ADC, CH_TEMP_COM_CH, 4, ADC_SAMP_TIME_71CYCLES5);
	/* Enable automatic injected conversion start after regular one */
	ADC_EnableAutoInjectedConv(ADC, DISABLE);
	/* ADC1 injected external trigger configuration */
	ADC_ConfigExternalTrigInjectedConv(ADC, ADC_EXT_TRIG_INJ_CONV_T1_CC4);
	/* Enablethe ADCx injected channels conversion through external trigger*/
	ADC_EnableExternalTrigInjectedConv(ADC, ENABLE);
	/* Enable JEOC interrupt */
	ADC_ConfigInt(ADC, ADC_INT_JENDC, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_VOL_CUR_ADC;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SUB_PRIO_VOL_CUR_ADC;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void adc_tim_drv_init(void)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	OCInitType TIM_OCInitStructure;

	RCC_EnableAPB2PeriphClk(RCC_VOL_CUR_ADC_DRV_TIM, ENABLE);

	TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.Period = VOL_CUR_ADC_DRV_TIM_PRD;
	TIM_TimeBaseStructure.Prescaler = 71; // 分频后 1M 基准
	TIM_TimeBaseStructure.ClkDiv = TIM_CLK_DIV1;
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_UP;
	TIM_TimeBaseStructure.RepetCnt = 0;
	TIM_InitTimeBase(VOL_CUR_ADC_DRV_TIM, &TIM_TimeBaseStructure);
	/* Prescaler configuration */
	TIM_ConfigPrescaler(VOL_CUR_ADC_DRV_TIM, 71, TIM_PSC_RELOAD_MODE_IMMEDIATE);

	TIM_OCInitStructure.OcMode = TIM_OCMODE_PWM1;
	TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_ENABLE;
	TIM_OCInitStructure.Pulse = VOL_CUR_ADC_DRV_TIM_OC;
	TIM_OCInitStructure.OcPolarity = TIM_OC_POLARITY_LOW;

	TIM_InitOc4(VOL_CUR_ADC_DRV_TIM, &TIM_OCInitStructure); // 注入通道

	TIM_ConfigOc4Preload(VOL_CUR_ADC_DRV_TIM, TIM_OC_PRE_LOAD_ENABLE);

	TIM_ConfigArPreload(VOL_CUR_ADC_DRV_TIM, ENABLE);

	TIM_EnableCtrlPwmOutputs(VOL_CUR_ADC_DRV_TIM, ENABLE);

	TIM_Enable(VOL_CUR_ADC_DRV_TIM, ENABLE);
}

void adc_enable_init(void)
{
	/* Enable ADC */
	ADC_Enable(ADC, ENABLE);
	/* Check ADC Ready */
	while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET)
		;
	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC);
	/* Check the end of ADC1 calibration */
	while (ADC_GetCalibrationStatus(ADC))
		;
}

void adc_monitor_base12_config(void)
{
	ADC_EnableSoftwareStartConv(ADC, DISABLE);
	DMA_EnableChannel(TEMP_ADC_DMA_CH, DISABLE);

	ADC_ConfigRegularChannel(ADC, BASE_1_2_ADC_CH, 1, ADC_SAMP_TIME_71CYCLES5);
	//		DMA_SetCurrDataCounter(TEMP_ADC_DMA_CH, CH_TEMP_DMA_LENGTH);
}

void adc_monitor_temp_config(void)
{
	ADC_EnableSoftwareStartConv(ADC, DISABLE);
	DMA_EnableChannel(TEMP_ADC_DMA_CH, DISABLE);

	ADC_ConfigRegularChannel(ADC, CH_TEMP_CH, 1, ADC_SAMP_TIME_71CYCLES5);
	//		DMA_SetCurrDataCounter(TEMP_ADC_DMA_CH, CH_TEMP_DMA_LENGTH);
}

void temp_base_gather_start(void)
{
	adc_regular_dma_state.dma_working_flag = 1;
	//		DMA_SetCurrDataCounter(TEMP_ADC_DMA_CH, CH_TEMP_DMA_LENGTH);
	DMA_EnableChannel(TEMP_ADC_DMA_CH, ENABLE);
	ADC_EnableSoftwareStartConv(ADC, ENABLE);
}

void temp_base_gather_stop(void)
{
	adc_regular_dma_state.dma_working_flag = 0;
	ADC_EnableSoftwareStartConv(ADC, DISABLE);
	DMA_EnableChannel(TEMP_ADC_DMA_CH, DISABLE);
}

void vol_cur_gather_start(void)
{
	adc_inject_irq_state.cnts = 0;
	TIM_SetAutoReload(VOL_CUR_ADC_DRV_TIM, VOL_CUR_ADC_DRV_TIM_PRD);
	TIM_EnableCtrlPwmOutputs(VOL_CUR_ADC_DRV_TIM, ENABLE);
	TIM_Enable(TIM1, ENABLE);
}

void vol_cur_gather_stop(void)
{
	TIM_EnableCtrlPwmOutputs(VOL_CUR_ADC_DRV_TIM, DISABLE);
	TIM_Enable(TIM1, DISABLE);
}

void TEMP_ADC_DMA_IRQHandler(void)
{
	if (DMA_GetIntStatus(DMA_INT_TXC5, DMA) != RESET)
	{
		DMA_ClrIntPendingBit(DMA_INT_TXC5, DMA);

		temp_base_gather_stop();

		//				sprintf(debug_string_out_0, "regular dma length: %d\n", DMA_GetCurrDataCounter(TEMP_ADC_DMA_CH));
		//				log_info(debug_string_out_0);

		adc_regular_dma_state.adc_sum = 0;

		for (i = 0; i < CH_TEMP_DMA_LENGTH; i++)
		{
			adc_regular_dma_state.adc_sum = adc_regular_dma_state.adc_sum + adc_regular_dma_state.adc_val[i];
			// adc_regular_dma_state.adc_sum_2 = adc_regular_dma_state.adc_sum_2 + adc_regular_dma_state.adc_val[i][1];
		}
		adc_regular_dma_state.adc_average = adc_regular_dma_state.adc_sum / CH_TEMP_DMA_LENGTH;
		// adc_regular_dma_state.adc_average_2 = adc_regular_dma_state.adc_sum_2 / CH_TEMP_DMA_LENGTH;
		adc_regular_dma_state.dma_working_flag = 0;
		#if DEBUG_CMD_CTROL == 1 && DEBUG_ADC_REG_DMA == 1
		if(ch_bsp_debug.adc_regular_dma_flag == 1){
		sprintf(debug_string_int_1, ",sum: %5.1f, avr: %4.3f\n",
				adc_regular_dma_state.adc_sum, adc_regular_dma_state.adc_average);
		strcat(debug_string_int_0, debug_string_int_1);
		sprintf(debug_string_int_1, ",sum_2: %5.1f, avr_2: %4.3f\n",
				adc_regular_dma_state.adc_sum_2, adc_regular_dma_state.adc_average_2);
		strcat(debug_string_int_0, debug_string_int_1);
		log_info(debug_string_int_0);

		
		}
		#endif
	}
}

void opa_base_vol_correct(void)
{

	adc_monitor_base12_config();
	temp_base_gather_start();
	delay_ms(10);
	//		if(adc_regular_dma_state.dma_working_flag == 0){
	//				opa_base_voltage = adc_regular_dma_state.adc_average / 4096 * 3.30f;
	////				sprintf(debug_string_out_0, "1.2V base voltage = %4.3f %4.3f\n",
	////																opa_base_voltage, adc_regular_dma_state.adc_average);
	//				log_info(debug_string_out_0);
	//		}else{
	//				log_info("1.2V base voltage adc dma error\n");
	//		}

	adc_monitor_temp_config();
	temp_base_gather_start();
	delay_ms(10);
	//		if(adc_regular_dma_state.dma_working_flag == 0){
	//				opa_base_voltage = adc_regular_dma_state.adc_average / 4096 * 3.30f;
	//				sprintf(debug_string_out_0, "temp now voltage = %4.3f %4.3f\n",
	//																opa_base_voltage, adc_regular_dma_state.adc_average);
	//				log_info(debug_string_out_0);
	//		}else{
	//				log_info("temp voltage adc dma error\n");
	//		}
}

void opa_init(volatile opa_correct_data *correct_data, double k, double b)
{
	correct_data->k = k;
	correct_data->b = b;
}

double adc_ret_vol(double adc_val)
{
	return adc_val * DEFAULT_AVCC_VOLTAGE / 4096.0f;
}

double opa_vol_cal(volatile opa_correct_data *correct_data, double vol)
{
	return (vol - correct_data->b) / correct_data->k;
}

// Vo = k*mV + b
void opa_correct_cal(volatile opa_correct_data *correct_data)
{
	for (correct_data->index = 0; correct_data->index < correct_data->length; correct_data->index++)
	{
	}
}

void adc_correct()
{
}
