//-------------------------------------------------
//          TestSuite: StackCollision
//-------------------------------------------------
// Tests if the StackEnd check is effective
// by allocating a large block of memory as a global
// variable.
// An error must be shown by the operating system.
//-------------------------------------------------
#include "progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == 3

#include "lcd.h"
#include "util.h"
#include "os_core.h"
#include "os_scheduler.h"

// Large global memory
uint8_t dummy[512];

PROGRAM(1, AUTOSTART)
{
  // Write something into the memory block to prevent omission at compile time.
  dummy[0] = 'A';
  dummy[sizeof(dummy) - 1] = 'B';

  lcd_clear();
  lcd_writeProgString(PSTR("OK if error"));

  while (1)
    ;
}

#endif