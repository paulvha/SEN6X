/*  
 *  version 1.0 / February 2025 / paulvha
 *    
 *  This example will connect to the SEN6x and an SGP30.
 *  
 *  This sketch will display the Mass, VOC, NOx, Temperature and humidity information, if available from 
 *  the SEN6x and also the VOC (tVOC), CO2-equivalent (CO2eq), 
 *  
 *  The SGP30 which is often adviced instead of a CCS-811
 *  
 *  == CO2 
 *  CO2 of the SGP30 will show 400 unless something gets near the sensor. A quick breath to the sensor will 
 *  have it react to value of 1386, then 642, 409 and back to 400. A handwave will cause a value of between 
 *  401 - 414. That change is happening within seconds.
 *  The SEN66 reacts slower and takes more time to down. Clearly it is using an average / calculation of 
 *  the past many detection.
 *  
 *  
 *  Tested on UNOR4 
 *   
 * *   ..........................................................
 *  SEN6x Pinout (backview)
 *               
 *  ---------------------
 *  !   | 123456 /      \|
 *  !___|_______/        |
 *  !           \       /|  
 *  !            \     / |
 *  !-------------=====---
 *  .........................................................
 *  
 *  Successfully tested on UNO R4 
 *  Wire1
 *  SEN6X pin     UNOR4
 *                Qwiic
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA 
 *  4 SCL -------- SCL 
 *  5 internal connected to pin 2 
 *  6 internal connected to Pin 1
 *  
 *  The pull-up resistors are already installed on the UNOR4 for Wire1.
 *  
 * ////////////////////// SGP30 /////////////////////////
 *  
 *          ---------
 *          |1 2 3 4 |
 *  ===========================
 *  
 *  WIRE
 *   SGP30               UNOR4   
 *  1. VCC  -----------   +3.3V
 *  2. GND  -----------   GND
 *  3. SDA  -----------   SDA
 *  4. SCL  -----------   SCL
 *  
 *  Pull-up resistors to 3V3, if not already on the board
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
 */
 
#include "sen6x.h"
#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30

////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN65, SEN66 or SEN68 */
////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface */
 ////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1
#define WIRE_SGP30 Wire

////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request debug messages */
////////////////////////////////////////////////////////////
#define DEBUG_SEN6x 0

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

SEN6x sen6x;
SGP30 mySensor; //create an object of the SGP30 class

struct sen6x_values val;
struct sen6x_concentration_values valPM;
struct sen6x_raw_values valRaw;

bool header = true;
uint8_t dev = Device;            // indicate connected sensor
bool det;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example23: Display SEN6x and SGP30. press <enter> to start");

  Serial.println(F("Trying to connect."));
  
  /////////////////////////// SEN6x //////////////////////
  
  // set library debug level
  sen6x.EnableDebugging(DEBUG_SEN6x);

  WIRE_sen6x.begin();

  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("Could not auto-detect SEN6x. Assume as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    // if incorrect set in the sketch, it may not be able to probe the device 
    sen6x.SetDevice(Device);
  }

  // get connected device
  dev = sen6x.GetDevice(&det);
  
  // check for connection
  if (! sen6x.probe()) {
    Serial.println(F("Could not probe / connect with SEN6x. \nDid you define the right sensor in sketch?"));
    while(1);
  }
  else  {
    Serial.println(F("Connected SEN6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("Could not reset sen6x."));
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

  /////////////////////////// SGP30  //////////////////////
  
  WIRE_SGP30.begin();

  //Initialize sensor
  if (mySensor.begin(WIRE_SGP30) == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  
  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();

  if (! sen6x.start()) {
    Serial.println(F("could not Start SEN6x."));
    while(1);
  }
  
  Serial.println(F("\nPress <ENTER> during measurement to show header again"));
}

void loop() {
  
  delay(1000); //Wait 1 second

  if (! sen6x.CheckDataReady()) return;
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read SEN6x values."));
    return;
  }
 
  // SGP30
  if (mySensor.measureAirQuality() != SGP30_SUCCESS){
    Serial.println(F("Error during reading SGP30 values"));
    return;
  }

  Display_val();
    
  // reprint header when <enter> has been pressed
  if (Serial.available()) {
    header= true;
    flush();
  }
}
  
void Display_val()
{
  if (header) {
    
    if (Device != SEN60)
      Serial.println(F("=========================== SEN6x ================================= ====== SGP30 =======")); 
    
    Serial.print(F("-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("   VOC:    NOX:"));
      Serial.print(F(" Humidity:  Temp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }
    
    // SGP30                                           SEN6x
    Serial.print(F("\tTVOC   CO2-eq:\n"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("\t\t\t\tindex  index"));
      Serial.print(F("    [%]      [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
    }

    // SGP30
    Serial.print(F("\t[ppb]  [ppm]"));
    
    Serial.println(F("\nP1.0\tP2.5\tP4.0\tP10\n"));
    header = false;
  }

  Serial.print(val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(val.MassPM10);

  // skip rest if SEN60
  if(Device != SEN60) {

    if (Device != SEN63) {
      Serial.print(F("\t")); 
      Serial.print(val.VOC);
      Serial.print(F("\t"));
      Serial.print(val.NOX);
    }
    
    Serial.print(F("\t"));
    Serial.print(val.Hum);
    Serial.print(F("\t"));
    Serial.print(val.Temp,2);
    Serial.print(F("\t"));
    
    if (Device == SEN66 || Device == SEN63) Serial.print(val.CO2);
    else if (Device == SEN68) Serial.print(val.HCHO,2);
  }
  
  // display SGP30 information
  Serial.print(F("\t"));
  Serial.print(mySensor.TVOC);
  Serial.print(F("\t"));
  Serial.print(mySensor.CO2);
 
  Serial.println();
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
