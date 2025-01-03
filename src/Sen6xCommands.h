/**
 * This file contains an overview of the commands for the different
 * SEN6x sensors. 
 * 
 * A 0x0000 means the command is not supported for this sensor
 * 
 * Version DRAFT 1.5 / December 2024 /paulvha
 * - updated version
 */
 
#include <sen6x.h>

/**
 * Contains the OpCodes for the commands for each sensor
 *  
 * if the opcode is 0x0000 the command is not supported by the sensor
 * 
 * KEEP IN SYNC WITH Sen6x_Comds_offset !!
 */
uint16_t SEN6xCommandOpCode [5][SEN6x_GET_SET_ALTITUDE +1] =
{
  /** SEN60 **/ 
  {
    0x2152, // SEN6x_START_MEASUREMENT
    0x3f86, // SEN6x_STOP_MEASUREMENT
    0xE4B8, // SEN6x_READ_DATA_RDY_FLAG
    0xEC05, // SEN6x_READ_MEASURED_VALUE
    0x0000, // SEN6x_READ_RAW_VALUE
    0x0000, // SEN6x_NUM_CONC_VALUES
    0x0000, // SEN6x_TEMP_OFFSET
    0x0000, // SEN6x_TEMP_ACC_PARAM
    0x0000, // SEN6x_READ_PRODUCT_NAME
    0x3682, // SEN6x_READ_SERIAL_NUMBER
    0xD100, // SEN6x_READ_VERSION
    0xE00B, // SEN6x_READ_DEVICE_REGISTER
    0x0000, // SEN6x_RD_CL_DEVICE_REGISTER
    0x3F8D, // SEN6x_RESET
    0x3730, // SEN6x_START_FAN_CLEANING
    0x0000, // SEN6x_ACTIVATE_SHT_HEATER
    0x0000, // SEN6x_GET_SET_VOC_TUNING
    0x0000, // SEN6x_GET_SET_VOC_STATE
    0x0000, // SEN6x_GET_SET_NOX_TUNING
    0x0000, // SEN6x_FORCE_C02_CAL
    0x0000, // SEN6x_GET_SET_C02_CAL
    0x0000, // SEN6x_GET_SET_AMBIENT_PRESS
    0x0000  // SEN6x_GET_SET_ALTITUDE
  },
  /** SEN63C **/ 
  {
    0x0021, // SEN6x_START_MEASUREMENT
    0x0104, // SEN6x_STOP_MEASUREMENT
    0x0202, // SEN6x_READ_DATA_RDY_FLAG
    0x0471, // SEN6x_READ_MEASURED_VALUE
    0x0492, // SEN6x_READ_RAW_VALUE
    0x0316, // SEN6x_NUM_CONC_VALUES
    0X60B2, // SEN6x_TEMP_OFFSET
    0X6100, // SEN6x_TEMP_ACC_PARAM
    0xD014, // SEN6x_READ_PRODUCT_NAME
    0xD033, // SEN6x_READ_SERIAL_NUMBER
    0xD100, // SEN6x_READ_VERSION
    0xD206, // SEN6x_READ_DEVICE_REGISTER
    0xD210, // SEN6x_RD_CL_DEVICE_REGISTER
    0xD304, // SEN6x_RESET
    0x5607, // SEN6x_START_FAN_CLEANING
    0X6765, // SEN6x_ACTIVATE_SHT_HEATER
    0X0000, // SEN6x_GET_SET_VOC_TUNING
    0X0000, // SEN6x_GET_SET_VOC_STATE
    0X0000, // SEN6x_GET_SET_NOX_TUNING
    0X6707, // SEN6x_FORCE_C02_CAL
    0X6711, // SEN6x_GET_SET_C02_CAL
    0X6720, // SEN6x_GET_SET_AMBIENT_PRESS
    0X6736 // SEN6x_GET_SET_ALTITUDE
  },
  /** SEN65 **/ 
  {
    0x0021, // SEN6x_START_MEASUREMENT
    0x0104, // SEN6x_STOP_MEASUREMENT
    0x0202, // SEN6x_READ_DATA_RDY_FLAG
    0x0446, // SEN6x_READ_MEASURED_VALUE
    0x0455, // SEN6x_READ_RAW_VALUE
    0x0316, // SEN6x_NUM_CONC_VALUES
    0X60B2, // SEN6x_TEMP_OFFSET
    0X6100, // SEN6x_TEMP_ACC_PARAM
    0xD014, // SEN6x_READ_PRODUCT_NAME
    0xD033, // SEN6x_READ_SERIAL_NUMBER
    0xD100, // SEN6x_READ_VERSION
    0xD206, // SEN6x_READ_DEVICE_REGISTER
    0xD210, // SEN6x_RD_CL_DEVICE_REGISTER
    0xD304, // SEN6x_RESET
    0x5607, // SEN6x_START_FAN_CLEANING
    0X6765, // SEN6x_ACTIVATE_SHT_HEATER
    0X60d0, // SEN6x_GET_SET_VOC_TUNING
    0X6181, // SEN6x_GET_SET_VOC_STATE
    0X60e1, // SEN6x_GET_SET_NOX_TUNING
    0x0000, // SEN6x_FORCE_C02_CAL
    0x0000, // SEN6x_GET_SET_C02_CAL
    0x0000, // SEN6x_GET_SET_AMBIENT_PRESS
    0x0000  // SEN6x_GET_SET_ALTITUDE
  },
  /** SEN66 **/ 
  {
    0x0021, // SEN6x_START_MEASUREMENT
    0x0104, // SEN6x_STOP_MEASUREMENT
    0x0202, // SEN6x_READ_DATA_RDY_FLAG
    0x0300, // SEN6x_READ_MEASURED_VALUE
    0x0405, // SEN6x_READ_RAW_VALUE
    0x0316, // SEN6x_NUM_CONC_VALUES
    0X60B2, // SEN6x_TEMP_OFFSET
    0X6100, // SEN6x_TEMP_ACC_PARAM
    0xD014, // SEN6x_READ_PRODUCT_NAME
    0xD033, // SEN6x_READ_SERIAL_NUMBER
    0xD100, // SEN6x_READ_VERSION
    0xD206, // SEN6x_READ_DEVICE_REGISTER
    0xD210, // SEN6x_RD_CL_DEVICE_REGISTER
    0xD304, // SEN6x_RESET
    0x5607, // SEN6x_START_FAN_CLEANING
    0X6765, // SEN6x_ACTIVATE_SHT_HEATER
    0X60d0, // SEN6x_GET_SET_VOC_TUNING
    0X6181, // SEN6x_GET_SET_VOC_STATE
    0X60e1, // SEN6x_GET_SET_NOX_TUNING
    0X6707, // SEN6x_FORCE_C02_CAL
    0X6711, // SEN6x_GET_SET_C02_CAL
    0X6720, // SEN6x_GET_SET_AMBIENT_PRESS
    0X6736  // SEN6x_GET_SET_ALTITUDE
  },
  /** SEN68 **/ 
  {
    0x0021, // SEN6x_START_MEASUREMENT
    0x0104, // SEN6x_STOP_MEASUREMENT
    0x0202, // SEN6x_READ_DATA_RDY_FLAG
    0x0467, // SEN6x_READ_MEASURED_VALUE
    0x0455, // SEN6x_READ_RAW_VALUE
    0x0316, // SEN6x_NUM_CONC_VALUES
    0X60B2, // SEN6x_TEMP_OFFSET
    0X6100, // SEN6x_TEMP_ACC_PARAM
    0xD014, // SEN6x_READ_PRODUCT_NAME
    0xD033, // SEN6x_READ_SERIAL_NUMBER
    0xD100, // SEN6x_READ_VERSION
    0xD206, // SEN6x_READ_DEVICE_REGISTER
    0xD210, // SEN6x_RD_CL_DEVICE_REGISTER
    0xD304, // SEN6x_RESET
    0x5607, // SEN6x_START_FAN_CLEANING
    0X6765, // SEN6x_ACTIVATE_SHT_HEATER
    0X60d0, // SEN6x_GET_SET_VOC_TUNING
    0X6181, // SEN6x_GET_SET_VOC_STATE
    0X60e1, // SEN6x_GET_SET_NOX_TUNING
    0x0000, // SEN6x_FORCE_C02_CAL
    0x0000, // SEN6x_GET_SET_C02_CAL
    0x0000, // SEN6x_GET_SET_AMBIENT_PRESS
    0x0000  // SEN6x_GET_SET_ALTITUDE
  }
};

// Write helpers
#define SEN6x_SET_VOC_STATE           0x55FF
#define SEN6x_SET_NOX_TUNING          0x55FE
#define SEN6x_SET_TEMP_COMP           0x55FD
#define SEN6x_SET_TEMP_ACCEL          0x55FC
#define SEN6x_SET_VOC_TUNING          0x55FB
#define SEN6x_SET_FORCE_C02_CAL       0x55FA
#define SEN6X_SET_SELF_CO2_CAL        0x55F9
#define SEN6X_SET_AMBIENT_PRESSURE    0x55F8
#define SEN6X_SET_ALTITUDE            0x55F7
