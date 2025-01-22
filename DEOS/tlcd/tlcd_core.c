/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "tlcd_core.h"
#include "../lib/atmega2560constants.h"
#include "../lib/lcd.h"
#include "../lib/util.h"
#include "../os_core.h"
#include "../os_scheduler.h"
#include "../spi/spi.h"
#include "tlcd_graphic.h"

#include <util/delay.h>

// #define DEBUG_SPI_LOW_LEVEL

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

bool tlcd_initialized = false;

//----------------------------------------------------------------------------
// Given functions
//----------------------------------------------------------------------------

inline bool spi_ack_or_timeout(uint8_t *retries)
{
	uint8_t read = spi_read();

#ifdef DEBUG_SPI_LOW_LEVEL
	if (read == ACK)
	{
		terminal_writeProgString(PSTR(" ACK"));
	}
	else if (read == NAK)
	{
		terminal_writeProgString(PSTR(" NCK"));
	}
	else
	{
		terminal_writeProgString(PSTR(" ???"));
	}
#endif

	return read != ACK && (*retries)++ < TLCD_MAX_RETRIES;
}

/*!
 *  This function requests the sending buffer from the TLCD. Should only be called, if SBUF pin is low (or through polling).
 */
void tlcd_requestData()
{
	const uint8_t bytesToSend[] = {DC2_BYTE, 0x01, S_BYTE};

	uint8_t bcc = INITIAL_BCC_VALUE;
	tlcd_calculateBCC(&bcc, bytesToSend, sizeof(bytesToSend));

	os_enterCriticalSection();

	uint8_t retries = 0;

	do
	{
		spi_writeData(bytesToSend, sizeof(bytesToSend));
		spi_write(bcc);
	} while (spi_ack_or_timeout(&retries));

	if (retries >= TLCD_MAX_RETRIES)
	{
		// os_error("no ACK");
	}

	os_leaveCriticalSection();
}

//----------------------------------------------------------------------------
// Your Homework
//----------------------------------------------------------------------------

/*!
 *  Initializes the TLCD
 */
void tlcd_init()
{
	if (tlcd_initialized)
	{
		return; // don't initialize twice
	}

	os_enterCriticalSection();

	spi_init();

	// spi_cs_disable(); // Done in spi_init

	tlcd_clearDisplay();

	tlcd_initialized = true;

	os_leaveCriticalSection();
}

/*!
 *  Returns true if the TLCD has been initialized
 */
bool tlcd_isInitialized()
{
	return tlcd_initialized;
}

/*!
 * Sends a command to the TLCD. Header and checksum will be added automatically.
 *
 * \param cmd pointer to the command data buffer
 * \param len length of the command data buffer
 */
void tlcd_writeCommand(const void *cmd, uint8_t len)
{
	os_enterCriticalSection();

	// Pre calculate BCC
	uint8_t bcc = INITIAL_BCC_VALUE + DC1_BYTE + len;
	tlcd_calculateBCC(&bcc, cmd, len);

	// DEBUG: Print command
#ifdef DEBUG_SPI_LOW_LEVEL
	DEBUG("tlcd_writeCommand(len: %d):", len);
	terminal_writeProgString(PSTR("        Sending: 11 "));
	terminal_writeHexByte(len);
	terminal_writeChar(' ');
	terminal_writeHexByte(DC1_BYTE);
	terminal_writeChar(' ');
	for (uint8_t i = 0; i < len; i++)
	{
		terminal_writeHexByte(((uint8_t *)cmd)[i]);
		terminal_writeChar(' ');
	}
	terminal_writeHexByte(bcc);
#endif

	// Send: DC1, len, cmd, BCC; repeat until ACK received
	uint8_t retries = 0;

	spi_cs_enable();

	do
	{
		spi_write(DC1_BYTE);
		spi_write(len);
		spi_writeData(cmd, len);
		spi_write(bcc);
	} while (spi_ack_or_timeout(&retries));

#ifdef DEBUG_SPI_LOW_LEVEL
	if (retries >= TLCD_MAX_RETRIES)
	{
		// os_error("no ACK");
		terminal_writeIndentedProgString(PSTR(" TLCD_MAX_RETRIES"));
	}

	terminal_newLine();
#endif

	spi_cs_disable();

	os_leaveCriticalSection();
}

/*!
 *  Calculates the BCC of a given data buffer and adds it to the given BCC value through mutating it.
 *
 *  \param bcc pointer to the BCC value that will be mutated
 *  \param data pointer to the data buffer
 *  \param len length of the data buffer
 */
void tlcd_calculateBCC(uint8_t *bcc, const void *data, uint8_t len)
{
	for (uint8_t i = 0; i < len; i++)
	{
		*bcc += ((uint8_t *)data)[i];
	}
}

/*!
 *  Calculates the BCC of a given data buffer in program memory and adds it to the given BCC value through mutating it.
 *
 *  \param bcc pointer to the BCC value that will be mutated
 *  \param data pointer to the data buffer in program memory
 *  \param len length of the data buffer
 */
void tlcd_calculateBCC_ProgMem(uint8_t *bcc, const void *data, uint8_t len)
{
	for (uint8_t i = 0; i < len; i++)
	{
		*bcc += pgm_read_byte((uint8_t *)data + i);
	}
}
