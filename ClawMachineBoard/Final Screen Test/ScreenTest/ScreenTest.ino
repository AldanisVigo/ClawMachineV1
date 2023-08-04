#include <LiquidCrystal_74HC595.h>

#define DS 10
#define SHCP 9
#define STCP 8

#define RS 1
#define E 2
#define D4 3
#define D5 4
#define D6 5
#define D7 6

#define inputDataPin 12
#define inputClockPin 2
#define inputLatchPin 3
#define inputClockEnable 4

LiquidCrystal_74HC595 lcd(DS, SHCP, STCP, RS, E, D4, D5, D6, D7);

void setup() {

  //Initialize The LCD Screen
  lcd.begin(20, 4);
  lcd.print("Vigo's Klaw");
  lcd.setCursor(8,1);
  lcd.print("M'Sheen");
  delay(3000);

  //Initialize The Input Shift Register
  pinMode(inputLatchPin, OUTPUT);
  pinMode(inputDataPin, INPUT);
  pinMode(inputClockPin, OUTPUT);
  pinMode(inputClockEnable, OUTPUT);
  digitalWrite(inputClockEnable,LOW);
}

unsigned char shiftIn165(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
  uint8_t value = 0;
  uint8_t i;

  for (i = 0; i < 8; ++i) {
    digitalWrite(clockPin, LOW);
    if (bitOrder == LSBFIRST) {
      value |= digitalRead(dataPin) << i;
    }
    else {
      value |= digitalRead(dataPin) << (7 - i);
    }
    digitalWrite(clockPin, HIGH);
  }
  return value;
}

void loop() {
  //Latch the inputs on the input shift register
  digitalWrite(inputLatchPin, LOW);
  delayMicroseconds(20);
  digitalWrite(inputLatchPin, HIGH);

  //Read the inputs from the shift register
  byte regByte = shiftIn(inputDataPin, inputClockPin, MSBFIRST);
  
  //Clear the LCD
  lcd.clear();
  delay(100);

  //Set the cursor to the top left
  lcd.setCursor(0,1);

  //Print the inputs as binary
  //Set the cursor to the top left
  lcd.setCursor(0,0);
  lcd.print("Limit Switches");
  lcd.setCursor(0,1);
  //Print the inputs as binary
  if (regByte & (1 << 7) ){
    // bit n is set
    lcd.print("1");
  }else{
    lcd.print("0");
  }
   if (regByte & (1 << 6) ){
    // bit n is set
    lcd.print("1");
  }else{
    lcd.print("0");
  }
   if (regByte & (1 << 5) ){
    // bit n is set
    lcd.print("1");
  }else{
    lcd.print("0");
  }
   if (regByte & (1 << 4) ){
    // bit n is set
    lcd.print("1");
  }else{
    lcd.print("0");
  }
   if (regByte & (1 << 3) ){
    // bit n is set
    lcd.print("1");
  }else{
    lcd.print("0");
  }
   if (regByte & (1 << 2) ){
    // bit n is set
    lcd.print("1");
  }else{
    lcd.print("0");
  }
   if (regByte & (1 << 1) ){
    // bit n is set
    lcd.print("1");
  }else{
    lcd.print("0");
  } if (regByte & (1 << 0) ){
    // bit n is set
    lcd.print("1");
  }else{
    lcd.print("0");
  }
  delay(1000);
}
