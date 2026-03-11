#ifndef _RSA_H_
#define _RSA_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Struct_RSAPrivateKeyCRT
{

	/* Prime1 */
	unsigned char *p;
	/* Prime2 */
	unsigned char *q;
	/* Prime1 Exponent */
	unsigned char *dP;
	/* Prime2 Exponent */
	unsigned char *dQ;
	/* Coefficient */
	unsigned char *qInv;

} RSA_PRIVATEKEY_CRT, *PRSA_PRIVATEKEY_CRT;

typedef struct Struct_RSAPrivateKeySTD
{
		/* Prime1 */
		unsigned char *d;
		/* Prime2 */
		unsigned char *n;
} RSA_PRIVATEKEY_STD, *PRSA_PRIVATEKEY_STD;

#define RSAPRIVATEKEYCTX	void

typedef struct Struct_RSAPublicKey
{
		/* Public Exponent */
		unsigned char *e;
		/* modulus */
		unsigned char *n;
} RSA_PUBLICKEY_CTX, *PRSA_PUBLICKEY_CTX;

// 加解密模式
#define RSA_NO_PADDING	  0				// 前补0x00
#define RSA_PKCS1_PADDING	1				// PKCS11补齐
#define RSA_PADING	RSA_LKT_PADING

/* type of RSA private key operation */
#define 	RSA_KEY_STD_TYPE									0x00
#define 	RSA_KEY_CRT_TYPE									0x08

#define		RSA_PubKey										    0x00
#define		RSA_PriKey											  0x01

#define		RSA1024													  1024
#define		RSA2048													  2048

#define 	RSA_E_LEN													0x04

#define 	RSA1024_PUB_KEY_LEN								0x80
#define 	RSA1024_PRIKEY_STD_LEN						0x80
#define 	RSA1024_PRIKEY_CRT_LEN						0x40

#define 	RSA2048_PUB_KEY_LEN								0x100
#define 	RSA2048_PRIKEY_STD_LEN						0x100
#define 	RSA2048_PRIKEY_CRT_LEN						0x80

#define		RSA_ONE_PACKET_LEN								0x80

#ifdef __cplusplus
}
#endif

#endif
