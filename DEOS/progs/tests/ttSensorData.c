//-------------------------------------------------
//          TestSuite: Sensor Data
//-------------------------------------------------
// Simulates sensor data and sends it over the
// network. Can be used to check correct parsing on
// a second receiver board.
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_SENSOR_DATA

#include "../../communication/rfAdapter.h"
#include "../../communication/sensorData.h"
#include "../../lib/lcd.h"
#include "../../os_scheduler.h"

#include <stdbool.h>
#include <string.h>

// Change PARTNER_ADDRESS to the device the data should be sent to
#define PARTNER_ADDRESS ADDRESS_BROADCAST

// Simulate sensor values with different value range and increment
#define SENSOR_VALUE_COUNT 4	  // Number of sensor values
#define SIMULATE_INTERVAL_MS 1000 // Interval of altering simulated sensor values

uint8_t lcdSensorDataIdx = 0;

//! Forward declarations
void lcd_writeFloat(float value, uint8_t decimalPlaces, bool forceDecimals);
void printSensorData(sensor_parameter_type_t paramType, sensor_parameter_t param);
void incrementSensorData(sensor_parameter_type_t paramType, sensor_parameter_t *param, sensor_parameter_t min, sensor_parameter_t max, sensor_parameter_t inc);
void rfAdapter_sendSensorData(address_t destAddr, sensor_type_t sensor, sensor_parameter_type_t paramType, sensor_parameter_t param);
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
	sensor_type_t sensor_type[SENSOR_VALUE_COUNT] = {SENSOR_BMP388, SENSOR_ALS_PT19, SENSOR_SCD30, SENSOR_AM2320};												// Sensor type
	sensor_parameter_type_t param_type[SENSOR_VALUE_COUNT] = {PARAM_TEMPERATURE_CELSIUS, PARAM_LIGHT_INTENSITY_PERCENT, PARAM_CO2_PPM, PARAM_HUMIDITY_PERCENT}; // Parameter type
	sensor_parameter_t value_min[SENSOR_VALUE_COUNT] = {{.fValue = -10}, {.fValue = 0}, {.uValue = 1000}, {.uValue = 0}};										// Min value to simulate
	sensor_parameter_t value_max[SENSOR_VALUE_COUNT] = {{.fValue = 35}, {.fValue = 100}, {.uValue = 80000}, {.uValue = 100}};									// Max value to simulate
	sensor_parameter_t value_inc[SENSOR_VALUE_COUNT] = {{.fValue = 0.25}, {.fValue = 1.0 / 3}, {.uValue = 1000}, {.uValue = 5}};								// Increment value while simulating
	sensor_parameter_t value_sim[SENSOR_VALUE_COUNT];																											// Current simulated value

	// Initialize all parameters with their minimum
	for (uint8_t i = 0; i < SENSOR_VALUE_COUNT; i++)
	{
		value_sim[i] = value_min[i];
	}

	// Wait until communication is ready
	while (!rfAdapter_isInitialized())
	{
		os_yield();
	}

	while (1)
	{
		lcd_clear();
		for (uint8_t i = 0; i < SENSOR_VALUE_COUNT; i++)
		{
			// Send
			rfAdapter_sendSensorData(PARTNER_ADDRESS, sensor_type[i], param_type[i], value_sim[i]);

			// Print
			printSensorData(param_type[i], value_sim[i]);

			// Increment
			incrementSensorData(param_type[i], &value_sim[i], value_min[i], value_max[i], value_inc[i]);
		}

		delayMs(SIMULATE_INTERVAL_MS);
	}
}

/*!
 *  Prints the passed float on the display
 *
 *  Note:
 *  sprintf_P(buf, PSTR("%f"), value) could also be used to generate a string,
 *  but the library consumes plenty of Flash and you need to add compiler options
 *  -Wl,-u,vfprintf -lprintf_flt
 *  (see https://startingelectronics.org/articles/atmel-AVR-8-bit/print-float-atmel-studio-7/)
 *
 *  \param value			Float to print
 *  \param decimalPlaces	Max. number of decimal places to print
 *  \param forceDecimals	Fill decimal places with trailing zeros
 */
void lcd_writeFloat(float value, uint8_t decimalPlaces, bool forceDecimals)
{
	os_enterCriticalSection();

	// Print sign and continue with positive value
	if (value < 0)
	{
		lcd_writeChar('-');
		value = -value;
	}

	// 10^decimalPlaces helps to round the float and to print leading zeros for the decimal part
	uint32_t powDec = 1;
	for (uint8_t i = 0; i < decimalPlaces; i++)
	{
		powDec *= 10;
	}

	// Round last decimal place
	value += 0.5 / powDec;

	// Get integer part
	int32_t iValue = value;

	// Print Integer part
	lcd_writeDec(iValue);

	// Get decimal part
	iValue = powDec * (value - iValue);

	// Only print decimal part if there is one or we are forced to
	if (!decimalPlaces || (!forceDecimals && !iValue))
	{
		os_leaveCriticalSection();
		return;
	}
	lcd_writeChar('.');

	// Print decimal part (with leading zeros)
	while (iValue < (powDec /= 10))
	{
		lcd_writeChar('0');
	}
	if (iValue)
	{
		lcd_writeDec(iValue);
	}
	os_leaveCriticalSection();
}

/*!
 * Prints given sensor data on the display
 * Note: You can call this function when processing incoming sensor data (of supported type)
 *       to test correct transmission and parsing
 *
 *  /param paramType Parameter type including unit specification
 *  /param param     Sensor value
 */
void printSensorData(sensor_parameter_type_t paramType, sensor_parameter_t param)
{
	// Position on display
	lcd_goto(lcdSensorDataIdx / 2, (lcdSensorDataIdx % 2) * 9);
	lcdSensorDataIdx = (lcdSensorDataIdx + 1) % 4;

	// Print parameter
	switch (paramType)
	{
	case PARAM_TEMPERATURE_CELSIUS:
		lcd_writeFloat(param.fValue, 2, false);
		// Add a switch case to lcd_writeChar that converts the degree sign to code 0xDF
		lcd_writeProgString(PSTR("°C"));
		break;

	case PARAM_LIGHT_INTENSITY_PERCENT:
		lcd_writeFloat(param.fValue, 2, false);
		lcd_writeProgString(PSTR("%"));
		break;

	case PARAM_CO2_PPM:
		lcd_writeDec(param.uValue);
		lcd_writeProgString(PSTR("ppm"));
		break;

	case PARAM_HUMIDITY_PERCENT:
		lcd_writeDec(param.uValue);
		lcd_writeProgString(PSTR("%"));
		break;

	default:
		lcd_writeChar('?');
		break;
	}
}

/*!
 * Increments the referenced parameter by given increment inc keeping the bounds min/max
 *
 *  /param paramType Parameter type including unit specification
 *  /param param     Sensor value pointer
 *  /param min       Min simulated sensor value
 *  /param max       Max simulated sensor value
 *  /param inc       Increment of sensor value
 */
void incrementSensorData(sensor_parameter_type_t paramType, sensor_parameter_t *param, sensor_parameter_t min, sensor_parameter_t max, sensor_parameter_t inc)
{
	switch (paramType)
	{
	case PARAM_TEMPERATURE_CELSIUS:
	case PARAM_LIGHT_INTENSITY_PERCENT:
		param->fValue += inc.fValue;
		if (param->fValue > max.fValue)
		{
			param->fValue = min.fValue;
		}
		break;

	case PARAM_CO2_PPM:
	case PARAM_HUMIDITY_PERCENT:
		param->uValue += inc.uValue;
		if (param->uValue > max.uValue)
		{
			param->uValue = min.uValue;
		}
		break;

	default:
		break;
	}
}

void rfAdapter_sendSensorData(address_t destAddr, sensor_type_t sensor, sensor_parameter_type_t paramType, sensor_parameter_t param)
{
	inner_frame_t innerFrame;
	innerFrame.command = CMD_SENSOR_DATA;

	cmd_sensorData_t *cmd = (cmd_sensorData_t *)innerFrame.payload;
	cmd->sensor = sensor;
	cmd->paramType = paramType;
	memcpy(&cmd->param, &param, sizeof(sensor_parameter_t));

	serialAdapter_writeFrame(destAddr, sizeof(command_t) + sizeof(cmd_sensorData_t), &innerFrame);
}

#endif