#include "Arduino.h"
#include "EEPROM.h"
#include "Wire.h"

#include <chrono>

HardwareSerial Serial;
EEPROMClass EEPROM;
TwoWire Wire;
TwoWire Wire1;
TwoWire Wire2;
// TwoWire Wire3;

static auto g_start = std::chrono::steady_clock::now();

unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(
               now - g_start)
        .count();
}

unsigned long micros() {
    auto now = std::chrono::steady_clock::now();
    return (unsigned long)std::chrono::duration_cast<std::chrono::microseconds>(
               now - g_start)
        .count();
}

// No-op delays in tests so blocking loops resolve immediately when their
// exit condition is mocked true.
void delay(unsigned long) {}
void delayMicroseconds(unsigned long) {}

void pinMode(int, int) {}
void digitalWrite(int, int) {}
int digitalRead(int) { return 0; }
int analogRead(int) { return 0; }
