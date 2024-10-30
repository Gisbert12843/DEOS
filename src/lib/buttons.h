/*! \file
 *  \brief Handles button presses and releases (pin change interrupt).
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */
#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <stdbool.h>

typedef enum Button
{
	BTN_RIGHT,
	BTN_UP,
	BTN_DOWN,
	BTN_LEFT,
	BTN_SELECT,
	BTN_NONE
} button_t;

//! Read the button that is currently pressed
button_t buttons_read();

//* Check if button got pressed.
bool buttons_pressed(button_t button);

//! Check if button got released.
bool buttons_released(button_t button);

//! Blocks until button got pressed
void buttons_waitForPressed(button_t button);

//! Blocks until button got released
void buttons_waitForReleased(button_t button);

#endif