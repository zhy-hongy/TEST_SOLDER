#include "uart_232.h"

volatile uint8_t U2_232_RxBuf[U2_232_RxBufSize] = {};
volatile uint8_t U2_232_RecvLen = 0, U2_232_RecvFlag = 0; 

void u2_rs232_init(void)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;
	  DMA_InitType DMA_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO | RS232_PERIPH_GPIO, ENABLE);

    RCC_EnableAPB1PeriphClk(RS232_PERIPH, ENABLE);


    GPIO_InitStructure.Pin        = RS232_TX_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
		GPIO_InitStructure.GPIO_Alternate = RS232_TX_IO_ALTTERNATE;
    GPIO_InitPeripheral(RS232_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.Pin            = RS232_RX_PIN;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Alternate = RS232_RX_IO_ALTTERNATE;
    GPIO_InitPeripheral(RS232_GPIO, &GPIO_InitStructure);   
		
		
		/* UART TX DMA channel configuration ----------------------------------------------*/
    DMA_DeInit(RS232_TX_DMA_CH);
		DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr     = (uint32_t)USART2_DAT_Base;
    DMA_InitStructure.MemAddr        = (uint32_t)&U2_232_RxBuf[0];
    DMA_InitStructure.Direction      = DMA_DIR_PERIPH_DST;
    DMA_InitStructure.BufSize        = sizeof(U2_232_RxBuf);
    DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode   = DMA_MODE_NORMAL;
    DMA_InitStructure.Priority       = DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
    DMA_Init(RS232_RX_DMA_CH, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_USART2_TX, DMA, RS232_TX_DMA_CH, ENABLE);

    /* Enable DMA channel3 */
    DMA_EnableChannel(RS232_RX_DMA_CH, ENABLE);
		
		/* UART RX DMA channel2 configuration ----------------------------------------------*/
    DMA_DeInit(RS232_RX_DMA_CH);
		DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr     = (uint32_t)USART2_DAT_Base;
    DMA_InitStructure.MemAddr        = (uint32_t)&U2_232_RxBuf[0];
    DMA_InitStructure.Direction      = DMA_DIR_PERIPH_SRC;
    DMA_InitStructure.BufSize        = sizeof(U2_232_RxBuf);
    DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode   = DMA_MODE_NORMAL;
    DMA_InitStructure.Priority       = DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
    DMA_Init(RS232_RX_DMA_CH, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_USART2_RX, DMA, RS232_RX_DMA_CH, ENABLE);

    /* Enable DMA channel3 */
    DMA_EnableChannel(RS232_RX_DMA_CH, ENABLE);
		

    USART_InitStructure.BaudRate            = 115200;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;

    // init uart
    USART_Init(RS232_USART2, &USART_InitStructure);
    
		USART_EnableDMA(RS232_USART2, USART_DMAREQ_TX | USART_DMAREQ_RX, ENABLE);

    /* Enable the USART Receive Interrupt */
    USART_ConfigInt(RS232_USART2, USART_INT_IDLEF, ENABLE);
		
    // enable uart
    USART_Enable(RS232_USART2, ENABLE);

    /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = NVIC_RS232_UART_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_RS232_UART;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_RS232_UART;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel                   = NVIC_RS232_RX_DMA_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEM_PRIO_RS232_DMA;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIO_RS232_DMA;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}




/**
 * @brief  This function handles USART2 global interrupt request.
 */
void USART2_IRQHandler(void)
{
    if (USART_GetIntStatus(RS232_USART2, USART_INT_IDLEF) != RESET)
    {
			  //清除空闲中断标志
			  (void)RS232_USART2->STS;
        (void)RS232_USART2->DAT;
			  
			  /*重新设置dma通道的传输数量*/
			  DMA_EnableChannel(RS232_TX_DMA_CH, DISABLE);  
			  DMA_SetCurrDataCounter(RS232_TX_DMA_CH,sizeof(U2_232_RxBuf));
			  DMA_EnableChannel(RS232_TX_DMA_CH, ENABLE);
			
        U2_232_RecvFlag = 1;
			  U2_232_RecvLen = sizeof(U2_232_RxBuf) / sizeof(U2_232_RxBuf[0]);
			
//			  log_info("\n%s",U2_232_RxBuf);
//			  memset(U1_RxBuffer, 0, sizeof(U1_RxBuffer));
    }
}

