#ifndef Message_hpp
#define Message_hpp

#ifndef UUID_LENGTH
#define UUID_LENGTH 5
#endif
#define MESSAGE_MAX_LENGTH 16

class Message {
  public:
	Message();
	Message(unsigned char *from, unsigned char *to, unsigned short payload, unsigned char length);
	Message(unsigned char *from, unsigned char *to, char const *message);
	void setLength(unsigned char length);
	void setTo(unsigned char *to);
	void setFrom(unsigned char *from);
	void setPayload(unsigned short payload);
	unsigned char getLength();
	unsigned char *getTo();
	unsigned char *getFrom();
	unsigned short getPayload();
	char *getPayloadString();

  protected:
	unsigned short stringToPayload(char const *message);
	char *payloadToString(unsigned short payload, unsigned char length);

  private:
	unsigned char *sender;
	unsigned char *receiver;
	unsigned char length;
	unsigned short payload;
};
#endif /* Message_hpp */