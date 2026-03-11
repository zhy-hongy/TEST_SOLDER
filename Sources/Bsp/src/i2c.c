#include "i2c.h"

uint8_t IICWriteAddr = 0x50;
uint8_t IICReadAddr = 0x51;
uint8_t nackFlag;

/**************************************************

***************************************************/
void i2c_init(void)
{

	GPIO_InitType GPIO_InitStructure;

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin = IIC_REST_GPIO_PIN;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(IIC_REST_GPIO_PORT, &GPIO_InitStructure);
	IIC_REST_H;

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin = IIC_SDA_GPIO_PIN | IIC_SCL_GPIO_PIN;
	GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(IIC_SDA_GPIO_PORT, &GPIO_InitStructure);
	IIC_SDA_H;
	IIC_SCL_H;
}

void IIC_Reset(void)
{
	IIC_REST_L;
	delay_ms(4);
	IIC_REST_H;
	delay_ms(20);
}

void stop(void)
{
	IIC_SCL_L;

	delay_us(1);
	IIC_SDA_L;
	delay_us(3);
	IIC_SCL_H;
	delay_us(3);
	IIC_SDA_H;
}

void start(void)
{
	IIC_SDA_H;
	IIC_SCL_H;
	delay_us(2);
	IIC_SDA_L;
	delay_us(4);
}

void ack(void)
{
	delay_us(4);
	IIC_SCL_H;
	delay_us(4);
	IIC_SCL_L;
	IIC_SDA_H;
}

void n_ack(void)
{
	IIC_SDA_H;
	delay_us(3);
	IIC_SCL_H;
	delay_us(4);
	IIC_SCL_L;
}

void checkack(void)
{
	IIC_SCL_L;
	IIC_SDA_H;
	delay_us(4);
	IIC_SCL_H;
	nackFlag = 0;
	delay_us(4);
	if (SdaState)
		nackFlag = 0x01;
}

void sendbyte(uint8_t *Sdata)
{
	uint8_t temp, i;

	temp = *Sdata;
	for (i = 0; i < 8; i++)
	{
		IIC_SCL_L;
		delay_us(2);
		if ((temp & 0x80) == 0x80)
			IIC_SDA_H;
		else
			IIC_SDA_L;
		delay_us(1);
		IIC_SCL_H;
		delay_us(2);
		temp = temp << 1;
	}
}

uint8_t recbyte(uint8_t *Rdata)
{
	char i;
	uint8_t k = 0;

	*Rdata = 0x00;
	for (i = 0; i < 8; i++)
	{
		IIC_SCL_L;
		delay_us(4);
		IIC_SCL_H;
		while (!SclState)
		{
			if (k > 30)
				return SCL_TimeOut;
			k++;
			delay_us(1);
		}
		delay_us(4);
		*Rdata = *Rdata << 1;
		if (SdaState)
			*Rdata |= 0x01;
	}
	IIC_SCL_L;
	IIC_SDA_L;

	return SUCCESS;
}

/*****************??n??????? ******************************/
uint8_t IIC_ReadData(TIIC_RData_Struct *Rdata) //(uint8_t *RecBuf, uint16_t *rlen ,uint16_t *sw)
{
	uint8_t res = 0;
	uint16_t len = 0;
	int i = 0, time = 1500000;
	memset(Rdata->len, 0x00, 0x02);
	memset(Rdata->sw, 0x00, 0x02);
	// *sw = 0x00;
	//*rlen = 0x00;

	start();
	sendbyte(&IICReadAddr);
	checkack();
	while (1 == nackFlag)
	{
		stop();
		if (i == time)
			return IICRadAddrNACK;

		delay_us(1);
		start();
		sendbyte(&IICReadAddr);
		checkack();
		IIC_SDA_H;
		i++;
	}

	IIC_SDA_H;

	// 1.读两字节长度
	for (i = 0; i < 2; i++)
	{
		res = recbyte(Rdata->len + i);
		if (res != SUCCESS)
		{
			stop();
			return res;
		}
		ack();
	}

	len = Rdata->len[0] * 0x100 + Rdata->len[1];
	// 2.读内容段长度
	for (i = 0; i < (len - 2); i++)
	{
		res = recbyte(Rdata->data + i);
		if (res != SUCCESS)
		{
			stop();
			return res;
		}
		ack();
	}
	// 3.读SW状态码
	for (i = 0; i < 0x02; i++)
	{
		res = recbyte(Rdata->sw + i);
		if (res != SUCCESS)
		{
			stop();
			return res;
		}
		if (i == 0x01)
		{
			n_ack();
			stop();
		}
		else
			ack();
	}
	return SUCCESS;
}

uint8_t IIC_WriteData(uint8_t *Sendbuf, uint16_t len, uint8_t Packet)
{
	uint16_t i;
	if (len == 0x00)
		return SUCCESS;
	if ((Packet == FirstPack) || (Packet == OnePack))
	{
		start();
		sendbyte(&IICWriteAddr);
		checkack();
		if (1 == nackFlag)
		{
			stop();
			return IICNACK;
		}
	}

	for (i = 0; i < len; i++)
	{
		delay_us(1);
		sendbyte(Sendbuf + i);
		checkack();
		if (1 == nackFlag)
		{
			stop();
			return IICNACK;
		}
	}
	if ((Packet == EndPack) || (Packet == OnePack))
		stop();
	return SUCCESS;
}

uint8_t IIC_SendApdu(uint16_t Sendlen, uint8_t *Sendbuf, TIIC_RData_Struct *Rdata)
{
	uint8_t state = 0xFF;
	if (Sendlen == 0x00)
		return SUCCESS;
	state = IIC_WriteData(Sendbuf, Sendlen, OnePack);
	if (state != SUCCESS)
		return state;
	delay_us(2);
	state = IIC_ReadData(Rdata);
	if (state != SUCCESS)
		return state;
	return SUCCESS;
}
