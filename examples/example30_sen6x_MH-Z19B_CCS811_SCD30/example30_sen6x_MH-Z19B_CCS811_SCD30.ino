/*  
 *  version DRAFT / January 2025 / paulvha
 *    
 *  This example will connect to the SEN6x (wire1), MH-Z19B (Serial1), an CCS811 (Wire) and SCD30 (Wire).
 * 
 *  This sketch will display the Mass, VOC, NOC, Temperature and humidity information, if available from 
 *  the SEN6x. Also the MH-Z19B CO2 and Temp, the VOC and CO2 from the CCS811 as well as the CO2, Humidity
 *  and Temperature from the SCD30. 
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
 *  
 *  SEN6X pin     UNOR4
 *                Qwiic (Wire1)
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
 *  
 *  -----------------
 *  |               |
 *  |               | 
 *  |7 5 6 4 3 2 1  |
 *  -----------------
 *  
 *  MH-Z19B             UNOR4 Wifi
 *                      Serial1
 *  1 Analog out
 *  2 NC
 *  3 GND  ------------- GND
 *  4 Vin  ------------- 5V
 *  5 TX   ------------- RX1
 *  6 RX   ------------- TX1
 *  7 NC
 *  
 * ////////////////////// CCS811 /////////////////////////
 *  
 *  -----------------
 *  |               |
 *  |               | 
 *  |1 2 3 4 5 6 7 8|
 *  -----------------
 *  
 *  CCS811              UNOR4 Wifi
 *  1 VIN  ------------- +5V
 *  2 3v3
 *  3 GND  ------------- GND
 *  4 SDA  ------------- SDA
 *  5 SCL  ------------- SCL
 *  6 WAKE ------------- GND
 *  7 RST
 *  8 INT
 *  
 *  Pull-up resistors to 5V on SCL and SDA
 * 
 *  OR ...in case you only have a 3v3 device then use a level converter
 *  
 *  CCS811             Lvl convert     UNOR4 Wifi (Wire)
 *                    | HV        |--- +5V
 *    VIN  -----------| LV        |--- +3.3V
 *                    |           |
 *    GND  -----------| GND       |--- GND
 *    SDA  -----------| LV <-> HV |--- SDA
 *    SCL  -----------| LV <-> HV |--- SCL
 *    WAKE --------------------------- GND
 *    RST
 *    INT 
 *  
 *  During test the CCS-811 would jump high by even touching the wires (especially SCL-line). I suspect
 *  that because I used a cheap board (CJMCU8118) the lines are quickly to long. Hence I connected with 
 *  a level converter the CCS-811. It then worked stable.   
 *  
 *  ///////////////////// SCD30 /////////////////////////
 *  --------------------------
 *  | 1 2 3 4 5 6 7      (S) |
 *  |  _________________     |    
 *  | /                 \====|  
 *  | ||||=====         [||  |
 *  | \_________________/====|
 *  |________________________|
 *  
 *  SCD30             UNOR4 
 *  1 VDD  ---------- 5V
 *  2 GND  ---------- GND
 *  3 SCL  ---------- SCL
 *  4 SDA  ---------- SDA
 *  5 RDY NC
 *  6 PWM NC
 *  7 SEL NC or GND (select I2C)
 *  
 *  Pull-up resistors to 5V on SCL and SDA
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
#include "Adafruit_CCS811.h" //Click here to get the library: http://librarymanager/All#Adafruit_CCS8
#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30

/////////////////////////////////////////////////////////////
/* define the SEN6x sensor connected
 * valid values, SEN60, SEN63, SEN65, SEN66 or SEN68 */
 ////////////////////////////////////////////////////////////
const SEN6x_device Device = SEN66;

/////////////////////////////////////////////////////////////
/* define which Wire interface */
 ////////////////////////////////////////////////////////////
#define WIRE_sen6x Wire1
#define WIRE_ccs811 Wire
#define WIRE_SCD30 Wire

/////////////////////////////////////////////////////////////
// define communication address to use for CCS811
/////////////////////////////////////////////////////////////
#define CCS811_ADDR 0x5A // Default I2C Address
//#define CCS811_ADDR 0x5B //Alternate I2C Address

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
Adafruit_CCS811 mySensor;
SCD30 airSensor;

struct sen6x_values val;

bool header = true;
uint8_t dev = Device;            // indicate connected sensor
bool det;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example30 (DRAFT): Display SEN6x,MH-Z19B, CCS811 and SCD30. press <enter> to start");

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

  /////////////////////////// MH-Z19B  //////////////////////

  mySerial.begin(9600); 
  myMHZ19.begin(mySerial);

  myMHZ19.autoCalibration(false);  // Turn auto calibration OFF

  /////////////////////////// SCD30  //////////////////////
  WIRE_SCD30.begin();

  if ( ! airSensor.begin() ) {
    Serial.println("Air sensor SCD30 not detected. Please check wiring. Freezing...");
    while (1);
  }
  else {
    Serial.println("Connected SCD30.");
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

  // get connected device
  dev = sen6x.GetDevice(&det);
  
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
    Serial.println(F("could not start sen6x."));
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
      Serial.println(F("========================== SEN6x ==================================== === MH-Z19B ===   === CCS811 ==== ======= SCD30 =========="));  
    
    Serial.print(F("-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print(F("  Humidity:  Temp:"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("   CO2:"));
      if (Device == SEN68) Serial.print(F("  HCHO:"));
    }
    
    // MH-Z19B                     CCS811         SCD30                  SEN6x
    Serial.print(F("\tCO2:\t Temp:\tCO2:    VOC:\tCO2:   Humidity: Temp:\n     Concentration [μg/m3]"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("      index  index"));
      Serial.print(F("    [%]      [*C]"));
      if (Device == SEN66 || Device == SEN63) Serial.print(F("\t[ppm]"));
      if (Device == SEN68) Serial.print(F("\t[ppb]"));
    }

    //                 MH-Z19B    CCS811        SCD30
    Serial.print(F("\t[ppm]\t *C\t[ppm]\t[ppb]\t[ppm]\t [%]\t [*C]"));
    
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
  Serial.print(Temp);  

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
  
  // display SCD30 information
  if (airSensor.dataAvailable()) {
    Serial.print(F("\t"));
    Serial.print(airSensor.getCO2());
    Serial.print(F("\t"));
    Serial.print(airSensor.getHumidity(), 1);
    Serial.print(F("\t"));
    Serial.print(airSensor.getTemperature(), 1);
  }
  else {
    Serial.print(F("\t--NOT AVAILABLE--"));
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
