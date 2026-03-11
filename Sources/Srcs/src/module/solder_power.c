#include "solder_power.h"
#include "solder_error.h"
void power_cal(volatile solder_ch_state_t *ch, uint8_t heat_flag)
{
	if (heat_flag > 0 && ch->power_sum < POWER_SUM_LEN)
	{
		ch->power_sum++;
	}

	if (ch->power_rec_buf[ch->power_buf_index] > 0 && ch->power_sum > 0)
	{
		ch->power_sum--;
	}

	//	DELAY_EXECUTE(500, {
	//		sprintf(debug_string_out_0, "power_sum: %2d\r\n", ch->power_sum);
	//		log_info(debug_string_out_0); });
	ch->power_rec_buf[ch->power_buf_index] = heat_flag;
	ch->power_buf_index++;

	if (ch->power_buf_index >= POWER_SUM_LEN)
	{
		ch->power_buf_index = 0;
	}

	if (ch->power_sum > 0)
	{
		if (ch->power_sum < 2)
		{
			ch->power_gear = 1;
		}
		else
		{
			ch->power_gear = ch->power_sum / 3;
		}

		if (ch->power_gear > 8)
		{
			ch->power_gear = 8;
		}
	}
	else
	{
		ch->power_gear = 0;
	}

	if (ch->power_sum > 30)
	{
		ch->power_exceed_cnts++;
	}
	else
	{
		ch->power_exceed_gap_cnts++;
	}

	if (ch->power_exceed_cnts > 10000 || ch->power_exceed_gap_cnts > 10000)
	{
		if (ch->power_exceed_gap_cnts < 600)
		{
			// 2 min Í£Ö¹¼ÓÈÈ
			ch->ch_err_flag |= ERROR_POWER_EXCEED;
		}

		ch->power_exceed_cnts = 0;
		ch->power_exceed_gap_cnts = 0;
	}
}
