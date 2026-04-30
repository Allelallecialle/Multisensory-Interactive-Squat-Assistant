#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

typedef bool boolean;
typedef uint8_t byte;

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#ifndef abs
#define abs(x) ((x) > 0 ? (x) : -(x))
#endif

unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
void delayMicroseconds(unsigned long us);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int v);
int digitalRead(int pin);
int analogRead(int pin);

class String {
public:
    std::string s;
    String() {}
    String(const char* c) : s(c ? c : "") {}
    String(const std::string& v) : s(v) {}
    String(int i) : s(std::to_string(i)) {}
    String(long i) : s(std::to_string(i)) {}
    String(unsigned int i) : s(std::to_string(i)) {}
    String(float f) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%f", f);
        s = buf;
    }
    String(double d) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%f", d);
        s = buf;
    }
    String operator+(const String& o) const { String r; r.s = s + o.s; return r; }
    String operator+(const char* c) const { String r; r.s = s + (c ? c : ""); return r; }
    const char* c_str() const { return s.c_str(); }
};
inline String operator+(const char* c, const String& o) {
    return String(c) + o;
}

class HardwareSerial {
public:
    void begin(unsigned long) {}
    void end() {}
    int available() { return 0; }
    int peek() { return -1; }
    int read() { return -1; }
    operator bool() const { return true; }

    template <typename T> void print(const T&) {}
    template <typename T> void print(const T&, int) {}
    template <typename T> void println(const T&) {}
    template <typename T> void println(const T&, int) {}
    void println() {}
    void print(const String& s) { (void)s; }
    void println(const String& s) { (void)s; }
};

extern HardwareSerial Serial;
