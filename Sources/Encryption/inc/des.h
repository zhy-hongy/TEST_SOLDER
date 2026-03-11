#ifndef __DES_h
#define __DES_h

#ifdef __cplusplus
	extern "C" {
#endif

void encrypt(uint8_t *input, uint8_t *key, uint8_t *output);
void decrypt(uint8_t *input, uint8_t *key, uint8_t *output);
uint8_t LK_DesEncrypt(uint8_t Mode, uint8_t *Iv, uint8_t key_len, uint8_t* pKey, uint8_t indata_len, uint8_t* pIn, uint8_t* outdata_len, uint8_t* pOut);
uint8_t LK_DesDecrypt(uint8_t Mode, uint8_t *Iv, uint8_t key_len, uint8_t* pKey, uint8_t indata_len, uint8_t* pIn, uint8_t* outdata_len, uint8_t* pOut);

#ifdef __cplusplus
	}
#endif


#endif
