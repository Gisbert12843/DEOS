/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "terminal.h"
#include "../os_scheduler.h"
#include "util.h"
#include <stdio.h>

#define BAUD 250000
#include <util/setbaud.h>

//----------------------------------------------------------------------------
// Configuration of stdio.h
//----------------------------------------------------------------------------
int stdio_put_char(char c, FILE *stream)
{
    os_enterCriticalSection();

    terminal_writeChar(c);
    if (c == '\n') { terminal_writeProgString(PSTR("        ")); }

    os_leaveCriticalSection();
    return 0;
}

void terminal_log_printf_p(const char *prefix, const char *fmt, ...)
{
    os_enterCriticalSection();

    terminal_writeProgString(prefix);

    va_list args;
    va_start(args, fmt);
    stdout->flags |= __SPGM;
    vfprintf_P(stdout, fmt, args);
    stdout->flags &= ~__SPGM;
    va_end(args);

    terminal_newLine();

    os_leaveCriticalSection();
}

FILE mystdout = FDEV_SETUP_STREAM(stdio_put_char, NULL, _FDEV_SETUP_WRITE);

//----------------------------------------------------------------------------
// USB port
//----------------------------------------------------------------------------

/*!
 * Initializes UART2 connected to the USB port
 */
void usb2_init()
{
	os_enterCriticalSection();
	
	// Set baud
	UBRR2 = UBRR_VALUE;
		
	// Set double speed if required by baud configuration
	#if USE_2X
	sbi(UCSR2A, U2X2);
	#else
	cbi(UCSR2A, U2X2);
	#endif
		
	// Enable receiver & transmitter
	sbi(UCSR2B, RXEN2);
	sbi(UCSR2B, TXEN2);
	
	os_leaveCriticalSection();
}

/*!
 *  Polls for incoming bytes at the USB port
 */
uint8_t usb2_read(void)
{
	// Wait for data to be received
	while (!gbi(UCSR2A, RXC2));

	// Get and return received data from buffer
	return UDR2;
}

/*!
 *  Transmits one byte to the USB port
 */
void usb2_write(uint8_t data)
{
	// Wait for empty transmit buffer
	while (!gbi(UCSR2A, UDRE2));

	// Put data into buffer, send the data
	UDR2 = data;
}

/*!
 *  Transmits the given string to the USB port
 */
void usb2_writeString(char *text)
{
	os_enterCriticalSection();
	
	for (uint8_t i = 0; i < UINT8_MAX; i++)
	{
		if (text[i] == '\n') { usb2_write('\r'); }
		if (text[i] == 0) { break; }
		usb2_write(text[i]);
	}
	
	os_leaveCriticalSection();
}

/*!
 *  Transmits the given prog string to the USB port, max. 255 chars
 */
void usb2_writeProgString(const char *text)
{
	os_enterCriticalSection();
	
	for (uint8_t i = 0; i < UINT8_MAX; i++)
	{
		char c = (char) pgm_read_byte(&text[i]);
		if (c == '\n') { usb2_write('\r'); }
		if (c == '\0') { break; }
		usb2_write(c);
	}

	os_leaveCriticalSection();
}

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------
/*!
 *  Initialize the terminal
 */
void terminal_init()
{
	usb2_init();
	
    stdout = &mystdout;
}

/*!
 *  Write a half-byte (a nibble)
 *
 *  \param number  The number to be written.
 */
void terminal_writeHexNibble(uint8_t number)
{
    if (number < 10) { usb2_write('0' + number); }
    else             { usb2_write('A' + number - 10); }
}

/*!
 *  Write one hexadecimal byte
 *
 *  \param number  The number to be written.
 */
void terminal_writeHexByte(uint8_t number)
{
    os_enterCriticalSection();

    terminal_writeHexNibble(number >> 4);
    terminal_writeHexNibble(number & 0xF);

    os_leaveCriticalSection();
}

/*!
 *  Write one hexadecimal word
 *
 *  \param number  The number to be written.
 */
void terminal_writeHexWord(uint16_t number)
{
    os_enterCriticalSection();

    terminal_writeHexByte(number >> 8);
    terminal_writeHexByte(number);

    os_leaveCriticalSection();
}

/*!
 *  Write a word as a decimal number without prefixes
 *
 *  \param number  The number to be written.
 */
void terminal_writeDec(uint16_t number)
{
    if (!number)
    {
        terminal_writeChar('0');
        return;
    }

    uint32_t pos = 10000;
    uint8_t print = 0;

	os_enterCriticalSection();
	
    do
    {
        uint8_t const digit = number / pos;
        number -= digit * pos;
        if (print |= digit) { terminal_writeChar(digit + '0'); }
    } while (pos /= 10);

    os_leaveCriticalSection();
}

/*!
 *  Write char to terminal
 *
 *  \param character  The character to be written.
 */
void terminal_writeChar(char character)
{
    if (character == '\n') { usb2_write('\r'); }
    usb2_write(character);
}

/*!
 *  Write string on terminal
 *
 *  \param str  The string to be written (a pointer to the first character).
 */
void terminal_writeString(char *str)
{
    usb2_writeString(str);
}

/*!
 *  Write char PROGMEM* string to terminal
 *
 *  \param pstr  The string to be written (a pointer to the first character).
 */
void terminal_writeProgString(const char *pstr)
{
	usb2_writeProgString(pstr);
}

/*!
 *  Write a new line
 */
void terminal_newLine()
{
    terminal_writeChar('\n');
}