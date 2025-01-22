/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef TLCD_CORE_H_
#define TLCD_CORE_H_

#include <stdbool.h>
#include <stdint.h>

#define INITIAL_BCC_VALUE 0
#define TLCD_MAX_RETRIES 50

// Protocol specific constants
#define ESC_BYTE 0x1B
#define NUL_BYTE 0x00
#define A_BYTE 0x41
#define B_BYTE 0x42
#define C_BYTE 0x43
#define D_BYTE 0x44
#define E_BYTE 0x45
#define F_BYTE 0x46
#define G_BYTE 0x47
#define H_BYTE 0x48
#define I_BYTE 0x49
#define J_BYTE 0x4A
#define K_BYTE 0x4B
#define L_BYTE 0x4C
#define M_BYTE 0x4D
#define N_BYTE 0x4E
#define O_BYTE 0x4F
#define P_BYTE 0x50
#define Q_BYTE 0x51
#define R_BYTE 0x52
#define S_BYTE 0x53
#define T_BYTE 0x54
#define U_BYTE 0x55
#define V_BYTE 0x56
#define W_BYTE 0x57
#define X_BYTE 0x58
#define Y_BYTE 0x59
#define Z_BYTE 0x5A
#define DC1_BYTE 0x11
#define DC2_BYTE 0x12
#define ACK 0x06
#define NAK 0x15

#define TLCD_DDR DDRB
#define TLCD_PORT PORTB
#define TLCD_RESET_BIT PB3

//! Physical size of the display
#define TLCD_WIDTH 480
#define TLCD_HEIGHT 272

//! Initializes the TLCD
void tlcd_init();

//! Check if the TLCD has been initialized
bool tlcd_isInitialized();

//! Sends a command to the TLCD. Header and checksum will be added automatically.
void tlcd_writeCommand(const void* cmd, uint8_t len);

//! Calculates the tlcd checksum of a given data buffer
void tlcd_calculateBCC(uint8_t* bcc, const void* data, uint8_t len);

//! Calculates the tlcd checksum of a given data buffer in program memory
void tlcd_calculateBCC_ProgMem(uint8_t* bcc, const void* data, uint8_t len);

//! Sends a request to the TLCD so it sends event data back
void tlcd_requestData();

#endif /* TLCD_CORE_H_ */