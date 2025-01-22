/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "tlcd_graphic.h"
#include "../lib/util.h"
#include "../os_core.h"
#include "../os_scheduler.h"
#include "../spi/spi.h"
#include "string.h"
#include "tlcd_core.h"

#include <avr/pgmspace.h>
#include <stdint.h>

#warning "0 equals the current background color"


// #define DEBUG_SPI_HIGH_LEVEL

//----------------------------------------------------------------------------
// Given Functions
//----------------------------------------------------------------------------

/*!
 *  Draw a text at position (x1,y1)
 *
 *  \param x1 X coordinate
 *  \param y1 Y coordinate
 *  \param text Text to draw
 */
    void tlcd_drawString(uint16_t x1, uint16_t y1, const char* text)
{
    tlcd_drawStringAligned(x1, y1, text, 1);
}


void tlcd_drawStringAligned(uint16_t x1, uint16_t y1, const char* text, uint8_t alignment)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DrawString: %d,%d: %s", x1, y1, text);
#endif
    uint8_t firstBytes[7];
    switch (alignment)
    {
        case 1:
        {
            memcpy(firstBytes, (uint8_t[]) { ESC_BYTE, Z_BYTE, L_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1) }, 7);
        }
        break;
        case 2:
        {
            memcpy(firstBytes, (uint8_t[]) { ESC_BYTE, Z_BYTE, C_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1) }, 7);

        }
        break;
        case 3:
        {
            memcpy(firstBytes, (uint8_t[]) { ESC_BYTE, Z_BYTE, R_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1) }, 7);
        }
        break;
    }


    const uint8_t textLength = strlen(text);
    const uint8_t lastBytes[] = { NUL_BYTE };

    uint8_t len = sizeof(firstBytes) + textLength + sizeof(lastBytes);
    uint8_t header[] = { DC1_BYTE, len };
    uint8_t bcc = INITIAL_BCC_VALUE;

    tlcd_calculateBCC(&bcc, header, sizeof(header));
    tlcd_calculateBCC(&bcc, firstBytes, sizeof(firstBytes));
    tlcd_calculateBCC(&bcc, text, textLength);
    tlcd_calculateBCC(&bcc, lastBytes, sizeof(lastBytes));

    uint8_t retries = 0;

    os_enterCriticalSection();

    spi_cs_enable();

    do
    {
        spi_writeData(header, sizeof(header));
        spi_writeData((uint8_t*)firstBytes, sizeof(firstBytes));
        spi_writeData((uint8_t*)text, textLength);
        spi_writeData((uint8_t*)lastBytes, sizeof(lastBytes));
        spi_write(bcc);
    } while (spi_read() != ACK && retries++ < TLCD_MAX_RETRIES);

    if (retries >= TLCD_MAX_RETRIES)
    {
        // os_error("no ACK");
    }

    spi_cs_disable();

    os_leaveCriticalSection();
}


void tlcd_drawStringInArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const char* text)
{

#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DrawString: %d,%d: %s", x1, y1, text);
#endif
    const uint8_t firstBytes[] = { ESC_BYTE, Z_BYTE, B_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), LOW(x2), HIGH(x2), LOW(y2), HIGH(y2), 0x05 };
    const uint8_t textLength = strlen(text);
    const uint8_t lastBytes[] = { NUL_BYTE };

    uint8_t len = sizeof(firstBytes) + textLength + sizeof(lastBytes);
    uint8_t header[] = { DC1_BYTE, len };
    uint8_t bcc = INITIAL_BCC_VALUE;

    tlcd_calculateBCC(&bcc, header, sizeof(header));
    tlcd_calculateBCC(&bcc, firstBytes, sizeof(firstBytes));
    tlcd_calculateBCC(&bcc, text, textLength);
    tlcd_calculateBCC(&bcc, lastBytes, sizeof(lastBytes));

    uint8_t retries = 0;

    os_enterCriticalSection();

    spi_cs_enable();

    do
    {
        spi_writeData(header, sizeof(header));
        spi_writeData((uint8_t*)firstBytes, sizeof(firstBytes));
        spi_writeData((uint8_t*)text, textLength);
        spi_writeData((uint8_t*)lastBytes, sizeof(lastBytes));
        spi_write(bcc);
    } while (spi_read() != ACK && retries++ < TLCD_MAX_RETRIES);

    if (retries >= TLCD_MAX_RETRIES)
    {
        // os_error("no ACK");
    }

    spi_cs_disable();

    os_leaveCriticalSection();
}

/*!
 *  Draw a text from prog mem at position (x1,y1)
 *
 *  \param x1 X coordinate
 *  \param x1 Y coordinate
 *  \param text Text to draw (address pointing to prog mem)
 */
void tlcd_drawProgString(uint16_t x1, uint16_t y1, const char* text)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DrawProgString: %d,%d: %s", x1, y1, text);
#endif
    const uint8_t firstBytes[] = { ESC_BYTE, Z_BYTE, C_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1) };
    const uint8_t textLength = strlen_P(text);
    const uint8_t lastBytes[] = { NUL_BYTE };

    uint8_t len = sizeof(firstBytes) + textLength + sizeof(lastBytes);
    uint8_t header[] = { DC1_BYTE, len };
    uint8_t bcc = INITIAL_BCC_VALUE;

    tlcd_calculateBCC(&bcc, header, sizeof(header));
    tlcd_calculateBCC(&bcc, firstBytes, sizeof(firstBytes));
    tlcd_calculateBCC_ProgMem(&bcc, text, textLength);
    tlcd_calculateBCC(&bcc, lastBytes, sizeof(lastBytes));

    uint8_t retries = 0;

    os_enterCriticalSection();

    spi_cs_enable();

    do
    {
        spi_writeData(header, sizeof(header));
        spi_writeData((uint8_t*)firstBytes, sizeof(firstBytes));
        spi_writeDataProgMem((uint8_t*)text, textLength);
        spi_writeData((uint8_t*)lastBytes, sizeof(lastBytes));
        spi_write(bcc);
    } while (spi_read() != ACK && retries++ < TLCD_MAX_RETRIES);

    if (retries >= TLCD_MAX_RETRIES)
    {
        // os_error("no ACK");
    }

    spi_cs_disable();

    os_leaveCriticalSection();
}

//----------------------------------------------------------------------------
// Your Homework
//----------------------------------------------------------------------------

/*!
 *  Define a touch area for which the TLCD will send touch events
 *
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 */
void tlcd_defineTouchArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("TouchArea: %d,%d:%d,%d", x1, y1, x2, y2);
#endif
    const uint8_t cmd[] = { ESC_BYTE, A_BYTE, H_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), LOW(x2), HIGH(x2), LOW(y2), HIGH(y2) };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/*!
 *  This function clears the display (fills hole display with background color).
 */
void tlcd_clearDisplay()
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("ClearDisplay");
#endif
    const uint8_t cmd[] = { ESC_BYTE, D_BYTE, L_BYTE };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/*!
 *  Draw a colored box at given coordinates
 *
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 *  \param fill_color color id of fill color
 */
void tlcd_drawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t fill_color)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DrawBox: %d,%d:%d,%d", x1, y1, x2, y2);
#endif
    const uint8_t cmd[] = { ESC_BYTE, R_BYTE, F_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), LOW(x2), HIGH(x2), LOW(y2), HIGH(y2), fill_color };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void tlcd_drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DrawRectangle: %d,%d:%d,%d", x1, y1, x2, y2);
#endif
    const uint8_t cmd[] = { ESC_BYTE, G_BYTE, R_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), LOW(x2), HIGH(x2), LOW(y2), HIGH(y2) };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/*!
 *  This function draws a line in graphic mode from the point (x1,y1) to (x2,y2)
 *
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 */
void tlcd_drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DrawLine: %d,%d:%d,%d", x1, y1, x2, y2);
#endif
    const uint8_t cmd[] = { ESC_BYTE, G_BYTE, D_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), LOW(x2), HIGH(x2), LOW(y2), HIGH(y2) };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/*!
 *  This function draws a point (x1,y1) on the display

 *  \param x1 The position on the x-axis.
 *  \param y1 The position on the y-axis.
 */
void tlcd_drawPoint(uint16_t x1, uint16_t y1)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DrawPoint: %d,%d", x1, y1);
#endif
    const uint8_t cmd[] = { ESC_BYTE, G_BYTE, P_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1) };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/*!
 *  Change the size of lines being drawn
 *
 *  \param size The new size for the lines
 */
void tlcd_changePenSize(uint8_t size)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("ChangePenSize: %d", size);
#endif
    const uint8_t cmd[] = { ESC_BYTE, G_BYTE, Z_BYTE, size, size }; // Size is width and height
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void tlcd_changeTextSize(uint8_t size)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("ChangePenSize: %d", size);
#endif
    const uint8_t cmd[] = { ESC_BYTE, Z_BYTE, Z_BYTE, size, size }; // Size is width and height
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/*!
 *  Change the color lines are drawn in
 *
 *  \param colorID Index of the color lines should be drawn in
 */
void tlcd_changeLineColor(uint8_t colorID)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("ChangeDrawColor: %d", colorID);
#endif
    const uint8_t cmd[] = { ESC_BYTE, F_BYTE, G_BYTE, colorID, NUL_BYTE }; // FG color, BG color
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void tlcd_changeTextColor(uint8_t colorID)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("ChangeDrawColor: %d", colorID);
#endif
    const uint8_t cmd[] = { ESC_BYTE, F_BYTE, Z_BYTE, colorID, NUL_BYTE }; // FG color, BG color
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/*
    0: 0 deg
    1: 90 deg
    2: 180 deg
    3: 270 deg
*/
void tlcd_changeTextAngle(uint8_t angle)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("ChangeTextAngle: %d", angle);
#endif
    const uint8_t cmd[] = { ESC_BYTE, Z_BYTE, W_BYTE, angle, NUL_BYTE };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}


/*!
 *  Define the color at a given index. Not all bits are used, refer to data sheet.
 *
 *  \param colorID The color index to change.
 *  \param The color to set.
 */
void tlcd_defineColor(uint8_t colorID, tlcd_color_t color)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DefineColor: %d:%d,%d,%d", colorID, color.red, color.green, color.blue);
#endif
    const uint8_t cmd[] = { ESC_BYTE, F_BYTE, P_BYTE, colorID, color.red, color.green, color.blue };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/*!
 *  Draw a character c at position (x1,y1)
 *
 *  \param c Character to draw
 *  \param x1 X coordinate
 *  \param x1 Y coordinate
 */
void tlcd_drawChar(uint16_t x1, uint16_t y1, char c)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("DrawChar: %d,%d: %c", x1, y1, c);
#endif
    const uint8_t cmd[] = { ESC_BYTE, Z_BYTE, C_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), c, 0 };
    tlcd_writeCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}


void tlcd_clearArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
#ifdef DEBUG_SPI_HIGH_LEVEL
    DEBUG("ClearArea: %d,%d:%d,%d", x1, y1, x2, y2);
#endif
    tlcd_drawBox(x1, y1, x2, y2, 1);
}