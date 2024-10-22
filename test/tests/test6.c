#include "progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == 6

#include "communication/rfAdapter.h"
#include "buttons.h"
#include "defines.h"
#include "lcd.h"
#include "util.h"
#include "os_scheduler.h"

#include <stdbool.h>

// Change PARTNER_ADDRESS to your partners address
#define PARTNER_ADDRESS ADDRESS_BROADCAST

PROGRAM(1, AUTOSTART)
{
	rfAdapter_init();
	while (1)
	{
		rfAdapter_worker();
	}
}

PROGRAM(2, AUTOSTART)
{
	lcd_writeProgString(PSTR("Waiting... "));

	while (1)
	{
		buttons_waitForPressed(BTN_LEFT);
		rfAdapter_sendLcdClear(PARTNER_ADDRESS);
		rfAdapter_sendLcdPrintProcMem(PARTNER_ADDRESS, PSTR("Hallo "));
		rfAdapter_sendSetLed(PARTNER_ADDRESS, true);
		lcd_writeChar('>');

		buttons_waitForReleased(BTN_LEFT);
		rfAdapter_sendSetLed(PARTNER_ADDRESS, false);
		lcd_writeChar('>');
	}
}

PROGRAM(3, AUTOSTART)
{
	while (1)
	{
		buttons_waitForPressed(BTN_RIGHT);
		rfAdapter_sendLcdPrintProcMem(PARTNER_ADDRESS, PSTR("DEOS"));
		rfAdapter_sendSetLed(PARTNER_ADDRESS, true);
		lcd_writeChar('>');

		buttons_waitForReleased(BTN_RIGHT);
		rfAdapter_sendSetLed(PARTNER_ADDRESS, false);
		lcd_writeChar('>');
	}
}

#endif