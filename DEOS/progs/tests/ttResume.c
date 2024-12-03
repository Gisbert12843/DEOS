//-------------------------------------------------
//          TestSuite: Resume
//-------------------------------------------------
// Runs several processes that depend on each
// other.
// This shows if the OS correctly resumes to
// processes that have been executed and paused by
// the scheduler.
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_RESUME

#include "../../lib/defines.h"
#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../os_core.h"
#include "../../os_process.h"
#include <avr/interrupt.h>

#define DELAY 500

uint8_t volatile i = 0;
uint8_t volatile iFlag = 0;

/*!
 * Prog 1 and 3 take turns printing.
 * Doing some simple communication
 */
PROGRAM(1, AUTOSTART) {
    for (;;) {
        while (iFlag);
        lcd_writeChar('0' + i);
        iFlag = 1;
        delayMs(DELAY);
    }
}

/*!
 * Program that prints Characters in ascending order
 */
PROGRAM(2, AUTOSTART) {
    // Declare register bound variable to test recovery of register values after scheduling
    register uint8_t j = 0;
    for (;;) {
        lcd_writeChar('a' + j);
        j = (j + 1) % ('z' - 'a' + 1);
        delayMs(DELAY);
    }
}

/*!
 * Prog 1 and 3 take turns printing.
 * Doing some simple communication.
 */
PROGRAM(3, AUTOSTART) {
    uint8_t k;
    for (;;) {
        if (iFlag) {
            k = i + 1;
            i = k % 10;
            iFlag = 0;
            lcd_writeChar(' ');
        }
        delayMs(DELAY);
    }
}

#endif