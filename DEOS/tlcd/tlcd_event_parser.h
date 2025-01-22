/*!
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef TLCD_EVENT_PARSER_H_
#define TLCD_EVENT_PARSER_H_

#include <stdint.h>

//! Identifier for a TOUCHPANEL_EVENT
#define TOUCHPANEL_EVENT 0x48

//! Enum giving information if the event announces pressing or letting go of the touch panel
typedef enum TouchEventType
{
	TOUCHPANEL_UP = 0,
	TOUCHPANEL_DOWN = 1,
	TOUCHPANEL_DRAG = 2,
} touch_event_type_t;

//! Struct holding the information that are transmitted when a TOUCHPANEL_EVENT occurs
typedef struct TouchEvent
{
	touch_event_type_t type;
	uint16_t x;
	uint16_t y;
} touch_event_t;

//! Callback function for touch events
typedef void EventCallback(touch_event_t event);

//! Set the function to be called if a touch event was detected
void tlcd_event_setCallback(EventCallback *callback);

//! Parse a received event and call the callback function if a touch event was detected
void tlcd_event_worker();

#endif /* TLCD_EVENT_PARSER_H_ */