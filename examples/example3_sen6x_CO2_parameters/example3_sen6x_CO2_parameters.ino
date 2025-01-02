/*  
 *  version DRAFT / January 2025/ paulvha
 *    
 *  This example will connect to the sen6x. It will read the serialnumber, name and different software
 *  levels. 
 *  
 *  It will display the Mass, VOC, NOC, Temperature and Humidity information.
 *  
 *  when <enter> is pressed a menu is presented to change the SEN6x CO2 parameters. 
 *  
 *  WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *  
 *  there is an excellent datasheet for the SCD4x CO2 sensor, which describes the different parameter setting and  
 *  action that can be performed by the SCD41 inside the SEN6x. Make sure to read this to understand the impact of 
 *  the changes. 
 *  
 *   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *  
 *  Tested only on UNOR4, 
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
/* define the SEN6x sensor connected, valid values :
 * SEN60, SEN63, SEN63C, SEN65, SEN66 or SEN68 */
/////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface */
/////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1

/////////////////////////////////////////////////////////////
/* Define default reading in Fahrenheit or Celsius
 *  1 = Celsius
 *  0 = Fahrenheit */
/////////////////////////////////////////////////////////////
#define TEMP_TYPE 1

/////////////////////////////////////////////////////////////
/* define SEN6x driver debug
 * 0 : no messages
 * 1 : request debug messages */
////////////////////////////////////////////////////////////
#define DEBUG 1

/////////////////////////////////////////////////////////////
// wait max INPUTDELAY seconds on input
/////////////////////////////////////////////////////////////
#define INPUTDELAY 10

/////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED /////////////
/////////////////////////////////////////////////////////////

SEN6x sen6x;

struct sen6x_values val;
struct sen6x_tmp_comp TempC;
struct sen6x_RHT_comp TempAcc;
bool TempC_default = true;        // is information in structure still default
bool TempAcc_default = true;      // is information in structure still default
int Type_temp = TEMP_TYPE;
uint8_t dev = Device;                      // Sen6x device type attached

bool header = true;

uint16_t Sen6x_altitude = 0;               // default value
uint16_t Sen6x_pressure = 1013;            // default hPa
uint16_t Sen6x_recal;


void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example3 (DRAFT): Display basic values and handle C02 parameters. Press <enter> to start");

  Serial.println(F("Trying to connect."));

  ///////////////////////// SEN6x ///////////////////////////////////
  
  // set library debug level
  sen6x.EnableDebugging(DEBUG);

  WIRE_sen6x.begin();

  // Begin communication channel
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("Could not auto-detect SEN6x. Set as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // check for connection
  if (! sen6x.probe()) {
    Serial.println(F("Could not probe / connect with sen6x."));
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

  if (! sen6x.start()) {
    Serial.println(F("Could not Start sen6x."));
    while(1);
  }

  Serial.setTimeout(INPUTDELAY * 1000);

  if (dev != SEN60 && dev != SEN66)
    Serial.println(F("Temperature settings NOT supported on connected SEN60"));
  else
    Serial.println(F("\nPress <enter> during measurement to handle CO2 parameters\n"));
}

void loop() 
{
  if (sen6x.CheckDataReady())  Display_val();
  
  // trigger reading adjusting Temperature compensation (pressed <enter>)
  if (Serial.available()) {
    flush();
    
    if (dev != SEN60 && dev != SEN66){
      Serial.println(F("CO2 settings NOT supported on connected sensor. Only SEN63C and SEN66"));
      delay(2000);
    }
    else
      CheckCo2Values();
  }
  else
    delay(2000);
}

void Display_val()
{
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read values."));
    return;
  }
  
  if (header) {
    Serial.print(F("==================================== SEN6x ================================   "));
    
    Serial.print(F("\n-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print("  Humidity:  Temperature:");
      if (Device == SEN66 || Device == SEN63) Serial.print(F(" CO2:"));
      if (Device == SEN68) Serial.print(F(" HCHO:"));
    }
    
    Serial.print(F("\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      
      if (Type_temp) Serial.print(F("   [%]\t[*C]\t     "));
      else Serial.print(F("   [%]\t[*F]\t   "));
      
      if (Device == SEN66 || Device == SEN63) Serial.print(F("[ppm]"));
      if (Device == SEN68) Serial.print(F("[ppb]"));
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
    
    if (Type_temp) Serial.print(val.Temp,2);
    else Serial.print( ((val.Temp *9)/5 + 32 ),2);
    
    Serial.print(F("\t     "));
    
    if (Device == SEN66 || Device == SEN63) Serial.print(val.CO2);
  }

  Serial.print(F("\n"));
}

/**
 * set the temperature value
 */
void CheckCo2Values() {
  int inp;
  uint8_t ret;
  float f;
  bool B_val;
  uint16_t U16_val;

  header = true;
  
  while(1) {
    
    disp_tmp_info();

    inp = GetInput(1,8,&f);

    // cancel (or timeout)
    if (inp == 8 || inp == -100) {
      Serial.println(F("Cancel"));
      return;
    }
    
    else if (inp == 1) {   // Get CO2 Self calibration setting
      
      if (sen6x.GetCo2SelfCalibratrion(&B_val) != SEN6x_ERR_OK) {
        Serial.println(F("Could not read the CO2 self calibration setting\n"));
        continue;
      }
      if (B_val) Serial.println(F("Self calibration is enabled"));
      else Serial.println(F("Self calibration is disabled"));
    }

    else if (inp == 2) { // Set CO2 Self calibration setting
      
      Serial.println(F("0 = disable , 1 = enable self calibration"));
      inp = GetInput(0,1,&f);
      if (inp == -100) continue;

      if (sen6x.SetCo2SelfCalibratrion(inp) != SEN6x_ERR_OK) {
        Serial.println(F("Could not set the CO2 self calibration setting\n"));
      }
      else
        Serial.println(F("Updated CO2 self calibration setting\n"));
    }

    else if (inp == 3) { // Get ambient pressure compensation value"));
      
      if (sen6x.GetAmbientPressure(&U16_val) != SEN6x_ERR_OK) {
        Serial.println(F("Could not read the CO2 ambient pressure compensation\n"));
        continue;
      }

      Serial.print(F("Ambient pressure compensation value "));
      Serial.print(U16_val);
      Serial.println(" hPa");
    
    }
    
    else if (inp == 4) { // set ambient pressure compensation value"));
      
      Serial.println(F("Ambient pressure overrides any altitude pressure compensation"));
      Serial.println(F("Provide new Ambient pressure compensation between 700 and 1200 hPa"));
      inp = GetInput(700,1200,&f);
      if (inp == -100) continue;
      U16_val= (uint16_t) inp;
      if (sen6x.SetAmbientPressure(U16_val) != SEN6x_ERR_OK) {
        Serial.println(F("Could not set the CO2 ambient pressure compensation\n"));
      }
      else
        Serial.println(F("New ambient pressure compensation has been set"));
   }
    
    else if (inp == 5) { // Get altitude compensation value"));
      if (sen6x.GetAltitude(&U16_val) != SEN6x_ERR_OK) {
        Serial.println(F("Could not read the CO2 altitude compensation pressure\n"));
        continue;
      }

      Serial.print(F("Altitude pressure compensation value "));
      Serial.print(U16_val);
      Serial.println(" meter");
    }

    else if (inp == 6) { // Set altitude compensation value"));
      Serial.println(F("Provide new Altitude pressure compensation between 0 and 3000 meter"));

      inp = GetInput(0,3000,&f);
      if (inp == -100) continue;
      U16_val= (uint16_t) inp;
      
      if (sen6x.SetAltitude(U16_val) != SEN6x_ERR_OK) {
        Serial.println(F("Could not set the CO2 altitude pressure compensation\n"));
      }
      else
        Serial.println(F("New altitude pressure compensation has been set"));
    }

    else if (inp == 7) {    // Force CO2 recalibration
      Serial.println(F("IF YOU DO NOT KNOW WHAT YOU ARE DOING... CANCEL !! IT CAN IMPACT THE QUALITY OF CO2 RESULTS\n"));

      Serial.println(F("Provide the target CO2 concentration in ppm which the SCD41 will be during Forced recalibration"));
      Serial.println(F("more information SCD41/SEN66 field calibration in the SCD4x Data sheet, chapter 3.8."));

      inp = GetInput(400,5000,&f);
      if (inp == -100) continue;
      U16_val= (uint16_t) inp;
           
      if (sen6x.ForceCO2Recal(&U16_val) != SEN6x_ERR_OK) {
        Serial.println(F("Could not trigger CO2 forced recalibration\n"));
      }
      else {
        Serial.print(F("Returned from the FRC correction (i.e. the magnitude of the correction) :"));
        Serial.println(U16_val);
      }
    }
  }
}
/**
 * Display values. 
 */
void disp_tmp_info()
{
  
  Serial.println(F("\n************** CO2 parameters **********************"));
  Serial.println(F("  !!! READ the datasheet for the SCD4x CO2 sensor  !!!"));
  Serial.println(F("****************************************************\n"));
  Serial.println(F("1 Get CO2 Self calibration setting"));
  Serial.println(F("2 Set CO2 Self calibration setting"));
  Serial.println(F("3 Get ambient pressure compensation value"));
  Serial.println(F("4 Set ambient pressure compensation value"));
  Serial.println(F("5 Get altitude compensation value"));
  Serial.println(F("6 Set altitude compensation value"));
  Serial.println(F("7 Force CO2 recalibration\n"));

  Serial.println(F("8 Cancel"));
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
 * This is not documented in the datasheet but taken from Sen5x
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
 * get input from keyboard
 * @param 
 *  maxop: the maximum number to expect
 *  minop : the minimum number
 *  f : pointer to float to store the float input value
 * 
 * @return : 
 *  -100 NO entry was given
 *  else valid entry.
 */
int GetInput(int minop, int maxop, float *f)
{
  // flush pending input
  flush();
  
  Serial.print(F("Provide the number. Only <enter> is return. (or wait "));
  Serial.print(INPUTDELAY);
  Serial.println(F(" seconds)"));
  
  while(1) {
    
    String Keyb = Serial.readStringUntil(0x0a);

    if (Keyb.length() < 2) return -100;
    *f = Keyb.toFloat();
    int in = Keyb.toInt();
    
    if(in > maxop || in < minop) {
      Serial.print(in);
      Serial.print(" Invalid option. Min ");
      Serial.print(minop);
      Serial.print(", Max ");
      Serial.println(maxop);
    }
    else
      return(in);
  }
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
