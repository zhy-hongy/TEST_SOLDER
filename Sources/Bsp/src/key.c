#include "key.h"
#include "solder.h"

// 按键状态和计数器
volatile sw_state_t key_states[KEY_MAX] = {sw_ideal};
volatile int16_t key_cnts[KEY_MAX][2] = {{0, 0}};
volatile int8_t key_ntfs[KEY_MAX], key_pressed_up_third_ntfs[KEY_MAX];
volatile int8_t key_pressed_up_cnt[KEY_MAX], key_pressed_time_gap_cnts = 0;

#ifdef THT_M120
// 按键配置数组
const key_config_t key_configs[KEY_MAX] = {
    {&key_states[KEY_ADD], key_cnts[KEY_ADD], &key_ntfs[KEY_ADD], KEY_ADD_PORT, KEY_ADD_PIN},
    {&key_states[KEY_SET], key_cnts[KEY_SET], &key_ntfs[KEY_SET], KEY_SET_PORT, KEY_SET_PIN},
    {&key_states[KEY_SUB], key_cnts[KEY_SUB], &key_ntfs[KEY_SUB], KEY_SUB_PORT, KEY_SUB_PIN},
    {&key_states[KEY_CH1], key_cnts[KEY_CH1], &key_ntfs[KEY_CH1], KEY_CH1_PORT, KEY_CH1_PIN},
    {&key_states[KEY_CH2], key_cnts[KEY_CH2], &key_ntfs[KEY_CH2], KEY_CH2_PORT, KEY_CH2_PIN},
    {&key_states[KEY_CH3], key_cnts[KEY_CH3], &key_ntfs[KEY_CH3], KEY_CH3_PORT, KEY_CH3_PIN}
};
#endif

#ifdef THT_L075
// 按键配置数组
const key_config_t key_configs[KEY_MAX] = {
    {&key_states[KEY_ADD], key_cnts[KEY_ADD], &key_ntfs[KEY_ADD], KEY_ADD_PORT, KEY_ADD_PIN},
    {&key_states[KEY_SET], key_cnts[KEY_SET], &key_ntfs[KEY_SET], KEY_SET_PORT, KEY_SET_PIN},
    {&key_states[KEY_SUB], key_cnts[KEY_SUB], &key_ntfs[KEY_SUB], KEY_SUB_PORT, KEY_SUB_PIN},
};
#endif

volatile int16_t key_update_cnts = 0;
volatile uint8_t gap_tmp_cnts = 0, single_cnts = 0, long_press_cnts = 0;

// 按键 GPIO 初始化
void key_gpio_init(void)
{
    GPIO_InitType GPIO_InitStructure;
    GPIO_InitStruct(&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
    GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;

    for (int i = 0; i < KEY_MAX; i++) {
        GPIO_InitStructure.Pin = key_configs[i].pin;
        GPIO_InitPeripheral(key_configs[i].port, &GPIO_InitStructure);
    }
}

// 按键状态检查函数，每 30ms 调用一次
int8_t sw_state_check(volatile sw_state_t* key_state, volatile int16_t cnts[], volatile int8_t *ntf)
{
    int8_t set_flag;
    if (cnts[0] > KEY_NOTIFY_CNTS) {  // 高电平
        set_flag = 0;
    } else if (cnts[1] > KEY_NOTIFY_CNTS) {  // 低电平
        set_flag = 1;
    } else {  // 无效电平
        set_flag = 2;
    }   
    cnts[0] = 0;
    cnts[1] = 0;

    switch (*key_state) {
        case sw_ideal:
            if (set_flag == 1) {
                *key_state = sw_trembling;//空闲时候按下按键进入消抖
            }
            break;

        case sw_trembling:
            if (set_flag == 0) {
                *key_state = sw_up;   //松开了
            } else if (set_flag == 1) {
                *key_state = sw_single_press;//还是按下，置通知位
                *ntf = 1;											
                gap_tmp_cnts = 0;
            }
            break;

        case sw_single_press:
            if (set_flag == 1) {
                gap_tmp_cnts++;
                if (gap_tmp_cnts > KEY_FRST_GAP_CNTS) {
                    *ntf = 2;// 连按通知
                    gap_tmp_cnts = 0;
                    single_cnts++;
                }
                if (single_cnts > KEY_SINGLE_CNTS) {
                    *key_state = sw_continu;
                    single_cnts = 0;
					long_press_cnts = 0;
                }
            } else {
                *key_state = sw_trembling;
            }
            break;

        case sw_continu:
            if (set_flag == 1) {
                gap_tmp_cnts++;
                if (gap_tmp_cnts >= KEY_CONT_GAP_CNTS) {
                    *ntf = 2;
                    gap_tmp_cnts = 0;
					long_press_cnts++;
					if(long_press_cnts == KEY_LONG_PRESS_CNTS){
						*ntf = 3;//长按通知
						long_press_cnts = 0;
					}
                }
            } else {
                *key_state = sw_trembling;
            }
            break;

        case sw_up:
            if (set_flag == 0) {
                *key_state = sw_ideal;
								*ntf = -1 ;
            } else {
                *key_state = sw_trembling;
            }
            break;

        default:
            *key_state = sw_ideal;
            break;
    }

    return 0;
}

// 按键轮询函数，每 10ms 调用一次
void key_poll(void)
{
    key_update_cnts++;  // 每 10ms 加 1

    for (int i = 0; i < KEY_MAX; i++) {
        if (GPIO_ReadInputDataBit(key_configs[i].port, key_configs[i].pin) == Bit_SET) {
            key_configs[i].cnts[0]++;   // 高电平
        } else {
            key_configs[i].cnts[1]++; // 低电平
        }
    }

    // 每 30ms 调用一次状态检查
    if (key_update_cnts >= KEY_NOTIFY_UPDATE_CNTS) {
        for (int i = 0; i < KEY_MAX; i++) {
            sw_state_check(key_configs[i].state, key_configs[i].cnts, key_configs[i].ntf);
						key_pressed_up_count_poll(i);
        }
        key_update_cnts = 0;
    }
}

// 检查按键是否被按下
bool is_key_pressed(key_num_t key)
{
    return key_states[key] == sw_single_press || key_states[key] == sw_continu;
}

// 检查两个按键是否同时被按下
bool is_double_key_pressed(key_num_t key1, key_num_t key2)
{
    return is_key_pressed(key1) && is_key_pressed(key2);
}

// 按键的多次 poll 方式
void key_pressed_up_count_poll(key_num_t key)
{
    key_pressed_time_gap_cnts++;
    if(key_pressed_time_gap_cnts > 30){
        key_pressed_time_gap_cnts = 0;			
        key_pressed_up_cnt[key] = 0;
         key_pressed_up_third_ntfs[key] = 0;
    }
    if(key_states[key] == sw_up){
        key_pressed_time_gap_cnts = 0;
        key_pressed_up_cnt[key]++;
        if(key_pressed_up_cnt[key] >= 3){
            key_pressed_up_third_ntfs[key] = 1;
            key_pressed_up_cnt[key] = 3;
        }	
    }
}

bool is_key_pressed_up_third(key_num_t key)
{
		if(key_pressed_up_third_ntfs[key] > 0){
				key_pressed_up_third_ntfs[key] = 0;
				return 1;
		}else{
				return 0;
		}
		
}

//清除3连击计数和状态
void clear_key_third_flag(key_num_t key)
{
        key_pressed_up_cnt[key] = 0;
        key_pressed_up_third_ntfs[key] = 0;
}
// 获取按键通知标志
int8_t get_key_notify(key_num_t key)
{
    return key_ntfs[key];
}

// 清除按键通知标志
void clear_key_notify(key_num_t key)
{
    key_ntfs[key] = 0;
}