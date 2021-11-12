#ifndef LCDKeypad_h
#define LCDKeypad_h

#include <LiquidCrystal.h>

/*
 * None:    1023
 * Select:  639
 * Left:    411
 * Right:   0-10
 * Up:      99
 * Down:    256
 */
typedef enum {LEFT, RIGHT, UP, DOWN, SELECT, NONE} Button;

class LCDKeypad : public LiquidCrystal {
  public:
    LCDKeypad(int rs, int enable, int d4, int d5, int d6, int d7, int buttonPin);
    Button getButtonPress();
  private:    
    int buttonPin;    
    const int DEBOUNCE_DELAY = 100;
    const int THRESHOLD = 25;
    const int VOLTAGES[6] = {411, 10, 99, 256, 639, 1023};
};


#endif
