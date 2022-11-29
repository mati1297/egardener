#ifndef MEMORY__H
#define MEMORY__H

#include "mbed.h"
#include <string>
#include <vector>

#define MAX_ADDRESS 0x0FFF

// 32kb
class Memory {
private:
    I2C eeprom;
    uint8_t i2cAddress;

public:
    Memory(PinName, PinName, uint8_t);

    // hasta paginas de 32 bytes. 32kb solo importan los 12 lsb
    uint16_t write(uint16_t, const std::vector<uint8_t>&);

    //solo importan los 12lsb. Se lee lo que entre en el vector. se devuelve la cantidad de bytes leidos.
    uint16_t read(uint16_t, std::vector<uint8_t>&);

    static std::vector<uint8_t> floatToBytes(float number);
    static float bytesToFloat(const std::vector<uint8_t>& vector);
    static std::vector<uint8_t> intToBytes(int number);
    static int bytesToInt(const std::vector<uint8_t>& vector);
};

#endif