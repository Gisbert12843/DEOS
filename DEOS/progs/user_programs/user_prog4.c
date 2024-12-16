#include "../progs.h"
#if defined(USER_PROGRAM_ENABLED) && USER_PROGRAM == 4

#include "../../lib/defines.h"
#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../os_core.h"
#include "../../os_process.h"
#include "../../communication/rfAdapter.h"
#include "../../communication/xbee.h"


PROGRAM(1, AUTOSTART)
{
	printf("PROGRAM 1\n");
	rfAdapter_init();

	while(1)
	{
		rfAdapter_sendToggleLed(serialAdapter_address);
		delayMs(2000);
	}
}

PROGRAM(2, AUTOSTART)
{
	printf("PROGRAM 2\n");
	while(1)
	{
		int x = xbee_getNumberOfBytesReceived();
		printf("Number of bytes received: %d\n", x);

		char buffer[32];
		for(int i = 0; i < x; i++)
		{
			uint8_t err = xbee_read(&buffer[i]);
			if( err != XBEE_SUCCESS)
			{
				printf("Error: %d\n", err);
			}
		}

		for(int i = 0; i < x; i++)
		{
			printf("Byte %d: %c\n", i, buffer[i]);
		}

		delayMs(DEFAULT_OUTPUT_DELAY);
	}
}

#endif