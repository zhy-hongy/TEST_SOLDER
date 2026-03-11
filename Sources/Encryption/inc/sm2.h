#ifndef _SM2_H_
#define _SM2_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef struct Struct_SM2_PubKey
{
		// As the name, point to the coordinate 'x'
		unsigned char *x;

		// As the name, point to the coordinate 'y'
		unsigned char *y;

} SM2_PUBLICKEY_CTX, SM2POINT;

typedef struct Struct_SM2_PriKey
{
		unsigned char *d;

} SM2_PRIVATEKEY_CTX;


typedef struct Struct_BigInteger
{
		// Point to the data
		unsigned char *pd;

		// The length of above data
		unsigned long len;

} BIGINTEGER, SM2_PLAINTEXT, IDINFO;


typedef struct Struct_SM2_Ciphertext
{
		// C1 -- It's one point of the current elliptic curve.
		SM2POINT *p;

		// C2 -- The true ciphertext.
		unsigned char *c;

		// C3 -- Reserve for algorithm which has hash value,
		// as lucky, the SM2 is, so you need it.
		unsigned char *h;

		// The length of C2(ciphertext).
		unsigned long clen;

} SM2_CIPHERTEXT;

typedef struct Struct_SM2_Signature
{
		// As the name
		unsigned char *r;

		// As the name
		unsigned char *s;

} SM2_SIGNATURE;



#define SM2_CIPHER_P_X_LEN			32
#define SM2_CIPHER_P_Y_LEN			32
#define SM2_CIPHER_P_LEN				64
#define SM2_CIPHER_H_LEN				32

#define SM2_PUBKEY_X_LEN				32
#define SM2_PUBKEY_Y_LEN				32
#define SM2_PUBKEY_LEN					64
#define SM2_PRIVATEKEY_LEN			32
#define SM2_SIGNATURE_LEN				64

#ifdef __cplusplus
}
#endif

#endif
