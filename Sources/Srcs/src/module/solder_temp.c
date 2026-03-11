#include "solder_temp.h"

void ch_temp_show_filt_once(volatile solder_ch_state_t *ch)
{
	if (ch->ch_stage == ch_working)
	{
		ch->temp_show = 0.96f * ch->temp_show + 0.04f * ch->controller.temp_ested;
	}
	else
	{
		ch->temp_show = 0.99f * ch->temp_show + 0.01f * ch->controller.temp_ested;
	}

	if (ch->ch_stage == ch_working || (ch->ch_stage == ch_sleep && ch->min_temp_enable_flag == 1))
		if (((ch->temp_show - ch->temp_target > 0.0f) &&
			 (ch->temp_show - ch->temp_target < 20.0f)) ||
			((ch->temp_target - ch->temp_show < 3.0f) &&
			 (ch->temp_target - ch->temp_show > 0.0f)))
		{
			ch->temp_show = ch->temp_target;
		}
}
