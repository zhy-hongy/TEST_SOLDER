#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g43x.h"

#ifdef THT_M120
/***����***
	��ఴ��
PB1			��
PB0			���� SET
PA7			��

PB4			һ��	CH1
PB5			����	CH2
PA6			����	CH3
***************/
	
#define KEY_ADD_PIN					  GPIO_PIN_1
#define KEY_ADD_PORT				  GPIOB
#define KEY_SET_PIN						GPIO_PIN_0
#define KEY_SET_PORT					GPIOB
#define KEY_SUB_PIN						GPIO_PIN_7
#define KEY_SUB_PORT					GPIOA

#define KEY_CH1_PIN						GPIO_PIN_4
#define KEY_CH1_PORT					GPIOB
#define KEY_CH2_PIN						GPIO_PIN_5
#define KEY_CH2_PORT					GPIOB
#define KEY_CH3_PIN						GPIO_PIN_6
#define KEY_CH3_PORT					GPIOA

#define KEY_SINGLE_CNTS					6
#endif

#ifdef THT_L075
#define KEY_ADD_PIN					  	GPIO_PIN_6
#define KEY_ADD_PORT				  	GPIOA
#define KEY_SET_PIN							GPIO_PIN_7
#define KEY_SET_PORT						GPIOA
#define KEY_SUB_PIN							GPIO_PIN_0
#define KEY_SUB_PORT						GPIOB

#define KEY_SINGLE_CNTS						3
#endif

#define KEY_CHECK_CYCLE_TIME			10		// 10ms  *

#define KEY_NOTIFY_UPDATE_CNTS	 	3
#define KEY_NOTIFY_CNTS						2

#define KEY_FRST_GAP_CNTS					8			// 300ms
#define KEY_CONT_GAP_CNTS					2 		// 200ms 1s 5��
#define KEY_LONG_PRESS_CNTS				10


// ����״̬ö��
typedef enum {
    sw_ideal,
    sw_trembling,
    sw_single_press,
    sw_continu,
    sw_up
} sw_state_t;

// �������ö��
typedef enum {
    KEY_ADD = 0,
    KEY_SET,
    KEY_SUB,
#ifdef THT_M120
    KEY_CH1,
    KEY_CH2,
    KEY_CH3,
#endif
    KEY_MAX
} key_num_t;

// �������ýṹ��
typedef struct {
    volatile 		sw_state_t* state;
    volatile 		int16_t* cnts;
    volatile 		int8_t* ntf;
    GPIO_Module* 	port;
    uint16_t 		pin;
} key_config_t;

// ��������
void key_gpio_init(void);
void key_poll(void);

bool is_key_pressed(key_num_t key);
bool is_double_key_pressed(key_num_t key1, key_num_t key2);

void key_pressed_up_count_poll(key_num_t key);
bool is_key_pressed_up_third(key_num_t key);
void clear_key_third_flag(key_num_t key);

int8_t get_key_notify(key_num_t key);
void clear_key_notify(key_num_t key);

#ifdef __cplusplus
}
#endif

#endif


