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
const char alphahex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

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
unsigned char* selectUUID();

unsigned short getNameLength(const char* s);

void timeout();

void setup() {
  Entropy.initialize();
  keypad.begin(16, 2);
  Serial.begin(115200);


  if (Memory::hasSchema()) {
    state = MENU;
  } else {
    state = SETUP;
  }
  Memory memory;

  while (rn) {
    keypad.setCursor(0, 0);
    if (state == SETUP) {
      memory.clearContacts();
      memory.clearMessages();
      keypad.print("Enter name:");
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
      while (true) {
        if (keypad.getButtonPress() == UP) {
          state = MENU;
          break;
        } else if (keypad.getButtonPress() == LEFT) {
          keypad.clear();
          state = SETUP;
          break;
        }
      }
    } else if (state == MENU) {
      keypad.clear();
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

    } else if (state == NEW_CONTACT) {
      if (memory.getNumberContacts() >= MAX_CONTACTS) {
        keypad.clear();
        keypad.setCursor(0, 0);
        keypad.print("Contact List");
        keypad.setCursor(0, 1);
        keypad.print("is full!");
        timeout();
        state = MENU;
      }
      keypad.clear();
      keypad.setCursor(0, 0);
      keypad.print("New Contact Name:");
      String nam = selectName();
      keypad.clear();
      keypad.setCursor(0, 0);
      keypad.print("New Contact UUID:");
      delay(250);
      unsigned char* uuid = selectUUID();
      Contact c(uuid, nam.c_str());
      memory.saveContact(c);
      keypad.clear();
      keypad.setCursor(0, 0);
      keypad.print("Contact Added!");
      timeout();
      state = MENU;
    } else if (state == CONTACTS) {
      keypad.clear();
      keypad.setCursor(0, 0);
      keypad.print("Contact:");
      if (memory.getNumberContacts() == 0) {
        keypad.setCursor(0, 1);
        keypad.print("No Contacts!");
        timeout();
        state = MENU;
      }
      for (int i = 0; i < memory.getNumberContacts(); i++) {
        keypad.clear();
        keypad.setCursor(0, 0);
        keypad.print("Contact:");
        keypad.setCursor(0, 1);
        keypad.print("<- ");
        const char* nam = memory.getContact(i).getName();
        keypad.setCursor(4 + getNameLength(nam) / 2, 1);
        keypad.print(nam);

        keypad.setCursor(13, 1);
        keypad.print(" ->");
        timeout();
      }


    }
    else {
    }

    delay(250);
  }


}

void loop() {}

void timeout() {
  unsigned long wait = millis();
  while (millis() - wait < 2000 && keypad.getButtonPress() != UP);
}

String selectName() {
  int index = 0;
  unsigned short len = 0;
  String nam = "";
  unsigned long term = millis();
  while (true) {
    keypad.setCursor(len, 1);
    keypad.print(alphabet[index]);
    if (keypad.getButtonPress() == UP) {
      index++;
      if (index >= 26) index = 0;
    } else if (keypad.getButtonPress() == DOWN) {
      index--;
      if (index < 0) index = 25;
    } else if (keypad.getButtonPress() == RIGHT) {
      nam += alphabet[index];
      len++;
      index = 0;
    } else if (keypad.getButtonPress() == LEFT) {
      if (len > 0) {
        keypad.setCursor(len--, 1);
        keypad.print(' ');
        index = 0;
      }
    } else if (keypad.getButtonPress() == SELECT) {
      //      nam[len++] = '\0';
      nam += alphabet[index];
      Serial.println(nam);
      return nam;

    }
    if ((millis() - term) > 1000) {
      keypad.setCursor(len, 1);
      keypad.print(' ');
      term = millis();
    }
    delay(150);
  }
}

unsigned char* selectUUID() {
  int index = 0;
  unsigned short len = 0;
  unsigned char uuid[5] = {0, 0, 0, 0, 0};
  unsigned long term = millis();
  while (true) {
    keypad.setCursor(0, 1);
    for (int i = 0; i < 5; i++) {
      if (uuid[i] < 10) {
        keypad.print('0');
        Serial.print('0');
      }
      keypad.print(uuid[i], HEX);
      Serial.print(uuid[i], HEX);
    }
    keypad.setCursor(len, 1);
    keypad.print(index, HEX);
    Serial.println();

    if (keypad.getButtonPress() == UP) {
      index++;
      if (index >= 16) index = 0;
    } else if (keypad.getButtonPress() == DOWN) {
      index--;
      if (index < 0) index = 15;
    } else if (keypad.getButtonPress() == RIGHT) {
      uuid[len / 2] |= index << (4 * ((len + 1) % 2));
      len++;
      index = 0;
      delay(100);
    } else if (keypad.getButtonPress() == LEFT) {
      if (len > 0) {
        if (len % 2 == 0) {
          uuid[len / 2] &= 0b11110000;
        } else {
          uuid[len / 2] = 0;
        }
        len--;
        index = 0;
      }
    } else if (keypad.getButtonPress() == SELECT) {
      return uuid;
    }
    if ((millis() - term) > 1000) {
      keypad.setCursor(len, 1);
      keypad.print(' ');
      term = millis();
    }
    delay(150);
  }
}

unsigned char* generateUUID() {
  unsigned char uuid[5] = {0, 0, 0, 0, 0};

  for (int i = 0; i < 5; i++) {
    uuid[i] = Entropy.randomByte();
  }

  return uuid;
}

unsigned short getNameLength(const char* s){
  unsigned short idx = 0;
  for(; *(s+idx) != '\0'; idx++);
  return idx;  
}
