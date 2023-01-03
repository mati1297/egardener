// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <string>
#include <vector>
#include "mbed.h"
#include "wifi.h"

WiFi::WiFi(PinName tx_pin, PinName rx_pin, int baud):
           serial(tx_pin, rx_pin, baud), ssid(), pwd(), asAP(false) {
  ThisThread::sleep_for(1s);
}

std::string WiFi::readNBytes(size_t size) {
  size_t readed, total_readed = 0;
  std::vector<char> buffer(size + 1);

  while (total_readed < size) {
    readed = serial.read(&buffer[total_readed], size - total_readed);
    if (readed <= 0)
      break;
    total_readed += readed;
  }

  return std::string(&buffer[0]);
}

std::string WiFi::readToString() {
  std::string size_buffer = readNBytes(6);

  char *endptr;
  size_t size = strtoul(size_buffer.c_str(), &endptr, 10);

  if (*endptr == size_buffer[0])
    return std::string();

  std::string buffer = readNBytes(size);
  return buffer;
}

std::string WiFi::connect(const std::string &ssid, const std::string &pwd) {
  this->ssid = ssid;
  this->pwd = pwd;

  restart();

  std::string str = "c" + this->ssid + (SEPARATOR_CHAR + this->pwd) + "\n";

  serial.write(str.c_str(), str.length());

  std::string response = readToString();

  return response;
}

std::string WiFi::disconnect() {
  std::string str = "d\n";

  serial.write(str.c_str(), str.length());

  std::string response = readToString();

  return response;
}

WiFiStatus WiFi::getStatus() {
  std::string str = "w\n";
  serial.write(str.c_str(), str.length());

  std::string response = readToString();

  char * endptr;
  uint64_t code = strtoul(response.c_str(), &endptr, 10);

  if (*endptr == response[0])
    return WiFiStatus::WL_FAILED_COMM;

  return (WiFiStatus)code;
}

std::string WiFi::post(const std::string &server, const std::string &request) {
  if (asAP)
    return "";

  std::string str = "p" + server + (SEPARATOR_CHAR + request) + "\n";

  serial.write(str.c_str(), str.length());

  std::string response = readToString();

  return response;
}

std::string WiFi::get(const std::string &url) {
  if (asAP)
    return "";

  std::string str = "g" + url + "\n\0";

  serial.write(str.c_str(), str.length());

  std::string response = readToString();

  return response;
}

void WiFi::setAsAP() {
  std::string str = "a\n";

  serial.write(str.c_str(), str.length());

  asAP = true;
}

void WiFi::getSsidAndPwd(std::string& ssid, std::string& pwd) {
  std::string str = "f\n";

  serial.write(str.c_str(), str.length());

  ssid = readToString();
  pwd = readToString();

  ssid[ssid.length() - 2] = 0;
  pwd[pwd.length() - 2] = 0;

  asAP = false;
}

void WiFi::restart() {
  std::string str = "r\n";

  serial.write(str.c_str(), str.length());

  readToString();

  asAP = false;
}
