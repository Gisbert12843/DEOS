//-------------------------------------------------
//          TestSuite: TLCD
//-------------------------------------------------
// Simple GUI for TLCD
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_TLCD

#include "../../lib/buttons.h"
#include "../../lib/util.h"
#include "../../os_process.h"
#include "../../os_scheduler.h"
#include "../../tlcd/tlcd_button.h"
#include "../../tlcd/tlcd_core.h"
#include "../../tlcd/tlcd_event_parser.h"
#include "../../tlcd/tlcd_graphic.h"

#include <util/atomic.h>

#define TLCD_SOFTWARE_CORRECTION
#define RED_MAX /*  */ 0b11111000
#define GREEN_MAX /**/ 0b11111100
#define BLUE_MAX /* */ 0b11111000
#define Red /*  */ 32
#define Green /**/ 64
#define Blue /* */ 32
#define RG (Green - 1)
#define Gr (RG + Red - 1)
#define GB (Gr + Blue - 1)
#define Bg (GB + Green - 1)
#define BR (Bg + Red - 1)
#define Rb (BR + Blue - 1)
#define MAX_COLORS 251

#define PENSIZE_INCREASE 33
#define PENSIZE_DECREASE 34
#define ERASER 35

/*!
 * Add the color bar as an invisible button at the bottom of the screen with x-offset 'colorbar_offset'
 */
void addButtonColorBar()
{
	tlcd_addButton(0, TLCD_HEIGHT - 40, TLCD_WIDTH, TLCD_HEIGHT, 0, 17);
}

/*!
 * Add the tool buttons on the left side of the screen: Increase pen, Decrease pen, Eraser
 */
void addButtonsTools()
{
	//+
	tlcd_addButtonWithChar(0, 0, 40, 40, 16, PENSIZE_INCREASE, '+');
	//-
	tlcd_addButtonWithChar(0, 40, 40, 80, 16, PENSIZE_DECREASE, '-');
	// eraser
	tlcd_addButtonWithChar(0, 80, 40, 120, 16, ERASER, 'X');
}

/*!
 * Method to get one of 251 colors
 * \param x Value of color
 * \return Returns color x.
 */
tlcd_color_t getColor(uint16_t x)
{
	tlcd_color_t res = {0, 0, 0};
	if (x <= RG)
	{
		res.red = 0b11111000;
		res.green = (x << 2);
		res.blue = 0;
	}
	else if (x <= Gr)
	{
		res.red = RED_MAX - ((x - RG) << 3);
		res.green = GREEN_MAX;
		res.blue = 0;
	}
	else if (x <= GB)
	{
		res.red = 0;
		res.green = GREEN_MAX;
		res.blue = (x - Gr) << 3;
	}
	else if (x <= Bg)
	{
		res.red = 0;
		res.green = GREEN_MAX - ((x - GB) << 2);
		res.blue = BLUE_MAX;
	}
	else if (x <= BR)
	{
		res.red = (x - Bg) << 3;
		res.green = 0;
		res.blue = BLUE_MAX;
	}
	else if (x <= Rb)
	{
		res.red = RED_MAX;
		res.green = 0;
		res.blue = BLUE_MAX - ((x - BR) << 3);
	}

	return res;
}

/*!
 * Draws the color bar at the bottom of the screen with x-offset 'colorbar_offset'
 */
void drawColorBar()
{
	tlcd_color_t color;
	uint8_t colorIdx;

	for (uint16_t x = 0; x < TLCD_WIDTH; x++)
	{
		colorIdx = (uint32_t)x * MAX_COLORS / TLCD_WIDTH;
		color = getColor(colorIdx);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			tlcd_defineColor(17, color);
			tlcd_changeDrawColor(17);
			tlcd_drawLine(x, TLCD_HEIGHT - 40, x, TLCD_HEIGHT);
		}
	}
}

/*!
 * Function to handle pressed buttons
 * \param code The code of the button that was pressed
 */
void pressButton(uint8_t code, uint16_t x, uint16_t y)
{
	static uint8_t penSize = 1;
	if (code == PENSIZE_INCREASE)
	{
		if (penSize < 15)
		{
			penSize++;
		}
		tlcd_changePenSize(penSize);
	}
	else if (code == PENSIZE_DECREASE)
	{
		if (penSize > 1)
		{
			penSize--;
		}
		tlcd_changePenSize(penSize);
	}
	else if (code == ERASER)
	{
		tlcd_changePenSize(15);
		tlcd_changeDrawColor(1);
	}
	else
	{
		tlcd_defineColor(code, getColor((uint32_t)x * MAX_COLORS / TLCD_WIDTH));
		tlcd_changePenSize(penSize); // Resetting pen size because the eraser could have been selected in between
		tlcd_changeDrawColor(code);
	}
}

/*!
 * Callback for touch events received on the TLCD with correction
 */
void eventHandlerWithCorrection(touch_event_t event)
{
	static touch_event_t previous = {};
	static uint8_t currentlyDown = 0;

	switch (event.type)
	{
	case TOUCHPANEL_DOWN:
		tlcd_drawPoint(event.x, event.y);
		currentlyDown = 1;
		break;
	case TOUCHPANEL_DRAG:
		if (currentlyDown)
		{
			tlcd_drawLine(previous.x, previous.y, event.x, event.y);
		}
		currentlyDown = 1;
		break;
	case TOUCHPANEL_UP:
		currentlyDown = 0; // Resetting this to not draw lines if dragged of a button.
	default:
		break;
	}
	previous = event;
}

/*!
 * Callback for touch events received on the TLCD
 */
void eventHandler(touch_event_t event)
{
	if (event.type == TOUCHPANEL_DOWN || event.type == TOUCHPANEL_DRAG)
	{
		tlcd_drawPoint(event.x, event.y);
	}
}

/*!
 * Paint program
 */
PROGRAM(1, AUTOSTART)
{
	while (!tlcd_isInitialized())
	{
		os_yield();
	}
	tlcd_defineTouchArea(0, 0, TLCD_WIDTH, TLCD_HEIGHT);
	tlcd_setButtonCallback(&pressButton);
#ifdef TLCD_SOFTWARE_CORRECTION
	tlcd_event_setCallback(&eventHandlerWithCorrection);
#else
	tlcd_event_setCallback(&eventHandler);
#endif

	addButtonsTools();
	tlcd_drawButtons();

	addButtonColorBar();
	drawColorBar();

	while (1)
	{
		buttons_waitForPressed(BTN_SELECT);
		tlcd_clearDisplay();
		tlcd_drawButtons();
		drawColorBar();
	}
}

PROGRAM(2, AUTOSTART)
{
	tlcd_init();
	while (1)
	{
		tlcd_event_worker();
		os_yield();
	}
}

#endif