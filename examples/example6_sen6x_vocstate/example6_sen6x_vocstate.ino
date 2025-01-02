/*  
 *  version  DRAFT/ December 2024 / paulvha
 * 
 *  This example will read the MASS / VOC values and save/restore VOC algorithm STATE.
 *  
 *  VOC algorithm state values  (source datasheet sen6x)
 *  Allows backup of the VOC algorithm state to resume operation after a power cycle or device reset,
 *  skipping initial learning phase. By default, the VOC Engine is reset, and the algorithm state is 
 *  retained if a measurement is stopped and started again. 
 *  If the VOC algorithm state shall be reset, a device reset, or a power cycle can be executed.
 * 
 *  Gets the current VOC algorithm state. This data can be used to restore the state with the 
 *  Set VOC Algorithm State command after a short power cycle or device reset.
 *  
 *  Pressing enter during the measurement will display the current VOC-values and provide a menu to
 *  save, display and/or restore previous saved parameters. The save parameters are not retained after 
 *  a reset of the sketch, it is only available to demonstrate and learn.
 *  
 *  There is NO information available about the meaning of the 8 different bytes and thus there is NO 
 *  edit option in the menu.
 *  
 *  Readings
 *  
 *   < 60 seconds voc = 0   0x00 0x00 0x00 0x00 0x00 0x32 0x00 0x00  
 *  later..       voc = 103 0x2F 0x91 0xAE 0x21 0x00 0x31 0xF9 0x0B  
 *  
 *  After writting back an earlier saved VocState is takes ~40 seconds to get results. But the result 
 *  start much higher then we starting after a reset.
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
 *  sen6x pin     ESP32
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
 *  sen6x pin     ATMEGA
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
 *  sen6x pin     UNO
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
 *  sen6x pin     Artemis
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

struct VocState {
  uint8_t VocState[VOC_ALO_SIZE];   // captured VocState
  unsigned long mtime;              // time when captured
  unsigned long wtime;              // time last time written
  float   VOC;                      // Voc value at time of saving
};
#define MAXSTATES 10               // can hold 10 captures
VocState Vocstates[MAXSTATES];

bool header = true;
bool DisplayVocHeader= true;
uint8_t dev = Device;            // indicate connected sensor

void setup() {
  
  Serial.begin(115200);
  while (!Serial) delay(100);

  serialTrigger((char *) "SEN6x-Example6 (DRAFT):  Display basic values and get/set VOC Algorithm State. press <enter> to start");

  Serial.println(F("Trying to connect."));

  // set library debug level
  sen6x.EnableDebugging(DEBUG);

  WIRE_sen6x.begin();
  
  // Begin communication channel;
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

  Serial.setTimeout(INPUTDELAY * 1000);

  // CO2 auto calibration
  if ((dev == SEN66 || dev == SEN63) & DISABLE_ASC) {
    if (sen6x.SetCo2SelfCalibratrion(false) == SEN6x_ERR_OK) {
      Serial.println(F("CO2 ASC disabled"));
    }
    else {
     Serial.println(F("Could not disable ASC"));
    }
  }

  InitVocStates();

  if (! sen6x.start()) {
    Serial.println(F("Could not Start sen6x."));
    while(1);
  }
  if (dev == SEN60 || dev == SEN63)
    Serial.println(F("\nVOC state NOT supported on connected device type"));
  else
    Serial.println(F("\nPress <enter> during measurement to handle VOC state\n"));
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
    CheckVocValues();
    header = true;
  }
  else
    delay(1000);
}

/**
 * display the values
 */
void Display_val()
{
  if (header) {
    Serial.print(F("\n-------------Mass -----------"));
    
    if (Device != SEN60) {
      if (Device != SEN63) Serial.print(F("    VOC:  NOX:"));
      Serial.print("  Humidity:  Temperature:");
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

/**
 * initialise VocState-slots
 */
void InitVocStates() {
  for (int i = 0 ; i < MAXSTATES ; i ++){
    Vocstates[i].mtime = 0;
    Vocstates[i].wtime = 0;
  }
}

/** 
 *  find the first empty slot in Vocstate
 *  
 *  @return :
 *  0xff NO empty slot
 *  else empty slot
 */
int GetEmptySlot()
{
  for (int i =0 ; i < MAXSTATES ; i ++) {
    if (Vocstates[i].mtime == 0x0) return i;
  }

  return (0xff);
}

/**
 * Display previous saved Vocstates and ask for input
 * 
 * @param ask : true is ask, false is only display
 * 
 * @return :
 * 0xff : NO slot with previous Vocstates
 * 0xfe : cancel / display ready
 * else selected slot
 */
int GetSlotToSave(bool ask)
{
  int i, inp;
  int FoundLast = 0xff;

  DisplayVocHeader= true; // enable header
  
  for (i = 0 ; i < MAXSTATES ; i ++) {
    if(Vocstates[i].mtime != 0) {  // if not empty
      DisplayVocState(i);
      FoundLast = i;
    }
  }

  if (! FoundLast == 0xff) return (0xff);
  
  if (!ask) return(0xfe);          // only dislay do not ask entry
  
  Serial.print(FoundLast+1);
  Serial.println(" is cancel. ");
  
  while(1) {
    inp = GetInput(0,FoundLast+1);
  
    // cancel (or timeout)
    if (inp == (FoundLast+1) || inp == -100) {
      Serial.println("Cancel");
      return(0xfe);
    }

    if (Vocstates[inp].mtime != 0) return (inp); 

    Serial.print("Invalid option ");
    Serial.println(inp);
  }
}

/**
 * display a VOCstate entry
 */
void DisplayVocState(int i) 
{
  if (DisplayVocHeader) 
  {
    Serial.println(F("\nSlot\tCaptured(s)\tVoc at save\t VocState\t\t\t\tLast Written(s)"));
    DisplayVocHeader = false;
  }

  Serial.print(i);
  Serial.print("\t");
  Serial.print(Vocstates[i].mtime/1000);
  Serial.print("\t\t");
  Serial.print (Vocstates[i].VOC); 
  Serial.print("\t\t");
  for (int j = 0 ; j <  VOC_ALO_SIZE; j++) {
    Serial.print(" 0x");
    if (Vocstates[i].VocState[j] < 0x10) Serial.print("0");
    Serial.print(Vocstates[i].VocState[j],HEX);
  }
  Serial.print("\t");
  if (Vocstates[i].wtime != 0) Serial.print(Vocstates[i].wtime/1000);
  Serial.println();
}

/**
 * Main menu for VOC state handling
 */
void CheckVocValues() {
  int inp, offSet;
  bool avail = false;
 
  while(1){

    // check for previous saved
    for (int i = 0 ; i < MAXSTATES ; i ++){
      if (Vocstates[i].mtime != 0x0) avail = true;
    }
    
    disp_voc_info(avail);
    
    if (avail) inp = GetInput(1,5);
    else inp = GetInput(1,2);

    // cancel (or timeout)
    if (inp == 1 || inp == -100) {
      Serial.println("Cancel");
      return;
    }
   
    if (inp == 2) {     // save current values
      offSet = GetEmptySlot();
      
      if (offSet == 0xff)  {
        Serial.println(F("Could not find empty slot."));
        continue;
      }
      
      if (sen6x.GetVocAlgorithmState(Vocstates[offSet].VocState, VOC_ALO_SIZE) != SEN6x_ERR_OK){
        Serial.println(F("Could not read VOC algorithm state."));
        return;
      }
      Vocstates[offSet].mtime = millis();
      Vocstates[offSet].VOC = val.VOC;
      Serial.print(F("\nRead VOC algorithm state in slot "));
      Serial.println(offSet);
      avail = true;
    }

    else if (inp == 3) {    // display slots

      offSet = GetSlotToSave(false);
      
      if (offSet == 0xff) {
        Serial.println(F("Could not find slots."));
      }
      
    }
    
    else if (inp == 4) {    // write Vocstate to device
      offSet = GetSlotToSave(true); // display available & ask
      
      if (offSet == 0xff) {
        Serial.println(F("Could not find slot to write back."));
        continue;
      }
      
      if (offSet == 0xfe)  continue;  // cancel

      // write back
      if (sen6x.SetVocAlgorithmState(Vocstates[offSet].VocState, VOC_ALO_SIZE) != SEN6x_ERR_OK)
        Serial.println(F("could not update VOC algorithm state."));
      else {
        Serial.print(F("\nUpdated VOC algorithm state from slot "));
        Serial.println(offSet);
        Vocstates[offSet].wtime = millis();
      }
      return;
    }
    else if (inp == 5) { // delete previous saved Vocstate
      offSet = GetSlotToSave(true); // display available
      
      if (offSet == 0xff) {
        Serial.println(F("Could not find slot to delete."));
        continue;
      }
      
      if (offSet == 0xfe)  continue;  // cancel

      Vocstates[offSet].mtime = 0x0;
      Vocstates[offSet].wtime = 0x0;
    }
  }
}

void disp_voc_info(bool avail)
{
  Serial.println(F("\n************************"));
  Serial.println(F("VOC algorithm STATE "));
  Serial.println(F("************************\n"));

  Serial.println(F("1 Cancel"));
  Serial.println(F("2 Read new algorithm state"));

  if ( !avail) return;
  Serial.println(F("3 Display the saved algorithm states"));
  Serial.println(F("4 Write an earlier saved algorithm state"));
  Serial.println(F("5 Delete an earlier saved algorithm state"));
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
  
  Serial.print(F("Provide the entry-number. Only <enter> is return. (or wait "));
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
    delay(100);
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
