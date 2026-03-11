#include "n32g43x.h"
#include "bsp_all.h"
#include "debug.h"
#include "solder.h"

static uint8_t i=0, num;


int main(void)
{
	#if DEBUG_STAGE_TEST == 1
	ch_logic_debug.stage_flag = 1;
	#endif
	solder_dev_init();
	screen_display();

	while(1){
		i++;
		if(i > 20){
			screen_display();
			solder_parameters_update_to_rom();
			i = 0;
		}

		#if DEBUG_ON == 1
		debug_task();
		uart_usb_send_dma();
		#endif		
		delay_ms(10);
	}
	return 0;
}
