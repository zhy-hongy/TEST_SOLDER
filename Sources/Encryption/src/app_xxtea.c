#include "string.h"
#include "macro.h"

#ifdef USE_XXTEA

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))
#define ADDR_OF_XXTEA_KEY 0x0900   
	
/**************************************************************************************************************

函数名称： void xxtea_encode(unsigned long *v, int n,   unsigned long   key[4]) 
函数功能：XXTEA加密

输入参数：
		 *v: 输入/输出数据
     n : 输入数据长度
    key: 密钥
		
输出参数：
		 *v：存放输出数据的内容

***************************************************************************************************************/

 void xxtea_encode(unsigned long *v, int n, unsigned long key[4]) {
      unsigned long  y, z, sum;
      unsigned long  p, rounds, e;
    
     rounds = 6 + 52/n;
      sum = 0;
      z = v[n-1];
      do {
        sum += DELTA;
        e = (sum >> 2) & 3;
        for (p=0; p<n-1; p++) {
          y = v[p+1]; 
          z = v[p] += MX;
        }
        y = v[0];
        z = v[n-1] += MX;
      } while (--rounds);
     
 }

/**************************************************************************************************************

函数名称： void xxtea_decode(unsigned long *v, int n, unsigned long const key[4])
函数功能：XXTEA解密

输入参数：
		 *v: 输入/输出数据
     n : 输入数据长度
    key: 密钥
		
输出参数：
		 *v：存放输出数据的内容

***************************************************************************************************************/
void xxtea_decode(unsigned long *v, int n, unsigned long const key[4]) {
		unsigned long y, z, sum;
		unsigned long p, rounds, e;
				
		rounds = 6 + 52/n;
		sum = rounds*DELTA;
		y = v[0];
		do {
				e = (sum >> 2) & 3;
				for (p=n-1; p>0; p--) {
					z = v[p-1];
					y = v[p] -= MX;
				}
				z = v[n-1];
				y = v[0] -= MX;
				sum -= DELTA;
		} while (--rounds);
}

	
/**************************************************************************************************************

函数名称： void XXTea_ENC(unsigned char *pIn, int n, unsigned char  *pKey,unsigned char *pOut)
函数功能：XXTEA加密

输入参数：
		 pIn: 输入数据
	     n : 输入数据长度
     pKey: 密钥
		
输出参数：
		pOut:   存放输出数据的内容
	*LenOfOut: 芯片输出的数据域数据长度
***************************************************************************************************************/	
uint8_t XXTea_ENC(const uint8_t *pKey,int n,uint8_t *pIn,   uint8_t *LenOfOut,uint8_t *pOut)
{
		xxtea_encode((unsigned long *)pIn, n/4, (unsigned long *)pKey);
		memcpy(pOut, pIn, n);

		*LenOfOut = n;
		return SUCCESS_WORK;
}
/**************************************************************************************************************

函数名称：void XXTea_DEC(unsigned char *pIn, int n, unsigned char  *pKey,unsigned char *pOut) 
函数功能：XXTEA解密

输入参数：
		  pIn: 输入数据
	     n : 输入数据长度
     pKey: 密钥
		
输出参数：
	   pOut:   存放输出数据的内容
	*LenOfOut: 芯片输出的数据域数据长度
***************************************************************************************************************/ 
uint8_t XXTea_DEC(const uint8_t *pKey,int n,uint8_t *pIn,   uint8_t *LenOfOut,uint8_t *pOut)
{
		xxtea_decode((unsigned long *)pIn, n/4, (unsigned long *)pKey);
		memcpy(pOut, pIn, n);
		*LenOfOut = n;
		return SUCCESS_WORK;
}
#endif
