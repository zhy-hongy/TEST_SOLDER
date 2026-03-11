#include "solder_tool.h"

void check_solder_channel_tool_type_from_res(volatile solder_ch_state_t *ch,
											 volatile vol_cur_data_t *vol_cur)
{
	uint8_t cur_test_i;

	if (ch->type_test_avg_res < RES_210_MIN)
	{
		ch->tool_type_3 = notool;
	}
	else if (ch->type_test_avg_res > RES_210_MIN &&
			 ch->type_test_avg_res < RES_245_MAX)
	{
		ch->tool_type_3 = tool_210_245;
	}
	else if (ch->type_test_avg_res > RES_115_MIN &&
			 ch->type_test_avg_res < RES_115_MAX)
	{
		ch->tool_type_3 = tool_115;
	}
	else if (ch->type_test_avg_res > RES_115_MAX)
	{
		ch->tool_type_3 = notool; // err
	}
}