/*######################################## Sketch Version ########################################*/
const char appName[] = "Logger";
const char appVersion[] = "0.5";
const char versionDate[] = "25.Feb\'15";
const byte showSplashFor = 30;

/*######################################## Global Variables ########################################*/
String menuItem[] = {"Light", "Temperature", "Humidity", "Water"};
const byte items = sizeof(menuItem)/sizeof(menuItem[0]);
int selector = items;
int itemValue = 0;
long check = 0;
long lastInput = 0;
unsigned int displayTimeout = 15000;

/*######################################## EEPROM ########################################*/
#include <EEPROM.h>

/*######################################## Rotary Encoder ########################################*/
#include <ClickEncoder.h>
#include <TimerOne.h>
ClickEncoder *encoder;
int last = 1;
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
extern uint8_t BigNumbers[];
const byte displayLight = 8;
bool displayLEDisOn = true;

/*######################################## DS3231 RTC ########################################*/
#include <DS3231.h>

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

/*######################################## Setup ########################################*/
void setup() {
  // Initialize the rtc object
  rtc.begin();
  
  pinMode(displayLight, OUTPUT);
  
  encoder = new ClickEncoder(A1, A0, A2, 4, LOW); //Rotary PIN-A, PIN-B, PIN-3, Anzahl der Steps, Knopf an 5V oder Ground
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  last = items-1;
  encoder->setAccelerationEnabled(true);

  display.InitLCD();
  
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
  delay(showSplashFor*100);
}

void readEncoder() {
  selector += encoder->getValue();
  if (selector != last) {
    displayLEDisOn = true;
    lastInput = millis();
    if (selector > items+1 || selector < 0) {
      if (selector < 0) {
        selector = items+1;
      }
      else {
        selector = 0;
      }
    }
    last = selector;
  }
}

void confirm(int conf, int from) {
  display.clrScr();
  digitalWrite(displayLight, LOW);
  display.setFont(BigNumbers);
  display.printNumI(conf, CENTER, 0, 2, '0');
  display.setFont(SmallFont);
  display.print(" Saved!", CENTER, 32);
  delay(1500);
  menu(from);
}

void screenSaver(bool withLight = true) {
  display.clrScr();
  if(withLight) {
    digitalWrite(displayLight, LOW);
  }
  else {
    digitalWrite(displayLight, HIGH);
  }
  while (selector == items) {
    readEncoder();
    display.print(rtc.getDateStr(FORMAT_LONG, FORMAT_BIGENDIAN, '/'), CENTER, 11);
    display.print(rtc.getTimeStr(), CENTER, 29);
    timer();
  }
}

void timer() {
  if(displayLEDisOn) {
    check = millis();
    if(check - lastInput > displayTimeout) {
      displayLEDisOn = false;
      selector = items;
      menu(items);
    }
  }
}

void readLog() {
  display.clrScr();
  digitalWrite(displayLight, LOW);
  display.print("Current", CENTER, 0);
  display.print("---------------", CENTER, 8);
  for(int i = 0; i < items; i++) {
    display.print(menuItem[i], LEFT, 16+8*i);
    display.printNumI(EEPROM.read(i), RIGHT, 16+8*i);
  }
  while (selector == items+1)
  {
    readEncoder();
    timer();
  }
}

void setValue(int menuSelector, int minimum, int maximum) {
  itemValue = EEPROM.read(menuSelector);
  display.clrScr();
  digitalWrite(displayLight, LOW);
  display.print("Click to set", CENTER, 0);
  display.print(menuItem[menuSelector], CENTER, 20);
  while (selector == menuSelector) {
    ClickEncoder::Button b = encoder->getButton();
    readEncoder();
    timer();
    if (b == ClickEncoder::Clicked) {
      display.clrScr();
      display.print("Set", CENTER, 0);
      display.print(menuItem[menuSelector], CENTER, 8);
      display.print("---------", CENTER, 16);
      while (true) {
        ClickEncoder::Button b = encoder->getButton();
        itemValue += encoder->getValue();
        if (itemValue > maximum || itemValue < minimum) {
          if (itemValue < minimum) {
            itemValue = minimum;
          }
          else {
            itemValue = maximum;
          }
        }
        display.setFont(BigNumbers);
        display.printNumI(itemValue, CENTER, 24, 2, '0');
        display.setFont(SmallFont);
        if (b == ClickEncoder::DoubleClicked) {
          lastInput = millis();
          menu(menuSelector);
          break;
        }
        if (b == ClickEncoder::Held) {
          lastInput = millis();
          EEPROM.write(menuSelector, itemValue);
          confirm(itemValue, menuSelector);
          break;
        }
      }
    }
  }
}

void menu(int switcher) {
  switch (switcher) {
    case 0:
        setValue(switcher, 0, 24);
      break;
    case 1:
        setValue(switcher, 15, 35);
      break;
    case 2:
        setValue(switcher, 0, 99);
      break;
    case 3:
        setValue(switcher, 10, 30);
      break;
    case items:
      screenSaver(displayLEDisOn);
      break;
    case items+1:
      readLog();
      break;
    default: 
      screenSaver();
  }
}
