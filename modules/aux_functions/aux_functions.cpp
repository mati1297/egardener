#include "mbed.h"
#include "aux_functions.h"
#include <string>
#include <vector>

uint8_t floatIntegersCount(float number) {
    int numberInt = number;

    uint8_t count = 0;

    if (numberInt == 0)
        return 1;

    if (numberInt < 0)
        numberInt = -numberInt;

    while (numberInt > 0) {
        numberInt /= 10;
        count++;
    }

    return count;
}

std::string floatToString(float number, uint8_t precision) {
    uint8_t length = (precision > 0) ? precision + 1 : 0;
    length += floatIntegersCount(number);
    length += (number < 0) ? 1 : 0;

    std::vector<char> output(length + 1);
    char buffer_format[6];

    snprintf(buffer_format, 5, "%%.%uf", precision);

    snprintf(&output[0], length + 1, buffer_format, number);

    return std::string(&output[0]);
}

