#include "mbed.h"
#include "memory.h"
#include <vector>

Memory::Memory(PinName SDA, PinName SCL, uint8_t address): eeprom(SDA, SCL), i2cAddress(address << 1) {}

uint16_t Memory::write(uint16_t address, const std::vector<uint8_t>& vector) {
    size_t write_length = vector.size();

    if (address > MAX_ADDRESS)
        return 0;
    if (write_length > (MAX_ADDRESS - address + 1))
        write_length = MAX_ADDRESS - address + 1;

    uint16_t count = 0;

    eeprom.start();
    count += eeprom.write(i2cAddress | 0);
    count += eeprom.write(address >> 8 & 0x0F);
    count += eeprom.write(address & 0xFF);

    for (uint16_t i = 0; i < write_length; i++)
        count += eeprom.write(vector[i]);
    
    eeprom.stop();

    if (count != write_length + 3)
        return 0;

    return write_length;
}

uint16_t Memory::read(uint16_t address, std::vector<uint8_t>& vector) {
    size_t read_length = vector.size();

    if (address > MAX_ADDRESS)
        return 0;
    if (read_length > (MAX_ADDRESS - address + 1))
        read_length = MAX_ADDRESS - address + 1;

    uint16_t count = 0;

    eeprom.start();
    count += eeprom.write(i2cAddress | 0);
    count += eeprom.write(address >> 8 & 0x0F);
    count += eeprom.write(address & 0xFF);
    eeprom.start();
    count += eeprom.write(i2cAddress | 1);
    for (uint16_t i = 0; i < read_length - 1; i++)
        vector[i] = eeprom.read(1);
    vector[read_length - 1] = eeprom.read(0);
    eeprom.stop();

    if (count != 4)
        return 0;

    return read_length;
}

std::vector<uint8_t> Memory::floatToBytes(float number){
    std::vector<uint8_t> vector(sizeof(float));

    memcpy(&vector[0], &number, sizeof(float));

    return vector;
}

// toma solo los primeros 4 si es mas grande
float Memory::bytesToFloat(const std::vector<uint8_t>& vector) {
    if (vector.size() < sizeof(float))
        return 0; //validar de otra manera.

    float out = 0;

    memcpy(&out, &vector[0], sizeof(float));

    return out;
}

std::vector<uint8_t> Memory::intToBytes(int number){
    std::vector<uint8_t> vector(sizeof(int));

    memcpy(&vector[0], (uint8_t*)&number, sizeof(int));

    return vector;
}

// toma solo los primeros 4 si es mas grande
int Memory::bytesToInt(const std::vector<uint8_t>& vector) {
    if (vector.size() < sizeof(int))
        return 0; //validar de otra manera.

    int out = 0;

    memcpy(&out, &vector[0], sizeof(int));

    return out;
}