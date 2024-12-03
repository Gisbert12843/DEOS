//-------------------------------------------------
//          TestSuite: Yield
//-------------------------------------------------
// Tests yielding processes
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_YIELD

#include "../../lib/defines.h"
#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../os_core.h"
#include "../../os_process.h"
#include "../../os_scheduler.h"

#include <stdbool.h>

#define PHASE1_ROUNDS 50
#define PHASE2_ROUNDS 50

#define PHASE1
#define PHASE2

process_id_t lastProcess;
uint8_t i;

PROGRAM(1, AUTOSTART)
{
	process_id_t pid = os_getCurrentProc();

#ifdef PHASE1
	/*
	 * Expected to yield between 4 processes (of which 3 are workers)
	 */
	lcd_clear();
	lcd_writeProgString(PSTR("Phase 1:"));
	lcd_line2();
	lcd_writeProgString(PSTR("Behavior"));

	i = 0;
	
	os_exec(2, DEFAULT_PRIORITY);
	os_exec(2, DEFAULT_PRIORITY);
	os_exec(2, DEFAULT_PRIORITY);

	while (i < PHASE1_ROUNDS)
	{
		lastProcess = pid;
	}

	lcd_writeProgString(PSTR(" OK"));
	delayMs(1000);

#endif
#ifdef PHASE2
	/*
	 * Expected to schedule to one of 3 workers. They mustn't yield because they are in a critical section. If they do, it's an error.
	 */
	lcd_clear();
	lcd_writeProgString(PSTR("Phase 2:"));
	lcd_line2();
	lcd_writeProgString(PSTR("Crit. sec."));

	i = 0;

	os_exec(3, DEFAULT_PRIORITY);
	os_exec(3, DEFAULT_PRIORITY);
	os_exec(3, DEFAULT_PRIORITY);

	while (i < PHASE2_ROUNDS)
	{
		lastProcess = pid;
		os_yield();
	}

	lcd_writeProgString(PSTR(" OK"));
	delayMs(1000);
#endif

	lcd_clear();
	while (1)
	{
		lcd_writeProgString(PSTR("  TESTS PASSED"));
		delayMs(500);
		lcd_clear();
		delayMs(500);
	}
}

// PHASE 1
PROGRAM(2, DONTSTART)
{
	process_id_t pid = os_getCurrentProc();
	while (i++ < PHASE1_ROUNDS)
	{
		lastProcess = pid;
		os_yield();
		if (lastProcess == pid && i < PHASE1_ROUNDS)
		{
			os_error("Error:          Didn't yield");
		}
	}
}

// PHASE 2
PROGRAM(3, DONTSTART)
{
	os_enterCriticalSection();

	process_id_t pid = os_getCurrentProc();
	while (i++ < PHASE2_ROUNDS)
	{
		lastProcess = pid;
		os_yield();
		if (lastProcess != pid)
		{
			os_error("Error:          Did yield");
		}
	}

	os_leaveCriticalSection();
}

#endif