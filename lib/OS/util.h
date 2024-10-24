/*! \file
 *  \brief Little helpers that don't fit elsewhere.
 *
 *  Tools, utilities and other useful stuff that we didn't bother to
 *  categorize.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef _UTIL_H
#define _UTIL_H

#include "atmega2560constants.h"
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t time_t;

//! Initializes system time
void initSystemTime(void);

//! Returns system time in ms
time_t getSystemTime_ms(void);

//----------------------------------------------------------------------------
// Function headers
//----------------------------------------------------------------------------

//! Waits for some milliseconds
void delayMs(uint16_t ms);

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------

#define sbi(x, b) x |= (1 << (b))
#define cbi(x, b) x &= ~(1 << (b))
#define gbi(x, b) (((x) >> (b)) & 1)

#define LOW(x) ((uint8_t)x)
#define HIGH(x) ((uint8_t)(x >> 8))

//! Assertion function that is used to ensure specific behavior
void assertPstr(bool exp, const char *errormsg, ...);

#define assert(expression, errormsg, ...) assertPstr(expression, PSTR(errormsg), ##__VA_ARGS__)

//----------------------------------------------------------------------------
// Assembler macros
//----------------------------------------------------------------------------

/*!
 * \brief Saves the register context on the stack
 *
 * All registers are saved to the proc's stack. Please note, that all processes
 * need to have their own stack, because we support preemptive scheduling.
 * So collisions with other stacks must be dealt with before making
 * this call, or data may be lost, resulting in weird effects.
 */
#define saveContext()                            \
  asm volatile(                                  \
      "push  r31                           \n\t" \
      "in    r31, __SREG__                 \n\t" \
      "push  r31                           \n\t" \
      "push  r30                           \n\t" \
      "push  r29                           \n\t" \
      "push  r28                           \n\t" \
      "push  r27                           \n\t" \
      "push  r26                           \n\t" \
      "push  r25                           \n\t" \
      "push  r24                           \n\t" \
      "push  r23                           \n\t" \
      "push  r22                           \n\t" \
      "push  r21                           \n\t" \
      "push  r20                           \n\t" \
      "push  r19                           \n\t" \
      "push  r18                           \n\t" \
      "push  r17                           \n\t" \
      "push  r16                           \n\t" \
      "push  r15                           \n\t" \
      "push  r14                           \n\t" \
      "push  r13                           \n\t" \
      "push  r12                           \n\t" \
      "push  r11                           \n\t" \
      "push  r10                           \n\t" \
      "push  r9                            \n\t" \
      "push  r8                            \n\t" \
      "push  r7                            \n\t" \
      "push  r6                            \n\t" \
      "push  r5                            \n\t" \
      "push  r4                            \n\t" \
      "push  r3                            \n\t" \
      "push  r2                            \n\t" \
      "push  r1                            \n\t" \
      "clr   r1                            \n\t" \
      "push  r0                            \n\t");

/*!
 * \brief Restores the register context on the stack
 *
 * All registers are saved to the proc's stack. Please note, that all processes
 * need to have their own stack, because we support preemptive scheduling.
 * So collisions with other stacks must be dealt with before making
 * this call, or data may be lost, resulting in weird effects.
 */
#define restoreContext()                         \
  asm volatile(                                  \
      "pop  r0                             \n\t" \
      "pop  r1                             \n\t" \
      "pop  r2                             \n\t" \
      "pop  r3                             \n\t" \
      "pop  r4                             \n\t" \
      "pop  r5                             \n\t" \
      "pop  r6                             \n\t" \
      "pop  r7                             \n\t" \
      "pop  r8                             \n\t" \
      "pop  r9                             \n\t" \
      "pop  r10                            \n\t" \
      "pop  r11                            \n\t" \
      "pop  r12                            \n\t" \
      "pop  r13                            \n\t" \
      "pop  r14                            \n\t" \
      "pop  r15                            \n\t" \
      "pop  r16                            \n\t" \
      "pop  r17                            \n\t" \
      "pop  r18                            \n\t" \
      "pop  r19                            \n\t" \
      "pop  r20                            \n\t" \
      "pop  r21                            \n\t" \
      "pop  r22                            \n\t" \
      "pop  r23                            \n\t" \
      "pop  r24                            \n\t" \
      "pop  r25                            \n\t" \
      "pop  r26                            \n\t" \
      "pop  r27                            \n\t" \
      "pop  r28                            \n\t" \
      "pop  r29                            \n\t" \
      "pop  r30                            \n\t" \
      "pop  r31                            \n\t" \
      "out  __SREG__, r31                  \n\t" \
      "pop  r31                            \n\t" \
      "reti                                \n\t");

#endif
