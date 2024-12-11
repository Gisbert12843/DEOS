/*!
 *  \brief Layer built on top of UART where frames get assembled.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "serialAdapter.h"
#include "../lib/lcd.h"
#include "../lib/util.h"
#include "../os_core.h"
#include "../os_scheduler.h"
#include "rfAdapter.h"
#include "xbee.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <string.h>

//----------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------

//! Timeout for receiving frames
#define SERIAL_ADAPTER_READ_TIMEOUT_MS ((time_t)500)

//----------------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------------

//! Calculates a checksum of given data
void serialAdapter_calculateChecksum(checksum_t *checksum, void *data, uint8_t length);

//! Calculates a checksum of the frame
void serialAdapter_calculateFrameChecksum(checksum_t *checksum, frame_t *frame);

//! Returns true if timestamp + timeoutMs is a timestamp in the past
bool serialAdapter_hasTimeout(time_t timestamp, time_t timeoutMs);

//----------------------------------------------------------------------------
// Given functions
//----------------------------------------------------------------------------

/*!
 *  Checks if a given timestamp has timed out
 *
 *  \param timestamp Timestamp to check
 *  \param timeoutMs Timeout in milliseconds
 *  \return True if timestamp + timeoutMs is a timestamp in the past
 */
bool serialAdapter_hasTimeout(time_t timestamp, time_t timeoutMs)
{
	return (getSystemTime_ms() - timestamp >= timeoutMs);
}

/*!
 *  Blocks process until at least one byte is available to be read
 */
void serialAdapter_waitForAnyByte()
{
	while (xbee_getNumberOfBytesReceived() == 0)
	{
		os_yield();
	}
}

//----------------------------------------------------------------------------
// Your Homework
//----------------------------------------------------------------------------

/*!
 *  Initializes the serialAdapter and their dependencies
 */
void serialAdapter_init(void)
{
	#warning [Praktikum 3] Implement here
	xbee_init();
}

/*!
 *  Sends a frame with given innerFrame
 *
 *  \param destAddr where to send the frame to
 *  \param length how many bytes the innerFrame has
 *  \param innerFrame buffer as payload of the frame
 */
void serialAdapter_writeFrame(address_t destAddr, inner_frame_length_t length, inner_frame_t *innerFrame)
{
	// Prepare header

	// Write header and calculate its checksum

	// Write inner frame and calculate its checksum

	// Prepare footer and write it

}

/*!
 *  Blocks process until byteCount bytes are available to be read.
 *
 *  \param byteCount Count of bytes that need to arrive so that the function will unblock
 *  \param frameTimestamp Start time of the first byte arrived from which the timeout will be calculated on
 *  \return False when it times out.
 */
bool serialAdapter_waitForData(uint8_t byteCount, time_t frameTimestamp)
{
	#warning [Praktikum 3] Implement here

	while (xbee_getNumberOfBytesReceived() != byteCount)
	{
		if(!serialAdapter_hasTimeout(frameTimestamp,SERIAL_ADAPTER_READ_TIMEOUT_MS))
			os_yield();
		else
	}
	
	
}

/*!
 *  Reads incoming data and processes it. Needs to be called periodically.
 *  Don't read from UART in any other process while this is running.
 */
void serialAdapter_worker()
{
	#warning [Praktikum 3] Implement here

	// Wait for data

	// Wait for arrival of complete header

	// Parse header one by one, abort if first byte is not part of the start flag

	// Wait for complete inner frame and footer

	// Read inner frame

	// Read checksum

	// Verify checksum

	// Check if we are addressed by this frame

	// Forward to next layer

}

/*!
 *  Calculates a checksum of given data
 *
 *  \param checksum pointer to a checksum that will be updated
 *  \param data buffer on which the checksum will be calculated
 *  \param length size of the given buffer
 */
void serialAdapter_calculateChecksum(checksum_t *checksum, void *data, uint8_t length)
{
	#warning [Praktikum 3] Implement here
	
	for(uint8_t i = 0; i<length;i++ )
	{
		*checksum ^= ((uint8_t*)data)[i];
	}
}

/*!
 *  Calculates a checksum of the frame
 *
 *  \param checksum pointer to a checksum that will be updated
 *  \param frame data on which the checksum will be calculated
 */
void serialAdapter_calculateFrameChecksum(checksum_t *checksum, frame_t *frame)
{
	serialAdapter_calculateChecksum(checksum,&frame->header,sizeof(frame_header_t));
	serialAdapter_calculateChecksum(checksum,&frame->innerFrame,frame->header.length);
}
