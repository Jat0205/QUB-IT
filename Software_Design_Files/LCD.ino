/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-lcd-20x4
 */

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20 column and 4 rows

void setup() {
  lcd.init(); // initialize the lcd
  lcd.backlight();

  lcd.setCursor(0, 0);            // move cursor the first row
  lcd.print("INITIALIZE IT!");          // print message at the first row
}

void loop() {
}
