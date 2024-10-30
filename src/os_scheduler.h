/*! \file
 *  \brief Scheduling module for the OS.
 *
 *  Contains the scheduler and process switching functionality for the OS.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef _OS_SCHEDULER_H
#define _OS_SCHEDULER_H

#include "lib/defines.h"
#include "os_process.h"

#include <stdbool.h>

//----------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------

//! The enum specifying which scheduling strategies exist
typedef enum SchedulingStrategy
{
	OS_SS_ROUND_ROBIN,
	OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN
} scheduling_strategy_t;

// Change this define to reflect the number of available strategies:
#define SCHEDULING_STRATEGY_COUNT 2

//----------------------------------------------------------------------------
// Function headers
//----------------------------------------------------------------------------

//! get a pointer to the process structure by process ID
process_t *os_getProcessSlot(process_id_t pid);

//! starts the scheduler
void os_startScheduler(void);

//! registers a program (will not be started)
program_id_t os_registerProgram(program_t *program);

//! checks if a program is to be executed at boot-time
bool os_checkAutostartProgram(program_id_t programID);

//! looks up the function of a program with the passed ID (index) and returns NULL on failure
program_t *os_lookupProgramFunction(program_id_t programID);

//! looks up the ID (i.e. index) of a program and returns INVALID_PROGRAM on failure
program_id_t os_lookupProgramID(program_t *program);

//! executes a process by instantiating a program
process_id_t os_exec(program_id_t programID, priority_t priority);

//! returns the number of programs
uint8_t os_getNumberOfRegisteredPrograms(void);

//! initializes scheduler arrays
void os_initScheduler(void);

//! returns the currently active process
process_id_t os_getCurrentProc(void);

//! returns the number of currently active processes
uint8_t os_getNumberOfActiveProcs(void);

//! sets the scheduling strategy
void os_setSchedulingStrategy(scheduling_strategy_t strategy);

//! gets the current scheduling strategy
scheduling_strategy_t os_getSchedulingStrategy(void);

//! calculates the checksum of the stack for the corresponding process of pid.
stack_checksum_t os_getStackChecksum(process_id_t pid);

//! check if the stack pointer is still in its bounds
bool os_isStackInBounds(process_id_t pid);

//! used to kill a running process and clear the corresponding process slot
bool os_kill(process_id_t pid);

//! triggers scheduler to schedule another process
void os_yield();

//----------------------------------------------------------------------------
// Critical section management
//----------------------------------------------------------------------------

//! enters a critical code section
void os_enterCriticalSection(void);

//! leaves a critical code section
void os_leaveCriticalSection(void);

#endif
