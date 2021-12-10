#include <Entropy.h>

#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#include "Contact.hpp"
#include "LCDKeypad.hpp"
#include "Memory.hpp"
#include "Message.hpp"

#define IRQ_PIN 2

RF24 radio(A1, A2);


// Used to control whether this node is sending or receiving
bool role = false;  // true = TX node, false = RX node

bool uuidcmp(unsigned char* id, unsigned char* id2);

LCDKeypad keypad(8, 9, 4, 5, 6, 7, A0);

unsigned char uuid[5] = {0, 0, 0, 0, 2};

const char alphabet[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                           'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
                          };
const char alphahex[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
                          };

typedef enum { BOOT, SETUP, MENU, ABOUT, NEW_CONTACT, MESSAGES, CONTACTS, NEW_MESSAGE, OPEN_MESSAGE } State;

State state = BOOT;
bool rn = true;

char counter = 'A';

const int buzzer = 7;

Memory memory;



unsigned short menuState = 0;

void interruptHandler(); // prototype to handle IRQ events
void printRxFifo();     // prototype to print RX FIFO with 1 buffer

unsigned char *generateUUID();
char* selectName();
unsigned char *selectUUID();
void printUUID(unsigned char* id);

unsigned short getNameLength(const char *s);

bool sendMessage(Contact c);

void timeout();

char* getContactName(unsigned char* id);

void setup() {
  pinMode(buzzer, OUTPUT);
  Entropy.initialize();
  keypad.begin(16, 2);
  Serial.begin(115200);
  printf_begin();
  keypad.clear();
  keypad.setCursor(0, 0);
  //memory.clearContacts();
  if (!radio.begin()) {
    keypad.print("Radio Error");
    while (1)
      ; //?
  }
//  Contact c(uuid, 'T');
//  memory.saveNodeInformation(c);

  // setup the IRQ_PIN
  pinMode(IRQ_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), interruptHandler, FALLING);

  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
  radio.enableDynamicPayloads();    // ACK payloads are dynamically sized

  unsigned char u[5] = {0, 0, 2, 0, 0x60};
  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(u);     // always uses pipe 0

  //  if (role) {
  //    radio.stopListening();
  //  } else {
  radio.maskIRQ(1, 1, 0); // args = "data_sent", "data_fail", "data_ready"
  radio.startListening(); // put radio in RX mode
  //  }

  if (Memory::hasSchema()) {
    state = MENU;
  } else {
    state = SETUP;
  }


  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, memory.getNodeUUID()); // using pipe 1

  for (int i = 0; i < 5; i++) {
    if (memory.getNodeUUID()[i] < 10) Serial.print(0);
    Serial.print(memory.getNodeUUID()[i], HEX);
  }
  Serial.println();

  // loop persistent variables
  volatile unsigned int contact_idx = 0;
  volatile unsigned int last_contact_idx = 100;
  int message_idx = 0;
  int last_message_idx = -1;
  bool blink = false;
  unsigned long last_blink_flip = millis();

  //TODO: remove
  unsigned char from[] = {0, 1, 2, 3, 4};
  unsigned char to[] = {1, 2, 3, 4, 5};
  Message testMessage(memory.getNodeUUID(), to, 23, 16);
  //      memory.saveMessage(testMessage);

  while (rn) {
    keypad.setCursor(0, 0);
    if (state == SETUP) {
      memory.clearContacts();
      memory.clearMessages();
      keypad.print("Enter name:");
      char* n = selectName();
      Serial.println(n);

      unsigned char *uuid = generateUUID();
      Contact c(uuid, n);

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
          keypad.print("<-  Contacts  ->"); // TODO: make arrows blink
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
          keypad.print("<-  Messages  ->"); // TODO: make arrows blink
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
          keypad.print("<- N.Contact  ->"); // TODO: make arrows blink
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
          keypad.print("<-  About Me  ->"); // TODO: make arrows blink
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

      if (millis() - last_blink_flip > 500) {
        last_blink_flip = millis();
        blink = !blink;
      }

      if (blink) {
        keypad.setCursor(0, 1);
        keypad.print("  ");
        keypad.setCursor(14, 1);
        keypad.print("  ");
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
      char* nam1;  //returns char                     //////////////marker
      nam1 = selectName();
      keypad.clear();
      keypad.setCursor(0, 0);
      keypad.print("New Contact UUID:");
      delay(250);
      unsigned char *uuid = selectUUID();
      Serial.println("INPUTED UUID: ");
      printUUID(uuid);
      char* nam2[11];
      for (int i = 0; i < 5; i++) {
        nam2[i] = nam1[i];
      }
      Contact c(uuid, nam2);
      memory.saveContact(c);
      printUUID(c.getUUID());
      keypad.clear();
      keypad.setCursor(0, 0);
      keypad.print("Contact Added!");
      timeout();
      state = MENU;

    } else if (state == CONTACTS) {
      if (memory.getNumberContacts() == 0) {
        keypad.setCursor(0, 1);
        keypad.print("No Contacts!");
        timeout();
        state = MENU;
        continue;
      }

      if (last_contact_idx != contact_idx) {
        last_contact_idx = contact_idx;
        keypad.clear();
        keypad.setCursor(0, 0);
        keypad.print("Contact:");
        keypad.setCursor(0, 1);
        keypad.print("<- "); Serial.print("contact_idx: "); Serial.println(contact_idx);
        char *namDisp;
        namDisp = memory.getContact(contact_idx).getName(); //Returning 'E'
        keypad.setCursor(3, 1);
        Serial.print("CONTACT NAME IS: ");
        for (int i = 0; i < 10 && (namDisp[i] != '\0'); i++) {
          Serial.print(namDisp[i]);
          //        keypad.print(namDisp[i]);
        }
        Serial.println();
        printUUID(memory.getContact(contact_idx).getUUID());
        keypad.setCursor(13, 1);
        keypad.print(" ->");
      }

      switch (keypad.getButtonPress()) {
        case SELECT:
          state = NEW_MESSAGE;
          break;
        case UP:
          state = MENU;
          break;
        case RIGHT:
          if (contact_idx < memory.getNumberContacts() - 1) {
            contact_idx++;
          } else {
            contact_idx = 0;
          }
          break;
        case LEFT:
          if (contact_idx > 0) {
            contact_idx--;
          }
          break;
        default:
          break;
      }
      delay(50);
    } else if (state == NEW_MESSAGE) {
      keypad.clear();
      keypad.print("To: ");
      Contact c = memory.getContact(contact_idx);
      keypad.print(c.getName());
      bool sent = sendMessage(c);
      if (!sent && state == MENU) {
        continue;
      }
      keypad.clear();
      keypad.setCursor(0, 0);
      if (sent) {
        keypad.print("Message Sent!"); // TODO: buzzer
      } else {
        tone(buzzer, 1000);
        keypad.print("Message Failed!");
      }
      timeout();
      last_contact_idx = 100;
      state = CONTACTS;
    }
    else if (state == MESSAGES) {
      keypad.print("Messages:");
      if (memory.getNumberMessages() < 1) {
        keypad.clear();
        keypad.setCursor(0, 0);
        keypad.print("No messages!");
        timeout();
        state = MENU;
      } else {
        if (message_idx != last_message_idx) {
          Message m = memory.getMessage(message_idx);

          unsigned char *from = m.getFrom();

          char sr = 'R';
          if (uuidcmp(m.getFrom(), memory.getNodeUUID()))
            sr = 'S';

          const char *nam = getContactName(from);

          keypad.clear();
          keypad.setCursor(14, 0);
          keypad.print('[');
          keypad.print(sr);
          keypad.setCursor(0, 1);

          keypad.print(message_idx + 1);
          keypad.print(". ");
          keypad.print(nam);
          last_message_idx = message_idx;
        }
      }

      switch (keypad.getButtonPress()) {
        case SELECT:
          state = OPEN_MESSAGE;
          break;
        case RIGHT:
          if (message_idx < memory.getNumberMessages() - 1)
            message_idx++;
          break;
        case LEFT:
          if (message_idx > 0)
            message_idx--;
          break;
        case UP:
          state = MENU;
          break;
        default:
          break;
      }
    } else if (state == OPEN_MESSAGE) {
      Message m = memory.getMessage(message_idx);

      unsigned char *from = m.getFrom();

      char sr = 'R';
      if (uuidcmp(m.getFrom(), memory.getNodeUUID())) //FIXME: this no worky
        sr = 'S';

      const char *nam = getContactName(from);

      keypad.clear();
      keypad.setCursor(0, 0);
      keypad.print(sr == 'R' ? "From: " : "Sent: ");
      keypad.print(nam);
      keypad.setCursor(0, 1);
      unsigned short payload = m.getPayload();
      keypad.print(m.getPayloadString());

      while (keypad.getButtonPress() != UP) {

      }
      state = MESSAGES;
      message_idx = 0;
    } else {
      keypad.clear();
      keypad.setCursor(0, 0);
      keypad.print("ERROR");
    }

    delay(250);
  }
}

void loop() {
  if (role) {
    if (radio.write(&counter, 23)) {
      if (radio.rxFifoFull()) {
        Serial.println(F("RX node's FIFO is full; it is not listening any more"));
      } else {
        Serial.println("Transmission successful, but the RX node might still be listening.");
      }
    } else {
      Serial.println(F("Transmission failed or timed out. Continuing anyway."));
      radio.flush_tx(); // discard payload(s) that failed to transmit
    }
    Serial.print("Counter at: "); Serial.println(counter++);
  } else {
  }
  delay(1000);
}

void timeout() {
  unsigned long wait = millis();
  while (millis() - wait < 2000 && keypad.getButtonPress() != UP)
    ;
}

char* selectName() {
  int index = 0;
  unsigned short len = 0;
  static char nam[11];
  unsigned long term = millis();
  while (true) {
    keypad.setCursor(len, 1);
    keypad.print(alphabet[index]);
    if (keypad.getButtonPress() == UP) {
      index++;
      if (index >= 26)
        index = 0;
    } else if (keypad.getButtonPress() == DOWN) {
      index--;
      if (index < 0)
        index = 25;
    } else if (keypad.getButtonPress() == RIGHT) {
      nam[len++] = alphabet[index];
      index = 0;
    } else if (keypad.getButtonPress() == LEFT) {
      if (len > 0) {
        keypad.setCursor(len--, 1);
        keypad.print(' ');
        index = 0;
      }
    } else if (keypad.getButtonPress() == SELECT) {

      nam[index] = alphabet[index];
      nam[++index] = '\0';
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

unsigned char *selectUUID() {
  int index = 0;
  unsigned short len = 0;
  static char uuid[5] = {0, 0, 0, 0, 0};
  unsigned long term = millis();
  while (true) {
    keypad.setCursor(0, 1);
    for (int i = 0; i < 5; i++) {
      if (uuid[i] < 10) {
        keypad.print('0');
      }
      keypad.print(uuid[i], HEX);
    }
    keypad.setCursor(len, 1);
    keypad.print(index, HEX);
    printUUID(uuid);
    Serial.println();

    if (keypad.getButtonPress() == UP) {
      index++;
      if (index >= 16)
        index = 0;
    } else if (keypad.getButtonPress() == DOWN) {
      index--;
      if (index < 0)
        index = 15;
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
      Serial.println("selected UUID");
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

bool sendMessage(Contact c) {
  unsigned short message = 0;
  int len = 0;
  keypad.setCursor(0, 1);
  while (true) {
    Serial.println(message, BIN);

    Button b = keypad.getButtonPress();
    if (b == SELECT) {
      break;
    } else if (b == RIGHT) {
      if (len < 16) {
        message |= (1 << (15 - len++));
      }
      keypad.print('-');
    } else if (b == LEFT) {
      if (len < 16) {
        len++;
        keypad.print('.');
      }
    } else if (b == UP) {
      state = MENU;
      return false;
    } else if (b == DOWN) {
      if (len > 0) {
        keypad.setCursor(len - 1, 1);
        keypad.print(' ');
        message &= ~((unsigned short)(1 << (16 - len)));
        len--;
      }
    }
    delay(150);
  }

  Message mess(memory.getNodeUUID(), c.getUUID(), message, len);

  memory.saveMessage(mess);


  radio.stopListening();
  delay(50);

  radio.openWritingPipe(c.getUUID());

  printUUID(c.getUUID());

  char* message_str = mess.getPayloadString();

  char mess_str[23];

  int j = 0;
  for (int i = 0; message_str[i] != '\0'; i++) {
    mess_str[i] = message_str[i];
    j++;
  }
  mess_str[j++] = '\0';
  for (int i = 0; i < 5; i++) {
    mess_str[j + i] = uuid[i];
  }
  mess_str[j + 5] = '\0';

  Serial.println(mess.getPayload(), BIN);

  bool sent = false;

  const char* testing = "1234567890123456789012";

  for (int i = 0; i < 5 && sent == false; i++) {
    if (radio.write(&mess, sizeof(Message))) {
      sent = true;
      if (radio.rxFifoFull()) {
        Serial.println(F("RX node's FIFO is full; it is not listening any more"));
      } else {
        Serial.println("Transmission successful, but the RX node might still be listening.");
      }
    } else {
      Serial.println(F("Transmission failed or timed out. Continuing anyway."));
      radio.flush_tx(); // discard payload(s) that failed to transmit

    }
  }

  delay(50);
  radio.maskIRQ(1, 1, 0); // args = "data_sent", "data_fail", "data_ready"
  radio.startListening(); // put radio in RX mode

  return sent;
}

void printUUID(unsigned char* id) {
  for (int i = 0; i < 5; i++) {
    if (id[i] < 10) Serial.print(0);
    Serial.print(id[i], HEX);
  }
  Serial.println();
}


unsigned char *generateUUID() {
  unsigned char uuid[5] = {0, 0, 0, 0, 0};

  for (int i = 0; i < 5; i++) {
    uuid[i] = Entropy.randomByte();
  }

  return uuid;
}

unsigned short getNameLength(const char *s) {
  unsigned short idx = 0;
  for (; * (s + idx) != '\0'; idx++)
    ;
  return idx;
}
void interruptHandler() {
  // print IRQ status and all masking flags' states
  /*  radio.printPrettyDetails();
    Serial.println("BEFORE");
    radio.printDetails();
    Serial.println(F("\tIRQ pin is actively LOW")); // show that this function was called
    Serial.println("AFtER");
  */
  delayMicroseconds(250);
  bool tx_ds, tx_df, rx_dr;                       // declare variables for IRQ masks
  radio.whatHappened(tx_ds, tx_df, rx_dr);        // get values for IRQ masks
  // whatHappened() clears the IRQ masks also. This is required for
  // continued TX operations when a transmission fails.
  // clearing the IRQ masks resets the IRQ pin to its inactive state (HIGH)

  //  Serial.print(F("\tdata_sent: "));
  //  Serial.print(tx_ds);                            // print "data sent" mask state
  //  Serial.print(F(", data_fail: "));
  //  Serial.print(tx_df);                            // print "data fail" mask state
  //  Serial.print(F(", data_ready: "));
  //  Serial.println(rx_dr);                          // print "data ready" mask state

  if (tx_df)                                      // if TX payload failed
    radio.flush_tx();                             // clear all payloads from the TX FIFO

  printRxFifo();
} // interruptHandler

void printRxFifo() {
  if (radio.available()) {                   // if there is data in the RX FIFO
    // to flush the data from the RX FIFO, we'll fetch it all using 1 buffer

    uint8_t pl_size = sizeof(Message);
    Message rx_fifo[3];       // RX FIFO is full & we know ACK payloads' size
    if (radio.rxFifoFull()) {
      radio.read(&rx_fifo, 3 * pl_size); // this clears the RX FIFO (for this example)
    } else {
      uint8_t i = 0;
      while (radio.available()) {
        radio.read(&rx_fifo + (i), pl_size);
        i++;
      }
    }
    Serial.print(F("Complete RX FIFO: "));
    Serial.println(rx_fifo[0].getPayload(), BIN);                 // print the entire RX FIFO with 1 buffer
//    char payload[16];
//    unsigned char from[5];
//    int i = 0;
//    for (; i < 16 && rx_fifo[i] != '\0'; i++) {
//      payload[i] = rx_fifo[i];
//    }
//    for (int j = 0; j < 5; j++) {
//      from[j] = rx_fifo[i + j];
//    }
//    unsigned short payload_short = 0;
//    for (int j = 0; j < i; j++) {
//      payload_short |= (payload[j] == '-' ? 1 : 0) << (15 - j);
//    }
//    Message message(from, uuid, payload_short, i);

    memory.saveMessage(rx_fifo[0]);

    delay(50);

    Serial.println("Rec Message!");
//    Serial.println(payload_short, BIN);
//    Serial.println(message.getPayloadString());
    keypad.setCursor(0, 0);
    keypad.print("New Message: ");
    keypad.setCursor(0, 1);
    keypad.print("From: ");
//    keypad.print(getContactName(from));
    keypad.print(memory.getNumberMessages());

    tone(buzzer, 1000);
    
    timeout();
  }
}

bool uuidcmp(unsigned char* id, unsigned char* id2) {
  bool equ = true;
  for (int i = 0; i < 5; i++) {
    if (id[i] != id2[i]) equ = false;
  }
  return equ;
}

char* getContactName(unsigned char* id) {
  char* nam = "Unknown";
  for (int i = 0; i < memory.getNumberContacts(); i++) {
    if (uuidcmp(id, memory.getContact(i).getUUID())) {
      nam = memory.getContact(i).getName();
    }
  }
  return nam;
}
