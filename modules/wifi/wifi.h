#ifndef WIFI__H
#define WIFI__H

#include <string>
#include "mbed.h"

#define BUFFER_SIZE 100

class WiFi {
private:
    BufferedSerial serial;
    std::string ssid;
    std::string pwd;

    std::string readToString();
    std::string readNBytes(size_t size);

public:
    WiFi(PinName, PinName, int);
    std::string connect(const std::string&, const std::string&);
    uint8_t getStatus();
    std::string disconnect();
    std::string post(const std::string&);
    std::string get(const std::string&);
};



#endif