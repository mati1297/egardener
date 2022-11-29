#include "trh_sensor.h"
#include "mbed.h"

TRHSensor::TRHSensor(PinName SDA, PinName SCL, uint8_t address): sensor(SDA, SCL), address(address << 1) {
    softReset();
    defaultConfig();
}

float TRHSensor::senseTemperature() {
    sensor.start();
    sensor.write(address);
    sensor.write(MEAS_TEMP_NHM_CMD);

    uint8_t ready = 0, timeout_counter = 0;

    do {
        ThisThread::sleep_for(MEASURE_TIME);
        sensor.start();
        ready = sensor.write(address | 1);
        if (timeout_counter > 10) {
            printf("Sali por timeout\n");
            break;
        }
        timeout_counter++;
    } while (ready != 1);

    uint8_t bytemsb = sensor.read(1);
    uint8_t bytelsb = sensor.read(1);
    uint8_t checksum = sensor.read(0);
    sensor.stop();

    int temp = (bytemsb << 8 | bytelsb) & TEMPERATURE_MASK;

    float temp_total = -46.85 + 175.72 * temp / (1 << 16);

    return temp_total;
}


bool TRHSensor::softReset() {
    int count = 0;

    sensor.start();
    count += sensor.write(address);
    count += sensor.write(SOFT_RESET_CMD);
    sensor.stop();

    ThisThread::sleep_for(SOFT_RESET_TIME);

    return count == 2;
}


// Se podria configurar
bool TRHSensor::defaultConfig() {
    int count = 0;
    uint8_t config;

    sensor.start();
    count += sensor.write(address);
    count += sensor.write(READ_REGISTER_CMD);
    sensor.start();
    count += sensor.write(address | 1);
    config = sensor.read(0);

    sensor.start();
    count += sensor.write(address);
    count += sensor.write(WRITE_REGISTER_CMD);
    count += sensor.write((config & CONFIG_ERASE_MASK) | DEFAULT_CONFIG);
    sensor.stop();

    return count == 6;
}

