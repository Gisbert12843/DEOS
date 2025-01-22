/*!
 *  \brief Layer built on top of serialAdapter where commands are defined.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef RF_ADAPTER_H_
#define RF_ADAPTER_H_

#include "serialAdapter.h"
#include "sensorData.h"

#include <stdbool.h>

#define ADDRESS(teamId, subId) ((address_t)((teamId << 3) & 0b11111000) | (subId & 0b00000111))
#define INITIAL_CHECKSUM_VALUE ((checksum_t)0)


//! Unique command IDs
typedef enum rfAdapterCommand
{
    CMD_SET_LED = 0x01,
    CMD_TOGGLE_LED = 0x02,
    CMD_LCD_CLEAR = 0x10,
    CMD_LCD_GOTO = 0x11,
    CMD_LCD_PRINT = 0x12,
    CMD_SENSOR_DATA = 0x20
} rfAdapterCommand_t;

//! Command payload of command CMD_SET_LED
typedef struct cmd_setLed
{
    uint8_t enable;
} cmd_setLed_t;

//! Command payload of command CMD_LCD_GOTO
typedef struct cmd_lcdGoto
{
    uint8_t x;
    uint8_t y;
} cmd_lcdGoto_t;

//! Command payload of command CMD_LCD_PRINT
typedef struct cmd_lcdPrint
{
    uint8_t length;
    char message[32];
} cmd_lcdPrint_t;



void print_sensor_data(sensor_data_t* sensor_data);


//! Initializes adapter
void rfAdapter_init();

//! Returns true if the communication has been initialized
uint8_t rfAdapter_isInitialized();

//! Main task of adapter
void rfAdapter_worker();

//! Is called on command frame receive
void serialAdapter_processFrame(frame_t* frame);

//! Sends a frame with command CMD_SET_LED
void rfAdapter_sendSetLed(address_t destAddr, bool enable);

//! Sends a frame with command CMD_TOGGLE_LED
void rfAdapter_sendToggleLed(address_t destAddr);

//! Sends a frame with command CMD_LCD_CLEAR
void rfAdapter_sendLcdClear(address_t destAddr);

//! Sends a frame with command CMD_LCD_GOTO
void rfAdapter_sendLcdGoto(address_t destAddr, uint8_t x, uint8_t y);

//! Sends a frame with command CMD_LCD_PRINT
void rfAdapter_sendLcdPrint(address_t destAddr, const char* message);

//! Sends a frame with command CMD_LCD_PRINT with a message from program memory
void rfAdapter_sendLcdPrintProcMem(address_t destAddr, const char* message);

void rfAdapter_receiveSensorData(sensor_data_t* sensor_data);

#endif /* RF_ADAPTER_H_ */