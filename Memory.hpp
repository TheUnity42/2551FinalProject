#ifndef Memory_hpp
#define Memory_hpp

#include "Contact.hpp"
#include "Message.hpp"

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

  private:
	const unsigned short MAX_CONTACTS = 10;
	const unsigned short MAX_MESSAGES = 20;
};

#endif /* Memory_hpp */