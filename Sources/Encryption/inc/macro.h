#ifndef __ENC_MACRO_H
#define __ENC_MACRO_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "stdint.h"
	
#define NULL 0

#define	DEBUG_KEY_SEL
#define	CONFIG_DIR
#define USE_DES
#undef USE_AES
#undef USE_SM1
#undef USE_SM4
#define USE_XXTEA
#undef USE_SHA256
#undef USE_RSA
#undef USE_SM2
#undef USE_SM3
#undef USE_FLOAT_TEST

#define		FAIL													0
#define		SUCCESS_WORK											1

/*对比认证采用的算法*/
#define		AU_ALG_3DES												0x00		// 对比认证采用算法： DES
#define		AU_ALG_AES_128										0x01		// 对比认证采用算法： AES 128
#define		AU_ALG_SM4												0x02		// 对比认证采用算法： SM4
#define		AU_ALG_XXTEA											0x03		// 对比认证采用算法： XXTEA
#define		AU_ALG_SHA256											0x04		// 对比认证采用算法： SHA256
#define		AU_ALG_SM3												0x05		// 对比认证采用算法： SM3

/*加解密采用的算法*/
#define		ENC_ALG_3DES											0x00		// 加密算法： DES
#define		ENC_ALG_AES_128										0x01		// 加密算法： AES 128
#define		ENC_ALG_SM1												0x02		// 加密算法： SM1
#define		ENC_ALG_SM4												0x03		// 加密算法： SM4
#define		ENC_ALG_XXTEA											0x04		// 加密算法： XXTEA

/*摘要采用的算法*/
#define		DIGEST_SHA1												0x00
#define		DIGEST_SHA256											0x01
#define		DIGEST_SM3												0x02

#define		MODE_ECB													0x01		//ECB模式
#define		MODE_CBC													0x02		//CBC模式


/* 加密芯片有调试、应用两种模式; 默认情况下工作于调试模式; 通过将key长度最高位置位, 可使加密芯片工作于应用模式; */
#define		DEBUG_MODE												0				//调试模式: 明文设置key
#define		SECURITY_MODE									    1				//安全模式: 密文更新key
#define		KEY_FIXED_LEN									    32
#define		READ_KEY_FIXED_LEN								16

/* 加密IC通信协议CMD Code */
#define		CMD_DATA_NOT											0x01		//数据取反命令
#define		CMD_SET_CONFIG_DATA								0x02		//设置关键参数命令：包含对比认证、读取保护数据所需key、Iv、保护数据等数据
#define		CMD_AUTHENTICATE									0x03		//对比认证命令
#define		CMD_READ_DATA_PROTECTED						0x04		//读取保护数据命令
#define		CMD_CODE_TRANSPLATE								0x05		//调用移植代码命令

#define		CMD_GET_CHIP_ID										0x06		//读取加密芯片ID命令
#define		CMD_GET_RANDOM										0x07		//读取随机数命令
#define		CMD_GET_DIGEST										0x08		//获取数据SHA256摘要值命令
#define		CMD_FLASH_ERASE										0x09		//擦除flash命令
#define		CMD_FLASH_WRITE										0x0A		//写flash命令
#define		CMD_GET_KEY_STATUS								0x0C		//获取key状态命令: 0：调试装填/1：应用状态

#define		CMD_CRYPT_SYMMETRIC								0x0D		//对称算法加解密命令

#define		CMD_SM2_SET_PUB_KEY								0x0E		//设置SM2公钥
#define		CMD_SM2_SET_PRI_KEY								0x0F		//设置SM2私钥
#define		CMD_SM2_ENC												0x10		//RSA加密命令
#define		CMD_SM2_DEC												0x11		//RSA解密命令
#define		CMD_SM2_SIGN											0x12		//RSA签名命令
#define		CMD_SM2_VERIFY										0x13		//RSA验签命令
#define		CMD_SM2_GENKEYPAIR								0x14		//生成RSA密钥对


#define		CMD_RSA_SET_PUB_KEY								0x15		//RSA设置公钥确认
#define		CMD_RSA_SET_PRI_KEY								0x16		//RSA设置私钥确认
#define		CMD_RSA_PUBKEY_ENC								0x17		//RSA公钥加密
#define		CMD_RSA_PRIKEY_DEC								0x18		//RSA私钥解密
#define		CMD_RSA_PRIKEY_SIGN								0x19		//RSA私钥签名
#define		CMD_RSA_PUBKEY_VERIFY							0x1A		//RSA公钥验签
#define		CMD_RSA_GENKEYPAIR								0x1B		//生成RSA密钥对
#define		CMD_SET_DATA									    0x1C		//RSA设置密钥、RSA、SM2加解密传输数据
#define		CMD_READ_OUTDATA									0x1D		//RSA读输出数据
#define		CMD_RSA_SETDATA_START							0x1E		//RSA设置数据

/* 设置关键参数命令操作码 */
//#define		SET_PARAMETER_KEY				        0x00		//设置数据保护key
#define		SET_KEY_DATA		          				0x01		//设置对比认证key
#define		SET_PARAMETER_DATA								0x02		//设置要保护的数据

#define		ENC												    		0
#define		DEC														    1

#define		CHALLENGE_LEN								      16			//对比认证的随机数长度
#define		IV_MAX_LEN												16			//Iv最大长度
#define		DES_IV_LEN												8				//Iv长度
#define		SM4_IV_LEN												16			//SM4 Iv长度
#define		SM1_IV_LEN												16			//SM1 Iv长度
#define		AES_IV_LEN												16			//AES Iv长度
#define		SHA256_OUT_DATA_LEN								32

#define		KEY_LEN_DES												8
#define		KEY_LEN_3DES											16
#define		KEY_LEN_AES128										16
#define		KEY_LEN_AES192										24
#define		KEY_LEN_AES256										32
#define		KEY_LEN_SM4												16
#define		KEY_LEN_SM1												16
#define		KEY_LEN_XXTEA											16
#define		KEY_LEN_MAX												32

#define		DES_BLOCK_SIZE   				0x08
#define		AES_BLOCK_SIZE   				0x10
#define		SM1_BLOCK_SIZE   				0x10
#define		SM4_BLOCK_SIZE   				0x10
#define		XXTEA_BLOCK_SIZE   			0x10
#define		OP_ENC 									0x00
#define		OP_DEC									0x01

#define	SM2_PubKey							0x00
#define	SM2_PriKey							0x01

#define SM2_PUBKEY_LEN          0x40
#define SM2_PRIVATEKEY_LEN      0x20
#define SM2_OLD_STANDARD        0x00
#define SM2_NEW_STANDARD        0x01

#define   Authenticate_En
#define   ParameterProtect_En
#define   Set_DebugKey

extern void RCC_Configuration(void);

#ifdef __cplusplus
}
#endif

#endif
