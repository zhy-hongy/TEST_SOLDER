/******************** (C) COPYRIGHT *********************************************
* Project Name       : encrypt funciton
* Author             : linksafe
* Version            : V1.0
* Date               : Jan-17-2024
* Description        : 	
********************************************************************************/
#include "i2c.h"
#include "string.h"
#include "macro.h"
#include "des.h"

#include "debug.h"

#ifdef USE_AES
#include "aes.h"
#endif
#ifdef USE_XXTEA
#include "app_xxtea.h"
#endif
#ifdef USE_SHA256
#include "sha256.h"
#endif
#ifdef USE_RSA
#include "rsa.h"
#endif
#ifdef USE_SM4
#include "sm4.h"
#endif
#ifdef USE_SM3
#include "sm3.h"
#endif


const uint8_t CMD_FIX_HEAD[4] = {0x80, 0x08, 0x00, 0x00};		//加密IC通信协议的固定指令头：8008 0000

/***************************************************************************************************
* Function： 						将字符串转换成十六进制数据
* input parameter: 
*		uint8_t *pbSrc:					输入的字符串
*		uint32_t len：						输入数据长度
* output parameter:
*		uint8_t* pbDest:					输出的十六进制数据
*	return:
*		0: 									转换失败
*		非0：								转换成功
****************************************************************************************************/
uint32_t StrToHex(uint8_t *pbDest, uint8_t *pbSrc, uint32_t len)
{
		char h1, h2;
		unsigned char s1, s2;
		int i;

		for (i=0; i<len; i++)
		{
				h1 = pbSrc[2*i];
				h2 = pbSrc[2*i+1];
				if((h1>=0x30) && (h1<=0x39)) //0~9
						s1 = h1 - 0x30;
				else if((h1>=65) && (h1<=70)) //A~F
						s1 = h1 - 55;
				else if((h1>=97) && (h1<=102)) //a~f
						s1 = h1 - 87;
				else
						return 0;

				if((h2>=0x30) && (h2<=0x39)) //0~9
						s2 = h2 - 0x30;
				else if((h2>=65) && (h2<=70)) //A~F
						s2 = h2 - 55;
				else if((h2>=97) && (h2<=102)) //a~f
						s2 = h2 - 87;
				else
						return 0;
				pbDest[i] = s1*16 + s2;
		}
		return i;
}

void CRC16_CCITTFALSE(uint8_t *data, int len, unsigned char *crc)
{
		uint16_t crc_register = 0xFFFF;
		uint8_t i;

		while(len-- != 0)
		{
				for(i=0x80; i!=0; i/= 2)
				{
						if((crc_register & 0x8000) != 0)
						{
								crc_register *= 2;
								crc_register ^= 0x11021;
						}
						else
								crc_register *= 2;

						if((*data&i) != 0)
								crc_register ^= 0x11021;
				} 
				data++;
		}
		crc[1] = (unsigned char)(crc_register & 0x00FF);
		crc[0] = (unsigned char)(crc_register >> 8);
}

/***************************************************************************************************
* Function： 						填充函数
* input parameter: 
*		uint8_t *InOutBuf:				待填充数据
*		int Inoutlen：			待填充数据长度
*		uint8_t Blocksize：			填充组长度
* output parameter:
*		uint8_t *InOutBuf:				填充后的数据
*	return:
*		len:								填充后的数据长度
****************************************************************************************************/
int LKTPading(uint8_t *InOutBuf, uint16_t Inoutlen, uint16_t Blocksize)
{
		uint16_t  paddata = 0x00;
		uint16_t len;
	  len =Inoutlen;
	  if(len%Blocksize ==0)
			return len;
	
		paddata = Blocksize - (len%Blocksize);  //需要补的数据长度
		InOutBuf[len++] = 0x80;
		paddata--;
		
		while(paddata--)
			InOutBuf[len++] = 0; 
		
		return len;	
}


/***************************************************************************************************
* Function： 						获取密钥区状态
* input parameter: 
* output parameter:
*		uint8_t* key_status:			key状态   0x00:调试密钥  0x01:安全密钥
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t get_key_status( uint8_t* key_status)
{
		uint16_t sw_code;
		uint16_t sLen;
		uint8_t tmpbuf[8];
	  TIIC_RData_Struct Rdata;
		sLen = 8;												//I2C指令长度
		tmpbuf[0] = 0x00;
		tmpbuf[1] = 0x06;									//后续指令长度
		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);

		tmpbuf[6] = 01;										//后续指令数据长度
		tmpbuf[7] = CMD_GET_KEY_STATUS;		//读取key状态

		//发送I2C指令, set key
	  Rdata.data = tmpbuf;
	
		if(IIC_SendApdu(sLen, tmpbuf, &Rdata)!=SUCCESS_WORK)
				return FAIL;

		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;
		*key_status = Rdata.data[0];
		return SUCCESS_WORK;
}


/***************************************************************************************************  
* Function： 						设置密钥区数据
* input parameter: 
*		uint8_t mode:
*												0: 表示工作在应用模式, 密文设置key
*												1: 表示工作在安全模式, 明文设置key;

*		uint8_t* new_key:				更新的key值, 32字节
*		uint8_t* old_key:				安全模式, 需要old key, 32字节; 调试阶段, 该参数为NULL。 
* output parameter:
* 	none
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
*	cmd ：						调试密钥: 8008000023 02 01 00 1122334455667788112233445566778800010203040506070807060504030201 
										正式密钥: 8008000025 02 01 01 7FFD87F2D0C91C899A3B16BAD4BC066445285D8706399BBBF99C51297CDB01D3 9B69

****************************************************************************************************/
uint8_t Set_Key(uint8_t mode, uint8_t* new_key, uint8_t* old_key)
{
	   uint8_t  tmpbuf[16];
	  uint8_t 	flag =0xFF;
		uint8_t	out_len;
		uint16_t sw_code;
		uint16_t sLen;
	  uint8_t *wkey;
		uint8_t	key_status;
		uint8_t	crc_val[2];
    TIIC_RData_Struct Rdata;
	  
	  if((mode != DEBUG_MODE)&&(mode != SECURITY_MODE))
			return FAIL;
	
		if(get_key_status(&key_status) != SUCCESS_WORK)	//获取当前状态
				return FAIL;


		//选择密钥模式
		if(mode == DEBUG_MODE)												//调试模式
		{
				if(key_status == SECURITY_MODE)						//如果已经处于应用状态, 不允许返回调试模式
					return FAIL;
				wkey  = new_key;
				sLen = KEY_FIXED_LEN + 10;				//指令长度
				flag = EndPack;
		}
		else{
			if(old_key == NULL)
				return FAIL;
			CRC16_CCITTFALSE(new_key, KEY_FIXED_LEN, crc_val); //计算新密钥CRC
			memcpy(tmpbuf,old_key,0x10);
			if(LK_DesEncrypt(MODE_ECB, NULL, KEY_LEN_3DES, tmpbuf, KEY_FIXED_LEN, new_key, &out_len, old_key) != SUCCESS_WORK)
				return FAIL;
			
	    wkey = old_key;
			sLen = KEY_FIXED_LEN +12;	  //加入CRC长度
			flag = ContinuePack;
		}
		
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);
		tmpbuf[7] = CMD_SET_CONFIG_DATA;		//设置key和参数	
		tmpbuf[8] = SET_KEY_DATA;
		tmpbuf[9] =  mode;								//模式
		

		tmpbuf[0] = (uint8_t)((sLen-2)>>8);
		tmpbuf[1] = (uint8_t)(sLen-2);			//后续指令长度	
		tmpbuf[6] = sLen-5-2;			    //后续指令数据长度
		
		if(IIC_WriteData(tmpbuf, 0x0A,FirstPack)!=SUCCESS_WORK)
				return FAIL;
		if(IIC_WriteData(wkey,KEY_FIXED_LEN,flag)!=SUCCESS_WORK)
				return FAIL;
		if(mode == SECURITY_MODE){
			 	if(IIC_WriteData(crc_val,0x02,EndPack)!=SUCCESS_WORK)
					return FAIL;
		}
		Rdata.data = tmpbuf;
		IIC_ReadData(&Rdata); 
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;

		return SUCCESS_WORK;
}

/***************************************************************************************************  
* Function： 						向参数区指定偏移地址写入指定长度数据
* input parameter: 
*		uint8_t offset：				参数区偏移地址
*		uint8_t* in_data:				设置的数据值
*		uint8_t data_len:				设置的数据值长度
*		uint8_t* key:						线路保护密钥
* output parameter:
* 	none
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL											失败
****************************************************************************************************/
uint8_t Set_Parameter(uint8_t offset,uint8_t data_len, uint8_t* in_data, uint8_t* pKey)
{
	//	uint8_t 	ret;
	  uint8_t 	tmpbuf[11];
		uint8_t  modbuf[8];
	  uint8_t flag=0xFF;
		uint8_t	outlen;
	  uint8_t	tmplen;
		uint16_t sw_code;
		uint16_t sLen;
		
	  TIIC_RData_Struct Rdata;
	  if(data_len == 0x00)
			return SUCCESS_WORK;
		
	  tmpbuf[10] = data_len;		//data长度
		
	  tmplen = data_len%DES_BLOCK_SIZE;
	  if(tmplen!= 0x00){
			memcpy(modbuf,in_data+(data_len-tmplen),tmplen);
			data_len -= tmplen;
			tmplen = LKTPading(modbuf, tmplen, DES_BLOCK_SIZE);
		}
	  
	  //固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);
    
		sLen = data_len + 11 +tmplen;					//指令长度
		tmpbuf[0] = 0x00;
		tmpbuf[1] = (uint8_t)(sLen-2);				//后续指令长度
		
		tmpbuf[6] = tmpbuf[1]-5;					//后续指令数据长度
		
		tmpbuf[7] = CMD_SET_CONFIG_DATA;	//设置key和参数
		//设置DATA
		tmpbuf[8] = SET_PARAMETER_DATA;
		tmpbuf[9] = offset;
		

		if(data_len !=0x00){
			if(LK_DesEncrypt(MODE_ECB, NULL, KEY_LEN_3DES, pKey, data_len, in_data, &outlen, in_data) != SUCCESS_WORK)
				return FAIL;
		}
		if(tmplen !=0x00){
			if(LK_DesEncrypt(MODE_ECB, NULL, KEY_LEN_3DES, pKey, tmplen, modbuf, &outlen, modbuf) != SUCCESS_WORK)
				return FAIL;
		}
		
		if(IIC_WriteData(tmpbuf, 0x0B,FirstPack) !=SUCCESS_WORK)
			return FAIL;
		if(tmplen == 0x00)
			flag = EndPack;
		else
			flag = ContinuePack;
		if(IIC_WriteData(in_data, data_len,flag) !=SUCCESS_WORK)
			return FAIL;
		if(flag == ContinuePack){
			if(IIC_WriteData(modbuf, tmplen,EndPack) !=SUCCESS_WORK)
				return FAIL;
		}
		
		Rdata.data = tmpbuf;
		if(IIC_ReadData(&Rdata)!= SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;

		return SUCCESS_WORK;
}


/**************************************************************************************************** 
* Function： 对比认证
* input parameter: 
*		uint8_t alg_sel:						认证采用的算法:
*				00: 3DES
*				01: AES_128
*				02: SM4
*				03: XXTEA
*				04: SHA256
*				05: SM3
*		uint8_t* key:							key值
*		uint8_t* random_16_bytes:	16字节随机数
* output parameter:
* none
*	return:
*		SUCCESS_WORK: 			成功
*		FAIL					失败
* cmd:					3DES : 8008 0000 13 03 20 10 A7A9FE3F5FCD1F42FC3F26AFC0135245
*****************************************************************************************************/
uint8_t Authenticate(uint8_t alg_sel, uint8_t* pKey, uint8_t* random_16_bytes)
{
		uint8_t tmpbuf[65];
		uint8_t *rand2;
		uint8_t outdata_len;
		uint16_t sw_code;
		uint8_t ret, i;
		uint16_t sLen, rLen;
		uint8_t key_len;
		uint8_t *Iv;
		//uint8_t *key_relocate;
		uint8_t *outdata;
		uint8_t *xordata;
	  
	  TIIC_RData_Struct Rdata;
#ifdef USE_XXTEA
		uint8_t rand1[16];
#endif
#ifdef USE_SM3	
sm3_ctx_t ctx;
#endif  
	
		if(alg_sel > AU_ALG_SM3)
		    {
			return FAIL;}
	
		
    outdata = tmpbuf+16;
		// 获取16字节随机数rand1,将随机数加密发给加密芯片

		if(alg_sel == AU_ALG_3DES)										// 采用的加密算法：3DES
		{
				//3DES_CBC模式  对16字节随机数进行加密 3DES CBC
			Iv = pKey + KEY_LEN_3DES;
			key_len = KEY_LEN_3DES;
			if(LK_DesEncrypt(MODE_CBC, Iv, key_len, pKey, CHALLENGE_LEN, random_16_bytes, &outdata_len, outdata) != SUCCESS_WORK)
		    	{
				return FAIL;}
				
		}
#ifdef USE_AES
		else if(alg_sel == AU_ALG_AES_128)										// 采用的加密算法：AES128
		{
				//AES Para: (8bytes iv) | (16bytes key) | (8bytes iv)
			memcpy(tmpbuf, pKey, 8);
			memcpy(tmpbuf+8, pKey+24, 8);
			key_relocate = pKey + 8;
			
			Iv = tmpbuf;
			key_len = KEY_LEN_AES128;
				//init key
			AES_Init(key_relocate);

				//AES_CBC模式
			if(LK_AesEncrypt(MODE_CBC, Iv, key_len, key_relocate, CHALLENGE_LEN, random_16_bytes, &outdata_len, outdata) != SUCCESS_WORK)
				{log_info("AES_CBC==>3");	
				return FAIL;}	
		}
#endif
#ifdef USE_SM4
		else if(alg_sel == AU_ALG_SM4)										// 采用的加密算法： SM4
		{
				//SM4 Para: (16bytes iv) | (16bytes key)
			key_relocate = pKey + 16;
			Iv = pKey;
			key_len = KEY_LEN_SM4;

				//SM4_CBC模式
			if(LK_SM4Encrypt(MODE_CBC, Iv, key_len, key_relocate, CHALLENGE_LEN, random_16_bytes, &outdata_len, outdata) != SUCCESS_WORK)
				{log_info("SM4_CBC==>4");	
				return FAIL;}
		}
#endif
#ifdef USE_XXTEA
		else if(alg_sel == AU_ALG_XXTEA)								// 采用的加密算法： XXTEA
		{
			memcpy(rand1, random_16_bytes, CHALLENGE_LEN);
				// 对随机数进行加密 XXTEA
			if(XXTea_ENC(pKey,CHALLENGE_LEN, rand1, &outdata_len, outdata ) != SUCCESS_WORK)
				return FAIL;
		}
#endif
		
		//对比认证指令
		sLen = CHALLENGE_LEN + 10;					// 指令总长度, 包含"2字节后续指令长度+8字节固定指令头+16字节数据长度"
		tmpbuf[0] = 0x00;
		tmpbuf[1] = CHALLENGE_LEN + 8;		// 后续指令长度
	
		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);

		tmpbuf[6] = CHALLENGE_LEN + 3;		// 后续指令数据长度, 3：操作码+算法选择码+长度这3个字节长度
		tmpbuf[7] = CMD_AUTHENTICATE;					// "对比认证"操作码

		tmpbuf[8] = alg_sel;									// 算法

		tmpbuf[9] = CHALLENGE_LEN;				// 对比认证数据的长度
		
		if((alg_sel != AU_ALG_SHA256) &&(alg_sel != AU_ALG_SM3))
			memcpy(tmpbuf+10, outdata, outdata_len);
		else
			memcpy(tmpbuf+10, random_16_bytes, CHALLENGE_LEN);
	
//		log_info("tmpbuf\n");
//		memset(debug_string_out_0, 0, 100);
//		for(uint8_t i = 0; i < 36; i++){
//			sprintf(debug_string_out_1, "%X ", tmpbuf[i]);
//			strcat(debug_string_out_0, debug_string_out_1);
//		}
//		strcat(debug_string_out_0, "\n");
//		log_info(debug_string_out_0);
		
		Rdata.data = tmpbuf+32;
		ret = IIC_SendApdu(sLen, tmpbuf,&Rdata);
		
//		log_info("Rdata\n");
//		memset(debug_string_out_0, 0, 100);
//		for(uint8_t i = 0; i < 36; i++){
//			sprintf(debug_string_out_1, "%X ", Rdata.data[i]);
//			strcat(debug_string_out_0, debug_string_out_1);
//		}
//		strcat(debug_string_out_0, "\n");
//		log_info(debug_string_out_0);
		
		if(ret != SUCCESS_WORK)
		{	
				return FAIL;}

		//取指令状态码, 正常状态码是0x9000
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
		{
				return FAIL;}

		outdata =tmpbuf+32; 
		if(alg_sel == AU_ALG_3DES)											
		{
				//MODE_CBC
				//对比认证返回数据, 先进行CBC解密, 得到明文数据为：len | rand1^rand2 | rand2
			if(LK_DesDecrypt(MODE_CBC, Iv, key_len, pKey, Rdata.data[0], Rdata.data+1, &outdata_len, outdata) != SUCCESS_WORK)
				{	
				return FAIL;}
		}
#ifdef USE_AES
		else if(alg_sel == AU_ALG_AES_128)										
		{
			memcpy(tmpbuf, pKey, 8);
			memcpy(tmpbuf+8, pKey+24, 8);
			Iv = tmpbuf;
				//MODE_CBC, 对比认证返回数据, 先进行CBC解密, 得到明文数据为：len | rand1^rand2 | rand2
			if(LK_AesDecrypt(MODE_CBC, Iv, key_len, key_relocate, Rdata.data[0], Rdata.data+1, &outdata_len, outdata) != SUCCESS_WORK)
				return FAIL;
		}
#endif
#ifdef USE_SM4
		else if(alg_sel == AU_ALG_SM4)											
		{
				//MODE_CBC, 对比认证返回数据, 先进行CBC解密, 得到明文数据为：len | rand1^rand2 | rand2
			if(LK_SM4Decrypt(MODE_CBC, Iv, key_len, key_relocate,Rdata.data[0], Rdata.data+1, &outdata_len, outdata) != SUCCESS_WORK)
				return FAIL;
		}
#endif
#ifdef USE_XXTEA
		else if(alg_sel == AU_ALG_XXTEA)							
		{
			memcpy(rand1, random_16_bytes, CHALLENGE_LEN);
			if(XXTea_DEC(pKey,Rdata.data[0],Rdata.data+1, (uint8_t*)&rLen, outdata) != SUCCESS_WORK)
					return FAIL;
			random_16_bytes = rand1;
		}
#endif
#ifdef USE_SHA256
		else if(alg_sel == AU_ALG_SHA256)						
		{		
			SHA256_Reset();
			SHA256_MsgIn(random_16_bytes,CHALLENGE_LEN);
			SHA256_MsgIn(pKey,KEY_FIXED_LEN);
			outdata =tmpbuf;
			SHA256_Out(outdata);
		}
#endif
#ifdef USE_SM3
		else if(alg_sel == AU_ALG_SM3)						
		{
			sm3_init(&ctx);
			sm3_update(&ctx, random_16_bytes, CHALLENGE_LEN);
			sm3_update(&ctx, pKey, KEY_FIXED_LEN);
			outdata =tmpbuf;
			sm3_final(&ctx, outdata);
		}
#endif
		if(alg_sel <= AU_ALG_XXTEA){
			rand2 = outdata + CHALLENGE_LEN;
			xordata = outdata;
			for(i=0; i<CHALLENGE_LEN; i++){
				if(xordata[i] != (random_16_bytes[i] ^ rand2[i]))
						return FAIL;
			}
		}
		else{
			if(memcmp(Rdata.data+1, outdata, SHA256_OUT_DATA_LEN) != 0)
				return FAIL;
		}
		return SUCCESS_WORK;
}

/**************************************************************************************************** 
* Function： 从参数区指定偏移量读出指定长度数据数据
* input parameter: 
*		uint8_t* pKey:							线路保护密钥
*		uint8_t random_16_bytes：	16字节随机值
*   uint8_t offset:            参数区的偏移量
*		uint8_t data_len:					需要读出的长度
* output parameter:
* 	uint8_t* outdata:					大小应为16字节 + 参数长度补足8字节倍数后的长度
*	return:
*		SUCCESS_WORK: 			成功
*		FAIL					失败
*****************************************************************************************************/
uint8_t Read_Data_Protected(uint8_t* random_16_bytes, uint8_t* pKey,  uint8_t offset,uint8_t pdata_len, uint8_t* outdata)
{
	 	uint8_t tmpbuf[16];
		uint8_t i;
		uint16_t sw_code;
	  uint8_t outlen;
	  TIIC_RData_Struct Rdata;
	
	  if(pdata_len == 0x00)
			return SUCCESS_WORK;
	
		//读取保护数据指令
	
		tmpbuf[0] = 0x00;
		tmpbuf[1] = CHALLENGE_LEN + 8;		// 后续指令长度
	
		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);

		tmpbuf[6] = CHALLENGE_LEN + 3;		// 后续指令数据长度, 3：操作码+长度这2个字节的长度
		tmpbuf[7] = CMD_READ_DATA_PROTECTED;		// "读取保护数据"操作码
		tmpbuf[8] = offset;
		tmpbuf[9] = pdata_len;				// 需要读取的参数长度

		//rand1用3DES MODE_ECB模式加密
		if(LK_DesEncrypt(MODE_ECB, NULL, 0x10, pKey, CHALLENGE_LEN, random_16_bytes, &outlen, outdata) != SUCCESS_WORK)
			return FAIL;
		
	
		if(IIC_WriteData(tmpbuf, 0x0A,FirstPack)!=SUCCESS_WORK)
			return FAIL;
	
		if(IIC_WriteData(outdata,CHALLENGE_LEN,EndPack)!=SUCCESS_WORK)
			return FAIL;

		Rdata.data = outdata;
		if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
			return FAIL;
	
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;
		

		//返回数据 {rand2密文 | data密文}, rand2密文是用参数保护key加密的, data密文是用(rand1^rand2)作为临时密钥加密的. 
		//解密rand2
		if(LK_DesDecrypt(MODE_ECB, NULL, 0x10, pKey, CHALLENGE_LEN, Rdata.data, &outlen, tmpbuf) != SUCCESS_WORK)
				return FAIL;

		//xordata = rand1^rand2 为临时密钥, 用临时密钥解密data密文
		
		for(i=0; i<CHALLENGE_LEN; i++)
				tmpbuf[i] = random_16_bytes[i] ^ tmpbuf[i]; //tempbuf = rand2; xor后tmpbuf为参数解密密钥

		//解密data密文
		if(LK_DesDecrypt(MODE_ECB, NULL, 0x10, tmpbuf,(Rdata.len[0]*0x100+Rdata.len[1])-0x12, Rdata.data+CHALLENGE_LEN, &outlen, outdata) != SUCCESS_WORK)
		{
				return FAIL;
		}
		return SUCCESS_WORK;
}


/**************************************************************************************************** 
* Function： 对输入数据取反操作
* input parameter: 
*		uint8_t LenOfIn:						输入数据长度
*		uint8_t* pIn:							输入数据
* output parameter:
*		uint8_t	LenOfOut:					取反后数据长度
* 	uint8_t* pOut:							取反后的数据
*	return:
*		SUCCESS_WORK: 			成功
*		FAIL					失败
*****************************************************************************************************/
uint8_t DataNot_Func(uint8_t LenOfIn, uint8_t *pIn, uint8_t *pOut)
{
		uint8_t tmpbuf[9];
		uint16_t sw_code;
	  TIIC_RData_Struct Rdata;

		tmpbuf[0] = 0x00;
		tmpbuf[1] = LenOfIn + 6;						// 后续指令长度
	
		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);

		tmpbuf[6] = LenOfIn + 1;						// 后续指令数据长度, 1：操作码
		tmpbuf[7] = CMD_DATA_NOT;						// 数据取反命令字
	
	
		//发送I2C指令
		if(IIC_WriteData(tmpbuf, 0x08,FirstPack)!=SUCCESS_WORK)
			return FAIL;
		if(IIC_WriteData(pIn,LenOfIn,EndPack)!=SUCCESS_WORK)
			return FAIL;

		Rdata.data = pOut;
		if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;
		
		memset(Rdata.data,0x00,0x20);
		if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
		return SUCCESS_WORK;
}



/******************************************************************************************************** 
* Function： 获取加密芯片ID
* input parameter: 
*		none
* output parameter:
*		uint8_t	LenOfOut:					ID长度, ID长度是8字节
* 	uint8_t* pOut:							ID值
*	return:
*		SUCCESS_WORK: 							成功
*		FAIL									失败
********************************************************************************************************/
uint8_t GetID(uint8_t *LenOfOut, uint8_t *pOut)
{
		uint8_t tmpbuf[9];
		uint8_t sLen;
		uint16_t sw_code;
	  TIIC_RData_Struct Rdata;
	
		sLen = 8;															// 指令总长度, 包含"2字节后续指令长度+6字节固定指令头+输入数据"
		tmpbuf[0] = 0x00;
		tmpbuf[1] = 6;											// 后续指令长度
	
		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);
		tmpbuf[6] = 1;											// 后续指令数据长度, 1：操作码长度
		tmpbuf[7] = CMD_GET_CHIP_ID;				// 获取芯片ID命令字
	
		//发送I2C指令
	  Rdata.data = pOut;
		if(IIC_SendApdu(sLen, tmpbuf, &Rdata)!= SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
			return FAIL;
		*LenOfOut = 0x08;
		return SUCCESS_WORK;
}

/******************************************************************************************************** 
* Function： 从加密芯片获取随机数
* input parameter: 
*		uint8_t	LenOfRand:				随机数长度
* output parameter:
* 	uint8_t* pOut:							获取的随机数
*	return:
*		SUCCESS_WORK: 							成功
*		FAIL									失败
********************************************************************************************************/
uint8_t GetRandom_Func(uint16_t LenOfRand, uint8_t *pOut)
{
		uint8_t tmpbuf[9];
	  uint16_t offset;
		uint16_t sw_code;
	  TIIC_RData_Struct Rdata;
	
	  offset = 0x00;
	  Rdata.data = pOut;
	  while(LenOfRand){
			memcpy(tmpbuf,"\x00\x07\x80\x08\x00\x00\x02\x00\x00",0x08);
			if((LenOfRand<=0xF5)){
				tmpbuf[8] = LenOfRand;
			}
			else{
				tmpbuf[8] = 0xF5 ;
			}
			tmpbuf[7] = CMD_GET_RANDOM;	
			tmpbuf[0] = 0x00;
			tmpbuf[1] = 7;											// 后续指令长度
	    LenOfRand -= tmpbuf[8];
			Rdata.data = pOut;
			Rdata.data +=offset;
			offset += tmpbuf[8];
			if(IIC_SendApdu(0x09, tmpbuf, &Rdata)!= SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
			if(sw_code != 0x9000)
				return FAIL;
		}
		return SUCCESS_WORK;
}


/********************************************************************************************************
* Function： 擦除flash
* input parameter: 
*		uint16_t	addr:							擦除flash扇区地址
*		uint16_t size:							擦除flash扇区长度
* output parameter:
* 	none
*	return:
*		SUCCESS_WORK: 							成功
*		FAIL									失败
********************************************************************************************************/

uint8_t FlashErase(uint16_t addr, uint16_t size)
{
	 	uint8_t tmpbuf[12];
		uint16_t sw_code;
		uint16_t sLen;
	  TIIC_RData_Struct Rdata;
	 
		sLen = 12;														// 指令总长度, 包含"2字节后续指令长度+6字节固定指令头+4字节flash地址和空间大小"
		tmpbuf[0] = 0x00;
		tmpbuf[1] = 10;											// 后续指令长度
	
		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);

		tmpbuf[6] = 5;											// 后续指令数据长度, 1：操作码长度
		tmpbuf[7] = CMD_FLASH_ERASE;				// 擦除flash命令字

		tmpbuf[8] = addr/0x100;							// 擦除flash地址高8bit
		tmpbuf[9] = addr%0x100;							// 擦除flash地址低8bit
		tmpbuf[10] = size/0x100;						// 擦除flash空间高8bit
		tmpbuf[11] = size%0x100;						// 擦除flash空间低8bit
		
		//发送I2C指令
		Rdata.data = tmpbuf;
		if(IIC_SendApdu(sLen, tmpbuf,&Rdata)!=SUCCESS_WORK)
				return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;

		return SUCCESS_WORK;
}


/********************************************************************************************************
* Function： 写flash
* input parameter: 
*		uint16_t	addr:							写flash首地址   ，需与芯片要求的对齐长度一致
*		uint8_t size:							写flash数据长度 , 需与芯片要求的对其长度一致
*		uint8_t *pData:						写数据
* output parameter:
* 	none
*	return:
*		SUCCESS_WORK: 							成功
*		FAIL									失败
********************************************************************************************************/
uint8_t FlashWrite(uint16_t addr, uint16_t in_len, uint8_t *pIndata)
{
		uint8_t tmpbuf[12];
		uint16_t sw_code;
	  uint16_t offset;
	  TIIC_RData_Struct Rdata;
	  
	  offset = 0x00;
	  while(in_len){
			memcpy(tmpbuf,"\x00\x00\x80\x08\x00\x00\x00\x00\x00\x00\x00",0x0B);
			if((in_len<=0xF0)){
				tmpbuf[1] = (uint8_t)(in_len+9);
				tmpbuf[10] = in_len;
				tmpbuf[6] = in_len+4;
			}
			else{
				tmpbuf[1] = 0xF0 + 9;
				tmpbuf[10] = 0xF0;
				tmpbuf[6] = 0xF0 + 4;
			}
			in_len -= tmpbuf[10];
			tmpbuf[7] = CMD_FLASH_WRITE;	
			tmpbuf[8] = (uint8_t)((addr+offset)>>8);							
			tmpbuf[9] = (uint8_t)(addr+offset);		
	
			if(IIC_WriteData(tmpbuf, 0x0B,FirstPack)!=SUCCESS_WORK)
				return FAIL;
			if(IIC_WriteData(pIndata+offset,tmpbuf[10],EndPack)!=SUCCESS_WORK)
				return FAIL;
			offset += tmpbuf[10];
			Rdata.data = tmpbuf;
			if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
			if(sw_code != 0x9000)
				return FAIL;	
		}
		return SUCCESS_WORK;
}


uint8_t  Transmit_Data(uint8_t *tmpbuf,uint16_t len , uint8_t *data ,uint8_t packdata)
{
		uint16_t sw_code;
	  uint16_t offset=0;
    TIIC_RData_Struct Rdata;
	
	  Rdata.data = tmpbuf;

		while(len){
			memcpy(tmpbuf,"\x00\x00\x80\x08\x00\x00\x00\x00",0x08);
			if((len<=packdata)){
				tmpbuf[1] = (uint8_t)(len+6);
				tmpbuf[6] = len+1;
			}
			else{
				tmpbuf[1] = packdata + 6;
				tmpbuf[6] = packdata + 1;
			}
			len -=( tmpbuf[6]-1);
			tmpbuf[7] = CMD_SET_DATA;	
			if(IIC_WriteData(tmpbuf, 0x08,FirstPack)!=SUCCESS_WORK)
				return FAIL;
			if(IIC_WriteData(data+offset,tmpbuf[6]-1,EndPack)!=SUCCESS_WORK)
				return FAIL;
			offset +=(tmpbuf[6]-1);
			Rdata.data = tmpbuf;
			if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];				
			if(sw_code != 0x9000)
				return FAIL;
		}
		
		return SUCCESS_WORK;

}


uint8_t Get_Data(uint8_t *tmpbuf,uint16_t read_len , uint8_t *pOutdata ,uint8_t packdata)
{
		uint16_t sw_code;
	  uint16_t offset=0;
    TIIC_RData_Struct Rdata;
	
		Rdata.data = pOutdata;
		while(read_len){
			memcpy(tmpbuf,"\x00\x07\x80\x08\x00\x00\x02\x00\x00",0x09);	
			if((read_len<=packdata))
				tmpbuf[8] = read_len;
			else
				tmpbuf[8] = packdata;
				
			read_len -=tmpbuf[8];
			tmpbuf[7] = CMD_READ_OUTDATA;	
				
			if(IIC_WriteData(tmpbuf, 0x09,OnePack)!=SUCCESS_WORK)
				return FAIL;
			Rdata.data += offset ;
			if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];	
			if(sw_code != 0x9000)
				return FAIL;
			offset = Rdata.len[0]*0x100 + Rdata.len[1]-2;	
		}		
		
		return SUCCESS_WORK;

}

/******************************************************************************************************** 
* Function： 调用安全芯片内的APP_Command函数
* input parameter: 
*		uint8_t FuncNu:						函数序号
*   uint8_t param_len          传入的参数长度
*		uint8_t *param:						传入该函数的参数
* output parameter:
*		TIIC_RData_Struct *Rdata:	存储返回的长度 、数据 、状态码			
*	return:
*		SUCCESS_WORK: 			成功
*		FAIL					失败
********************************************************************************************************/
uint8_t Call_APP_Command(uint8_t FuncNu,uint8_t param_len, uint8_t *param, TIIC_RData_Struct *Rdata)
{
		uint8_t tmpbuf[8];
		uint16_t sLen ;

		sLen  = param_len + 1 +5;
	
		tmpbuf[0] = (uint8_t)(sLen>>8);
		tmpbuf[1] = (uint8_t)sLen;						// 后续指令长度
	  
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4); //固定指令头8008 0000
	  tmpbuf[6] = param_len + 1;
	  tmpbuf[7] = FuncNu;
	
	  
		if(IIC_WriteData(tmpbuf, 0x09,FirstPack)!=SUCCESS_WORK)
			return FAIL;
		 if(IIC_WriteData(param, param_len,EndPack)!=SUCCESS_WORK)
			return FAIL;

		if(IIC_ReadData(Rdata)!= SUCCESS_WORK)
			return FAIL;
	 
		return SUCCESS_WORK;
}

/********************************************************************************************************
* Function： 计算摘要值
* input parameter: 
*		uint8_t	alg：							ALG_SHA256： SHA256	  ALG_SM3: SM3  	ALG_SHA1：SHA1	
*		uint8_t	in_len:					  进行摘要计算的数据长度
*		uint8_t *pIndata:					进行摘要计算的数据
* output parameter:
* 	uint8_t *pOutdata:					摘要值
*		uint8_t *out_len:					摘要长度
*	return:
*		SUCCESS_WORK: 							成功
*		FAIL									失败
********************************************************************************************************/
uint8_t Get_Hash_Digest_Func(uint8_t alg, uint16_t in_len, uint8_t *pIndata, uint8_t *out_len, uint8_t *pOutdata)
{
		uint8_t tmpbuf[256],count,i;
		uint16_t sw_code;
	  TIIC_RData_Struct Rdata;
	
		*out_len = 0x00;
	 
	  count=in_len/240;	 
	  if(count<1){//小于240字节不用分包
			memcpy(tmpbuf,"\x00\x07\x80\x08\x00\x00\x02\x00\x00",0x09);
			tmpbuf[1]=9+in_len;
			tmpbuf[6]=2+in_len;
			tmpbuf[7] = CMD_GET_DIGEST;					// 计算摘要值命令字
		  if(alg == DIGEST_SHA1)
				tmpbuf[8] = 0;									// 计算摘要算法SHA1
		  else if(alg == DIGEST_SHA256)
				tmpbuf[8] = 1;									// 计算摘要算法SHA256
		  else if(alg == DIGEST_SM3)
				tmpbuf[8] = 2;									// 计算摘要算法SM3
			memcpy(tmpbuf+9,pIndata,in_len);
			if(IIC_WriteData(tmpbuf, 9+in_len,OnePack)!=SUCCESS_WORK)
				return FAIL;
			Rdata.data = pOutdata;
			if(IIC_ReadData(&Rdata)!= SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
			if(sw_code != 0x9000)
				return FAIL;			
		}else{
		  for(i=0;i<count;i++){
			  memcpy(tmpbuf,"\x00\x07\x80\x08\x00\x00\x02\x00\x00",0x09);
			  tmpbuf[1]=9+240;
			  tmpbuf[6]=2+240;
			  tmpbuf[7] = CMD_GET_DIGEST;					// 计算摘要值命令字
				
				if(i==0){
					if(alg == DIGEST_SHA1)
						tmpbuf[8] = 0;									// 计算摘要算法SHA1
					else if(alg == DIGEST_SHA256)
						tmpbuf[8] = 1;									// 计算摘要算法SHA256
					else if(alg == DIGEST_SM3)
						tmpbuf[8] = 2;									// 计算摘要算法SM3
				}else{
				  if(alg == DIGEST_SHA1)
						tmpbuf[8] = 0x10;									// 计算摘要算法SHA1
					else if(alg == DIGEST_SHA256)
						tmpbuf[8] = 0x11;									// 计算摘要算法SHA256
					else if(alg == DIGEST_SM3)
						tmpbuf[8] = 0x12;
				}
				
				memcpy(tmpbuf+9,pIndata,240);
				if(IIC_WriteData(tmpbuf, 9+in_len,OnePack)!=SUCCESS_WORK)
					return FAIL;
				Rdata.data = pOutdata;
				if(IIC_ReadData(&Rdata)!= SUCCESS_WORK)
					return FAIL;
				sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
				if(sw_code != 0x9000)
					return FAIL;
				*out_len = Rdata.len[0] * 0x100 + Rdata.len[1]-2;	
			}
			if((in_len%240)>0){
			  memcpy(tmpbuf,"\x00\x07\x80\x08\x00\x00\x02\x00\x00",0x09);
			  tmpbuf[1]=9+(in_len%240);
			  tmpbuf[6]=2+(in_len%240);
			  tmpbuf[7] = CMD_GET_DIGEST;					// 计算摘要值命令字
				if(alg == DIGEST_SHA1)
					tmpbuf[8] = 0x10;									// 计算摘要算法SHA1
				else if(alg == DIGEST_SHA256)
					tmpbuf[8] = 0x11;									// 计算摘要算法SHA256
				else if(alg == DIGEST_SM3)
					tmpbuf[8] = 0x12;									// 计算摘要算法SM3
				memcpy(tmpbuf+9,pIndata,(in_len%240));
				if(IIC_WriteData(tmpbuf, 9+(in_len%240),OnePack)!=SUCCESS_WORK)
					return FAIL;
				Rdata.data = pOutdata;
				if(IIC_ReadData(&Rdata)!= SUCCESS_WORK)
					return FAIL;
				sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
				if(sw_code != 0x9000)
					return FAIL;
				*out_len = Rdata.len[0] * 0x100 + Rdata.len[1]-2;
			}		
		}
		memcpy(tmpbuf,"\x00\x07\x80\x08\x00\x00\x02\x08\x00",0x09);
		if(alg == DIGEST_SHA1)
			tmpbuf[8] = 0x20;									// 计算摘要算法SHA1
		else if(alg == DIGEST_SHA256)
			tmpbuf[8] = 0x21;									// 计算摘要算法SHA256
		else if(alg == DIGEST_SM3)
			tmpbuf[8] = 0x22;									// 计算摘要算法SM3
		if(IIC_WriteData(tmpbuf, 9,OnePack)!=SUCCESS_WORK)
			return FAIL;
		Rdata.data = pOutdata;
		if(IIC_ReadData(&Rdata)!= SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
			return FAIL;
		*out_len = Rdata.len[0] * 0x100 + Rdata.len[1]-2;
		return SUCCESS_WORK;		
		
}

#ifdef USE_RSA

/***************************************************************************************************  
* Function： 						设置RSA 公私钥
* input parameter: 
*		uint8_t key_len:
*											 1024 : RSA1024 
*											 2048 : RSA2048  
*   uint8_t key_type：      00: RSA_PubKey
* 										 01: RSA_PriKey
*		uint8_t *keyparam:			 Key Data
* output parameter:
*		none
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t Set_RsaKey(uint16_t key_len,uint8_t key_type , uint8_t *keyparam)
{  
		uint8_t tmpbuf[10];
	  uint16_t mod=0;
		uint16_t sw_code;
    TIIC_RData_Struct Rdata;
	
	  Rdata.data = tmpbuf;
	
	  mod = key_len;
	  if(key_type == RSA_PubKey)
			key_len =(key_len/8)+4;  //e+N
    else if(key_type == RSA_PriKey)		
			key_len =((key_len/8)/2)*5; //P, Q, Dp, Dq, Qinv
		
		//1. 传入密钥数据

		if(Transmit_Data(tmpbuf,key_len ,keyparam ,0xF5 )!= SUCCESS_WORK)
				return FAIL;
		
		//2.启用所传入公钥密钥参数
		memcpy(tmpbuf,"\x00\x08\x80\x08\x00\x00\x03\x00\x00\x00",0x0A);
		if(key_type == RSA_PubKey)
			tmpbuf[7] = CMD_RSA_SET_PUB_KEY;										//设置RSA公钥
    else if(key_type == RSA_PriKey)		
			tmpbuf[7] = CMD_RSA_SET_PRI_KEY;										//设置RSA私钥	
		tmpbuf[8] = (uint8_t)(mod>>8);
		tmpbuf[9] = (uint8_t)(mod);
		if(IIC_WriteData(tmpbuf, 0x0A,OnePack)!=SUCCESS_WORK)
			return FAIL;
		if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;
		
		return SUCCESS_WORK;
}


/***************************************************************************************************  
* Function： 						RSA 加密或解密
* input parameter: 
*		uint8_t key_len:
*												1024 : RSA1024 
*											  2048 : RSA2048  
*   uint8_t mode							ENC/DEC
*		uint8_t padingmode       RSA_NO_PADDING / RSA_PKCS1_PADDING
*		uint16_t in_len:					明文数据长度
*		uint8_t* pIndata:				明文数据(RSA算法要求明文长度m必须0<m<n(模数)
* output parameter:
    uint16_t *outlen         输出数据长度
*		uint8_t* pOutdata：			输出数据
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/

uint8_t Rsa_EncAndDec(uint16_t key_len,uint8_t mode, uint8_t padingmode, uint16_t in_len, uint8_t* pIndata, uint16_t *out_len, uint8_t* pOutdata)
{
		uint8_t tmpbuf[11];
		uint16_t sw_code;
		uint16_t rLen;
		uint16_t offset, block_size;
	  uint16_t slen;
	  TIIC_RData_Struct Rdata;
	 
		if((padingmode == RSA_PKCS1_PADDING)&&(mode ==ENC))
			block_size = key_len/8 - 11;
	  else
			block_size = key_len/8; 
	
		*out_len = 0x00;
		offset= 0x00;
		while(in_len){
			
			if(in_len >block_size )
				slen = block_size;
			else 
				slen = in_len;
			//1.传数据 
			if(Transmit_Data(tmpbuf,slen ,pIndata+offset, 0xF5 )!= SUCCESS_WORK)
				return FAIL;
			in_len -= slen;
			offset += slen;

			
			//2.启动加密或解密
			memcpy(tmpbuf,"\x00\x09\x80\x08\x00\x00\x04\x00\x00\x00\x00",0x0B);
			if(mode == ENC)
				tmpbuf[7] = CMD_RSA_PUBKEY_ENC;	
			else
				tmpbuf[7] = CMD_RSA_PRIKEY_DEC;	
			tmpbuf[8] = (uint8_t)(key_len>>8);
			tmpbuf[9] = (uint8_t)(key_len);
			tmpbuf[10] = padingmode;
			if(IIC_WriteData(tmpbuf, 0x0B,OnePack)!=SUCCESS_WORK)
				return FAIL;
			Rdata.data = tmpbuf;
			if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];		
			if(sw_code != 0x9000)
				return FAIL;
			rLen = Rdata.data[0] * 0x100 + Rdata.data[1];
			
			//3.读出加密或解密数据
			
			if(Get_Data(tmpbuf,rLen , pOutdata+*out_len  ,0xF5) != SUCCESS_WORK){
				*out_len  = 0;
				return FAIL;
			}
			*out_len  += rLen;
	  }
		
		return  SUCCESS_WORK;
}



/***************************************************************************************************  
* Function： 						RSA 签名
* input parameter: 
*		uint8_t key_len:
*												0x00: RSA 1024;
*												0x01: RSA 2048;
*   uint8_t padingmode       RSA_NO_PADDING / RSA_PKCS1_PADDING
*		uint16_t in_len:					待签名数据长度
*		uint8_t* pIndata:				待签名数据
* output parameter:
*		uint8_t* pOutdata:				签名长度
*		uint8_t* pOutdata：			签名数据
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t Rsa_Sign(uint16_t key_len, uint8_t padingmode, uint16_t in_len, uint8_t* pIndata, uint16_t *out_len, uint8_t* pOutdata)
{
		uint8_t tmpbuf[11];
		uint16_t sw_code;
		uint16_t rLen;
		uint16_t offset, block_size;
	  uint16_t slen;
	  TIIC_RData_Struct Rdata;
	
	  
		if(padingmode == RSA_NO_PADDING)
			block_size = key_len/8;
		else 
			block_size = in_len;
	
		*out_len = 0x00;
		offset= 0x00;
		while(in_len){
			
			if(in_len >block_size )
				slen = block_size;
			else 
				slen = in_len;
			//1.传数据 
			if(Transmit_Data(tmpbuf,slen ,pIndata+offset, 0xF5 )!= SUCCESS_WORK)
				return FAIL;
			in_len -= slen;
			offset += slen;
			
			
			//2.启动签名
			memcpy(tmpbuf,"\x00\x09\x80\x08\x00\x00\x04\x00\x00\x00\x00",0x0B);
			tmpbuf[7] = CMD_RSA_PRIKEY_SIGN;	
			tmpbuf[8] = (uint8_t)(key_len>>8);
			tmpbuf[9] = (uint8_t)(key_len);
			tmpbuf[10] = padingmode;
			if(IIC_WriteData(tmpbuf, 0x0B,OnePack)!=SUCCESS_WORK)
				return FAIL;
			Rdata.data = tmpbuf;
			if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];		
			if(sw_code != 0x9000)
				return FAIL;
			rLen = Rdata.data[0] * 0x100 + Rdata.data[1];
			//3.读出加密或解密数据
			if(Get_Data(tmpbuf,rLen , pOutdata+*out_len  ,0xF5) != SUCCESS_WORK){
				*out_len  = 0;
				return FAIL;
			}
			*out_len  += rLen;
		}
	
		return SUCCESS_WORK;
}

/***************************************************************************************************  
* Function： 						RSA 公钥验签
* input parameter: 
*		uint8_t key_len:
*												1024: RSA1024;
*												2048: RSA2048;
*   uint8_t padingmode       RSA_NO_PADDING / RSA_PKCS1_PADDING
*		uint16_t msg_len:				原消息长度
*		uint8_t* pIndata:				原消息
*   uint16_t sign_len：      签名长度
*   uint8_t *sign:           签名值
* output parameter:
*		none
*	return:
*		SUCCESS_WORK: 						验签成功
*		FAIL								验签失败
****************************************************************************************************/

uint8_t Rsa_Verify(uint16_t key_len, uint8_t padingmode, uint16_t msg_len, uint8_t *msg, uint16_t sign_len , uint8_t *sign)
{
		uint8_t tmpbuf[11];
		uint16_t sw_code;
		uint16_t offset, block_size;
	  uint16_t slen;
	  TIIC_RData_Struct Rdata;
	
	  if(padingmode == RSA_NO_PADDING)
			block_size = key_len/8;
		else 
			block_size = msg_len;
		
		offset =0x00;
		while(msg_len){
			if(msg_len >block_size )
				slen = block_size;
			else 
				slen = msg_len;
			//1.传数据 			
			if(Transmit_Data(tmpbuf,key_len/8 ,sign+offset, 0xF5 )!= SUCCESS_WORK) //传输签名数据
				return FAIL;	
			if(Transmit_Data(tmpbuf,slen,msg+offset, 0xF5 )!= SUCCESS_WORK) //传输签名原数据
				return FAIL;
			msg_len -= slen;
			offset += slen;
			
			//2.启动验签
			memcpy(tmpbuf,"\x00\x09\x80\x08\x00\x00\x04\x00\x00\x00\x00",0x0B);
			tmpbuf[7] = CMD_RSA_PUBKEY_VERIFY;	
			tmpbuf[8] = (uint8_t)(key_len>>8);
			tmpbuf[9] = (uint8_t)(key_len);
			tmpbuf[10] = padingmode;
			if(IIC_WriteData(tmpbuf, 0x0B,OnePack)!=SUCCESS_WORK)
				return FAIL;
			Rdata.data = tmpbuf;
			if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];		
			if(sw_code != 0x9000)
				return FAIL;	
		}
		return SUCCESS_WORK;
}

/***************************************************************************************************  
* Function： 						生成RSA 密钥对
* input parameter: 
*		uint8_t key_len:
*												1024: RSA1024;
*												2048: RSA2048;
* output parameter:
*		uint8_t* pubkey          输出 e+N
*		uint8_t* prikey          输出 P+Q+DP+DQ+PQ
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t Rsa_Gen_KeyPair(uint16_t key_len, uint8_t* pubkey, uint8_t* prikey)
{
		uint8_t tmpbuf[10];
		uint8_t flag =0xFF;
	  uint16_t offset;
		uint16_t sw_code;
	  uint16_t pubkey_len;
	  uint16_t prikey_len;
		uint16_t len;
	  TIIC_RData_Struct Rdata;

		//1.生成密钥对
		tmpbuf[0] = 0x00;
		tmpbuf[1] = 0x08;				// 后续指令长度

		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);
		tmpbuf[5] = 0x00;
		tmpbuf[6] = 0x03;											// 后续指令长度
		tmpbuf[7] = CMD_RSA_GENKEYPAIR;				// 生成RSA密钥对
	 	tmpbuf[8] = (uint8_t)(key_len>>8);
		tmpbuf[9] = (uint8_t)(key_len);

		if(IIC_WriteData(tmpbuf, 0x0A,OnePack)!=SUCCESS_WORK)
			return FAIL;
		Rdata.data = tmpbuf;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];		
			if(sw_code != 0x9000)
				return FAIL;	
		}
		return SUCCESS_WORK;
}

/***************************************************************************************************  
* Function： 						生成RSA 密钥对
* input parameter: 
*		uint8_t key_len:
*												1024: RSA1024;
*												2048: RSA2048;
* output parameter:
*		uint8_t* pubkey          输出 e+N
*		uint8_t* prikey          输出 P+Q+DP+DQ+PQ
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t Rsa_Gen_KeyPair(uint16_t key_len, uint8_t* pubkey, uint8_t* prikey)
{
		uint8_t tmpbuf[10];
		uint8_t flag =0xFF;
	  uint16_t offset;
		uint16_t sw_code;
	  uint16_t pubkey_len;
	  uint16_t prikey_len;
		uint16_t len;
	  TIIC_RData_Struct Rdata;

		//1.生成密钥对
		tmpbuf[0] = 0x00;
		tmpbuf[1] = 0x08;				// 后续指令长度

		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);
		tmpbuf[5] = 0x00;
		tmpbuf[6] = 0x03;											// 后续指令长度
		tmpbuf[7] = CMD_RSA_GENKEYPAIR;				// 生成RSA密钥对
	 	tmpbuf[8] = (uint8_t)(key_len>>8);
		tmpbuf[9] = (uint8_t)(key_len);

		if(IIC_WriteData(tmpbuf, 0x0A,OnePack)!=SUCCESS_WORK)
			return FAIL;
		Rdata.data = tmpbuf;
		if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];		
		if(sw_code != 0x9000)
			return FAIL;

    pubkey_len = key_len/8 + 4;  // e+N
    prikey_len = (key_len/8/2)*5;	// P+Q+DP+DQ+PQ
    if((Rdata.data[0] * 0x100 + Rdata.data[1]) !=(pubkey_len+prikey_len) )
			return FAIL;
		
		//2.读公、私钥参数
		Rdata.data = pubkey;
		offset = 0x00;
		len = pubkey_len;
		flag = 0x01;
		while(len){
			memcpy(tmpbuf,"\x00\x07\x80\x08\x00\x00\x02\x00\x00",0x09);	
			if((len<=0xF5))
				tmpbuf[8] = len;
			else
				tmpbuf[8] = 0xF5;
			len -=tmpbuf[8];
			tmpbuf[7] = CMD_READ_OUTDATA;	
				
			if(IIC_WriteData(tmpbuf, 0x09,OnePack)!=SUCCESS_WORK)
				return FAIL;
			Rdata.data += offset ;
			if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];	
			if(sw_code != 0x9000)
				return FAIL;
			offset = Rdata.len[0]*0x100 + Rdata.len[1]-2;	
			if((flag == 0x01)&&(len == 0x00)){
				flag = 0xFF;
				Rdata.data = prikey;
				offset = 0x00;
				len = prikey_len;
			}
		}		
		
		return SUCCESS_WORK;
}
#endif
/********************************************************************************************************
* Function： 对称算法加解密函数
* input parameter: 
*		uint8_t alg:								加解密算法 
*													0： DES
*													1: 	AES
*													2： SM1
*													3:	SM4
*													4:	XXTEA
*		uint8_t mode:							模式
*													0： ECB
*													1：	CBC											
*		uint8_t op_type:						加解密
*													0：加密
*													1：解密
*		uint8_t in_len:						输入数据长度
*		uint8_t* pIn:							输入数据
* output parameter:
* 	uint8_t *LenOfOut：				输出数据长度
*		uint8_t *pOut：						输出数据
*	return:
*		SUCCESS_WORK: 							成功
*		FAIL									失败
********************************************************************************************************/
uint8_t  SAlgo_EncAndDec_Func(uint8_t alg,uint8_t op_type, uint8_t mode,  uint16_t in_len, uint8_t* pIn, uint16_t *out_len, uint8_t *pOut)
{
		uint8_t tmpbuf[10];
	  uint16_t offset;
		uint16_t sw_code;
		uint8_t  block_size;
	  TIIC_RData_Struct Rdata;
    
		if(alg == ENC_ALG_3DES)
				block_size = DES_BLOCK_SIZE;
		
		else if(alg == ENC_ALG_AES_128)
				block_size = AES_BLOCK_SIZE;
		
		else if(alg == ENC_ALG_SM1)	
				block_size = SM1_BLOCK_SIZE;
		
		else if(alg == ENC_ALG_SM4)
				block_size = SM4_BLOCK_SIZE;
		
		else if(alg == ENC_ALG_XXTEA)
				block_size = XXTEA_BLOCK_SIZE;
		else
				return FAIL;

		in_len = LKTPading(pIn, in_len, block_size);
		*out_len = 0x00;
		offset = 0x00;
		while(in_len){
			memcpy(tmpbuf,"\x00\x00\x80\x08\x00\x00\x00\x00\x00", 9); //80080000			
			if((in_len<=0xF0)){
			  tmpbuf[1] = (uint8_t)(in_len+7);
			  tmpbuf[6] = in_len+2;
			}
			else{
			  tmpbuf[1] = 0xF0+7;
			  tmpbuf[6] = 0xF0+2;
			}
			tmpbuf[8] = alg;										// 0：DES 	 1: 	AES   2： SM1   3:	SM4   4:	XXTEA
			if(mode == MODE_ECB)
				tmpbuf[8] &= ~(1<<4);							// ECB
			else if(mode == MODE_CBC)
				tmpbuf[8] |= (1<<4);							// CBC

			if(op_type == ENC)
				tmpbuf[8] &= ~(1<<5);							// 加密
			else if(op_type == DEC)
				tmpbuf[8] |= (1<<5);							// 解密
			
			if(in_len <= 0xF0)
				tmpbuf[8] |= (1<<6);							// 中间或尾
			else
				tmpbuf[8] &= ~(1<<6);							// 首帧
			
			in_len -=( tmpbuf[6]-2);
		  tmpbuf[7] = CMD_CRYPT_SYMMETRIC;			
			if(IIC_WriteData(tmpbuf, 0x09,FirstPack)!=SUCCESS_WORK)
				return FAIL;
			if(IIC_WriteData(pIn+offset,tmpbuf[6]-2,EndPack)!=SUCCESS_WORK)
				return FAIL;
			offset +=tmpbuf[6]-2;
			Rdata.data = pOut;
			Rdata.data +=*out_len;
			if(IIC_ReadData(&Rdata)!=SUCCESS_WORK)
				return FAIL;
			sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
			if(sw_code != 0x9000)
				return FAIL;	
			*out_len += Rdata.len[0]*0x100 + Rdata.len[1]-2;
	 }
	 return SUCCESS_WORK;
}

#ifdef USE_SM2
/***************************************************************************************************  
* Function： 						生成SM2密钥对
* input parameter: 
*	none
* output parameter:
*	uint8_t *key:				       pubkey +  prikey
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t SM2_Gen_KeyPair(uint8_t *key)
{
		uint8_t tmpbuf[9];
		uint16_t sw_code;
	  TIIC_RData_Struct Rdata;


		tmpbuf[0] = 0x00;
		tmpbuf[1] = 0x06;									//后续指令长度

		//固定指令头8008 0000
		memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);
		tmpbuf[6] = 1;										//后续指令数据长度
		tmpbuf[7] = CMD_SM2_GENKEYPAIR;		//生成SM2密钥对
    
		//发送I2C指令
	  
	  Rdata.data = key;
		if(IIC_SendApdu(0x08, tmpbuf, &Rdata)!= SUCCESS_WORK)
			return FAIL;
    sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;

		return SUCCESS_WORK;
}


/***************************************************************************************************  
* Function： 						设置SM2公钥
* input parameter: 
* uint8_t key_type:		  SM2_PubKey / SM2_PriKey	
*	uint8_t *key:				   public key or private key
* output parameter:
*			none
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t Set_Sm2Key(uint8_t key_type, uint8_t *key)
{
		uint8_t tmpbuf[8];
		uint16_t sw_code;
	  TIIC_RData_Struct Rdata;

	  memcpy(tmpbuf+2, CMD_FIX_HEAD, 4);

		tmpbuf[0] = 0x00;
	  if(key_type == SM2_PubKey){  
			tmpbuf[1] = 0x00;	
		  tmpbuf[6] = 0x41;	
			tmpbuf[7] = CMD_SM2_SET_PUB_KEY;	
		}
		else if(key_type == SM2_PriKey){
			tmpbuf[1] = 0x00;	
		  tmpbuf[6] = 0x21;	
			tmpbuf[7] = CMD_SM2_SET_PRI_KEY;
		}
		
		if(IIC_WriteData(tmpbuf, 0x08,FirstPack) != SUCCESS_WORK)
			return FAIL;
		if(IIC_WriteData(key, tmpbuf[6]-1,EndPack) != SUCCESS_WORK)
			return FAIL;
		Rdata.data = tmpbuf;
		if(IIC_ReadData(&Rdata) != SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
				return FAIL;

		return SUCCESS_WORK;
}

/***************************************************************************************************  
* Function： 					SM2 加密或解密
* input parameter: 
*	uint8_t mode             ENC / DEC
*	uint16_t in_len:					明文数据长度
*	uint8_t* pIndata:				明文数据
* output parameter:
* uint16_t *out_len：      密文长度
*	uint8_t* pOutdata：			输出数据
* uint8_t sm2_standard：   数据输出模式
*	return:
*		SUCCESS_WORK: 					成功
*		FAIL							失败
****************************************************************************************************/
uint8_t SM2_EncAndDec_Func(uint8_t mode,uint16_t in_len, uint8_t* pIndata,uint16_t *out_len, uint8_t* pOutdata ,uint8_t sm2_standard)
{
		uint8_t tmpbuf[9];
		uint16_t sw_code;
		uint16_t rLen;
	  TIIC_RData_Struct Rdata;
	
	  *out_len = 0x00;

    //1.传输数据
	  if(Transmit_Data(tmpbuf,in_len ,pIndata, 0xF5 )!= SUCCESS_WORK)
				return FAIL;	
		//2.启动加密或解密
		memcpy(tmpbuf,"\x00\x06\x80\x08\x00\x00\x01\x00",0x08);
		if(mode == ENC)
			tmpbuf[7] = CMD_SM2_ENC;
		else
			tmpbuf[7] = CMD_SM2_DEC;
		if(IIC_WriteData(tmpbuf, 0x08,OnePack)!=SUCCESS_WORK)
			return FAIL;
		Rdata.data = tmpbuf;
		if(IIC_ReadData(&Rdata)!= SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
			return FAIL;
		
		rLen = Rdata.data[0] * 0x100 + Rdata.data[1];
			
		//3.读出加密或解密数据
		*out_len  = rLen;
		if(Get_Data(tmpbuf,rLen , pOutdata ,0xF5) != SUCCESS_WORK){
			*out_len  = 0;
			return FAIL;
		}
		
		return  SUCCESS_WORK;
}

/***************************************************************************************************  
* Function： 						SM2私钥签名
* input parameter: 
*		uint16_t in_len:					待签名数据长度
*		uint8_t* pIndata:				待签名数据
* output parameter:
*		uint8_t* pOutdata：			签名数据（64字节）
*	return:
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t SM2_Sign(uint16_t in_len, uint8_t* pIndata, uint8_t* pOutdata)
{
	  uint8_t tmpbuf[9];
		uint16_t sw_code;
	  TIIC_RData_Struct Rdata;
	
	  //1.传数据 
	  if(Transmit_Data(tmpbuf,in_len ,pIndata, 0xF5 )!= SUCCESS_WORK)
				return FAIL;	

		//2.启动签名
		memcpy(tmpbuf,"\x00\x06\x80\x08\x00\x00\x01\x00",0x08);
		tmpbuf[7] = CMD_SM2_SIGN;	

		if(IIC_WriteData(tmpbuf, 0x08,OnePack)!=SUCCESS_WORK)
			return FAIL;
		Rdata.data = pOutdata;
		if(IIC_ReadData(&Rdata)!= SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
			return FAIL;
		return SUCCESS_WORK;
}

/***************************************************************************************************  
* Function： 						SM2公钥验签
* input parameter: 
*
*	uint16_t msg_len:		待验签数据长度
*	uint8_t *msg:				待验签数据
*  uint8_t *sign:      签名数据
* output parameter:
*		none
*	return
*		SUCCESS_WORK: 						成功
*		FAIL								失败
****************************************************************************************************/
uint8_t SM2_Verify(uint16_t msg_len, uint8_t *msg, uint8_t *sign)
{
		uint8_t tmpbuf[9];
		uint16_t sw_code;
	  TIIC_RData_Struct Rdata;
	
	 //1.传数据 			
		if(Transmit_Data(tmpbuf,0x40 ,sign, 0xF5 )!= SUCCESS_WORK) //传输签名数据
			return FAIL;	
		if(Transmit_Data(tmpbuf,msg_len,msg, 0xF5 )!= SUCCESS_WORK) //传输签名原数据
			return FAIL;	
	
		//2.启动验签
		memcpy(tmpbuf,"\x00\x06\x80\x08\x00\x00\x01\x00",0x08);
		tmpbuf[7] = CMD_SM2_VERIFY;	

		if(IIC_WriteData(tmpbuf, 0x08,OnePack)!=SUCCESS_WORK)
			return FAIL;
		Rdata.data = tmpbuf;
		if(IIC_ReadData(&Rdata)!= SUCCESS_WORK)
			return FAIL;
		sw_code = Rdata.sw[0] * 0x100 + Rdata.sw[1];
		if(sw_code != 0x9000)
			return FAIL;
		return SUCCESS_WORK;
}
#endif