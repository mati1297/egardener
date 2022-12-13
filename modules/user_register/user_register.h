#ifndef MODULES_USER_REGISTER_USER_REGISTER_H_
#define MODULES_USER_REGISTER_USER_REGISTER_H_

#include <vector>
#include <string>
#include "mbed.h"

#define MAX_NUMBER_USERS 4
#define MAX_PWD_USER_LENGTH 10

class UserRegister {
 private:
  std::vector<uint32_t> users;
  std::string pwd;
  bool thereIsPwd;

 public:
  UserRegister();
  bool isEmpty();
  bool hasPwd();
  bool isUserRegistered(const std::string&);
  bool isUserRegistered(uint32_t);
  uint8_t addUser(const std::string&, const std::string&);
  uint8_t addUser(uint32_t, const std::string&);
  bool removeUser(const std::string&);
  bool setPwd(const std::string&, const std::string&);
  const std::vector<uint32_t>& getUsers();
  void getUsers(std::vector<std::string>&);
};



#endif  // MODULES_USER_REGISTER_USER_REGISTER_H_