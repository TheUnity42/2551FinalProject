#include "LCDKeypad.hpp"
#include <Arduino.h>
#include <LiquidCrystal.h>

LCDKeypad::LCDKeypad(int rs, int enable, int d4, int d5, int d6, int d7, int buttonPin)
	: LiquidCrystal(rs, enable, d4, d5, d6, d7) {
	this->buttonPin = buttonPin;
	pinMode(buttonPin, INPUT);
}

Button LCDKeypad::getButtonPress() {
	Button b = NONE;
	int value = analogRead(buttonPin);
	delayMicroseconds(DEBOUNCE_DELAY);
	if (abs(analogRead(buttonPin) - value) <= THRESHOLD) {
		for (int i = 0; i < 6; i++) {
			if (abs(value - VOLTAGES[i]) <= THRESHOLD) {
				b = static_cast<Button>(i);
			}
		}
	}
	return b;
}
