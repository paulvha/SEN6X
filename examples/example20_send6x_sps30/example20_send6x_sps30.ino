/*  This example will require BOTH an SPS30 and a SEN6x.
 *  
 *  It will read both and display the information measured next to each other so you 
 *  can compare the results.
 *   
 *  As the SEN6x is 3V3 we connect to I2C and the SPS30 serial. 
 *  
 *  Version DRAFT / December 2024 / Paulvha
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
 *                QWICC / STEMMA connector
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA 
 *  4 SCL -------- SCL 
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

#include "sen6x.h"  // https://github.com/paulvha/SEN6X
#include "sps30.h"  // https://github.com/paulvha/sps30

///////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN63C, SEN65, SEN66 or SEN68 */
//////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface for sen6x*/
 ////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1

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
#define SPS30_DEBUG 0

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

// create constructors
SPS30 sps30;
SEN6x sen6x;

// variables
struct sen6x_values sen6x_val;
struct sen6x_concentration_values sen6x_valPM;
struct sps_values sps30_val;
bool header = true;

void setup() {
  
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "Example20:(DRAFT) SEN6x and SPS30. Press <enter> to start");

  Serial.println(F("Trying to connect."));

  // set driver debug level
  sen6x.EnableDebugging(SEN6x_DEBUG);
  sps30.EnableDebugging(SPS30_DEBUG);

  /////////////////////////////// SEN6x /////////////////////////////////////
  WIRE_sen6x.begin();
  
  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("could not auto-detect SEN6x. set as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // check for SEN6x connection
  if (! sen6x.probe()) {
    Serial.println(F("could not probe / connect with SEN6x."));
    while(1);
  }
  else  {
    Serial.println(F("Connected SEN6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("could not reset SEN6x."));
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
  else  Serial.println(F("Connected SPS30."));

  // reset SPS30 connection
  if (! sps30.reset()){
    Serial.println(F("Could NOT reset SPS30. Freeze"));
    while(1);
  }
  
  // start measurement
  if (sps30.start()) Serial.println(F("SPS30 measurement started"));
  else {
    Serial.println(F("Could NOT start SPS30 measurement. Freeze"));
    while(1);
  }

  if (sen6x.start()) Serial.println(F("SEN6x measurement started"));
  else { 
    Serial.println(F("could not Start sen6x."));
    while(1);
  }

  serialTrigger((char *) "Hit <enter> to start reading.");
}

void loop() {
  
  if (! sen6x.CheckDataReady()) return;
  
  // start reading Sen6x
  if (sen6x.GetValues(&sen6x_val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read values."));
    Check_error_6x();
    return;
  }
  
  // also Sen6x
  if (sen6x.GetConcentration(&sen6x_valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read PM values."));
    Check_error_6x();
    return;
  }
 

  // get SPS30
  if (sps30.GetValues(&sps30_val) != SPS30_ERR_OK) {
    Serial.println(F("Could not read SPS30 values."));
    return;
  }

  Display_data();

  delay(2000);
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
