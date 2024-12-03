/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdint.h>
#include <stdio.h>

#define INFO(str, ...) terminal_log_printf_p(PSTR("[INFO]  "), PSTR(str), ##__VA_ARGS__)
#define WARN(str, ...) terminal_log_printf_p(PSTR("[WARN]  "), PSTR(str), ##__VA_ARGS__)
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(str, ...) terminal_log_printf_p(PSTR("[DEBUG] "), PSTR(str), ##__VA_ARGS__) // You could use __LINE__ or __FILE__ to include line number or file name in the log message

//! Initialize the terminal
void terminal_init();

//! Write a half-byte (a nibble)
void terminal_writeHexNibble(uint8_t number);

//! Write one hexadecimal byte
void terminal_writeHexByte(uint8_t number);

//! Write one hexadecimal word
void terminal_writeHexWord(uint16_t number);

//! Write one hexadecimal word without prefixes
void terminal_writeHex(uint16_t number);

//! Write a byte as a decimal number without prefixes
void terminal_writeDec(uint16_t number);

//! Write char to usb terminal
void terminal_writeChar(char character);

//! Draw string on usb terminal
void terminal_writeString(char *str);

//! Write char PROGMEM* string
void terminal_writeProgString(const char *pstr);

//! Write char PROGMEM* string, with indentation after each \n, unless it's the last one
void terminal_writeIndentedProgString(const char *pstr);

//! Write a new line
void terminal_newLine();

//! Write a formatted string to the terminal with a prefix
void terminal_log_printf_p(const char *prefix, const char *fmt, ...);

#endif /* TERMINAL_H_ */