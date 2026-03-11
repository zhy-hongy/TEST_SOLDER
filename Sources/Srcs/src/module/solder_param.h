#ifndef __SOLDER_PARAM_H__
#define __SOLDER_PARAM_H__

#include "solder.h"
extern solder_param_info_t solder_param_info[512];
extern uint8_t solder_param[512];

void save_min_temp_settings(volatile solder_ch_state_t *ch);
void solder_param_save(void *params, param_name_t name);
void solder_param_get(void *params, param_name_t name);
void solder_param_write_to_array(void *params, param_name_t name);
void solder_param_get_from_array(void *params, param_name_t name);
void solder_parameters_update_to_rom();

void solder_param_write_to_rom(param_name_t name);
void solder_param_load_from_rom(param_name_t name);

#endif /* __SOLDER_PARAM_H__ */