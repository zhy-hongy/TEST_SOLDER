#ifndef IIC_H
#define IIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"
#include "string.h"
#include "systick.h"

#define IIC_REST_GPIO_PIN		GPIO_PIN_13
#define IIC_REST_GPIO_PORT		GPIOC
	
#define IIC_SDA_GPIO_PIN		GPIO_PIN_14
#define IIC_SDA_GPIO_PORT		GPIOD

#define IIC_SCL_GPIO_PIN		GPIO_PIN_15
#define IIC_SCL_GPIO_PORT		GPIOD	

	
#define FirstPack       		0x00
#define ContinuePack			0x01
#define EndPack					0x02
#define OnePack					0x03

#define  IICNACK           		0xE2
#define  IICRadAddrNACK    		0xE3
#define  IICSWError        		0xE4
#define  IICRadLenError    		0xE5
#define  SCL_TimeOut       		0xE6
 
#define  IIC_SDA_H            GPIO_WriteBit(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, Bit_SET)
#define  IIC_SDA_L            GPIO_WriteBit(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, Bit_RESET)

#define  IIC_SCL_H            GPIO_WriteBit(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, Bit_SET)
#define  IIC_SCL_L            GPIO_WriteBit(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, Bit_RESET)

#define  IIC_REST_H           GPIO_WriteBit(IIC_REST_GPIO_PORT, IIC_REST_GPIO_PIN, Bit_SET)
#define  IIC_REST_L           GPIO_WriteBit(IIC_REST_GPIO_PORT, IIC_REST_GPIO_PIN, Bit_RESET)

#define  SdaState             (GPIOD->PID & (uint16_t)0x4000)
#define  SclState             (GPIOD->PID & (uint16_t)0x8000)

typedef struct {  //芯片返回数据格式为 len + data + sw
	uint8_t  	len[2];    //data+sw数据长度
	uint8_t 	sw[2];     //sw内容
	uint8_t 	*data;     //数据内容
} TIIC_RData_Struct;

extern void i2c_init(void);

extern uint8_t IIC_WriteData(uint8_t *Sendbuf, uint16_t len,uint8_t Packet);
extern uint8_t IIC_ReadData(TIIC_RData_Struct *Rdata);
extern void IIC_Reset(void);
extern void start(void);
extern void stop(void);
extern void checkack(void);
//extern uint8_t IIC_SendApdu(uint16_t Sendlen,uint8_t* Sendbuf,uint8_t* RecBuf,uint16_t* Relen);
extern uint8_t IIC_SendApdu(uint16_t Sendlen, uint8_t* Sendbuf,   TIIC_RData_Struct *Rdata);
extern void sendbyte(uint8_t *Sdata);
extern void ack(void);
extern void n_ack(void);
extern uint8_t recbyte(uint8_t *Rdata);



#ifdef __cplusplus
}
#endif

#endif
