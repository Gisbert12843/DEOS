/*
 * display_prog1.c
 *
 * Created: 11/01/2025 01:55:13
 *  Author: wwwgi
 */
#include "../progs.h"

#if defined(USER_PROGRAM_ENABLED) && USER_PROGRAM == 5

#include "../../communication/rfAdapter.h"
#include "../../lib/buttons.h"
#include "../../lib/defines.h"
#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../os_scheduler.h"
#include "../../lib/terminal.h"
#include "../../tlcd/tlcd_graphic.h"
#include "../../tlcd/tlcd_core.h"
#include "string.h"
#include <stdint.h>

#include <stdbool.h>

#include "../../gui/gui.h"
#include "../../gui/gui_helper.h"

#include "../../communication/serialAdapter.h"

PROGRAM(1, AUTOSTART)
{
    rfAdapter_init();
    while (1)
    {
        rfAdapter_worker();
    }
}

PROGRAM(2, AUTOSTART)
{
    tlcd_init();
    // DEBUG("tlcd_init() done\n");
    // tlcd_color_t color = {255, 0, 0};
    // tlcd_defineColor(2, color);
    // tlcd_changeDrawColor(2);/*  */
    // tlcd_drawBox(0, 0, 100, 100, 2);

    gui_worker();
}

static void serialAdapter_calculateChecksum(checksum_t* checksum, void* data, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        *checksum ^= ((uint8_t*)data)[i];
    }
}

static void serialAdapter_calculateFrameChecksum(checksum_t* checksum, frame_t* frame)
{
    serialAdapter_calculateChecksum(checksum, &frame->header, sizeof(frame->header));
    serialAdapter_calculateChecksum(checksum, &frame->innerFrame, frame->header.length);
}

PROGRAM(3, AUTOSTART)
{
    delayMs(4000);
    uint32_t value = 1;
    while (1)
    {
        for (int i = 0; i < 6; i++)
        {
            frame_t frame;
            frame.header.destAddr = 255;
            frame.header.srcAddr = 13 + i;
            frame.header.length = sizeof(command_t) + sizeof(cmd_sensorData_t);

            frame.innerFrame.command = CMD_SENSOR_DATA;
            cmd_sensorData_t cmd;
            cmd.sensor = SENSOR_TMP117;
            cmd.paramType = PARAM_TEMPERATURE_CELSIUS;
            cmd.param.fValue = (13.0);
            cmd.param.fValue *= (value);

            memcpy(frame.innerFrame.payload, &cmd, sizeof(cmd_sensorData_t));

            checksum_t frame_checksum = INITIAL_CHECKSUM_VALUE;
            serialAdapter_calculateFrameChecksum(&frame_checksum, &frame);
            frame.footer.checksum = frame_checksum;

            // printFrame(&frame, "PROGRAM(3, AUTOSTART)");
            // printf("\n\n\n\n");
            serialAdapter_processFrame(&frame);
            printf("value: %d\n", cmd.param.uValue);

            delayMs(2000);
        }
        value += 1;
    }
    while (1);
}

#endif