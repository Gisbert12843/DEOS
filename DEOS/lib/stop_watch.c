#include "stop_watch.h"
#include "../os_core.h"
#include "../os_scheduler.h"
#include "util.h"
#include <avr/interrupt.h>
#include <avr/io.h>

uint8_t stopWatch_runningInstances = 0;
volatile time_t stopWatch_time = 0;
time_t stopWatch_offset = 10;

/* Private function declarations */
time_t stopWatch_getTime(void);
void stopWatch_deactivateInterrupt(void);
void stopWatch_activateInterrupt(void);

/*!
 *  ISR that counts the number of occurred Timer 1 compare matches for the getTime function
 */
ISR(TIMER1_COMPA_vect)
{
    // braces to increase probability of compiler optimization
    stopWatch_time += (0xFFFF / 2 + 1); // every counter tick is 1/2 microsecond, overflows at 0x10000 counts
}

/*!
 *  Deactivates the interrupt for the stop watch
 */
void stopWatch_deactivateInterrupt(void)
{
    cbi(TCCR1B, CS10); // disabled	0
    cbi(TCCR1B, CS11); // disabled	0
    cbi(TCCR1B, CS12); // disabled	0
}

/*!
 *  Activates the interrupt for the stop watch
 */
void stopWatch_activateInterrupt(void)
{
    cbi(TCCR1B, CS10); // /8 prescaler	0
    sbi(TCCR1B, CS11); // /8 prescaler	1
    cbi(TCCR1B, CS12); // /8 prescaler	0
}

/*!
 *  Initializes the stop watch
 */
void stopWatch_init(void)
{
    // Set Timer/Counter Control Registers to 0
    TCCR1A = 0x00; // Normal mode

    // Set the Timer/Counter Register to 0
    TCNT1 = 0;

    // Set the compare match value to 1 (1 microsecond)
    OCR1A = 1;

    // Enable the Output Compare Match A interrupt
    TIMSK1 |= (1 << TOIE1);

    stopWatch_deactivateInterrupt();
}

/*!
 *  Safe way to read current time value
 *
 *  \return The current time value
 */
time_t stopWatch_getTime(void)
{
    // In case interrupts are off we check the overflow flag manually, clear it and
    // increment stopWatch_time to avoid freezing the system time
    if (!gbi(SREG, 7) && gbi(TIFR1, TOV1))
    {
        sbi(TIFR1, TOV1);
        stopWatch_time += (0xFFFF / 2 + 1); // every counter tick is 1/2 microsecond, overflows at 0x10000 counts
    }
    uint16_t counted = TCNT1;
    TCNT1 = TCNT1 & 0b1;           // if its odd /2 lost one count (so we offset with 1 in case it is odd)
    stopWatch_time += counted / 2; // every counter tick is 1/2 microsecond

    // Synchronize access to stopWatch_time
    uint8_t ie = gbi(SREG, 7);
    cli();
    time_t time = stopWatch_time;
    if (ie)
    {
        sei();
    }
    return time;
}

/*!
 *  Starts the stop watch and returns a handler with that you'd retrieve your measurement later
 *
 *  \return The handler to the stop watch
 */
stop_watch_handler_t stopWatch_start(void)
{
    os_enterCriticalSection();
    if (stopWatch_runningInstances == 0xFF)
    {
        os_error("Stop watch instances overflow");
    }
    ++stopWatch_runningInstances;
    if (stopWatch_runningInstances == 1)
    {
        sbi(TIFR1, OCF1A); // Clear potentially unhandled interrupt flag
        stopWatch_time = 0;
        TCNT1 = 0;
        stopWatch_activateInterrupt();
    }
    stop_watch_handler_t handler = stopWatch_getTime();
    os_leaveCriticalSection();

    return handler;
}

/*!
 *  Measures the time in micro seconds that elapsed since the handler was created
 *
 *  \param stopWatchHandler The handler to the stop watch
 *  \return The measured time
 */
time_t stopWatch_measure(stop_watch_handler_t stopWatchHandler)
{
    return stopWatch_getTime() - stopWatchHandler - stopWatch_offset;
}

/*!
 *  Measures the time in micro seconds that elapsed since the handler was created.
 *  Stops the stop watch.
 *
 *  \param stopWatchHandler The handler to the stop watch
 *  \return The measured time
 */
time_t stopWatch_stop(stop_watch_handler_t stopWatchHandler)
{
    os_enterCriticalSection();
    time_t measurement = stopWatch_measure(stopWatchHandler);
    if (stopWatch_runningInstances == 0)
    {
        os_error("Stop watch underflow");
    }
    --stopWatch_runningInstances;
    if (stopWatch_runningInstances == 0)
    {
        stopWatch_deactivateInterrupt();
    }
    os_leaveCriticalSection();
    return measurement;
}