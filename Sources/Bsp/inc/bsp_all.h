#ifndef __BSP_ALL_H__
#define __BSP_ALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>	
#include <stdint.h>

#include "n32g43x.h"
	
#include "i2c.h"
#include "uart_usb.h"
#include "uart_232.h"
	
#include "adc_aq.h"
#include "led.h"
#include "systick.h"
#include "rcc.h"
#include "key.h"
#include "beep.h"
#include "mux_ctrl.h"
#include "tool_io.h"
#include "zero_check.h"
#include "heat_ctrl.h"
#include "bsp_time.h"
#include "beep.h"
#include "iwdg_config.h"
						 
#ifdef LED_SCREEN
#include "led_screen.h"
#endif

#ifdef LCD_SCREEN
#include "lcd.h"
#endif

// TIM1	驱动读取电压
// TIM2	驱动蜂鸣器响
// TIM3	调试用的TIM
// TIM4	适应直流驱动，代替过零信号
// TIM6 驱动控制阶段细节

#define USB_UART_TXBUFFER_SIZE			1024
#define USB_UART_RXBUFFER_SIZE			256

#define NVIC_PREEM_PRIO_DEBUG_US_TIM	1
#define NVIC_SUB_PRIO_DEBUG_US_TIM		1

// Systick interrupt
#define NVIC_PREEM_PRIO_SYSTICK   		0
#define NVIC_SUB_PRIO_SYSTICK     		1
	
// Zero interrupt
#define NVIC_PREEM_PRIO_ZERO			0
#define NVIC_SUB_PRIO_ZERO				0

// Controller TIMER interrupt
#define NVIC_PREEM_PRIO_CTRL_START_TIM	1
#define NVIC_SUB_PRIO_CTRL_START_TIM	0
#define NVIC_PREEM_PRIO_CTRL_TIM		1
#define NVIC_SUB_PRIO_CTRL_TIM			1	
	
// ADC DMA channel interrupt	
#define TEMP_ADC_DMA_IRQn				DMA_Channel5_IRQn
#define NVIC_PREEM_PRIO_DMA_CH_ADC   	2
#define NVIC_SUB_PRIO_DMA_CH_ADC     	0

// Voltage and Current monitor TIMER - ADC interrupt
#define NVIC_PREEM_PRIO_VOL_CUR_ADC		3
#define NVIC_SUB_PRIO_VOL_CUR_ADC			0

// ADC interrupt



// USART USB-LINK interrupt
#define NVIC_USB_LINK_UART_IRQ				USART1_IRQn
#define NVIC_PREEM_PRIO_USB_UART			6
#define NVIC_SUB_PRIO_USB_UART				0

#define NVIC_USB_LINK_TX_DMA_IRQ			DMA_Channel1_IRQn
#define NVIC_USB_LINK_RX_DMA_IRQ			DMA_Channel2_IRQn
#define NVIC_PREEM_PRIO_USB_DMA				6
#define NVIC_SUB_PRIO_USB_DMA				1


// USART RS232 interrupt
#define NVIC_RS232_UART_IRQ					USART2_IRQn
#define NVIC_PREEM_PRIO_RS232_UART			7
#define NVIC_SUB_PRIO_RS232_UART			0

#define NVIC_RS232_TX_DMA_IRQ				DMA_Channel3_IRQn
#define NVIC_RS232_RX_DMA_IRQ				DMA_Channel4_IRQn
#define NVIC_PREEM_PRIO_RS232_DMA			7
#define NVIC_SUB_PRIO_RS232_DMA				1


// DMA channel config
#define USB_UART_TX_DMA_CH					DMA_CH1
#define USB_UART_RX_DMA_CH					DMA_CH2
#define RS232_TX_DMA_CH						DMA_CH3
#define RS232_RX_DMA_CH               		DMA_CH4

#define TEMP_ADC_DMA_CH           			DMA_CH5
#define TEMP_ADC_DMA_IRQHandler    			DMA_Channel5_IRQHandler 


#define DEBUG_US_COUNTER_TIM				TIM3


void bsp_init(void);
void bsp_rcc_init(void);
void bsp_nvic_init(void);

#ifdef __cplusplus
}
#endif

#endif


