/*
 * gui_helper.h
 *
 * Created: 09/01/2025 21:05:06
 *  Author: wwwgi
 */


#ifndef GUI_HELPER_H_
#define GUI_HELPER_H_

#define QUEUE_SIZE 12

#include "../communication/sensorData.h"
#include "../communication/rfAdapter.h"

#include <stdbool.h>

typedef struct
{
    sensor_parameter_t data[QUEUE_SIZE];
    int head;
    int tail;
    int count;
} data_queue_t; //data history as a queue of a unique sensor/device combination

void queue_init(data_queue_t* queue);

// Check if the Queue is empty
bool queue_is_empty(const data_queue_t* queue);

// Add a new element to the Queue
void queue_push(data_queue_t* queue, sensor_parameter_t value);

// peek the oldest element and remove it
sensor_parameter_t queue_pop(data_queue_t* queue);

bool queue_peek_element(const data_queue_t* queue, sensor_parameter_t* data, int pos);

bool queue_peek_oldest_data(data_queue_t* queue, sensor_parameter_t* value);
bool queue_peek_newest_data(data_queue_t* queue, sensor_parameter_t* value);

bool sensor_value_is_smaller(sensor_parameter_t* value1, sensor_parameter_t* value2, sensor_parameter_type_t sensor_data_type);
bool sensor_value_is_greater(sensor_parameter_t* value1, sensor_parameter_t* value2, sensor_parameter_type_t sensor_data_type);


#endif /* GUI_HELPER_H_ */