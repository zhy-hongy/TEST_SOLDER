#ifdef LCD_SCREEN

#include "lcd.h"

/* LCD段码屏数据缓存 */
volatile uint8_t array_RAM[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* 数码管数据表 */
//const uint8_t array_num[10] = {
//    0xF5, 0x60, 0xBC, 0x9E, 0x4E, // 0, 1, 2, 3, 4
//    0xD6, 0xF6, 0x8A, 0xFE, 0xDE  // 5, 6, 7, 8, 9
//};
	
//#ifdef THT_M120
const uint8_t array_num[10] = {
	
		0x1F, 0x3F, 0x7F, 0xFF, 0x00
};
//const uint8_t array_num[10] = {
//    0x00, 0x01, 0x02, 0x04, 0x08, // 0, 1, 2, 3, 4
//    0x10, 0x20, 0x40, 0x80, 0x0F  // 5, 6, 7, 8, 9
//};		
/* 数码管数据表1 */
const uint8_t array_num1[11] = {
    0xAF, 0xA0, 0xCB, 0xE9, 0xE4, // 0, 1, 2, 3, 4
    0x6D, 0x6F, 0xA8, 0xEF, 0xED  // 5, 6, 7, 8, 9
};

/* 数码管数据表2 */
const uint8_t array_num2[11] = {
    0x5F, 0x06, 0x6B, 0x2F, 0x36, // 0, 1, 2, 3, 4
    0x3D, 0x7D, 0x07, 0x7F, 0x3F  // 5, 6, 7, 8, 9
};
//


#ifdef THT_L075

#endif

const uint8_t init_num = 0x00;

static void lcd_gpio_init(void)
{
    GPIO_InitType GPIO_InitStructure;

	GPIO_InitStruct(&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pull	= GPIO_No_Pull;
    // PA00 -- LCD_LEDCtrl
    GPIO_InitStructure.Pin 			= LCD_LED_PIN;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;    
    GPIO_InitPeripheral(LCD_LED_PORT, &GPIO_InitStructure);

    // PA01 -- LCD_DATA 数据总线
    GPIO_InitStructure.Pin 			= LCD_DATA_PIN;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;    
    GPIO_InitPeripheral(LCD_DATA_PORT, &GPIO_InitStructure);

    // PA04 -- LCD_CS 片选信号
    GPIO_InitStructure.Pin 			= LCD_CS_PIN;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;    
    GPIO_InitPeripheral(LCD_CS_PORT, &GPIO_InitStructure);

    // PA06 -- LCD_WR 写时钟线
    GPIO_InitStructure.Pin 			= LCD_WR_PIN;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;    
    GPIO_InitPeripheral(LCD_WR_PORT, &GPIO_InitStructure);
}

/**
 * @brief 写一位数据到HT1621
 * @param data 数据
 * @param num 个数
 */
static void WriteBitToHT1621MSB(uint8_t data, uint8_t num)
{
		uint8_t i, tmp = data;
    for ( i = 0; i < num; i++)
    {
        LCD_WR_RESET(); // WR置低

        if ((tmp & 0x80) > 0) // 如果当前bit为1，DATA置高
        {
            LCD_DATA_SET();
        }
        else // 如果当前bit为0，DATA置低
        {
            LCD_DATA_RESET();
        }
				delay_us(5);
        LCD_WR_SET(); // WR置高
				delay_us(5);
        tmp <<= 1;   // 移位
    }
}

/**
 * @brief 写一位数据到HT1621
 * @param data 数据
 * @param num 个数
 */
static void WriteBitToHT1621LSB(uint8_t data, uint8_t num)
{
		uint8_t i, tmp = data;
    for ( i = 0; i < num; i++)
    {
        LCD_WR_RESET(); // WR置低

        if ((tmp & 0x01) > 0) // 如果当前bit为1，DATA置高
        {
            LCD_DATA_SET();
        }
        else // 如果当前bit为0，DATA置低
        {
            LCD_DATA_RESET();
        }
				delay_us(15);
        LCD_WR_SET(); // WR置高
				delay_us(15);
        tmp >>= 1;   // 移位
    }
}

/**
 * @brief 发送命令到HT1621
 * @param cmdVal 命令
 */
void WriteCmdToHT1621(uint8_t cmdVal)
{
    LCD_CS_RESET(); // CS置低
    WriteBitToHT1621MSB(WRITE_CMD_CODE, 3);
    WriteBitToHT1621MSB(cmdVal, 8);
    WriteBitToHT1621MSB(0, 1);
    LCD_CS_SET(); // CS置高
}

/**
 * @brief 发送数据到HT1621
 * @param addr 地址
 * @param dataVal 数据
 */
//void WriteDataToHT1621(uint8_t addr, uint8_t dataVal)
//{
//    LCD_CS_RESET(); // CS置低
//    WriteBitToHT1621(WRITE_DATA_CODE, 3);
//    WriteBitToHT1621(addr << 2, 6);
//    WriteBitToHT1621(dataVal << 4, 4);
//    LCD_CS_SET(); // CS置高
//}

/**
 * @brief 发送N个数据到HT1621
 * @param addr 地址
 * @param dataVal 数据
 * @param cnt 数据总数
 */
void WriteNDataToHT1621(uint8_t addr, uint8_t *dataVal, uint8_t cnt)
{
		uint8_t i;
	
    LCD_CS_RESET(); // CS置低
    WriteBitToHT1621MSB(WRITE_DATA_CODE, 3);
    WriteBitToHT1621MSB(addr << 2, 6);
    for ( i = 0; i < cnt; i++)
    {
        WriteBitToHT1621LSB(*dataVal++, 8);
    }
    LCD_CS_SET(); // CS置高
}

static void HT1621_init(void)
{
    LCD_CS_SET();
    LCD_WR_SET();
    LCD_DATA_SET();
    WriteCmdToHT1621(HT1621_LCD_OFF);
    WriteCmdToHT1621(HT1621_SYS_EN);
    WriteCmdToHT1621(HT1621_RC256);
    WriteCmdToHT1621(HT1621_BIAS);
    WriteCmdToHT1621(HT1621_LCD_ON);
}

/**
 * @brief LCD硬件初始化
 * @param
 */
void lcd_init(void)
{
    lcd_gpio_init();
    HT1621_init();
	LCD_LED_SET();
	display_fix_flag(1);
	lcd_scan();
}

/**
 * @brief 将缓存区的数据写入HT1621
 * @param
 */
void lcd_scan(void)
{
    WriteNDataToHT1621(9, (void*)array_RAM, 8);
//		WriteNDataToHT1621(7, (void*)array_RAM+1, 1);
//		WriteNDataToHT1621(9, (void*)array_RAM+2, 1);
//		WriteNDataToHT1621(11, (void*)array_RAM+3, 1);
//		WriteNDataToHT1621(13, (void*)array_RAM+4, 1);
//		WriteNDataToHT1621(15, (void*)array_RAM+5, 1);
//		WriteNDataToHT1621(17, (void*)array_RAM+6, 1);
//		WriteNDataToHT1621(19, (void*)array_RAM+7, 1);
//		WriteNDataToHT1621(21, (void*)array_RAM+8, 1);
}



// Temp标  Set标  Power标
void display_fix_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM[1] |= 0x01;
		}else{
			array_RAM[1] &= 0xFE;
		}
}

void display_error_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM[0] |= 0x01;
		}else{
			array_RAM[0] &= 0xFE;
		}
}

void display_no_tool_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM[4] |= 0x01;
		}else{
			array_RAM[4] &= 0xFE;
		}
}

void display_unitC_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM[5] |= 0x01;
		}else{
			array_RAM[5] &= 0xFE;
		}
}
void display_unitF_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM[6] |= 0x01;
		}else{
			array_RAM[6] &= 0xFE;
		}
}

// 最大值 80
void display_power_gear(uint8_t gear)
{
		uint8_t tmp;
//		tmp = gear;
//		tmp = 1 << (tmp-1);
		if(gear > 8)
			gear = 8;
		array_RAM[3] = array_power[gear];
}

void display_temp_set(int16_t temp_set)
{
		uint16_t num;
	
		if(temp_set < -100){
			array_RAM[0] = 0x04;
			array_RAM[1] = 0x04;
			array_RAM[2] = 0x04;
		}else{
			num = temp_set % 1000 / 100;
			array_RAM[0] = array_num[num];
			num = temp_set % 100  / 10;
			array_RAM[1] = array_num[num];
			num = temp_set % 10;
			array_RAM[2] = array_num[num];
		}
}

void display_temp_show(int16_t temp_show)
{
		uint8_t num;
	
		if(temp_show < -100){
			array_RAM[4] = 0x04;
			array_RAM[5] = 0x04;
			array_RAM[6] = 0x04;
		}else{
			num = temp_show % 1000 / 100;
			array_RAM[4] = array_num[num];
			num = temp_show % 100  / 10;
			array_RAM[5] = array_num[num];
			num = temp_show % 10;
			array_RAM[6] = array_num[num];	
		}
	
}

#endif