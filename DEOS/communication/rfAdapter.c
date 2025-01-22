/*!
 *  \brief Layer built on top of serialAdapter where commands are defined.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "rfAdapter.h"
#include "../lib/lcd.h"
#include "../os_core.h"
#include "string.h"
#include "../gui/gui.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

//! Flag that indicates whether the adapter has been initialized
bool rfAdapter_initialized = false;

//! Start-Flag that announces a new frame
start_flag_t serialAdapter_startFlag = 0x5246; // "RF"

//! Configuration what address this microcontroller has
address_t serialAdapter_address = ADDRESS(1, 4);

//----------------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------------

void rfAdapter_receiveSetLed(cmd_setLed_t*);
void rfAdapter_receiveToggleLed();
void rfAdapter_receiveLcdGoto(cmd_lcdGoto_t*);
void rfAdapter_receiveLcdPrint(cmd_lcdPrint_t*);
void rfAdapter_receiveLcdClear();

//----------------------------------------------------------------------------
// Your Homework
//----------------------------------------------------------------------------

/*!
 *  Initializes the rfAdapter and their dependencies
 */
void rfAdapter_init()
{
    serialAdapter_init();
    DDRB |= (1 << PB7);
    rfAdapter_initialized = true;
}

/*!
 * Check if adapter has been initialized
 *
 * \return True if the communication has been initialized
 */
uint8_t rfAdapter_isInitialized()
{
    return rfAdapter_initialized;
}

/*!
 *  Main task of adapter
 */
void rfAdapter_worker()
{
    serialAdapter_worker();
}

/*!
 *  Is called on command frame receive
 *
 *  \param frame Received frame
 */
void serialAdapter_processFrame(frame_t* frame)
{

    if (frame->header.length > COMM_MAX_PAYLOAD_LENGTH + sizeof(uint8_t) || frame->header.length < sizeof(command_t))
    {
        return;
    }

    //printFrame(frame, "serialAdapter_processFrame");

    switch (frame->innerFrame.command)
    {
        case CMD_SET_LED:
        {
            if (frame->header.length - sizeof(command_t) != sizeof(cmd_setLed_t))
            {
                return;
            }
            else
            {
                // printf_P(PSTR("Setting LED to %d\n"), ((cmd_setLed_t *)&(frame->innerFrame.payload))->enable);
                rfAdapter_receiveSetLed(((cmd_setLed_t*)&(frame->innerFrame.payload)));
            }
        }
        break;
        case CMD_TOGGLE_LED:
        {
            if (frame->header.length - sizeof(command_t) != 0)
            {
                return;
            }
            else
            {
                // printf_P(PSTR("Toggling LED\n"));
                rfAdapter_receiveToggleLed();
            }
        }
        break;

        case CMD_LCD_CLEAR:
        {
            if (frame->header.length - sizeof(command_t) != 0)
            {
                return;
            }
            else
            {
                // printf_P(PSTR("Clearing LCD\n"));
                rfAdapter_receiveLcdClear();
            }
        }
        break;

        case CMD_LCD_GOTO:
        {
            if (frame->header.length - sizeof(command_t) != sizeof(cmd_lcdGoto_t))
            {
                printf_P(PSTR("Invalid length for CMD_LCD_GOTO. Length w/o command is %d instead of %d\n"), frame->header.length - sizeof(command_t), sizeof(cmd_lcdGoto_t));
                return;
            }
            else
            {
                printf_P(PSTR("Goto LCD: %d, %d\n"), ((cmd_lcdGoto_t*)&(frame->innerFrame.payload))->x, ((cmd_lcdGoto_t*)&(frame->innerFrame.payload))->y);
                rfAdapter_receiveLcdGoto((cmd_lcdGoto_t*)&(frame->innerFrame.payload));
            }
        }
        break;

        case CMD_LCD_PRINT:
        {
            // cmd_lcdPrint_t toprnt = *(cmd_lcdPrint_t*)&(frame->innerFrame.payload);

            if (frame->header.length - sizeof(command_t) > sizeof(cmd_lcdPrint_t))
            {
                printf_P(PSTR("Invalid length for CMD_LCD_PRINT. Length w/o command is %d instead of %d\n"), frame->header.length - sizeof(command_t), sizeof(cmd_lcdPrint_t));
                return;
            }
            else if ((*(cmd_lcdPrint_t*)&(frame->innerFrame.payload)).length != frame->header.length - sizeof(command_t) - sizeof((*(cmd_lcdPrint_t*)&(frame->innerFrame.payload)).length))
            {
                printf_P(PSTR("Invalid length for CMD_LCD_PRINT. Length of expected String is not equal to length described in cmd_lcdPrint_t object\n"), frame->header.length - sizeof(command_t), sizeof(cmd_lcdPrint_t));
            }
            else
            {
                printf_P(PSTR("Printing to LCD: %s\n"), ((cmd_lcdPrint_t*)&(frame->innerFrame.payload))->message);

                rfAdapter_receiveLcdPrint((cmd_lcdPrint_t*)&(frame->innerFrame.payload));
            }
        }
        break;

        case CMD_SENSOR_DATA:
        {
            uint8_t test1 = frame->header.length - sizeof(command_t);
            uint8_t test2 = sizeof(cmd_sensorData_t);

            if (test1 != test2)
            {
                DEBUG("Invalid length for CMD_SENSOR_DATA. Length w/o command is %d instead of %d\n\n\n\n\n\n", test1, test2);
            }
            else
            {
                cmd_sensorData_t payload = *(cmd_sensorData_t*)frame->innerFrame.payload;

                sensor_data_t sensor_data;
                sensor_data.sensor_src_address = frame->header.srcAddr;
                sensor_data.sensor_type = payload.sensor;
                sensor_data.sensor_data_type = payload.paramType;
                sensor_data.sensor_data_value = payload.param;
                sensor_data.sensor_last_update = getSystemTime_ms();

                rfAdapter_receiveSensorData(&sensor_data);
            }
        }
        break;

        default:
            return;
    }
}

void rfAdapter_receiveSensorData(sensor_data_t* sensor_data)
{
    switch (sensor_data->sensor_type)
    {
        case SENSOR_LPS28DFW: // BOL Sensor (1,3) // 11
        {
            // printf("rfAdapter_receiveSensorData() for BOL Sensor\n");
            if (sensor_data->sensor_data_type == PARAM_PRESSURE_PASCAL) //float
            {
                enqueue_sensor_data_into_buffer(sensor_data);
            }
            else
            {
                printf_P(PSTR("rfAdapter_receiveSensorData() ignored %d Value of BOL Sensor\n\n\n\n\n\n"), sensor_data->sensor_data_type);
            }
        }
        break;

        case SENSOR_SHTC3: // Adil Sensor adress (1,7) // 15
        {
            if (sensor_data->sensor_data_type == PARAM_HUMIDITY_PERCENT) //float
            {
                enqueue_sensor_data_into_buffer(sensor_data);
            }
            else
            {
                printf_P(PSTR("rfAdapter_receiveSensorData() ignored %d Value of Adil Sensor\n\n\n\n"), sensor_data->sensor_data_type);
            }
        }
        break;

        case SENSOR_SCD41: // Richard Sensor Addresse(1, 6) // 14
        {
            if (sensor_data->sensor_data_type == PARAM_CO2_PPM) //uint
            {

                enqueue_sensor_data_into_buffer(sensor_data);
            }
            else
            {
                printf_P(PSTR("rfAdapter_receiveSensorData() ignored %d Value of Richard Sensor\n\n\n\n"), sensor_data->sensor_data_type);
            }
        }
        break;
        case SENSOR_SGP40: // Niklas Sensor Addresse(1,8) //8
        {
            if (sensor_data->sensor_data_type == PARAM_TVOC_PPB) //uint
            {
                enqueue_sensor_data_into_buffer(sensor_data);
            }
            else
            {
                printf_P(PSTR("rfAdapter_receiveSensorData() ignored %d Value of Niklas Sensor\n\n\n\n"), sensor_data->sensor_data_type);
            }
        }
        break;
        case SENSOR_TMP117: // Jannick Sensor Adresse(1,5) //13
        {
            if (sensor_data->sensor_data_type == PARAM_TEMPERATURE_CELSIUS) //float
            {
                enqueue_sensor_data_into_buffer(sensor_data);
            }
            else
            {
                printf_P(PSTR("rfAdapter_receiveSensorData() ignored %d Value of Jannick Sensor\n\n\n\n"), sensor_data->sensor_data_type);
            }
        }
        break;

        default:
        {
            printf_P(PSTR("rfAdapter_receiveSensorData() ignored %d Value of unknown Sensor\n\n\n\n"), sensor_data->sensor_data_type);
        }
        break;
    }
}

/*!
 *  Handler that's called when command CMD_SET_LED was received
 *
 *  \param data Payload of received frame
 */
void rfAdapter_receiveSetLed(cmd_setLed_t* data)
{
    // printf("rfAdapter_receiveSetLed()");
    if ((bool)data->enable)
    {
        PORTB |= (1 << PB7); // on
    }
    else
    {
        PORTB &= ~(1 << PB7); // off
    }
}

/*!
 *  Handler that's called when command CMD_TOGGLE_LED was received
 */
void rfAdapter_receiveToggleLed()
{
    // printf("rfAdapter_receiveToggleLed()");
    PORTB ^= (1 << PB7);
}

/*!
 *  Handler that's called when command CMD_LCD_CLEAR was received
 */
void rfAdapter_receiveLcdClear()
{
    // printf("rfAdapter_receiveLcdClear()");
    lcd_clear();
}

/*!
 *  Handler that's called when command CMD_LCD_GOTO was received
 *
 *  \param data Payload of received frame
 */
void rfAdapter_receiveLcdGoto(cmd_lcdGoto_t* data)
{
    // printf("rfAdapter_receiveLcdGoto()");
    lcd_goto(data->x, data->y);
}

/*!
 *  Handler that's called when command CMD_LCD_PRINT was received
 *
 *  \param data Payload of received frame
 */
void rfAdapter_receiveLcdPrint(cmd_lcdPrint_t* data)
{
    // printf("rfAdapter_receiveLcdPrint()");
    if (data->length > 32)
        return;

    char buffer[33];
    memcpy(buffer, data->message, data->length);
    buffer[data->length] = '\0';
    lcd_writeString(buffer);
}

/*!
 *  Sends a frame with command CMD_SET_LED
 *
 *  \param destAddr Where to send the frame
 *  \param enable Whether the receiver should enable or disable their led
 */
void rfAdapter_sendSetLed(address_t destAddr, bool enable)
{

    inner_frame_t inner_frame;
    inner_frame.command = CMD_SET_LED;
    inner_frame.payload[0] = enable;

    int size = sizeof(command_t) + sizeof(enable);

    printf("rfAdapter_sendSetLed() with size: %d\n", size);

    serialAdapter_writeFrame(destAddr, size, &inner_frame);
}

/*!
 *  Sends a frame with command CMD_TOGGLE_LED
 *
 *  \param destAddr Where to send the frame
 */
void rfAdapter_sendToggleLed(address_t destAddr)
{
    inner_frame_t inner_frame;
    inner_frame.command = CMD_TOGGLE_LED;

    int size = sizeof(command_t);

    printf("rfAdapter_sendToggleLed() with size: %d\n", size);

    serialAdapter_writeFrame(destAddr, size, &inner_frame);
}

/*!
 *  Sends a frame with command CMD_LCD_CLEAR
 *
 *  \param destAddr Where to send the frame
 */
void rfAdapter_sendLcdClear(address_t destAddr)
{
    inner_frame_t inner_frame;
    inner_frame.command = CMD_LCD_CLEAR;

    int size = sizeof(command_t);

    printf("rfAdapter_sendLcdClear() with size: %d\n", size);

    serialAdapter_writeFrame(destAddr, size, &inner_frame);
}

/*!
 *  Sends a frame with command CMD_LCD_GOTO
 *
 *  \param destAddr Where to send the frame
 *  \param x Which column should be selected by the receiver
 *  \param y Which row should be selected by the receiver
 */
void rfAdapter_sendLcdGoto(address_t destAddr, uint8_t x, uint8_t y)
{
    inner_frame_t inner_frame;
    inner_frame.command = CMD_LCD_GOTO;

    cmd_lcdGoto_t cmd;
    cmd.x = x;
    cmd.y = y;
    memcpy(&inner_frame.payload, &cmd, sizeof(cmd));

    int size = sizeof(command_t) + sizeof(cmd_lcdGoto_t);

    printf("rfAdapter_sendLcdGoto() with size: %d\n", size);

    serialAdapter_writeFrame(destAddr, size, &inner_frame);
}

/*!
 *  Sends a frame with command CMD_LCD_PRINT
 *
 *  \param destAddr Where to send the frame
 *  \param message Which message should be printed on receiver side
 */
void rfAdapter_sendLcdPrint(address_t destAddr, const char* message)
{
    inner_frame_t inner_frame;
    inner_frame.command = CMD_LCD_PRINT;

    cmd_lcdPrint_t* print;
    print = (cmd_lcdPrint_t*)&inner_frame.payload;

    print->length = strlen(message) < 32 ? strlen(message) : 32;

    memcpy(print->message, message, print->length);

    printf("rfAdapter_sendLcdPrint() with size: %d\n", print->length);

    serialAdapter_writeFrame(destAddr, sizeof(command_t) + print->length + 1, &inner_frame);
}

/*!
 *  Sends a frame with command CMD_LCD_PRINT
 *
 *  \param destAddr Where to send the frame
 *  \param message Which message should be printed on receiver side as address to program memory. Use PSTR for creating strings on program memory
 */
void rfAdapter_sendLcdPrintProcMem(address_t destAddr, const char* message)
{
    inner_frame_t inner_frame;
    inner_frame.command = CMD_LCD_PRINT;

    cmd_lcdPrint_t* print;
    print = (cmd_lcdPrint_t*)&inner_frame.payload;

    print->length = strlen_P(message) < 32 ? strlen_P(message) : 32;

    memcpy_P(print->message, message, print->length);

    printf("rfAdapter_sendLcdPrint() with size: %d\n", print->length);

    serialAdapter_writeFrame(destAddr, sizeof(command_t) + print->length + 1, &inner_frame);
}

void print_sensor_data(sensor_data_t* sensor_data)
{
    printf_P(PSTR("{\nsensor_src_address: %d\n"), sensor_data->sensor_src_address);
    printf_P(PSTR("sensor_type: %d\n"), sensor_data->sensor_type);
    printf_P(PSTR("sensor_data_type: %d\n"), sensor_data->sensor_data_type);
    printf_P(PSTR("sensor_data_value: %f\n"), sensor_data->sensor_data_value.fValue);
    printf_P(PSTR("sensor_last_update: %d\n"), sensor_data->sensor_last_update);
    printf_P(PSTR("}\n"));
}