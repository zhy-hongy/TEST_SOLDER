#ifndef __SOLDER_ERROR_H__
#define __SOLDER_ERROR_H__
#include "solder.h"

/*********** CHANNEL ERROR *************/	
//#define CH_ERR_NONE							0x00		// 0
//#define CH_ERR_TEMP_ERROR						0x01
//#define CH_ERR_CUR_ERROR						0x02
//#define CH_ERR_REST_SHORT                     0x04
//#define CH_ERR_REST_CUTOFF                    0x08

#define ERROR_NONE					0x00
#define ERROR_TEMP_ERROR			0x01
#define ERROR_TEMP_TOOL_ERROR		0x10

#define ERROR_VOL_ERROR				0x02
#define ERROR_CUR_SHORT_ERROR		0x04
#define ERROR_CUR_CUT_ERROR			0x08

#define CH_ERR_PCB_TEMP_ERROR		0x20	//°åÎÂ¹ý¸ß´íÎó
#define CH_ERR_ILEAK_ERROR			0x40	//µçÁ÷Ð¹Â©´íÎó

#define ERROR_POWER_EXCEED			0x80

#define TEMP_ERROR_CONT_MAX			100


void heat_temperature_error(volatile solder_ch_state_t *ch);
void check_cur_vol_res_error(volatile solder_ch_state_t *ch, volatile vol_cur_data_t *vol_cur);

#endif /* __SOLDER_ERROR_H__ */