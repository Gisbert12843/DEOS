/*
 * gui.h
 *
 * Created: 09/01/2025 19:02:01
 *  Author: wwwgi
 */


#ifndef GUI_H_
#define GUI_H_

#include "gui_helper.h"

#define COLOR_WHITE 1 //white as initialized in init_gui()
#define COLOR_BLACK 2 //black as initialized in init_gui()
#define COLOR_GREY 3 //grey as initialized in init_gui()
#define COLOR_RED 4//red as initialized in init_gui()
#define COLOR_ORANGE 5 //orange as initialized in init_gui()
#define COLOR_LIGHT_BLUE 6 //light blue as initialized in init_gui()
#define COLOR_BLUE 7 //blue as initialized in init_gui()
#define COLOR_GREEN 8 //green as initialized in init_gui()

#define BACKGROUND_COLOR COLOR_WHITE
// #define TEXT_COLOR COLOR_BLACK


typedef struct
{
    address_t sensor_src_address;
    sensor_parameter_type_t sensor_data_type;
    sensor_type_t sensor_type;
    data_queue_t sensor_data_queue;
    time_t sensor_last_update;
    sensor_parameter_t min_value;
    sensor_parameter_t max_value;
    int row1; //which row the element starts | 0-3
    int column1; //which column the element starts | 0-3
    int row2; //which row the element ends | 0-3
    int column2; //which column the element ends | 0-3
    uint8_t ui_state; //state describing how the element is displayed, dependend on the sensor_data_type
    bool timeout_flag;
} gui_element_container_t; //describes the GUI element that a single sensor/device combination occupies



/*
* main worker function of the GUI layer
* initializes the GUI
* updates gui elements with new sensor data waiting in the buffer
* updates gui elements that have not been updated for (sensor_data_update_timeout_ms) by grey scaling them
* does not return
*/
void gui_worker();

//called by the communication layer to cache incoming sensor data
void enqueue_sensor_data_into_buffer(sensor_data_t* data);
//prints the gui element to the terminal
void print_gui_element(gui_element_container_t* gui_element);

#endif /* GUI_H_ */