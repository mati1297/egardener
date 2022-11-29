#include <string>
#include <vector>
#include "mbed.h"
#include "wifi.h"

// poner const

WiFi::WiFi(PinName tx_pin, PinName rx_pin, int baud):
            serial(tx_pin, rx_pin, baud), ssid(), pwd() {
    // ver lo de que cuando inicio sin reiniciar el otro se rompe.
    serial.sync();
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

    char * endptr;
    size_t size = strtoul(size_buffer.c_str(), &endptr, 10);

    // verificar endptr

    std::string buffer = readNBytes(size);
    return buffer;
}

std::string WiFi::connect(const std::string& ssid, const std::string& pwd) {
    this->ssid = ssid;
    this->pwd = pwd;
    std::string str = "c" + ssid + "," + pwd + "\n";
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

uint8_t WiFi::getStatus() {
    std::string str = "w\n";
    serial.write(str.c_str(), str.length());

    std::string response = readToString();

    return stoul(response);
}

std::string WiFi::post(const std::string& url) {
    std::string str = "p" + url + "\n";

    serial.write(str.c_str(), str.length());

    std::string response = readToString();

    return response;
}

std::string WiFi::get(const std::string& url) {
    std::string str = "g" + url + "\n";

    serial.write(str.c_str(), str.length());

    std::string response = readToString();

    return response;
}


