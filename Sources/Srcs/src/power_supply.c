#include "power_supply.h"

uint8_t i;

void power_supply_detection_once(power_supply_t *power_info)
{
	power_info->power_test_flag = 1;
	power_info->power_test_zero_cnts = 0;
	while(power_info->power_test_zero_cnts < POWER_INFO_TEST_ZERO_CNTS){
		
	}
	
}

void power_supply_detection_update_in_zero(power_supply_t *power_info, float* vol)
{
	for(i = 0; i < 10; i++){
		power_info->voltage_average = vol[i];
	}
	for(i = 0; i < 10; i++){
		
	}
}
