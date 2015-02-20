/*######################################## EEPROM ########################################*/
#include <EEPROM.h>
// start reading from the first byte (address 0) of the EEPROM
int lightAddress = 0;
int tempAddress = 1;

/*######################################## Rotary Encoder ########################################*/
#include <ClickEncoder.h>
#include <TimerOne.h>
ClickEncoder *encoder;
int last = 1;
int value = 1;
void timerIsr() {
  encoder->service();
}

/*######################################## NOKIA 5110 Display ########################################*/
// It is assumed that the LCD module is connected to
// the following pins using a levelshifter to get the
// correct voltage to the module.
//      SCK  - Pin 7
//      MOSI - Pin 6
//      DC   - Pin 5
//      RST  - Pin 3
//      CS   - Pin 4
//
#include <LCD5110_Basic.h>
LCD5110 display(7,6,5,3,4);
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
const byte displayLight = 8;

/*######################################## DS3231 RTC ########################################*/
#include <DS3231.h>

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

/*######################################## Sketch Version ########################################*/
const char appName[] = "Logger";
const char appVersion[] = "0.2";
const char versionDate[] = "17.Feb\'15";

/*######################################## Global Variables ########################################*/
byte selector = 1;
bool lightOn = false;
byte temp, light;

/*######################################## Setup ########################################*/
void setup() {
  Serial.begin(9600);
  // Initialize the rtc object
  rtc.begin();
  
  pinMode(displayLight, OUTPUT);
  
  encoder = new ClickEncoder(A1, A0, A2, 4, LOW); //Rotary PIN-A, PIN-B, PIN-3, Anzahl der Steps, Knopf an 5V oder Ground
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  last = -1;
  encoder->setAccelerationEnabled(true);

  display.InitLCD();
  
  light = EEPROM.read(lightAddress);
  temp = EEPROM.read(tempAddress);
  
  splashScreen();
}

/*######################################## Main Loop ########################################*/
void loop() {
  menu(selector);
  delay(10);
}

/*######################################## Functions ########################################*/
void splashScreen() {
  display.clrScr();
  digitalWrite(displayLight, LOW);
  display.setFont(SmallFont);
  display.print(appName, CENTER, 0);
  display.print("Version:", LEFT, 16);
  display.print(appVersion, RIGHT, 16);
  display.print("Date:", LEFT, 32);
  display.print(versionDate, RIGHT, 32);
  digitalWrite(displayLight, HIGH);
}

void readEncoder() {
  value += encoder->getValue();
  
  if (value != last) {
    if (value > 4 || value < 1) {
      if (value < 1) {
        value = 4;
      }
      else {
        value = 1;
      }
    }
    last = value;
    selector = value;
  } 
}

void confirm(int conf, int from) {
  display.clrScr();
  digitalWrite(displayLight, LOW);
  display.printNumI(conf, CENTER, 0, 2, '0');
  display.print(" Saved!", CENTER, 8);
  delay(2000);
//  selector = 3;
  menu(from);
}

void screenSaver() {
  readEncoder();
  display.clrScr();
  digitalWrite(displayLight, HIGH);
  display.print(rtc.getDateStr(FORMAT_LONG, FORMAT_BIGENDIAN, '/'), CENTER, 11);
  display.print(rtc.getTimeStr(), CENTER, 29);
}

void readLog() {
  display.clrScr();
  digitalWrite(displayLight, LOW);
  display.print("Stored data", CENTER, 0);
  display.print("-----------------", CENTER, 8);
  display.print("Light: ", LEFT, 16);
  display.printNumI(light, RIGHT, 16);
  display.print("Temp.: ", LEFT, 24);
  display.printNumI(temp, RIGHT, 24);
  while (selector == 2)
  {
    readEncoder();
  }
}

void setLight() {
  display.clrScr();
  digitalWrite(displayLight, LOW);
  display.print("Click to", CENTER, 0);
  display.print("set timer.", CENTER, 8);
  while (selector == 3) {
    ClickEncoder::Button b = encoder->getButton();
    readEncoder();
    if (b == ClickEncoder::Clicked) {
      display.clrScr();
      display.print("Set Timer", CENTER, 0);
      display.print("---------", CENTER, 8);
      display.print("Lights on for: ", CENTER, 16);
      display.print("hours", CENTER, 32);
      while (true) {
        ClickEncoder::Button b = encoder->getButton();
        light += encoder->getValue();
        if (light > 24 || light < 0) {
          light = 0;
        }
        display.printNumI(light, CENTER, 24, 2, '0');
        if (b == ClickEncoder::DoubleClicked) {
          menu(selector);
          break;
        }
        if (b == ClickEncoder::Held) {
          EEPROM.write(lightAddress, light);
          confirm(light, selector);
          break;
        }
      }
    }
  }
}

void setTemp() {
  display.clrScr();
  digitalWrite(displayLight, LOW);
  display.print("Click to", CENTER, 0);
  display.print("set temp.", CENTER, 8);
  while (selector == 4) {
    ClickEncoder::Button b = encoder->getButton();
    readEncoder();
    if (b == ClickEncoder::Clicked) {
      display.clrScr();
      display.print("Set Temp.", CENTER, 0);
      display.print("---------", CENTER, 8);
      display.print("Max. temp.: ", CENTER, 16);
      display.print("celsius", CENTER, 32);
      while (true) {
        ClickEncoder::Button b = encoder->getButton();
        temp += encoder->getValue();
        if (temp > 30 || temp < 20) {
          temp = 20;
        }
        display.printNumI(temp, CENTER, 24, 2, '0');
        if (b == ClickEncoder::DoubleClicked) {
          menu(selector);
          break;
        }
        if (b == ClickEncoder::Held) {
          EEPROM.write(tempAddress, temp);
          confirm(temp, selector);
          break;
        }
      }
    }
  }
}

void menu(int switcher) {
  switch (switcher) {
    case 1:
      screenSaver();
      break;
    case 2:
      readLog();
      break;
    case 3:
      setLight();
      break;
    case 4:
      setTemp();
      break;
    default: 
      screenSaver();
  }
}
