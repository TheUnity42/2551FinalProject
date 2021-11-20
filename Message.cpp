#include "Message.hpp"
#include <string.h>

Message::Message() {
	this->sender = new unsigned char[UUID_LENGTH];
	this->receiver = new unsigned char[UUID_LENGTH];
	this->payload = 0;
	this->length = 0;
}

Message::Message(unsigned char *from, unsigned char *to, unsigned short payload,
				 unsigned char length) {
	this->sender = from;
	this->receiver = to;
	this->payload = payload;
	this->length = length;
}

Message::Message(unsigned char *from, unsigned char *to, char const *message) {
	this->sender = from;
	this->receiver = to;
	this->payload = stringToPayload(message);
	this->length = strlen(message);
}

void Message::setLength(unsigned char length) { this->length = length; }

void Message::setTo(unsigned char *to) { this->receiver = to; }

void Message::setFrom(unsigned char *from) { this->sender = from; }

void Message::setPayload(unsigned short payload) { this->payload = payload; }

unsigned char Message::getLength() { return this->length; }

unsigned char *Message::getTo() { return this->receiver; }

unsigned char *Message::getFrom() { return this->sender; }

unsigned short Message::getPayload() { return this->payload; }

char *Message::getPayloadString() { return payloadToString(this->payload, this->length); }

unsigned short Message::stringToPayload(char const *message) {
	unsigned short payload = 0;
	for (int i = 0; i < strlen(message) && i < MESSAGE_MAX_LENGTH; i++) {
		payload |= (message[i] == '-') << i;
	}
	return payload;
}

char *Message::payloadToString(unsigned short payload, unsigned char length) {
	char *message = new char[length + 1];
	for (int i = 0; i < length; i++) {
		message[i] = (payload & (1 << i)) ? '-' : '.';
	}
	message[length] = '\0';
	return message;
}