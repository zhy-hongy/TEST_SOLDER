#ifndef APP_XXTEA_H
#define APP_XXTEA_H

#ifdef __cplusplus
	extern "C" {
#endif

uint8_t XXTea_ENC(uint8_t *pIn, int n, uint8_t *pKey, uint8_t *pOut, uint8_t *LenOfOut);
uint8_t XXTea_DEC(uint8_t *pIn, int n, uint8_t *pKey, uint8_t *pOut, uint8_t *LenOfOut);

#ifdef __cplusplus
extern "C" {
#endif


#endif
