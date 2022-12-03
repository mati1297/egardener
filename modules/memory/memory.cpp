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
    if (write_length > MAX_PAGE_SIZE)
        return 0;

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

    ThisThread::sleep_for(WRITE_TIME);

    return write_length;
}

uint16_t Memory::read(uint16_t address, std::vector<uint8_t>& vector) {
    size_t read_length = vector.size();

    if (address > MAX_ADDRESS)
        return 0;
    if (read_length > (MAX_ADDRESS - address + 1))
        read_length = MAX_ADDRESS - address + 1;
    if (read_length > MAX_PAGE_SIZE)
        return 0;

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

// address_to no se lee.
std::vector<uint8_t> Memory::read(uint16_t address_from, uint16_t address_to) {
    size_t read_length = address_to - address_from;

    std::vector<uint8_t> vector(read_length);

    if (address_from > MAX_ADDRESS)
        return std::vector<uint8_t>();
    if (address_to > address_from)
        return std::vector<uint8_t>();
    if (read_length > (MAX_ADDRESS - address_from + 1))
        read_length = MAX_ADDRESS - address_from + 1;
    if (read_length > MAX_PAGE_SIZE)
        return std::vector<uint8_t>();

    uint16_t count = 0;

    eeprom.start();
    count += eeprom.write(i2cAddress | 0);
    count += eeprom.write(address_from >> 8 & 0x0F);
    count += eeprom.write(address_from & 0xFF);
    eeprom.start();
    count += eeprom.write(i2cAddress | 1);
    for (uint16_t i = 0; i < read_length - 1; i++)
        vector[i] = eeprom.read(1);
    vector[read_length - 1] = eeprom.read(0);
    eeprom.stop();

    if (count != 4)
        return std::vector<uint8_t>();

    return vector;
}

bool Memory::write(uint16_t address, float number) {
    std::vector<uint8_t> vector(sizeof(float));

    memcpy(&vector[0], &number, sizeof(float));

    return write(address, vector) == sizeof(float);
}

bool Memory::read(uint16_t address, float& number) {
    std::vector<uint8_t> vector(sizeof(float));

    if (read(address, vector) != sizeof(float))
        return false;

    memcpy(&number, &vector[0], sizeof(float));

    return true;
}

bool Memory::write(uint16_t address, bool boolean) {
    std::vector<uint8_t> vector(sizeof(bool));

    memcpy(&vector[0], &boolean, sizeof(bool));

    return write(address, vector) == sizeof(bool);
}

bool Memory::read(uint16_t address, bool& boolean) {
    std::vector<uint8_t> vector(sizeof(bool));

    if (read(address, vector) != sizeof(bool))
        return false;

    memcpy(&boolean, &vector[0], sizeof(bool));

    return true;
}

bool Memory::write(uint16_t address, int number) {
    std::vector<uint8_t> vector(sizeof(int));

    memcpy(&vector[0], (uint8_t*)&number, sizeof(int));

    return write(address, vector) == sizeof(int);
}

bool Memory::read(uint16_t address, int& number) {
    std::vector<uint8_t> vector(sizeof(int));

    if (read(address, vector) != sizeof(int))
        return false;

    memcpy(&number, &vector[0], sizeof(int));

    return true;
}

bool Memory::write(uint16_t address, const std::string& string) {
    size_t length = string.length();
    std::vector<uint8_t> vector(length + 1);
    vector[length] = 0;

    memcpy(&vector[0], (uint8_t*)string.c_str(), length + 1);

    return write(address, vector) == length + 1;
}

bool Memory::read(uint16_t address, std::string& string) {
    std::vector<uint8_t> vector(MAX_PAGE_SIZE);

    if (read(address, vector) != MAX_PAGE_SIZE)
        return false;

    for (auto c: vector){
        if (!c)
            break;
        string += c;
    }

    return true;
}