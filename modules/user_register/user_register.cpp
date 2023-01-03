// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <string>
#include <vector>
#include "mbed.h"
#include "user_register.h"


UserRegister::UserRegister(): users(), pwd(), thereIsPwd(false) {}

bool UserRegister::isEmpty() {
  return users.empty();
}

bool UserRegister::hasPwd() {
  return thereIsPwd;
}

bool UserRegister::isUserRegistered(const std::string& user) {
  return isUserRegistered(strtoll(user.c_str(), nullptr, 10));
}

bool UserRegister::isUserRegistered(uint32_t user_id) {
  return std::find(users.begin(), users.end(), user_id) != users.end();
}

uint8_t UserRegister::addUser(uint32_t user, const std::string& pwd) {
  if (isUserRegistered(user))
    return 1;

  if (thereIsPwd && pwd != this->pwd)
    return 2;

  if (users.size() > MAX_NUMBER_USERS)
    return 3;

  users.push_back(user);
  return 0;
}

uint8_t UserRegister::addUser(const std::string& user, const std::string& pwd) {
  return addUser(strtoll(user.c_str(), nullptr, 10), pwd);
}

bool UserRegister::removeUser(const std::string& user) {
  if (!isUserRegistered(user))
    return false;
  for (auto it = users.begin(); it != users.end(); it++) {
    if (*it == strtoll(user.c_str(), nullptr, 10)) {
      users.erase(it);
      break;
    }
  }
  return true;
}

bool UserRegister::setPwd(const std::string& old_pwd, const std::string& pwd) {
  if (thereIsPwd && old_pwd != this->pwd)
    return false;

  if (pwd.length() > MAX_PWD_USER_LENGTH)
    return false;

  if (pwd.length() == 0) {
    thereIsPwd = false;
    return true;
  }

  this->pwd = pwd;
  thereIsPwd = true;

  return true;
}

const std::vector<uint32_t>& UserRegister::getUsers() {
  return users;
}

void UserRegister::getUsers(std::vector<std::string>& output) {
  for (auto user : users)
    output.push_back(to_string(user));
}
