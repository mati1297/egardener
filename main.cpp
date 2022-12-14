/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "egardener.h"

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

int main() {
    eGardener eGardener;

    eGardener.execute();
}
