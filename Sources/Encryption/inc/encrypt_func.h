#ifndef __ENCRYPT_FUNC_H
#define __ENCRYPT_FUNC_H

#include "macro.h"

#ifdef __cplusplus
	extern "C" {
#endif

extern uint32_t StrToHex(uint8_t *pbDest, uint8_t *pbSrc, uint32_t len);

extern uint8_t get_key_status(uint8_t* key_status);
extern uint8_t Set_Key(uint8_t mode,uint8_t* new_key, uint8_t* old_key);
extern uint8_t Set_Parameter(uint8_t offset,uint8_t data_len, uint8_t* in_data, uint8_t* pKey);
extern uint8_t Authenticate(uint8_t alg_sel, uint8_t* pKey, uint8_t* random_16_bytes);
extern uint8_t Read_Data_Protected(uint8_t* random_16_bytes, uint8_t* pKey,  uint8_t offset,uint8_t pdata_len, uint8_t* outdata);
extern uint8_t DataNot_Func(uint8_t LenOfIn, uint8_t *pIn, uint8_t *pOut);
extern uint8_t Code_Porting_Func(uint8_t LenOfIn, uint8_t *pIn, uint8_t *LenOfOut, uint8_t *pOut);
extern uint8_t GetID(uint8_t *LenOfOut, uint8_t *pOut);
extern uint8_t GetRandom(uint16_t LenOfRand, uint8_t *pOut);
extern uint8_t Get_Hash_Digest_Func(uint8_t alg, uint16_t in_len, uint8_t *pIndata, uint8_t *out_len, uint8_t *pOutdata);
extern uint8_t FlashErase(uint16_t addr, uint16_t size);
extern uint8_t FlashWrite(uint16_t addr, uint16_t in_len, uint8_t *pIndata);

//uint8_t Rsa_Gen_KeyPair(uint8_t key_len_sel, uint8_t key_type, void *pub_key, void* pri_key);
extern uint8_t Set_RsaKey(uint16_t key_len,uint8_t key_type , uint8_t *keyparam);
extern uint8_t Set_Rsa_PriKey(uint8_t key_len_sel, uint8_t* pri_key);
extern uint8_t Rsa_EncAndDec(uint16_t key_len,uint8_t mode, uint8_t padingmode, uint16_t in_len, uint8_t* pIndata, uint16_t *outlen, uint8_t* pOutdata);
extern uint8_t Rsa_Sign(uint16_t key_len, uint8_t padingmode, uint16_t in_len, uint8_t* pIndata, uint16_t *out_len, uint8_t* pOutdata);
extern uint8_t Rsa_Verify(uint16_t key_len, uint8_t padingmode, uint16_t msg_len, uint8_t *msg, uint16_t sign_len , uint8_t *sign);
extern uint8_t Rsa_Gen_KeyPair(uint16_t key_len, uint8_t* pubkey, uint8_t* prikey);
extern uint8_t SAlgo_EncAndDec_Func(uint8_t alg, uint8_t op_type,uint8_t mode,  uint16_t in_len, uint8_t* pIn, uint16_t *LenOfOut, uint8_t *pOut);

extern uint8_t SM2_Gen_KeyPair(uint8_t *key);
extern uint8_t Set_Sm2Key(uint8_t key_type, uint8_t *pub_key);
extern uint8_t SM2_EncAndDec_Func(uint8_t mode,uint16_t in_len, uint8_t* pIndata,uint16_t *out_len, uint8_t* pOutdata ,uint8_t sm2_standard);
extern uint8_t SM2_Sign(uint16_t in_len, uint8_t* pIndata, uint8_t* pOutdata);
extern uint8_t SM2_Verify(uint16_t msg_len, uint8_t *msg, uint8_t *sign);

extern void LKTPading(uint8_t *InOutBuf, uint16_t *Inoutlen, uint16_t Blocksize);

#ifdef __cplusplus
	}
#endif


#endif /* __ENCRYPT_FUNC_H */
