#include "Contact.hpp"
#include "LCDKeypad.hpp"
#include "Message.hpp"
#include "Memory.hpp"

LCDKeypad keypad(8, 9, 4, 5, 6, 7, A0);

unsigned char uuid[5] = {0, 1, 2, 3, 4};

void setup() {
	keypad.begin(16, 2);
  
	Contact contact(uuid, "Carl");
	Message message(uuid, uuid, "--..--");

  Memory memory(contact);
}

void loop() {}
