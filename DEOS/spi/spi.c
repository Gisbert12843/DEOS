/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "spi.h"
#include "../lib/atmega2560constants.h"
#include "../lib/lcd.h"
#include "../lib/util.h"
#include "../os_scheduler.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

// Hardware mappings
#define SPI_PORT /* */ PORTB
#define SPI_DDR /*  */ DDRB
#define SPI_PIN /*  */ PINB

#define SPI_CS_BIT /*   */ PB0
#define SPI_MOSI_BIT /* */ PB2
#define SPI_MISO_BIT /* */ PB3
#define SPI_CLK_BIT /*  */ PB1

// Long form of registers
#define SPI_CONTROL_REGISTER /*   */ SPCR
#define SPI_ENABLE /*             */ SPE
#define SPI_DATA_ORDER /*         */ DORD
#define SPI_MASTER_ENABLE /*      */ MSTR
#define SPI_CLOCK_POLARITY /*     */ CPOL
#define SPI_CLOCK_PHASE /*        */ CPHA
#define SPI_CLOCK_RATE_SELECT0 /* */ SPR0
#define SPI_CLOCK_RATE_SELECT1 /* */ SPR1
#define SPI_DUMMY_BYTE /*         */ 0xFF

//----------------------------------------------------------------------------
// Given functions
//----------------------------------------------------------------------------

/*!
 *  Enables the SPI chip select
 */
inline void spi_cs_enable()
{
	SPI_PORT &= ~(1 << SPI_CS_BIT);
}

/*!
 *  Disables the SPI chip select
 */
inline void spi_cs_disable()
{
	SPI_PORT |= (1 << SPI_CS_BIT);
}

//----------------------------------------------------------------------------
// Your Homework
//----------------------------------------------------------------------------

/*!
 *  Initializes the SPI interface
 */
void spi_init()
{
	// Set the direction of the SPI pins
	// Output: CS, MOSI, CLK
	// Input: MISO
	SPI_DDR |= (1 << SPI_CS_BIT) | (1 << SPI_MOSI_BIT) | (1 << SPI_CLK_BIT);

	// Disable CS
	spi_cs_disable();

	// Enable the pull-up resistor for MISO
	SPI_PORT |= (1 << SPI_MISO_BIT);

	// TLCD SPI: Mode 3, LSB first (CPOL=1, CPHA=1, DORD=1)
	// TLCD SPI Speed: 200 kHz withouth pause between bytes,
	//                 3 MHz with 100 µs pause between bytes

	// ATmega 2560 @16MHz:
	// SPR = 00 -> fosc/4   = 4000kHz (too fast)
	// SPR = 01 -> fosc/16  = 1000kHz (100 µs pause between bytes)
	// SPR = 10 -> fosc/64  =  250kHz (100 µs pause between bytes)
	// SPR = 11 -> fosc/128 =  125kHz (no pause)

	// Configure the SPI control register:
	// Enable SPI 			 						-> SPI_ENABLE: 1
	// Data Order: 0:MSB first, 1:LSB first			-> SPI_DATA_ORDER: 1
	// ATmega is Master								-> SPI_MASTER_ENABLE: 1
	// Clock Polarity: 0:low, 1:high (idle)			-> SPI_CLOCK_POLARITY: 1
	// Clock Phase: 0:leading, 1:trailing (sample) 	-> SPI_CLOCK_PHASE: 1
	// Clock Rate: fosc/128							-> SPI_CLOCK_RATE_SELECT0: 1, SPI_CLOCK_RATE_SELECT1: 1
	SPI_CONTROL_REGISTER = (1 << SPI_ENABLE) |
						   (1 << SPI_DATA_ORDER) |
						   (1 << SPI_MASTER_ENABLE) |
						   (1 << SPI_CLOCK_POLARITY) |
						   (1 << SPI_CLOCK_PHASE) |
						   (1 << SPI_CLOCK_RATE_SELECT0) |
						   (1 << SPI_CLOCK_RATE_SELECT1);
}

/*!
 *  Writes and simultaneously reads a byte over the SPI interface
 *
 *  \param byte The byte that will be sent
 *  \return The byte that has been read
 */
uint8_t spi_write_read(uint8_t byte)
{
	os_enterCriticalSection();

	// spi_cs_enable(); // Select the slave
	//_delay_us(6);	 // wait for slave to be ready

	// send the byte
	SPDR = byte;

	// wait for transmission to be completed
	while (!(SPSR & (1 << SPIF)))
		;

	// read the received byte
	uint8_t receivedByte = SPDR;

	// deselect the slave
	// spi_cs_disable();

	os_leaveCriticalSection();

	return receivedByte;
}

/*!
 *  Reads a byte over the SPI interface by writing a dummy byte
 *
 *  \return The byte that has been read
 */
uint8_t spi_read()
{
	// spi_cs_enable(); // Select the slave
	//_delay_us(6);	 // wait for slave to be ready
	return spi_write_read(SPI_DUMMY_BYTE);
	// spi_cs_disable(); // Deselect the slave
}

/*!
 *  Writes a byte over the SPI interface and discards the received byte
 *
 *  \param byte the byte that will be sent
 */
void spi_write(uint8_t byte)
{
	// spi_cs_enable(); // Select the slave
	//_delay_us(6);	 // wait for slave to be ready
	spi_write_read(byte);
	// spi_cs_disable(); // Deselect the slave
}

/*!
 *  Writes multiple bytes over the SPI interface
 *
 *  \param data buffer with will be sent through SPI
 *  \param length size of the buffer
 */
void spi_writeData(const void *data, uint8_t length)
{
	// os_enterCriticalSection();

	// spi_cs_enable(); // Select the slave
	//_delay_us(6);	 // wait for slave to be ready

	for (uint8_t i = 0; i < length; i++)
	{
		spi_write_read(((uint8_t *)data)[i]);
	}

	// spi_cs_disable(); // Deselect the slave

	// os_leaveCriticalSection();
}

/*!
 *  Writes multiple bytes over the SPI interface from program memory
 *
 *  \param data buffer with will be sent through SPI
 *  \param length size of the buffer
 */
void spi_writeDataProgMem(const void *data, uint8_t length)
{
	// os_enterCriticalSection();

	// spi_cs_enable(); // Select the slave
	//_delay_us(6);	 // wait for slave to be ready

	for (uint8_t i = 0; i < length; i++)
	{
		spi_write_read(pgm_read_byte(data + i));
	}

	// spi_cs_disable(); // Deselect the slave

	// os_leaveCriticalSection();
}