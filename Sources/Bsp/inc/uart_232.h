#ifndef __uart_232_H__
#define __uart_232_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"
#include "bsp_all.h"

#define RS232_USART2                             USART2
#define RS232_PERIPH                             RCC_APB1_PERIPH_USART2
#define RS232_GPIO                               GPIOB
	
#define RS232_PERIPH_GPIO                        RCC_APB2_PERIPH_GPIOB
#define RS232_TX_PIN                             GPIO_PIN_4
#define RS232_RX_PIN                             GPIO_PIN_5
#define RS232_TX_IO_ALTTERNATE                   GPIO_AF4_USART2
#define RS232_RX_IO_ALTTERNATE                   GPIO_AF6_USART2
#define USART2_DAT_Base                          (USART2_BASE + 0x04)

#define U2_232_IRQN                              USART2_IRQn


#define U2_232_RxBufSize 32 //(countof(RxBuffer) - 1)

extern volatile uint8_t U2_232_RecvLen , U2_232_RecvFlag ;
extern volatile uint8_t U2_232_RxBuf[U2_232_RxBufSize];

void u2_rs232_init(void);

#ifdef __cplusplus
}
#endif

#endif


