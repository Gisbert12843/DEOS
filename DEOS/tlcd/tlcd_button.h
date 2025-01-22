/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef TLCD_BUTTON_H
#define TLCD_BUTTON_H

#include "tlcd_event_parser.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_BUTTONS 10

#undef MIN
#undef MAX

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

//! Callback function for button presses
typedef void ButtonCallback(uint8_t buttonCode, uint16_t x, uint16_t y);

//! Set the function to be called if a button was pressed
void tlcd_setButtonCallback(ButtonCallback *callback);

//! Add a button to the screen and internal logic to be handled by the handleButtons function
void tlcd_addButton(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t downCode);

//! Add a button to the screen and internal logic to be handled by the handleButtons function. In addition the button will present a character as label.
void tlcd_addButtonWithChar(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t downCode, char c);

//! Draw the buttons onto the screen. This function should be called whenever the screen was cleared.
void tlcd_drawButtons();

//! Check an event against all buttons for collision and execute button function if a button was pressed. Will return true if the event was inside a button and a false otherwise.
bool tlcd_handleButtons(touch_event_t event);

#endif