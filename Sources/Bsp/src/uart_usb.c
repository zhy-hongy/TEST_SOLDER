/*****************************************************************************
 * Copyright (c) 2022, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file log.c
 * @author Nations 
 * @version V1.2.2
 *
 * @copyright Copyright (c) 2022, Nations Technologies Inc. All rights reserved.
 */
#include <string.h>
#include "queue.h"

#include "bsp_all.h"
#include "debug.h"

/*队列*/
volatile uint8_t usb_tx_tmp_buffer[USB_UART_TXBUFFER_SIZE];
volatile uint8_t usb_tx_buffer[QUEUE_TX_SIZE_MAX];
volatile circle_queue_t usb_tx_queue;
volatile uint8_t usb_txdma_transing_flag = 0;

volatile uint8_t usb_rx_flag = 0, usb_rx_length = 0;
volatile uint8_t usb_rx_tmp_buffer[USB_UART_RXBUFFER_SIZE];
volatile uint8_t usb_rx_buffer[QUEUE_RX_SIZE_MAX];
volatile circle_queue_t usb_rx_queue;

void uart_usb_init(void)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;
	 DMA_InitType DMA_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);

    /* Configure USARTy Tx as alternate function push-pull */
    GPIO_InitStructure.Pin            = GPIO_PIN_6;   
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_USART1;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    /* Configure USARTy Rx as alternate function push-pull and pull-up */
    GPIO_InitStructure.Pin            = GPIO_PIN_7;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_USART1;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure); 

		
		/* DMA channel 1 configuration ----------------------------------------------*/
    DMA_DeInit(USB_UART_TX_DMA_CH);
	DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr     	= (uint32_t)USART1_DAT_Base;
    DMA_InitStructure.MemAddr        	= (uint32_t)usb_tx_tmp_buffer;
    DMA_InitStructure.Direction      	= DMA_DIR_PERIPH_DST;
    DMA_InitStructure.BufSize        	= sizeof(usb_tx_tmp_buffer);
    DMA_InitStructure.PeriphInc      	= DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  	= DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize 	= DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize    	= DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode   	= DMA_MODE_NORMAL;
    DMA_InitStructure.Priority       	= DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem        	= DMA_M2M_DISABLE;
    DMA_Init(USB_UART_TX_DMA_CH, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_USART1_TX, DMA, USB_UART_TX_DMA_CH, ENABLE);		
		
		DMA_ConfigInt(USB_UART_TX_DMA_CH, DMA_INT_TXC, ENABLE);
		
		/* DMA channel 2 configuration ----------------------------------------------*/
    DMA_DeInit(USB_UART_RX_DMA_CH);
		DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr     	= (uint32_t)USART1_DAT_Base;
    DMA_InitStructure.MemAddr        	= (uint32_t)usb_rx_tmp_buffer;
    DMA_InitStructure.Direction      	= DMA_DIR_PERIPH_SRC;
    DMA_InitStructure.BufSize        	= sizeof(usb_rx_tmp_buffer);
    DMA_InitStructure.PeriphInc      	= DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  	= DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize 	= DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize    	= DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode   	= DMA_MODE_NORMAL;
    DMA_InitStructure.Priority       	= DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem        	= DMA_M2M_DISABLE;
    DMA_Init(USB_UART_RX_DMA_CH, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_USART1_RX, DMA, USB_UART_RX_DMA_CH, ENABLE);

    /* Enable DMA channel 1 */
    DMA_EnableChannel(USB_UART_TX_DMA_CH, DISABLE);
    /* Enable DMA channel2 */
    DMA_EnableChannel(USB_UART_RX_DMA_CH, ENABLE);    

		/* Enable USART DMA */
		USART_EnableDMA(USB_LINK_USART1, USART_DMAREQ_TX | USART_DMAREQ_RX, ENABLE);
	

    USART_InitStructure.BaudRate            = 115200;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;

    /* Init USART */
    USART_Init(USB_LINK_USART1, &USART_InitStructure);

    /* Enable the USART Receive Interrupt */
    USART_ConfigInt(USB_LINK_USART1, USART_INT_IDLEF, ENABLE);

    /* Enable Uart */
    USART_Enable(USB_LINK_USART1, ENABLE);

    /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = NVIC_USB_LINK_UART_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_USB_UART;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_USB_UART;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel                   = NVIC_USB_LINK_TX_DMA_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_USB_DMA;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_USB_DMA;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void uart_usb_queue_init(void)
{
		queue_init( (void*)&usb_tx_queue, (uint8_t*)usb_tx_buffer, sizeof(usb_tx_buffer), 1 );
		queue_init( (void*)&usb_rx_queue, (uint8_t*)usb_rx_buffer, sizeof(usb_rx_buffer), 1 );
}

void uart_usb_send_to_txbuffer(char * str, uint32_t data_len)
{
		__disable_irq();
		queue_push_array((void*)&usb_tx_queue, (uint8_t *)str, data_len);
		__enable_irq();

}

void uart_usb_send_dma(void)
{		
		uint32_t data_len_tmp = 0;

		if(usb_tx_queue.data_len > 0 && usb_txdma_transing_flag == 0){
			__disable_irq();
			
			DMA_EnableChannel(USB_UART_TX_DMA_CH, DISABLE);			
			usb_txdma_transing_flag = 1;
					
			if(usb_tx_queue.data_len <= USB_UART_TXBUFFER_SIZE){
				data_len_tmp = queue_pop_all((void*)&usb_tx_queue, (void*)usb_tx_tmp_buffer);
			}else{
				data_len_tmp = USB_UART_TXBUFFER_SIZE;
				queue_pop_array((void*)&usb_tx_queue, (void*)usb_tx_tmp_buffer, data_len_tmp);
			}
			
			
			DMA_SetCurrDataCounter(USB_UART_TX_DMA_CH, data_len_tmp);									
			DMA_EnableChannel(USB_UART_TX_DMA_CH, ENABLE);
			__enable_irq();
		}

}



/**
 * @brief  This function handles USART1 global interrupt request.
 */
void USART1_IRQHandler(void)
{
    if (USART_GetIntStatus(USB_LINK_USART1, USART_INT_IDLEF) != RESET)
    {
			__disable_irq();
			//清除空闲中断标志
			(void)USB_LINK_USART1->STS;
			(void)USB_LINK_USART1->DAT;
		
			DMA_EnableChannel(USB_UART_RX_DMA_CH, DISABLE);  							
		
			usb_rx_length = USB_UART_RXBUFFER_SIZE - DMA_GetCurrDataCounter(USB_UART_RX_DMA_CH);
		
			/*将接收到的一组数据入队*/
			queue_push_array((void*)&usb_rx_queue, (uint8_t *)usb_rx_tmp_buffer, usb_rx_length);
			
//			log_info("receive once\n");
//			log_info(usb_rx_queue.buffer);
			memset((void*)usb_rx_tmp_buffer, 0, usb_rx_length);
			/*重新设置dma通道的传输数量，开启下一次传输*/
			DMA_SetCurrDataCounter(USB_UART_RX_DMA_CH, USB_UART_RXBUFFER_SIZE);
			
			DMA_EnableChannel(USB_UART_RX_DMA_CH, ENABLE);			
			__enable_irq();
    }
}

void DMA_Channel1_IRQHandler(void)
{
		if (DMA_GetIntStatus(DMA_INT_TXC1, DMA) != RESET)
		{			
			DMA_EnableChannel(USB_UART_TX_DMA_CH, DISABLE); 			
			DMA_ClearFlag(DMA_INT_TXC1, DMA);
			usb_txdma_transing_flag = 0;			
		}
}




#ifdef USE_FULL_ASSERT

__WEAK void assert_failed(const uint8_t* expr, const uint8_t* file, uint32_t line)
{
    log_error("assertion failed: `%s` at %s:%d", expr, file, line);
    while (1)
    {
    }
}
#endif // USE_FULL_ASSERT

