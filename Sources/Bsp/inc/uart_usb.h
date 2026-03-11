#ifndef __UART_USB_H__
#define __UART_USB_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "n32g43x.h"
#include "bsp_all.h"
#include "queue.h"
	
#define USB_LINK_USART1                             USART1
#define USB_LINK_UART_PERIPH                        RCC_APB2_PERIPH_USART1
#define USB_LINK_GPIO                               GPIOB
#define USB_LINK_PERIPH_GPIO                        RCC_APB2_PERIPH_GPIOB
#define USB_LINK_TX_PIN                             GPIO_PIN_6
#define USB_LINK_RX_PIN                             GPIO_PIN_7

#define USART1_DAT_Base                        			(USART1_BASE + 0x04)
#define U1_RXDAT_DMA_CH                        			DMA_CH2
#define NVIC_PREEM_PRIO_LOG										 			7
#define NVIC_SUB_PRIO_USB_LOG								   			1
	

extern volatile circle_queue_t usb_tx_queue;
extern volatile uint8_t usb_txdma_transing_flag;

extern volatile circle_queue_t usb_rx_queue;
	
void uart_usb_init(void);
void uart_usb_queue_init(void);
void uart_usb_send_to_txbuffer(char * str, uint32_t data_len);
void uart_usb_send_dma(void);
	
#ifdef __cplusplus
}
#endif

#endif
