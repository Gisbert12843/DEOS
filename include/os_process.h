/*! \file
 *  \brief Struct specifying a process.
 *
 *  Contains the struct that bundles all properties of a running process.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef _OS_PROCESS_H
#define _OS_PROCESS_H

#include <stdbool.h>
#include <stdint.h>

//! The type for the ID of a running process.
typedef uint8_t process_id_t;

//! The type for the ID of a program.
typedef uint8_t program_id_t;

//! The type for the checksum used to check stack consistency.
typedef uint8_t stack_checksum_t;

//! Type for the state a specific process is currently in.
typedef enum ProcessState
{
  OS_PS_UNUSED,
  OS_PS_READY,
  OS_PS_RUNNING
} process_state_t;

//! The type of the priority of a process.
typedef enum Priority
{
  OS_PRIO_HIGH,
  OS_PRIO_NORMAL,
  OS_PRIO_LOW
} priority_t;

#define PRIORITY_COUNT 3

//! A union that holds the current stack pointer of a given process.
//! We use a union so we can reduce the number of explicit casts.
typedef union StackPointer
{
  uint16_t as_int;
  uint8_t *as_ptr;
} stack_pointer_t;

//! The struct that holds all information for a process.
//! Note that additional scheduling information (such as the current time-slice)
//! are stored by the module that implements the actual scheduling strategies.
typedef struct Process
{
  program_id_t progID;
  process_state_t state;
  stack_pointer_t sp;
  priority_t priority;
  stack_checksum_t checksum; // will be relevant in task_02
} process_t;

//! This is the type of a program function (not the pointer to one!).
typedef void program_t(void);

//! Specifies if a program should be automatically executed on boot-up.
typedef enum
{
  DONTSTART,
  AUTOSTART
} on_start_do_t;

/*!
 * Defines a program function with the name prog0, prog1, prog2, ...
 * depending on the numerical index you pass as the first macro-parameter.
 * Note that this will break, if you pass a non-trivial expression as index,
 * e.g. PROGRAMM(1+1, ...) will not have the desired effect, but will in fact
 * not even compile. Make sure to always pass natural numbers as first argument.
 * The second macro parameter specifies whether you want this program to be
 * auto-magically executed when the system boots.
 * If you pass 'AUTOSTART', it will create a process for this program while
 * initializing the scheduler. If you pass 'DONTSTART' instead, only the
 * program will be registered (which you may execute manually).
 * Use this macro in this fashion:
 *
 *   PROGRAM(3, AUTOSTART) {
 *     foo();
 *     bar();
 *     ...
 *   }
 */
#define PROGRAM(INDEX, ON_START_DO)                               \
  void program_with_index_##INDEX##_defined_twice(void) {}        \
  program_t prog##INDEX;                                          \
  void registerProgram##INDEX(void) __attribute__((constructor)); \
  void registerProgram##INDEX(void)                               \
  {                                                               \
    program_t **os_getProgramSlot(program_id_t progId);           \
    *(os_getProgramSlot(INDEX)) = prog##INDEX;                    \
    extern uint16_t os_autostart;                                 \
    os_autostart |= (ON_START_DO == AUTOSTART) << (INDEX);        \
  }                                                               \
  void prog##INDEX(void)

//! Returns whether the passed process can be selected to run.
bool os_isRunnable(process_t const *process);

#endif
