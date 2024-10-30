/*! \file
 *  \brief Constants for ATMega2560 board
 *  Defines several useful constants for the ATMega2560
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef _ATMEGA2560CONSTANTS_H
#define _ATMEGA2560CONSTANTS_H

#define __AVR_ATmega328P__

// uncomment when running with simulator to disable delay loops
// #define RUNNING_WITH_SIMULATOR

//----------------------------------------------------------------------------
// Board properties
//----------------------------------------------------------------------------

//! Clock frequency on evaluation board in HZ (16 MHz)
#define AVR_CLOCK_FREQUENCY 16000000ul

#ifdef F_CPU
#undef F_CPU
#endif
//! Clock frequency for WinAVR delay function
#define F_CPU AVR_CLOCK_FREQUENCY

//! For running with simulator so it doesn't take forever in delay loops
#ifdef RUNNING_WITH_SIMULATOR
#undef F_CPU
#define F_CPU 0ul
#endif

//----------------------------------------------------------------------------
// MC properties
//----------------------------------------------------------------------------

//! Flash memory available on AVR (in bytes) (256 KiB)
#define AVR_MEMORY_FLASH (1ul << 18)

//! SRAM memory available on AVR (in bytes) (8 KiB)
#define AVR_MEMORY_SRAM (1ul << 13)

//! EEPROM memory available on AVR (in bytes) (4 KiB)
#define AVR_MEMORY_EEPROM (1ul << 12)

//! Starting address of SRAM memory
#define AVR_SRAM_START 0x0200

//! Ending address of SRAM memory -- First invalid address
#define AVR_SRAM_END (AVR_SRAM_START + AVR_MEMORY_SRAM)

//! Last address of SRAM memory
#define AVR_SRAM_LAST (AVR_SRAM_END - 1)

#endif
