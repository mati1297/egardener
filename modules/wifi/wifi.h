// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_WIFI_WIFI_H_
#define MODULES_WIFI_WIFI_H_

#include <string>
#include "mbed.h"

#define BUFFER_SIZE 100

enum WiFiStatus {
  WL_NO_SHIELD = 255,
  WL_IDLE_STATUS = 0,
  WL_NO_SSID_AVAIL = 1,
  WL_SCAN_COMPLETED = 2,
  WL_CONNECTED = 3,
  WL_CONNECT_FAILED = 4,
  WL_CONNECTION_LOST = 5,
  WL_DISCONNECTED = 6,
  WL_FAILED_COMM = 7
};

class WiFi {
 private:
  BufferedSerial serial;
  std::string ssid;
  std::string pwd;

  std::string readToString();
  std::string readNBytes(size_t size);

 public:
  WiFi(PinName, PinName, int);
  std::string connect(const std::string &, const std::string &);
  WiFiStatus getStatus();
  std::string disconnect();
  std::string post(const std::string &, const std::string&);
  std::string get(const std::string &);
};

#endif  // MODULES_WIFI_WIFI_H_
