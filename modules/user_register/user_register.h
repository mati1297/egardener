// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_USER_REGISTER_USER_REGISTER_H_
#define MODULES_USER_REGISTER_USER_REGISTER_H_

#include <vector>
#include <string>
#include "mbed.h"

#define MAX_NUMBER_USERS 4
#define MAX_PWD_USER_LENGTH 10


// Stores and manages information of registered users
class UserRegister {
 private:
  std::vector<uint32_t> users;
  std::string pwd;
  bool thereIsPwd;

 public:
  UserRegister();

  // Returns true if register is empty
  bool isEmpty();

  // Returns true is has a set up password
  bool hasPwd();

  // Returns true if user id is registered
  bool isUserRegistered(const std::string&);

  // Returns true if user id is registered
  bool isUserRegistered(uint32_t);

  // Adds user to register
  uint8_t addUser(const std::string&, const std::string&);

  // Adds user to register
  uint8_t addUser(uint32_t, const std::string&);

  // Removes user from register
  bool removeUser(const std::string&);

  // Sets password of register
  bool setPwd(const std::string&, const std::string&);

  // Returns vector with users
  const std::vector<uint32_t>& getUsers();

  // Returns users in vector
  void getUsers(std::vector<std::string>&);
};

#endif  // MODULES_USER_REGISTER_USER_REGISTER_H_
