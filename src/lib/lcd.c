#include "lcd.h"
#include "../os_scheduler.h"
#include <avr/pgmspace.h>
#include <stdio.h>

//----------------------------------------------------------------------------
// Configuration of stdio.h
//----------------------------------------------------------------------------
int lcd_stdioPutChar(char c, FILE *stream)
{
	lcd_writeChar(c);
	return 0;
}

/*!
 * \param fmt  The format string as progstr
 * \param ...  The arguments to be formatted
 */
void lcd_printf_p(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	stdout->flags |= __SPGM;
	vfprintf_P(&lcd_stdout, fmt, args);
	stdout->flags &= ~__SPGM;
	va_end(args);
}

FILE lcd_stdout = FDEV_SETUP_STREAM(lcd_stdioPutChar, NULL, _FDEV_SETUP_WRITE);

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------

//! Global var, stores character count.
/*!
 *  \internal
 *  This value is in [0;32]
 *       ... yes, this is no mistake it can be both 0 and 32
 */
uint8_t charCtr;

/*!
 *  Send a pulse to the EN pin to latch data/command.
 */
static void lcd_enablePulse(void)
{
	os_enterCriticalSection();
	LCD_EN_HIGH();
	_delay_us(1); // Enable pulse must be >450ns
	LCD_EN_LOW();
	_delay_us(100); // Commands need >37us to settle
	os_leaveCriticalSection();
}

/*!
 *  Send a nibble (4 bits) to the LCD.
 *
 *  \param nibble  The 4-bit data to send (lower nibble ignored)
 */
static void lcd_sendNibble(uint8_t nibble)
{
	os_enterCriticalSection();
	if (nibble & 0x01)
		LCD_D4_HIGH();
	else
		LCD_D4_LOW();
	if (nibble & 0x02)
		LCD_D5_HIGH();
	else
		LCD_D5_LOW();
	if (nibble & 0x04)
		LCD_D6_HIGH();
	else
		LCD_D6_LOW();
	if (nibble & 0x08)
		LCD_D7_HIGH();
	else
		LCD_D7_LOW();

	lcd_enablePulse();
	os_leaveCriticalSection();
}

/*!
 *  Initialize the LCD in 4-bit mode.
 */
void lcd_init(void)
{
	os_enterCriticalSection();
	// Set pin directions to output
	DDRH |= (1 << LCD_RS_PIN) | (1 << LCD_EN_PIN) | (1 << LCD_D6_PIN) | (1 << LCD_D7_PIN);
	DDRE |= (1 << LCD_D5_PIN);
	DDRG |= (1 << LCD_D4_PIN);
	DDRF |= (1 << LCD_RW_PIN);

	// RW is always LOW (write mode)
	LCD_RW_LOW();

	// Initialization sequence
	_delay_ms(50); // Wait for more than 40ms after Vcc rises to 2.7V

	lcd_sendNibble(0x03); // Function set: 8-bit mode
	_delay_ms(5);		  // Wait for more than 4.1ms

	lcd_sendNibble(0x03); // Function set: 8-bit mode
	_delay_us(200);		  // Wait for more than 100µs

	lcd_sendNibble(0x03); // Function set: 8-bit mode
	_delay_us(200);

	lcd_sendNibble(0x02); // Function set: 4-bit mode

	lcd_sendCommand(LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS);
	lcd_sendCommand((LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON) & ~LCD_CURSOR_ON & ~LCD_BLINK_ON);
	lcd_sendCommand(LCD_CMD_CLEAR_DISPLAY);
	lcd_sendCommand(LCD_CMD_ENTRY_MODE_SET | 0x02); // Increment cursor, no display shift

	_delay_ms(5);
	os_leaveCriticalSection();
}

/*!
 *  Clear the display and set the cursor to the home position.
 */
void lcd_clear(void)
{
	os_enterCriticalSection();
	charCtr = 0;
	lcd_sendCommand(LCD_CMD_CLEAR_DISPLAY);
	_delay_ms(2); // Clearing the display requires a delay
	os_leaveCriticalSection();
}

/*!
 *  Set the cursor to the home position.
 */
void lcd_home(void)
{
	os_enterCriticalSection();
	lcd_sendCommand(LCD_CMD_RETURN_HOME);
	_delay_ms(2); // Returning home requires a delay
	os_leaveCriticalSection();
}

/*!
 *  Turn the LCD display on.
 */
void lcd_displayOn(void)
{
	lcd_sendCommand((LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON) & ~LCD_CURSOR_ON & ~LCD_BLINK_ON);
}

/*!
 *  Turn the LCD display off.
 */
void lcd_displayOff(void)
{
	lcd_sendCommand(LCD_CMD_DISPLAY_CONTROL & ~LCD_DISPLAY_ON & ~LCD_CURSOR_ON & ~LCD_BLINK_ON);
}

/*!
 *  Turn the LCD cursor on.
 */
void lcd_cursorOn(void)
{
	lcd_sendCommand((LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_ON) & ~LCD_BLINK_ON);
}

/*!
 *  Turn the LCD cursor off.
 */
void lcd_cursorOff(void)
{
	lcd_sendCommand((LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON) & ~LCD_CURSOR_ON & ~LCD_BLINK_ON);
}

/*!
 *  Enable the cursor blink feature.
 */
void lcd_blinkOn(void)
{
	lcd_sendCommand(LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_ON);
}

/*!
 *  Disable the cursor blink feature.
 */
void lcd_blinkOff(void)
{
	lcd_sendCommand((LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_ON) & ~LCD_BLINK_ON);
}

/*!
 *  Set the cursor to the specified position.
 *
 *  \param row  The row position (0 or 1 for a 2-line display)
 *  \param col  The column position (0-indexed)
 */
void lcd_goto(uint8_t row, uint8_t col)
{

	if (row > 1)
		row = 1; // We only support two lines

	os_enterCriticalSection();
	lcd_sendCommand(LCD_CMD_SET_DDRAM_ADDR | (col + 0x40 * row));
	charCtr = row * 16 + col;
	os_leaveCriticalSection();
}

/*!
 *  Print a string to the LCD.
 *
 *  \param str  The null-terminated string to print
 */
void lcd_writeString(char *string)
{
	char c;
	os_enterCriticalSection();

	while ((c = *(string++)) != '\0')
	{
		lcd_writeChar(c);
	}

	os_leaveCriticalSection();
}

/*!
 *  Writes a string of 8-Bit ASCII-values from the program flash memory to
 *  the LCD. This function features automated line breaks as long as you
 *  don't manually do something with the cursor. This works fine in concert
 *  with lcd_write_char without messing up anything.\n
 *  After 32 character have been written, the LCD will be erased and the
 *  cursor starts again at the first character of the first line.
 *  So the function will automatically seize output after 32 characters.\n
 *  This is a mighty tool for saving SRAM memory.
 *
 *  \param string  The string to be written (a pointer to the first character).
 */
void lcd_writeProgString(char const *string)
{
	char c;
	os_enterCriticalSection();

	while ((c = (char)pgm_read_byte(string++)) != '\0')
	{
		lcd_writeChar(c);
	}

	os_leaveCriticalSection();
}

/*!
 *  Send a command to the LCD.
 *
 *  \param cmd  The command byte to send
 */
void lcd_sendCommand(uint8_t cmd)
{
	os_enterCriticalSection();

	LCD_RS_LOW(); // Command mode
	lcd_sendNibble(cmd >> 4);
	lcd_sendNibble(cmd);
	_delay_us(40); // Most commands take < 37µs

	os_leaveCriticalSection();
}

/*!
 *  Send data (a character) to the LCD.
 *
 *  \param data  The data byte to send
 */
void lcd_sendData(uint8_t data)
{
	os_enterCriticalSection();

	LCD_RS_HIGH(); // Data mode
	lcd_sendNibble(data >> 4);
	lcd_sendNibble(data);
	_delay_us(40); // Most characters take < 37µs

	os_leaveCriticalSection();
}

/*!
 *  LCD draw char
 */
void lcd_writeChar(char character)
{
	os_enterCriticalSection();

	if (character == '\n')
	{
		charCtr = (charCtr & LCD_COLS) + LCD_COLS; // <16 -> 16, <32 -> 32
	}

	// Perform line-break if necessary
	if (charCtr == LCD_COLS)
	{
		lcd_line2();
	}
	else if (charCtr == 2 * LCD_COLS)
	{
		lcd_clear();
		lcd_line1();
	}

	// Check for non-ASCII characters the LCD knows
    switch (character)
    {
        case (char)0xC3A4:
            character = 0xE1; // Character code for 'ä'
            break;
        case (char)0xC3B6:
            character = 0xEF; // Character code for 'ö'
            break;
        case (char)0xC3BC:
            character = 0xF5; // Character code for 'ü'
            break;
        case (char)0xC39F:
            character = 0xE2; // Character code for 'ß'
            break;
        case (char)0xC384:
            character = 0xE1; // Character code for 'Ä'
            break;
        case (char)0xC396:
            character = 0xEF; // Character code for 'Ö'
            break;
        case (char)0xC39C:
            character = 0xF5; // Character code for 'Ü'
            break;
        case (char)0xC2B0:
            character = 0xDF; // Character code for '°' (degree symbol)
            break;
        case (char)0xC2B5 :
            character = 0xE4; // Character code for 'µ' (backslash)
            break;
    }

	// Draw character
	lcd_sendData(character);
	charCtr++;

	os_leaveCriticalSection();
}

/*!
 *  Writes a hexadecimal half-byte (one nibble)
 *
 *  \param number  The number to be written.
 */
void lcd_writeHexNibble(uint8_t number)
{
	os_enterCriticalSection();

	// get low and high nibble
	uint8_t const low = number & 0xF;

	if (low < 10)
		lcd_writeChar(low + '0'); // write as ASCII number
	else
		lcd_writeChar(low - 10 + 'A'); // write as ASCII letter

	os_leaveCriticalSection();
}

/*!
 *  Writes a hexadecimal byte (equals two chars)
 *
 *  \param number  The number to be written.
 */
void lcd_writeHexByte(uint8_t number)
{
	os_enterCriticalSection();

	lcd_writeHexNibble(number >> 4);
	lcd_writeHexNibble(number & 0xF);

	os_leaveCriticalSection();
}

/*!
 *  Writes a hexadecimal word (equals two bytes)
 *
 *  \param number  The number to be written.
 */
void lcd_writeHexWord(uint16_t number)
{
	os_enterCriticalSection();

	lcd_writeHexByte(number >> 8);
	lcd_writeHexByte(number);

	os_leaveCriticalSection();
}

/*!
 * Writes a hexadecimal word, without adding leading 0s
 *
 * \param number The number to be written.
 */
void lcd_writeHex(uint16_t number)
{
	uint16_t nib = 16;
	uint8_t print = 0;

	os_enterCriticalSection();

	// iterate over all nibbles and start printing when we find the first non-zero
	while (nib)
	{
		nib -= 4;
		print |= number >> nib;
		if (print)
		{
			lcd_writeHexNibble(number >> nib);
		}
	}

	os_leaveCriticalSection();
}

/*!
 *  Writes a 16 bit integer as a decimal number without leading 0s
 *
 *  \param number The number to be written.
 */
void lcd_writeDec(uint16_t number)
{
	if (!number)
	{
		lcd_writeChar('0');
		return;
	}

	uint32_t pos = 10000;
	uint8_t print = 0;

	os_enterCriticalSection();

	do
	{
		uint8_t const digit = number / pos;
		number -= digit * pos;
		if (print |= digit)
			lcd_writeChar(digit + '0');
	} while (pos /= 10);

	os_leaveCriticalSection();
}

/*!
 *   Write a drawbar
 *
 *  \param percent The percentage of the bar to be drawn.
 */
void lcd_drawBar(uint8_t percent)
{

	os_enterCriticalSection();

	lcd_clear();
	// calculate number of bars
	uint16_t const val = ((percent <= 100) ? percent : 100) * 16;
	uint16_t i = 0;
	// draw bars
	for (i = 0; i < val; i += 100)
	{
		lcd_writeChar(LCD_CHAR_BAR);
	}

	os_leaveCriticalSection();
}

/*!
 *   Jump into first line with cursor
 */
void lcd_line1()
{
	lcd_goto(0, 0);
}

/*!
 *   Jump into second line with cursor
 */
void lcd_line2()
{
	lcd_goto(1, 0);
}
