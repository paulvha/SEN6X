/**
 * SEN6x Library Header file
 *
 * Copyright (c) December 2024, Paul van Haastrecht
 *
 * All rights reserved.
 *
 * Development environment specifics:
 * Arduino IDE 1.8.19 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 * Version DRAFT 1.3 / December 2024 
 * - Initial version by paulvha
 *********************************************************************
*/
#ifndef SEN6x_H
#define SEN6x_H

#include <Arduino.h>                // Needed for Stream

/**
 * library version levels
 */
#define DRIVER_MAJOR_6x 1
#define DRIVER_MINOR_6x 3

/**
 * select default debug serial
 */
#define SEN6x_DEBUGSERIAL Serial

/**
 * If the platform is an ESP32 AND it is planned to connect an SCD30 as well,
 * you have to remove the comments from the line below
 *
 * The standard I2C on an ESP32 does NOT support clock stretching
 * which is needed for the SCD30. You must have SCD30 library downloaded
 * from https://github.com/paulvha/scd30 and included in your sketch
 * (see examples)
 */
//#define SCD30_SEN6x_ESP32 1

#if defined SCD30_SEN6x_ESP32   // in case of use in combination with SCD30
  #include <SoftWire/SoftWire.h>
#else
  #include "Wire.h"            // for I2c
#endif

/**
 * Auto detect that some boards have low memory. (like Uno)
 */

#if defined (__AVR_ATmega328__) || defined(__AVR_ATmega328P__)|| defined(__AVR_ATmega16U4__) || (__AVR_ATmega32U4__)
  #define SMALLFOOTPRINT 1
#endif 

/**
 * An AVR has 32 I2C buffer. For reading values that is enough, but the Serial number and name, both can have 32 characters. As
 * after each 2 characters a CRC-byte is added, the total to read becomes 48. Thus reading will fail.
 * 
 * This check will enable to expect (and read) max 32 bytes in that case.
 * 
 * CAN impact Serialnumber and Name if either is longer than 24 characters
 */
 
#if defined ARDUINO_ARCH_AVR
  #define SEN6x_MAX_32_TO_EXPECT 1
#endif

/* structure to return mass values */
struct sen6x_values {
  float   MassPM1;        // Mass Concentration PM1.0 [μg/m3]     ALL
  float   MassPM2;        // Mass Concentration PM2.5 [μg/m3]     ALL
  float   MassPM4;        // Mass Concentration PM4.0 [μg/m3]     ALL
  float   MassPM10;       // Mass Concentration PM10 [μg/m3]      ALL
  float   NumPM0;         // Number Concentration PM0.5 [#/cm3]	  SEN60
  float   NumPM1;         // Number Concentration PM1.0 [#/cm3]   SEN60
  float   NumPM2;         // Number Concentration PM2.5 [#/cm3]   SEN60
  float   NumPM4;         // Number Concentration PM4.0 [#/cm3]   SEN60
  float   NumPM10;        // Number Concentration PM4.0 [#/cm3]   SEN60
  float   Hum;            // Compensated Ambient Humidity [%RH]   SEN63C SEN65 SEN66 SEN68
  float   Temp;           // Compensated Ambient Temperature [°C] SEN63C SEN65 SEN66 SEN68
  float   VOC;            // VOC Index SEN65 SEN66 SEN68
  float   NOX;            // NOx Index SEN65 SEN66 SEN68
  uint16_t CO2;			  		// CO2 concentration [ppm]  SEN63C SEN66
  float   HCHO;			  		// HCHO concentration [ppb] SEN68
};

struct sen6x_raw_values {
  float    Hum;           // Compensated Ambient Humidity [%RH]   SEN63C SEN65 SEN66 SEN68
  float    Temp;          // Compensated Ambient Temperature [°C] SEN63C SEN65 SEN66 SEN68
  uint16_t VOC;           // VOC Index SEN65 SEN66 SEN68
  uint16_t NOX;           // NOx Index SEN65 SEN66 SEN68
  uint16_t CO2;			  		// CO2 concentration [ppm] SEN66
};

struct sen6x_concentration_values {
  float   NumPM0;         // Number Concentration PM0.5 [#/cm3]	  ALL
  float   NumPM1;         // Number Concentration PM1.0 [#/cm3]	  ALL
  float   NumPM2;         // Number Concentration PM2.5 [#/cm3]	  ALL
  float   NumPM4;         // Number Concentration PM4.0 [#/cm3]	  ALL
  float   NumPM10;        // Number Concentration PM4.0 [#/cm3]	  ALL
};

/**
 * Obtain different version levels
 */
struct sen6x_version {
  uint8_t F_major;        // Firmware level SEN6x
  uint8_t F_minor;
  bool F_debug;           // Firmware in debug state   (NOT DOCUMENTED)
  uint8_t H_major;        // Hardware level SEN6x      (NOT DOCUMENTED)
  uint8_t H_minor;
  uint8_t P_major;        // protocol level SEN6x      (NOT DOCUMENTED)
  uint8_t P_minor;
  uint8_t L_major;        // library level
  uint8_t L_minor;
};

/**
 * Use to read / write the VOC and Nox Algorithm values
 * 
 * More details on the tuning instructions are provided in 
 * the application note “Engineering Guidelines for SEN5x" ?????
 * (see extra-folder in this library)
 */
struct sen6x_xox {
  /** NOx index representing typical (average) conditions. Allowed
   * values are in range 1..250. The default value is 1. */
  int16_t IndexOffset;
  
  /** Time constant to estimate the NOx algorithm offset from the
   * history in hours. Past events will be forgotten after about twice the
   * learning time. Allowed values are in range 1..1000. 
   * The default value is 12 hours */
  int16_t LearnTimeOffsetHours;
  
  /** The time constant to estimate the NOx algorithm gain from the
   * history has no impact for NOx. This parameter is still in place for
   * consistency reasons with the VOC tuning parameters command.
   * =========================================================
   * !! This parameter must always be set to 12 hours.!!!! 
   * =========================================================*/
  int16_t LearnTimeGainHours;
  
  /** Maximum duration of gating in minutes (freeze of estimator during
   * high NOx index signal). Set to zero to disable the gating. Allowed
   * values are in range 0..3000. The default value is 720 minutes.*/
  int16_t GateMaxDurationMin;
  
  /** The initial estimate for standard deviation parameter has no
   * impact for NOx. This parameter is still in place for consistency
   * reasons with the VOC tuning parameters command. 
   * =========================================================
   * !!! This parameter must always be set to 50
   * =========================================================*/
  int16_t stdInitial;
  
  /** Gain factor to amplify or to attenuate the NOx index output.
   * Allowed values are in range 1..1000. The default value is 230.*/
  int16_t GainFactor;
};

/**
 * Temperature compensation
 * More details about the tuning of these parameters are included 
 * in the application note “Temperature Acceleration and
 * Compensation Instructions for SEN5x”.
 */ 
struct sen6x_tmp_comp {
  /**Temperature offset [°C] (default value: 0)
   * scale 200*/
  int16_t offset;
  
  /**Normalized temperature offset slope (default value: 0)
   * scale 1000*/
  int16_t slope;
  
  /** Time constant in seconds (default value: 0) 
   * scale 1
   * Where slope and offset are the values set with this command, 
   * smoothed with the specified time constant. The time constant 
   * is how fast the slope and offset are applied. 
   * After the speciﬁed value in seconds, 63% of the new slope 
   * and offset are applied.*/
  uint16_t time;
  
  /**
   * The temperature offset slot to be modified. Valid values are
   * 0 .. 4. If the value is outside this range, the parameters will
   * not be applied
   */
  uint16_t slot;
};

/**
 * To set custom temperature acceleration parameters of the RH/T engine.  
 * 
 * Use to overwrite the default temperature acceleration parameters 
 * of the RH/T engine with custom values. 
 * 
 * For more details on how to compensate the temperature on the SEN6x platform, 
 * refer to “Temperature Acceleration and Compensation Instructions for SEN6x” [3].
 */
struct sen6x_RHT_comp {
	
	// Filter constant K scaled with factor 10 (K = value / 10)
	uint16_t K;
	
	// Filter constant P scaled with factor 10 (P = value / 10).
	uint16_t P;
	
	// Time constant T1 scaled with factor 10 (T1 [s] = value / 10)
	uint16_t T1;
	
	// Time constant T2 scaled with factor 10 (T2 [s] = value / 10).
	uint16_t T2;	
}; 

#ifndef SMALLFOOTPRINT

  // error description
  struct SEN6x_Description {
    uint8_t code;
    char    desc[80];
  };
#endif

/*************************************************************/

/**
 * Define Sen6x device
 */
enum SEN6x_device {
	SEN60 = 0,
	SEN63 = 1,
	SEN63C = SEN63,
	SEN65 = 2,
	SEN66 = 3,
	SEN68 = 4
};

/**
 * Status register Error result
 */
enum SEN6x_status {
  STATUS_OK_6x = 0,
  STATUS_SPEED_ERROR_6x =             0x0001,
  STATUS_FAN_ERROR_6x =               0x0004,
  STATUS_GAS_ERROR_6x =               0x0008, 
  STATUS_RHT_ERROR_6x =               0x0010,
  STATUS_CO2_2_ERROR_6x =             0x0020,
  STATUS_CO2_1_ERROR_6x =             0x0040,
  STATUS_HCHO_ERROR_6x =              0x0080,
  STATUS_PM_ERROR_6x =     	          0x0100
};

/**
 * Overview of available commands for different
 * SEN6x sensors.
 * 
 * KEEP IN SYNC WITH SEN6xCommandOpCode[][]
 */
enum Sen6x_Comds_offset {
	SEN6x_START_MEASUREMENT = 0,
	SEN6x_STOP_MEASUREMENT,
	SEN6x_READ_DATA_RDY_FLAG,
	SEN6x_READ_MEASURED_VALUE,
	SEN6x_READ_RAW_VALUE,
	SEN6x_NUM_CONC_VALUES,
	SEN6x_TEMP_OFFSET,
	SEN6x_TEMP_ACC_PARAM,
	SEN6x_READ_PRODUCT_NAME,
	SEN6x_READ_SERIAL_NUMBER,
	SEN6x_READ_VERSION,
	SEN6x_READ_DEVICE_REGISTER,
	SEN6x_RD_CL_DEVICE_REGISTER,
	SEN6x_RESET,
	SEN6x_START_FAN_CLEANING,
	SEN6x_ACTIVATE_SHT_HEATER,
	SEN6x_GET_SET_VOC_TUNING,
	SEN6x_GET_SET_VOC_STATE,
	SEN6x_GET_SET_NOX_TUNING,
	SEN6x_FORCE_C02_CAL,
	SEN6x_GET_SET_C02_CAL,
	SEN6x_GET_SET_AMBIENT_PRESS,
	SEN6x_GET_SET_ALTITUDE
	/** expect something for the H2HO in the near future */
};

/**
 * error codes 
 */
#define SEN6x_ERR_OK                  0x00
#define SEN6x_ERR_DATALENGTH          0X01
#define SEN6x_ERR_UNKNOWNCMD          0x02
#define SEN6x_ERR_ACCESSRIGHT         0x03
#define SEN6x_ERR_PARAMETER           0x04
#define SEN6x_ERR_OUTOFRANGE          0x28
#define SEN6x_ERR_CMDSTATE            0x43
#define SEN6x_ERR_TIMEOUT             0x50
#define SEN6x_ERR_PROTOCOL            0x51
#define SEN6x_ERR_FIRMWARE            0x88


// Receive buffer length.
// in case of name / serial number the max is 32 + 16 CRC = 48
#define SEN6x_MAXBUFLENGTH            50

// I2c fixed addresses
#define SEN6x_I2CAddress              0x6B
#define SEN60_I2CAddress              0X6C            

class SEN6x
{
  public:

    SEN6x(void);
    
    /** 
     * @brief : Set device connected to library.
     * 
     * @param valid options
     * SEN60, SEN63, SEN65, SEN66 or SEN68
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     */
    void SetDevice(SEN6x_device d);

		/**
		 * @brief : Get device will return the device used by library.
		 * 
		 * @return :
     * SEN60, SEN63, SEN65, SEN66 or SEN68
     * 
     * detected : 
     *  true  : device was autodetected 
		 *  false : device was set (either hardcoded or by sketch)
		 * 
		 * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
		 */
    uint8_t GetDevice(bool *detected);
   
    /**
    * @brief : Enable or disable the printing of sent/response HEX values.
    *
    * @param act: 
    *  0 : no debug message
    *  1 : sending and receiving data
    * 
    * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
    */
    void EnableDebugging(uint8_t act);

    /**
     * @brief : Begin with assigment I2C communication port 
     *
     * @param port: I2C communication channel to be used
     *
     * User must have performed the wirePort.begin() in the sketch.
     * 
     * @return
     * true : device was correctly autodetected
     * false : device was not detected (either no match or device not found)
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     */
    bool begin(TwoWire *wirePort);

    /**
     * @brief : Perform SEN6x instructions
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     */
    bool probe();	
    bool reset();		
    bool start();
    bool stop();
    bool clean();

    /**
     * @brief : retrieve Error message details
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     */
    void GetErrDescription(uint8_t code, char *buf, int len);

    /**
     * @brief : retrieve device information from the sen6x
     * 
     * @param ser: buffer to hold the read result
     * @param len: length of the buffer (max 32 char)
     * 
     * @return
     *  SEN6x_ERR_OK = ok
     *  else error
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     */
    uint8_t GetSerialNumber(char *ser, uint8_t len);
    uint8_t GetProductName(char *ser, uint8_t len);

    /**
     * @brief : retrieve version information from the SEN6x and library
     * 
     * @return
     *  SEN6x_ERR_OK = ok
     *  else error
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     */
    uint8_t GetVersion(struct sen6x_version *v);

    /** 
     * @brief : Read Device Status from the SEN6x
     *
     * @param  *status
     *  return status as an 'or':
	   * STATUS_OK_6x = 0,
     * STATUS_SPEED_ERROR_6x =     0x0001,
     * STATUS_FAN_ERROR_6x =       0x0004,
     * STATUS_GAS_ERROR_6x =       0x0008, 
     * STATUS_RHT_ERROR_6x =       0x0010,
     * STATUS_CO2_2_ERROR_6x =     0x0020,	// SEN63C, SEN66
     * STATUS_CO2_1_ERROR_6x =     0x0040,	// SEN63C, SEN66
     * STATUS_HCHO_ERROR_6x =      0x0080,  // SEN68
     * STATUS_PM_ERROR_6x =        0x0100
     * 
     * @return
     *  STATUS_OK_6x = ok, no isues found
     *  else SEN6x_ERR_OUTOFRANGE, issues found
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     */
    uint8_t GetStatusReg(uint16_t *status);
		
		/**
		 * @brief : check for data ready
		 *
		 * @return
		 *  true  if available
		 *  false if not
		 * 
		 * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
		 */
    bool Check_data_ready();
    
    /**
     * @brief : retrieve measurement values from SEN6x
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     * 
     * @return
     *  STATUS_OK_6x = ok
     *  else error
     */
    uint8_t GetValues(struct sen6x_values *v);
    
    /**
     * @brief : retrieve PM numbervalues from SEN6x
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     *  
     * @return
     *  STATUS_OK_6x = ok
     *  else error
     * 
     * Applies to: SEN60, SEN63C, SEN65, SEN66, SEN68
     */
		uint8_t GetConcentration(struct sen6x_concentration_values *v);
	
		/**
		 * @brief : retrieve Raw values from SEN6x
		 * 
		 * Applies to: SEN63C, SEN65, SEN66, SEN68
		 */
		uint8_t GetRawValues(struct sen6x_raw_values *v);
    
    /**
     * @brief : save or restore the VOC algorithm 
     * 
     * Applies to: SEN65, SEN66, SEN68
     *
     * @param :
     * table     : to hold the values
     * tablesize : size of table ( must be at least VOC_ALO_SIZE)
     * 
     * @return :
     * SEN6x_ERR_OK : all OK
     * else error
     */ 
    #define VOC_ALO_SIZE 8 
    uint8_t SetVocAlgorithmState(uint8_t *table, uint8_t tablesize);
    uint8_t GetVocAlgorithmState(uint8_t *table, uint8_t tablesize);

    /**
     * @brief : save or restore the Nox algorithm 
     * 
     * Applies to: SEN65, SEN66, SEN68
     * 
     * @param :
     * Nox : structure to hold the values
     * 
     * @return :
     * SEN6x_ERR_OK : all OK
     * else error
     */ 
    uint8_t GetNoxAlgorithm(sen6x_xox *nox);
    uint8_t SetNoxAlgorithm(sen6x_xox *nox);
    
    /**
     * @brief : save or restore the Voc algorithm 
     * 
     * Applies to: SEN65, SEN66, SEN68
     * 
     * @param :
     * voc : structure to hold the values
     * 
     * @return :
     * SEN6x_ERR_OK : all OK
     * else error
     */ 
    uint8_t GetVocAlgorithm(sen6x_xox *voc);
    uint8_t SetVocAlgorithm(sen6x_xox *voc);

    /**
     * @brief : Read and Update the Temperature compensation 
     * 
     * Applies to: SEN63C, SEN65, SEN66, SEN68
     * 
     * @param :
     * tmp : structure to hold the values
     * 
     * @return :
     * SEN6x_ERR_OK : all OK
     * else error
     */ 
    uint8_t SetTmpComp(sen6x_tmp_comp *tmp);

		/**
		 * @brief : This command allows you to use the inbuilt heater 
		 * in SHT sensor to reverse creep at high humidity.
		 * 
		 * This command activates the SHT sensor heater with 200mW for 1s. 
		 * The heater is then automatically deactivated again.
		 * 
		 * Wait at least 20s after this command before starting a measurement 
		 * to get coherent temperature values (heating consequence to disappear).
		 * 
		 * @return :
     * True  : all OK
     * false : error
		 * 
		 * Applies to: SEN63C, SEN65, SEN66, SEN68
		 */
		 bool ActivateSHTHeater();

    /**
     * @brief : This command allows to set custom temperature acceleration 
     * parameters of the RH/T engine. It verwrites the default temperature 
     * acceleration parameters of the RH/T engine with custom values. 
     * This configuration is volatile, i.e. the parameters will be reverted 
     * to their default values after a device reset.
     * 
     * For more details on how to compensate the temperature on the SEN6x platform, 
     * refer to “Temperature Acceleration and Compensation Instructions for SEN6x” [3].
     * 
     * @return :
     * SEN6x_ERR_OK : all OK
     * else error
     * 
     * Applies to: SEN63C, SEN65, SEN66, SEN68
     */ 
    uint8_t SetTempAccelMode(sen6x_RHT_comp *val); 
  
		/**
		 * @brief : Execute the forced recalibration (FRC) of the CO2 signal. 
		 * See the datasheet of the SCD4x sensor for details how the forced recalibration shall be used [6].
		 * 
		 * Note: After power-on wait at least 1000 ms and after stopping a measurement 600 ms before sending 
		 * this command. The recalibration procedure will take about 500 ms to complete, during which time no 
		 * other functions can be executed.
		 * 
		 * 
		 * @return :
     * SEN6x_ERR_OK : all OK
     * else error
     * 
		 * Applies to: SEN63C, SEN66
		 */
		uint8_t ForceCO2Recal(uint16_t *val);

		/**
		 * @brief : Gets the status of the CO2 sensor automatic self-calibration (ASC). The CO2 sensor supports
		 * automatic self-calibration (ASC) for long-term stability of the CO2 output. 
		 * This feature can be enabled or disabled. By default, it is enabled.
		 * 
		 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
		 * 
		 * @return :
     * SEN6x_ERR_OK : all OK
     * else error
     * 
		 * Applies to: SEN63C, SEN66
		 */
		uint8_t GetCo2SelfCalibratrion(bool *val);
		uint8_t SetCo2SelfCalibratrion(bool val);	
	
		/**
		 * @brief : Get or set Ambient Pressure
		 * 
		 * Description: GET
		 * Gets the ambient pressure value. The ambient pressure can be used for pressure 
		 * compensation in the CO2 sensor.
		 * 
		 * Description: SET
		 * Sets the ambient pressure value. The ambient pressure can be used for pressure 
		 * compensation in the CO2 sensor. Setting an ambient pressure overrides any pressure 
		 * compensation based on a previously set sensor altitude. 
		 * Use of this command is recommended for applications experiencing significant 
		 * ambient pressure changes to ensure CO2 sensor accuracy. 
		 * 
		 * Valid input values are between 700 to 1’200 hPa. The default value is 1013 hPa.
		 * 
		 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
		 * 
		 * @return :
     * SEN6x_ERR_OK : all OK
     * else error
		 * 
		 * Applies to: SEN63C, SEN66
		 */ 
		 uint8_t GetAmbientPressure(uint16_t *val);
		 uint8_t SetAmbientPressure(uint16_t val);

		/**
		 *	@brief : Get or set Altitude
		 * 
		 * Description: get 
		 * Gets the current sensor altitude. The sensor altitude 
		 * can be used for pressure compensation in the CO2 sensor.
		 * 
		 * Description: SET
		 * Sets the current sensor altitude. The sensor altitude can be used 
		 * for pressure compensation in the CO2 sensor. 
		 * The default sensor altitude value is set to 0 meters above sea level. 
		 * Valid input values are between 0 and 3000m.
		 * 
		 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
		 * 
		 * @return :
     * SEN6x_ERR_OK : all OK
     * else error
		 * 
		 * Applies to: SEN63C, SEN66
		 */ 
		 uint8_t GetAltitude(uint16_t *val);
		 uint8_t SetAltitude(uint16_t val);

	 
  private:
    /** debug */
    void DebugPrintf(const char *pcFmt, ...);
    int _Debug;                   // program debug level
    char prfbuf[256];
    
    /** shared variables */
    uint8_t _Receive_BUF[SEN6x_MAXBUFLENGTH]; // buffers
    uint8_t _Send_BUF[SEN6x_MAXBUFLENGTH];
    uint8_t _Receive_BUF_Length;
    uint8_t _Send_BUF_Length;
    SEN6x_device _device;
    bool _deviceDetected;								// true : device was automatically detected
    bool _restart;											// whether to restart after executing command
    bool _started;                      // indicate the measurement has started
    uint8_t _FW_Major, _FW_Minor;       // holds sensor firmware level
    
    uint32_t data32;                    // pass data to i2c_fill_buffer
    uint16_t data16;
    uint8_t _I2CAddress;								// which _i2Caddress to use (SEn60 or SEN6x)
    
    /** shared supporting routines */
    bool FWCheck(uint8_t major, uint8_t minor); 
    uint16_t LookupCommand(Sen6x_Comds_offset cmd);

    bool CheckStarted();
    bool CheckRestart();
		bool DetectDevice();
    
    /** translate/transform */
    typedef union {
      byte array[4];
      uint32_t value;
    } ByteToU32;
    
    uint32_t byte_to_U32(int x);
    uint16_t byte_to_Uint16_t(int x);
    int16_t byte_to_int16_t(int x);
    
    /** I2C communication */
    TwoWire *_i2cPort;                  // holds the I2C port
    void I2C_init();
    uint8_t I2C_fill_buffer(uint16_t cmd, void *val = NULL);
    uint8_t I2C_ReadToBuffer(uint8_t count, bool chk_zero);
    uint8_t I2C_SetPointer_Read(uint8_t cnt, bool chk_zero = false);
    uint8_t I2C_SetPointer();
    uint8_t I2C_calc_CRC(uint8_t data[2]);
};
#endif /* SEN6x_H */
