/*  
 *  version DRAFT / December 2024 / paulvha
 *    
 *  This example will connect to the sen6x. It will read the serialnumber, name and different software
 *  levels. It will also connect to a BME280 sensor
 *  
 *  It will display the Mass, VOC, NOC, Temperature and Humidity information, as well as the BME280 Humidity,
 *  Temperature, Pressure and Altitude
 *  
 *  when <enter> is pressed a menu is presented to change some of the SEN6x temperature and/or acceleration 
 *  information. At the time for written (December 2024) not much of the additional information has been made
 *  available bySensirion.
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
 *  ===============  BME280 sensor =========================
 *  BME280
 *  VCC  ------ VCC  (5V)  I2C Address: 0x77 (Default)
 *  GND  ------ GND
 *  SCK  ------ SCL
 *  SDI  ------ SDA
 *  
 *  ADAFRUIT  (connected to 5V Wire !!!!)
 *  BME280
 *  VIN ------ VCC  (5V)
 *  3V3        output (do not use)
 *  GND ------ GND
 *  SCK ------ SCL
 *  SDO        if connected to GND address is 0x76 instead of 0x77  
 *  SDI ------ SDA
 *  CS         do not connect
 * 
 *  Make sure to have pull-up resistors for SCL and SDA
 * ---------------------------------------------------------------
 *  ADAFRUIT  (connected to 3V3 Wire1 on Arduino R4 Wifi !!!)
 *  BME280
 *  VIN        do not connect 
 *  3V3 ------ VCC  (3V3)
 *  GND ------ GND
 *  SCK ------ SCL
 *  SDO        if connected to GND address is 0x76 instead of 0x77  
 *  SDI ------ SDA
 *  CS         do not connect
 *  
 *  Wire1 on UNOR4 Wifi already has pull-up resistors
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
#include "SparkFunBME280.h"

///////////////////////////////////////////////////////////////
//                          BME280                           //
///////////////////////////////////////////////////////////////
/* define the BME280 address.
 * Use if address jumper is closed (SDO - GND) : 0x76.*/
#define I2CADDR 0x77

/* Define default reading in Fahrenheit or Celsius
 *  1 = Celsius
 *  0 = Fahrenheit */
#define TEMP_TYPE 1

/* define default display alitude in Meters or Foot
 *  1 = Meters
 *  0 = Foot */
#define BME_HIGHT 1

/////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN65, SEN66 or SEN68 */
////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface */
////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1
#define WIRE_BME280 Wire

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
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

SEN6x sen6x;
BME280 mySensor; 

struct sen6x_values val;
struct sen6x_tmp_comp TempC;
struct sen6x_RHT_comp TempAcc;
bool TempC_default = true;        // is information in structure still default
bool TempAcc_default = true;      // is information in structure still default

bool detect_BME280 = false;
int Type_temp = TEMP_TYPE;
int Type_hight = BME_HIGHT;

bool header = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example2 (DRAFT): Display basic values & BME280 press <enter> to start");

  Serial.println(F("Trying to connect."));

  //********************** BME280 **************************************************

  WIRE_BME280.begin();
  
  // set BME280 I2C address.
  mySensor.setI2CAddress(I2CADDR);

  if (mySensor.beginI2C(WIRE_BME280) == false) // Begin communication over I2C
    Serial.println(F("The BME280 did not respond. Please check wiring."));
  else
  {
    detect_BME280 = true;
    Serial.println(F("Detected BME280."));
  }

  ///////////////////////// SEN6x ///////////////////////////////////
  
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

  if (! sen6x.start()) {
    Serial.println(F("could not Start sen6x."));
    while(1);
  }

  Serial.setTimeout(INPUTDELAY * 1000);

  // there is NO SEN6x call to GET / obtain the current temperature compensation setting
  // so we start the sketch with the default values, (December 2024: based on information from Sen5x)
  TempC.offset = 0;
  TempC.slope  = 0;
  TempC.time   = 0;
  TempC.slot   = 0;


  // there is NO SEN6x call to GET / obtain the current temperature Acceleration setting
  // the default values to use are not known (December 2024: documentation is not available)
  TempAcc.K = 0;
  TempAcc.P = 0;
  TempAcc.T1 = 0;
  TempAcc.T2 = 0;
}

void loop() {

  Display_val();
  
  // trigger reading adjusting Temperature compensation (pressed <enter>)
  if (Serial.available()) {
    flush();
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
    Serial.print(F("==================================== SEN66 ================================   "));
    if(detect_BME280) Serial.print(F("================= BME280 =============="));
    
    Serial.print(F("\n-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print("  Humidity:  Temperature:");
      if (Device == SEN66 || Device == SEN63) Serial.print(F(" CO2:"));
      if (Device == SEN68) Serial.print(F(" HCHO:"));
    }
    if(detect_BME280) Serial.print(F("   Humidity  Temperature Pressure Altitude"));

    
    Serial.print(F("\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      
      if (Type_temp) Serial.print(F("   [%]\t[*C]\t     "));
      else Serial.print(F("   [%]\t[*F]\t   "));
      
      if (Device == SEN66 || Device == SEN63) Serial.print(F("[ppm]"));
      if (Device == SEN68) Serial.print(F("[ppb]"));
    }
    
    if(detect_BME280) {
      
      if (Type_temp) Serial.print(F("\t[%]      [*C]"));
      else Serial.print(F("\t[%]      [*F]"));

      Serial.print(F("     [hPa]\t   "));

      if (Type_hight) Serial.print(F("Meter\t"));
      else Serial.print(F("Foot\t"));
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

  if(detect_BME280) {
    Serial.print(F("\t"));
    Serial.print(mySensor.readFloatHumidity(), 1);
    Serial.print(F("\t  "));
    
    if (Type_temp) Serial.print(mySensor.readTempC(), 2);
    else Serial.print(mySensor.readTempF(), 2);
    Serial.print(F("\t    "));
    
    Serial.print(mySensor.readFloatPressure()/100, 0);
    Serial.print(F("    "));

    if (Type_hight) Serial.print(mySensor.readFloatAltitudeMeters(), 1);
    else Serial.print(mySensor.readFloatAltitudeFeet(), 1);
  }

  Serial.print(F("\n"));
}

/**
 * set the temperature value
 */
void CheckTmpValues() {
  int inp;
  bool TC_change = false;
  bool TA_change = false;
  struct sen6x_tmp_comp TempCT;
  struct sen6x_RHT_comp TempAccT;

  // there are NO calls to GET the temperature offset nor acceleration setting
  // save start position
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
    
    disp_tmp_info();

    inp = GetInput(1,12);

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
    
    Serial.println(inp);
  
    // update values
    if (inp == 9) {
    
      // write back
      if (TC_change) {
        if (sen6x.SetTmpComp(&TempC) != SEN6x_ERR_OK){
          Serial.println(F("Could not update temperature compensation."));
          TempC_default = false;
          
          // restore start position
          TempC.offset = TempCT.offset;
          TempC.slope  = TempCT.slope;
          TempC.time = TempCT.time;
          TempC.slot = TempCT.slot; 
        }
        else
          Serial.println(F("Updated temperature compensation."));
      }

      if (TA_change) {
        if (sen6x.SetTempAccelMode(&TempAcc) != SEN6x_ERR_OK) {
          Serial.println(F("Could not update Acceleration mode."));
          TempAcc_default = false;

          // restore start position
          TempAcc.K = TempAccT.K;
          TempAcc.P = TempAccT.P;
          TempAcc.T1 = TempAccT.T1;
          TempAcc.T2 = TempAccT.T2;
        }
        else
          Serial.println(F("Updated Acceleration mode."));
      }
     
      return;
    }

    else if (inp == 1) {
      Serial.print(F("1 Temperature offset\t"));
      Serial.print(TempC.offset);
      Serial.println("  range -10..10"); // articifial limit
      inp = GetInput(-10,10);
      if (inp == -100) continue;
      TempC.offset = inp;
      TC_change = true;
    }

    else if (inp == 2) {
      Serial.print(F("2 Temperature slope\t"));
      Serial.print(TempC.slope);
      Serial.println(" range 0..50"); // articifial limit
      inp = GetInput(0,50);
      if (inp == -100) continue;
      TempC.slope = inp;
      TC_change = true;
    }

    else if (inp == 3) {
      Serial.print(F("3 Temperature time (seconds)\t"));
      Serial.print(TempC.time);
      Serial.println(" range 0..10"); // articifial limit
      inp = GetInput(0,12);
      if (inp == -100) continue;
      TempC.time = inp;
      TC_change = true;
    }
    
    else if (inp == 4) {
      Serial.print(F("4 slot\t"));
      Serial.print(TempC.slot);
      Serial.println(" range 1..4");
      inp = GetInput(1,4);
      if (inp == -100) continue;
      TempC.slot = inp;
      TC_change = true;
   }
    
    else if (inp == 5) {
      Serial.print(F("5 constant K"));
      Serial.print(TempAcc.K);
      Serial.println(" range 0..2");
      inp = GetInput(0,2);
      if (inp == -100) continue;
      TempAcc.K = inp;
      TA_change = true;
    }

    else if (inp == 6) {
      Serial.print(F("6 constant P"));
      Serial.print(TempAcc.P);
      Serial.println(" range 0..2");
      inp = GetInput(0,2);
      if (inp == -100) continue;
      TempAcc.P = inp;
      TA_change = true;
    }

    else if (inp == 7) {
      Serial.print(F("7 constant T1"));
      Serial.print(TempAcc.T1);
      Serial.println(" range 0..2");
      inp = GetInput(0,2);
      if (inp == -100) continue;
      TempAcc.T1 = inp;
      TA_change = true;
    }

    else if (inp == 8) {
      Serial.print(F("8 constant T2"));
      Serial.print(TempAcc.K);
      Serial.println(" range 0..2");
      inp = GetInput(0,2);
      if (inp == -100) continue;
      TempAcc.T2 = inp;
      TA_change = true;
    }
    
    else if (inp ==11) {
      if (Type_temp == 1) Type_temp = 0;
      else Type_temp = 1;
      header = true;
    }

    else if (inp == 12) {
      if (Type_hight == 1) Type_hight = 0;
      else Type_hight = 1;
      header = true;
    }
  }
}

void disp_tmp_info()
{
  Serial.println(F("\n***************************************"));
  Serial.print(F("\nTemperature compensation "));
  if (TempC_default) Serial.println(F("(DEFAULT setting)"));
  
  Serial.print(F("\n1  Temperature offset\t "));
  Serial.println(TempC.offset);
  Serial.print(F("2  Temperature slope\t "));
  Serial.println(TempC.slope);
  Serial.print(F("3  Temperature time\t "));
  Serial.println(TempC.time);
  Serial.print(F("4  Slot\t\t\t "));
  Serial.println(TempC.slot);
  
  Serial.print(F("\nTemperature acceleration "));
  if (TempAcc_default) Serial.println(F("(DEFAULT setting)"));
  Serial.print(F("\n5  filter constant K\t "));
  Serial.println(TempAcc.K);
  Serial.print(F("6  filter constant P\t "));
  Serial.println(TempAcc.P);
  Serial.print(F("7  Time constant T1\t "));
  Serial.println(TempAcc.T1);
  Serial.print(F("8  Time constant T2\t "));
  Serial.println(TempAcc.T2);
  
  Serial.println(F("\n9  Update Sen6x values (MUST be done to enable above changes)"));
  Serial.println(F("10 Cancel & disgard above changes"));

  Serial.println(F("\nFollowing changes are applied immediately:"));
  Serial.print(F("11 Switch to temperature display "));
  if (Type_temp == 1)  Serial.println(F("Fahrenheit"));
  else Serial.println(F("Celcius"));
  
  Serial.print(F("12 Switch to height display in "));
  if (Type_hight == 1)  Serial.println(F("foot\n"));
  else Serial.println(F("meters\n"));
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
  
  Serial.print(F("Provide the number. Only <enter> is return. (or wait "));
  Serial.print(INPUTDELAY);
  Serial.println(F(" seconds)"));
  
  while(1) {
    
    String Keyb = Serial.readStringUntil(0x0a);

    if (Keyb.length() < 2) return -100;
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