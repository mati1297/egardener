// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include "mbed.h"
#include "egardener.h"

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

int main() {
    eGardener eGardener;

    eGardener.execute();
}
