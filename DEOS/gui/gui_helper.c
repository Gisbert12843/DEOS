/*
 * gui_helper.h
 *
 * Created: 09/01/2025 21:05:18
 *  Author: wwwgi
 */
#include "gui_helper.h"
#include "../lib/terminal.h"

// Check if the Queue is empty
bool queue_is_empty(const data_queue_t* queue)
{
    return queue->count == 0;
}

// Add a new element to the Queue
void queue_push(data_queue_t* queue, sensor_parameter_t value)
{

    queue->data[queue->head] = value;
    queue->head = (queue->head + 1) % QUEUE_SIZE;

    if (queue->count == QUEUE_SIZE)
    {
        queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    }
    else
    {
        queue->count++;
    }
    DEBUG("queue_push() head: %d, tail: %d, count: %d", queue->head, queue->tail, queue->count);
}

void queue_init(data_queue_t* queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

bool queue_peek_element(const data_queue_t* queue, sensor_parameter_t* data, int pos)
{
    if (pos < 0 || pos >= queue->count) {
        return false;
    }

    int actual_index = (queue->tail + pos) % QUEUE_SIZE;
    *data = queue->data[actual_index];
    return true;
}

bool queue_peek_oldest_data(data_queue_t* queue, sensor_parameter_t* value)
{
    if (queue_is_empty(queue))
    {
        return false;
    }
    *value = queue->data[queue->tail];
    return true;
}


bool queue_peek_newest_data(data_queue_t* queue, sensor_parameter_t* value)
{
    if (queue_is_empty(queue))
    {
        return false;
    }
    *value = queue->data[(queue->head + QUEUE_SIZE - 1) % QUEUE_SIZE];
    return true;
}

bool sensor_value_is_smaller(sensor_parameter_t* value1, sensor_parameter_t* value2, sensor_parameter_type_t sensor_data_type)
{
    switch (sensor_data_type)
    {

        case PARAM_TEMPERATURE_CELSIUS:
        {
            return value1->fValue < value2->fValue;
        }
        break;
        case PARAM_HUMIDITY_PERCENT:
        {
            return value1->uValue < value2->uValue;
        }
        break;
        case PARAM_LIGHT_INTENSITY_PERCENT:
        {
            return value1->fValue < value2->fValue;
        }
        break;
        case PARAM_ALTITUDE_M:
        {
            return value1->uValue < value2->uValue;
        }
        break;
        case PARAM_PRESSURE_PASCAL:
        {
            return value1->fValue < value2->fValue;
        }
        break;
        case PARAM_E_CO2_PPM:
        {
            return value1->uValue < value2->uValue;
        }
        break;
        case PARAM_TVOC_PPB:
        {
            return value1->uValue < value2->uValue;
        }
        break;
        case PARAM_CO2_PPM:
        {
            return value1->uValue < value2->uValue;
        }
        break;

        default:
            break;
    }
    printf_P("ERROR: sensor_value_is_smaller: Unknown sensor data type\n");
    return false;
}


bool sensor_value_is_greater(sensor_parameter_t* value1, sensor_parameter_t* value2, sensor_parameter_type_t sensor_data_type)
{
    switch (sensor_data_type)
    {

        case PARAM_TEMPERATURE_CELSIUS:
        {
            return value1->fValue > value2->fValue;
        }
        break;
        case PARAM_HUMIDITY_PERCENT:
        {
            return value1->uValue > value2->uValue;
        }
        break;
        case PARAM_LIGHT_INTENSITY_PERCENT:
        {
            return value1->fValue > value2->fValue;
        }
        break;
        case PARAM_ALTITUDE_M:
        {
            return value1->uValue > value2->uValue;
        }
        break;
        case PARAM_PRESSURE_PASCAL:
        {
            return value1->fValue > value2->fValue;
        }
        break;
        case PARAM_E_CO2_PPM:
        {
            return value1->uValue > value2->uValue;
        }
        break;
        case PARAM_TVOC_PPB:
        {
            return value1->uValue > value2->uValue;
        }
        break;
        case PARAM_CO2_PPM:
        {
            return value1->uValue > value2->uValue;
        }
        break;

        default:
            break;
    }
    printf_P("sensor_value_is_greater: Unknown sensor data type\n");
    return false;
}