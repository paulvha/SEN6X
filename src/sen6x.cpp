/**
 * SEN6x Library file
 *
 * Copyright (c) December 2024, Paul van Haastrecht
 *
 * All rights reserved.
 *
 * The library can communicated with the SEN6x to get and set information. 
 *
 * Development environment specifics:
 * Arduino IDE 1.8.19
 *
 * ================ Disclaimer ===================================
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 **********************************************************************
 * Version 1.0 / December 2024 /paulvha
 * - Initial version
 *
 *********************************************************************
 */

#include "sen6x.h"
#include <stdarg.h>
#include <stdio.h>

#if not defined SMALLFOOTPRINT
/* error descripton */
struct SEN6x_Description SEN6x_ERR_desc[11] =
{
  {SEN6x_ERR_OK, "All good"},
  {SEN6x_ERR_DATALENGTH, "Wrong data length for this command (too much or little data)"},
  {SEN6x_ERR_UNKNOWNCMD, "Unknown command"},
  {SEN6x_ERR_ACCESSRIGHT, "No access right for command"},
  {SEN6x_ERR_PARAMETER, "Illegal command parameter or parameter out of allowed range"},
  {SEN6x_ERR_OUTOFRANGE, "Internal function argument out of range"},
  {SEN6x_ERR_CMDSTATE, "Command not allowed in current state"},
  {SEN6x_ERR_TIMEOUT, "No response received within timeout period"},
  {SEN6x_ERR_PROTOCOL, "Protocol error"},
  {SEN6x_ERR_FIRMWARE, "Not supported on this SEN6x firmware level"},
  {0xff, "Unknown Error"}
};
#endif // SMALLFOOTPRINT

/**
 * @brief constructor and initialize variables
 */
SEN6x::SEN6x(void)
{
  _Send_BUF_Length = 0;
  _Receive_BUF_Length = 0;
  _Debug = 0;
  _started = false;
  _FW_Major = _FW_Minor = 0;
  _device = SEN66;						// default to SEN66
  _deviceDetected = false;
}

/** 
 * @brief  Get or Set SEN6x device.
 */
void SEN6x::SetDevice(SEN6x_device d) {
	_device = d;
	_deviceDetected = false;
}

uint8_t SEN6x::GetDevice(bool *detected) 
{
	return(_device);
	*detected = _deviceDetected;
}

/**
 * @brief : Try to detect device based on device name
 * 
 * @return :
 * true : device detected
 * false: not detected (either NO match or device not found)
 * 
 * In case the device is NOT connected OR it is an SEN60 the GetProductName() 
 * will fail. By testing with probe() we try to read version info. If that
 * works a SEN60 is assumed.
 * 
 * NOT TESTED !!
 */
bool SEN6x::DetectDevice()
{
	char Dtmp[32];
	char needle[5];

	// get the name (all except SEN60)
	if (GetProductName(Dtmp, 32) != SEN6x_ERR_OK) {
	  
	  // opcode to obtain nam is different
	  DebugPrintf("Got no reading from a SEN6x device (MAYBE it is SEN60");
	  
	  _device = SEN60; // try to detect Serial number fro SEN60
	  
	  if (GetSerialNumber(Dtmp, 32) == SEN6x_ERR_OK ){
			// so detected Serial number from SEN60
			return(true);
		}

	  // assume that the device is not connected
	  DebugPrintf("We can connect detect.");

		return(false);
	}
	
	/* apperently the GetProductName() worked. But at the time of writting
	 * I got one of the first SEN66. The product name was empty and as 
	 * such I have not been able to test this.*/
	 
	// not sure name is SEN63 or SEN63C, so only take first 5 characters 
	strncpy(needle,(char*) _Receive_BUF,5);

	if (strcmp(needle,"SEN63") == 0) {_device = SEN63; return(true);}
	else if (strcmp(needle,"SEN65") == 0) {_device = SEN65; return(true);}
	else if (strcmp(needle,"SEN66") == 0) {_device = SEN66; return(true);}
	else if (strcmp(needle,"SEN68") == 0) {_device = SEN68; return(true);}
	
	return(false);
}

/**
 * @brief Enable or disable the printing of sent/response HEX values.
 *
 * @param act : level of debug to set
 *  0 : no debug message
 *  1 : sending and receiving data
 */
void SEN6x::EnableDebugging(uint8_t act) {
  _Debug = act;
}

/**
 * @brief Print debug message if enabled 
 */
void SEN6x::DebugPrintf(const char *pcFmt, ...)
{
  va_list pArgs;
  
  if (_Debug == 0) return;
  
  va_start(pArgs, pcFmt);
  vsprintf(prfbuf, pcFmt, pArgs);
  va_end(pArgs);

  SEN6x_DEBUGSERIAL.print(prfbuf);
}

/**
 * @brief begin communication
 *
 * @param port : I2C communication channel to be used
 *
 * User must have preform the wirePort.begin() in the sketch.
 */
bool SEN6x::begin(TwoWire *wirePort)
{
  _i2cPort = wirePort;            // Grab which port the user wants us to use
  _i2cPort->setClock(100000);     // some boards do not set 100K

  // try detect the device by device name 
  // (NOT ABLE TO TEST, wait for NON-pre-release version..)
  if (DetectDevice()) {
	  _deviceDetected = true;
		return(true);
	}	
	
	return(false);
}

/**
 * @brief check if SEN6x sensor is available (read version information)
 *
 * Return:
 *   true on success else false
 */
bool SEN6x::probe() 
{
  struct sen6x_version v;

  if (GetVersion(&v) == SEN6x_ERR_OK)  return(true);

  return(false);
}

/**
 * @brief Check Firmware level 
 *
 * @param  Major : minimum Major level of firmware
 * @param  Minor : minimum Minor level of firmware
*
 * @return
 *  True if SEN6x has required firmware
 *  False does not have required firmware level.
 *
 *  NOTE : not sure this is needed for Sen6x (YET)
 */
bool SEN6x::FWCheck(uint8_t major, uint8_t minor) 
{
  // do we have the current FW level
  if (_FW_Major == 0) {
      if (! probe()) return (false);
  }

  // if requested level is HIGHER than current
  if (major > _FW_Major || minor > _FW_Minor) return(false);

  return(true);
}

/**
 * @brief Read status register
 *
 * @param  *status
 *  return status as an 'or':
 * STATUS_OK_6x = 0,
 * STATUS_SPEED_ERROR_6x =     0x0001,
 * STATUS_FAN_ERROR_6x =       0x0004,
 * STATUS_GAS_ERROR_6x =       0x0008, 
 * STATUS_RHT_ERROR_6x =       0x0010,
 * STATUS_CO2_2_ERROR_6x =     0x0020,
 * STATUS_CO2_1_ERROR_6x =     0x0040,
 * STATUS_HCHO_ERROR_6x =      0x0080,
 * STATUS_PM_ERROR_6x =        0x0100
 *
 * @return
 *  SEN6x_ERR_OK = ok, no isues found 
 *  SEN6x_ERR_OUTOFRANGE, ERROR status feedback 
 */
uint8_t SEN6x::GetStatusReg(uint16_t *status) 
{
  uint8_t ret;

  *status = STATUS_OK_6x;

  // check for minimum Firmware level ???????
  if(! FWCheck(2,0)) return(SEN6x_ERR_FIRMWARE);

  if (_device == SEN60 ){
		
		I2C_fill_buffer(SEN60_READ_DEVICE_REGISTER);
		ret = I2C_SetPointer_Read(2);
	
		if (ret != SEN6x_ERR_OK) return (ret);
	
		if (_Receive_BUF[1] & 0b00000010) *status |= STATUS_SPEED_ERROR_6x;
		if (_Receive_BUF[1] & 0b00010000) *status |= STATUS_FAN_ERROR_6x;
  }
  	  
  else {
	 	I2C_fill_buffer(SEN6x_RD_CL_DEVICE_REGISTER);
		ret = I2C_SetPointer_Read(4);
		if (ret != SEN6x_ERR_OK) return (ret);
		
		if (_Receive_BUF[1] & 0b00100000) *status |= STATUS_SPEED_ERROR_6x;
		
		if (_Receive_BUF[2] & 0b00000010) *status |= STATUS_CO2_2_ERROR_6x;
		if (_Receive_BUF[2] & 0b00000100) *status |= STATUS_HCHO_ERROR_6x;
		if (_Receive_BUF[2] & 0b00001000) *status |= STATUS_PM_ERROR_6x;
		if (_Receive_BUF[2] & 0b00010000) *status |= STATUS_CO2_1_ERROR_6x;
		  
		if (_Receive_BUF[3] & 0b10000000) *status |= STATUS_GAS_ERROR_6x;
		if (_Receive_BUF[3] & 0b01000000) *status |= STATUS_RHT_ERROR_6x;
		if (_Receive_BUF[3] & 0b00010000) *status |= STATUS_FAN_ERROR_6x;
  }

  if (*status != STATUS_OK_6x) return(SEN6x_ERR_OUTOFRANGE);
  
  return(SEN6x_ERR_OK);
}

bool SEN6x::reset() 
{
  if (_device == SEN60) I2C_fill_buffer(SEN60_RESET);
	else I2C_fill_buffer(SEN6x_RESET);

  if (I2C_SetPointer() != SEN6x_ERR_OK) return(false);
	_started = false;
	
	delay(500); //support for UNOR4 (else it will fail)
	_i2cPort->begin();       // some I2C channels need a reset
	delay(500); //support for UNOR4
	
	return(true);
}

bool SEN6x::start()
{
	if (_started) return(true);
	
	if (_device == SEN60) I2C_fill_buffer(SEN60_START_MEASUREMENT);
	else I2C_fill_buffer(SEN6x_START_MEASUREMENT);

  if (I2C_SetPointer() != SEN6x_ERR_OK) return(false);
  
	_started = true;
	
	delay(1000);            // needs at least 20ms, we give plenty of time ?????
	
	return(true);
}

bool SEN6x::stop()
{
	if (! _started) return(true);
	
	if (_device == SEN60) I2C_fill_buffer(SEN60_STOP_MEASUREMENT);
	else I2C_fill_buffer(SEN6x_STOP_MEASUREMENT);

  if (I2C_SetPointer() != SEN6x_ERR_OK) return(false);

	_started = false;
	
	return(true);
}

bool SEN6x::clean()
{
	if (_started)
  {
		// according to datasheet not during measurement
		if (! stop()){
			DebugPrintf("ERROR: Could not stop measurement\n");
			return(false);
		}
		
		// give some time to stop
		delay(500);
  }

	if (_device == SEN60) I2C_fill_buffer(SEN60_START_FAN_CLEANING);
	else I2C_fill_buffer(SEN6x_START_FAN_CLEANING);

  if (I2C_SetPointer() != SEN6x_ERR_OK) return(false);

	return(true);
}

/**
 * Applies to: SEN63C, SEN65, SEN66, SEN68
 * 
 * Description: This command allows you to use the inbuilt heater 
 * in SHT sensor to reverse creep at high humidity.
 * 
 * This command activates the SHT sensor heater with 200mW for 1s. 
 * The heater is then automatically deactivated again.
 * 
 * Wait at least 20s after this command before starting a measurement 
 * to get coherent temperature values (heating consequence to disappear).
 * 
 * No wait is implemented as this can/must be done in the sketch to enable 
 * checking other values.
 */
bool SEN6x::ActivateSHTHeater()
{
	// should NOT be done during measurement
	
	if (_device == SEN60) return(false);

	I2C_fill_buffer(SEN6x_ACTIVATE_SHT_HEATER);

  if (I2C_SetPointer() != SEN6x_ERR_OK) return(false);

	return(true);
}
	
/**
 * @brief Read version info
 *
 * @param v :
 * 	store version information
 * 
 * @return
 *  SEN6x_ERR_OK = ok
 *  else error
 */
uint8_t SEN6x::GetVersion(struct sen6x_version *v) 
{
  uint8_t ret; 

  memset(v, 0x0, sizeof(struct sen6x_version));

  I2C_fill_buffer(SEN6x_READ_VERSION);

  ret = I2C_SetPointer_Read(8);

  if( ret  == SEN6x_ERR_OK) {
    v->F_major = _Receive_BUF[0];
    v->F_minor = _Receive_BUF[1];
    v->F_debug = _Receive_BUF[2];
    v->H_major = _Receive_BUF[3];
    v->H_minor = _Receive_BUF[4];
    v->P_major = _Receive_BUF[5];
    v->P_minor = _Receive_BUF[6];
    v->L_major = DRIVER_MAJOR_6x;
    v->L_minor = DRIVER_MINOR_6x;
  
    // internal library use
    _FW_Major = v->F_major;
    _FW_Minor = v->F_minor;
  }
  
  return(ret);
}

/**
 * @brief General Read device info
 *
 * @param type:
 *  Product Name  : SEN6x_READ_PRODUCT_NAME
 *  Serial Number : SEN6x_READ_SERIAL_NUMBER
 *
 * @param ser     : buffer to hold the read result
 * @param len     : length of the buffer
 *
 * @return
 *  SEN6x_ERR_OK = ok
 *  else error
 */
uint8_t SEN6x::Get_Device_info(uint16_t type, char *ser, uint8_t len)
{
  uint8_t ret = SEN6x_ERR_OK,i;
  char	s60[] = "SEN60";

  // Serial or name code
  if (type == SEN6x_READ_SERIAL_NUMBER) {
    if (_device == SEN60) I2C_fill_buffer(SEN60_READ_SERIAL_NUMBER);
    else I2C_fill_buffer(type);
  }
  
  else if (type == SEN6x_READ_PRODUCT_NAME) {
    
    // NO command to obtain productname for SEN60
    if(_device == SEN60) {
			for (i = 0; i < len, i < strlen(s60); i++) {
				ser[i] = s60[i];
			}
			return SEN6x_ERR_OK;
    }

    I2C_fill_buffer(type);  
  }  

  else
    ret = SEN6x_ERR_PARAMETER;

  // true = check zero termination
  ret =  I2C_SetPointer_Read(len,true);
  
  if (ret == SEN6x_ERR_OK) {

    // get data
    for (i = 0; i < len ; i++) {
      ser[i] = _Receive_BUF[i];
      if (ser[i] == 0x0) break;
    }
  }

  return(ret);
}

/**
 * Applies to: SEN63C, SEN65, SEN66, SEN68
 * 
 * @brief : This command allows to set custom temperature acceleration 
 * parameters of the RH/T engine. It verwrites the default temperature 
 * acceleration parameters of the RH/T engine with custom values. 
 * This configuration is volatile, i.e. the parameters will be reverted 
 * to their default values after a device reset.
 * 
 * For more details on how to compensate the temperature on the SEN6x platform, 
 * refer to “Temperature Acceleration and Compensation Instructions for SEN6x” [3].
 */ 
uint8_t SEN6x::SetTempAccelMode(sen6x_RHT_comp *table) 
{
	uint8_t ret;
	
	if (_device == SEN60) return(SEN6x_ERR_PARAMETER);
	
  I2C_fill_buffer(SEN6x_SET_TEMP_ACCEL, table);

  ret = I2C_SetPointer();

	return(ret);
}

/**
 * Applies to: SEN65, SEN66, SEN68
 * 
 * @brief : Allows backup of the VOC algorithm state to resume 
 * operation after a power cycle or device reset, skipping initial learning phase. 
 * 
 * By default, the VOC Engine is reset, and the algorithm state is retained if a
 * measurement is stopped and started again. If the VOC algorithm state shall be 
 * reset, a device reset, or a power cycle can be executed.
 * 
 * Gets the current VOC algorithm state. This data can be used to restore the 
 * state with Set VOC Algorithm State command after a short power cycle or device reset.
 * 
 * This command can be used either in measure mode or in idle mode (which will 
 * then return the state at the time when the measurement was stopped). 
 * In measure mode, the state can be read each measure interval to always have 
 * the latest state available, even in case of a sudden power loss.
 */
uint8_t SEN6x::GetVocAlgorithmState(uint8_t *table, uint8_t tablesize) {
  uint8_t ret;
    
  if (_device == SEN60 || _device == SEN63) return (SEN6x_ERR_PARAMETER);
  
  // Check for Voc Algorithm length
  if (tablesize < VOC_ALO_SIZE) return(SEN6x_ERR_PARAMETER);
  
  I2C_fill_buffer(SEN6x_GET_SET_VOC_STATE);

  ret = I2C_SetPointer_Read(VOC_ALO_SIZE);

  // save VOC data
  for (int i = 0; i < VOC_ALO_SIZE; i++) {
    table[i] =_Receive_BUF[i];
  }  

  return(ret);
}

/**
 * ONLY valid for SEN65, SEN66 and SEN68
 * 
 * @brief : Gets the parameters to customize the NOx algorithm. For more 
 * information on what the parameters below do, refer to Sensirion’s 
 * NOx Index for Indoor Air Applications [5].
 */
uint8_t SEN6x::GetNoxAlgorithm(sen6x_xox *nox) {
  
  uint8_t ret;
  
  if (_device == SEN60 || _device == SEN63) return (SEN6x_ERR_PARAMETER);
  
  if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);
  
  I2C_fill_buffer(SEN6x_GET_SET_NOX_TUNING);
  
  ret = I2C_SetPointer_Read(12);

  nox->IndexOffset  = byte_to_int16_t(0) ;
  nox->LearnTimeOffsetHours  = byte_to_int16_t(2) ;
  nox->LearnTimeGainHours  = byte_to_int16_t(4) ;
  nox->GateMaxDurationMin  = byte_to_int16_t(6) ;
  nox->stdInitial  = byte_to_int16_t(8) ;
  nox->GainFactor  = byte_to_int16_t(10) ;

	if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);
  
  return(ret);
}

/**
 * 
 * ONLY valid for SEN65, SEN66 and SEN68
 * 
 * @brief : Gets the parameters to customize the VOC algorithm. For more information on 
 * what the parameters below do, refer to Sensirion’s VOC Index for Indoor Air Applications [4].
 * 
 * This configuration is volatile, i.e. the parameters will be reverted to their default 
 * values after a device reset.
 */
uint8_t SEN6x::GetVocAlgorithm(sen6x_xox *voc) {
  uint8_t ret;
  
  if (_device == SEN60 || _device == SEN63) return (SEN6x_ERR_PARAMETER);
  
  if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);
  
  I2C_fill_buffer(SEN6x_GET_SET_VOC_TUNING);
  
  ret = I2C_SetPointer_Read(12);

  voc->IndexOffset  = byte_to_int16_t(0) ;
  voc->LearnTimeOffsetHours  = byte_to_int16_t(2) ;
  voc->LearnTimeGainHours  = byte_to_int16_t(4) ;
  voc->GateMaxDurationMin  = byte_to_int16_t(6) ;
  voc->stdInitial  = byte_to_int16_t(8) ;
  voc->GainFactor  = byte_to_int16_t(10) ;

	if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);
  
  return(ret);
}

/**
 * 
 * only valid for SEN65, SEN66 and SEN68
 * 
 * @brief Allows setting of the VOC algorithm state to resume operation 
 * after a power cycle or device reset, skipping initial learning phase. 
 * By default, the VOC Engine is reset, and the algorithm state is 
 * retained if a measurement is stopped and started again. If the VOC 
 * algorithm state shall be reset, a device reset, or a power cycle can be executed.
 */
uint8_t SEN6x::SetVocAlgorithmState(uint8_t *table, uint8_t tablesize)
{
  uint8_t ret;
  
  if (_device == SEN60 || _device == SEN63) return (SEN6x_ERR_PARAMETER);
  
  if (tablesize < VOC_ALO_SIZE) return(SEN6x_ERR_PARAMETER);
  
  if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);
  
  I2C_fill_buffer(SEN6x_SET_VOC_ALGO, table);

  ret = I2C_SetPointer();
  
  if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);
	
  return(ret);
}

/**
 * 
 * only valid for SEN65, SEN66 and SEN68
 */ 
uint8_t SEN6x::SetNoxAlgorithm(sen6x_xox *nox)
{
  uint8_t ret;
  
  if (_device == SEN60 || _device == SEN63) return (SEN6x_ERR_PARAMETER);
  
  if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);
  
  // MUST be / strongly advised values (according to datasheet))
  nox->LearnTimeGainHours = 12;
  nox->stdInitial = 50;
  
  // check limits
  if (nox->IndexOffset > 250 || nox->IndexOffset < 1) nox->IndexOffset = 1;
  if (nox->LearnTimeOffsetHours > 1000 || nox->LearnTimeOffsetHours < 1) nox->LearnTimeOffsetHours = 12;
  if (nox->GateMaxDurationMin > 3000 || nox->GateMaxDurationMin < 1) nox->GateMaxDurationMin = 720;
  if (nox->GainFactor > 1000 || nox->GainFactor < 1) nox->GainFactor = 230;
  
  I2C_fill_buffer(SEN6x_SET_NOX_TUNING, nox);
	
  ret = I2C_SetPointer();
    
  if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);

	return(ret);
}

/**
 * only valid for SEN65, SEN66 and SEN68
 * 
 * Description: Sets the parameters to customize the VOC algorithm. It has 
 * no effect if at least one parameter is outside the specified range. 
 * For more information on what the parameters below do, refer to 
 * Sensirion’s VOC Index for Indoor Air Applications [4].
 * 
 * This configuration is volatile, i.e. the parameters will be reverted to 
 * their default values after a device reset
 */

uint8_t SEN6x::SetVocAlgorithm(sen6x_xox *voc)
{
  uint8_t ret;
  
  if (_device == SEN60 || _device == SEN63) return (SEN6x_ERR_PARAMETER);
  
  if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);
	
  // check limits (else default according to datasheet)
  if (voc->IndexOffset > 250 || voc->IndexOffset < 1) voc->IndexOffset = 100;
  if (voc->LearnTimeOffsetHours > 1000 || voc->LearnTimeOffsetHours < 1) voc->LearnTimeOffsetHours = 12;
  if (voc->LearnTimeGainHours > 1000 || voc->LearnTimeGainHours < 1) voc->LearnTimeGainHours = 12;
  if (voc->GateMaxDurationMin > 3000 || voc->GateMaxDurationMin < 1) voc->GateMaxDurationMin = 180;
  if (voc->GateMaxDurationMin > 5000 || voc->GateMaxDurationMin < 10) voc->GateMaxDurationMin = 50;
  if (voc->GainFactor > 1000 || voc->GainFactor < 1) voc->GainFactor = 230;
  
  I2C_fill_buffer(SEN6x_SET_VOC_TUNING, voc);
  
  ret = I2C_SetPointer();
    
  if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);

	return(ret);
}

/**
 * ONLY valid for SEN63, SEN66
 * 
 * Execute the forced recalibration (FRC) of the CO2 signal. 
 * See the datasheet of the SCD4x sensor for details how the forced recalibration shall be used [6].
 * 
 * Note: After power-on wait at least 1000 ms and after stopping a measurement 600 ms before sending 
 * this command. The recalibration procedure will take about 500 ms to complete, during which time no 
 * other functions can be executed.
 */
uint8_t SEN6x::ForceCO2Recal(uint16_t *val)
{
  uint8_t ret;
  
  if (_device != SEN63 && _device != SEN66) return (SEN6x_ERR_PARAMETER);
  
  if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);

	data16 = *val;
	
	// max wait time indicated
	delay(1000);
	
	I2C_fill_buffer(SEN6x_SET_FORCE_C02_CAL);
	
	// wait recalibration time
	delay(1000);
	
	// read result
	ret = I2C_ReadToBuffer(2, false); 
	
	if (ret == SEN6x_ERR_OK) 	*val = byte_to_Uint16_t(0) ;

	if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);

	return(ret);
}

/**
 * ONLY valid for SEN63, SEN66
 * 
 * Gets the status of the CO2 sensor automatic self-calibration (ASC). The CO2 sensor supports
 * automatic self-calibration (ASC) for long-term stability of the CO2 output. 
 * This feature can be enabled or disabled. By default, it is enabled.
 * 
 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
 */
uint8_t SEN6x::SetCo2SelfCalibratrion(bool val)
{
  uint8_t ret;
  
  if (_device != SEN63 && _device != SEN66) return (SEN6x_ERR_PARAMETER);
  
	if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);

	data16 = (uint16_t) val; 
	
	I2C_fill_buffer(SEN6X_SET_SELF_CO2_CAL);
	
	ret = I2C_SetPointer();
	
	if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);

	return(ret);
}

/**
 * ONLY valid for SEN63, SEN66
 * 
 * Gets the status of the CO2 sensor automatic self-calibration (ASC). The CO2 sensor supports
 * automatic self-calibration (ASC) for long-term stability of the CO2 output. 
 * This feature can be enabled or disabled. By default, it is enabled.
 * 
 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
 */
uint8_t SEN6x::GetCo2SelfCalibratrion(bool *val)
{  
  uint8_t ret;
  
  if (_device != SEN63 && _device != SEN66) return (SEN6x_ERR_PARAMETER);
  
	if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);

  I2C_fill_buffer(SEN6x_GET_SET_C02_CAL);

  ret = I2C_SetPointer_Read(2);

  if( ret  == SEN6x_ERR_OK) {
		
		*val = (bool) _Receive_BUF[1];

		if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);
	}
	return(ret);
}

/**
 * Get or set Ambient Pressure
 * Applies to: SEN63C, SEN66
 * 
 * Description: GET
 * Gets the ambient pressure value. The ambient pressure can be used for pressure 
 * compensation in the CO2 sensor.
 * 
 * Valid values are between 700 to 1’200 hPa. The default value is 1013 hPa.
 * 
 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
 */ 
uint8_t SEN6x::GetAmbientPressure(uint16_t *val)
{
  uint8_t ret;
  
  if (_device != SEN63 && _device != SEN66) return(SEN6x_ERR_PARAMETER);

  I2C_fill_buffer(SEN6x_GET_SET_AMBIENT_PRESS);

  ret = I2C_SetPointer_Read(2);

  if (ret == SEN6x_ERR_OK) *val = byte_to_Uint16_t(0);

	return(ret);
}

/**
 * set Ambient Pressure
 * Applies to: SEN63C, SEN66
 * 
 * Sets the ambient pressure value. The ambient pressure can be used for pressure 
 * compensation in the CO2 sensor. Setting an ambient pressure overrides any pressure 
 * compensation based on a previously set sensor altitude. 
 * Use of this command is recommended for applications experiencing significant 
 * ambient pressure changes to ensure CO2 sensor accuracy. 
 * 
 * Valid input values are between 700 to 1’200 hPa. The default value is 1013 hPa.
 * 
 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
 */ 
uint8_t SEN6x::SetAmbientPressure(uint16_t val)
{
	if (_device != SEN63 && _device != SEN66) return(SEN6x_ERR_PARAMETER);
	
	if (val < 700 || val > 1200) return(SEN6x_ERR_PARAMETER);
	
	data16 = val;
	
	I2C_fill_buffer(SEN6X_SET_AMBIENT_PRESSURE);
	
	return(I2C_SetPointer());
}

/**
 * @brief : if sensor was started then stop and remember it was
 * started.
 * 
 * @return :
 * true is Ok, false is error
 */
bool SEN6x::CheckStarted()
{
	_restart = false;
	
	if (_started)
  {
		// according to datasheet not during measurement
    if (! stop()) {
			DebugPrintf("ERROR: Could not stop measurement\n");
			return(false);
		}
 		_restart = true;
  }
  
  return(true);
}

/**
 * @brief if the sensor was stopped during CheckStarted(), it will
 * be restarted now
 * 
 * @return :
 * true is Ok, false is error
 */
bool SEN6x::CheckRestart()
{
	if (_restart){
		
		// according to datasheet not during measurement
		if (! start()) {
			DebugPrintf("ERROR: Could not (re)start measurement\n");
			return(false);
		}
		
		_restart = false;
	}
	
	return(true);
}

/**
 * Applies to: SEN63C, SEN66
 * 
 * Description: get 
 * Gets the current sensor altitude. The sensor altitude 
 * can be used for pressure compensation in the CO2 sensor.
 * 
 * The default sensor altitude value is set to 0 meters above sea level. 
 * Valid input values are between 0 and 3000m.
 * 
 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
 */ 
uint8_t SEN6x::GetAltitude(uint16_t *val)
{
  uint8_t ret;
  
  if (_device != SEN63 && _device != SEN66) return(SEN6x_ERR_PARAMETER);
  
  if (! CheckStarted()) return(SEN6x_ERR_PROTOCOL);

  I2C_fill_buffer(SEN6x_GET_SET_ALTITUDE);

  ret = I2C_SetPointer_Read(2);

  if( ret == SEN6x_ERR_OK) {
		
		*val = byte_to_Uint16_t(0);
	
		if (! CheckRestart()) return(SEN6x_ERR_PROTOCOL);
	}

	return(ret);
}

/**
 * Applies to: SEN63C, SEN66
 * 
 * Description: Sets the current sensor altitude. The sensor altitude can be used 
 *              for pressure compensation in the CO2 sensor. 
 * 
 * The default sensor altitude value is set to 0 meters above sea level. 
 * Valid input values are between 0 and 3000m.
 * 
 * This configuration is volatile, i.e. the parameter will be reverted to its default value after a device reset.
 */  
 
uint8_t SEN6x::SetAltitude(uint16_t val)
{
	if (_device != SEN63 && _device != SEN66) return(SEN6x_ERR_PARAMETER);
	
	if (val > 3000) return (SEN6x_ERR_PARAMETER);
	
	data16 = val;
	
	I2C_fill_buffer(SEN6X_SET_ALTITUDE);
	
	return(I2C_SetPointer());
}

/**
 * Applies to: SEN63C, SEN65, SEN66, SEN68
 * Using the sen5x information for now. (December 2024)
 */ 
uint8_t SEN6x::SetTmpComp(sen6x_tmp_comp *tmp)
{
  uint8_t ret;
	sen6x_tmp_comp t;
	 
  if (_device == SEN60) return(SEN6x_ERR_PARAMETER);

	t.offset = tmp->offset * 200;
	t.slope = tmp->slope * 1000;
	t.slot = tmp->slot;
	t.time = tmp->time;
  
  // check slot (not clear what this is. awaiting on documentation)???
	if (t.slot > 4) t.slot  = 4;
	
  I2C_fill_buffer(SEN6x_SET_TEMP_COMP, &t);
  
  ret = I2C_SetPointer();
  
  return(ret);
}

/**
 * @brief : get error description
 * @param code : error code
 * @param buf  : buffer to store the description
 * @param len  : length of buffer
 */
void SEN6x::GetErrDescription(uint8_t code, char *buf, int len)
{

#if defined SMALLFOOTPRINT
  strncpy(buf, "Error-info not disabled", len);
#else
  int i=0;

  while (SEN6x_ERR_desc[i].code != 0xff) {
      if(SEN6x_ERR_desc[i].code == code) break;
      i++;
  }

  strncpy(buf, SEN6x_ERR_desc[i].desc, len);
#endif // SMALLFOOTPRINT
}

/**
 * @brief : 
 * 	read all values from the sensor and store in structure.
 *  depending on the sensor different values are returned. See datasheet
 * 
 * @param v: pointer to structure to store
 *
 * @return
 *  SEN6x_ERR_OK = ok
 *  else error
 */
uint8_t SEN6x::GetValues(struct sen6x_values *v)
{
  uint8_t ret, len;
  
  memset(v,0x0,sizeof(struct sen6x_values));
  
  // measurement started already?
  if (!_started) {
		if (! start()) {
			DebugPrintf("Could not start\n");
			return(SEN6x_ERR_CMDSTATE);
		}
	}

	if (_device == SEN60 ) {
		I2C_fill_buffer(SEN60_READ_MEASURED_VALUE);
		len = 18;
	}
	else if(_device == SEN63 ) {
		I2C_fill_buffer(SEN63_READ_MEASURED_VALUE);
    len = 14;
  }	
	else if(_device == SEN65 ) {
		I2C_fill_buffer(SEN65_READ_MEASURED_VALUE);
		len = 16;
	}
	else if(_device == SEN66 ) {
		I2C_fill_buffer(SEN66_READ_MEASURED_VALUE);
		len = 18;
	}
	else {
		I2C_fill_buffer(SEN68_READ_MEASURED_VALUE);
		len = 18;
	}
  
  ret = I2C_SetPointer_Read(len);

  if (ret != SEN6x_ERR_OK) return (ret);

  // get data
  v->MassPM1 = (float)((byte_to_Uint16_t(0)) / (float) 10);
  v->MassPM2 = (float)((byte_to_Uint16_t(2)) / (float) 10);
  v->MassPM4 = (float)((byte_to_Uint16_t(4)) / (float) 10);
  v->MassPM10 =(float)((byte_to_Uint16_t(6)) / (float) 10);
  
  if (_device == SEN60 ){
	  v->NumPM0 =  (float)((byte_to_Uint16_t(8)) / (float) 10);
	  v->NumPM1 =  (float)((byte_to_Uint16_t(10)) / (float) 10);
	  v->NumPM2 =  (float)((byte_to_Uint16_t(12)) / (float) 10);
	  v->NumPM4 =  (float)((byte_to_Uint16_t(14)) / (float) 10);
	  v->NumPM10 = (float)((byte_to_Uint16_t(16)) / (float) 10);
	  // v->PartSize =(float)((byte_to_Uint16_t(18)) / (float) 1000);
  }
  
  else if (_device == SEN63) {
    v->Hum =  (float)((byte_to_int16_t(8)) / (float) 100);       // Compensated Ambient Humidity [%RH]
		v->Temp = (float)((byte_to_int16_t(10)) / (float) 200);      // Compensated Ambient Temperature [°C]
		v->CO2 =  byte_to_Uint16_t(12);															 // CO2
  }
  
  else if (_device == SEN65 ){
    v->Hum =  (float)((byte_to_int16_t(8)) / (float) 100);       // Compensated Ambient Humidity [%RH]
		v->Temp = (float)((byte_to_int16_t(10)) / (float) 200);      // Compensated Ambient Temperature [°C]
		v->VOC =  (float)((byte_to_int16_t(12)) / (float) 10);       // VOC Index
		v->NOX =  (float)((byte_to_int16_t(14)) / (float) 10);       // NOx Index
	}
	
  else if (_device == SEN66 ){
    v->Hum =  (float)((byte_to_int16_t(8)) / (float) 100);       // Compensated Ambient Humidity [%RH]
		v->Temp = (float)((byte_to_int16_t(10)) / (float) 200);      // Compensated Ambient Temperature [°C]
		v->VOC =  (float)((byte_to_int16_t(12)) / (float) 10);       // VOC Index
		v->NOX =  (float)((byte_to_int16_t(14)) / (float) 10);       // NOx Index
		v->CO2 =  byte_to_Uint16_t(16);															 // CO2
	}

  else if (_device == SEN68 ){
    v->Hum =  (float)((byte_to_int16_t(8)) / (float) 100);       // Compensated Ambient Humidity [%RH]
		v->Temp = (float)((byte_to_int16_t(10)) / (float) 200);      // Compensated Ambient Temperature [°C]
		v->VOC =  (float)((byte_to_int16_t(12)) / (float) 10);       // VOC Index
		v->NOX =  (float)((byte_to_int16_t(14)) / (float) 10);       // NOx Index
		v->HCHO = (float)((byte_to_Uint16_t(16)) / (float) 10); 		 // HCHO (formaldehyde)
	}

  return(SEN6x_ERR_OK);
}

/**
 * get RAW values
 * 
 * Applies to: SEN63C, SEN65, SEN66, SEN68
 * 
 */
uint8_t SEN6x::GetRawValues(struct sen6x_raw_values *v)
{
  uint8_t ret, len;

  memset(v,0x0,sizeof(struct sen6x_raw_values));

	// not supportedon SEN60
	if (_device == SEN60) return(SEN6x_ERR_PARAMETER);
  
  // measurement started already?
	if (!_started) {
		if (! start()) {
			DebugPrintf("Could not start\n");
			return(SEN6x_ERR_CMDSTATE);
		}
	}
	
	if(_device == SEN63 ) {
		I2C_fill_buffer(SEN63_READ_RAW_VALUE);
    len = 4;
  }	
	else if(_device == SEN65 ) {
		I2C_fill_buffer(SEN65_READ_RAW_VALUE);
		len = 8;
	}
	else if(_device == SEN66 ) {
		I2C_fill_buffer(SEN66_READ_RAW_VALUE);
		len = 10;
	}
	else {
		I2C_fill_buffer(SEN68_READ_RAW_VALUE);
		len = 8;
	}
  
  ret = I2C_SetPointer_Read(len);
  
  if (ret != SEN6x_ERR_OK) return (ret);
	
	if (_device == SEN63) {
    v->Hum =  (float)((byte_to_int16_t(0)) / (float) 100);       // Compensated Ambient Humidity [%RH]
		v->Temp = (float)((byte_to_int16_t(2)) / (float) 200);       // Compensated Ambient Temperature [°C]
  }
  
  else if (_device == SEN65 || _device == SEN68) {
    v->Hum =  (float)((byte_to_int16_t(0)) / (float) 100);       // Compensated Ambient Humidity [%RH]
		v->Temp = (float)((byte_to_int16_t(2)) / (float) 200);       // Compensated Ambient Temperature [°C]
		v->VOC =  byte_to_Uint16_t(4);       												 // VOC Index
		v->NOX =  byte_to_Uint16_t(6);       												 // NOx Index
	}
	
  else if (_device == SEN66 ) {
		v->Hum =  (float)((byte_to_int16_t(0)) / (float) 100);       // Compensated Ambient Humidity [%RH]
		v->Temp = (float)((byte_to_int16_t(2)) / (float) 200);       // Compensated Ambient Temperature [°C]
		v->VOC =  byte_to_Uint16_t(4);       												 // VOC Index
		v->NOX =  byte_to_Uint16_t(6);       												 // NOx Index
		v->CO2 =  byte_to_Uint16_t(8);															 // CO2
	}

  return(SEN6x_ERR_OK);
}

/**
 * read concentration of the sensor ( the PM numbers)
 */
uint8_t SEN6x::GetConcentration(struct sen6x_concentration_values *v)
{
	uint8_t ret, len, offset;
  
  memset(v,0x0,sizeof(struct sen6x_concentration_values));
  
  // measurement started already?
	if (!_started) {
		if (! start()) {
			DebugPrintf("Could not start\n");
			return(SEN6x_ERR_CMDSTATE);
		}
	}
  
  // Number concentration values for SEN60 are included in the output of Read Measured Values SEN60.
  if (_device == SEN60 ) {
		I2C_fill_buffer(SEN60_READ_MEASURED_VALUE);
		len = 18;
		offset = 8;
	}
	else {
		I2C_fill_buffer(SEN6x_NUM_CONC_VALUES);
    len = 10;
    offset = 0;
  }	
	  
  ret = I2C_SetPointer_Read(len);

  if (ret == SEN6x_ERR_OK) {
		v->NumPM0 =  (float)((byte_to_Uint16_t(offset)) / (float) 10);
		v->NumPM1 =  (float)((byte_to_Uint16_t(offset + 2)) / (float) 10);
		v->NumPM2 =  (float)((byte_to_Uint16_t(offset + 4)) / (float) 10);
		v->NumPM4 =  (float)((byte_to_Uint16_t(offset + 6)) / (float) 10);
		v->NumPM10 = (float)((byte_to_Uint16_t(offset + 8)) / (float) 10);
	}
	
  return (ret);
}

////////////////// convert routines ///////////////////////////////
/**
 * @brief : translate 4 bytes to Uint32
 * @param x : offset in _Receive_BUF
 *
 * Used for Get Auto Clean interval
 * return : Uint32 number
 */
uint32_t SEN6x::byte_to_U32(int x)
{
  ByteToU32 conv;

  for (byte i = 0; i < 4; i++){
    conv.array[3-i] = _Receive_BUF[x+i]; //or conv.array[i] = _Receive_BUF[x+i]; depending on endianness
  }

  return conv.value;
}

/**
 * @brief : translate 2 bytes to uint16_t
 * @param x : offset in _Receive_BUF
 *
 * return : uint16_t number
 */
uint16_t SEN6x::byte_to_Uint16_t(int x)
{
  uint16_t conv;
  
  conv = _Receive_BUF[x] << 8;
  conv |= _Receive_BUF[x+1];
  
  return conv;
}

/**
 * @brief : translate 2 bytes to int16_t
 * @param x : offset in _Receive_BUF
 *
 * return : uint16_t number
 */
int16_t SEN6x::byte_to_int16_t(int x)
{
  int16_t conv;
  
  conv = _Receive_BUF[x] << 8;
  conv |= _Receive_BUF[x+1];
  
  return conv;
}


////////////////// I2C routines ///////////////////////////////
//************************************************************/

/**
 * @brief : start I2C communication from library
 */
void SEN6x::I2C_init()
{
  _i2cPort->begin();
  _i2cPort->setClock(100000);
}

/**
 * @brief : Fill buffer to send over I2C communication
 * @param cmd: I2C commmand
 * @param interval (optional): value to set for a set-command
 *
 */
void SEN6x::I2C_fill_buffer(uint16_t cmd, void *val)
{
  memset(_Send_BUF,0x0,sizeof(_Send_BUF));
  _Send_BUF_Length = 0;
  
  int i = 0;
  
  uint8_t * vv = (uint8_t *) val;
  struct sen6x_xox * n = (sen6x_xox *) val;
  struct sen6x_tmp_comp * t = (sen6x_tmp_comp *) val;
	struct sen6x_RHT_comp * ta = (sen6x_RHT_comp *) val;
	
  switch(cmd) {
   
    case SEN6x_SET_VOC_ALGO:
 
      _Send_BUF[i++] = SEN6x_GET_SET_VOC_STATE >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_GET_SET_VOC_STATE & 0xff;        //1 LSB
      
      for (int j = 0, k = 0 ; j < VOC_ALO_SIZE;) {
        _Send_BUF[i++] = vv[j++] & 0xff;

        if (k++ == 1) {
          uint8_t st = i - 2;
          _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[st]); //CRC
          k = 0;
        }
      }
      break;
      
    case SEN6x_SET_NOX_TUNING:
      _Send_BUF[i++] = SEN6x_GET_SET_NOX_TUNING >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_GET_SET_NOX_TUNING & 0xff;        //1 LSB    
    
      // add data
      _Send_BUF[i++] = n->IndexOffset >>8 & 0xff;          //2 MSB
      _Send_BUF[i++] = n->IndexOffset & 0xff;              //3 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC
      _Send_BUF[i++] = n->LearnTimeOffsetHours >>8 & 0xff; //5 MSB
      _Send_BUF[i++] = n->LearnTimeOffsetHours & 0xff;     //6 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[5]); //7 CRC
      _Send_BUF[i++] = n->LearnTimeGainHours >>8 & 0xff;   //8 MSB
      _Send_BUF[i++] = n->LearnTimeGainHours & 0xff;       //9 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[8]); //10 CRC
      _Send_BUF[i++] = n->GateMaxDurationMin >>8 & 0xff;   //11 MSB
      _Send_BUF[i++] = n->GateMaxDurationMin & 0xff;       //12 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[11]); //13 CRC
      _Send_BUF[i++] = n->stdInitial >>8 & 0xff;           //14 MSB
      _Send_BUF[i++] = n->stdInitial & 0xff;               //15 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[14]); //16 CRC
      _Send_BUF[i++] = n->GainFactor >>8 & 0xff;           //17 MSB
      _Send_BUF[i++] = n->GainFactor & 0xff;               //18 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[17]); //19 CRC
      break;

    case SEN6x_SET_VOC_TUNING:
      _Send_BUF[i++] = SEN6x_GET_SET_VOC_TUNING >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_GET_SET_VOC_TUNING & 0xff;        //1 LSB    
    
      // add data
      _Send_BUF[i++] = n->IndexOffset >>8 & 0xff;          //2 MSB
      _Send_BUF[i++] = n->IndexOffset & 0xff;              //3 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC
      _Send_BUF[i++] = n->LearnTimeOffsetHours >>8 & 0xff; //5 MSB
      _Send_BUF[i++] = n->LearnTimeOffsetHours & 0xff;     //6 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[5]); //7 CRC
      _Send_BUF[i++] = n->LearnTimeGainHours >>8 & 0xff;   //8 MSB
      _Send_BUF[i++] = n->LearnTimeGainHours & 0xff;       //9 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[8]); //10 CRC
      _Send_BUF[i++] = n->GateMaxDurationMin >>8 & 0xff;   //11 MSB
      _Send_BUF[i++] = n->GateMaxDurationMin & 0xff;       //12 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[11]); //13 CRC
      _Send_BUF[i++] = n->stdInitial >>8 & 0xff;           //14 MSB
      _Send_BUF[i++] = n->stdInitial & 0xff;               //15 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[14]); //16 CRC
      _Send_BUF[i++] = n->GainFactor >>8 & 0xff;           //17 MSB
      _Send_BUF[i++] = n->GainFactor & 0xff;               //18 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[17]); //19 CRC
      break;      
      
    case SEN6x_SET_TEMP_COMP:
      _Send_BUF[i++] = SEN6x_TEMP_OFFSET >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_TEMP_OFFSET & 0xff;        //1 LSB
        
       // add data
      _Send_BUF[i++] = t->offset >>8 & 0xff;          //2 MSB
      _Send_BUF[i++] = t->offset & 0xff;              //3 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC
      _Send_BUF[i++] = t->slope >>8 & 0xff;           //5 MSB
      _Send_BUF[i++] = t->slope & 0xff;               //6 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[5]); //7 CRC
      _Send_BUF[i++] = t->time >>8 & 0xff;            //8 MSB
      _Send_BUF[i++] = t->time & 0xff;                //9 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[8]); //10 CRC
      _Send_BUF[i++] = t->slot >>8 & 0xff;            //11 MSB
      _Send_BUF[i++] = t->slot & 0xff;                //12 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[8]); //13 CRC
      break;
      
		case SEN6x_SET_TEMP_ACCEL:
      _Send_BUF[i++] = SEN6x_TEMP_ACC_PARAM >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_TEMP_ACC_PARAM & 0xff;        //1 LSB
      
       // add data
      _Send_BUF[i++] = ta->K >>8 & 0xff;          //2 MSB
      _Send_BUF[i++] = ta->K & 0xff;              //3 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC
      _Send_BUF[i++] = ta->P >>8 & 0xff;           //5 MSB
      _Send_BUF[i++] = ta->P & 0xff;               //6 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[5]); //7 CRC
      _Send_BUF[i++] = ta->T1 >>8 & 0xff;            //8 MSB
      _Send_BUF[i++] = ta->T1 & 0xff;                //9 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[8]); //10 CRC
      _Send_BUF[i++] = ta->T2 >>8 & 0xff;            //11 MSB
      _Send_BUF[i++] = ta->T2 & 0xff;                //12 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[8]); //13 CRC
      break;
      
     case SEN6x_SET_FORCE_C02_CAL:
      _Send_BUF[i++] = SEN6x_FORCE_C02_CAL >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_FORCE_C02_CAL & 0xff;        //1 LSB
        
       // add data
      _Send_BUF[i++] = data16 >>8 & 0xff;          //2 MSB
      _Send_BUF[i++] = data16 & 0xff;              //3 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC 
      break;
      
     case SEN6X_SET_SELF_CO2_CAL:
      _Send_BUF[i++] = SEN6x_GET_SET_C02_CAL >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_GET_SET_C02_CAL & 0xff;        //1 LSB
        
       // add data
      _Send_BUF[i++] = 0x0;                        //2 MSB
      _Send_BUF[i++] = data16 & 0xff;              //3 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC 
      break;
      
    case SEN6X_SET_AMBIENT_PRESSURE: 
      _Send_BUF[i++] = SEN6x_GET_SET_AMBIENT_PRESS >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_GET_SET_AMBIENT_PRESS & 0xff;        //1 LSB
        
       // add data
      _Send_BUF[i++] = data16 >>8 & 0xff;          //2 MSB
      _Send_BUF[i++] = data16 & 0xff;              //3 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC 
      break;
      
    case SEN6X_SET_ALTITUDE: 
      _Send_BUF[i++] = SEN6x_GET_SET_ALTITUDE >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = SEN6x_GET_SET_ALTITUDE & 0xff;        //1 LSB
        
       // add data
      _Send_BUF[i++] = data16 >>8 & 0xff;          //2 MSB
      _Send_BUF[i++] = data16 & 0xff;              //3 LSB
      _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC 
      break;
    
    default:
      // add command
      _Send_BUF[i++] = cmd >> 8 & 0xff;   //0 MSB
      _Send_BUF[i++] = cmd & 0xff;        //1 LSB
      break;
  }

 _Send_BUF_Length = i;
}

/**
 * @brief : SetPointer (and write data if included) with I2C communication
 *
 * @return :
 * Ok SEN6x_ERR_OK
 * else error
 */
uint8_t SEN6x::I2C_SetPointer()
{ 
  if (_Send_BUF_Length == 0) return(SEN6x_ERR_DATALENGTH);

	if(_device == SEN60) _I2CAddress = SEN60_I2CAddress;
	else _I2CAddress = SEN6x_I2CAddress;

  if (_Debug) {
    DebugPrintf("I2C address: 0x%02X\n",_I2CAddress);
    DebugPrintf("I2C Sending: ");
    for(byte i = 0; i < _Send_BUF_Length; i++)
      DebugPrintf(" 0x%02X", _Send_BUF[i]);
    DebugPrintf("\n");
  }
	
	if(_device == SEN60) _i2cPort->beginTransmission(_I2CAddress);
	else _i2cPort->beginTransmission(_I2CAddress);
	
  _i2cPort->write(_Send_BUF, _Send_BUF_Length);
  _i2cPort->endTransmission();

  return(SEN6x_ERR_OK);
}

/**
 * @brief : read with I2C communication
 * 
 * @param cnt: number of data bytes to get
 * 
 * @param chk_zero : needed for read info buffer
 *  false : expect all the bytes
 *  true  : expect NULL termination and cnt is MAXIMUM byte
 * 
 * @return :
 * OK   SEN6x_ERR_OK
 * else error
 */
uint8_t SEN6x::I2C_SetPointer_Read(uint8_t cnt, bool chk_zero)
{
  uint8_t ret;

  // set pointer
  ret = I2C_SetPointer();
  
  if (ret != SEN6x_ERR_OK) {
    DebugPrintf("Can not set pointer\n");
    return(ret);
  }
  
  // Taking enough time for the command to execute on device.
  delay(100);
  
  // read from Sensor
  ret = I2C_ReadToBuffer(cnt, chk_zero);

  if (_Debug) {
    DebugPrintf("I2C Received: ");
    for(byte i = 0; i < _Receive_BUF_Length; i++)
      DebugPrintf("0x%02X ",_Receive_BUF[i]);
    DebugPrintf("length: %d\n\n",_Receive_BUF_Length);
  }

  if (ret != SEN6x_ERR_OK) {
    DebugPrintf("Error during reading from I2C (error): 0x%02X\n", ret);
  }
  
  return(ret);
}

/**
 * @brief       : receive from Sensor with I2C communication
 * @param count : number of data bytes to expect
 * @param chk_zero :  check for zero termination ( Serial and product code)
 *  false : expect and rea all the data bytes
 *  true  : expect NULL termination and count is MAXIMUM data bytes
 *
 * @return :
 * OK   SEN6x_ERR_OK
 * else error
 */
uint8_t SEN6x::I2C_ReadToBuffer(uint8_t count, bool chk_zero)
{
  uint8_t data[3];
  uint8_t i, j, exp_cnt, rec_cnt;

  j = i = _Receive_BUF_Length = 0;
  
  // 2 data bytes  + crc
  exp_cnt = count / 2 * 3;
  
// in case of AVR expect small wire buffer
// this will only impact reading serial number and name
#ifdef SEN6x_MAX_32_TO_EXPECT
  if (exp_cnt > 32) exp_cnt = 32;
#endif

	rec_cnt = _i2cPort->requestFrom(_I2CAddress, exp_cnt);

  if (rec_cnt != exp_cnt ){
    DebugPrintf("Did not receive all bytes: Expected 0x%02X, got 0x%02X\n",exp_cnt & 0xff,rec_cnt & 0xff);
    return(SEN6x_ERR_PROTOCOL);
  }
  
  while (_i2cPort->available()) {  // read all

    data[i++] = _i2cPort->read();
DebugPrintf("data 0x%02X\n", data[i-1]);
    // 2 bytes RH, 1 CRC
    if( i == 3) {

      if (data[2] != I2C_calc_CRC(&data[0])){
        DebugPrintf("I2C CRC error: Expected 0x%02X, calculated 0x%02X\n",data[2] & 0xff,I2C_calc_CRC(&data[0]) & 0xff);
        return(SEN6x_ERR_PROTOCOL);
      }

      _Receive_BUF[_Receive_BUF_Length++] = data[0];
      _Receive_BUF[_Receive_BUF_Length++] = data[1];

      i = 0;

      // check for zero termination (Serial and product code)
      if (chk_zero) {

        if (data[0] == 0 && data[1] == 0) {

          // flush any bytes pending (if NOT clearing rxBuffer)
          // Logged as an issue and expect this could be removed in the future
          while (_i2cPort->available()) _i2cPort->read();
          return(SEN6x_ERR_OK);
        }
      }

      if (_Receive_BUF_Length >= count) break;
    }
  }

  if (i != 0) {
    DebugPrintf("Error: Data counter %d\n",i);
    while (j < i) _Receive_BUF[_Receive_BUF_Length++] = data[j++];
  }

  if (_Receive_BUF_Length == 0) {
    DebugPrintf("Error: Received NO bytes\n");
    return(SEN6x_ERR_PROTOCOL);
  }

  if (_Receive_BUF_Length == count) return(SEN6x_ERR_OK);

  DebugPrintf("Error: Expected bytes : %d, Received bytes %d\n", count,_Receive_BUF_Length);

  return(SEN6x_ERR_DATALENGTH);
}

/**
 * @brief :check for data ready
 *
 * @return
 *  true  if available
 *  false if not
 */
bool SEN6x::Check_data_ready()
{
	if (!_started) {
		
		if(! start()) {
			DebugPrintf("Could not start\n");
			return(false);
		}
		
		// give time to start
		delay(1000);
	}
	
	if (_device == SEN60) I2C_fill_buffer(SEN60_READ_DATA_RDY_FLAG);
	else I2C_fill_buffer(SEN6x_READ_DATA_RDY_FLAG);
    
  if (I2C_SetPointer_Read(2) != SEN6x_ERR_OK) return(false);
  
  if (_Receive_BUF[1] == 1) return(true);
  
  return(false);
}

/**
 * @brief : calculate CRC for I2c comms
 * @param data : 2 databytes to calculate the CRC from
 *
 * Source : datasheet SEN6x
 *
 * return CRC
 */
uint8_t SEN6x::I2C_calc_CRC(uint8_t data[2])
{
  uint8_t crc = 0xFF;
  for(int i = 0; i < 2; i++) {
    crc ^= data[i];
    for(uint8_t bit = 8; bit > 0; --bit) {
      if(crc & 0x80) {
        crc = (crc << 1) ^ 0x31u;
      } else {
        crc = (crc << 1);
      }
    }
  }

  return crc;
}
