/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "tlcd_event_parser.h"
#include "../lib/lcd.h"
#include "../lib/util.h"
#include "../os_core.h"
#include "../os_scheduler.h"
#include "../spi/spi.h"
#include "tlcd_button.h"
#include "tlcd_core.h"

//! Globals
EventCallback *eventCallback;

//! Forward declarations
void readDataIntoBuffer();
void parseInputBuffer();
void parseTouchEvent();
void parseButtonEvent();
void parseUnknownEvent();

/*!
 *  This function reads a byte from the SPI interface and updates the given BCC.\
 *
 *  \param bcc The current BCC value to be updated
 *  \param length The length value to be decremented
 */
uint8_t read(uint8_t *bcc, uint8_t *length)
{
	uint8_t data = spi_read();
	tlcd_calculateBCC(bcc, &data, 1);
	*length -= 1;
	return data;
}

/*!
 *  Handles incoming data and calls the parsing process.
 */
void tlcd_event_worker()
{
	// DEBUG("tlcd_event_worker");
	os_enterCriticalSection();
	spi_cs_enable();
	tlcd_requestData();

	uint8_t len = 0;
	uint8_t bcc = INITIAL_BCC_VALUE;
	uint8_t type;
	uint8_t byte;

	// read header
	if (read(&bcc, &len) != DC1_BYTE)
	{
		spi_cs_disable();
		os_leaveCriticalSection();
		return;
	}

	// read length
	len = read(&bcc, &len);

	while (len >= 2)
	{
		byte = read(&bcc, &len);
		if (byte != ESC_BYTE)
		{
			spi_cs_disable();
			os_leaveCriticalSection();
			return;
		}

		type = read(&bcc, &len);

		if (type == H_BYTE)
		{
			parseTouchEvent(&bcc, &len);
		}
		else
		{
			parseUnknownEvent(&bcc, &len);
		}
	}

	byte = spi_read();
	if (byte != bcc)
	{

		// lcd_writeHexByte(byte);
		// os_error("BCC failed afterwards");
		//  ERROR: Event handlers got called although the data got corrupted. This is a case that's unhandled!
	}
	spi_cs_disable();
	os_leaveCriticalSection();
}

/*!
 *  Display a TouchEvent to the Character-LCD.
 *
 *  \param event The TouchEvent to display
 */
void tlcd_displayEvent(touch_event_t event)
{
	lcd_clear();
	lcd_writeProgString(PSTR("Touch: "));
	switch (event.type)
	{
	case TOUCHPANEL_UP:
		lcd_writeProgString(PSTR("UP"));
		break;
	case TOUCHPANEL_DOWN:
		lcd_writeProgString(PSTR("DOWN"));
		break;
	case TOUCHPANEL_DRAG:
		lcd_writeProgString(PSTR("DRAG"));
		break;
	}
	lcd_line2();
	lcd_writeProgString(PSTR("X: "));
	lcd_writeDec(event.x);
	lcd_goto(2, 7);
	lcd_writeProgString(PSTR("Y: "));
	lcd_writeDec(event.y);
}

/*!
 *	This function is called when a free touch panel event packet
 *	has been received. The content of the subframe corresponding
 *	to that event is parsed in this function and delegated to the
 *	event handler tlcd_eventHandler(WithCorrection).
 *
 *	\param bcc The current BCC value to be updated
 * 	\param length The length value to be decremented
 */
void parseTouchEvent(uint8_t *bcc, uint8_t *length)
{
	touch_event_t touchEvent;
	uint8_t x_low;
	uint8_t y_low;

	if (*length < 6)
	{
		// Should be at least 6 (len + 5 data bytes)
		return;
	}

	read(bcc, length); // omit size, always 5

	touchEvent.type = read(bcc, length);

	x_low = read(bcc, length);
	touchEvent.x = ((read(bcc, length) << 8) | x_low); // combine low and high byte

	y_low = read(bcc, length);
	touchEvent.y = ((read(bcc, length) << 8) | y_low);

	if (tlcd_handleButtons(touchEvent))
	{
		touchEvent.type = TOUCHPANEL_UP;
	}
	if (eventCallback != 0)
	{
		eventCallback(touchEvent);
	}

	// tlcd_displayEvent(touchEvent);
}

/*!
 *	This function is called when an unknown event packet
 *	has been received and skips the payload by manipulating
 *  the tail of the input buffer.
 *
 *  \param bcc The current BCC value to be updated
 *  \param length The length value to be decremented
 */
void parseUnknownEvent(uint8_t *bcc, uint8_t *length)
{
	// skip packet
	uint8_t len = read(bcc, length);
	for (uint8_t i = 0; i < len && *length; i++)
	{
		uint8_t byte = read(bcc, length);
		lcd_writeHexByte(byte);
	}
}

/*!
 *  Set the function to be called if a TouchEvent is received.
 *
 *  \param callback The function to be called
 */
void tlcd_event_setCallback(EventCallback *callback)
{
	eventCallback = callback;
}