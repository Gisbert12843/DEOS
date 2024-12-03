#ifndef _LCD_H_
#define _LCD_H_

#include "../lib/util.h"
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

extern FILE lcd_stdout;

//! Write a formatted string to the LCD
void lcd_printf_p(const char *fmt, ...);

#define LCD(str, ...) lcd_printf_p(PSTR(str), ##__VA_ARGS__)

// Pin Definitions
#define LCD_RS_PIN PH5
#define LCD_EN_PIN PH6
#define LCD_RW_PIN PF0

#define LCD_D4_PIN PG5
#define LCD_D5_PIN PE3
#define LCD_D6_PIN PH3
#define LCD_D7_PIN PH4

// Macros to control pins
#define LCD_RS_HIGH() (PORTH |= (1 << LCD_RS_PIN))
#define LCD_RS_LOW() (PORTH &= ~(1 << LCD_RS_PIN))

#define LCD_EN_HIGH() (PORTH |= (1 << LCD_EN_PIN))
#define LCD_EN_LOW() (PORTH &= ~(1 << LCD_EN_PIN))

#define LCD_RW_HIGH() (PORTF |= (1 << LCD_RW_PIN))
#define LCD_RW_LOW() (PORTF &= ~(1 << LCD_RW_PIN))

#define LCD_D4_HIGH() (PORTG |= (1 << LCD_D4_PIN))
#define LCD_D4_LOW() (PORTG &= ~(1 << LCD_D4_PIN))

#define LCD_D5_HIGH() (PORTE |= (1 << LCD_D5_PIN))
#define LCD_D5_LOW() (PORTE &= ~(1 << LCD_D5_PIN))

#define LCD_D6_HIGH() (PORTH |= (1 << LCD_D6_PIN))
#define LCD_D6_LOW() (PORTH &= ~(1 << LCD_D6_PIN))

#define LCD_D7_HIGH() (PORTH |= (1 << LCD_D7_PIN))
#define LCD_D7_LOW() (PORTH &= ~(1 << LCD_D7_PIN))

// LCD Commands
#define LCD_CMD_CLEAR_DISPLAY /*   */ 0x01
#define LCD_CMD_RETURN_HOME /*     */ 0x02
#define LCD_CMD_ENTRY_MODE_SET /*  */ 0x04
#define LCD_CMD_DISPLAY_CONTROL /* */ 0x08
#define LCD_CMD_CURSOR_SHIFT /*    */ 0x10
#define LCD_CMD_FUNCTION_SET /*    */ 0x20
#define LCD_CMD_SET_CGRAM_ADDR /*  */ 0x40
#define LCD_CMD_SET_DDRAM_ADDR /*  */ 0x80

// Display control flags
#define LCD_DISPLAY_ON /*         */ 0x04
#define LCD_DISPLAY_OFF /*        */ 0x00
#define LCD_CURSOR_ON /*          */ 0x02
#define LCD_CURSOR_OFF /*         */ 0x00
#define LCD_BLINK_ON /*           */ 0x01
#define LCD_BLINK_OFF /*          */ 0x00

// Function set flags
#define LCD_8BIT_MODE /*          */ 0x10
#define LCD_4BIT_MODE /*          */ 0x00
#define LCD_2LINE /*              */ 0x08
#define LCD_1LINE /*              */ 0x00
#define LCD_5x10DOTS /*           */ 0x04
#define LCD_5x8DOTS /*            */ 0x00

#define LCD_ROWS /*   */ 2
#define LCD_COLS /*   */ 16

//! Character that looks like filled rectangle
#define LCD_CHAR_BAR 0xFF

// Public API Functions

//! Initialize the LCD in 4-bit mode
void lcd_init(void);

//! Clear the display and set the cursor to the home position
void lcd_clear(void);

//! Set the cursor to the home position
void lcd_home(void);

//! Turn the display on
void lcd_displayOn(void);

//! Turn the display off
void lcd_displayOff(void);

//! Turn the cursor on
void lcd_cursorOn(void);

//! Turn the cursor off
void lcd_cursorOff(void);

//! Turn the cursor blink on
void lcd_blinkOn(void);

//! Turn the cursor blink off
void lcd_blinkOff(void);

//! Set the cursor to a specific position
void lcd_goto(uint8_t row, uint8_t col);

//! Send a nibble to the LCD
void lcd_sendCommand(uint8_t cmd);

//! Send a nibble to the LCD
void lcd_sendData(uint8_t data);

bool lcd_shift(uint8_t item, uint8_t direction);

//! Write a half-byte (a nibble)
void lcd_writeHexNibble(uint8_t number);

//! Write one hexadecimal byte
void lcd_writeHexByte(uint8_t number);

//! Write one hexadecimal word
void lcd_writeHexWord(uint16_t number);

//! Write one hexadecimal word without prefixes
void lcd_writeHex(uint16_t number);

//! Write a byte as a decimal number without prefixes
void lcd_writeDec(uint16_t number);

//! Write a drawbar
void lcd_drawBar(uint8_t percent);

//! Jump into first line with cursor
void lcd_line1();

//! Jump into second line with cursor
void lcd_line2();

//! LCD draw char
void lcd_writeChar(char character);

//! Draw string on LCD
void lcd_writeString(char *str);

//! Write char PROGMEM* string
void lcd_writeProgString(const char *pstr);

#endif
