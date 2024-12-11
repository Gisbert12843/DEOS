/*!
 *  \brief Layer built on top of UART where frames get assembled.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef SERIAL_ADAPTER_H_
#define SERIAL_ADAPTER_H_

#include "../lib/util.h"

#include <stdbool.h>
#include <stdint.h>

// Typedefs
typedef uint16_t start_flag_t;
typedef uint8_t address_t;
typedef uint8_t command_t;
typedef uint8_t inner_frame_length_t;
typedef uint8_t checksum_t;

// Defines
#define COMM_START_FLAG_LENGTH sizeof(start_flag_t)
#define COMM_HEADER_LENGTH (COMM_START_FLAG_LENGTH + sizeof(address_t) * 2 + sizeof(inner_frame_length_t))
#define COMM_FOOTER_LENGTH sizeof(checksum_t)
#define COMM_MAX_PAYLOAD_LENGTH 48
#define COMM_MAX_INNER_FRAME_LENGTH (sizeof(checksum_t) + COMM_MAX_PAYLOAD_LENGTH)
#define ADDRESS_BROADCAST ((address_t)255)

// Structs
//! Specification of the header of the outer communication frame
typedef struct FrameHeader
{
	start_flag_t startFlag;
	address_t srcAddr;
	address_t destAddr;
	inner_frame_length_t length;
} frame_header_t;

//! Specification of a communication frame, that is the inner box for every command
typedef struct InnerFrame
{
	command_t command;
	uint8_t payload[COMM_MAX_PAYLOAD_LENGTH];
} inner_frame_t;

//! Specification of the footer of the outer communication frame
typedef struct FrameFooter
{
	checksum_t checksum;
} frame_footer_t;

//! Specification of a communication frame, that is the outer box for every command
typedef struct Frame
{
	frame_header_t header;
	inner_frame_t innerFrame;
	frame_footer_t footer;
} frame_t;

//! Start-Flag that announces a new frame
extern start_flag_t serialAdapter_startFlag;

//! Configuration what address this microcontroller has
extern address_t serialAdapter_address;

//! Is called on command frame receive
extern void serialAdapter_processFrame(frame_t *frame);

//! Initializes the serial adapter
void serialAdapter_init(void);

//! Reads incoming data and processes it
void serialAdapter_worker(void);

//! Sends a frame with given innerFrame
void serialAdapter_writeFrame(address_t destAddr, inner_frame_length_t length, inner_frame_t *innerFrame);

//! Blocks process until byteCount bytes arrived
bool serialAdapter_waitForData(uint8_t byteCount, time_t frameTimestamp);

//! Blocks process until at least one byte can be read from input buffer
void serialAdapter_waitForAnyByte();

#endif /* SERIAL_ADAPTER_H_ */
