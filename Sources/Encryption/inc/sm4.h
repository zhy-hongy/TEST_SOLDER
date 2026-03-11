#ifndef _SM4_H_
#define _SM4_H_


#ifdef __cplusplus
extern "C" {
#endif

uint8_t LK_SM4Encrypt(uint8_t Mode, uint8_t *pIv, uint8_t key_len, uint8_t* pKey, uint8_t indata_len, uint8_t* pIn, uint8_t* outdata_len, uint8_t* pOut);
uint8_t LK_SM4Decrypt(uint8_t Mode, uint8_t *pIv, uint8_t key_len, uint8_t* pKey, uint8_t indata_len, uint8_t* pIn, uint8_t* poutdata_len, uint8_t* pOut);

#ifdef __cplusplus
}
#endif


#endif
