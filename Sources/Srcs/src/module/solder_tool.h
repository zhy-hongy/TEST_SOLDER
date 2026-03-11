#ifndef __SOLDER_TOOL_H__
#define __SOLDER_TOOL_H__

#include "solder.h"

void check_solder_channel_tool_type_from_res(volatile solder_ch_state_t *ch,
											 volatile vol_cur_data_t *vol_cur);

#endif /* __SOLDER_TOOL_H__ */