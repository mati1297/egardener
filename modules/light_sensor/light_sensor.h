#ifndef LIGHT_SENSOR__H 
#define LIGHT_SENSOR__H

#include "mbed.h"

#define VOLTAGE_REF_DEFAULT 3.3
#define AVERAGE_POINTS_DEFAULT 10

class LightSensor {
private:
    AnalogIn analog;
    float max, min;
    uint8_t averagePoints;

    float senseRaw();

public:
    LightSensor(PinName, float vref = VOLTAGE_REF_DEFAULT, uint8_t averagePoints = AVERAGE_POINTS_DEFAULT);
    void calibrateMax();
    void calibrateMin();

    float sense(bool raw = false);

    float getMax();
    float getMin();
    bool setMaxAndMin(float, float);
};

#endif