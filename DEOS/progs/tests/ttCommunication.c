//-------------------------------------------------
//          TestSuite: COMMUNICATION
//-------------------------------------------------
// Can be used to test communication between two
// boards. A string is shown on the LCD and a LED
// is set.
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && (TESTTASK == TT_COMMUNICATION)

#include "../../communication/rfAdapter.h"
#include "../../lib/buttons.h"
#include "../../lib/defines.h"
#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../os_scheduler.h"

#include <stdbool.h>

// Change PARTNER_ADDRESS to your partners address
#define PARTNER_ADDRESS ADDRESS_BROADCAST

PROGRAM(1, AUTOSTART)
{
	rfAdapter_init();
	int i = 0;
	while (1)
	{
		printf("worker %d\n", i);
		rfAdapter_worker();
		i++;
	}
}

PROGRAM(2, AUTOSTART)
{
	lcd_writeProgString(PSTR("Waiting... "));

	while (1)
	{
		buttons_waitForPressed(BTN_LEFT);
		printf_P(PSTR("PROGRAM2\n"));
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
		printf_P(PSTR("PROGRAM3\n"));
		rfAdapter_sendLcdPrint(PARTNER_ADDRESS, "DEOS");
		lcd_writeChar('>');
		rfAdapter_sendSetLed(PARTNER_ADDRESS, true);

		buttons_waitForReleased(BTN_RIGHT);
		rfAdapter_sendSetLed(PARTNER_ADDRESS, false);
		lcd_writeChar('>');
	}
}

#endif