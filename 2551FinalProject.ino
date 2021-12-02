#include <Entropy.h>

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

const char alphabet[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};


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

unsigned char* generateUUID();
String selectName();

void setup() {
  Entropy.initialize();
  keypad.begin(16, 2);

  Memory memory;

  while (rn) {
    Serial.begin(9600);
    keypad.setCursor(0, 0);

    if (state == BOOT) {
      //TODO: check schema and setup memory
      state = SETUP;

    } else if (state == SETUP) {
      keypad.print("Enter name:");
      selectName();
      String n = selectName();
      Serial.println(n);
      
      unsigned char* uuid = generateUUID();
      Contact c(uuid, n.c_str());

      memory.saveNodeInformation(c);

      keypad.clear();
      state = MENU;
    } else if (state == ABOUT) {
      keypad.print("Name: ");
      keypad.print(memory.getNodeName());
      keypad.setCursor(0, 1);
      keypad.print("UUID: ");
      for (int i = 0; i < UUID_LENGTH; i++) {
        if (memory.getNodeUUID()[i] < 10) {
          keypad.print("0");
        }
        keypad.print(memory.getNodeUUID()[i], HEX);
      }

    } else if (state == MENU) {
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

    }
    else {
    }

    delay(250);
  }


}

void loop() {}

String selectName() {
  int index = 0;
  unsigned short len = 0;
  String nam = "";
  while(true) {
    keypad.setCursor(len, 1);
    keypad.print(alphabet[index]);
    if(keypad.getButtonPress() == UP) {
      index++;
      if(index >= 26) index = 0;
    } else if(keypad.getButtonPress() == DOWN) {
      index--;
      if(index < 0) index = 25;
    } else if(keypad.getButtonPress() == RIGHT) {
      nam += alphabet[index];
      len++;
      index = 0;
    } else if(keypad.getButtonPress() == LEFT) {
      if(len > 0) {
        keypad.setCursor(len--, 1);
        keypad.print(' ');
        index = 0;
      }
    } else if(keypad.getButtonPress() == SELECT) {
//      nam[len++] = '\0';
      return nam.c_str();
    }
    delay(250);
  }
}

unsigned char* generateUUID() {
  unsigned char uuid[5] = {0, 0, 0, 0, 0};

  for (int i = 0; i < 5; i++) {
    uuid[i] = Entropy.randomByte();
  }

  return uuid;
}
