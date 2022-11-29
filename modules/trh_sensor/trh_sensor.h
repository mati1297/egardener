#ifndef TRH_SENSOR__H
#define TRH_SENSOR__H

#include "mbed.h"

#define DEFAULT_CONFIG 0x02
#define CONFIG_ERASE_MASK 0x38

#define SOFT_RESET_TIME 20ms
#define MEASURE_TIME 50ms

#define SOFT_RESET_CMD 0xFE
#define READ_REGISTER_CMD 0xE7
#define WRITE_REGISTER_CMD 0xE6
#define MEAS_TEMP_HM_CMD 0xE3
#define MEAS_TEMP_NHM_CMD 0xF3
#define MEAS_RH_HM_CMD 0xE5
#define MEAS_RH_NHM_CMD 0xF5

#define TEMPERATURE_MASK 0xFFFC
#define RH_MASK 0xFFF0

class TRHSensor {
private:
    I2C sensor;
    uint8_t address;

    bool defaultConfig();
    bool softReset();

public:
    TRHSensor(PinName, PinName, uint8_t);
    float senseTemperature();
    float senseHumidity();
};

#endif