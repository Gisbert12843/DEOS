/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef TLCD_GRAPHIC_H_
#define TLCD_GRAPHIC_H_

#include <stdint.h>



//! Struct to define a color
typedef struct TLCD_Color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} tlcd_color_t;

//! Defines a free touch area. Events that occur in this area are caught
void tlcd_defineTouchArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

//! Draws a line on the display with given start and endpoint
void tlcd_drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

//! Draws a point to a given location on the display
void tlcd_drawPoint(uint16_t x1, uint16_t y1);

//! Changes the size of the pen
void tlcd_changePenSize(uint8_t size);
void tlcd_changeTextSize(uint8_t size);

//! Changes the color of the pen
void tlcd_changeLineColor(uint8_t color);
void tlcd_changeTextColor(uint8_t color);

void tlcd_changeTextAngle(uint8_t angle);


//! Draws a Box on the display
void tlcd_drawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color);


void tlcd_drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);



/*!
 *  Define the color at a given index. Not all bits are used, refer to data sheet.
 *
 *  \param colorID The color index to change.
 *  \param The color to set.
 */
void tlcd_defineColor(uint8_t colorID, tlcd_color_t color);

//! Draws a character on the display
void tlcd_drawChar(uint16_t x1, uint16_t y1, char c);

//! Draws a string on the display
void tlcd_drawString(uint16_t x1, uint16_t y1, const char* text);
/*
alignment:
    1: left
    2: center
    3: right
*/
void tlcd_drawStringAligned(uint16_t x1, uint16_t y1, const char* text, uint8_t alignment);
void tlcd_drawStringInArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const char* text);

//! Draws a string on the display from program memory
void tlcd_drawProgString(uint16_t x1, uint16_t y1, const char* text);

//! Clears the display
void tlcd_clearDisplay();

void tlcd_clearArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif /* TLCD_GRAPHIC_H_ */