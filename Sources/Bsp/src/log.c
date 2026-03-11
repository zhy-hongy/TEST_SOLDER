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
#include "log.h"
#include <string.h>
#include "queue.h"

#if LOG_ENABLE

#include "n32g43x.h"
#include "n32g43x_gpio.h"
#include "n32g43x_usart.h"
#include "n32g43x_rcc.h"

#define LOG_USARTx                             USART1
#define LOG_PERIPH                             RCC_APB2_PERIPH_USART1
#define LOG_GPIO                               GPIOB
#define LOG_PERIPH_GPIO                        RCC_APB2_PERIPH_GPIOB
#define LOG_TX_PIN                             GPIO_PIN_6
#define LOG_RX_PIN                             GPIO_PIN_7
#define USART1_DAT_Base                        (USART1_BASE + 0x04)
#define U1_RXDAT_DMA_CH                        DMA_CH2
#define NVIC_PREEM_PRIO_LOG										 7
#define NVIC_SUB_PRIO_USB_LOG								   1

/*队列*/
uint8_t g_rcvDataBuf[MAX_BUF_SIZE];
circle_queue_t g_rcvQueue;


#define countof(a) (sizeof(a) / sizeof(*(a)))

volatile uint8_t U1_RxBuffer[U1_RxBufferSize] = {};
volatile uint8_t U1_RecvLen = 0, U1_RecvFlag = 0; 

void log_init(void)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;
	  DMA_InitType DMA_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO | LOG_PERIPH_GPIO, ENABLE);

    RCC_EnableAPB2PeriphClk(LOG_PERIPH, ENABLE);


    GPIO_InitStructure.Pin        = LOG_TX_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
		GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_USART1;
    GPIO_InitPeripheral(LOG_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.Pin            = LOG_RX_PIN;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_USART1;
    GPIO_InitPeripheral(LOG_GPIO, &GPIO_InitStructure);   
		
		/* DMA channel2 configuration ----------------------------------------------*/
    DMA_DeInit(DMA_CH2);
		DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr     = (uint32_t)USART1_DAT_Base;
    DMA_InitStructure.MemAddr        = (uint32_t)&U1_RxBuffer[0];
    DMA_InitStructure.Direction      = DMA_DIR_PERIPH_SRC;
    DMA_InitStructure.BufSize        = sizeof(U1_RxBuffer);
    DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode   = DMA_MODE_NORMAL;
    DMA_InitStructure.Priority       = DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
    DMA_Init(DMA_CH2, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_USART1_RX, DMA, DMA_CH2, ENABLE);

    /* Enable DMA channel2 */
    DMA_EnableChannel(DMA_CH2, ENABLE);
		

    USART_InitStructure.BaudRate            = 115200;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;

    // init uart
    USART_Init(LOG_USARTx, &USART_InitStructure);
    
		USART_EnableDMA(LOG_USARTx, USART_DMAREQ_RX, ENABLE);

    /* Enable the USART Receive Interrupt */
    USART_ConfigInt(LOG_USARTx, USART_INT_IDLEF, ENABLE);
		
    // enable uart
    USART_Enable(LOG_USARTx, ENABLE);

    /* Enable the USARTz Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_LOG;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_USB_LOG;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static int is_lr_sent = 0;

int fputc(int ch, FILE* f)
{
    if (ch == '\r')
    {
        is_lr_sent = 1;
    }
    else if (ch == '\n')
    {
        if (!is_lr_sent)
        {
            USART_SendData(LOG_USARTx, (uint8_t)'\r');
            /* Loop until the end of transmission */
            while (USART_GetFlagStatus(LOG_USARTx, USART_FLAG_TXC) == RESET)
            {
            }
        }
        is_lr_sent = 0;
    }
    else
    {
        is_lr_sent = 0;
    }
    USART_SendData(LOG_USARTx, (uint8_t)ch);
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(LOG_USARTx, USART_FLAG_TXC) == RESET)
    {
    }
    return ch;
}

void print_log(const char *format, uint32_t value) 
{
    __disable_irq(); // 关闭全局中断
    log_info(format, value);
    __enable_irq(); // 打开全局中断
}

void log_queue_init(void)
{
	  log_init();
    queue_init(&g_rcvQueue, g_rcvDataBuf, MAX_BUF_SIZE);
}

/**
 * @brief  This function handles USART1 global interrupt request.
 */
void USART1_IRQHandler(void)
{
    if (USART_GetIntStatus(LOG_USARTx, USART_INT_IDLEF) != RESET)
    {
			  //清除空闲中断标志
			  (void)LOG_USARTx->STS;
        (void)LOG_USARTx->DAT;
			
			  U1_RecvFlag = 1;
			  U1_RecvLen = sizeof(U1_RxBuffer) / sizeof(U1_RxBuffer[0]);
			  
			  /*将接收到的一组数据入队*/
			  queue_push_array(&g_rcvQueue,(uint8_t *)U1_RxBuffer,U1_RecvLen);
			  
			  /*重新设置dma通道的传输数量，开启下一次传输*/
			  DMA_EnableChannel(DMA_CH2, DISABLE);  
			  DMA_SetCurrDataCounter(DMA_CH2,sizeof(U1_RxBuffer));
			  DMA_EnableChannel(DMA_CH2, ENABLE);			
			
//        log_info("\n%s-%d",g_rcvDataBuf,QueueCount(&g_rcvQueue));			
			
			  memset((void *)U1_RxBuffer, 0, sizeof(U1_RxBuffer));
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

#endif // LOG_ENABLE
