#include <Arduino.h>
#include "Memory.hpp"
#include "Contact.hpp"
#include "Message.hpp"

Memory::Memory() {
//    if (!hasSchema()) {
//        setSchema();
//        clearMessages();
//        clearContacts();        
//    }
}

Memory::Memory(Contact node) {
    if (!hasSchema()) {
        setSchema();
        clearMessages();
        clearContacts();
    }
    saveContactToMemory(node, NODE_IDENTIFIER);
}

unsigned char* Memory::getNodeUUID() {
    return getUUIDFromMemory(NODE_IDENTIFIER); // could load whole contact...
}

char* Memory::getNodeName() {
    return getContactFromMemory(NODE_IDENTIFIER).getName();
}

unsigned short Memory::getNumberContacts() {
    return read(CONTACT_COUNTER);
}

unsigned short Memory::getNumberMessages() {
    return read(MESSAGE_COUNTER);
}

Contact Memory::getContact(unsigned short index) {
    return getContactFromMemory(CONTACT_START + (index * CONTACT_SIZE));
}

Message Memory::getMessage(unsigned short index) {
    return getMessageFromMemory(MESSAGE_START + (index * MESSAGE_SIZE));
}

bool Memory::saveContact(Contact contact) {
    if (getNumberContacts() < MAX_CONTACTS) {
        saveContactToMemory(contact, CONTACT_START + (getNumberContacts() * CONTACT_SIZE));
        write(CONTACT_COUNTER, getNumberContacts() + 1);
        return true;
    }
    return false;
}

void Memory::saveMessage(Message message) {
	saveMessageToMemory(message, MESSAGE_START + (getNumberMessages() * MESSAGE_SIZE));
	if (getNumberMessages() < MAX_MESSAGES) {        
        write(MESSAGE_COUNTER, getNumberMessages() + 1);
    }
}

void Memory::saveNodeInformation(Contact contact) {
    saveContactToMemory(contact, NODE_IDENTIFIER);
}

bool Memory::hasSchema() {
    bool hasSchema = true;

    hasSchema &= read(INIT_START) == (0xFF & (INIT_FLAG >> 16));
	hasSchema &= read(INIT_START + 1) == (0xFF & (INIT_FLAG >> 8));
	hasSchema &= read(INIT_START + 2) == (0xFF & (INIT_FLAG));

    hasSchema &= read(CONTACT_FLAG_START) == (0xFF & (CONTACT_FLAG >> 8));
    hasSchema &= read(CONTACT_FLAG_START + 1) == (0xFF & (CONTACT_FLAG));

    hasSchema &= read(MESSAGE_FLAG_START) == (0xFF & (MESSAGE_FLAG >> 8));
    hasSchema &= read(MESSAGE_FLAG_START + 1) == (0xFF & (MESSAGE_FLAG));

	return hasSchema;
}

void Memory::setSchema() {
    write(INIT_START, (0xFF & (INIT_FLAG >> 16)));
    write(INIT_START + 1, (0xFF & (INIT_FLAG >> 8)));
    write(INIT_START + 2, (0xFF & (INIT_FLAG)));

    write(CONTACT_FLAG_START, (0xFF & (CONTACT_FLAG >> 8)));
    write(CONTACT_FLAG_START + 1, (0xFF & (CONTACT_FLAG)));

    write(MESSAGE_FLAG_START, (0xFF & (MESSAGE_FLAG >> 8)));
    write(MESSAGE_FLAG_START + 1, (0xFF & (MESSAGE_FLAG)));
}

void Memory::clearContacts() {
    for (unsigned short i = 0; i < MAX_CONTACTS; i++) {
        write(CONTACT_START + (i * CONTACT_SIZE), 0xFF);
    }
    write(CONTACT_COUNTER, 0);
}

void Memory::clearMessages() {
    for (unsigned short i = 0; i < MAX_MESSAGES; i++) {
        write(MESSAGE_START + (i * MESSAGE_SIZE), 0xFF);
    }
    write(MESSAGE_COUNTER, 0);
}

unsigned short Memory::getMessagePointerOffset() {
    return MESSAGE_START + (getNumberMessages() * MESSAGE_SIZE);
}

unsigned char Memory::read(unsigned int addr) {
	// wait for last read to complete, may be neccessary
	delay(5);

	// clear eeprom address
	EEARH &= 0x00;
	EEARL &= 0x00;
	// set address registers
	EEARH |= (unsigned char)(addr >> 8) & 0x03;
	EEARL |= (unsigned char)addr;

	// read EEPROM
	EECR |= 0x01;

	return EEDR;
}

void Memory::write(unsigned int addr, unsigned char data) {
	// wait for last to complete
	delay(5);

	// clear eeprom address
	EEARH &= 0x00;
	EEARL &= 0x00;
	// set address registers
	EEARH |= (unsigned char)(addr >> 8) & 0x03;
	EEARL |= (unsigned char)addr;

	// write data to buffer
	EEDR &= 0x00;
	EEDR |= data;

	// write to EEPROM
	EECR |= 0x04;
	EECR |= 0x02;
}

Contact Memory::getContactFromMemory(unsigned short addr) {
    unsigned char *uuid = getUUIDFromMemory(addr);
    char *name = new char[NAME_LENGTH + 1];
    for(int i = 0; i < NAME_LENGTH; i++) {
        name[i] = read(addr + i + UUID_LENGTH);
    }
    name[NAME_LENGTH] = '\0';
    return Contact(uuid, name);
}

Message Memory::getMessageFromMemory(unsigned short addr) {
    unsigned char *sender = getUUIDFromMemory(addr);
    unsigned char *receiver = getUUIDFromMemory(addr + UUID_LENGTH);
    unsigned char length = read(addr + UUID_LENGTH + UUID_LENGTH);
	unsigned short payload = 0;
	payload |= read(addr + UUID_LENGTH + UUID_LENGTH + 1);
    payload |= read(addr + UUID_LENGTH + UUID_LENGTH + 2) << 8;
    return Message(sender, receiver, payload, length);
}

unsigned char *Memory::getUUIDFromMemory(unsigned short addr) {
    unsigned char *uuid = new unsigned char[UUID_LENGTH];
    for(int i = 0; i < UUID_LENGTH; i++) {
        uuid[i] = read(addr + i);
    }
    return uuid;
}

void Memory::saveContactToMemory(Contact contact, unsigned short addr) {
	saveUUIDToMemory(contact.getUUID(), addr);
    for(int i = 0; i < NAME_LENGTH; i++) {
        write(addr + i + UUID_LENGTH, contact.getName()[i]);
    }
}

void Memory::saveMessageToMemory(Message message, unsigned short addr) {
	saveUUIDToMemory(message.getFrom(), addr);
    saveUUIDToMemory(message.getTo(), addr + UUID_LENGTH);
    write(addr + UUID_LENGTH + UUID_LENGTH, message.getLength());
    write(addr + UUID_LENGTH + UUID_LENGTH + 1, message.getPayload() & 0xFF);
    write(addr + UUID_LENGTH + UUID_LENGTH + 2, message.getPayload() >> 8);
}

void Memory::saveUUIDToMemory(unsigned char *uuid, unsigned short addr) {
    for(int i = 0; i < UUID_LENGTH; i++) {
        write(addr + i, uuid[i]);
    }
}
