/*! \file
 *
 *  Contains basic utility functions used all over the system.
 *
 */
#include "util.h"
#include "../os_core.h"
#include "../os_scheduler.h"
#include "atmega2560constants.h"
#include "defines.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

//! Set TCCR0B accordingly
#define TIMER_PRESCALER 64

//! TIMER_OCR = F_CPU / 1000 / TIMER_PRESCALER
#define TIMER_OCR ((F_CPU / 1000 / TIMER_PRESCALER) - 1)

//! System timestamp with precision 1ms (OCR0A * Prescaler / F_CPU)
volatile time_t os_coarseSystemTime;

/*!
 *  ISR that counts the number of occurred Timer 0 compare matches for the getSystemTime function mainly used in delayMs.
 */
ISR(TIMER0_COMPA_vect)
{
	++os_coarseSystemTime;
}

/*!
 *  Initializes Timer 0 as system clock
 */
void initSystemTime(void)
{
	os_coarseSystemTime = 0;

	// Init timer 0 with prescaler 64
	sbi(TCCR0B, CS00);
	sbi(TCCR0B, CS01);
	cbi(TCCR0B, CS02);

	// CTC mode
	sbi(TCCR0A, WGM01);

	// Compare Match after 61 timer ticks = 1ms
	OCR0A = TIMER_OCR;
	sbi(TIMSK0, OCIE0A);
}

/*!
 *  Get precise system Time with a precision of 1 ms
 *
 *  \return The current system time in milliseconds
 */
time_t getSystemTime_ms(void)
{
	// In case interrupts are off we check the OCF manually, clear it and
	// increment os_coarseSystemTime to avoid freezing the system time
	if (!gbi(SREG, 7) && gbi(TIFR0, OCF0A))
	{
		sbi(TIFR0, OCF0A);
		++os_coarseSystemTime;
	}

	// Synchronize access to os_coarseSystemTime
	uint8_t ie = gbi(SREG, 7);
	cli();
	time_t t = os_coarseSystemTime;
	if (ie)
	{
		sei();
	}

	return t;
}

/*!
 *  Function that may be used to wait for specific time intervals.
 *  Therefore, we calculate the relative time to wait. This value is added to the current system time
 *  in order to get the destination time. Then, we wait until a temporary variable reaches this value.
 *
 *  \param ms  The time to be waited in milliseconds (max. 65535)
 */
void delayMs(uint16_t ms)
{
#ifdef RUNNING_WITH_SIMULATOR
	return;
#endif

	if (ms == 0)
	{
		return;
	}
	time_t startTime = getSystemTime_ms();

	while (getSystemTime_ms() - startTime < ms)
	{
		_delay_us(100);
	}
}

/*!
 *  Simple assertion function that is used to ensure specific behavior
 *
 *  \param exp The boolean expression that is expected to be true.
 *  \param errormsg The string that shall be shown as an error message if the given expression is false as progmem string.
 */
void assertPstr(bool exp, const char *errormsg, ...)
{
	if (!exp)
	{
		va_list args;
		va_start(args, errormsg);
		os_errorPstr(errormsg, args);
		va_end(args);
	}
}