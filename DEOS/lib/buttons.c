/*! \file
 *  \brief Handles button presses and releases (pin change interrupt).
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */
#include "buttons.h"
#include "util.h"

#include <avr/io.h>
#include <stdbool.h>

/*!
 *  Read the button that is currently pressed
 *
 *  \return the button that is currently pressed
 */
button_t buttons_read()
{
	uint16_t value;
	
	// Pin ADC0 (PF0) must be configured as input
	cbi(DDRF, PF0);
	
	// Read the value from the ADC
	ADMUX = (1 << REFS0);												// Select Vref=AVcc and select ADC0 (default is ADC0 when ADMUX lower bits are 0000)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);	// Enable ADC and set prescaler to 128
	ADCSRA |= (1 << ADSC);												// Start conversion
	while (ADCSRA & (1 << ADSC));										// Wait until conversion is complete
	value = ADC;														// ADC is a 10-bit register, so you get a value between 0 and 1023

	// Return the button that was pressed
	if (value < 66)
		return BTN_RIGHT;
	else if (value < 219)
		return BTN_UP;
	else if (value < 393)
		return BTN_DOWN;
	else if (value < 600)
		return BTN_LEFT;
	else if (value < 872)
		return BTN_SELECT;
	else
		return BTN_NONE;
}

/*!
 *  Check if button got pressed.
 *
 * 	\param button button to check
 *  \return true if only the given button is currently pressed
 */
bool buttons_pressed(button_t button)
{
	return buttons_read() == button;
}

/*!
 *  Check if button got released.
 *
 * 	\param button button to check
 *  \return true if any other button or none is pressed
 */
bool buttons_released(button_t button)
{
	return buttons_read() != button;
}

/*!
 *  Blocks until button got pressed
 *	\param button button to wait for
 */
void buttons_waitForPressed(button_t button)
{
	while (!buttons_pressed(button));
}

/*!
 *  Blocks until button got released
 *
 * \param button button to wait for
 */
void buttons_waitForReleased(button_t button)
{
	while (!buttons_released(button));
}