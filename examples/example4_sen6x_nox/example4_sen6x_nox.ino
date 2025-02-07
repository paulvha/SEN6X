/*  
 *  version 1.0 / February 2025 / paulvha
 * 
 *  This example will read the MASS / VOC / NOx values and save/restore Nox algorithm tuning
 * 
 *  Pressing enter during the measurement will display the current Nox-values and provide a menu to
 *  change the parameters that can be changed. 
 *   
 *  Nox values  (source datasheet sen6x)
 *  Gets the parameters to customize the NOx algorithm. For more information on what the
 *  parameters below do, refer to Sensirion’s NOx Index for Indoor Air Applications [5].
 *  
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

///////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN63C, SEN65, SEN66 or SEN68 */
//////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface */
////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1

/////////////////////////////////////////////////////////////
// wait max 10 seconds on input
/////////////////////////////////////////////////////////////
#define INPUTDELAY 10

/////////////////////////////////////////////////////////////
/* define library debug
 * 0 : no messages
 * 1 : request debug messages */
//////////////////////////////////////////////////////////////
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

SEN6x sen6x;
struct sen6x_values val;
struct sen6x_concentration_values valPM;
struct sen6x_xox nox;
bool header = true;
uint8_t dev = Device;            // indicate connected sensor

void setup() {
  
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example4: Display basic values and get/set Nox Algorithm Tuning. press <enter> to start");

  Serial.println(F("Trying to connect."));

  // set library debug level
  sen6x.EnableDebugging(DEBUG);

  WIRE_sen6x.begin();
  
  // Begin communication channel;
  if (! sen6x.begin(&WIRE_sen6x)) {
    Serial.println(F("could not auto-detect SEN6x. Assume as defined in sketch."));
    
    // inform the library about the SEN6x sensor connected
    sen6x.SetDevice(Device);
  }

  // check for connection
  if (! sen6x.probe()) {
    Serial.println(F("Could not probe / connect with sen6x.\nDid you define the right sensor in sketch?"));
    while(1);
  }
  else  {
    Serial.println(F("Connected sen6x."));
  }

  // reset SEN6x
  if (! sen6x.reset()) {
    Serial.println(F("Could not reset sen6x. Freeze."));
    while(1);
  }

  Display_Device_info();

  Serial.setTimeout(INPUTDELAY * 1000);

  if (! sen6x.start()) {
    Serial.println(F("Could not Start sen6x. Freeze."));
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

  if (dev == SEN60 || dev == SEN63)
    Serial.println(F("VOC / Nox NOT supported on connected device type"));
  else
    Serial.println(F("\nPress <enter> during measurement to handle Nox state\n"));

}

void loop() {
  
  if (! sen6x.CheckDataReady())  return;
  
  if (sen6x.GetValues(&val) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read values."));
    return;
  }

  if (sen6x.GetConcentration(&valPM) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read PM values."));
    return;
  }
  
  Display_val();

  // trigger reading adjusting VOC algorithm (pressed <enter>)
  if (Serial.available()) {
    flush();
    CheckNOxValues();
    header = true;
  }
  else
    delay(2000);
}

/**
 * display the mass and NOxVOC values
 */
void Display_val()
{
  if (header) {
    Serial.print(F("\n-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print(F("  Humidity:  Temperature:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("    CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }

    Serial.print(F("\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("      %     [*C]"));
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

void CheckNOxValues() {
  int inp;
  bool changed = false;

  if(sen6x.GetNoxAlgorithm(&nox) != SEN6x_ERR_OK){
    Serial.println(F("Could not read Nox Algorithm."));
    return; 
  }
  
  while(1){
    
    disp_nox_info(changed);
    
    if (changed) inp = GetInput(1,8);
    else inp = GetInput(1,7);
    
    // cancel (or timeout)
    if (inp == 7 || inp == -100) {
      Serial.println("Cancel");
      return;
    }
    
    // update Nox algorithm
    if (inp == 8) {

      // write back
      if (sen6x.SetNoxAlgorithm(&nox) != SEN6x_ERR_OK)
        Serial.println(F("Could not update Nox algorithm."));
      else
        Serial.println(F("Updated Nox algorithm"));
    
      return;
    }

    if (inp == 1) {
      Serial.print(F("1 IndexOffset\t\t"));
      Serial.print(nox.IndexOffset);
      Serial.println(F(" range 1..250"));
      inp = GetInput(1,250);
      if (inp == -100) continue;
      changed = true;
      nox.IndexOffset = inp;
    }
    
    else if (inp == 2) {
      Serial.print(F("2 LearnTimeOffsetHours   "));
      Serial.print(nox.LearnTimeOffsetHours);
      Serial.println(F(" range 1..1000"));
      inp = GetInput(1,1000);
      if (inp == -100) continue;
      changed = true;
      nox.LearnTimeOffsetHours = inp;
    }

    else if (inp == 3) {
      Serial.print(F("3 LearnTimeGainHours   "));
      Serial.print(nox.LearnTimeGainHours);
      Serial.println(F(" range 1..1000"));
      inp = GetInput(1,1000);
      if (inp == -100) continue;
      changed = true;
      nox.LearnTimeGainHours = inp;
    }

    else if (inp == 4) {
      Serial.print(F("4 GateMaxDurationMin   "));
      Serial.print(nox.GateMaxDurationMin);
      Serial.println(F(" range 0..3000"));
      inp = GetInput(0,3000);
      if (inp == -100) continue;
      changed = true;
      nox.GateMaxDurationMin = inp;
    }

    else if (inp == 5) {
      Serial.print(F("4 stdInitial   "));
      Serial.print(nox.stdInitial);
      Serial.println(F(" range 10..5000"));
      inp = GetInput(10,5000);
      if (inp == -100) continue;
      changed = true;
      nox.stdInitial = inp;
    }
    
    else if (inp == 6) {
      Serial.print(F("6 GainFactor   "));
      Serial.print(nox.GainFactor);
      Serial.println(F(" range 1..1000"));
      inp = GetInput(1,1000);
      if (inp == -100) continue;
      changed = true;
      nox.GainFactor = inp;
    }
  }
}

void disp_nox_info(bool changed)
{
  Serial.println(F("\n************************"));
  Serial.println(F("Nox algorithm tuning"));
  Serial.println(F("************************\n"));
  
  Serial.print(F("1 IndexOffset\t\t"));
  Serial.println(nox.IndexOffset);
  Serial.print(F("2 LearnTimeOffsetHours  "));
  Serial.println(nox.LearnTimeOffsetHours);
  Serial.print(F("3 LearnTimeGainHours    "));
  Serial.println(nox.LearnTimeGainHours);
  Serial.print(F("4 GateMaxDurationMin    "));
  Serial.println(nox.GateMaxDurationMin);
  Serial.print(F("5 stdInitial\t\t"));
  Serial.println(nox.stdInitial);
  Serial.print(F("6 GainFactor\t\t"));
  Serial.println(nox.GainFactor);
  
  Serial.println(F("7 Cancel"));
  if (changed) Serial.println(F("8 Update Sensor with changes\n"));
  else Serial.println();
}

/** 
 *  display the device information
 */
void Display_Device_info()
{
  char num[32];
  bool det;
  
  // get sen6x serial number
  if (sen6x.GetSerialNumber(num,32) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read serial number. Freeze"));
    while(1);
  }
  Serial.print(F("Serial number: "));
  Serial.println(num);

  //get product name
  if (sen6x.GetProductName(num,32) != SEN6x_ERR_OK) {
    Serial.println(F("Could not read product name. Frreze."));
    while(1);
  }
  
  Serial.print(F("Product name : "));
  
  // if name is provided by the device
  if (strlen(num) > 2) {
    Serial.println(num);
  }
  else  // based on setting in sketch (starting with dot if not autodetected)
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
 * This is not well documented in the datasheet
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
 * 
 * return : 
 *  -100 NO entry was given
 *  else valid entry.
 */
int GetInput(int minop, int maxop)
{
  // flush pending input
  flush();
  
  Serial.print(F("Provide the entry-number to change. Only <enter> is return. (or wait "));
  Serial.print(INPUTDELAY);
  Serial.println(" seconds)");
  
  while(1) {
    
    String Keyb = Serial.readStringUntil(0x0a);

    if (Keyb.length() < 2) return -100;
    int in = Keyb.toInt();
    
    if(in > maxop || in < minop) {
      Serial.print(in);
      Serial.print(F(" Invalid option. Min "));
      Serial.print(minop);
      Serial.print(F(", Max "));
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
