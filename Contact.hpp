#ifndef Contact_hpp
#define Contact_hpp

#define UUID_LENGTH 5
#define NAME_LENGTH 10

class Contact {
  public:
	Contact();
	Contact(unsigned char *givenUUID, char const *givenName);
	Contact(unsigned char *givenUUID, char givenName);
	void setUUID(unsigned char *givenUUID);
	void setName(char const *givenName);
	void setName(char givenName);
	unsigned char *getUUID();
	char *getName();

  private:
	unsigned char* UUID;
	char *name;
};
#endif /* Contact_hpp */
