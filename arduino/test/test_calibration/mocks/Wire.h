#pragma once

#include "Arduino.h"

class TwoWire {
public:
    void begin() {}
    void begin(uint8_t) {}
    void setClock(uint32_t) {}
    void beginTransmission(uint8_t) {}
    uint8_t endTransmission() { return 0; }
    uint8_t endTransmission(bool) { return 0; }
    size_t write(uint8_t) { return 1; }
    size_t requestFrom(uint8_t, size_t) { return 0; }
    int available() { return 0; }
    int read() { return -1; }
};

extern TwoWire Wire;
extern TwoWire Wire1;
extern TwoWire Wire2;
extern TwoWire Wire3;
