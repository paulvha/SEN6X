/*  
 *  version DRAFT / January 2025 / paulvha
 *    
 *  This example will connect to the SEN6x and an SVM30.
 *  
 *  The SCD30 is an earlier released device that Measures indoor air quality parameters total VOC (tVOC),
 *  CO2-equivalent (CO2eq), relative humidity RH and temperature T
 *  
 *  This sketch will display the Mass, VOC, NOC, Temperature and humidity information, if available from 
 *  the SEN6x and also the VOC (tVOC), CO2-equivalent (CO2eq), relative humidity RH and temperature T 
 *  from the SVM30. 
 *  
 *  The SVM30 is a Multi-gas, humidity and temperature sensor combo module containing an SGP30 gas sensor
 *  as well as an SHTC1 humidity and temperature sensor.(https://sensirion.com/products/catalog?page=1&perPage=12&series=SVM30) 
 *  
 *  Although the SVM30 is obsolete it has the SGP30 which is often adviced instead of a CCS-811
 *  
 *  == CO2 
 *  CO2 of the SVM30 will show 400 unless something gets near the sensor. A quick breath to the sensor will 
 *  have it react to value of 1386, then 642, 409 and back to 400. A handwave will cause a value of between 
 *  401 - 414. That change is happening within seconds.
 *  The SEN66 reacts slower and takes more time to down. Clearly it is using an average of the past many detection.
 *  
 *  == TVOC
 *  The SVM30 TVOC values move repeatedly between 0 and 10 all the time, where the SEN66 value is 1 and stays 1.
 *  Breathing into the sensors makes the SVM30 react to a TVOC of 213, 236, 112, 97, 60, 46 down repeatedly 
 *  changing values between 0 and 10. 
 *  
 *  The VOC value on the SEN6x is slowly changing between 100 and 103 and on the same breath moves to 
 *  129,163, 208, 231, 237, 231 etc and takes a couple of seconds longer to return 100. Then it 
 *  undershoots in steps to a low of 77 to then slowly climing up to 100. That process takes about 15 seconds. 
 *  Again calculations are being made in the results before sensor readings are returned. 
 *  
 *  
 *  == Temperature & Humidity
 *  Humidity is on the SEN66 is showing 47.14%, where the SVM30 shows 45,42%. 
 *  The external meter is showing 47%
 *  
 *  Temperature on the SEN66 is showing 19.35C, where the SVM30 is showing 21.5C 
 *  The external meter is showing 18.9C
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
 * ////////////////////// SVM30 /////////////////////////
 *  
 *          ---------
 *          |1 2 3 4 |
 *  ===========================
 *  
 *  WIRE
 *   SVM30               UNOR4   
 *  1. SCL  -----------   SCL
 *  2. GND  -----------   GND
 *  3. VCC  -----------   +5V
 *  4. SDA  -----------   SDA
 *  
 *  Pull-up resistors to 5V
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
#include "svm30.h" // https://github.com/paulvha/svm30

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
#define WIRE_SVM30 Wire

/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request debug messages */
 ////////////////////////////////////////////////////////////
#define DEBUG_SEN6x 0
#define DEBUG_SVM30 0

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
SVM30 svm;

struct sen6x_values val;
struct sen6x_concentration_values valPM;
struct svm_values v;

bool header = true;
uint8_t dev = Device;            // indicate connected sensor
bool det;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example23 (DRAFT): Display SEN6x and SVM30. press <enter> to start");

  Serial.println(F("Trying to connect."));
  
  /////////////////////////// SEN6x //////////////////////
  
  // set library debug level
  sen6x.EnableDebugging(DEBUG_SEN6x);

  WIRE_sen6x.begin();

  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("Could not auto-detect SEN6x. Set as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    // if incorrect set in the sketch, it may not be able to probe the device 
    sen6x.SetDevice(Device);
  }

  // get connected device
  dev = sen6x.GetDevice(&det);
  
  // check for connection
  if (! sen6x.probe()) {
    Serial.println(F("Could not probe / connect with SEN6x."));
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

  /////////////////////////// SVM30  //////////////////////
  
  // enable debug messages
  svm.EnableDebugging(DEBUG_SVM30);
  
  WIRE_SVM30.begin();
  
  if (! svm.begin(&WIRE_SVM30) ) {
    Serial.println( "could not start SGP30. Freeze");
    while(1);    
  }

  // try to detect SVM30 sensors
  if (! svm.probe() ) {
    Serial.println( "could not detect SVM30 sensors. Freeze");
    while(1);
  }
  else {
    Serial.println(F("connected SVM30"));
  }

  if (! sen6x.start()) {
    Serial.println(F("could not Start sen6x."));
    while(1);
  }
}

void loop() {
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read SEN6x values."));
    return;
  }

  if (sen6x.GetConcentration(&valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read SEN6x PM values."));
    return;
  }

  // SVM30
  if (! svm.GetValues(&v)){
    Serial.println(F("Error during reading SVM30 values"));
    return;
  }
  
  Display_val();

  // wait x seconds
  KeepTrigger(2);
}

/*
 * @brief : keep triggering SGP30 while waiting
 * 
 * @param del : number of seconds to wait
 *
 * source datasheet SVM30:
 * The on-chip baseline compensation algorithm has been optimized 
 * for 1HZ sampling rate. The sensor shows best performance when 
 * used with this sampling rate.
 * 
 */
void KeepTrigger(uint8_t del)
{
  uint8_t w_seconds = del;
  unsigned long startMillis;
 
  if (w_seconds == 0) w_seconds = 1;
  
  while (w_seconds--)
  {
    startMillis = millis();
    
    if (! svm.TriggerSGP30())
      Serial.println("Error during trigger waiting");
      
    // this gives 1Hz /1000ms (aboutisch)
    while(millis() - startMillis < 1000);
  }
}

  
void Display_val()
{
  if (header) {
    
    if (Device != SEN60)
      Serial.println(F("========================== SEN6x ================================= ============ SVM30 ===============")); 
    
    Serial.print(F("-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print(F("  Humidity:  Temp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }
    
    // SVM30                                           SEN6x
    Serial.print(F("\tTVOC   CO2:   Humidity: Temp:\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("    [%]      [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
    }

    // SVM30
    Serial.print(F("\t[ppb]  [ppm]\t [%]\t [*C]"));
    
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
  
  // display SVM30 information
  Serial.print(F("\t"));
  Serial.print(v.TVOC);
  Serial.print(F("\t"));
  Serial.print(v.CO2eq);
  Serial.print(F("\t"));
  Serial.print((float) v.humidity/1000), 1;
  Serial.print(F("\t"));
  Serial.print((float) v.temperature/1000), 1;

  
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
