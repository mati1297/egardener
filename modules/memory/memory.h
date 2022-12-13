// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_MEMORY_MEMORY_H_
#define MODULES_MEMORY_MEMORY_H_

#include <string>
#include <vector>
#include "mbed.h"

// Entiendo que esto es para las high-endurance
#define MAX_ADDRESS 0x0FFF
#define MAX_PAGE_SIZE 32
#define WRITE_TIME 20ms

// 32kb
class Memory {
 private:
  I2C eeprom;
  uint8_t i2cAddress;

 public:
  Memory(PinName, PinName, uint8_t);

  // hasta paginas de 32 bytes. 32kb solo importan los 12 lsb
  uint16_t write(uint16_t, const std::vector<uint8_t> &);

  // solo importan los 12lsb. Se lee lo que entre en el vector.
  // se devuelve la cantidad de bytes leidos.
  uint16_t read(uint16_t, std::vector<uint8_t> &);
  std::vector<uint8_t> read(uint16_t address_from, uint16_t address_to);

  bool write(uint16_t, float);
  bool read(uint16_t, float &);
  bool write(uint16_t, bool);
  bool read(uint16_t, bool &);
  bool write(uint16_t, int);
  bool read(uint16_t, int &);
  bool write(uint16_t, uint8_t);
  bool read(uint16_t, uint8_t&);
  bool write(uint16_t, uint32_t);
  bool read(uint16_t, uint32_t&);
  bool write(uint16_t, char);
  bool read(uint16_t, char&);
  bool write(uint16_t, const std::string &);
  bool read(uint16_t, std::string &);
};

#endif  // MODULES_MEMORY_MEMORY_H_
