
#include <string.h>
#include "macro.h"
#include "des.h"

void movram(uint8_t* source,uint8_t* target,uint8_t length);
void doxor(uint8_t* sourceaddr,uint8_t* targetaddr,uint8_t length);
void setbit(uint8_t* dataddr,uint8_t pos,uint8_t b0);
uint8_t getbit(uint8_t* dataddr,uint8_t pos);
void selectbits(uint8_t* source,const uint8_t* table,uint8_t* target,uint8_t count);
void shlc(uint8_t* data_p);
void shrc(uint8_t* data_p);
void strans(uint8_t* index,uint8_t* target);

const uint8_t DesIp[] =
{ 
		58, 50, 42, 34, 26, 18, 10, 2,
		60, 52, 44, 36, 28, 20, 12, 4,
		62, 54, 46, 38, 30, 22, 14, 6,
		64, 56, 48, 40, 32, 24, 16, 8,
		57, 49, 41, 33, 25, 17, 9,   1,
		59, 51, 43, 35, 27, 19, 11, 3,
		61, 53, 45, 37, 29, 21, 13, 5,
		63, 55, 47, 39, 31, 23, 15, 7
};

const uint8_t DesIp_1[] =
{
		40, 8,  48, 16, 56, 24, 64, 32,
		39, 7,  47, 15, 55, 23, 63, 31,
		38, 6,  46, 14, 54, 22, 62, 30,
		37, 5,  45, 13, 53, 21, 61, 29,
		36, 4,  44, 12, 52, 20, 60, 28,
		35, 3,  43, 11, 51, 19, 59, 27,
		34, 2,  42, 10, 50, 18, 58, 26,
		33, 1,  41, 9,  49, 17, 57, 25
};

const uint8_t DesS[8][4][16] = 
{
		{
				{14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
				{0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
				{4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
				{15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}
		},
		
		{
				{15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
				{3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
				{0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
				{13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}
		},
		
		{
				{10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
				{13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
				{13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
				{1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}
		},
		
		{
				{7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
				{13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
				{10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
				{3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}
		},
		
		{
				{2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
				{14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
				{4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
				{11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}
		},
		
		{
				{12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
				{10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
				{9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
				{4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}
		},
		
		{
				{4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
				{13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
				{1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
				{6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}
		},
		
		{
				{13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
				{1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
				{7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
				{2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}
		}
};

const uint8_t DesE[] =
{
		32, 1, 2, 3, 4, 5,
		4, 5, 6, 7, 8, 9,
		8, 9, 10, 11, 12,13,
		12, 13, 14, 15, 16, 17,
		16, 17, 18, 19, 20, 21,
		20, 21, 22, 23, 24, 25,
		24, 25, 26, 27, 28, 29,
		28, 29, 30, 31, 32, 1
};

const uint8_t DesP[] = 
{
		16, 7, 20, 21,
		29, 12, 28, 17,
		1, 15, 23, 26,
		5, 18, 31, 10,
		2, 8, 24,  14,
		32, 27, 3, 9,
		19, 13, 30, 6,
		22, 11, 4, 25
};

const uint8_t DesPc_1[] =
{
		57, 49, 41, 33, 25, 17, 9,
		1, 58, 50, 42, 34, 26, 18,
		10, 2, 59, 51, 43, 35, 27,
		19, 11, 3, 60, 52, 44, 36,
		63, 55, 47, 39, 31, 23, 15,
		7, 62, 54, 46, 38, 30, 22,
		14, 6, 61, 53, 45, 37, 29,
		21, 13, 5, 28, 20, 12, 4
};

const uint8_t DesPc_2[] =
{
		14, 17, 11, 24, 1, 5,
		3, 28, 15, 6, 21, 10,
		23, 19, 12, 4, 26, 8,
		16, 7, 27, 20, 13, 2,
		41, 52, 31, 37, 47, 55,
		30, 40, 51, 45, 33, 48,
		44, 49, 39, 56, 34, 53,
		46, 42, 50, 36, 29, 32
};

const uint8_t DesRots[] = 
{
		1, 1, 2, 2, 
		2, 2, 2, 2, 
		1, 2, 2, 2, 
		2, 2, 2, 1, 
		0
};

void movram(uint8_t* source, uint8_t* target, uint8_t length)
{
		uint8_t i;
		
		for(i = 0;i < length;i++){
				target[i] = source[i];
		}
}

void doxor(uint8_t* sourceaddr, uint8_t* targetaddr, uint8_t length)
{
		uint8_t i;
		
		for (i = 0;i < length;i++) {
				sourceaddr[i] ^= targetaddr[i];
		}
}

void setbit(uint8_t* dataddr, uint8_t pos, uint8_t b0)	
{
		uint8_t byte_count;
		uint8_t bit_count;
		uint8_t temp;

		temp = 1;
		byte_count = (pos - 1) / 8;
		bit_count = 7 - ((pos - 1) % 8);
		temp <<= bit_count;
		
		if(b0) {
				dataddr[byte_count] |= temp;
		}
		else{
				temp = ~temp;
				dataddr[byte_count] &= temp;
		}
}

uint8_t getbit(uint8_t* dataddr, uint8_t pos)	
{
		uint8_t byte_count;
		uint8_t bit_count;
		uint8_t temp;

		temp = 1;
		byte_count = (pos - 1) / 8;
		bit_count = 7 - ((pos - 1) % 8);
		temp <<= bit_count;
		if(dataddr[byte_count] & temp){
				return(1);
		}
		else {
				return(0);
		}
}

void selectbits(uint8_t* source, const uint8_t* table, uint8_t* target, uint8_t count)
{
		uint8_t i;

		for(i=0; i<count; i++){
				setbit(target, i + 1, getbit(source, table[i])); 
		}
}

void shlc(uint8_t* data_p)
{
		uint8_t i,b0;

		b0 = getbit(data_p, 1);

		for(i=0; i<7; i++) {
				data_p[i] <<= 1;
				
				if(i != 6){
						setbit(&data_p[i], 8, getbit(&data_p[i + 1], 1));
				}
		}

		setbit(data_p, 56, getbit(data_p, 28));
		setbit(data_p, 28, b0);
}

void shrc(uint8_t* data_p)
{
		uint8_t b0;
		int i;
		
		b0 = getbit(data_p, 56);
		
		for(i=6; i>=0; i--) {
				data_p[i] >>= 1;
				
				if(i != 0) {
						setbit(&data_p[i], 1, getbit(&data_p[i - 1], 8)); 
				}
		}

		setbit(data_p, 1, getbit(data_p,29));
		setbit(data_p, 29, b0);
}

/* The problem is about yielded in this function */
void strans(uint8_t* index, uint8_t* target)
{
		uint8_t row, line, t, i, j;

		for(i=0; i<4; i++) {
				row = line = t = 0;
				setbit(&line, 7, getbit(index, i * 12 + 1));
				setbit(&line, 8, getbit(index, i * 12 + 6));
				
				for(j=2; j<6; j++) {
						setbit(&row, 3+j, getbit(index,i * 12 + j));
				}
				
				t = DesS[i * 2][line][row];
				t <<= 4;
				line = row = 0; 
				setbit(&line, 7, getbit(index,i * 12 + 7));
				setbit(&line, 8, getbit(index,i * 12 + 12));
				
				for(j=2; j<6; j++) {
						setbit(&row, 3+j, getbit(index, i * 12 + 6 + j));
				}
				
				t |= DesS[i * 2 + 1][line][row];
				target[i] = t;
		}
}

/* type: 1-加密, 0-解密 */
void des(uint8_t *data, uint8_t* key, int op)
{
		uint8_t tempbuf[12];
		uint8_t tempkey[7];
		uint8_t i;
		uint8_t j;
		uint8_t count;
		void (*shift_fun)(uint8_t* data);

		selectbits(data, DesIp, tempbuf, 64);/*????*/
		movram(tempbuf, data, 8);
		selectbits(key, DesPc_1, tempkey, 56);/*KEY?????*/

		for(i=0; i<16; i++) {
				selectbits(data+4, DesE, tempbuf, 48);/*????*/
				
				if(op == OP_ENC) {		//encrypt
						shift_fun = shlc;
						count = i;
				}
				else if(op == OP_DEC){
						count = 16 - i;	
						shift_fun = shrc;
				}
				else
						return ;
				
				for(j=0; j<DesRots[count]; j++) {
						shift_fun(tempkey);
				}

				selectbits(tempkey, DesPc_2, tempbuf+6, 48);
				doxor(tempbuf, tempbuf+6, 6);
				strans(tempbuf, tempbuf + 6);
				selectbits(tempbuf+6, DesP, tempbuf, 32);
				doxor(tempbuf,data, 4);

				if(i < 15) {
						movram(data + 4,data,4);
						movram(tempbuf,data + 4,4);
				}
		}

		movram(tempbuf, data, 4);
		selectbits(data, DesIp_1, tempbuf, 64);
		movram(tempbuf, data, 8);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void encrypt(uint8_t *input, uint8_t *key, uint8_t *output)
{
		uint8_t tmp[DES_BLOCK_SIZE];
		memcpy(tmp, input, DES_BLOCK_SIZE);
		des(tmp, key, OP_ENC);
		memcpy(output, tmp, DES_BLOCK_SIZE);
		return ;
}

void decrypt(uint8_t *input, uint8_t *key, uint8_t *output)
{
		uint8_t tmp[DES_BLOCK_SIZE];
		memcpy(tmp, input, DES_BLOCK_SIZE);
		des(tmp, key, OP_DEC);
		memcpy(output, tmp, DES_BLOCK_SIZE);
		return ;
}
/*********************************************************************************************************************
LK_DesEncrypt : DES/3DES加密接口函数,支持MODE_ECB和MODE_CBC模式，数据不足8字节按我司默认数据补齐规则补齐（参看开发手册）
*Mode：				Mode=0时MODE_ECB模式, Mode=1时为MODE_CBC模式
*Iv：					MODE_ECB模式时填写NULL, MODE_CBC模式是为8字节初始向量
*key_len:			密钥长度
*pKey：				密钥值 key_len == 0x08时, 使用DES算法; key_len == 0x10 时使用3DES算法
*indata_len: 	明文长度
*pIn：				明文数据 注：单次明文最大长度为 0xF8
*outdata_len：加密后的密文数据长度
*pOut：				加密后的密文数据
***********************************************************************************************************************/
uint8_t LK_DesEncrypt(uint8_t Mode, uint8_t *pIv, uint8_t key_len, uint8_t* pKey, uint8_t indata_len, uint8_t* pIn, uint8_t* outdata_len, uint8_t* pOut)
{
		uint8_t pad_num , k;
		uint8_t *tempIv;
		unsigned short i;
		unsigned short blk_num;

		if((indata_len == 0) ||  ((key_len != DES_BLOCK_SIZE) && (key_len != 0x10)))
				return FAIL;

		if((indata_len%DES_BLOCK_SIZE) != 0x00) {
				pad_num = DES_BLOCK_SIZE - (indata_len%DES_BLOCK_SIZE);
				if(pad_num != 0x00)
						pIn[indata_len+1] = 0x80;
				for(i=1; i<pad_num; i++)
						pIn[indata_len+1+i] = 0x00;
				indata_len += pad_num;
		}

		blk_num = indata_len / DES_BLOCK_SIZE;

		if((Mode == MODE_CBC) && (pIv == NULL))
				return FAIL;
		else if((Mode != MODE_ECB) && (Mode != MODE_CBC))
				return FAIL;	

		*outdata_len = indata_len;
		if(Mode == MODE_CBC)
			tempIv = pIv;

		for(i=0; i<blk_num; i++) {
				if( Mode == MODE_CBC) {
						for(k=0; k<DES_BLOCK_SIZE; k++)
								pOut[k] = pIn[k] ^ tempIv[k];
						encrypt(pOut, pKey, pOut);
				}
				else
						encrypt(pIn, pKey, pOut);
				if(key_len == 0x10) {
						decrypt(pOut, pKey+DES_BLOCK_SIZE, pOut);
						encrypt(pOut, pKey, pOut);
				}
				if( Mode == MODE_CBC)
					tempIv =pOut;
				pIn  += DES_BLOCK_SIZE;
				pOut += DES_BLOCK_SIZE;
		}
		return SUCCESS_WORK;
}

/*********************************************************************************************************************
LK_DesDecrypt: DES/3DES解密接口函数,支持MODE_ECB和MODE_CBC模式

*Mode：				Mode=0时MODE_ECB模式, Mode=1时为MODE_CBC模式
*Iv：					MODE_ECB模式时填写NULL, MODE_CBC模式是为8字节初始向量
*key_len:			密钥长度
*pKey：				密钥值 key_len == 0x08时, 使用DES算法; key_len == 0x10 时使用3DES算法
*indata_len: 	密文长度
*pIn：				密文数据 注：单次密文最大长度为 0xF8
*outdata_len：解密后的明文数据长度
*pOut：				解密后的明文数据

***********************************************************************************************************************/
uint8_t LK_DesDecrypt(uint8_t Mode, uint8_t *pIv, uint8_t key_len, uint8_t* pKey, uint8_t indata_len, uint8_t* pIn, uint8_t* poutdata_len, uint8_t* pOut)
{
		uint8_t  n;
		uint8_t	*tempIv;
		unsigned short  i;
		unsigned short blk_num;

		if((indata_len == 0) || ((indata_len%DES_BLOCK_SIZE) != 0x00) || ((key_len != DES_BLOCK_SIZE) && (key_len != 0x10)))
				return FAIL;	

		blk_num = indata_len / DES_BLOCK_SIZE;

		if(( Mode == MODE_CBC) && (pIv == NULL))
				return FAIL;	
		else if((Mode != MODE_CBC) && (Mode != MODE_ECB))
				return FAIL;	

		*poutdata_len = indata_len;
		
		if (pOut != pIn)
		{
				memcpy(pOut, pIn, indata_len);
		}

		pOut += indata_len - DES_BLOCK_SIZE; //最后一块
		for (i=0; i<blk_num; i++)
		{
				decrypt(pOut, pKey, pOut);
				if(key_len == 0x10) {
						encrypt(pOut, pKey+DES_BLOCK_SIZE, pOut);
						decrypt(pOut, pKey, pOut);
				}
			
				if(Mode == MODE_CBC)
				{
						if(i == (blk_num-1))	// 最后一块数据
						{
								for(n=0; n<DES_BLOCK_SIZE; n++)
									pOut[n] ^= pIv[n];
						}
						else
						{
							  tempIv = pOut - DES_BLOCK_SIZE;
								for(n=0; n<DES_BLOCK_SIZE; n++)
									pOut[n] ^=  tempIv[n];
						}
				}
				pOut -= DES_BLOCK_SIZE;
		}
		
		
	/*	if( Mode == MODE_CBC)
				memcpy(tempIv, Iv, DES_BLOCK_SIZE);
		
		

		for(i=0; i<blk_num; i++) {
				decrypt(pIn, pKey, pOut);

				if(key_len == 0x10) {
						encrypt(pOut, pKey+DES_BLOCK_SIZE, pOut);
						decrypt(pOut, pKey, pOut);
				}
				if( Mode == MODE_CBC) {
						for(k=0; k<DES_BLOCK_SIZE; k++)
								pOut[k] ^= tempIv[k];
						memcpy(tempIv, pIn, DES_BLOCK_SIZE);
				}
				pIn  += DES_BLOCK_SIZE;
				pOut += DES_BLOCK_SIZE;
		}*/
		return SUCCESS_WORK;
}
