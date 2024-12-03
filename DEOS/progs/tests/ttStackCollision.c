//-------------------------------------------------
//          TestSuite: StackCollision
//-------------------------------------------------
// A large block of global vars is allocated.
// This should exceed the STACK_OFFSET defined in
// defines.h
// 
// An error must be thrown by your OS!
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_STACK_COLLISION

#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../os_core.h"
#include "../../os_scheduler.h"

// Large global memory
volatile uint8_t dummy[512];

PROGRAM(1, AUTOSTART)
{
	// Write something into the memory block to prevent omission at compile time.
	dummy[0] = 'A';
	dummy[sizeof(dummy) - 1] = 'B';

	lcd_clear();
	lcd_writeProgString(PSTR("OK if error"));

	while (1);
}

#endif