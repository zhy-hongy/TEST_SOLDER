/**************************************************************************************/
#include "stdint.h"
#include "macro.h"
#include "sm4.h"
#include "string.h"

#ifdef USE_SM4

const static uint8_t SboxTable[256] = 
{
		0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,		//00
		0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,		//01
		0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,		//02
		0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,		//03
		0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,		//04
		0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,		//05
		0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,		//06
		0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,		//07
		0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,		//08
		0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,		//09
		0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,		//0A
		0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,		//0B
		0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,		//0C
		0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,		//0D
		0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,		//0E
		0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48			//0F
};

// System parameter 
const static uint32_t FK[4] = {0xa3b1bac6,0x56aa3350,0x677d9197,0xb27022dc};

// fixed parameter 
const static uint32_t CK[32] =
{
		0x00070e15,0x1c232a31,0x383f464d,0x545b6269,
		0x70777e85,0x8c939aa1,0xa8afb6bd,0xc4cbd2d9,
		0xe0e7eef5,0xfc030a11,0x181f262d,0x343b4249,
		0x50575e65,0x6c737a81,0x888f969d,0xa4abb2b9,
		0xc0c7ced5,0xdce3eaf1,0xf8ff060d,0x141b2229,
		0x30373e45,0x4c535a61,0x686f767d,0x848b9299,
		0xa0a7aeb5,0xbcc3cad1,0xd8dfe6ed,0xf4fb0209,
		0x10171e25,0x2c333a41,0x484f565d,0x646b7279
};

#define ROTL(n,X) ( ( (X) << (n) ) | ( (X) >> ( 32 - (n) ) ) )

#define	SMS4_Sbox(x) ( (uint32_t)(SboxTable[(uint8_t)x])					\
							| ( (uint32_t)(SboxTable[(uint8_t)(x>>8)])<<8 )			\
							| ( (uint32_t)(SboxTable[(uint8_t)(x>>16)])<<16 )		\
							| ( (uint32_t)(SboxTable[(uint8_t)(x>>24)])<<24 )	) 

#define LTX(x)				( x ^ ROTL( 8, x ) ^ ROTL( 16, x ) )


#define	SMS4_LTX(z,x) {	x = SMS4_Sbox(x);					\
								z ^= x ^ ROTL( 24, x );						\
								x = ROTL( 2, x ); 								\
								z ^= LTX(x);								}

#define	SMS4_LTRK(z,x)		{	x = SMS4_Sbox(x);						\
								z ^= x ^ ROTL( 13, x ) ^ ROTL( 23, x );		}


#ifdef BIG_ENDIAN
#define READ_UINT4(p) (*(uint32_t *)(p))
#else
#define READ_UINT4(p) ( (((uint32_t) (p)[0]) << 24)			\
										| (((uint32_t) (p)[1]) << 16)				\
										| (((uint32_t) (p)[2]) << 8)					\
										| ((uint32_t) (p)[3]) )
#endif


#define READ_UINT1(p) ( (((uint32_t) (p)[0]) << 24)			\
										| (((uint32_t) (p)[1]) << 16)				\
										| (((uint32_t) (p)[2]) << 8)					\
										| ((uint32_t) (p)[3]) )



#define WRITE_UINT4(p, i, j) {											\
											j = i;												\
											(p)[0] = (uint8_t)((j >> 24));			\
											(p)[1] = (uint8_t)((j >> 16));			\
											(p)[2] = (uint8_t)((j >> 8));			\
											(p)[3] = (uint8_t)(j);							\
										} 

void SM4_Encrypt(uint8_t Result[16], uint8_t Message[16], uint8_t MK[16])
{
		uint8_t i;
		uint32_t *pResult,middle, RK[4], Result_temp[4], temp;
		pResult = Result_temp;

		//RK初始化
		for(i=0; i<4; i++)
		{
				temp = READ_UINT1(MK+4*i);
				RK[i] = temp^FK[i];
				pResult[i] = READ_UINT1(Message+4*i);
		}

		i = 0;
		while(i < 32)
		{
				middle = RK[(i+1)&3] ^ RK[(i+2)&3] ^ RK[(i+3)&3] ^ CK[i];
				SMS4_LTRK(RK[i&3], middle);
				
				middle = pResult[(i+1)&3] ^ pResult[(i+2)&3] ^ pResult[(i+3)&3] ^ RK[i&3];
				SMS4_LTX(pResult[i&3], middle);

				i++;
		}

		for(i=0; i<2; i++)
		{
				middle = pResult[i];
				pResult[i] = pResult[3-i];
				pResult[3-i] = middle;
		}

		i = 0;
		while(i < 4)
		{
				WRITE_UINT4((uint8_t *)(&Result[4*i]), pResult[i], middle);
				i++;
		}
}

void SM4_Decrypt(uint8_t Result[16], uint8_t Message[16], uint8_t MK[16])
{
		uint8_t i;
		uint32_t *pResult,middle, RK[32], Result_temp[4], temp;

		pResult = Result_temp;
		//RK初始化
		for(i=0; i<4; i++)
		{
				temp = READ_UINT1(MK+4*i);
				RK[i] = temp^FK[i];
				pResult[i] = READ_UINT1(Message+4*i);
		}

		for(i=0; i<32; i++)
		{
				if(i<4)
				{
						middle = RK[(i+1)&3] ^ RK[(i+2)&3] ^ RK[(i+3)&3] ^ CK[i];
						SMS4_LTRK(RK[i&3], middle);
				}
				else
				{
						middle = RK[i-3] ^ RK[i-2] ^ RK[i-1] ^ CK[i];
						RK[i] = RK[i-4];
						SMS4_LTRK(RK[i], middle);
				}
		}

		i = 0;
		while(i<32)
		{	
				middle = pResult[(i+1)&3] ^ pResult[(i+2)&3] ^ pResult[(i+3)&3] ^ RK[31-i];
				SMS4_LTX(pResult[i&3], middle);

				i++;
		}

		for(i=0; i<2; i++)
		{
				middle = pResult[i];
				pResult[i] = pResult[3-i];
				pResult[3-i] = middle;
		}

		i = 0;
	  while(i<4)
		{
				WRITE_UINT4((uint8_t *)(&Result[4*i]), pResult[i], middle);
				i++;
		}
}


void LongGetByte(uint8_t *buf,uint32_t val)
{  
		 buf[3] = (uint8_t) val;
		 buf[2] = (uint8_t)(val>>8);	 
		 buf[1] = (uint8_t)(val>>16);
		 buf[0] = (uint8_t)(val>>24);    
}

uint32_t ByteGetLong(uint8_t*buf) // get Long for 4 bytes
{
		uint32_t temp;
		uint32_t result;

		result = 0;
		temp = 0;
		temp = (uint32_t)buf[3];
		result = temp;
		temp = (uint32_t)buf[2];
		result += temp<<8;
		temp = (uint32_t)buf[1];
		result += temp<<16;
		temp = (uint32_t)buf[0];
		result += temp<<24; 
		return result;
}


int PKCS7Padding(uint8_t *InOutBuf, uint32_t *Inoutlen ,uint8_t Blocksize)
{
		uint8_t  paddata = 0x00;
		uint16_t  padoffset = 0x00;
		uint16_t i;
		uint32_t lp;

		lp = *Inoutlen;
		paddata = Blocksize-(lp%Blocksize);
		padoffset =(lp/Blocksize)*Blocksize + (lp%Blocksize);  
		for(i=0; i<paddata; i++)
				InOutBuf[padoffset+i] = paddata;  
		*Inoutlen += paddata;
		return ((lp/Blocksize)+1)*Blocksize;
}

/*********************************************************************************************************************
LK_SM4Encrypt : SM4加密接口函数,支持MODE_ECB和MODE_CBC模式，数据不足8字节按我司默认数据补齐规则补齐（参看开发手册）
*Mode：				Mode=0时MODE_ECB模式, Mode=1时为MODE_CBC模式
*pIv：				MODE_ECB模式时填写NULL, MODE_CBC模式是为8字节初始向量
*key_len:			密钥长度
*pKey：				密钥值
*indata_len: 	明文长度
*pIn：				明文数据
*outdata_len：加密后的密文数据长度
*pOut：				加密后的密文数据
***********************************************************************************************************************/
uint8_t LK_SM4Encrypt(uint8_t Mode, uint8_t *pIv, uint8_t key_len, uint8_t* pKey, uint8_t indata_len, uint8_t* pIn, uint8_t* outdata_len, uint8_t* pOut)
{
		uint8_t pad_num;
	  uint8_t n;
	  uint8_t *tempIv;
		unsigned short i;
		unsigned short blk_num;
	
		if((indata_len == 0) ||  (key_len != KEY_LEN_SM4))
				return FAIL;

		if((indata_len%SM4_BLOCK_SIZE) != 0x00) {
				pad_num = SM4_BLOCK_SIZE - (indata_len%SM4_BLOCK_SIZE);
				if(pad_num != 0x00)
						pIn[indata_len+1] = 0x80;
				for(i=1; i<pad_num; i++)
						pIn[indata_len+1+i] = 0x00;
				indata_len += pad_num;
		}

		blk_num = indata_len / SM4_BLOCK_SIZE;

		if((Mode == MODE_CBC) && (pIv == NULL))
				return FAIL;
		else if((Mode != MODE_ECB) && (Mode != MODE_CBC))
				return FAIL;	
		
		*outdata_len = indata_len;
		
		if(Mode == MODE_CBC)
			tempIv = pIv;
		
		if (pOut != pIn)
		{
				memcpy(pOut, pIn, indata_len);
		}
		
		for(i=0; i<blk_num; i++)
		{
				if( Mode == MODE_CBC)
				{
						for(n=0; n<SM4_BLOCK_SIZE; n++)
								pOut[n] ^= tempIv[n];
				}
				SM4_Encrypt(pOut, pOut, pKey);
				tempIv = pOut;
				pOut += SM4_BLOCK_SIZE;
		}
		return SUCCESS;
}

/*********************************************************************************************************************
LK_SM4Decrypt: SM4解密接口函数,支持MODE_ECB和MODE_CBC模式

*Mode：				Mode=0时MODE_ECB模式, Mode=1时为MODE_CBC模式
*pIv：					MODE_ECB模式时填写NULL, MODE_CBC模式是为8字节初始向量
*key_len:			密钥长度
*pKey：				密钥值
*indata_len: 	密文长度
*pIn：				密文数据
*outdata_len：解密后的明文数据长度
*pOut：				解密后的明文数据

***********************************************************************************************************************/
uint8_t LK_SM4Decrypt(uint8_t Mode, uint8_t *pIv, uint8_t key_len, uint8_t* pKey, uint8_t indata_len, uint8_t* pIn, uint8_t* poutdata_len, uint8_t* pOut)
{
		uint8_t *tempIv;
		unsigned short i, n;
		unsigned short blk_num;


		if((indata_len == 0) || ((indata_len%SM4_BLOCK_SIZE) != 0x00) || (key_len != KEY_LEN_SM4))
				return FAIL;	

		blk_num = indata_len/SM4_BLOCK_SIZE;

		if(( Mode == MODE_CBC) && (pIv == NULL))
				return FAIL;	
		else if((Mode != MODE_CBC) && (Mode != MODE_ECB))
				return FAIL;	

		*poutdata_len = indata_len;
	
		if (pOut != pIn)
		{
				memcpy(pOut, pIn, indata_len);
		}

		pOut += indata_len - SM4_BLOCK_SIZE; //最后一块
		for (i=0; i<blk_num; i++)
		{
				SM4_Decrypt(pOut,pOut,pKey); //解密最后一块数据
				if(Mode == MODE_CBC)
				{
						if(i == (blk_num-1))	// 最后一块数据
						{
								for(n=0; n<SM4_BLOCK_SIZE; n++)
									pOut[n] ^= pIv[n];
						}
						else
						{
							  tempIv = pOut - SM4_BLOCK_SIZE;
								for(n=0; n<SM4_BLOCK_SIZE; n++)
									pOut[n] ^=  tempIv[n];
						}
				}
				pOut -= SM4_BLOCK_SIZE;
		}
		return SUCCESS;
}

#endif
