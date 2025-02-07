/*  
 *  version 1.0 / February 2025 / paulvha
 *    
 *  This example will connect to the sen6x. It will read the serialnumber, name and different software
 *  levels. 
 *  
 *  It will display the Mass, VOC, NOx, Temperature and humidity information, as well as the raw values
 *  from the humidity, temperature, VOC, NOx and CO2 if the sensor provides. NOT valid for SEN60 !

 *  January 2025
 *  !!!! Some RAW values did not make sense to me.  BUT... now they do (somehow)
 *  More details below. 
 *  
 *  Based on the better understanding the raw values are more suitable for debugging and testing
 *  the algorithm. Checking whether new data is available can / should be done with GET DATA READY SEN6x
 
 *  According to the datasheet:
 *  Description: Returns the measured raw values. The command Get Data Ready SEN6x can be used to check if
 *  new data is available since the last read operation. If no new data is available, the previous values will be
 *  returned. If no data is available at all (e.g. measurement not running for at least one second), all values will be
 *  at their upper limit (0xFFFF for uint16, 0x7FFF for int16).
 *  
 *  -------------------------------------------------------------
 *  
 *  Take humidity: 
 *  ==============
 *  According to datasheet:  Value is scaled with factor 100: RH [%] = value / 100
 *  
 *  When just starting the raw value is 3766, the provided value is 37.7 (Make sense / 100)
 *  After ~5 min
 *  The raw value has been trending around 3780, the provided value is trending to 47.09 (external measured 46%)
 *  so the real value makes sense.... the raw value does not.
 * 
 *  After contact with Sensirion I have understood that they know how much impact the surrounding in the SEN6x has on 
 *  the sensor and make the calculated adjustments. If that is not enough (e.g. the SEN6x is further boxed), you can make
 *  more adjustments with the Temperature Offset parameters and/or Temperature acceleration parameters.
 *  
 *  Take Temperature:
 *  =================
 *  According to datasheet: Value is scaled with factor 200: T [°C] = value / 200
 *  
 *  When just starting the raw value is 4452, the provided value is 22.23C (makes sens /200)
 *  after ~5 min
 *  The raw value is trending around 4429, the real value is 18.6 (external measured 19C)
 *  so the real value makes sense.... the raw value does not.
 * 
 *  After contact with Sensirion I have understood that they know how much impact the surrounding in the SEN6x has to 
 *  the sensor and make the calculated adjustment. If that is not enough (e.g. the SEN6x is further boxed), you can make
 *  more adjustments with the Temperature Offset parameters and /or Temperature acceleration parameters.
 *  
 *  
 *  Take VOC:
 *  ========
 *  According to datasheet: Raw measured VOC ticks without scale factor
 *  
 *  When just starting the raw value is quickly 31011, and slowing moving up to 32212. There is for 
 *  sure logic to have the VOC index created from the VOC index from the SGP41.
 *  However with the VOC at 101 and the ticks around 32210, breathing to the SEN66. Make the VOC index 
 *  go in big steps (<10 seconds) to 180 and another 10 second later to 97 and slowly return to 100. The  
 *  VOC ticks however moved a little down to 32056 and soon after back to 32220.
 *  Is that correct ??? I don't know. I am spending more time to better understand and will update later
 *  
 *  Take NOx:
 *  =========
 *  According to datasheet: Raw measured Nox ticks without scale factor
 * 
 *  When just starting the raw value is quickly 17969, and slowing moving down to ~15730. There is for 
 *  sure logic to have the Nox index of 1 created from the Nox index from the SGP41.
 *  However with the Nox at 1 and the ticks around 15730, breathing to the SEN66. Make the NOX index 
 *  go not changing and staying 1. The Nox ticks however moved a quickly to 16481 soon after back to 15720.
 *  Is that correct ??? I don't know.  I am spending more time to better understand and will update later
 *  
 *  Take CO2
 *  ========
 *  Raw CO2 looks correctly correlated to provided real value.
 *  
 *  --------------------------------------------------------------------------------------------
 *  Tested on UNOR4 
 *  
 *   ..........................................................
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
 *                Qwiic connector
 *  SEN6X pin     UNOR4
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA 
 *  4 SCL -------- SCL 
 *  5 internal connected to pin 2 
 *  6 internal connected to Pin 1
 *  
 *  The pull-up resistors are already installed on the UNOR4 for Wire1.
 * ..................................................................
 * 
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
 *  
 */

#include "sen6x.h"

/////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * SEN60, SEN63, SEN63C, SEN65, SEN66 or SEN68 */
////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

////////////////////////////////////////////////////////////
/* define which Wire interface */
////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1

////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request debug messages */
////////////////////////////////////////////////////////////
#define DEBUG 0

//////////////////////////////////////////////////////////////
/* By default the SEN66 and SEN63 perform CO2 automatic self
 * calibration (ASC). This requires the sensor to be exposed
 * for at least 4 hours during a week to external / outside air.
 * If not do not, the advise is to disable.
 * See SCD4x data sheet, chapter 3.8
 * 
 * true = disable ASC, false keep enabled */
//////////////////////////////////////////////////////////////
#define DISABLE_ASC false

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

SEN6x sen6x;

struct sen6x_values val;
struct sen6x_raw_values valRaw;
bool header = true;
bool DisplayHex = false;         // start display RAW variables decimal
uint8_t dev = Device;            // indicate connected sensor

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example7: Display basic values and RAW values. Press <enter> to start");

  Serial.println(F("Trying to connect."));

  // set library debug level
  sen6x.EnableDebugging(DEBUG);

  WIRE_sen6x.begin();

  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("Could not auto-detect SEN6x. Assume as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // check for connection
  if (! sen6x.probe()) {
    Serial.println(F("Could not probe / connect with sen6x. \nDid you define the right sensor in sketch?"));
    while(1);
  }
  else  {
    Serial.println(F("Connected sen6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("Could not reset sen6x."));
    while(1);
  }
  
  Display_Device_info();

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
    Serial.println(F("Could not Start sen6x."));
    while(1);
  }
  
  if ( dev == SEN60) {
    Serial.println(F("\nSEN60 does NOT support RAW values"));
  }
  else
    Serial.println(F("\nPRESS <ENTER> DURING MEASUREMENT TO CHANGE RAW VALUES OUTPUT"));
}

void loop() {
  
  if (! sen6x.CheckDataReady()) return;
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read values."));
    return;
  }

  if (sen6x.GetRawValues(&valRaw) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read Raw values."));
    return;
  }

  Display_val();

  // Switch RAW output when <enter> has been pressed.
  if (Serial.available()) {
    if (DisplayHex) DisplayHex = false;
    else DisplayHex = true;
    header = true;
    flush();
  }
  
  delay(2000);
}
  
void Display_val()
{

  if (header) {

    Serial.print(F("\n============================ values =================================="));
    if (DisplayHex) Serial.println(F("  ============ RAW values (HEX) ======="));
    else Serial.println(F("  ============ RAW values (DEC) ======="));
    
    Serial.print(F("\n-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("\tVOC:\tNOx:"));
      Serial.print(F("\tHum:\tTemp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\tCO2:"));
      if (Device == SEN68) Serial.print(F("\tHCHO:"));

      // raw values
      Serial.print(F("\tHum:\tTemp:"));
      if (Device != SEN63) Serial.print(F("\tVOC:\tNOX:"));
      if (Device == SEN66) Serial.print(F("\tCO2:"));       // NOT SEN63 according to datasheet ???
    }

    Serial.print(F("\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("\tindex\tindex"));
      Serial.print(F("\t %\t[*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
      
      // raw values
      Serial.print(F("\t %\t[*C]"));
      if (Device != SEN63) Serial.print(F("\tticks\tticks"));
      if (Device == SEN66) Serial.print(F("\t[ppm]"));          // NOT SEN63 according to datasheet ???
    }

    Serial.println(F("\nP1.0\tP2.5\tP4.0\tP10\t\n"));
    header = false;
  }

  Serial.print(val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(val.MassPM10);
    
  if (Device == SEN60) {
    Serial.println();
    return;
  }

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

  // raw values
  if (DisplayHex)
  {
    Serial.print(F("\t0x"));
    Serial.print(valRaw.Hum, HEX);
    Serial.print(F("\t0x"));
    Serial.print(valRaw.Temp,HEX);
  
    if (Device != SEN63) {
      Serial.print(F("\t0x")); 
      Serial.print(valRaw.VOC,HEX);
      Serial.print(F("\t0x"));
      Serial.print(valRaw.NOX,HEX);
    }
  
    if (Device == SEN66) {
      Serial.print(F("\t0x"));
      Serial.print(valRaw.CO2,HEX);
    }    
  }
  else 
  {
    Serial.print(F("\t"));
    Serial.print(valRaw.Hum);
    Serial.print(F("\t"));
    Serial.print(valRaw.Temp);
  
    if (Device != SEN63) {
      Serial.print(F("\t")); 
      Serial.print(valRaw.VOC);
      Serial.print(F("\t"));
      Serial.print(valRaw.NOX);
    }
  
    if (Device == SEN66) {
      Serial.print(F("\t"));
      Serial.print(valRaw.CO2);
    }
  }
  Serial.println();
}

/** 
 *  display the device information
 */
void Display_Device_info()
{
  char num[32];
  bool det;
  
  // get SEN6x serial number
  if (sen6x.GetSerialNumber(num,32) != SEN6x_ERR_OK) {
    Serial.println(F("could not read serial number. Freeze."));
    while(1);
  }
  Serial.print(F("Serial number: "));
  Serial.println(num);

  //get product name
  if (sen6x.GetProductName(num,32) != SEN6x_ERR_OK) {
    Serial.println(F("could not read product name. Freeze."));
    while(1);
  }
  Serial.print(F("Product name : "));
  
  // if name is provided by the device
  if (strlen(num) > 2) {
    Serial.println(num);
  }
  else  // based on setting in sketch (starting with dot)
  {
    dev = sen6x.GetDevice(&det);
    if (! det) Serial.print(".");
    if (dev == SEN60) Serial.println(F("SEN60"));
    else if (dev == SEN63) Serial.println(F("SEN63C"));
    else if (dev == SEN65) Serial.println(F("SEN65"));
    else if (dev == SEN66) Serial.println(F("SEN66"));
    else if (dev == SEN68) Serial.println(F("SEN68"));
    else Serial.println(F("Unknown !"));
  }
  
  Display_Versions();
}

/**
 * Display different versions
 * 
 * This is not documented in the datasheet
 */
void Display_Versions()
{
  struct sen6x_version v;
  
  if (sen6x.GetVersion(&v) != SEN6x_ERR_OK) {
    Serial.println(F("could not read version. Freeze"));
    while(1);
  }

  Serial.print(F("Version info : "));
  Serial.print(F("Firmware: "));
  Serial.print(v.F_major);
  Serial.print(".");
  Serial.print(v.F_minor);
  
  Serial.print(F(", Hardware: "));
  Serial.print(v.H_major);
  Serial.print(".");
  Serial.print(v.H_minor);

  Serial.print(F(", Protocol: "));
  Serial.print(v.P_major);
  Serial.print(".");
  Serial.print(v.P_minor);

  Serial.print(F(", Library: "));
  Serial.print(v.L_major);
  Serial.print(".");
  Serial.print(v.L_minor);
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
