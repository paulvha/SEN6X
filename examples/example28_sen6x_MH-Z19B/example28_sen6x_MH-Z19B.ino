/*  
 *  version 1.0 / February 2025 / paulvha
 *    
 *  This example will connect to the SEN6x and an MH-Z19B.
 *  
 *  This sketch will display the Mass, VOC, NOx, Temperature and humidity information, if available from 
 *  the SEN6x and also the CO2 and Temp from the MH-Z19B. 
 *  
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
 * ////////////////////// MH-Z19B /////////////////////////
 *  -----------------
 *  |               |
 *  |               | 
 *  |7 5 6 4 3 2 1  |
 *  -----------------
 *  
 *  MH-Z19B             UNOR4
 *  1 Analog out
 *  2 NC
 *  3 GND  ------------- GND
 *  4 Vin  ------------- 5V
 *  5 TX   ------------- RX1
 *  6 RX   ------------- TX1
 *  7 NC
 *  .........................................................................
 *  There is NO reason why this sketch would not work on other MCU / board.
 *  Be aware to add pull-up resistors to 3V3 as I2C on most boards don't have those
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
#include "MHZ19.h" //Click here to get the library Jonathan Dempsey: http://librarymanager/All#MH-Z19

/////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN65, SEN66 or SEN68 */
////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

////////////////////////////////////////////////////////////
/* define which Wire interface */
////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1

/////////////////////////////////////////////////////////////
// define communication to use for MH-Z19B
/////////////////////////////////////////////////////////////
#define mySerial Serial1

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
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

SEN6x sen6x;
MHZ19 myMHZ19;

struct sen6x_values val;

bool header = true;
uint8_t dev = Device;            // indicate connected sensor
bool det;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example28: Display SEN6x and MH-Z19B. press <enter> to start");

  Serial.println(F("Trying to connect."));

  /////////////////////////// MH-Z19  //////////////////////

  mySerial.begin(9600);
  myMHZ19.begin(mySerial);

  myMHZ19.autoCalibration(false);  // Turn auto calibration OFF (ON autoCalibration())
 
  /////////////////////////// SEN6x //////////////////////
  
  // set library debug level
  sen6x.EnableDebugging(DEBUG_SEN6x);

  WIRE_sen6x.begin();

  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("could not auto-detect SEN6x. Assume as defined in sketch."));
    
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
    while(1);
  }
  else  {
    Serial.println(F("Connected SEN6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("Could not reset sen6x. Freeze."));
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

  if (! sen6x.start()) {
    Serial.println(F("Could not start sen6x."));
    while(1);
  }
  else {
    Serial.println(F("SEN6x started."));
  }
}

void loop() {
  
  if (! sen6x.CheckDataReady())  return;
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read SEN6x values."));
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
      Serial.println(F("========================== SEN6x ==================================== ==== MH-Z19B ====")); 
    
    Serial.print(F("-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print(F("  Humidity:  Temp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }
    
    // MH-Z19B                           SEN6x
    Serial.print(F("\tCO2:\t Temp:\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("    [%]      [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
    }

    // MH-Z19B
    Serial.print(F("\t[ppm]\t *C"));
    
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

  // display MH-Z19B information
  int CO2 = myMHZ19.getCO2();         // Request CO2 (as ppm)
  Serial.print("\t");               
  Serial.print(CO2);                                

  float Temp;
  Temp = myMHZ19.getTemperature();    // Request Temperature (as Celsius)
  Serial.print("\t");                  
  Serial.println(Temp);  
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
