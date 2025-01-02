/*  This example will require an SPS30, SEN55 and a SEN6x.
 *  
 *  It will read and display the information measured next to each other so you 
 *  can compare the results.
 *  
 *  Tested on UNO R4
 *  As the SEN6x is 3V3 it is connected to Qwiic I2C/Wire1, the SPS30 serial and the SEN55 to (5v) wire. 
 *  
 *  Version 1.0 / December 2024 / Paulvha
 *  -initial version
 *
 * *  ..........................................................
 *  Successfully tested on UNO R4
 *  
 *  //////////////// SEN66 //////////////////////
 *  
 *  SEN6x Pinout (backview)
 *               
 *  ---------------------
 *  !   | 123456 /      \|
 *  !___|_______/        |
 *  !           \       /|  
 *  !            \     / |
 *  !-------------=====---
 *  
 *  Wire1
 *  SEN6x pin     UNO R4
 *                Qwiic
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA 
 *  4 SCL -------- SCL 
 *  5 NC
 *  6 NC
 *  
 *  //////////////// SEN55 //////////////////////
 *  
 *  SEN55 Pinout (back / sideview)
 *  
 *  ---------------------
 *  ! 1 2 3 4 5 6        |
 *  !___________         |
 *              \        |  
 *               |       |
 *               """""""""
 *  Wire
 *  SEN55 pin     UNO R4
 *  1 VCC -------- 5V
 *  2 GND -------- GND
 *  3 SDA -------- SDA
 *  4 SCL -------- SCL
 *  5 Select ----- GND  (select I2c)
 *  6 NC
 *  
 *  The pull-up resistors to 5V.
 *  
 *  //////////////// SPS30 //////////////////////
 *  
 *  SPS30 Pinout (backview) SERIAL !!
 *  ----------------------
 *  |                    |
 *  |          1 2 3 4 5 |
 *  ----------------------
 *    
 *  SPS30 pin     UNO R4
 *  1 VCC -------- 5V
 *  2 RX  -------- TX
 *  3 TX  -------- RX 
 *  4 Select      (NOT CONNECTED)
 *  5 GND -------- GND
 * 
 *  ================================ Disclaimer ======================================
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  ===================================================================================
 *
 *  NO support, delivered as is, have fun, good luck !!
 *   
 */
 
///////////////////////////////////////////////////////////////
#include "sen55.h"  // https://github.com/paulvha/sen55
#include "sen6x.h"  // https://github.com/paulvha/SEN6X
#include "sps30.h"  // https://github.com/paulvha/sps30

///////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN63C, SEN65, SEN66 or SEN68 */
///////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface for sen6x and sen55         */
/////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1
#define WIRE_sen55 Wire

/////////////////////////////////////////////////////////////
// define serial communication channel to use for SPS30
/////////////////////////////////////////////////////////////
#define SP30_COMMS Serial1

/////////////////////////////////////////////////////////////
/* define Library debug
 * 0 : no messages
 * 1 : request debug messages */
//////////////////////////////////////////////////////////////
#define SEN6x_DEBUG 0
#define SEN55_DEBUG 0
#define SPS30_DEBUG 0

///////////////////////////////////////////////////////////////
/* By default the SEN66 and SEN63 perform CO2 automatic self
 * calibration (ASC). This requires the sensor to be exposed
 * for at least 4 hours during a week to external / outside air.
 * If not do not, the advise is to disable.
 * See SCD4x data sheet, chapter 3.8
 * 
 * true = disable ASC, false keep enabled */
///////////////////////////////////////////////////////////////
#define DISABLE_ASC false

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

// create constructors
SPS30 sps30;
SEN6x sen6x;
SEN55 sen55;

// variables
struct sen6x_values sen6x_val;
struct sen6x_concentration_values sen6x_valPM;
struct sen_values_pm sen55_val;
struct sps_values sps30_val;
bool header = true;
uint8_t dev = Device;            // indicate connected sensor
bool det;

void setup() {
  
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "Example22 (DRAFT): SEN6x, SEN55 and SPS30. Press <enter> to start");

  Serial.println(F("Trying to connect."));

  // set driver debug level
  sen55.EnableDebugging(SEN55_DEBUG);
  sen6x.EnableDebugging(SEN6x_DEBUG);
  sps30.EnableDebugging(SPS30_DEBUG);

  /////////////////////////////// SEN6x /////////////////////////////////////
  WIRE_sen6x.begin();
  
  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("Could not auto-detect SEN6x. set as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // get connected device
  dev = sen6x.GetDevice(&det);
  
  // check for SEN6x connection
  if (! sen6x.probe()) {
    Serial.println(F("Could not probe / connect with SEN6x."));
    while(1);
  }
  else  {
    Serial.println(F("Connected SEN6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("Could not reset SEN6x."));
    while(1);
  }
  
  // CO2 auto calibration
  if ((dev == SEN66 || dev == SEN63) & DISABLE_ASC) {
    if (sen6x.SetCo2SelfCalibratrion(false) == SEN6x_ERR_OK) {
      Serial.println(F("CO2 ASC disabled"));
    }
    else {
     Serial.println(F("Could not disable ASC"));
    }
  }

  /////////////////////////////// SEN55 /////////////////////////////////////
  WIRE_sen55.begin();
  
  // Begin communication channel;
  if (! sen55.begin(&WIRE_sen55)) {
    Serial.println(F("could not initialize communication channel."));
    while(1);
  }

  // check for SEN55 connection
  if (! sen55.probe()) {
    Serial.println(F("could not probe / connect with SEN55."));
    while(1);
  }
  else  {
    Serial.println(F("Connected SEN5x."));
  }

  // reset SEN55
  if (! sen55.reset()) {
    Serial.println(F("could not reset SEN55."));
    while(1);
  }

/////////////////////////////// sps30 /////////////////////////////////////
  SP30_COMMS.begin(115200);

  // Initialize SPS30 library
  if (! sps30.begin(&SP30_COMMS))
  {
    Serial.println(F("Could not set SPS30 serial communication channel. Freeze"));
    while(1);
  }

  // check for SPS30 connection
  if (! sps30.probe()){
    Serial.println(F("Could not probe / connect with SPS30. Freeze"));
    while(1);
  }
  else {
    Serial.println(F("Connected SPS30."));
  }

  // reset SPS30 connection
  if (! sps30.reset()){
    Serial.println(F("Could NOT reset SPS30. Freeze"));
    while(1);
  }
  
  if (sen6x.start()) Serial.println(F("SEN6x measurement started"));
  else { 
    Serial.println(F("could not Start sen6x."));
    while(1);
  }

  if (sen55.start()) Serial.println(F("SEN55 measurement started"));
  else { 
    Serial.println(F("could not Start sen55."));
    while(1);
  }

  // start measurement
  if (sps30.start()) Serial.println(F("SPS30 measurement started"));
  else {
    Serial.println(F("Could NOT start SPS30 measurement. Freeze"));
    while(1);
  }
  
  serialTrigger((char *) "Hit <enter> to start reading.");

  Serial.println(F("Hit <enter> to perform a fan clean."));
}

void loop() {
  
  // Read Sen6x
  if (! sen6x.CheckDataReady()) return;
  
  if (sen6x.GetValues(&sen6x_val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read values."));
    Check_error_6x();
    return;
  }
  
  // also Sen6x PM values
  if (sen6x.GetConcentration(&sen6x_valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read PM values."));
    Check_error_6x();
    return;
  }

  // Read SEN55
  if ( sen55.Check_data_ready() ){
    // start reading
    if (sen55.GetValuesPM(&sen55_val) != SEN55_ERR_OK) {
      Serial.println(F("Could not read SEN55 values."));
      Check_error_55(); 
      return;
    }
  }
  
  // Read SPS30
  if (sps30.GetValues(&sps30_val) != SPS30_ERR_OK) {
    Serial.println(F("Could not read SPS30 values."));
    return;
  }

  Display_data();

  // trigger clean cycle (pressed <enter>)
  if (Serial.available()) {
    flush();
    
    Serial.print(F("Clean cycle in progress"));
    sps30.clean();
    sen6x.clean();
    Clean_sen55();
    Serial.println(F("\nContinue measurement"));
    
    header = true;
  }
  else
    delay(2000);
}

/**
 * start cleaning and wait for ready clean
 */
void Clean_sen55()
{
  uint8_t stat;
  
  if( !sen55.clean()){
    Serial.println(F("Could not start cleaning Sen55"));
    return;
  }
  
  delay(1000);
  
  do {
    if (sen55.GetStatusReg(&stat) == SEN55_ERR_OK){
      Serial.print(".");
      delay(1000);
    }
  
    else {
      Check_error_55(); 
    }

  } while (stat == STATUS_FAN_CLEAN_ACTIVE_55);

  Serial.println();
}

/**
 * check for status errors
 * 
 * return :
 * true : all OK
 * false : status error
 */
bool Check_error_55() {
  uint8_t stat;

  if (sen55.GetStatusReg(&stat) == SEN55_ERR_OK) return(true);

  Serial.println("ERROR !!");
  if (stat & STATUS_SPEED_ERROR_55) {
    Serial.println(F("Fan speed is too high or too low"));
  }
  
  if (stat & STATUS_LASER_ERROR_55) {
    Serial.println(F("Laser is switched on and current is out of range"));
  }

  if (stat & STATUS_FAN_ERROR_55) {
    Serial.println(F("Fan is switched on, but the measured fan speed is 0 RPM"));
  }

  if (stat & STATUS_GAS_ERROR_55) {
    Serial.println(F("Gas sensor error"));
  }
  
  if (stat & STATUS_RHT_ERROR_55) {
    Serial.println(F("Error in internal communication with the RHT sensor"));
  }     

  return(false);
}

/**
 * check for status errors on Sen6x
 * 
 * return :
 * true : all OK
 * false : status error
 */
bool Check_error_6x() {
  uint16_t stat;

  if (sen6x.GetStatusReg(&stat) == SEN6x_ERR_OK) return(true);

  Serial.println("ERROR !!");
  if (stat & STATUS_SPEED_ERROR_6x) {
    Serial.println(F("Fan speed is too high or too low"));
  }

  if (stat & STATUS_FAN_ERROR_6x) {
    Serial.println(F("Fan is switched on, but the measured fan speed is 0 RPM"));
  }

  if (stat & STATUS_GAS_ERROR_6x) {
    Serial.println(F("Gas sensor error"));
  }
  
  if (stat & STATUS_RHT_ERROR_6x) {
    Serial.println(F("Error in internal communication with the RHT sensor"));
  }     

  if (stat & STATUS_CO2_1_ERROR_6x || stat & STATUS_CO2_2_ERROR_6x ) {
    Serial.println(F("Error with CO2 sensor"));
  }   

  if (stat & STATUS_PM_ERROR_6x) {
    Serial.println(F("Error with the PM sensor"));
  }  

  if (stat & STATUS_HCHO_ERROR_6x) {
    Serial.println(F("Error with the HCHO sensor"));
  }  

  return(false);
}

/**
 * Display the SPS30 and the sen6x result next to each other.
 */
void Display_data()
{

  // print header
  if (header) {
    Serial.println(F("\t-------------Mass -----------    ------------- Number --------------   -Average-"));
    Serial.println(F("\t     Concentration [μg/m3]             Concentration [#/cm3]             [μm]"));
    Serial.println(F("\tP1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10\tPartSize\n"));
    header = false;
  }
  Serial.print(F("SEN6x\t"));
  Serial.print(sen6x_val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM10);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM0);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM1);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM2);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM4);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM10);
  Serial.print(F("\t"));
  Serial.print(F("---"));
  Serial.print(F("\n"));

  Serial.print(F("SEN5x\t"));
  Serial.print(sen55_val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(sen55_val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(sen55_val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(sen55_val.MassPM10);
  Serial.print(F("\t"));
  Serial.print(sen55_val.NumPM0);
  Serial.print(F("\t"));
  Serial.print(sen55_val.NumPM1);
  Serial.print(F("\t"));
  Serial.print(sen55_val.NumPM2);
  Serial.print(F("\t"));
  Serial.print(sen55_val.NumPM4);
  Serial.print(F("\t"));
  Serial.print(sen55_val.NumPM10);
  Serial.print(F("\t"));
  Serial.print(sen55_val.PartSize);
  Serial.print(F("\n"));


  Serial.print(F("SPS30\t"));
  Serial.print(sps30_val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(sps30_val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(sps30_val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(sps30_val.MassPM10);
  Serial.print(F("\t"));
  Serial.print(sps30_val.NumPM0);
  Serial.print(F("\t"));
  Serial.print(sps30_val.NumPM1);
  Serial.print(F("\t"));
  Serial.print(sps30_val.NumPM2);
  Serial.print(F("\t"));
  Serial.print(sps30_val.NumPM4);
  Serial.print(F("\t"));
  Serial.print(sps30_val.NumPM10);
  Serial.print(F("\t"));
  Serial.print(sps30_val.PartSize);
  Serial.println(F("\n"));
}

/**
 * flush input
 */
void flush()
{
  do {
    delay(200);
    Serial.read();
  } while(Serial.available());
}

/**
 * serialTrigger prints repeated message, then waits for enter
 * to come in from the serial port.
 */
void serialTrigger(char * mess)
{
  Serial.println();

  while (!Serial.available()) {
    Serial.println(mess);
    delay(2000);
  }
  
  flush();
}
