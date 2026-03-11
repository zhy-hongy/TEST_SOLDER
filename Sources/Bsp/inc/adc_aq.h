#ifndef __ADC_AQ_H__
#define __ADC_AQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"
#include "debug.h"

#define BASE_1_2_ADC_CH1_PIN			GPIO_PIN_0
#define BASE_1_2_ADC_PORT			    GPIOA
#define BASE_1_2_ADC_CH			      ADC_CH_1_PA0

#define ILEAK_ADC_CH2_PIN			    GPIO_PIN_1
#define ILEAK_ADC_PORT			      GPIOA
#define ILEAK_ADC_CH			        ADC_CH_2_PA1

#define SOLDER_VOUT_CH3_PIN		    GPIO_PIN_2
#define SOLDER_VOUT_PORT		  		GPIOA
#define SOLDER_VOUT_CH            ADC_CH_3_PA2

#define SOLDER_CURRENT_CH4_PIN		GPIO_PIN_3
#define SOLDER_CURRENT_PORT		    GPIOA
#define SOLDER_CURRENT_CH         ADC_CH_4_PA3

#define CH_TEMP_ADC_CH5_PIN				GPIO_PIN_4
#define CH_TEMP_PORT			        GPIOA
#define CH_TEMP_CH                ADC_CH_5_PA4

#define CH_TEMP_ADC_CH6_PIN_COM		GPIO_PIN_5
#define CH_TEMP_PORT_COM			    GPIOA
#define CH_TEMP_COM_CH            ADC_CH_6_PA5

// 电压电流读取，定时读取
#define VOL_CUR_ADC_DRV_TIM				TIM1
#define RCC_VOL_CUR_ADC_DRV_TIM		RCC_APB2_PERIPH_TIM1
#define VOL_CUR_ADC_DRV_TIM_PRD		1000	//	500us
#define VOL_CUR_ADC_DRV_TIM_OC		500 //  OC output

#define VOL_CUR_DATA_LENGTH				30
#define VOL_CUR_ONCE_NUM				4

#define CH_TEMP_DMA_LENGTH        10

//#define SOLDER_VOUT_ADC						ADC1
//#define SOLDER_VOL_CUR_IRQHandler ADC1_2_IRQHandler

#define DEFAULT_BASE_VOLTAGE			1.65f
#define DEFAULT_AVCC_VOLTAGE			3.30f
//#define cal_default_voltage(ADC_VAL_IN)		(ADC_VAL_IN * 2.50f / 4096)


#define CORRECT_TAB_LENGTH				10

typedef struct
{
    uint16_t base_1_2_adc_val;
		uint16_t ileak_adc_val;
		uint16_t solder_vout_adc_val;
		uint16_t solder_current_adc_val;
		uint16_t ch_temp_adc_val;
		uint16_t ch_temp_com_adc_val;
}adc_aq_fltr_val_t;

typedef struct{
		uint8_t  dma_working_flag;
		uint8_t  length;
		uint16_t adc_val[CH_TEMP_DMA_LENGTH];		
		double   adc_average, adc_sum;				
		// double   adc_average_2, adc_sum_2;	
} adc_regular_dma_t;

typedef struct{
		uint8_t  irq_working_flag;
		uint8_t  length;
		uint8_t  cnts;
		uint16_t adc_val[VOL_CUR_ONCE_NUM][VOL_CUR_DATA_LENGTH];
} adc_inject_irq_t;

typedef struct{
		uint8_t  index;
	  uint8_t  length;
		double   mv_tab[CORRECT_TAB_LENGTH];
		double 	 Vo_tab[CORRECT_TAB_LENGTH];

		double   k, b;
} opa_correct_data;

extern volatile adc_regular_dma_t adc_regular_dma_state;
extern volatile adc_inject_irq_t  adc_inject_irq_state;

extern volatile double opa_base_voltage;

extern volatile opa_correct_data temp_210_opa_correct;
extern volatile opa_correct_data temp_245_opa_correct;
extern volatile opa_correct_data ileak_opa_correct;
extern volatile opa_correct_data voltage_opa_correct;
extern volatile opa_correct_data current_opa_correct;

void adc_gpio_init(void);
void adc_temperature_init(void);
void adc_tim_drv_init(void);
void adc_monitor_init(void);
void adc_enable_init(void);

void adc_monitor_base12_config(void);
void adc_monitor_temp_config(void);
void temp_base_gather_start(void);
void temp_base_gather_stop(void);

void vol_cur_gather_start(void);
void vol_cur_gather_stop(void);

void opa_base_vol_correct(void);

void opa_init(volatile opa_correct_data *correct_data, double k, double b);

void opa_correct_cal(volatile opa_correct_data *correct_data);

double adc_ret_vol(double adc_val);
double opa_vol_cal(volatile opa_correct_data *correct_data, double vol);

#ifdef __cplusplus
}
#endif

#endif


