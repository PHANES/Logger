/*######################################## Sketch Version ########################################*/
const char appName[] = "Logger";
const char appVersion[] = "0.711";
const char versionDate[] = "14.Mar\'15";
const byte showSplashFor = 3;

/*######################################## My Menu Variables ########################################*/
String menuItem[] = {"Light", "Temperature", "Humidity", "Water"};
const byte items = sizeof(menuItem)/sizeof(menuItem[0]);
int selector = items;
int itemValue = 0;
long check = 0;
long lastInput = 0;
unsigned int displayTimeout = 15000;

/*######################################## EEPROM ########################################*/
#include <EEPROM.h>

/*######################################## DS18B20 ########################################*/
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 9 on the Arduino
const int ONE_WIRE_BUS = 10;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

float waterTemp = 0.0;

/*######################################## DHT22 ########################################*/
#include "DHT.h"

const int dhtPin = 9;

DHT dht;

float humidity = 0.0;
float temperature = 0.0;

/*######################################## Shift Register ########################################*/
//Pin connected to ST_CP of 74HC595
const int latchPin = 13;
//Pin connected to SH_CP of 74HC595
const int clockPin = 12;
////Pin connected to DS of 74HC595
const int dataPin = 11;

// holders for infromation you're going to pass to shifting function
byte data;
byte dataArray[8] = {1, 2, 4, 8, 16, 32, 64, 128};

// remember if a relay is on->[1] or off->[0]
byte relayIsOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/*######################################## Rotary Encoder ########################################*/
#include <ClickEncoder.h>
#include <TimerOne.h>

ClickEncoder *encoder;
int last = items-1;

void timerIsr() {
  encoder->service();
}

/*######################################## NOKIA 5110 Display ########################################*/
// It is assumed that the LCD module is connected to
// the following pins using a levelshifter to get the
// correct voltage to the module.
//      CLK  - Pin 7
//      Din  - Pin 6
//      DC   - Pin 5
//      CE   - Pin 4
//      RST  - Pin 3
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
  Serial.begin(9600);
  // Initialize the RealTimeClock object
  rtc.begin();
  
  // Initialize the DallasTemperature object
  sensors.begin();

  // Inicialize the DHT22 object
  dht.setup(dhtPin);
  
  //shift register pins to output
  pinMode(latchPin, OUTPUT);
  
  // Initialize Rotary Encoder object
  encoder = new ClickEncoder(A1, A0, A2, 4, LOW); //Rotary PIN-A, PIN-B, PIN-3, Anzahl der Steps, Knopf an 5V oder Ground
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  encoder->setAccelerationEnabled(true);

  // Initialize LCD Display object
  display.InitLCD();
  pinMode(displayLight, OUTPUT);
  
  readAir();
  
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
  display.print("Temp :", LEFT, 24);
  display.print("C", RIGHT, 24);
  display.print("Humid:", LEFT, 32);
  display.print("%", RIGHT, 32);
  display.print("Water:", LEFT, 40);
  display.print("C", RIGHT, 40);
  if(withLight) {
    digitalWrite(displayLight, LOW);
  }
  else {
    digitalWrite(displayLight, HIGH);
  }
  while (selector == items) {
    readWater();
    readEncoder();
    display.printNumF(waterTemp, 2, 46, 40);
    display.printNumF(temperature, 2, 46, 24);
    display.printNumF(humidity, 2, 46, 32);
    display.print(rtc.getDateStr(FORMAT_LONG, FORMAT_BIGENDIAN, '/'), CENTER, 0);
    display.print(rtc.getTimeStr(), CENTER, 8);
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
  display.print("Setpoints", CENTER, 0);
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

void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {  
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}

void switchRelay(int relay) {
  // set relay to 0 index
  relay --;
  // set the byte for this relay to on or off
  relayIsOn[relay] = (1 == relayIsOn[relay]) ? 0 : 1;
  // switch the state of the relay
  /*
    int x = 12;     // binary: 1100
    int y = 10;     // binary: 1010
    int z = x ^ y;  // binary: 0110, or decimal 6
  */
  data = data ^ dataArray[relay];
  //ground latchPin and hold low for as long as you are transmitting
  digitalWrite(latchPin, 0);
  //move 'em out
  shiftOut(dataPin, clockPin, data);
  //return the latch pin high to signal chip that it 
  //no longer needs to listen for information
  digitalWrite(latchPin, 1);
  delay(300);
}

void readWater() {
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  waterTemp = sensors.getTempCByIndex(0);
  Serial.print("Water temp.: \n");
  Serial.println(waterTemp);
}

void readAir() {
  
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();

  Serial.print("Humid.: \n");
  Serial.println(humidity);
  Serial.print("Air temp.: \n");
  Serial.println(temperature);
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
