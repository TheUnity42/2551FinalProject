#include <RF24_config.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

#include "Contact.hpp"
#include "LCDKeypad.hpp"
#include "Message.hpp"
#include "Memory.hpp"

LCDKeypad keypad(8, 9, 4, 5, 6, 7, A0);

unsigned char uuid[5] = {0, 1, 2, 3, 4};

typedef enum {
  BOOT,
  SETUP,
  MENU,
  ABOUT,
  NEW_CONTACT,
  MESSAGES,
  CONTACTS
} State;

State state = BOOT;
bool rn = true;

unsigned short menuState = 0;

void setup() {
  keypad.begin(16, 2);
  unsigned char uuid[] = {255, 128, 64, 47, 2};
  Contact n(uuid, "Bob");
  Memory memory(n);
  while (rn) {
    keypad.setCursor(0, 0);
    switch (state) {
      case BOOT:
        //TODO: check schema and setup memory
        state = SETUP;
        break;
      case SETUP:
        keypad.print("Enter name:");
        //TODO: input name
        //TODO: generate UUID
        keypad.clear();
        state = MENU;
        break;
      case ABOUT:
        keypad.print("Name: ");
        keypad.print(memory.getNodeName());
        keypad.setCursor(0, 1);
        keypad.print("UUID: ");
        for (int i = 0; i < UUID_LENGTH; i++) {
          keypad.print(memory.getNodeUUID()[i], HEX);
        }

        break;
      case MENU:
        keypad.print("Menu:");
        keypad.setCursor(0, 1);
        switch (menuState) {
          case 0:
            keypad.print("<-  Contacts  ->"); //TODO: make arrows blink
            switch (keypad.getButtonPress()) {
              case LEFT:
                menuState = 3;
                break;
              case RIGHT:
                menuState++;
                break;
              case SELECT:
                state = CONTACTS;
                break;
              default:
                break;
            }
            break;
          case 1:
            keypad.print("<-  Messages  ->"); //TODO: make arrows blink
            switch (keypad.getButtonPress()) {
              case LEFT:
                menuState--;
                break;
              case RIGHT:
                menuState++;
                break;
              case SELECT:
                state = MESSAGES;
                break;
              default:
                break;
            }
            break;
          case 2:
            keypad.print("<- N.Contact  ->"); //TODO: make arrows blink
            switch (keypad.getButtonPress()) {
              case LEFT:
                menuState--;
                break;
              case RIGHT:
                menuState++;
                break;
              case SELECT:
                state = NEW_CONTACT;
                break;
              default:
                break;
            }
            break;
          case 3:
            keypad.print("<-  About Me  ->"); //TODO: make arrows blink
            switch (keypad.getButtonPress()) {
              case LEFT:
                menuState--;
                break;
              case RIGHT:
                menuState = 0;
                break;
              case SELECT:
                state = ABOUT;
                break;
              default:
                break;
            }
            break;
        }

        break;
      default:
        break;
    }
    delay(250);
  }


}

void loop() {}
