/*  
 *  version 1.0 / February 2025 / paulvha
 *    
 *  This example will connect to the sen6x. It will read the serialnumber, name and different software
 *  levels. 
 *  
 *  It will display the Mass, VOC, NOx, Temperature and Humidity information, as well as display
 *  Mass and PM numbers. 
 *  
 *  Version 1.0 / January 2026 / paulvha 
 *  Special for UNOQ
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
 *  Successfully tested on UNOQ
 *  
 *                connector
 *  SEN6X pin     UNOQ
 *  1 VCC -------- 3v3
 *  2 GND -------- GND        10K
 *  3 SDA -------- SDA ------=====---- 3v3
 *  4 SCL -------- SCL ------=====---- 3v3
 *  5 internal connected to pin 2 
 *  6 internal connected to Pin 1
 *  
 *  The pull-up resistors are needed to 3v3
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
#include <Arduino_RouterBridge.h>   // special for UNOQ
#define SERIAL 1
#define BRIDGE 2

///////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN63C, SEN65, SEN66 or SEN68 */
///////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface */
////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire

/////////////////////////////////////////////////////////////
/* define output / display channel to use. valid options:
 *   SERIAL 
 *   BRIDGE / Monitor
 */
/////////////////////////////////////////////////////////////
#define OUTPUT BRIDGE

/////////////////////////////////////////////////////////////
/* define driver debug (only when connected to Serial)
 * 0 : no messages
 * 1 : request debug messages */
////////////////////////////////////////////////////////////
#define DEBUG 0

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

#if OUTPUT == SERIAL 
  #define DISPLAY Serial
#elif OUTPUT == BRIDGE
  #define DISPLAY Monitor
#else
  #error "Incorrect OUTPUT defined"
#endif

SEN6x sen6x;

struct sen6x_values val;
struct sen6x_concentration_values valPM;
bool header = true;
uint8_t dev = Device;       // indicate connected sensor
char out[300];              // display buffer

void setup() {
  DISPLAY.begin(115200);
  while (!DISPLAY) delay(100);

  DISPLAYTrigger((char *) "SEN6x-Example40: UNOQ - Display basic values press <enter> to start");

  DISPLAY.print(F("Trying to connect.\r\n"));

  // set driver debug level ONLY when OUTPUT is SERIAL
#if OUTPUT == SERIAL
  sen6x.EnableDebugging(DEBUG);
#endif 

   WIRE_sen6x.begin();

  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    DISPLAY.print(F("Could not auto-detect SEN6x. Assume as defined in sketch.\r\n"));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // check for connection
  if (! sen6x.probe()) {
    DISPLAY.print(F("Could not probe / connect with sen6x. \r\nDid you define the right sensor in sketch?\r\n"));
    while(1);
  }
  else  {
    DISPLAY.print(F("Connected sen6x.\r\n"));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    DISPLAY.print(F("Could not reset sen6x. Freeze.\r\n"));
    while(1);
  }
  
  Display_Device_info();

  // CO2 auto calibration
  if ((dev == SEN66 || dev == SEN63) & DISABLE_ASC) {
    if (sen6x.SetCo2SelfCalibratrion(false) == SEN6x_ERR_OK) {
      DISPLAY.print(F("CO2 ASC disabled\r\n"));
    }
    else {
     DISPLAY.print(F("Could not disable ASC\r\n"));
    }
  }
  
  if (! sen6x.start()) {
    DISPLAY.print(F("Could not Start sen6x.Freeze.\r\n"));
    while(1);
  }
    
  DISPLAY.print(F("\r\nPRESS <ENTER> DURING MEASUREMENT TO SWITCH OUTPUT\r\n"));

}

void loop() {
  static bool DisplayVal = true;
  
  if (! sen6x.CheckDataReady()) return;
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    DISPLAY.print(F("Could not read values.\r\n"));
    return;
  }

  if (sen6x.GetConcentration(&valPM) != SEN6x_ERR_OK) {
    DISPLAY.print(F("Could not read PM values.\r\n"));
    return;
  }

  if (DisplayVal)  Display_val();
  else  Display_valPM();

  // Switch output when <enter> has been pressed.
  if (DISPLAY.available()) {
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
    
    if (Device == SEN60) {
      snprintf(out, sizeof(out),"\r\n%s\r\n%s\r\n%s\r\n","-------------Mass -----------","      Concentration [μg/m3]"
      , "P1.0\tP2.5\tP4.0\tP10");
    }
    else if (Device == SEN63) {
      snprintf(out, sizeof(out),"\r\n%s%s%s\r\n%s%s%s\r\n%s\r\n","-------------Mass -----------","  Humidity:  Temperature:",
      "   CO2:","      Concentration [μg/m3]", "   %        [*C]", "\t\t[ppm]", "P1.0\tP2.5\tP4.0\tP10");
    }
    else if (Device == SEN66) {
      snprintf(out, sizeof(out),"\r\n%s%s%s%s\r\n%s%s%s%s\r\n%s\r\n","-------------Mass -----------","    VOC:  NOX:", 
      "  Humidity:  Temperature:", "   CO2:", "      Concentration [μg/m3]", "      index  index", 
      "   %        [*C]", "\t\t[ppm]", "P1.0\tP2.5\tP4.0\tP10");
    }
    else if (Device == SEN68) {
      snprintf(out, sizeof(out),"\r\n%s%s%s%s\r\n%s%s%s%s\r\n%s\r\n","-------------Mass -----------","    VOC:  NOX:", 
      "  Humidity:  Temperature:", "   HCHO:", "      Concentration [μg/m3]", "      index  index", 
      "   %        [*C]", "\t\t[ppb]", "P1.0\tP2.5\tP4.0\tP10");
    }

    DISPLAY.print(out);
    header = false;
  }

  if (Device == SEN60) {
    snprintf(out, sizeof(out),"%2.2f\t%2.2f\t%2.2f\t%2.2f\r\n",val.MassPM1, val.MassPM2, val.MassPM4, val.MassPM10);
  }
  else if (Device == SEN63) {
    snprintf(out, sizeof(out),"%2.2f\t%2.2f\t%2.2fs\t%2.2f\t%2.2f\t%2.2f\t%d\r\n",val.MassPM1, val.MassPM2, val.MassPM4, 
    val.MassPM10, val.Hum, val.Temp, val.CO2);
  }
  else if (Device == SEN66) {
    snprintf(out, sizeof(out),"%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t\t%d\r\n",val.MassPM1, val.MassPM2, val.MassPM4, val.MassPM10, 
    val.VOC, val.NOX, val.Hum, val.Temp, val.CO2);
  }
  else if (Device == SEN68) {
    snprintf(out, sizeof(out),"%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t\t%2.2f\r\n",val.MassPM1, val.MassPM2, val.MassPM4, val.MassPM10, 
    val.VOC, val.NOX, val.Hum, val.Temp, val.HCHO);
  }

  DISPLAY.print(out);
}

void Display_valPM()
{
  if (header) {
    snprintf(out, sizeof(out),"\r\n%s\r\n%s\r\n%s\r\n",
    "-------------Mass -----------   ------------- Number --------------",
    "     Concentration [μg/m3]             Concentration [#/cm3]",
    "P1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10");
    
    DISPLAY.print(out);
    header = false;
  }
  
  snprintf(out, sizeof(out),"%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\r\n",val.MassPM1, val.MassPM2, val.MassPM4,
   val.MassPM10, valPM.NumPM0, valPM.NumPM1, valPM.NumPM2, valPM.NumPM4, valPM.NumPM10);

  DISPLAY.print(out);
}

/** 
 *  display the device information
 */
void Display_Device_info()
{
  char num[32];
  bool det;
  
  // get SEN6x DISPLAY number
  if (sen6x.GetSerialNumber(num,32) != SEN6x_ERR_OK) {
    DISPLAY.print(F("Could not read DISPLAY number. Freeze.\r\n"));
    while(1);
  }
  snprintf(out, sizeof(out),"%s%s\r\n", "Serial number: ", num);
  DISPLAY.print(out);

  //get product name
  if (sen6x.GetProductName(num,32) != SEN6x_ERR_OK) {
    DISPLAY.print(F("Could not read product name. Freeze.\r\n"));
    while(1);
  }
  
  // if name is provided by the device
  if (strlen(num) > 2) {
    snprintf(out, sizeof(out),"%s%s\r\n", "Product name: ", num);
  }
  else 
  {
    snprintf(out, sizeof(out),"%s%s\r\n", "Product name: ", "not available");
  }
  DISPLAY.print(out);
  
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
    DISPLAY.print(F("Could not read version. Freeze\r\n"));
    while(1);
  }
  snprintf(out,sizeof(out),"Firmware level: %d.%d, Hardware level: %d.%d, protocol: %d.%d, Library level: %d.%d\r\n ", 
  v.F_major, v.F_minor, v.H_major, v.H_minor, v.P_major, v.P_minor, v.L_major, v.L_minor );

  DISPLAY.print(out);
}

/**
 * flush input
 */
void flush()
{
  do {
    delay(200);
    DISPLAY.read();
  } while(DISPLAY.available());
}

/**
 * DISPLAYTrigger prints repeated message, then waits for enter
 * to come in from the DISPLAY port.
 */
void DISPLAYTrigger(char * mess)
{
  DISPLAY.println();

  while (!DISPLAY.available()) {
    DISPLAY.println(mess);
    delay(2000);
  }

  flush();
}
