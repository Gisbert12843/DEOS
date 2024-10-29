#include "defines.h"
#include "lcd.h"
#include "util.h"
#include "os_core.h"
#include "os_scheduler.h"

//----------------------------------------------------------------------------
// Operation System Booting
//----------------------------------------------------------------------------

//! Program's entry point
int main(void)
{
  // Give the operating system a chance to initialize its private data.
  // This also registers and starts the idle program.
  os_init();

  // os_init shows a boot message
  // Wait and clear the LCD
  delayMs(1000);
  lcd_clear();

  // while (true)
  // {
  //   lcd_clear();
  //   printf("lcd cleared");

  //   delayMs(1000);
  //   lcd_writeString("OS started");
  //   printf("OS started");
  //   delayMs(1000);
  // }

  // Start the operating system
  os_startScheduler();

  return 1;
}