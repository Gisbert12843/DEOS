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
	// Prepare frame
	frame_t newFrame;

	newFrame.header.destAddr = destAddr;
	newFrame.header.length = length;
	newFrame.header.srcAddr = serialAdapter_address;
	newFrame.header.startFlag = serialAdapter_startFlag;


	newFrame.footer.checksum = INITIAL_CHECKSUM_VALUE;

	newFrame.header = newFrame.header;
	newFrame.innerFrame = *innerFrame;
	newFrame.footer = newFrame.footer;

	serialAdapter_calculateFrameChecksum(&newFrame.footer.checksum, &newFrame);

	xbee_writeData(&newFrame.header, sizeof(newFrame.header));
	xbee_writeData(innerFrame, length);
	xbee_writeData(&newFrame.footer, sizeof(newFrame.footer));
}

/*!
 *  Blocks process until byteCount bytes are available to be read.
 *
 *  \param byteCount Count of bytes that need to arrive so that the function will unblock
 *  \param frameTimestamp Start time of the first byte arrived from which the timeout will be calculated on
 *  \return False when it times out.
 */
bool serialAdapter_waitForData(uint16_t byteCount, time_t frameTimestamp)
{
	int i = xbee_getNumberOfBytesReceived();

	while (i < byteCount)
	{
		if (serialAdapter_hasTimeout(frameTimestamp, SERIAL_ADAPTER_READ_TIMEOUT_MS))
			return false;
		else
			os_yield();
		i = xbee_getNumberOfBytesReceived();
	}
	return true;
}

/*!
 *  Reads incoming data and processes it. Needs to be called periodically.
 *  Don't read from UART in any other process while this is running.
 */
void serialAdapter_worker()
{
	if (!serialAdapter_waitForData(sizeof(start_flag_t), getSystemTime_ms()))
	{
		return;
	}

	// Parse header one by one, abort if first byte is not part of the start flag
	uint8_t flag_buffer[sizeof(start_flag_t)];

	if (xbee_readBuffer(&flag_buffer[0], 1) != XBEE_SUCCESS)
	{
		return;
	}
	if (xbee_readBuffer(&flag_buffer[1], 1) != XBEE_SUCCESS)
	{
		return;
	}

	if (flag_buffer[0] != (serialAdapter_startFlag & 0xFF))
		return;
	if (flag_buffer[1] != ((serialAdapter_startFlag >> 8) & 0xFF))
		return;

	// Wait for arrival of complete header

	if (!serialAdapter_waitForData(sizeof(frame_header_t) - (sizeof(start_flag_t)), getSystemTime_ms()))
	{
		return;
	}


	frame_t received_frame;

	received_frame.header.startFlag = serialAdapter_startFlag;

	int err;
	err = xbee_readBuffer((uint8_t *)&received_frame.header.srcAddr, sizeof(received_frame.header.srcAddr));
	if (err != XBEE_SUCCESS)
	{
		return;
	}

	err = xbee_readBuffer((uint8_t *)&received_frame.header.destAddr, sizeof(received_frame.header.destAddr));
	if (err != XBEE_SUCCESS)
	{
		return;
	}

	err = xbee_readBuffer((uint8_t *)&received_frame.header.length, sizeof(received_frame.header.length));
	if (err != XBEE_SUCCESS)
	{
		return;
	}





	if (received_frame.header.length > COMM_MAX_INNER_FRAME_LENGTH)
		return;

	// Wait for complete inner frame and footer
	if (!serialAdapter_waitForData(received_frame.header.length + sizeof(frame_footer_t), getSystemTime_ms()))
	{
		return;
	}

	// Read inner frame
	err = xbee_readBuffer((uint8_t *)&received_frame.innerFrame, received_frame.header.length);
	if (err != XBEE_SUCCESS)
	{
		return;
	}

	// Read footer
	err = xbee_readBuffer((uint8_t *)&received_frame.footer, sizeof(frame_footer_t));
	if (err != XBEE_SUCCESS)
	{
		return;
	}


	// Read checksum
	checksum_t frame_checksum = INITIAL_CHECKSUM_VALUE;
	serialAdapter_calculateFrameChecksum(&frame_checksum, &received_frame);
	


	// Verify checksum
	if (frame_checksum != received_frame.footer.checksum)
		return;

	// Check if we are addressed by this frame
	if (received_frame.header.destAddr != ADDRESS_BROADCAST && received_frame.header.destAddr != serialAdapter_address)
		return;



	// Forward to next layer
	serialAdapter_processFrame(&received_frame);
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
	for (uint8_t i = 0; i < length; i++)
	{
		*checksum ^= ((uint8_t *)data)[i];
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
	serialAdapter_calculateChecksum(checksum, &frame->header, sizeof(frame->header));
	serialAdapter_calculateChecksum(checksum, &frame->innerFrame, frame->header.length);
}

void printFrame(frame_t *frame, char* func_name)
{
	printf_P(
		PSTR("\n================ %s ================\n"
		     "Frame Header:\n"
		     "  ├─ Start Flag:           0x%04X\n"
		     "  ├─ Source Address:       %u\n"
		     "  ├─ Destination Address:  %u\n"
		     "  └─ Length:               %u\n\n"
		     "Inner Frame Data:\n"),
		func_name,
		frame->header.startFlag,
		frame->header.srcAddr,
		frame->header.destAddr,
		frame->header.length);

	// Inline printing of frame data
	for (uint8_t i = 0; i < frame->header.length; i++)
	{
		printf_P(PSTR("  ├─ Byte %d: 0x%02X\n"), i, ((uint8_t *)&frame->innerFrame)[i]);
	}

	// Conditionally print end of data or no data
	if (frame->header.length > 0)
	{
		printf_P(PSTR("  └─ End of Data\n"));
	}
	else
	{
		printf_P(PSTR("  └─ No Data\n"));
	}

	// Print footer and close
	printf_P(
		PSTR("\nFrame Footer:\n"
		     "  └─ Checksum:             0x%02X\n"
		     "================================================\n\n"),
		frame->footer.checksum);
}

