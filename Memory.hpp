#ifndef Memory_hpp
#define Memory_hpp

#include "Contact.hpp"
#include "Message.hpp"

#define MAX_CONTACTS 10
#define MAX_MESSAGES 20

#define INIT_FLAG 0xC0FFEE
#define CONTACT_FLAG 0xFACE
#define MESSAGE_FLAG 0xCA11

#define INIT_START 0
#define CONTACT_FLAG_START 18
#define MESSAGE_FLAG_START 171
#define CONTACT_START 21
#define MESSAGE_START 174

#define NODE_IDENTIFIER 3
#define CONTACT_COUNTER 20
#define MESSAGE_COUNTER 173

#define CONTACT_SIZE sizeof(Contact)
#define MESSAGE_SIZE sizeof(Message)


class Memory {
  public:
	Memory();
	Memory(Contact node);
	unsigned char *getNodeUUID();
	char *getNodeName();
	unsigned short getNumberContacts();
	unsigned short getNumberMessages();
	Contact getContact(unsigned short index);
	Message getMessage(unsigned short index);
	bool saveContact(Contact contact);
	void saveMessage(Message message);
	void saveNodeInformation(Contact contact);

  protected:
	bool hasSchema();
	void setSchema();
	void clearMessages();
	void clearContacts();
	unsigned short getMessagePointerOffset();

  public:
	unsigned char read(unsigned int uiAddress);
	void write(unsigned int uiAddress, unsigned char data);
	Contact getContactFromMemory(unsigned short addr);
	Message getMessageFromMemory(unsigned short addr);
	unsigned char *getUUIDFromMemory(unsigned short addr);
	void saveContactToMemory(Contact contact, unsigned short addr);
	void saveMessageToMemory(Message message, unsigned short addr);
	void saveUUIDToMemory(unsigned char *uuid, unsigned short addr);
};

#endif /* Memory_hpp */
