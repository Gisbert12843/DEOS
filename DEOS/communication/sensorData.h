/*!
 *  \brief Defines sensor types and an uniform interface for all sensors in a network
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */
#ifndef SENSORDATA_H_
#define SENSORDATA_H_

//! Definition of sensor types
typedef enum SensorType
{
	SENSOR_MPL3115A2 = 1,
	SENSOR_AM2320 = 2,
	SENSOR_SCD30 = 3,
	SENSOR_MCP9808 = 4,
	SENSOR_BMP388 = 5,
	SENSOR_LPS331AP = 6,
	SENSOR_ALS_PT19 = 7,
	SENSOR_SGP30 = 8
} sensor_type_t;

//! Definition of sensor values
typedef enum SensorParameterType
{
	PARAM_TEMPERATURE_CELSIUS = 1,
	PARAM_HUMIDITY_PERCENT = 2,
	PARAM_LIGHT_INTENSITY_PERCENT = 3,
	PARAM_ALTITUDE_M = 4,
	PARAM_PRESSURE_PASCAL = 5,
	PARAM_E_CO2_PPM = 6,
	PARAM_TVOC_PPB = 7,
	PARAM_CO2_PPM = 8
} sensor_parameter_type_t;

//! A sensor value can be float, signed or unsigned integer
typedef union SensorParameter
{
	float fValue;
	uint32_t uValue;
	int32_t iValue;
} sensor_parameter_t;

//! Command payload of command CMD_SENSOR_DATA
typedef struct cmd_sensorData
{
	sensor_type_t sensor;
	sensor_parameter_type_t paramType;
	sensor_parameter_t param;
} cmd_sensorData_t;

#endif /* SENSORDATA_H_ */