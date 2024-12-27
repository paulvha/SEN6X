/*  
 *  version 1.0 / December 2024 / paulvha
 *    
 *  This example will connect to the sen6x and sen55. 
 *  
 *  you can switch (by pressing enter during measurement) between display the Mass, VOC, NOC, 
 *  Temperature and humidity information, OR display the Mass and PM numbers. 
 *  
 *  
 *  Tested on UNOR4. 
 *  
 *  //////////////// SEN6x //////////////////////
 *  SEN6x Pinout (backview)
 *               
 *  ---------------------
 *  !   | 123456 /      \|
 *  !___|_______/        |
 *  !           \       /|  
 *  !            \     / |
 *  !-------------=====---
 *
 *  Wire1
 *  SEN6x pin     UNO R4
 *                Qwiic
 *  1 VCC -------- 3v3
 *  2 GND -------- GND 
 *  3 SDA -------- SDA 
 *  4 SCL -------- SCL 
 *  
 *  The pull-up resistors are already installed on the UNOR4 for Wire1.
 * 
 *  //////////////// sen55 //////////////////////
 *  
 *  sen55 Pinout (backview)
 *  
 *  ---------------------
 *  ! 1 2 3 4 5 6        |
 *  !___________         |
 *              \        |  
 *               |       |
 *               """""""""
 *  Wire
 *  SEN55 pin     UNO R4
 *  1 VCC -------- 5V
 *  2 GND -------- GND
 *  3 SDA -------- SDA
 *  4 SCL -------- SCL
 *  5 Select ----- GND  (select I2c)
 *  6 NC
 *  
 *  The pull-up resistors to 5V.
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

#include "sen55.h"  // https://github.com/paulvha/sen55
#include "sen6x.h"  // https://github.com/paulvha/SEN6X

///////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN63C, SEN65, SEN66 or SEN68 */
///////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface for sen6x and sen55         */
////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1
#define WIRE_sen55 Wire

/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request debug messages */
////////////////////////////////////////////////////////////
#define SEN6x_DEBUG 0
#define SEN55_DEBUG 0

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

SEN6x sen6x;
SEN55 sen55;

struct sen6x_values sen6x_val;
struct sen6x_concentration_values sen6x_valPM;

struct sen_values sen55_val;
struct sen_values_pm sen55_valPM;
bool header = true;

void setup() {
  
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example21: (DRAFT) SEN6x, sen55. Press <enter> to start");

  Serial.println(F("Trying to connect."));

  // set library debug level
  sen55.EnableDebugging(SEN55_DEBUG);
  sen6x.EnableDebugging(SEN6x_DEBUG);

  /////////////////////////////// SEN6x /////////////////////////////////////
  
  WIRE_sen6x.begin();
  
  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("could not auto-detect SEN6x. set as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // check for SEN6x connection
  if (! sen6x.probe()) {
    Serial.println(F("could not probe / connect with SEN6x."));
    while(1);
  }
  else  {
    Serial.println(F("Connected SEN6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("could not reset SEN6x."));
    while(1);
  }

  /////////////////////////////// sen55 /////////////////////////////////////
  WIRE_sen55.begin();
  
  // Begin communication channel;
  if (! sen55.begin(&WIRE_sen55)) {
    Serial.println(F("could not initialize communication channel."));
    while(1);
  }

  // check for sen55 connection
  if (! sen55.probe()) {
    Serial.println(F("could not probe / connect with sen55."));
    while(1);
  }
  else  {
    Serial.println(F("Connected SEN55."));
  }

  // reset sen55
  if (! sen55.reset()) {
    Serial.println(F("could not reset sen55."));
    while(1);
  }

  if (sen6x.start()) Serial.println(F("SEN6x measurement started"));
  else { 
    Serial.println(F("could not Start sen6x."));
    while(1);
  }

  if (sen55.start()) Serial.println(F("sen55 measurement started"));
  else { 
    Serial.println(F("could not Start sen55."));
    while(1);
  }
  
  Serial.println("PRESS <ENTER> DURING MEASUREMENT TO SWITCH OUTPUT");
}

void loop() {
  static bool DisplayVal = true;
  
  // start reading Sen6x
  if (! sen6x.CheckDataReady()) return;
  
  if (sen6x.GetValues(&sen6x_val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read values."));
    Check_error_6x();
    return;
  }
  
  // also PM values
  if (sen6x.GetConcentration(&sen6x_valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read PM values."));
    Check_error_6x();
    return;
  }

  // start reading sen55
  if ( sen55.Check_data_ready() ){

    if (sen55.GetValues(&sen55_val) != SEN55_ERR_OK) {
      Serial.println(F("Could not read values."));
      return;
    }
    // also PM values
    if (sen55.GetValuesPM(&sen55_valPM) != SEN55_ERR_OK) {
      Serial.println(F("Could not read sen55 values."));
      Check_error_55(); 
      return;
    }
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
    Serial.print(F("\n\t-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print("  Humidity:  Temperature:");
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }

    Serial.print(F("\n\t     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("   %        [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t\t[ppb]"));
    }

    Serial.println(F("\n\tP1.0\tP2.5\tP4.0\tP10\t\n"));
    header = false;
  }
  Serial.print("SEN6x  ");
  Serial.print(sen6x_val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM10);
    
  if(Device != SEN60) {
  
    if (Device != SEN63) {
      Serial.print(F("\t")); 
      Serial.print(sen6x_val.VOC);
      Serial.print(F("\t"));
      Serial.print(sen6x_val.NOX);
    }
    else{
      Serial.print(F("\t")); 
      Serial.print("-----");
      Serial.print(F("\t"));
      Serial.print("-----");
    }
    
    Serial.print(F("\t"));
    Serial.print(sen6x_val.Hum);
    Serial.print(F("\t"));
    Serial.print(sen6x_val.Temp,2);
    Serial.print(F("\t\t"));
    
    if (Device == SEN66 || Device == SEN63) Serial.print(sen6x_val.CO2);
    else if (Device == SEN68) Serial.print(sen6x_val.HCHO,2);
  }
  Serial.println();

  Serial.print("SEN55  ");
  Serial.print(sen55_val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(sen55_val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(sen55_val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(sen55_val.MassPM10);
  Serial.print(F("\t")); 
  Serial.print(sen55_val.VOC);
  Serial.print(F("\t"));
  Serial.print(sen55_val.NOX);
  Serial.print(F("\t"));
  Serial.print(sen55_val.Hum);
  Serial.print(F("\t"));
  Serial.print(sen55_val.Temp,2);
  Serial.print(F("\t\t-----"));
  Serial.println("\n");
}

void Display_valPM()
{
  if (header) {
    Serial.println(F("\n\t-------------Mass -----------   ------------- Number --------------"));
    Serial.println(F("\t     Concentration [μg/m3]             Concentration [#/cm3]"));
    Serial.println(F("\tP1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10\n"));
    header = false;
  }
  
  Serial.print(F("SEN6x\t"));
  Serial.print(sen6x_val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(sen6x_val.MassPM10);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM0);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM1);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM2);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM4);
  Serial.print(F("\t"));
  Serial.print(sen6x_valPM.NumPM10);
  Serial.println();

  Serial.print(F("SEN55\t"));
  Serial.print(sen55_valPM.MassPM1);
  Serial.print(F("\t"));
  Serial.print(sen55_valPM.MassPM2);
  Serial.print(F("\t"));
  Serial.print(sen55_valPM.MassPM4);
  Serial.print(F("\t"));
  Serial.print(sen55_valPM.MassPM10);
  Serial.print(F("\t"));
  Serial.print(sen55_valPM.NumPM0);
  Serial.print(F("\t"));
  Serial.print(sen55_valPM.NumPM1);
  Serial.print(F("\t"));
  Serial.print(sen55_valPM.NumPM2);
  Serial.print(F("\t"));
  Serial.print(sen55_valPM.NumPM4);
  Serial.print(F("\t"));
  Serial.print(sen55_valPM.NumPM10);
  Serial.println(F("\n"));
}

/**
 * check for status errors
 * 
 * return :
 * true : all OK
 * false : status error
 */
bool Check_error_55() {
  uint8_t stat;

  if (sen55.GetStatusReg(&stat) == SEN55_ERR_OK) return(true);

  Serial.println("ERROR !!");
  if (stat & STATUS_SPEED_ERROR_55) {
    Serial.println(F("Fan speed is too high or too low"));
  }
  
  if (stat & STATUS_LASER_ERROR_55) {
    Serial.println(F("Laser is switched on and current is out of range"));
  }

  if (stat & STATUS_FAN_ERROR_55) {
    Serial.println(F("Fan is switched on, but the measured fan speed is 0 RPM"));
  }

  if (stat & STATUS_GAS_ERROR_55) {
    Serial.println(F("Gas sensor error"));
  }
  
  if (stat & STATUS_RHT_ERROR_55) {
    Serial.println(F("Error in internal communication with the RHT sensor"));
  }     

  return(false);
}

/**
 * check for status errors on Sen6x
 * 
 * return :
 * true : all OK
 * false : status error
 */
bool Check_error_6x() {
  uint16_t stat;

  if (sen6x.GetStatusReg(&stat) == SEN6x_ERR_OK) return(true);

  Serial.println("ERROR !!");
  if (stat & STATUS_SPEED_ERROR_6x) {
    Serial.println(F("Fan speed is too high or too low"));
  }

  if (stat & STATUS_FAN_ERROR_6x) {
    Serial.println(F("Fan is switched on, but the measured fan speed is 0 RPM"));
  }

  if (stat & STATUS_GAS_ERROR_6x) {
    Serial.println(F("Gas sensor error"));
  }
  
  if (stat & STATUS_RHT_ERROR_6x) {
    Serial.println(F("Error in internal communication with the RHT sensor"));
  }     

  if (stat & STATUS_CO2_1_ERROR_6x || stat & STATUS_CO2_2_ERROR_6x ) {
    Serial.println(F("Error with CO2 sensor"));
  }   

  if (stat & STATUS_PM_ERROR_6x) {
    Serial.println(F("Error with the PM sensor"));
  }  

  if (stat & STATUS_HCHO_ERROR_6x) {
    Serial.println(F("Error with the HCHO sensor"));
  }  

  return(false);
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
