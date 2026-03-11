#ifndef __POWER_SUPPLY_H__
#define __POWER_SUPPLY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#define 	POWER_INFO_TEST_ZERO_CNTS		100	

typedef struct power_supply_params{
	float 			frequency;
	float			voltage_average;
	float			voltage_peak;
	
	uint8_t			power_test_zero_cnts;
	uint8_t 		power_test_flag;
} power_supply_t;
	
#ifdef __cplusplus
}
#endif

#endif
