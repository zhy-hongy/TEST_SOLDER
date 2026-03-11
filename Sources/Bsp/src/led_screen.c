#ifdef LED_SCREEN

#include "bsp_all.h"

#ifdef __cplusplus 
extern "C" {
#endif

/* LCD段码屏数据缓存 */
volatile uint8_t array_RAM_LCD[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	
volatile uint8_t array_RAM_LED[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/* 数码管数据表 */
//const uint8_t array_num[10] = {
//    0xF5, 0x60, 0xBC, 0x9E, 0x4E, // 0, 1, 2, 3, 4
//    0xD6, 0xF6, 0x8A, 0xFE, 0xDE  // 5, 6, 7, 8, 9
//};
	

const uint8_t array_num[10] = {
    0xFA, 0x60, 0xBC, 0xF4, 0x66, // 0, 1, 2, 3, 4
    0xD6, 0xDE, 0x70, 0xFE, 0xF6  // 5, 6, 7, 8, 9
};

const uint8_t array_power[10] = {
		0x00, 0x08, 0x0C, 0x0E, 0x0F, 
		0x1F, 0x3F, 0x7F, 0xFF, 0x00
};
//const uint8_t array_num[10] = {
//    0x00, 0x01, 0x02, 0x04, 0x08, // 0, 1, 2, 3, 4
//    0x10, 0x20, 0x40, 0x80, 0x0F  // 5, 6, 7, 8, 9
//};		
/* 数码管数据表1 */
//const uint8_t array_num1[10] = {
//    0xAF, 0xA0, 0xCB, 0xE9, 0xE4, // 0, 1, 2, 3, 4
//    0x6D, 0x6F, 0xA8, 0xEF, 0xED  // 5, 6, 7, 8, 9
//};

/* 数码管数据表2 */
//const uint8_t array_num[10] = {
//    0x5F, 0x06, 0x6B, 0x2F, 0x36, // 0, 1, 2, 3, 4
//    0x3D, 0x7D, 0x07, 0x7F, 0x3F  // 5, 6, 7, 8, 9
//};
//


#ifdef THT_L075

#endif

const uint8_t init_num = 0x00;

static void lcd_gpio_init(void)
{
    GPIO_InitType GPIO_InitStructure;

	GPIO_InitStruct(&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pull	= GPIO_No_Pull;
  
    // PA01 -- LCD_DATA 数据总线
    GPIO_InitStructure.Pin 			= LED_DATA_PIN;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;    
    GPIO_InitPeripheral(LED_DATA_PORT, &GPIO_InitStructure);

    // PA04 -- LCD_CS 片选信号
    GPIO_InitStructure.Pin 			= LED_STB_PIN;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;    
    GPIO_InitPeripheral(LED_STB_PORT, &GPIO_InitStructure);

    // PA06 -- LCD_WR 写时钟线
    GPIO_InitStructure.Pin 			= LED_CLK_PIN;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;    
    GPIO_InitPeripheral(LED_CLK_PORT, &GPIO_InitStructure);
}

/**
 * @brief 写一位数据到AIP1624
 * @param data 数据
 * @param num 个数
 */
static void WriteByteToAIP1624MSB(uint8_t data, uint8_t num)
{
	uint8_t i, tmp = data;
	
    for ( i = 0; i < num; i++)
    {
        LED_CLK_RESET(); // WR置低

        if ((tmp & 0x80) > 0) // 如果当前bit为1，DATA置高
        {
            LED_DATA_SET();
        }
        else // 如果当前bit为0，DATA置低
        {
            LED_DATA_RESET();
        }
		delay_us(5);
		
        LED_CLK_SET(); // WR置高
		
		delay_us(5);
        tmp <<= 1;   // 移位
    }
}

/**
 * @brief 写一位数据到AIP1624
 * @param data 数据
 * @param num 个数
 */
static void WriteByteToAIP1624LSB(uint8_t data, uint8_t num)
{
	uint8_t i, tmp = data;
	
    for ( i = 0; i < num; i++)
    {
        LED_CLK_RESET(); // WR置低

        if ((tmp & 0x01) > 0) // 如果当前bit为1，DATA置高
        {
            LED_DATA_SET();
        }
        else // 如果当前bit为0，DATA置低
        {
            LED_DATA_RESET();
        }
		delay_us(15);
        LED_CLK_SET(); // WR置高
		delay_us(15);
        tmp >>= 1;   // 移位
    }
}

/**
 * @brief 发送命令到AIP1624
 * @param cmdVal 命令
 */
void WriteCmdToAIP1624(uint8_t cmdVal)
{
    LED_STB_RESET(); // CS置低
    WriteByteToAIP1624LSB(cmdVal, 8);
    LED_STB_SET(); // CS置高
	delay_us(20);
}

/**
 * @brief 发送数据到AIP1624
 * @param addr 地址
 * @param dataVal 数据
 */
void WriteDataToAIP1624(uint8_t addr, uint8_t dataVal)
{
	uint8_t add_cmd;
	
	LED_STB_RESET(); // CS置低
	add_cmd = SET_ADDR_CMD | (0x0F & addr);
	WriteByteToAIP1624LSB(add_cmd, 8); 
    WriteByteToAIP1624LSB(dataVal, 8);
    LED_STB_SET(); // CS置高
	delay_us(20);
}

/**
 * @brief 发送N个数据到AIP1624
 * @param addr 地址
 * @param dataVal 数据
 * @param cnt 数据总数
 */
void WriteNDataToAIP1624(uint8_t addr, uint8_t *dataVal, uint8_t cnt)
{
	uint8_t i, add_cmd;
	uint8_t *p = dataVal;
	
	LED_STB_RESET(); // CS置低
	add_cmd = SET_ADDR_CMD | (0x0F & addr);   
    WriteByteToAIP1624LSB(add_cmd, 8); 
	
    for ( i = 0; i < cnt; i++)
    {
        WriteByteToAIP1624LSB(*p, 8);
		p++;
    }
    LED_STB_SET(); // CS置高
}

static void AIP1624_init(void)
{
	delay_ms(10);
	WriteCmdToAIP1624(DISP_MODE_CMD);	
	WriteCmdToAIP1624(SET_DATA_ADD_INC_CMD);
	WriteNDataToAIP1624(0, (uint8_t*)array_RAM_LED, 8);
	WriteCmdToAIP1624(DISP_CONTROL_OFF);
}

void array_convert(uint8_t* array_old, uint8_t* array_new)
{
	uint8_t i, tmp1, tmp2;
	
	for(i = 0; i < 4; i++){
		tmp1 = 0x01 << i;
		tmp2 = 0x10 << i;
		array_new[0+2*i] = 	( (array_old[0]&tmp1)>>(i) ) 		| 
							(((array_old[0]&tmp2)>>(4+i))<<1) 	|
							(((array_old[1]&tmp1)>>(i)  )<<2) 	|
							(((array_old[1]&tmp2)>>(4+i))<<3) 	|
							(((array_old[2]&tmp1)>>(i)	)<<4) 	|
							(((array_old[2]&tmp2)>>(4+i))<<5) 	|
							(((array_old[3]&tmp1)>>(i)	)<<6) 	| 
							(((array_old[3]&tmp2)>>(4+i))<<7);
		
		array_new[1+2*i] = 	( (array_old[4]&tmp1)>>(i) ) 		| 
							(((array_old[4]&tmp2)>>(4+i))<<1) 	|
							(((array_old[5]&tmp1)>>(i)  )<<2) 	|
							(((array_old[5]&tmp2)>>(4+i))<<3) 	|
							(((array_old[6]&tmp1)>>(i)	)<<4) 	|
							(((array_old[6]&tmp2)>>(4+i))<<5) 	|
							(((array_old[7]&tmp1)>>(i)	)<<6) 	| 
							(((array_old[7]&tmp2)>>(4+i))<<7);
	}
	
	#ifdef THT_M120
	#endif
	#ifdef THT_L075
		array_new[3] = (array_new[3] & 0xFE) | ((array_old[4] & 0x08) >> 3);
		array_new[7] = (array_new[7] & 0xFE) | ((array_old[4] & 0x02) >> 1);
	#endif
}

/**
 * @brief 将缓存区的数据写入AIP1624
 * @param
 */
void lcd_scan()
{
	uint8_t i;
	array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
	WriteCmdToAIP1624(SET_DATA_ADD_INC_CMD);
	WriteNDataToAIP1624(0, (uint8_t*)array_RAM_LED, 8);
	WriteCmdToAIP1624(DISP_CONTROL_ON);
}

/**
 * @brief LCD硬件初始化
 * @param
 */
void lcd_init(void)
{
	uint16_t i,j,tmp;
	
    lcd_gpio_init();
    AIP1624_init();
	
//	for(i = 0; i < 8; i++){
//		for(j = 0; j < 8; j++){
//			tmp = 0x01 << j;
//			array_RAM_LED[i] = tmp;
//			lcd_scan();
//			delay_ms(1000);
//		}
//		delay_ms(2000);
//	}

	
	

//	
//	display_no_tool_flag(1); 
//	array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
//	lcd_scan();
//	delay_ms(500);
//	
//	display_unitC_flag(1);
//	array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
//	lcd_scan();
//	delay_ms(500);
//	
//	display_unitF_flag(1);
//	array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
//	lcd_scan();
//	delay_ms(500);
//	
//	for(i = 1; i < 9; i++){
//		display_power_gear(i*10);
//		array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
//		lcd_scan();
//		delay_ms(500);
//	}
//	for(i = 0; i < 9; i++){
//		display_temp_set(111*i);
//		array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
//		lcd_scan();
//		delay_ms(500);
//	}	
//	for(i = 0; i < 9; i++){
//		display_temp_show(111*i);
//		array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
//		lcd_scan();
//		delay_ms(500);
//	}
	
//	display_fix_flag(1); 
//	array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
//	lcd_scan();
//	delay_ms(500);

//	display_error_flag(1);
//	array_convert((uint8_t*)array_RAM_LCD, (uint8_t*)array_RAM_LED);
//	lcd_scan();
//	delay_ms(500);
	
}




// Temp标  Set标  Power标
void display_fix_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM_LCD[1] |= 0x01;
		}else{
			array_RAM_LCD[1] &= 0xFE;
		}
}

void display_error_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM_LCD[0] |= 0x01;
		}else{
			array_RAM_LCD[0] &= 0xFE;
		}
}

void display_no_tool_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM_LCD[4] |= 0x01;
		}else{
			array_RAM_LCD[4] &= 0xFE;
		}
}

void display_unitC_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM_LCD[5] |= 0x01;
		}else{
			array_RAM_LCD[5] &= 0xFE;
		}
}
void display_unitF_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM_LCD[6] |= 0x01;
		}else{
			array_RAM_LCD[6] &= 0xFE;
		}
}
void display_unitH_flag(uint8_t flag)
{
		if(flag > 0){
			array_RAM_LCD[2] |= 0x01;
		}else{
			array_RAM_LCD[2] &= 0xFE;
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
		array_RAM_LCD[3] = array_power[gear];
}

void display_temp_set(int16_t temp_set)
{
		uint16_t num;
	
		if(temp_set < -100){
			array_RAM_LCD[0] = 0x04;
			array_RAM_LCD[1] = 0x04;
			array_RAM_LCD[2] = 0x04;
		}else{
			num = temp_set % 1000 / 100;
			array_RAM_LCD[0] = array_num[num];
			num = temp_set % 100  / 10;
			array_RAM_LCD[1] = array_num[num];
			num = temp_set % 10;
			array_RAM_LCD[2] = array_num[num];
		}
}

void display_temp_show(int16_t temp_show)
{
		uint8_t num;
	
		if(temp_show < -100){
			array_RAM_LCD[4] = 0x04;
			array_RAM_LCD[5] = 0x04;
			array_RAM_LCD[6] = 0x04;
		}else{
			num = temp_show % 1000 / 100;
			array_RAM_LCD[4] = array_num[num];
			num = temp_show % 100  / 10;
			array_RAM_LCD[5] = array_num[num];
			num = temp_show % 10;
			array_RAM_LCD[6] = array_num[num];	
		}
	
}

#ifdef __cplusplus
}
#endif

#endif