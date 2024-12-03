/*! \file
 *
 *  The main system core with initialization functions and error handling.
 *
 */

#include "os_core.h"
#include "lib/defines.h"
#include "lib/lcd.h"
#include "lib/stop_watch.h"
#include "lib/terminal.h"
#include "lib/util.h"

#include <avr/interrupt.h>

/*!
 * Initializes the scheduler.
 */
void os_initScheduler(void);

/*!
 *  Initializes the used timers.
 */
void os_initTimer(void)
{
	// Init timer 2 (Scheduler)
	sbi(TCCR2A, WGM21); // Clear on timer compare match

	sbi(TCCR2B, CS22);	 // Prescaler 1024  1
	sbi(TCCR2B, CS21);	 // Prescaler 1024  1
	sbi(TCCR2B, CS20);	 // Prescaler 1024  1
	sbi(TIMSK2, OCIE2A); // Enable interrupt
	OCR2A = 60;
}

/*!
 *  Readies stack, scheduler and heap for first use. Additionally, the LCD is initialized. In order to do those tasks,
 *  it calls the subfunction os_initScheduler().
 */
void os_init()
{
	initSystemTime();
	os_initTimer();
	stopWatch_init();

	// Init LCD display
	lcd_init();
	terminal_init();

	// display on
	lcd_displayOn();
	lcd_clear();

	lcd_writeProgString(PSTR("Booting DEOS ..."));

	terminal_writeProgString(PSTR("\n\n##################################################\n"));
	INFO("Booting DEOS ...");
	INFO("Used global vars: %d/%d bytes", (uint16_t)&__heap_start - AVR_SRAM_START, STACK_OFFSET);
	terminal_writeProgString(PSTR("--------------------------------------------------\n"));

	// Security check if the stack crushes global variables
	assert((uint16_t)&__heap_start < AVR_SRAM_START + STACK_OFFSET, " Stack collides with global vars");

	os_initScheduler();
}

/*!
 *  Terminates the OS and displays a corresponding error on the LCD and terminal
 *
 *  \param msg The error to be displayed as progmem string
 *  \param ... Formatting arguments for the error message
 */
void os_errorPstr(const char *msg, ...)
{
	cli();

	// Make sure we have enough stack left for sure (we can mess with it because we won't go out of this function)
	SP = BOTTOM_OF_MAIN_STACK;

	// Clear display and write error message
	lcd_clear();

	// Print error message to lcd (variadic arguments)
	va_list args;
	va_start(args, msg);
	lcd_printf_p(msg, args);
	va_end(args);

	// Print error message to terminal (variadic arguments)
	va_start(args, msg);
	terminal_log_printf_p(PSTR("[ERROR] "), msg, args);
	va_end(args);

	// Catch system in this infinite loop and play a small animation to indicate an error
	while (1)
	{
		lcd_displayOn();
		_delay_ms(1500);
		lcd_displayOff();
		_delay_ms(100);
	}
}