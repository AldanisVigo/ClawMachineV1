#ifndef _LCD_SCREEN_H_
#define _LCD_SCREEN_H_

//AVR --> Serial Backpack Pins
#define DS 10
#define SHCP 9
#define STCP 8

//LCD --> Backpack SR Pins
#define RS 1
#define E 2
#define D4 3
#define D5 4
#define D6 5
#define D7 6

#include <LiquidCrystal_74HC595.h> 

LiquidCrystal_74HC595 lcd(DS, SHCP, STCP, RS, E, D4, D5, D6, D7);

void initialize_lcd_screen(){
  //Initialize The LCD Screen
  lcd.begin(20, 4);
  lcd.print("Vigo's Klaw");
  lcd.setCursor(8,1);
  lcd.print("M'Sheen");
  delay(3000);
}

#endif //LCD_SCREEN_H
