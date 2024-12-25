/*  
 *  version DRAFT / December 2024 / paulvha
 *    
 *  This example will connect to the SEN6x and an CCS811.
 *  
 *  The CCS811 Sensor module is equipped with SGP40 and SH40
 * 
 *  This sketch will display the Mass, VOC, NOC, Temperature and humidity information, if available from 
 *  the SEN6x and also the VOC and CO2 from the CCS811. 
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
 * ////////////////////// CCS811 /////////////////////////
 *  
 *  -----------------
 *  |               |
 *  |               | 
 *  |1 2 3 4 5 6 7 8|
 *  -----------------
 *  
 *  CCS811              UNOR4
 *  1 VIN  ------------- +5V
 *  2 3v3
 *  3 GND  ------------- GND
 *  4 SDA  ------------- SDA
 *  5 SCL  ------------- SCL
 *  6 WAKE ------------- GND
 *  7 RST
 *  8 INT
 *  
 *  Pull-up resistors to 5V
 * 
 *  OR ...in case you only have a 3v3 device then use a level converter
 *  
 *  CCS811             Lvl convert     UNOR4 (Wire)
 *                    | HV        |--- +5V
 *    VIN  -----------| LV        |--- +3.3V
 *                    |           |
 *    GND  -----------| GND       |
 *    SDA  -----------| LV <-> HV |--- SDA
 *    SCL  -----------| LV <-> HV |--- SCL
 *    WAKE --------------------------- GND
 *    RST
 *    INT 
 *  
 *  During test the CCS811 would jump high by even touching the wires (especially SCL-line). I suspect
 *  that because I used a cheap board (CJMCU8118) the lines are quickly to long. Hence I connected with 
 *  a level converter the CCS-811. It then worked stable. 
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
#include "Adafruit_CCS811.h" //Click here to get the library: http://librarymanager/All#Adafruit_CCS811

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
#define WIRE_ccs811 Wire

/////////////////////////////////////////////////////////////
// define communication address to use for CCS811
/////////////////////////////////////////////////////////////
#define CCS811_ADDR 0x5A // Default I2C Address
//#define CCS811_ADDR 0x5B //Alternate I2C Address

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
Adafruit_CCS811 mySensor;

struct sen6x_values val;

bool header = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example3 (DRAFT): Display SEN6x and CCS811. press <enter> to start");

  Serial.println(F("Trying to connect."));

  /////////////////////////// CCS811  //////////////////////

  WIRE_ccs811.begin();
  
  // Initialize CCS811 library
  if (! mySensor.begin(CCS811_ADDR,&WIRE_ccs811)){
    Serial.println( "could not start CCS811. Freeze");
    while(1);    
  }
  else
  {
    Serial.println(F("Connected CCS811."));
  }
 
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

  if (! sen6x.start()) {
    Serial.println(F("could not start sen6x."));
    while(1);
  }
  else {
    Serial.println(F("SEN6x started."));
  }
}

void loop() {
  
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
      Serial.println(F("========================== SEN6x ==================================== ======= CCS811 ==========")); 
    
    Serial.print(F("-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print(F("  Humidity:  Temp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }
    
    // CCS811                           SEN6x
    Serial.print(F("\tVOC:    CO2:\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("    [%]      [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
    }

    // CCS811
    Serial.print(F("\t[ppb]\t[ppm]"));
    
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

  // display CCS811 information
  if (mySensor.available())
  {
    // get and calculated
    mySensor.readData();
    Serial.print(F("\t"));
    Serial.print(mySensor.geteCO2());
    Serial.print(F("\t"));
    Serial.print(mySensor.getTVOC());
  }
  else {
    Serial.print("\t-- NO DATA --");
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
