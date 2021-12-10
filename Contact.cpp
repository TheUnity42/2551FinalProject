#include "Contact.hpp"
#include <string.h>

Contact::Contact() {
  this->name[0] = '\0';
  this->UUID[0] = 0;
}

Contact::Contact(unsigned char *givenUUID, char const *givenName) {
  //this->UUID = givenUUID;
  memcpy(UUID, givenUUID, 5);
  for(int i = 0; i < 10 && (givenName[i] != '\0'); i++){
     this->name[i] = givenName[i];
  }
}

Contact::Contact(unsigned char *givenUUID, char givenName) {
 // this->UUID = givenUUID;
  memcpy(UUID, givenUUID, 5);
  this->name[0] = givenName;
  this->name[1] = '\0';
}

void Contact::setUUID(unsigned char *givenUUID) {
  //this->UUID = givenUUID;
  memcpy(UUID, givenUUID, 5);
}

void Contact::setName(char const *givenName) {
  strcpy(this->name, givenName);
}

void Contact::setName(char givenName) {
  this->name[0] = givenName;
  this->name[1] = '\0';
}

unsigned char *Contact::getUUID() {
  return this->UUID;
}

char *Contact::getName() {
  return name;
}
