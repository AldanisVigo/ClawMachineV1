#include <BigCrystal.h>

BigCrystal lcd;

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16,2);
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.write("Hello World");
}
