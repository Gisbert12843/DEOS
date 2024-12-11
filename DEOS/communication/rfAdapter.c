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
address_t serialAdapter_address = ADDRESS(1, 0);

//----------------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------------

void rfAdapter_receiveSetLed(cmd_setLed_t *);
void rfAdapter_receiveToggleLed();
void rfAdapter_receiveLcdGoto(cmd_lcdGoto_t *);
void rfAdapter_receiveLcdPrint(cmd_lcdPrint_t *);
void rfAdapter_receiveLcdClear();

//----------------------------------------------------------------------------
// Your Homework
//----------------------------------------------------------------------------

/*!
 *  Initializes the rfAdapter and their dependencies
 */
void rfAdapter_init()
{
	#warning [Praktikum 3] Implement here
}

/*!
 * Check if adapter has been initialized
 *
 * \return True if the communication has been initialized
 */
uint8_t rfAdapter_isInitialized()
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Main task of adapter
 */
void rfAdapter_worker()
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Is called on command frame receive
 *
 *  \param frame Received frame
 */
void serialAdapter_processFrame(frame_t *frame)
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Handler that's called when command CMD_SET_LED was received
 *
 *  \param data Payload of received frame
 */
void rfAdapter_receiveSetLed(cmd_setLed_t *data)
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Handler that's called when command CMD_TOGGLE_LED was received
 */
void rfAdapter_receiveToggleLed()
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Handler that's called when command CMD_LCD_CLEAR was received
 */
void rfAdapter_receiveLcdClear()
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Handler that's called when command CMD_LCD_GOTO was received
 *
 *  \param data Payload of received frame
 */
void rfAdapter_receiveLcdGoto(cmd_lcdGoto_t *data)
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Handler that's called when command CMD_LCD_PRINT was received
 *
 *  \param data Payload of received frame
 */
void rfAdapter_receiveLcdPrint(cmd_lcdPrint_t *data)
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Sends a frame with command CMD_SET_LED
 *
 *  \param destAddr Where to send the frame
 *  \param enable Whether the receiver should enable or disable their led
 */
void rfAdapter_sendSetLed(address_t destAddr, bool enable)
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Sends a frame with command CMD_TOGGLE_LED
 *
 *  \param destAddr Where to send the frame
 */
void rfAdapter_sendToggleLed(address_t destAddr)
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Sends a frame with command CMD_LCD_CLEAR
 *
 *  \param destAddr Where to send the frame
 */
void rfAdapter_sendLcdClear(address_t destAddr)
{
	#warning [Praktikum 3] Implement here
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
	#warning [Praktikum 3] Implement here
}

/*!
 *  Sends a frame with command CMD_LCD_PRINT
 *
 *  \param destAddr Where to send the frame
 *  \param message Which message should be printed on receiver side
 */
void rfAdapter_sendLcdPrint(address_t destAddr, const char *message)
{
	#warning [Praktikum 3] Implement here
}

/*!
 *  Sends a frame with command CMD_LCD_PRINT
 *
 *  \param destAddr Where to send the frame
 *  \param message Which message should be printed on receiver side as address to program memory. Use PSTR for creating strings on program memory
 */
void rfAdapter_sendLcdPrintProcMem(address_t destAddr, const char *message)
{
	#warning [Praktikum 3] Implement here
}