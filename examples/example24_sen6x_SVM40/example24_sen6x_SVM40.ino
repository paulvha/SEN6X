/*  
 *  version DRAFT / December 2024 / paulvha
 *    
 *  This example will connect to the SEN6x and an SVM40.
 *  
 *  The SVM40 Sensor module is equipped with SGP40 and SH40
 * 
 *  This sketch will display the Mass, VOC, NOC, Temperature and humidity information, if available from 
 *  the SEN6x and also the VOC relative humidity RH and temperature from the SVM40. 
 *  
 *  The Sen6x has an SGP43 (a later version of the SGP40) as well as an SH41 (also later version).
 *  You need to give the SEN6x > 5 min (or longer) before making any judgement
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
 * ////////////////////// SVM40 /////////////////////////
 *  pin view
 *          ------------
 *   ___   |1 2 3 4 5 6 |
 *  ="""========================
 *  
 *  WIRE
 *   SVM40                  UNOR4   
 *  1. VDD     -----------   +5V
 *  2. GND     -----------   GND
 *  3. RX/SDA  -----------   SDA
 *  4. TX/SCL  -----------   SCL
 *  5. SEL     -----------   GND  (select I2C, BEFORE applying power)
 *  6. NC
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
#include "svm40.h" // https://github.com/paulvha/svm40

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
// define communication channel to use for SVM40
/////////////////////////////////////////////////////////////
#define SVM40_COMMS Wire

/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request debug messages */
 ////////////////////////////////////////////////////////////
#define DEBUG_SEN6x 0
#define DEBUG_SVM40 0

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

SEN6x sen6x;
SVM40 svm40;

struct sen6x_values val;
struct sen6x_concentration_values valPM;
struct svm40_values v;

bool header = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example24 (DRAFT): Display SEN6x and SVM40. press <enter> to start");

  Serial.println(F("Trying to connect."));
  
  /////////////////////////// SEN6x //////////////////////
  
  // set library debug level
  sen6x.EnableDebugging(DEBUG_SEN6x);

  WIRE_sen6x.begin();

  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("could not auto-detect SEN6x. set as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    // if incorrect set in the sketch, it may not be able to probe the device 
    sen6x.SetDevice(Device);
  }

  // check for connection
  if (! sen6x.probe()) {
    Serial.println(F("could not probe / connect with SEN6x."));
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

  /////////////////////////// SVM40  //////////////////////
  
  // set driver debug level
  svm40.EnableDebugging(DEBUG_SVM40);

  SVM40_COMMS.begin();
  
  // Initialize SVM40 library
  if (! svm40.begin(&SVM40_COMMS)){
    Serial.println( "could not start SVM40. Freeze");
    while(1);    
  }

  // try to detect SVM40 sensors
  if (! svm40.probe() ) {
    Serial.println( "could not detect SVM40 sensors. Freeze");
    while(1);
  }
  else {
    Serial.println(F("Connected SVM40"));
  }

  // set Temperature celcius
  svm40.SetTempCelsius(true);
  
  if (! sen6x.start()) {
    Serial.println(F("could not start sen6x."));
    while(1);
  }
  else {
    Serial.println(F("SEN6x started."));
  }

  // start measurement
  if (! svm40.start()) {
    Serial.println(F("could not start SVM40. Freeze"));
    while(1);
  }
  else {
    Serial.println(F("SVM40 started"));
  }
}

void loop() {
  
  if (! sen6x.CheckDataReady()) return;
    
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read SEN6x values."));
    return;
  }

  if (sen6x.GetConcentration(&valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read SEN6x PM values."));
    return;
  }

  // SVM40
  if (svm40.GetValues(&v) != ERR_OK){
    Serial.println(F("Error during reading SVM40 values"));
    return;
  }
  
  Display_val();

  // wait x seconds
  delay(2000);
}
  
void Display_val()
{
  if (header) {
    
    if (Device != SEN60)
      Serial.println(F("========================== SEN6x ==================================== ======= SVM40 ==========")); 
    
    Serial.print(F("-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print(F("  Humidity:  Temp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }
    
    // SVM40                                           SEN6x
    Serial.print(F("\tVOC   Humidity: Temp:\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("    [%]      [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
    }

    // SVM40
    Serial.print(F("\tindex\t [%]\t [*C]"));
    
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
  
  // display SVM40 information
  Serial.print(F("\t"));
  Serial.print((float) v.VOC_index,2);
  Serial.print(F("\t"));
  Serial.print((float) v.humidity, 2);
  Serial.print(F("\t"));
  Serial.print((float) v.temperature, 2);

  
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
