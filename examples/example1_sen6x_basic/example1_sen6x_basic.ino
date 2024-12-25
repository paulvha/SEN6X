/*  
 *  version DRAFT / December 2024 / paulvha
 *    
 *  This example will connect to the sen6x. It will read the serialnumber, name and different software
 *  levels. 
 *  
 *  It will display the Mass, VOC, NOC, Temperature and humidity information, as well as display
 *  Mass and PM numbers. 
 *  
 *  
 *  Tested on UNOR4 
 *  
 *  ### TODO #### Artemis ATP, UNOR3, ATmega, Due, ESP32
 *   
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
 * ..........................................................
 * ## TODO
 *  Successfully tested on ESP32 
 *  SEN55 pin     ESP32
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA (pin 21)
 *  4 SCL -------- SCL (pin 22)
 *  5 internal connected to pin 2
 *  6 internal connected to Pin 1
 *
 *  The pull-up resistors should be to 3V3
 *  ..........................................................
 * ## TODO
 *  Successfully tested on ATMEGA2560, Due
 *
 *  SEN55 pin     ATMEGA
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA
 *  4 SCL -------- SCL
 *  5 internal connected to pin 2
 *  6 internal connected to Pin 1
 *
 *  ..........................................................
 *  Successfully tested on UNO R3
 * ## TODO
 *  SEN55 pin     UNO
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA
 *  4 SCL -------- SCL
 *  5 internal connected to pin 2
 *  6 internal connected to Pin 1
 *
 *  When UNO-board is detected some buffers reduced and the call 
 *  to GetErrDescription() is removed to allow enough memory.
 *  
 *  ..........................................................
 *  Successfully tested on Artemis/Apollo3 Sparkfun
 * ## TODO
 *  SEN55 pin     Artemis
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA (pin 21)
 *  4 SCL -------- SCL (pin 22)
 *  5 internal connected to pin 2
 *  6 internal connected to Pin 1
 *  
 *  The pull-up resistors should be to 3v3.
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
 * valid values, SEN60, SEN63, SEN63C, SEN65, SEN66 or SEN68
*/
 ////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface */
 ////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1

/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request debug messages */
 ////////////////////////////////////////////////////////////
#define DEBUG 0

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

SEN6x sen6x;

struct sen6x_values val;
struct sen6x_concentration_values valPM;
bool header = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example1 (DRAFT): Display basic values press <enter> to start");

  Serial.println(F("Trying to connect."));

  // set library debug level
  sen6x.EnableDebugging(DEBUG);

  WIRE_sen6x.begin();

  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("could not auto-detect SEN6x. set as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // check for connection
  if (! sen6x.probe()) {
    Serial.println(F("could not probe / connect with sen6x."));
    while(1);
  }
  else  {
    Serial.println(F("Connected sen6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("could not reset sen6x."));
    while(1);
  }
  
  Display_Device_info();
  
  Serial.println("PRESS <ENTER> DURING MEASUREMENT TO SWITCH OUTPUT");

  if (! sen6x.start()) {
    Serial.println(F("could not Start sen6x."));
    while(1);
  }
}

void loop() {
  static bool DisplayVal = true;
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read values."));
    return;
  }

  if (sen6x.GetConcentration(&valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read PM values."));
    return;
  }

  if (DisplayVal)  Display_val();
  else  Display_valPM();

  // Switch output when <enter> has been pressed.
  if (Serial.available()) {
    if (DisplayVal) DisplayVal = false;
    else DisplayVal = true;
    header = true;
    flush();
  }
  
  delay(2000);
}
  
void Display_val()
{
  if (header) {
    Serial.print(F("\n-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print("  Humidity:  Temperature:");
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }

    Serial.print(F("\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("   %        [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t\t[ppb]"));
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
    
  if(Device == SEN60) {
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
  Serial.print(F("\t\t"));
  
  if (Device == SEN66 || Device == SEN63) Serial.print(val.CO2);
  else if (Device == SEN68) Serial.print(val.HCHO,2);
  Serial.println();
}

void Display_valPM()
{
  if (header) {
    Serial.println(F("\n-------------Mass -----------   ------------- Number --------------"));
    Serial.println(F("     Concentration [μg/m3]             Concentration [#/cm3]"));
    Serial.println(F("P1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10\n"));
    header = false;
  }
  
  Serial.print(val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(val.MassPM10);
  Serial.print(F("\t"));
  Serial.print(valPM.NumPM0,2);
  Serial.print(F("\t"));
  Serial.print(valPM.NumPM1,2);
  Serial.print(F("\t"));
  Serial.print(valPM.NumPM2,2);
  Serial.print(F("\t"));
  Serial.print(valPM.NumPM4,2);
  Serial.print(F("\t"));
  Serial.print(valPM.NumPM10,2);
  Serial.println();
}

/** 
 *  display the device information
 */
void Display_Device_info()
{
  char num[32];
  
  // get SEN6x serial number
  if (sen6x.GetSerialNumber(num,32) != SEN6x_ERR_OK) {
    Serial.println(F("could not read serial number."));
    while(1);
  }
  Serial.print(F("Serial number: "));
  Serial.println(num);

  //get product name
  if (sen6x.GetProductName(num,32) != SEN6x_ERR_OK) {
    Serial.println(F("could not read product name."));
    while(1);
  }
  Serial.print(F("Product name : "));
  
  // if name is provided by the device
  if (strlen(num) > 2) {
    Serial.println(num);
  }
  else  // based on setting in sketch (starting with dot)
  {
    if (Device == SEN60) Serial.println(".SEN60");
    else if (Device == SEN63) Serial.println(".SEN63C");
    else if (Device == SEN65) Serial.println(".SEN65");
    else if (Device == SEN66) Serial.println(".SEN66");
    else if (Device == SEN68) Serial.println(".SEN68");
    else Serial.println("Unknown !");
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
