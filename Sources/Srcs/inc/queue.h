#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdint.h>
#include <string.h>


#define QUEUE_TX_SIZE_MAX				4096
#define QUEUE_RX_SIZE_MAX				256

typedef struct
{
    uint32_t head;        //数组下标，指向队头
    uint32_t tail;        //数组下标，指向队尾
    uint32_t size;        //队列缓存长度（初始化时赋值）
		uint32_t data_len;    //队列中数据的长度（初始化时赋值）
    volatile void *buffer;      //队列缓存数组（初始化时赋值）
		uint16_t byte_size;		// 1 2 4
} circle_queue_t;

typedef enum
{
	QUEUE_OK = 0,       //队列正常
	QUEUE_ERROR,        //队列错误
	QUEUE_OVERLOAD,     //队列已满
	QUEUE_EMPTY         //队列已空
} queue_status_t;

/**
****************************************************************
* @brief   初始化（创建）队列，每个队列须先执行该函数才能使用
* @param   queue, 队列变量指针
* @param   buffer, 队列缓存区地址
* @param   size, 队列缓存区长度
* @return 
****************************************************************
*/
void queue_init(volatile circle_queue_t *queue, uint8_t *buffer, uint32_t size, uint16_t byte_size);

/**
****************************************************************
* @brief   压入数据到队列中
* @param   queue, 队列变量指针
* @param   data, 待压入队列的数据
* @return  压入队列是否成功
****************************************************************
*/
queue_status_t queue_push(volatile circle_queue_t *queue, void *pdata);

/**
****************************************************************
* @brief   从队列中弹出数据
* @param   queue, 队列变量指针
* @param   pdata, 待弹出队列的数据缓存地址
* @return  弹出队列是否成功
****************************************************************
*/
queue_status_t queue_pop(volatile circle_queue_t *queue, void *pdata);

/**
****************************************************************
* @brief   压入一组数据到队列中
* @param   queue, 队列变量指针
* @param   pArray, 待压入队列的数组地址
* @param   len, 待压入队列的元素个数
* @return  实际压入到队列的元素个数
****************************************************************
*/
queue_status_t queue_push_array(volatile circle_queue_t *queue, uint8_t *pArray, uint32_t len);

/**
****************************************************************
* @brief   从队列中弹出一组数据
* @param   queue, 队列变量指针
* @param   pArray, 待弹出队列的数据缓存地址
* @param   len, 待弹出队列的数据的最大长度
* @return  实际弹出数据的数量
****************************************************************
*/
queue_status_t queue_pop_array(volatile circle_queue_t *queue, uint8_t *pArray, uint32_t len);

/**
****************************************************************
* @brief   从队列中弹出所有数据
* @param   queue, 队列变量指针
* @param   pArray, 待弹出队列的数据缓存地址
* @return  实际弹出数据的数量
****************************************************************
*/
uint32_t queue_pop_all(volatile circle_queue_t *queue, uint8_t *pArray);

/**
****************************************************************
* @brief   从队列中发送数据出去，由硬件DMA完成后，在DMA中调用此方法
* @param   queue, 队列变量指针
* @return  实际弹出数据的数量
****************************************************************
*/
uint32_t queue_pop_all_throw_hardware(volatile circle_queue_t *queue);


/**
****************************************************************
* @brief   获取队列中数据的个数
* @param   queue, 队列变量指针
* @return  队列中数据的个数
****************************************************************
*/
uint32_t queue_data_length(volatile circle_queue_t *queue);


queue_status_t queue_read_data(volatile circle_queue_t *queue, void* data, uint16_t pos);
#endif
