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
 * @file log.h
 * @author Nations 
 * @version V1.2.2
 *
 * @copyright Copyright (c) 2022, Nations Technologies Inc. All rights reserved.
 */
#ifndef __LOG_H__
#define __LOG_H__

#ifndef LOG_ENABLE
#define LOG_ENABLE 1
#endif

#if LOG_ENABLE

#include "n32g43x.h"
#include <stdio.h>
#include "queue.h"

#define LOG_NONE    0
#define LOG_ERROR   10
#define LOG_WARNING 20
#define LOG_INFO    30
#define LOG_DEBUG   40

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

#if LOG_LEVEL >= LOG_INFO
#define log_info(...) printf(__VA_ARGS__)
#else
#define log_info(...)
#endif

#if LOG_LEVEL >= LOG_ERROR
#define log_error(...) printf(__VA_ARGS__)
#else
#define log_error(...)
#endif

#if LOG_LEVEL >= LOG_WARNING
#define log_warning(...) printf(__VA_ARGS__)
#else
#define log_warning(...)
#endif

#if LOG_LEVEL >= LOG_DEBUG
#define log_debug(...) printf(__VA_ARGS__)
#else
#define log_debug(...)
#endif

#define U1_RxBufferSize 32 //(countof(RxBuffer) - 1)
extern volatile uint8_t U1_RxBuffer[U1_RxBufferSize] ;
extern volatile uint8_t U1_RecvLen , U1_RecvFlag ; 

/*队列*/
#define PACKET_DATA_LEN_MIN    10                 //最小包长度
#define PACKET_DATA_LEN_MAX    32                 //最大包长度
#define MAX_BUF_SIZE           (32*10)
extern uint8_t g_rcvDataBuf[MAX_BUF_SIZE];
extern circle_queue_t g_rcvQueue;

void print_log(const char *format, uint32_t value);

void log_queue_init(void);

#else /* !LOG_ENABLE */

#define log_info(...)
#define log_warning(...)
#define log_error(...)
#define log_debug(...)
#define log_init()

#endif

#define log_func() log_debug("call %s\r\n", __FUNCTION__)

#endif /* __LOG_H__ */
