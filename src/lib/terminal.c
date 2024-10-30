/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "terminal.h"
#include "../os_scheduler.h"
#include "uart.h"
#include "util.h"
#include <stdio.h>

//----------------------------------------------------------------------------
// Configuration of stdio.h
//----------------------------------------------------------------------------
int stdio_put_char(char c, FILE *stream)
{
    os_enterCriticalSection();

    terminal_writeChar(c);
    if (c == '\n')
    {
        terminal_writeProgString(PSTR("        "));
    }

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
// Implementation
//----------------------------------------------------------------------------
/*!
 *  Initialize the terminal
 */
void terminal_init()
{
    os_enterCriticalSection();
    uart2_init(UART_BAUD_SELECT(38400, F_CPU));
    stdout = &mystdout;
    os_leaveCriticalSection();
}

/*!
 *  Write a half-byte (a nibble)
 *
 *  \param number  The number to be written.
 */
void terminal_writeHexNibble(uint8_t number)
{
    os_enterCriticalSection();

    if (number < 10)
    {
        uart2_putc('0' + number);
    }
    else
    {
        uart2_putc('A' + number - 10);
    }

    os_leaveCriticalSection();
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
 *  Write one hexadecimal word without prefixes
 *
 *  \param number  The number to be written.
 */
void terminal_writeHex(uint16_t number)
{
    os_enterCriticalSection();

    uint8_t nib = 12;
    uint8_t print = 0;

    while (nib)
    {
        nib -= 4;
        print |= number >> nib;
        if (print)
        {
            terminal_writeHexNibble(number >> nib);
        }
    }

    os_leaveCriticalSection();
}

/*!
 *  Write a word as a decimal number without prefixes
 *
 *  \param number  The number to be written.
 */
void terminal_writeDec(uint16_t number)
{
    os_enterCriticalSection();

    if (!number)
    {
        terminal_writeChar('0');
        return;
    }

    uint32_t pos = 10000;
    uint8_t print = 0;

    do
    {
        uint8_t const digit = number / pos;
        number -= digit * pos;
        if (print |= digit)
            terminal_writeChar(digit + '0');
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
    os_enterCriticalSection();

    // TX buffer needs to be flushed manually if interrupts are disabled
    if (!gbi(SREG, 7) && uart2_gettxcount() >= UART2_TX_BUFFER_SIZE - 2) // at most 2 chars can be written in case of \n
    {
        terminal_flushBlocking();
    }

    if (character == '\n')
    {
        uart2_putc('\r');
    }
    uart2_putc(character);

    os_leaveCriticalSection();
}

/*!
 *  Write string on terminal
 *
 *  \param str  The string to be written (a pointer to the first character).
 */
void terminal_writeString(char *str)
{
    os_enterCriticalSection();

    uart2_puts(str);

    os_leaveCriticalSection();
}

/*!
 *  Write char PROGMEM* string to terminal
 *
 *  \param pstr  The string to be written (a pointer to the first character).
 */
void terminal_writeProgString(const char *pstr)
{
    os_enterCriticalSection();

    char *ptr = (char *)pstr;
    char c;
    while ((c = pgm_read_byte(ptr++)) != '\0')
    {
        terminal_writeChar(c);
    }

    os_leaveCriticalSection();
}

/*!
 *  Write char PROGMEM* string to terminal with indentation after each \n
 *
 *  \param pstr  The string to be written (a pointer to the first character).
 */
void terminal_writeIndentedProgString(const char *pstr)
{
    os_enterCriticalSection();

    char *ptr = (char *)pstr;
    char c;
    while ((c = pgm_read_byte(ptr++)) != '\0')
    {
        terminal_writeChar(c);
        if (c == '\n')
        {
            char next = pgm_read_byte(ptr + 1);
            if (next != '\n' && next != '\0')
            {
                terminal_writeProgString(PSTR("         "));
            }
            continue;
        }
    }

    os_leaveCriticalSection();
}

/*!
 *  Write a new line
 */
void terminal_newLine()
{
    os_enterCriticalSection();

    terminal_writeChar('\n');

    os_leaveCriticalSection();
}

/*!
 *  Flush the terminal without needing interrupts enabled
 */
void terminal_flushBlocking()
{
    os_enterCriticalSection();

    uart2_flush_blocking();

    os_leaveCriticalSection();
}