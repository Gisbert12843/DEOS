#include "../progs.h"
#if defined(USER_PROGRAM_ENABLED) && USER_PROGRAM == 2

#include "../../lib/defines.h"
#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../os_core.h"
#include "../../os_process.h"

PROGRAM(1, AUTOSTART)
{
	while (1)
	{
		lcd_writeChar('A');
		delayMs(DEFAULT_OUTPUT_DELAY);
	}
}

PROGRAM(2, AUTOSTART)
{
	while (1)
	{
		lcd_writeChar('B');
		delayMs(DEFAULT_OUTPUT_DELAY);
	}
}

PROGRAM(3, AUTOSTART)
{
	while (1)
	{
		lcd_writeChar('C');
		delayMs(DEFAULT_OUTPUT_DELAY);
	}
}

//! No autostart!
PROGRAM(4, DONTSTART)
{
	while (1)
	{
		lcd_writeChar('D');
		delayMs(DEFAULT_OUTPUT_DELAY);
	}
}

#endif