// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_WIFI_WIFI_H_
#define MODULES_WIFI_WIFI_H_

#include <string>
#include "mbed.h"

#define BUFFER_SIZE 100
#define SEPARATOR_CHAR std::static_cast<char>(254)

enum WiFiStatus {
  WL_NO_SHIELD = 255,
  WL_IDLE_STATUS = 0,
  WL_NO_SSID_AVAIL = 1,
  WL_SCAN_COMPLETED = 2,
  WL_CONNECTED = 3,
  WL_CONNECT_FAILED = 4,
  WL_CONNECTION_LOST = 5,
  WL_DISCONNECTED = 6,
  WL_FAILED_COMM = 7,
  WL_AS_AP = 10
};


// WiFi driver. Drives ESP32 through serial port
class WiFi {
 private:
  BufferedSerial serial;
  std::string ssid;
  std::string pwd;

  std::string readToString();
  std::string readNBytes(size_t size);

  bool asAP;

  void restart();

 public:
  WiFi(PinName, PinName, int);

  // Conects WiFi to a network with ssid and password
  std::string connect(const std::string &, const std::string &);

  // Returns WiFi status
  WiFiStatus getStatus();

  // Disconnects WiFi
  std::string disconnect();

  // Sends post request through WiFi
  std::string post(const std::string &, const std::string&);

  // Sends get request through WiFi
  std::string get(const std::string &);

  // Sets WiFi module as Access Point
  void setAsAP();

  // Returns SSID and password set in WiFi module
  void getSsidAndPwd(std::string&, std::string&);
};

#endif  // MODULES_WIFI_WIFI_H_
