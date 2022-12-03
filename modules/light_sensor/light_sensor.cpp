#include "mbed.h"
#include "light_sensor.h"


LightSensor::LightSensor(PinName analog, float vref, uint8_t averagePoints): 
                        analog(analog, vref), max(-1), min(-1), averagePoints((averagePoints > 0) ? averagePoints : 1) {}


float LightSensor::sense(bool raw) {
    float rawValue = senseRaw();

    if (raw)
        return rawValue;

    if (max == -1 || min == -1)
        return -1;

    rawValue = (rawValue <= max) ? rawValue : max;
    rawValue = (rawValue >= min) ? rawValue : min;

    return (rawValue - min) / (max - min);
}

bool LightSensor::setMaxAndMin(float max, float min) {
    if (max <= min)
        return false;
    this->max = max;
    this->min = min;

    return true;
}

float LightSensor::getMax() {
    return max;
}

float LightSensor::getMin() {
    return min;
}

float LightSensor::senseRaw() {
    float total = 0;
    if (averagePoints < 1)
        averagePoints = 1;
    for (uint16_t i = 0; i < averagePoints; i++) {
        total += analog.read();
    }
    return (total / averagePoints) * -1 + 1;
}