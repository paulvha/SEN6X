/*  
 *  version DRAFT / December 2024 / paulvha
 *    
 *  This example will connect to the sen6x. It will read the serialnumber, name and different software
 *  levels. 
 *  
 *  It will display the Mass, VOC, NOC, Temperature and Humidity information.
 *  
 *  when <enter> is pressed a menu is presented to change the SEN6x temperature and/or acceleration 
 *  information. 
 *  
 *  December 2024: Not much of the additional information has been made available by Sensirion about 
 *  these settings
 *  
 *  while the temperature compensation offset and slope can be set with more accuracy than 1.00.  
 *  e.g. enter the value is 1.5. 
 *  
 *  
 *  ================================  TEST ============================
 *  Adjusting the temperature (up or down) will also impact the humidity in the opposite direction with 
 *  ~ 2 x applied change to Temperature offset. One can say if temperature is correct so is humidity, but
 *  it is not documented anywhere and the impact is times two !.
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
#define DEBUG 0

/////////////////////////////////////////////////////////////
// wait max INPUTDELAY seconds on input
/////////////////////////////////////////////////////////////
#define INPUTDELAY 10

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
uint8_t dev = Device;            // indicate connected sensor

bool header = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example2 (DRAFT): Display basic Temperature values. Press <enter> to start");

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

  if (dev == SEN60)
    Serial.println(F("Temperature settings NOT supported on connected SEN60"));
  else
    Serial.println(F("\nPress <enter> during measurement to handle Temperature parameters\n"));

  // CO2 auto calibration
  if ((dev == SEN66 || dev == SEN63) & DISABLE_ASC) {
    if (sen6x.SetCo2SelfCalibratrion(false) == SEN6x_ERR_OK) {
      Serial.println(F("CO2 ASC disabled"));
    }
    else {
     Serial.println(F("Could not disable ASC"));
    }
  }

  InitStructures();
}

void loop() 
{
  if (sen6x.CheckDataReady())  Display_val();
  
  // trigger reading adjusting Temperature compensation (pressed <enter>)
  if (Serial.available()) {
    flush();
    
    if (dev == SEN60) {
      Serial.println(F("Temperature settings NOT supported on connected SEN60"));
      delay(2000);
    }
    else
      CheckTmpValues();
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
 *  initialize structures
 *  
 *  December 2024: NO default information available
 */
void InitStructures()
{
  // There is NO SEN6x call to GET / obtain the current temperature compensation setting
  // so we start the sketch with the default values of all zero's (based on information from Sen5x)
  TempC.offset = 0;
  TempC.slope  = 0;
  TempC.time   = 0;
  TempC.slot   = 0;

  // There is NO SEN6x call to GET / obtain the current temperature Acceleration setting
  // the correct default values to use are not known
  TempAcc.K = 0;
  TempAcc.P = 0;
  TempAcc.T1 = 0;
  TempAcc.T2 = 0;
}

/**
 * set the temperature value
 */
void CheckTmpValues() {
  int inp, sel;
  bool TC_change = false;
  bool TA_change = false;
  float changed[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,};
  struct sen6x_tmp_comp TempCT;
  struct sen6x_RHT_comp TempAccT;
  float f;

  // there are NO calls to GET the temperature offset nor acceleration setting
  // save start position before changing
  TempCT.offset = TempC.offset;
  TempCT.slope  = TempC.slope;
  TempCT.time = TempC.time;
  TempCT.slot = TempC.slot;

  TempAccT.K = TempAcc.K;
  TempAccT.P = TempAcc.P;
  TempAccT.T1 = TempAcc.T1;
  TempAccT.T2 = TempAcc.T2;

  header = true;
  
  while(1) {
    
    disp_tmp_info(changed);

    inp = GetInput(1,12,&f);

    // cancel (or timeout)
    if (inp == 10 || inp == -100) {
      Serial.println(F("Cancel"));
      
      // restore start position if any change was made
      if (TC_change) {
        TempC.offset = TempCT.offset;
        TempC.slope  = TempCT.slope;
        TempC.time = TempCT.time;
        TempC.slot = TempCT.slot; 
      }

      if (TA_change) {
        TempAcc.K = TempAccT.K;
        TempAcc.P = TempAccT.P;
        TempAcc.T1 = TempAccT.T1;
        TempAcc.T2 = TempAccT.T2;
      }
      return;
    }
    
    sel = inp;
    
    // update values and return
    if (inp == 9) {
    
      // write back
      if (TC_change) {
        if (sen6x.SetTmpComp(&TempC) != SEN6x_ERR_OK){
          Serial.println(F("Could not update temperature compensation."));
          
          // restore start position
          TempC.offset = TempCT.offset;
          TempC.slope  = TempCT.slope;
          TempC.time = TempCT.time;
          TempC.slot = TempCT.slot; 
        }
        else {
          Serial.println(F("Updated temperature compensation."));
          TempC_default = false;      // indicated default values have been updated
        }
      }

      if (TA_change) {
        if (sen6x.SetTempAccelMode(&TempAcc) != SEN6x_ERR_OK) {
          Serial.println(F("Could not update Acceleration mode."));

          // restore start position
          TempAcc.K = TempAccT.K;
          TempAcc.P = TempAccT.P;
          TempAcc.T1 = TempAccT.T1;
          TempAcc.T2 = TempAccT.T2;
        }
        else {
          Serial.println(F("Updated Acceleration mode."));
          TempAcc_default = false;      // indicated default values have been updated
        }
      }
     
      return;
    }

    else if (inp == 1) {
      Serial.print(F("1 Temperature offset\t"));
      Serial.print(TempC.offset);
      Serial.println("  range -10..10"); // articifial limit as no definition available
      Serial.println(F("Provide offset to apply to the ORIGINAL result\n"));
      inp = GetInput(-10,10,&f);
      Serial.println(f);
      if (inp == -100) continue;
      changed[sel-1] = TempC.offset;
      TempC.offset = f;
      TC_change = true;
    }

    else if (inp == 2) {
      Serial.print(F("2 Temperature slope\t"));
      Serial.print(TempC.slope);
      Serial.println(" range 0..50"); // articifial limit as no definition available
      Serial.println(F("Provide slope to apply to the ORIGINAL result\n"));
      inp = GetInput(0,50,&f);
      if (inp == -100) continue;
      changed[sel-1] = TempC.slope;
      TempC.slope = f;
      TC_change = true;
    }

    else if (inp == 3) {
      Serial.print(F("3 Temperature time (seconds)\t"));
      Serial.print(TempC.time);
      Serial.println(" range 0..10"); // articifial limit as no definition available
      inp = GetInput(0,10,&f);
      if (inp == -100) continue;
      changed[sel-1] = TempC.time;
      TempC.time = inp;
      TC_change = true;
    }
    
    else if (inp == 4) {
      Serial.print(F("4 slot\t"));
      Serial.print(TempC.slot);
      Serial.println(" range 0..4");
      inp = GetInput(0,4,&f);
      if (inp == -100) continue;
      changed[sel-1] = TempC.slot;
      TempC.slot = inp;
      TC_change = true;
   }
    
    else if (inp == 5) {
      Serial.print(F("5 constant K"));
      Serial.print(TempAcc.K);
      Serial.println(" range 0..2");
      inp = GetInput(0,2,&f);
      if (inp == -100) continue;
      changed[sel-1] = TempAcc.K;
      TempAcc.K = inp;
      TA_change = true;
    }

    else if (inp == 6) {
      Serial.print(F("6 constant P"));
      Serial.print(TempAcc.P);
      Serial.println(" range 0..2");
      inp = GetInput(0,2,&f);
      if (inp == -100) continue;
      changed[sel-1] = TempAcc.P;
      TempAcc.P = inp;
      TA_change = true;
    }

    else if (inp == 7) {
      Serial.print(F("7 constant T1"));
      Serial.print(TempAcc.T1);
      Serial.println(" range 0..2");
      inp = GetInput(0,2,&f);
      if (inp == -100) continue;
      changed[sel-1] = TempAcc.T1;
      TempAcc.T1 = inp;
      TA_change = true;
    }

    else if (inp == 8) {
      Serial.print(F("8 constant T2"));
      Serial.print(TempAcc.K);
      Serial.println(" range 0..2");
      inp = GetInput(0,2,&f);
      if (inp == -100) continue;
      changed[sel-1] = TempAcc.T2;
      TempAcc.T2 = inp;
      TA_change = true;
    }
    
    else if (inp == 11) {  // change celsius / Fahrenheit
      if (Type_temp == 1) Type_temp = 0;
      else Type_temp = 1;
      header = true;
      return;
    }

    else if (inp == 12) {  // activate heather
      if (sen6x.ActivateSHTHeater()) {
        Serial.println("Heater activated. Wait 20 seconds");
        for (int j = 1 ; j < 21 ; j++) {
          delay(1000);
          Serial.print(j);
          Serial.print(" ");
        } 
      header = true;
      Serial.println();
      return;
      }
      else {
        Serial.println(F("Could not activate heater"));
      }
    }
  }
}
/**
 * Display values. 
 * @param ch :
 * will hold the previous value if changed
 */
void disp_tmp_info(float *ch)
{
  int i = 0;
  bool cha = false;
  
  Serial.println(F("\n************** Temperature parameters **********************"));
  Serial.print(F("\nTemperature compensation "));
  if (TempC_default) Serial.println(F("(DEFAULT setting)"));
  
  Serial.print(F("\n1  Temperature offset\t "));
  Serial.print(TempC.offset);
  if(ch[i] != 0xff) { Serial.print(F("\twas: ")); Serial.println(ch[i]); cha = true;}
  else Serial.println();
  
  Serial.print(F("2  Temperature slope\t "));
  Serial.print(TempC.slope);
  if(ch[++i] != 0xff) { Serial.print(F("\twas: ")); Serial.println(ch[i]);cha = true;}
  else Serial.println();
  
  Serial.print(F("3  Temperature time\t "));
  Serial.print(TempC.time);
  if(ch[++i] != 0xff) { Serial.print(F("\twas: ")); Serial.println(ch[i],0);cha = true;}
  else Serial.println();
  
  Serial.print(F("4  Slot\t\t\t "));
  Serial.print(TempC.slot);
  if(ch[++i] != 0xff) { Serial.print(F("\twas: ")); Serial.println(ch[i],0);cha = true;}
  else Serial.println();
  
  Serial.print(F("\nTemperature acceleration "));
  if (TempAcc_default) Serial.println(F("(DEFAULT setting)"));
  Serial.print(F("\n5  filter constant K\t "));
  Serial.print(TempAcc.K);
  if(ch[++i] != 0xff) { Serial.print(F("\twas: ")); Serial.println(ch[i]);cha = true;}
  else Serial.println();
  
  Serial.print(F("6  filter constant P\t "));
  Serial.print(TempAcc.P);
  if(ch[++i] != 0xff) { Serial.print(F("\twas: ")); Serial.println(ch[i]);cha = true;}
  else Serial.println();
  
  Serial.print(F("7  Time constant T1\t "));
  Serial.print(TempAcc.T1);
  if(ch[++i] != 0xff) { Serial.print(F("\twas: ")); Serial.println(ch[i]);cha = true;}
  else Serial.println();
  
  Serial.print(F("8  Time constant T2\t "));
  Serial.print(TempAcc.T2);
  if(ch[++i] != 0xff) { Serial.print(F("\twas: ")); Serial.println(ch[i]);cha = true;}
  Serial.println("\n");

  if (cha)
    Serial.println(F("9  UPDATE Sen6x values: MUST be done to apply above changes !!\n"));
    
  Serial.println(F("10 Cancel & discard above changes"));

  Serial.print(F("\n11 Switch to temperature display "));
  if (Type_temp == 1)  Serial.println(F("Fahrenheit (applied immediately)"));
  else Serial.println(F("Celsius (applied immediately)"));

  Serial.println(F("12 Activate inbuilt heater. Takes 20 seconds. (applied immediately)\n"));
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
