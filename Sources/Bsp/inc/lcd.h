#ifndef _LCD_DRV_H_
#define _LCD_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_all.h"

#ifdef THT_M120
	
#define LCD_CS_PORT			GPIOB	
#define LCD_CS_PIN 			GPIO_PIN_13
	
#define LCD_LED_PORT		GPIOB
#define LCD_LED_PIN 		GPIO_PIN_12
#endif
	
#ifdef THT_L75
#define LCD_CS_PORT			GPIOB	
#define LCD_CS_PIN 			GPIO_PIN_13
	
#define LCD_LED_PORT		GPIOB
#define LCD_LED_PIN 		GPIO_PIN_12
#endif
	
#define LCD_DATA_PORT		GPIOB	
#define LCD_DATA_PIN 		GPIO_PIN_14
	
#define LCD_WR_PORT			GPIOB
#define LCD_WR_PIN 			GPIO_PIN_15

#define LCD_LED_SET() 		GPIO_WriteBit(LCD_LED_PORT, LCD_LED_PIN, Bit_SET)
#define LCD_LED_RESET() 	GPIO_WriteBit(LCD_LED_PORT, LCD_LED_PIN, Bit_RESET)

#define LCD_DATA_SET()		GPIO_WriteBit(LCD_DATA_PORT, LCD_DATA_PIN, Bit_SET)
#define LCD_DATA_RESET() 	GPIO_WriteBit(LCD_DATA_PORT, LCD_DATA_PIN, Bit_RESET)

#define LCD_CS_SET() 		GPIO_WriteBit(LCD_CS_PORT, LCD_CS_PIN, Bit_SET)
#define LCD_CS_RESET() 		GPIO_WriteBit(LCD_CS_PORT, LCD_CS_PIN, Bit_RESET)

#define LCD_WR_SET() 		GPIO_WriteBit(LCD_WR_PORT, LCD_WR_PIN, Bit_SET)
#define LCD_WR_RESET() 		GPIO_WriteBit(LCD_WR_PORT, LCD_WR_PIN, Bit_RESET)


#define WRITE_CMD_CODE 		0x80  // 发送命令
#define WRITE_DATA_CODE 	0xA0  // 发送数据
#define READ_DATA_CODE 		0xC0  // 读取数据

#define HT1621_SYS_EN 		0x01  // 打开系统振荡器
#define HT1621_RC256 		0x18  // 内部时钟
#define HT1621_BIAS 		0x29  // 1/3duty 4com LCD偏置
#define HT1621_LCD_OFF 		0x02 	// 关闭LCD偏压
#define HT1621_LCD_ON 		0x03  // 打开LCD偏压

#define HT1621_SYS_DIS 		0x00  // 关闭系统振荡器和LCD偏压发生器
#define HT1621_XTAL 		0x14  // 外部接时钟
#define HT1621_WDT_DIS 		0x05  // 关闭看门狗
#define HT1621_TONE_ON 		0x09  // 打开声音输出
#define HT1621_TONE_OFF 	0x08 	// 关闭声音输出


void lcd_init(void);

void display_num(uint8_t num);

void display_fix_flag(uint8_t flag);
void display_error_flag(uint8_t flag);
void display_no_tool_flag(uint8_t flag);
void display_unitC_flag(uint8_t flag);
void display_unitF_flag(uint8_t flag);

void display_power_gear(uint8_t gear);
void display_temp_set( int16_t temp_set);
void display_temp_show(int16_t temp_show);

void lcd_scan(void);

#ifdef __cplusplus
}
#endif

#endif
