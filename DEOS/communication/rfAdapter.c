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
#include "rfAdapter.h"

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
void serialAdapter_processFrame(frame_t *frame)
{

	if (frame->header.length > COMM_MAX_PAYLOAD_LENGTH + sizeof(uint8_t) || frame->header.length < sizeof(command_t))
	{
		return;
	}

	printFrame(frame, "serialAdapter_processFrame");

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
			printf_P(PSTR("Setting LED to %d\n"), ((cmd_setLed_t *)&(frame->innerFrame.payload))->enable);
			rfAdapter_receiveSetLed(((cmd_setLed_t *)&(frame->innerFrame.payload)));
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
			printf_P(PSTR("Toggling LED\n"));
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
			printf_P(PSTR("Clearing LCD\n"));
			rfAdapter_receiveLcdClear();
		}
	}
	break;

	case CMD_LCD_GOTO:
	{
		if (frame->header.length - sizeof(command_t) != sizeof(cmd_lcdGoto_t))
		{
			return;
		}
		else
		{
			printf_P(PSTR("Goto LCD: %d, %d\n"), ((cmd_lcdGoto_t *)&(frame->innerFrame.payload))->x, ((cmd_lcdGoto_t *)&(frame->innerFrame.payload))->y);
			rfAdapter_receiveLcdGoto((cmd_lcdGoto_t *)&(frame->innerFrame.payload));
		}
	}
	break;

	case CMD_LCD_PRINT:
	{
		if (frame->header.length - sizeof(command_t) != sizeof(cmd_lcdPrint_t))
		{
			return;
		}
		else
		{
			printf_P(PSTR("Printing to LCD: %s\n"), ((cmd_lcdPrint_t *)&(frame->innerFrame.payload))->message);
			rfAdapter_receiveLcdPrint((cmd_lcdPrint_t *)&(frame->innerFrame.payload));
		}
	}
	break;

	case CMD_SENSOR_DATA:
	{
#warning TODO!!!
	}
	break;

	default:
		return;
	}
}

/*!
 *  Handler that's called when command CMD_SET_LED was received
 *
 *  \param data Payload of received frame
 */
void rfAdapter_receiveSetLed(cmd_setLed_t *data)
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
void rfAdapter_receiveLcdGoto(cmd_lcdGoto_t *data)
{
	// printf("rfAdapter_receiveLcdGoto()");
	lcd_goto(data->x, data->y);
}

/*!
 *  Handler that's called when command CMD_LCD_PRINT was received
 *
 *  \param data Payload of received frame
 */
void rfAdapter_receiveLcdPrint(cmd_lcdPrint_t *data)
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

	printf("rfAdapter_sendSetLed() with size: %d\n",size );


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

	serialAdapter_writeFrame(destAddr,size, &inner_frame);
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

	int size = sizeof(sizeof(command_t) + sizeof(cmd_lcdGoto_t));

	printf("rfAdapter_sendLcdGoto() with size: %d\n", size);

	serialAdapter_writeFrame(destAddr, size, &inner_frame);
}

/*!
 *  Sends a frame with command CMD_LCD_PRINT
 *
 *  \param destAddr Where to send the frame
 *  \param message Which message should be printed on receiver side
 */
void rfAdapter_sendLcdPrint(address_t destAddr, const char *message)
{
	// printf("rfAdapter_sendLcdPrint()");
	inner_frame_t inner_frame;
	inner_frame.command = CMD_LCD_PRINT;

	cmd_lcdPrint_t cmd;
	cmd.length = strlen(message);
	if (cmd.length > 32)
		cmd.length = 32;

	strncpy(cmd.message, message, cmd.length);
	cmd.message[cmd.length] = '\0';

	memcpy(inner_frame.payload, &cmd, sizeof(cmd));

	int size = sizeof(command_t) + sizeof(cmd_lcdPrint_t);

	printf("rfAdapter_sendLcdPrint() with size: %d\n", size);

	serialAdapter_writeFrame(destAddr, size, &inner_frame);
}

/*!
 *  Sends a frame with command CMD_LCD_PRINT
 *
 *  \param destAddr Where to send the frame
 *  \param message Which message should be printed on receiver side as address to program memory. Use PSTR for creating strings on program memory
 */
void rfAdapter_sendLcdPrintProcMem(address_t destAddr, const char *message)
{
	inner_frame_t inner_frame;
	inner_frame.command = CMD_LCD_PRINT;

	printf("rfAdapter_sendLcdPrintProcMem() with size: %d\n", sizeof(inner_frame));

	// Send the frame
	serialAdapter_writeFrame(destAddr, sizeof(inner_frame), &inner_frame);
}