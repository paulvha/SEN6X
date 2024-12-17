/*  
 *  version DRAFT / December 2024 / paulvha
 *    
 *  This example will connect to the SEN6x and an SCD30.
 *  
 *  The SCD30 is an earlier released device that can detect C02, temperature and humidity. (https://sensirion.com/products/catalog/SCD30) 
 *  
 *  This sketch will display the Mass, VOC, NOC, Temperature and humidity information, if available from 
 *  the SEN6x and also the CO2, temperature and humidity information from the SCD30. 
 *  On the SCD30 an SHT31 is used to obtain the RH & T.
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
 * ////////////////////// SCD30 /////////////////////////
 *  --------------------------
 *  | 1 2 3 4 5 6 7      (S) |
 *  |  _________________     |    
 *  | /                 \====|  
 *  | ||||=====         [||  |
 *  | \_________________/====|
 *  |________________________|
 *  
 *  SCD30             UNOR4 
 *  1 VDD  ---------- 5V
 *  2 GND  ---------- GND
 *  3 SCL  ---------- SCL
 *  4 SDA  ---------- SDA
 *  5 RDY NC
 *  6 PWM NC
 *  7 SEL NC or GND (select I2C)
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
 
#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
#include "sen6x.h"

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
#define WIRE_SCD30 Wire

/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request debug messages */
 ////////////////////////////////////////////////////////////
#define DEBUG_SEN6x 0

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

SEN6x sen6x;
SCD30 airSensor;

struct sen6x_values val;
struct sen6x_concentration_values valPM;
bool header = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example4 (DRAFT): Display SEN6x and SCD30. press <enter> to start");

  Serial.println(F("Trying to connect."));

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

  /////////////////////////// SCD30  //////////////////////
  WIRE_SCD30.begin();

  if ( ! airSensor.begin() ) {
    Serial.println("Air sensor SCD30 not detected. Please check wiring. Freezing...");
    while (1);
  }
  else {
    Serial.println("Connected SCD30.");
  }

  //The SCD30 has data ready every two seconds (not using the RDY wire connection)

  if (! sen6x.start()) {
    Serial.println(F("could not Start sen6x."));
    while(1);
  }
}

void loop() {
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read values."));
    return;
  }

  if (sen6x.GetConcentration(&valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read PM values."));
    return;
  }

  Display_val();
  
  delay(2000);
}
  
void Display_val()
{
  if (header) {
    
    if (Device != SEN60)
      Serial.println(F("========================== SEN6x ============================= ========== SCD30 =============")); 
    
    Serial.print(F("-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print(F("  Humidity:  Temp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }
    
    Serial.print(F("\tCO2:   Humidity: Temp:\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("    [%]      [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
    }
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
  
  // display SCD30 information
  if (airSensor.dataAvailable()) {
    Serial.print(F("\t"));
    Serial.print(airSensor.getCO2());
    Serial.print(F("\t"));
    Serial.print(airSensor.getHumidity(), 1);
    Serial.print(F("\t"));
    Serial.print(airSensor.getTemperature(), 1);
  }
  else {
    Serial.print(F("\t--NOT AVAILABLE--"));
  }
  
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
