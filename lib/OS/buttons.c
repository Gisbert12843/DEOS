/*! \file
 *  \brief Handles button presses and releases (pin change interrupt).
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "buttons.h"
#include "os_scheduler.h"

#include <avr/io.h>
#include <stdbool.h>

button_t buttons_currentlyPressed;

/*!
 *  Read the button that is currently pressed
 *
 *  \return the button that is currently pressed
 */
button_t buttons_read()
{
	uint16_t value;

	// Read the value from the ADC
	ADMUX = (1 << REFS0);																							 // Select Vref=AVcc and select ADC0 (default is ADC0 when ADMUX lower bits are 0000)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC and set prescaler to 128
	ADCSRA |= (1 << ADSC);																						 // Start conversion
	ADMUX = (1 << REFS0);																							 // REFS0 = 1 selects AVcc as the reference voltage; MUX3:MUX0 = 0000 selects ADC0
	while (ADCSRA & (1 << ADSC))
		;					 // Wait until conversion is complete
	value = ADC; // ADC is a 10-bit register, so you get a value between 0 and 1023

	// Return the button that was pressed
	if (value < 50)
		return BTN_RIGHT;
	else if (value < 195)
		return BTN_UP;
	else if (value < 380)
		return BTN_DOWN;
	else if (value < 555)
		return BTN_LEFT;
	else if (value < 790)
		return BTN_SELECT;
	else
		return BTN_NONE;
}

/*!
 *  Check if button got pressed.
 *  After calling this function the pressed flag will reset and you will need to wait for a second press for this to return true again
 *
 * 	\param button button to check
 *  \return true if button got pressed and the function was called the first time after the button press
 */
bool buttons_pressed(button_t button)
{
	bool current = buttons_read();
	if (current == button && current != buttons_currentlyPressed)
	{
		buttons_currentlyPressed = current;
		return true;
	}

	return false;
}

/*!
 *  Check if button got released.
 *  After calling this function the released flag will reset and you will need to wait for a second release for this to return true again
 *
 * 	\param button button to check
 *  \return true if button got released and the function was called the first time after the button press
 */
bool buttons_released(button_t button)
{
	bool current = buttons_read();
	if (button == buttons_currentlyPressed && current != buttons_currentlyPressed)
	{
		buttons_currentlyPressed = current;
		return true;
	}
	return false;
}

/*!
 *  Blocks until button got pressed
 *	\param button button to wait for
 */
void buttons_waitForPressed(button_t button)
{
	while (!buttons_pressed(button))
	{
		os_yield();
	}
}

/*!
 *  Blocks until button got released
 *
 * \param button button to wait for
 */
void buttons_waitForReleased(button_t button)
{
	while (!buttons_released(button))
	{
		os_yield();
	}
}