/*  
 *  version 1.0 / February 2025 / paulvha
 *    
 *  This example will connect to the SEN6x and an external SCD41.
 *  
 *  The SEN66 and SEN63 have a build in SCD41. This example will compare the output of an external SCD41
 *  with the SEN66 results. 
 * 
 *  This sketch will display the Mass, VOC, NOC, Temperature and humidity information, if available from 
 *  the SEN6x and also the CO2, relative humidity RH and temperature from the SCD41. 
 *  !!! Be aware the Temperature and Humidity in the SEN66 are taken from an SGP41 and NOT from the SCD41 !!!
 *  
 *  CO2 measurement
 *  ===============
 *  When breathing to the SEN6x and other CO2 sensor, we see a very quick high response and recovery on the many
 *  external sensors, where the SEN66 is taking longer time to increase the reading (not as high as the 
 *  external sensor) and long time to recover. THAT IS NOT THE CASE WITH THE EXTERNAL SCD41.
 *  
 *  The SEN66 and external SCD41 react rising output nearly at the same (slower) speed. the external SCD41 
 *  recovers slower than other sensors, but much faster than the SEN66 (which takes 2 -3 minutes) to back.
 *  It looks to indicate that the slowness in CO2 increase values is mainly driven by the SCD41, where the 
 *  recover time back to "normal" is mainly driven by calculation.
 *  
 *  Temperature and Humidity
 *  ========================
 *  While the SEN66 is taking the Temperature and humidity from the SGP41, this sketch shows that also
 *  from the SCD41. The information from the SEN66 shows a little closer to an external meter. The SCD41  
 *  about 1 degree *C off, with the humidity about 4% to low.
 *  
 *  Tested on UNOR4 
 *   
 *  ..........................................................
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
 *  ////////////////////// SCD41 /////////////////////////
 *  There are different boards. See your own board
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
#include "SparkFun_SCD4x_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD4x

/////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN65, SEN66 or SEN68
*/
 ////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface */
 ////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1

/////////////////////////////////////////////////////////////
// define communication channel to use for SCD41
/////////////////////////////////////////////////////////////
#define SCD41_COMMS Wire

/////////////////////////////////////////////////////////////
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
/* Many of the CO2 sensors need either a forced or automatic calibration. 
 * This SEN6x library has API calls to perform this function, but 
 * not all the SCD41 libraries enable that.
 * 
 * There is lab-test to check by exposing to a known CO2 value. but else
 * you can check your SCD41: expose to "clean" outside air for a longer
 * time and observe the reading. This should be close between 420.
 * If much higher calcualted the correction factor. 
 * e.g. if the reading is 480 - 420 = 60 set SCD4_CORRECTION to -60
 */
///////////////////////////////////////////////////////////////
#define SCD4_CORRECTION 0

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

SEN6x sen6x;
SCD4x mySensor;

struct sen6x_values val;
struct sen6x_concentration_values valPM;

bool header = true;
uint8_t dev = Device;            // indicate connected sensor
bool det;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example31: Display SEN6x and SCD41. press <enter> to start");

  Serial.println(F("Trying to connect."));
  
  /////////////////////////// SEN6x //////////////////////
  
  // set library debug level
  sen6x.EnableDebugging(DEBUG_SEN6x);

  WIRE_sen6x.begin();

  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("Could not auto-detect SEN6x. Assume as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // get connected device
  dev = sen6x.GetDevice(&det);
  
  // check for SEN6x connection
  if (! sen6x.probe()) {
    Serial.println(F("Could not probe / connect with SEN6x. \nDid you define the right sensor in sketch?"));
    while(1);
  }
  else  {
    Serial.println(F("Connected SEN6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("could not reset sen6x."));
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

  /////////////////////////// SCD41  //////////////////////

  SCD41_COMMS.begin();
  
  // Initialize SCD41 library
  if (! mySensor.begin(&SCD41_COMMS)){
    Serial.println( "could not start SCD41. Freeze");
    while(1);    
  }
  else {
    Serial.println(F("Connected SCD41"));
  }
  
  if (! sen6x.start()) {
    Serial.println(F("Could not start sen6x. Freeze."));
    while(1);
  }
  else {
    Serial.println(F("SEN6x started."));
  }

  Serial.println(F("\nPress <ENTER> during measurement to show header again"));
}

void loop() {
  static uint8_t cnt = 0;

  // wait x seconds
  delay(2000);
  
  if (! sen6x.CheckDataReady()) return;
    
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read SEN6x values."));
    return;
  }

  if (sen6x.GetConcentration(&valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read SEN6x PM values."));
    return;
  }

  // SCD41
  if (! mySensor.readMeasurement()){
  
  if (cnt++ > 5) {
      Serial.println(F("Error during reading SCD41 values"));
      cnt = 0;
    }
    return;
  }
  else
    cnt = 0;
  
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
      Serial.println(F("========================== SEN6x ==================================== ======= SCD41 ==========")); 
    
    Serial.print(F("-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print(F("  Humidity:  Temp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }
    
    // SCD41                                           SEN6x
    Serial.print(F("\tCO2  Humidity: Temp:\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("    [%]      [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
    }

    // SCD41
    Serial.print(F("\t[ppm]\t [%]\t [*C]"));
    
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
  
  // display SCD41 information
  Serial.print(F("\t"));
  Serial.print(mySensor.getCO2() + SCD4_CORRECTION);
  Serial.print(F("\t"));
  Serial.print((float) mySensor.getTemperature(), 2);
  Serial.print(F("\t"));
  Serial.print((float) mySensor.getHumidity(), 2);
  
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
