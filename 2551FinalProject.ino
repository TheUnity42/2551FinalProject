#include "LCDKeypad.hpp"


LCDKeypad keypad(8, 9, 4, 5, 6, 7, A0);

void setup() {
  keypad.begin(16, 2);
  keypad.print("Hello, World");
  
  
}

void loop() {
  keypad.clear();
  keypad.setCursor(0, 1);
  keypad.print(keypad.getButtonPress());
  delay(100);
}
