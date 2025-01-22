/*
 * gui.c
 *
 * Created: 09/01/2025 19:02:11
 *  Author: wwwgi
 */

#include "gui.h"
#include <stddef.h>
#include <stdbool.h>

#include "../tlcd/tlcd_core.h"
#include "../tlcd/tlcd_graphic.h"
#include "../os_scheduler.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "../lib/terminal.h"

#define GUI_ELEMENT_CONTAINER_SIZE 6 // Amount of GUI Elements allowed

#define SENSOR_DATA_UPDATE_TIMEOUT_MS 8000

#define GRID_STATUSBAR_HEIGHT 22

#define GRID_TOTAL_WIDTH (TLCD_WIDTH)
#define GRID_TOTAL_HEIGHT (TLCD_HEIGHT - GRID_STATUSBAR_HEIGHT)

#if GRID_TOTAL_WIDTH > GRID_TOTAL_HEIGHT
#define GRID_OUTER_HOR_PADDING (uint16_t)(GRID_TOTAL_WIDTH / 40)
#define GRID_OUTER_VER_PADDING (uint16_t) GRID_OUTER_HOR_PADDING
#warning display is horizontal
#elif GRID_TOTAL_WIDTH < GRID_TOTAL_HEIGHT
#define GRID_OUTER_VER_PADDING (uint16_t)(GRID_TOTAL_HEIGHT / 40)
#define GRID_OUTER_HOR_PADDING (uint16_t) GRID_OUTER_VER_PADDING
#warning display is vertical
#else
#define GRID_OUTER_HOR_PADDING (uint16_t)(GRID_TOTAL_WIDTH / 40)
#define GRID_OUTER_VER_PADDING (uint16_t)(GRID_TOTAL_HEIGHT / 40)
#endif

#define GRID_INNER_WIDTH (TLCD_WIDTH - GRID_OUTER_HOR_PADDING * 2)
#define GRID_INNER_HEIGHT (TLCD_HEIGHT - GRID_STATUSBAR_HEIGHT - GRID_OUTER_VER_PADDING * 2)

#define GRID_COLUMN_COUNT 3
#define GRID_ROW_COUNT 2

#define GRID_CELL_TIMEOUT_SYMBOL_HEIGHT 6

#define GRID_CELL_TOTAL_WIDTH (uint16_t)((TLCD_WIDTH - GRID_OUTER_HOR_PADDING * 2) / GRID_COLUMN_COUNT)
#define GRID_CELL_TOTAL_HEIGHT (uint16_t)((TLCD_HEIGHT - GRID_OUTER_VER_PADDING * 2 - GRID_STATUSBAR_HEIGHT) / GRID_ROW_COUNT)
#define GRID_CELL_INNER_HOR_PADDING (uint16_t)(GRID_CELL_TOTAL_WIDTH * 0.1)
#define GRID_CELL_INNER_VER_PADDING (uint16_t)(GRID_CELL_TOTAL_HEIGHT * 0.1)

#define GRID_CELL_START_X (uint16_t)(GRID_OUTER_HOR_PADDING + (gui_element->column1 * GRID_CELL_TOTAL_WIDTH))                                 // Start X of an individual cell
#define GRID_CELL_START_Y (uint16_t)(GRID_STATUSBAR_HEIGHT + GRID_OUTER_VER_PADDING + (gui_element->row1 * GRID_CELL_TOTAL_HEIGHT))           // Start Y of an individual cell
#define GRID_CELL_END_X (uint16_t)(GRID_OUTER_HOR_PADDING + (((gui_element)->column2 + 1) * GRID_CELL_TOTAL_WIDTH) - 1)                       // End X of an individual cell
#define GRID_CELL_END_Y (uint16_t)(GRID_STATUSBAR_HEIGHT + GRID_OUTER_VER_PADDING + (((gui_element)->row2 + 1) * GRID_CELL_TOTAL_HEIGHT) - 1) // End Y of an individual cell

#define GRID_CELL_CONTENT_START_X (uint16_t)(GRID_CELL_START_X + GRID_CELL_INNER_HOR_PADDING)
#define GRID_CELL_CONTENT_START_Y (uint16_t)(GRID_CELL_START_Y + GRID_CELL_INNER_VER_PADDING + TEXT_HEIGHT + GRID_CELL_TIMEOUT_SYMBOL_HEIGHT) // Start Y of the content of an individual cell
#define GRID_CELL_CONTENT_END_X (uint16_t)(GRID_CELL_END_X - GRID_CELL_INNER_HOR_PADDING)
#define GRID_CELL_CONTENT_END_Y (uint16_t)(GRID_CELL_END_Y - GRID_CELL_INNER_VER_PADDING)


#define TEXT_HEIGHT 20
#define TEXT_START_X_OFFSET 3
#define TEXT_START_Y_OFFSET 

int current_top_row = 0;

static bool grid[GRID_ROW_COUNT][GRID_COLUMN_COUNT];

#define SENSOR_ELEMENT_BUFFER_SIZE 10 // Amount of SensorData allowed to be cached

int sensor_element_buffer_count = 0; // Amount of SensorData in the sensor_element_buffer
sensor_data_t sensor_element_buffer[SENSOR_ELEMENT_BUFFER_SIZE]; // Buffers incoming SensorData to be processed later by the gui_worker

void enqueue_sensor_data_into_buffer(sensor_data_t* data)
{
    os_enterCriticalSection();

    for (int i = 0; i < sensor_element_buffer_count; i++) // override prior data of this sensor still cached
    {
        if (sensor_element_buffer[i].sensor_src_address == data->sensor_src_address && sensor_element_buffer[i].sensor_type == data->sensor_type && sensor_element_buffer[i].sensor_data_type == data->sensor_data_type)
        {
            sensor_element_buffer[i] = *data;
            // DEBUG("enqueue_sensor_data_into_buffer() overwrote previously buffered sensor data\n");
            // DEBUG("enqueue_sensor_data_into_buffer() at %d is now:\n", i);
            // print_sensor_data(&sensor_element_buffer[i]);
            os_leaveCriticalSection();
            return;
        }
    }

    if (sensor_element_buffer_count == SENSOR_ELEMENT_BUFFER_SIZE)
    {
        os_leaveCriticalSection();
        printf_P(PSTR("enqueue_sensor_data_into_buffer() would overflow buffer\n\n"));
        return;
    }


    sensor_element_buffer[sensor_element_buffer_count] = *data;
    sensor_element_buffer_count++;

    // DEBUG("enqueue_sensor_data_into_buffer() sensor_element_buffer_count: %d\n", sensor_element_buffer_count);
    // DEBUG("enqueue_sensor_data_into_buffer() at %d is now:\n", 0);
    // print_sensor_data(&sensor_element_buffer[0]);
    os_leaveCriticalSection();
}

/**
 * Updates the sensor data for a GUI element.
 *
 * This function checks if the provided sensor_data's fValue is less than the
 * gui_element's current minimum value or greater than its current maximum value.
 * If so, it updates the respective min_value or max_value in the gui_element.
 * Additionally, it pushes the sensor_data into the sensor_data_queue and increments
 * the queue count.
 *
 * @param gui_element Pointer to the GUI element container to be updated.
 * @param sensor_data Pointer to the sensor data containing the new value.
 */
void update_sensor_data(gui_element_container_t* gui_element, sensor_parameter_t* sensor_data)
{
    if (sensor_value_is_smaller(sensor_data, &gui_element->min_value, gui_element->sensor_data_type))
    {
        gui_element->min_value = *sensor_data;
    }
    else if (sensor_value_is_greater(sensor_data, &gui_element->max_value, gui_element->sensor_data_type))
    {
        gui_element->max_value = *sensor_data;
    }

    queue_push(&gui_element->sensor_data_queue, *sensor_data);
}
// Called when data from a new sensor was cached and will be added as a gui_element
// Returns true if the element was added successfully, false if there was no space in the grid
// Will add a new gui_element to the sensor_gui_elements array, so update_sensor_data() should not be called for this sensor_element
bool add_gui_element(gui_element_container_t sensor_gui_elements[16], uint8_t* sensor_gui_elements_count, sensor_data_t* sensor_element)
{
    os_enterCriticalSection();

    if (*sensor_gui_elements_count == GUI_ELEMENT_CONTAINER_SIZE)
    {
        os_leaveCriticalSection();
        WARN("add_gui_element() received sensor_gui_elements_count >= #GUI_ELEMENT_CONTAINER_SIZE");
        return false;
    }

    // DEBUG("add_gui_element()\nSensor data received:");
    // print_sensor_data(sensor_element);

    for (int row = 0; row < GRID_ROW_COUNT; row++)
    {
        for (int column = 0; column < GRID_COLUMN_COUNT; column++)
        {
            if (!grid[row][column])
            {
                grid[row][column] = true;

                gui_element_container_t new_sensor_gui_element;

                new_sensor_gui_element.sensor_src_address = sensor_element->sensor_src_address;
                new_sensor_gui_element.sensor_data_type = sensor_element->sensor_data_type;
                new_sensor_gui_element.sensor_type = sensor_element->sensor_type;
                queue_init(&new_sensor_gui_element.sensor_data_queue);
                queue_push(&new_sensor_gui_element.sensor_data_queue, sensor_element->sensor_data_value);
                new_sensor_gui_element.sensor_last_update = getSystemTime_ms();

                new_sensor_gui_element.min_value.fValue = 0.0;
                new_sensor_gui_element.max_value.fValue = 0.0;

                new_sensor_gui_element.row1 = row;
                new_sensor_gui_element.column1 = column;
                new_sensor_gui_element.row2 = row;
                new_sensor_gui_element.column2 = column;

                new_sensor_gui_element.ui_state = 1;
                new_sensor_gui_element.timeout_flag = false;

                sensor_gui_elements[*sensor_gui_elements_count] = new_sensor_gui_element; // add new element to the container
                (*sensor_gui_elements_count)++;

                INFO("add_gui_element() added element at row: %d, column: %d", row, column);
                // print_gui_element(&new_sensor_gui_element);
                os_leaveCriticalSection();

                char buffer[50];  // Buffer to store the formatted string

                gui_element_container_t* gui_element = &new_sensor_gui_element;
                // Format the string with the hex value and float value
                sprintf(buffer, "Address: %d, Sensor: %d", new_sensor_gui_element.sensor_src_address, new_sensor_gui_element.sensor_type);
                tlcd_drawString(GRID_CELL_START_X + TEXT_START_X_OFFSET, GRID_CELL_START_Y, buffer);
                memset(buffer, 0, sizeof(buffer));
                return true;
            }
        }
    }
    os_leaveCriticalSection();
    WARN("!! add_gui_element() had no space in the grid !!");
    return false;
}

void drawHourglass(int startX, int startY, int endX, int endY) {
    tlcd_drawLine(startX, startY, endX, endY);
    tlcd_drawLine(startX, startY, endX, startY);
    tlcd_drawLine(startX, endY, endX, endY);
    tlcd_drawLine(endX, startY, startX, endY);
}

// called when sensor_data is already cached to update an existing gui_element with either real data or to mark it as outdated
void update_gui_element(gui_element_container_t* gui_element, bool real_update)
{
    if (!real_update)
    {
        //draw triangle by lines
        drawHourglass(GRID_CELL_CONTENT_END_X - 6, GRID_CELL_CONTENT_START_Y - GRID_CELL_INNER_VER_PADDING - GRID_CELL_TIMEOUT_SYMBOL_HEIGHT, GRID_CELL_CONTENT_END_X, GRID_CELL_CONTENT_START_Y - GRID_CELL_INNER_VER_PADDING + GRID_CELL_TIMEOUT_SYMBOL_HEIGHT);

        printf("update_gui_element() WRITTEN TIMEOUT SYMBOL\n");
        return;
        //tlcd_changeLineColor(COLOR_GREY);
    }
    else // real update
    {
        gui_element->timeout_flag = false;
        gui_element->sensor_last_update = getSystemTime_ms();
        //tlcd_changeLineColor(COLOR_RED);
        tlcd_clearArea(GRID_CELL_START_X, GRID_CELL_CONTENT_START_Y - GRID_CELL_INNER_VER_PADDING - GRID_CELL_TIMEOUT_SYMBOL_HEIGHT, GRID_CELL_END_X, GRID_CELL_CONTENT_END_Y);
    }

    sensor_parameter_t data;

    if (!queue_peek_newest_data(&gui_element->sensor_data_queue, &data))
    {
        WARN("update_gui_element() TEMP String length out of bounds");
        tlcd_changeTextSize(2);
        char buffer[5];  // Buffer to store the formatted string
        memset(buffer, 0, sizeof(buffer));
        strcat(buffer, "ERR");
        tlcd_drawStringInArea(GRID_CELL_CONTENT_START_X, GRID_CELL_CONTENT_START_Y, GRID_CELL_CONTENT_END_X, GRID_CELL_CONTENT_END_Y, buffer);
        tlcd_changeTextSize(1);
        return;
    }

    char buffer[50];  // Buffer to store the formatted string


    int length = 0;

    switch (gui_element->sensor_data_type)
    {
        case PARAM_CO2_PPM: //ALSO TEST CASE
        {
            switch (gui_element->ui_state)
            {
                case 1:
                {
                    sprintf(buffer, "%u", (unsigned int)data.uValue);
                    if (data.uValue > 1400)
                    {
                        tlcd_changeLineColor(COLOR_RED);
                    }
                    else if (data.uValue > 1000)
                    {
                        tlcd_changeLineColor(COLOR_ORANGE);
                    }
                    else
                    {
                        tlcd_changeLineColor(COLOR_GREEN);
                    }
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 1, GRID_CELL_CONTENT_START_Y + 1, GRID_CELL_END_X - 2 - 1, GRID_CELL_CONTENT_END_Y + 1);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 2, GRID_CELL_CONTENT_START_Y + 2, GRID_CELL_END_X - 2 - 2, GRID_CELL_CONTENT_END_Y + 2);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 3, GRID_CELL_CONTENT_START_Y + 3, GRID_CELL_END_X - 2 - 3, GRID_CELL_CONTENT_END_Y + 3);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 4, GRID_CELL_CONTENT_START_Y + 4, GRID_CELL_END_X - 2 - 4, GRID_CELL_CONTENT_END_Y + 4);
                    tlcd_changeLineColor(COLOR_BLACK);

                    strcat(buffer, " ");
                    strcat(buffer, "PPM");
                    length = strlen(buffer);
                }
                break;
                case 2:
                {

                }
                break;
                default:
                    WARN("update_gui_element() unknown ui_state");
                    return;
            }
        }
        break;

        case PARAM_TEMPERATURE_CELSIUS:
        {
            switch (gui_element->ui_state)
            {
                case 1:
                {
                    sprintf(buffer, "%.1f", data.fValue);

                    if (data.fValue >= 30.0)
                    {
                        tlcd_changeLineColor(COLOR_RED);
                        //make border
                    }
                    else if (data.fValue >= 25.0)
                    {
                        tlcd_changeLineColor(COLOR_ORANGE);
                    }
                    else if (data.fValue < 16.0)
                    {
                        tlcd_changeLineColor(COLOR_LIGHT_BLUE);
                    }
                    else if (data.fValue < 12.0)
                    {
                        tlcd_changeLineColor(COLOR_BLUE);
                    }
                    else
                    {
                        tlcd_changeLineColor(COLOR_GREEN);
                    }

                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 1, GRID_CELL_CONTENT_START_Y + 1, GRID_CELL_END_X - 2 - 1, GRID_CELL_CONTENT_END_Y + 1);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 2, GRID_CELL_CONTENT_START_Y + 2, GRID_CELL_END_X - 2 - 2, GRID_CELL_CONTENT_END_Y + 2);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 3, GRID_CELL_CONTENT_START_Y + 3, GRID_CELL_END_X - 2 - 3, GRID_CELL_CONTENT_END_Y + 3);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 4, GRID_CELL_CONTENT_START_Y + 4, GRID_CELL_END_X - 2 - 4, GRID_CELL_CONTENT_END_Y + 4);

                    tlcd_changeLineColor(COLOR_BLACK);

                    strcat(buffer, " ");
                    strcat(buffer, "*");
                    strcat(buffer, "C");
                    length = strlen(buffer);
                }
                break;
                case 2:
                {

                }
                break;
                default:
                    WARN("update_gui_element() unknown ui_state");
                    return;
            }
        }
        break;

        case PARAM_LIGHT_INTENSITY_PERCENT:
        case PARAM_HUMIDITY_PERCENT:
        {
            switch (gui_element->ui_state)
            {
                case 1:
                {
                    sprintf(buffer, "%.1f", data.fValue);

                    strcat(buffer, " ");
                    strcat(buffer, "\%");
                    length = strlen(buffer);
                }
                break;
                case 2:
                {

                }
                break;
                default:
                    WARN("update_gui_element() unknown ui_state");
                    return;
            }
        }
        break;

        case PARAM_PRESSURE_PASCAL:
        {
            switch (gui_element->ui_state)
            {
                case 1:
                {
                    sprintf(buffer, "%.1f", data.fValue);
                    strcat(buffer, " ");
                    strcat(buffer, "hPa");
                    length = strlen(buffer);
                }
                break;
                case 2:
                {

                }
                break;
                default:
                    WARN("update_gui_element() unknown ui_state");
                    return;
            }
        }
        break;

        case PARAM_E_CO2_PPM:
        {
            switch (gui_element->ui_state)
            {
                case 1:
                {
                    sprintf(buffer, "%.1f", data.fValue);

                    strcat(buffer, " ");
                    strcat(buffer, "PPM");
                    length = strlen(buffer);

                    if (data.fValue > 2000)
                    {
                        tlcd_changeLineColor(COLOR_RED);
                    }
                    else if (data.fValue > 1500)
                    {
                        tlcd_changeLineColor(COLOR_ORANGE);
                    }
                    else
                    {
                        tlcd_changeLineColor(COLOR_GREEN);
                    }

                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 1, GRID_CELL_CONTENT_START_Y + 1, GRID_CELL_END_X - 2 - 1, GRID_CELL_CONTENT_END_Y + 1);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 2, GRID_CELL_CONTENT_START_Y + 2, GRID_CELL_END_X - 2 - 2, GRID_CELL_CONTENT_END_Y + 2);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 3, GRID_CELL_CONTENT_START_Y + 3, GRID_CELL_END_X - 2 - 3, GRID_CELL_CONTENT_END_Y + 3);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 4, GRID_CELL_CONTENT_START_Y + 4, GRID_CELL_END_X - 2 - 4, GRID_CELL_CONTENT_END_Y + 4);
                    tlcd_changeLineColor(COLOR_BLACK);
                }
                break;
                case 2:
                {

                }
                break;
                default:
                    WARN("update_gui_element() unknown ui_state");
                    return;
            }
        }
        break;

        case PARAM_TVOC_PPB:
        {
            switch (gui_element->ui_state)
            {
                case 1:
                {
                    sprintf(buffer, "%u", (unsigned int)data.uValue);

                    strcat(buffer, " ");
                    strcat(buffer, "PPB");
                    length = strlen(buffer);

                    if (data.uValue > 1200)
                    {
                        tlcd_changeLineColor(COLOR_RED);
                    }
                    else if (data.uValue > 550)
                    {
                        tlcd_changeLineColor(COLOR_ORANGE);
                    }
                    else
                    {
                        tlcd_changeLineColor(COLOR_GREEN);
                    }

                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 1, GRID_CELL_CONTENT_START_Y + 1, GRID_CELL_END_X - 2 - 1, GRID_CELL_CONTENT_END_Y + 1);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 2, GRID_CELL_CONTENT_START_Y + 2, GRID_CELL_END_X - 2 - 2, GRID_CELL_CONTENT_END_Y + 2);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 3, GRID_CELL_CONTENT_START_Y + 3, GRID_CELL_END_X - 2 - 3, GRID_CELL_CONTENT_END_Y + 3);
                    tlcd_drawRectangle(GRID_CELL_START_X + 2 + 4, GRID_CELL_CONTENT_START_Y + 4, GRID_CELL_END_X - 2 - 4, GRID_CELL_CONTENT_END_Y + 4);
                    tlcd_changeLineColor(COLOR_BLACK);
                }
                break;
                case 2:
                {

                }
                break;
                default:
                    WARN("update_gui_element() unknown ui_state");
                    return;
            }
        }
        break;
        default:
            break;
    }

    if (length <= 8)
    {
        tlcd_changeTextSize(2);
    }
    else if (length <= 12)
    {
        tlcd_changeTextSize(1);
    }
    else
    {
        WARN("update_gui_element() TEMP String length out of bounds");
        tlcd_changeTextSize(2);
        memset(buffer, 0, sizeof(buffer));
        strcat(buffer, "ERR");
        tlcd_drawStringInArea(GRID_CELL_CONTENT_START_X, GRID_CELL_CONTENT_START_Y, GRID_CELL_CONTENT_END_X, GRID_CELL_CONTENT_END_Y, buffer);
        tlcd_changeTextSize(1);
        return;
    }

    tlcd_drawStringInArea(GRID_CELL_CONTENT_START_X, GRID_CELL_CONTENT_START_Y, GRID_CELL_CONTENT_END_X, GRID_CELL_CONTENT_END_Y, buffer);
    tlcd_changeTextSize(1);
}

void init_gui(gui_element_container_t sensor_gui_elements[GUI_ELEMENT_CONTAINER_SIZE])
{
    // 1 is the BG color from the Device

    tlcd_defineColor(0, (tlcd_color_t) { 0, 255, 0 });
    tlcd_defineColor(1, (tlcd_color_t) { 255, 255, 255 }); // white as background
    tlcd_defineColor(2, (tlcd_color_t) { 0, 0, 0 }); // black

    tlcd_defineColor(3, (tlcd_color_t) { 200, 200, 200 }); // grey
    tlcd_defineColor(4, (tlcd_color_t) { 255, 0, 0 }); // red
    tlcd_defineColor(5, (tlcd_color_t) { 255, 165, 0 }); // orange
    tlcd_defineColor(6, (tlcd_color_t) { 173, 216, 230 }); // light blue
    tlcd_defineColor(7, (tlcd_color_t) { 0, 0, 255 }); // blue
    tlcd_defineColor(8, (tlcd_color_t) { 0, 255, 0 }); // green

    tlcd_changeLineColor(COLOR_BLACK);
    tlcd_changeTextColor(COLOR_BLACK);
    //tlcd_changeDisplayColor(BACKGROUND_COLOR);

    tlcd_clearDisplay();

    printf_P(PSTR("Clearing Display\n"));
    delayMs(2000);

   // tlcd_setFontZoom(1, 1);
    tlcd_changePenSize(1);

    // create TLCD_WIDTH x 22 StatusBar
    tlcd_drawLine(0, GRID_STATUSBAR_HEIGHT, TLCD_WIDTH, GRID_STATUSBAR_HEIGHT);

    tlcd_drawString(TLCD_WIDTH - 380, 0, "Running for:");

    // create grid 400x200 adding outer padding
    // tlcd_drawBox(GRID_OUTER_HOR_PADDING, GRID_OUTER_VER_PADDING + GRID_STATUSBAR_HEIGHT - 1, TLCD_WIDTH - GRID_OUTER_HOR_PADDING, TLCD_HEIGHT - GRID_OUTER_VER_PADDING, COLOR_GREY);
}

void update_clock()
{
    char buffer[50];  // Buffer to store the formatted string
    time_t local_system_time = getSystemTime_ms();
    time_t seconds = local_system_time / 1000;
    time_t minutes = seconds / 60;
    time_t hours = minutes / 60;
    time_t days = hours / 24;
    time_t years = days / 365;

    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    days %= 365;

    static time_t last_second = 0;
    static time_t last_minute = 0;
    static time_t last_hour = 0;
    static time_t last_day = 0;
    static time_t last_year = 0;

    if (seconds != last_second)
    {
        last_second = seconds;
        tlcd_clearArea(TLCD_WIDTH - 18, 0, TLCD_WIDTH, GRID_STATUSBAR_HEIGHT - 2);
    }
    if (minutes != last_minute)
    {
        last_minute = minutes;
        tlcd_clearArea(TLCD_WIDTH - 90, 0, TLCD_WIDTH - 72, GRID_STATUSBAR_HEIGHT - 2);
    }
    if (hours != last_hour)
    {
        last_hour = hours;
        tlcd_clearArea(TLCD_WIDTH - 156, 0, TLCD_WIDTH - 142, GRID_STATUSBAR_HEIGHT - 2);
    }
    if (days != last_day)
    {
        last_day = days;
        tlcd_clearArea(TLCD_WIDTH - 220, 0, TLCD_WIDTH - 198, GRID_STATUSBAR_HEIGHT - 2);
    }
    if (years != last_year)
    {
        last_year = years;
        tlcd_clearArea(TLCD_WIDTH - 274, 0, TLCD_WIDTH - 258, GRID_STATUSBAR_HEIGHT - 2);
    }

    sprintf(buffer, "Years:%02d Days:%03d Hours:%02d Minutes:%02d Seconds:%02d", (int)years, (int)days, (int)hours, (int)minutes, (int)seconds);
    tlcd_drawString(TLCD_WIDTH - 310, 0, buffer);
}

/*
this function is the main worker function of the GUI layer
- initializes the GUI
- regulary updates gui elements with new sensor data waiting in the buffer
- updates gui elements that have not been updated for (sensor_data_update_timeout_ms) by grey scaling them

!! this function is blocking but yields if no new sensor data is available

- function does not return
*/
void gui_worker()
{
    DEBUG("STACK_SIZE_PROC: %d", STACK_SIZE_PROC);
    DEBUG("Array requires %d bytes", sizeof(gui_element_container_t) * GUI_ELEMENT_CONTAINER_SIZE);


    uint8_t sensor_gui_elements_count = 0; // Amount of GUI Elements currently displayed
    gui_element_container_t sensor_gui_elements[GUI_ELEMENT_CONTAINER_SIZE]; // Array of GUI Elements currently displayed
    time_t local_system_time = getSystemTime_ms();
    time_t last_update = local_system_time;

    init_gui(sensor_gui_elements);

    DEBUG("gui_worker() started");


    while (1)
    {
        local_system_time = getSystemTime_ms();

        os_enterCriticalSection();

        if ((local_system_time - last_update) > 1000)
        {
            update_clock();
            // Update the Rest of the GUI Elements if enough time has passed
            for (int i = 0; i < sensor_gui_elements_count; i++)
            {
                // DEBUG("Checking if sensor_gui_elements[%d] needs to be timedout", i);
                if (!sensor_gui_elements[i].timeout_flag && (local_system_time - sensor_gui_elements[i].sensor_last_update) > SENSOR_DATA_UPDATE_TIMEOUT_MS)
                {
                    sensor_gui_elements[i].timeout_flag = true;
                    update_gui_element(&sensor_gui_elements[i], false);
                    DEBUG("sensor_gui_elements[%d] was timedout\n\n", i);
                }
            }
            last_update = local_system_time;
        }

        if (sensor_element_buffer_count == 0)
        {
            os_leaveCriticalSection();
            os_yield();
            continue;
        }

        // save local copy of sensor_element_buffer
        int local_sensor_element_buffer_count = sensor_element_buffer_count; //copy of global sensor_element_buffer_count
        sensor_data_t local_sensor_element_buffer[SENSOR_ELEMENT_BUFFER_SIZE]; // copy of global sensor_element_buffer
        memcpy(local_sensor_element_buffer, sensor_element_buffer, sizeof sensor_element_buffer);

        // Clear sensor_element_buffer
        memset(sensor_element_buffer, 0, sizeof sensor_element_buffer);
        sensor_element_buffer_count = 0;

        os_leaveCriticalSection();

        for (int i = 0; i < local_sensor_element_buffer_count; i++) // iterate all updating data elements
        {
            bool found = false;
            for (int j = 0; j < sensor_gui_elements_count; j++) // possibly find the updating data in the present elements
            {
                if (local_sensor_element_buffer[i].sensor_src_address == sensor_gui_elements[j].sensor_src_address) // comparing sensor_address and sensor_data_type
                {
                    update_sensor_data(&sensor_gui_elements[j], &local_sensor_element_buffer[i].sensor_data_value);             // update min max values
                    update_gui_element(&sensor_gui_elements[j], true);                                                       // update GUI element
                    found = true;
                    DEBUG("sensor_gui_elements[%d] was updated\n\n", j);
                    break;
                }
            }
            if (!found) // sensor was not yet added to the GUI, we gotta do that now
            {
                DEBUG("Sensor %d was not yet added to the GUI\n", local_sensor_element_buffer[i].sensor_src_address);
                if (!add_gui_element(sensor_gui_elements, &sensor_gui_elements_count, &local_sensor_element_buffer[i]))
                    continue;                                                                  // we couldn't add it and add_gui_element() printed an Error for us
                update_gui_element(&sensor_gui_elements[sensor_gui_elements_count - 1], true); // -1 as we successfully added it before to the last position in the array
            }
        }
    }
}

void print_gui_element(gui_element_container_t* gui_element)
{

    printf_P(PSTR("{\nsensor_src_address: %d\n"), gui_element->sensor_src_address);
    printf_P(PSTR("sensor_data_type: %d\n"), gui_element->sensor_data_type);
    printf_P(PSTR("sensor_last_update: %d\n"), gui_element->sensor_last_update);
    printf_P(PSTR("min_value: %d\n"), gui_element->min_value);
    printf_P(PSTR("max_value: %d\n"), gui_element->max_value);
    printf_P(PSTR("row1: %d\n"), gui_element->row1);
    printf_P(PSTR("column1: %d\n"), gui_element->column1);
    printf_P(PSTR("row2: %d\n"), gui_element->row2);
    printf_P(PSTR("column2: %d\n}\n"), gui_element->column2);
}