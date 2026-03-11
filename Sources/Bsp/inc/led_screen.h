#ifndef LED_SCREEN_H
#define LED_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"

#ifdef THT_M120
	
#define LED_STB_PORT		GPIOB	
#define LED_STB_PIN 		GPIO_PIN_13
	
//#define LCD_LED_PORT		GPIOB
//#define LCD_LED_PIN 		GPIO_PIN_12
#endif
	
#ifdef THT_L075
#define LED_STB_PORT		GPIOB	
#define LED_STB_PIN 		GPIO_PIN_13
	
//#define LCD_LED_PORT		GPIOB
//#define LCD_LED_PIN 		GPIO_PIN_12
#endif
	
#define LED_DATA_PORT		GPIOB	
#define LED_DATA_PIN 		GPIO_PIN_14
	
#define LED_CLK_PORT		GPIOB
#define LED_CLK_PIN 		GPIO_PIN_15

//#define LCD_LED_SET() 		GPIO_WriteBit(LCD_LED_PORT, LCD_LED_PIN, Bit_SET)
//#define LCD_LED_RESET() 	GPIO_WriteBit(LCD_LED_PORT, LCD_LED_PIN, Bit_RESET)

#define LED_DATA_SET()		GPIO_WriteBit(LED_DATA_PORT, LED_DATA_PIN, Bit_SET)
#define LED_DATA_RESET() 	GPIO_WriteBit(LED_DATA_PORT, LED_DATA_PIN, Bit_RESET)

#define LED_STB_SET() 		GPIO_WriteBit(LED_STB_PORT, LED_STB_PIN, Bit_SET)
#define LED_STB_RESET() 	GPIO_WriteBit(LED_STB_PORT, LED_STB_PIN, Bit_RESET)

#define LED_CLK_SET() 		GPIO_WriteBit(LED_CLK_PORT, LED_CLK_PIN, Bit_SET)
#define LED_CLK_RESET() 	GPIO_WriteBit(LED_CLK_PORT, LED_CLK_PIN, Bit_RESET)


#define DISP_MODE_CMD			0x00

#define SET_DATA_ADD_INC_CMD	0x40
#define SET_DATA_ADD_FIX_CMD	0x44

#define SET_ADDR_CMD			0xC0

#define DISP_CONTROL_ON			0x8F
#define DISP_CONTROL_OFF		0x87


void lcd_init(void);

void display_num(uint8_t num);

void display_fix_flag(uint8_t flag);
void display_error_flag(uint8_t flag);
void display_no_tool_flag(uint8_t flag);
void display_unitC_flag(uint8_t flag);
void display_unitF_flag(uint8_t flag);
void display_unitH_flag(uint8_t flag);

void display_power_gear(uint8_t gear);
void display_temp_set( int16_t temp_set);
void display_temp_show(int16_t temp_show);

void lcd_scan(void);
	
	
#ifdef __cplusplus
}
#endif

#endif