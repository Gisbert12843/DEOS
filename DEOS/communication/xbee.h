/*!
 *  \brief Layer forwarding the underlying UART library.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef XBEE_H_
#define XBEE_H_

#include <stdbool.h>
#include <stdint.h>

#define XBEE_SUCCESS 0
#define XBEE_BUFFER_INCONSISTENCY (1 << 0)
#define XBEE_READ_ERROR (1 << 1)
#define XBEE_DATA_MISSING (1 << 2)

//! Initializes the UART connection
void xbee_init();

//! Polls for incoming bytes at the XBee, returns error status
uint8_t xbee_read(uint8_t *byte);

//! Transmits one byte to the XBee
void xbee_write(uint8_t byte);

//! Transmits the given data to the XBee
void xbee_writeData(void *data, uint8_t length);

//! Reads data to the buffer
uint8_t xbee_readBuffer(uint8_t *buffer, uint8_t length);

//! Returns current filling of the buffer in byte
uint16_t xbee_getNumberOfBytesReceived();

#endif /* XBEE_H_ */