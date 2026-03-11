#include "bsp_all.h"
#include "counter_ms_us.h"
#include "debug.h"

TIM_TimeBaseInitType TIM_TimeBaseStructure;
OCInitType TIM_OCInitStructure;
uint16_t PrescalerValue = 0;

void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void TIM_Configuration(void);

/**
 * @brief  Configures tim2 clocks.
 */
void debug_us_counter_timer_init(void)
{
	  NVIC_InitType NVIC_InitStructure;
	    /* TIM2 clock enable */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM2, ENABLE);

    /* Compute the prescaler value */
//    PrescalerValue = 17; //(uint16_t) (SystemCoreClock / 12000000) - 1;

    /* Time base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);    
    TIM_TimeBaseStructure.Period    = DEBUG_UNIT_BASE;
    TIM_TimeBaseStructure.Prescaler = 35;
    TIM_TimeBaseStructure.ClkDiv    = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_DOWN;

    TIM_InitTimeBase(TIM2, &TIM_TimeBaseStructure);

//    /* Prescaler configuration */
//    TIM_ConfigPrescaler(TIM2, PrescalerValue, TIM_PSC_RELOAD_MODE_IMMEDIATE);

    /* TIM2 enable update irq */
    TIM_ConfigInt(TIM2, TIM_INT_UPDATE, ENABLE);

    /* TIM2 enable counter */
    TIM_Enable(TIM2, ENABLE);
			
		/* Enable the TIM2 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_DEBUG_US_TIM;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_DEBUG_US_TIM;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

uint16_t get_debug_us_counter(void)
{
		return TIM2->AR;
}


void TIM2_IRQHandler(void)
{
		uint32_t cnt;

    if (TIM_GetIntStatus(TIM2, TIM_INT_UPDATE) != RESET)
    {
        TIM_ClrIntPendingBit(TIM2, TIM_INT_UPDATE);
				debug_unit_counter++;
				if(debug_unit_counter == DEBUG_UNIT_COUNTER_MAX){
					debug_unit_counter = 0;
				}
				if(ch_bsp_debug.timer_us_flag == 1){
					if( (debug_unit_counter % (1000000/DEBUG_UNIT_BASE) )== 0){
						cnt = debug_unit_counter / (1000000/DEBUG_UNIT_BASE);
						sprintf(debug_string_out_0, "%ds\n\r", cnt);
						log_info(debug_string_out_0);
					}
				}
		}
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param file pointer to the source file name
 * @param line assert_param error line source number
 */
void assert_failed(const uint8_t* expr, const uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    while (1)
    {
    }
}

#endif

/**
 * @}
 */

/**
 * @}
 */
