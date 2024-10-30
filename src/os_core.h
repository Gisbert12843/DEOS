/*! \file
 *  \brief Core of the OS.
 *
 *  Contains functionality to for core OS operations.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef _OS_CORE_H
#define _OS_CORE_H

#include "lib/defines.h"
#include "lib/lcd.h"
#include "lib/terminal.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "os_scheduler.h"

/*!
 *  Terminates the OS and displays a corresponding error on the LCD.
 *
 *  \param str  The error to be displayed
 *
 */
#define os_error(msg, ...) os_errorPstr(PSTR(msg), ##__VA_ARGS__)

//! Needed to determine where the heap may start in order not to crush global variables unknowingly
extern uint8_t const __heap_start;

//----------------------------------------------------------------------------
// Function headers
//----------------------------------------------------------------------------

//! Initializes timers
void os_initTimer(void);

//! Initializes OS
void os_init();

//! Terminates the OS and displays a corresponding error on the LCD and terminal
void os_errorPstr(const char *msg, ...);

#endif
