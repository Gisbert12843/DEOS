/*!
 *  \brief Layer forwarding the underlying UART library.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "xbee.h"
#include "../lib/uart.h"
#include "../os_scheduler.h"
#include "rfAdapter.h"
#include <string.h>


#include <avr/interrupt.h>

//----------------------------------------------------------------------------
// Your Homework
//----------------------------------------------------------------------------

/*!
 *  Initializes the XBee
 */
void xbee_init()
{
	uart3_init(UART_BAUD_SELECT(38400,16000000UL));
}

/*!
 *  Transmits one byte to the XBee
 *
 *  \param byte one byte that will be sent through UART
 */
void xbee_write(uint8_t byte)
{
	uart3_putc(byte);
}

/*!
 *  Transmits the given data to the XBee
 *
 *  \param data buffer with will be sent through UART
 *  \param length size of the buffer
 */
void xbee_writeData(void *data, uint8_t length)
{
	for(int i = 0; i < length;i++)
	{
		xbee_write(*(((uint8_t*)data)+i));
	}
}

/*
 *  Receives one byte from the XBee
 *
 *  \param byte Reference parameter where the read byte will be written to
 *  \return Error code or XBEE_SUCCESS. When XBEE_BUFFER_INCONSISTENCY gets returned, `byte` still gets updated
 */
uint8_t xbee_read(uint8_t *byte)
{
	//we are assuming that "int" from the uart library is implemented as uint16_t, kinda ugly ngl
	uint16_t temp = (uint16_t)uart3_getc();
	
	switch((temp >> 8) & 0xFF)
	{
		case 0:
		{
			*byte = (uint8_t)temp;
			return XBEE_SUCCESS;
		}
		break;
		
		case UART_OVERRUN_ERROR:
		{
			*byte = (uint8_t)temp;
			return XBEE_BUFFER_INCONSISTENCY;
		}
		break;
		
		case UART_BUFFER_OVERFLOW:
		{
			*byte = (uint8_t)temp;
			return XBEE_BUFFER_INCONSISTENCY;
		}
		break;
		
		case UART_FRAME_ERROR:
		{
			return XBEE_READ_ERROR;
		}
		break;
		
		case UART_NO_DATA:
		{
			return XBEE_DATA_MISSING;
		}
		break;
		default:
			break;
	}
	return 255;
}



/*!
 *	Returns current filling of the buffer in byte
 *
 *  \return count of bytes that can be received through `xbee_read`
 */
uint16_t xbee_getNumberOfBytesReceived()
{
	return uart3_getrxcount();
}

/*!
 *  Receives `length` bytes and writes them to `buffer`. Make sure there are enough bytes to be read
 *
 *	\param message_buffer Buffer where to store received bytes
 *  \param buffer_size Amount of bytes that need to be received
 *  \return error Code if there is any. If it's not XBEE_SUCCESS, you shouldn't use the result
 */
uint8_t xbee_readBuffer(uint8_t *buffer, uint8_t length)
{
	if (xbee_getNumberOfBytesReceived() < length)
		return XBEE_DATA_MISSING;

	uint8_t temp_buff[length];
	
	for (uint8_t i = 0; i < length; i++)
	{
		uint8_t err = xbee_read(&temp_buff[i]);
		if (err != XBEE_SUCCESS)
			return err; // Early return if an error occurs
	}

	// Copy received data into destination buffer
	memcpy(buffer, temp_buff, length);

	return XBEE_SUCCESS;
}
