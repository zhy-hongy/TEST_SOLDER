#include <stdint.h>
#include "queue.h"

/**
****************************************************************
* @brief   初始化（创建）队列，每个队列须先执行该函数才能使用
* @param   queue, 队列变量指针
* @param   buffer, 队列缓存区地址
* @param   size, 队列缓存区长度
* @return 
****************************************************************
*/
void queue_init(volatile circle_queue_t *queue, uint8_t *buffer, uint32_t size, uint16_t byte_size)
{
    queue->buffer = buffer;
    queue->size = size;
    queue->head = 0;
    queue->tail = 0;
		queue->data_len = 0;
		queue->byte_size = byte_size;
}

/**
****************************************************************
* @brief   压入数据到队列中
* @param   queue, 队列变量指针
* @param   data, 待压入队列的数据
* @return  压入队列是否成功
****************************************************************
*/
queue_status_t queue_push(volatile circle_queue_t *queue, void *pdata)
{
    uint32_t index = (queue->tail + 1) % queue->size;

    if( index == queue->head || (queue->data_len + 1) == queue->size )
    {
        return QUEUE_OVERLOAD;
    }
    memcpy((void*)queue->buffer + queue->tail*queue->byte_size, pdata, queue->byte_size);
    queue->tail = index;
		queue->data_len++;
    return QUEUE_OK;
}

/**
****************************************************************
* @brief   从队列中弹出数据
* @param   queue, 队列变量指针
* @param   pdata, 待弹出队列的数据缓存地址
* @return  弹出队列是否成功
****************************************************************
*/
queue_status_t queue_pop(volatile circle_queue_t *queue, void *pdata)
{
    if(queue->head == queue->tail || queue->data_len == 0)
    {
        return QUEUE_EMPTY;
    }

    memcpy(pdata, (void*)queue->buffer + queue->head*queue->byte_size, queue->byte_size);
    queue->head = (queue->head + 1) % queue->size;
		queue->data_len--;
		
    return QUEUE_OK;
}

/**
****************************************************************
* @brief   压入一组数据到队列中
* @param   queue, 队列变量指针
* @param   pArray, 待压入队列的数组地址
* @param   len, 待压入队列的元素个数
* @return  实际压入到队列的元素个数
****************************************************************
*/
queue_status_t queue_push_array(volatile circle_queue_t *queue, uint8_t *pArray, uint32_t len)
{
    uint16_t tmp_len = 0;
		uint16_t tmp_byte_size, push_byte_size = len * queue->byte_size;

		if( len == 0 ){
			return QUEUE_OK;
		}
		if( queue->data_len + len > queue->size ){
			return QUEUE_OVERLOAD;
		}
				
		if( queue->head <= queue->tail && queue->tail + len >= queue->size){
			tmp_len = queue->size - queue->tail;
			tmp_byte_size = tmp_len * queue->byte_size;
			if(tmp_len > 0){
				memcpy((void*)queue->buffer + queue->tail*queue->byte_size, pArray, tmp_byte_size);
			}
			if(len-tmp_len > 0){
				memcpy((void*)queue->buffer, pArray+tmp_byte_size, push_byte_size - tmp_byte_size);					
			}
			queue->tail = len - tmp_len;

		}else{
			memcpy((void*)queue->buffer + queue->tail*queue->byte_size, pArray, push_byte_size);
			queue->tail = queue->tail + len;
		}
		queue->data_len = queue->data_len + len;
    return QUEUE_OK;
}

/**
****************************************************************
* @brief   从队列中弹出一组数据
* @param   queue, 队列变量指针
* @param   pArray, 待弹出队列的数据缓存地址
* @param   len, 待弹出队列的数据的最大长度
* @return  实际弹出数据的数量
****************************************************************
*/
queue_status_t queue_pop_array(volatile circle_queue_t *queue, uint8_t *pArray, uint32_t len)
{
    uint16_t tmp_len = 0;
		uint16_t tmp_byte_size, pop_byte_size = len * queue->byte_size;
	
		if(len > queue->data_len)
		{
			return QUEUE_ERROR;
		}	
		if(queue->head == queue->tail){
			return QUEUE_EMPTY;
		}
		
		if(queue->head > queue->tail && queue->head + len > queue->size){
				tmp_len = queue->size - queue->head;
				tmp_byte_size = tmp_len * queue->byte_size;
				if(tmp_len > 0){
					memcpy(pArray, (void*)queue->buffer + queue->head*queue->byte_size, tmp_byte_size);
				}
				if(len-tmp_len > 0){
					memcpy(pArray+tmp_byte_size, (void*)queue->buffer, pop_byte_size - tmp_byte_size);					
				}				
				queue->head = queue->head + len - queue->size;
		}else{
				memcpy(pArray, (void*)queue->buffer + queue->head*queue->byte_size, pop_byte_size);
				queue->head = queue->head + len;
				if(queue->head == queue->size){
					queue->head = 0;
				}
		}
		
		queue->data_len = queue->data_len - len;
    return QUEUE_OK;
}

/**
****************************************************************
* @brief   从队列中弹出一组数据
* @param   queue, 队列变量指针
* @param   pArray, 待弹出队列的数据缓存地址
* @param   len, 待弹出队列的数据的最大长度
* @return  实际弹出数据的数量
****************************************************************
*/
queue_status_t queue_pop_array_discard(volatile circle_queue_t *queue, uint32_t len)
{
    uint32_t tmp_len = 0;
	
		if(len > queue->data_len)
		{
			return QUEUE_ERROR;
		}	
		if(queue->head == queue->tail){
			return QUEUE_EMPTY;
		}else if(queue->head < queue->tail){
			queue->head = queue->head + len;
		}else{
			if(queue->head + len > queue->size){
				queue->head = queue->head + len - queue->size;
			}
		}

		queue->data_len = queue->data_len - len;
    return QUEUE_OK;
}

/**
****************************************************************
* @brief   从队列中弹出一组数据
* @param   queue, 队列变量指针
* @param   pArray, 待弹出队列的数据缓存地址
* @param   len, 待弹出队列的数据的最大长度
* @return  实际弹出数据的数量
****************************************************************
*/
uint32_t queue_pop_all(volatile circle_queue_t *queue, uint8_t *pArray)
{
    uint16_t tmp_len = 0;
		uint16_t tmp_byte_size, pop_byte_size = queue->data_len * queue->byte_size;
	
		if(queue->head == queue->tail && queue->data_len == 0){
			return 0;
		}else{		
			if(queue->head > queue->tail){
					tmp_len = queue->size - queue->head;
					tmp_byte_size = queue->byte_size * tmp_len;
					if(tmp_len > 0){
						memcpy(pArray, (void*)queue->buffer+queue->head*queue->byte_size, tmp_byte_size);
					}
					if(queue->tail > 0){
						memcpy(pArray+tmp_byte_size, (void*)queue->buffer, queue->tail*queue->byte_size);
					}
					queue->head = queue->tail;
			}else{
					tmp_len = queue->data_len;
					memcpy(pArray, (void*)queue->buffer + queue->head*queue->byte_size, pop_byte_size);
					queue->head = queue->tail;
			}
		}
		tmp_len = queue->data_len;
		queue->data_len = 0;
    return tmp_len;
}

/**
****************************************************************
* @brief   从队列中发送数据出去，由硬件DMA完成后，在DMA中调用此方法
* @param   queue, 队列变量指针
* @return  实际弹出数据的数量
****************************************************************
*/
uint32_t queue_pop_all_throw_hardware(volatile circle_queue_t *queue)
{
    uint32_t tmp_len = 0;
	
		if(queue->data_len > 0){				
			queue->head = queue->head + queue->data_len;
			if(queue->head > queue->size){
				queue->head = queue->head - queue->size;
			}
			queue->data_len = 0;
		}
    return tmp_len;
}

/**
****************************************************************
* @brief   获取队列中数据的个数
* @param   queue, 队列变量指针
* @return  队列中数据的个数
****************************************************************
*/
uint32_t queue_data_length(volatile circle_queue_t *queue)
{
	if (queue->head <= queue->tail)
	{
		return queue->tail - queue->head;
	}
	
	return queue->size + queue->tail - queue->head;
}


queue_status_t queue_read_data(volatile circle_queue_t *queue, void* data, uint16_t pos)
{
	uint16_t tmp_len;
	if(pos > queue->data_len)
		return QUEUE_ERROR;
	
	tmp_len = pos + queue->head;
	if(tmp_len >= queue->size){
		tmp_len = tmp_len - queue->size;
	}
	
	memcpy(data, (void*)queue->buffer + tmp_len*queue->byte_size, queue->byte_size);
	
	return QUEUE_OK;
}
