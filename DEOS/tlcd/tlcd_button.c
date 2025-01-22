/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "tlcd_button.h"
#include "tlcd_event_parser.h"
#include "tlcd_graphic.h"

typedef struct
{
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
	uint8_t color;
	uint8_t downCode;
	char c;
} Button;

Button buttons[MAX_BUTTONS];
uint8_t usedButtons = 0;
ButtonCallback *buttonCallback = 0;

/*!
 *  Set the function to be called if a button was pressed
 *
 *  \param callback The function to be called
 */
void tlcd_setButtonCallback(ButtonCallback *callback)
{
	buttonCallback = callback;
}

/*!
 *  Add a button to the screen and internal logic to be handled by the handleButtons function
 *
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 *  \param color The color index of the color the button should be drawn in
 *  \param downCode The code to be send if the button is pressed
 */
void tlcd_addButton(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t downCode)
{
	tlcd_addButtonWithChar(x1, y1, x2, y2, color, downCode, 0);
}

/*!
 *  Add a button to the screen and internal logic to be handled by the handleButtons function. In addition the button will present a character as label.
 *
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 *  \param color The color index of the color the button should be drawn in
 *  \param downCode The code to be send if the button is pressed
 *  \param c The character to display on the button
 */
void tlcd_addButtonWithChar(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t downCode, char c)
{
	if (usedButtons == MAX_BUTTONS)
	{
		return;
	}
	buttons[usedButtons].x1 = MIN(x1, x2);
	buttons[usedButtons].y1 = MIN(y1, y2);
	buttons[usedButtons].x2 = MAX(x1, x2);
	buttons[usedButtons].y2 = MAX(y1, y2);
	buttons[usedButtons].downCode = downCode;
	buttons[usedButtons].color = color;
	buttons[usedButtons].c = c;
	usedButtons++;
}

/*!
 *  Draw the buttons onto the screen. This function should be called whenever the screen was cleared.
 */
void tlcd_drawButtons()
{
	for (uint8_t i = 0; i < usedButtons; i++)
	{
		if (buttons[i].color)
		{
			tlcd_drawBox(buttons[i].x1, buttons[i].y1, buttons[i].x2, buttons[i].y2, buttons[i].color);
			if (buttons[i].c != 0)
			{
				uint16_t centerX = buttons[i].x1 + (buttons[i].x2 - buttons[i].x1) / 2;
				uint16_t centerY = buttons[i].y1 + (buttons[i].y2 - buttons[i].y1) / 2;
				tlcd_drawChar(centerX, centerY, buttons[i].c);
			}
		}
	}
}

/*!
 *  Check an event against all buttons for collision and execute button function if a button was pressed. Will return true if the event was inside a button and a false otherwise.
 *
 *  \param event The touchevent to handle
 *  \return True if event was handled, false otherwise
 */
bool tlcd_handleButtons(touch_event_t event)
{
	// go through buttons and check for collision
	for (uint8_t i = 0; i < usedButtons; i++)
	{
		if (event.x >= buttons[i].x1 && event.x <= buttons[i].x2)
		{
			if (event.y >= buttons[i].y1 && event.y <= buttons[i].y2)
			{
				if (event.type == TOUCHPANEL_DOWN && buttonCallback != 0)
				{
					buttonCallback(buttons[i].downCode, event.x, event.y);
				}
				return true;
			}
		}
	}
	return false;
}