//#include <ShiftLCD.h>
//ShiftLCD lcd(10, 8, 9);
// Include the LCD helper and the application string
#include "LCD.h"
#include "Strings.h"
#include "Version.h"

//! AlphaLCD class instance for display hardware control
AlphaLCD lcd(8,9,10);
void setup() {
  //lcd.begin(16, 2);
}

void loop() {
//  lcd.clear();
//  lcd.write("Klaw MCheen");
//  delay(1000);
// Initializes the LCD library
  lcd.begin(LCDCHARS, LCDROWS);
  // Turn LCD On
  lcd.display();
  welcome();
}
