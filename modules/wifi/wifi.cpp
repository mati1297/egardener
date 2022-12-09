// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <string>
#include <vector>
#include "mbed.h"
#include "wifi.h"

// poner const

WiFi::WiFi(PinName tx_pin, PinName rx_pin, int baud):
           serial(tx_pin, rx_pin, baud), ssid(), pwd() {
  // ver lo de que cuando inicio sin reiniciar el otro se rompe.
  //serial.sync();
  ThisThread::sleep_for(1s); // Doy tiempo a inicializar el serial.
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

  std::string str = "c" + this->ssid + "," + this->pwd + "\n";

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
  unsigned long code = strtoul(response.c_str(), &endptr, 10);

  if (*endptr == response[0])
    return WiFiStatus::WL_FAILED_COMM;

  return (WiFiStatus)code;
}

std::string WiFi::post(const std::string &server, const std::string &request) {
  std::string str = "p" + server + "," + request + "\n";

  serial.write(str.c_str(), str.length());

  std::string response = readToString();

  return response;
}

std::string WiFi::get(const std::string &url) {
  std::string str = "g" + url + "\n";

  serial.write(str.c_str(), str.length());

  std::string response = readToString();

  return response;
}
