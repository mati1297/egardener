#include "mbed.h"
#include "aux_functions.h"
#include <string>


std::string floatToString(float number, uint8_t presicion) {
    uint8_t length = (presicion > 0) ? presicion + 1 : 0;
    length += floatIntegersCount(number);
    lenght += 
}

uint8_t floatIntegersCount(float number) {
    int numberInt = number;

    uint8_t count;

    if (number == 0)
        return 0;

    if (number < 0)
        number = -number;

    while (number > 0) {
        number /= 10;
        count++;
    }

    return count;
}

