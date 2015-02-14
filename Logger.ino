#include <EEPROM.h>

#include <ClickEncoder.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

char appName[] = "Logger";
char appVersion[] = "0.1";
char versionDate[] = "2015-02-11";

tmElements_t tm;

void setup() {
  Serial.begin(9600);

  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);
  
  display.clearDisplay();   // clears the screen and buffer
  
  // text display tests
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println(appName);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.print("Version: ");
  display.println(appVersion);
  display.println("Date: ");
  display.print(versionDate);
  display.display();
  delay(4000);
}

void loop() {
  RTC.read(tm);
  screenSaver();
  delay(1000);
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    display.print('0');
  }
  display.print(number);
}

void screenSaver() {
  display.clearDisplay();   // clears the screen and buffer
  display.setCursor(15,0);
  print2digits(tmYearToCalendar(tm.Year));
  display.print("-");
  print2digits(tm.Month);
  display.print("-");
  print2digits(tm.Day);
  display.setCursor(20,20);
  print2digits(tm.Hour);
  display.print(":");
  print2digits(tm.Minute);
  display.print(":");
  print2digits(tm.Second);
  display.display();
}
