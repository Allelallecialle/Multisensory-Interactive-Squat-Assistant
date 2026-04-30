#pragma once

#include <cstdint>
#include <cstring>

class EEPROMClass {
public:
    static constexpr int SIZE = 4096;
    uint8_t storage[SIZE];

    EEPROMClass() { std::memset(storage, 0, SIZE); }

    void clear() { std::memset(storage, 0, SIZE); }

    template <typename T>
    T& get(int idx, T& t) {
        std::memcpy(&t, storage + idx, sizeof(T));
        return t;
    }

    template <typename T>
    const T& put(int idx, const T& t) {
        std::memcpy(storage + idx, &t, sizeof(T));
        return t;
    }

    uint8_t read(int idx) const { return storage[idx]; }
    void write(int idx, uint8_t v) { storage[idx] = v; }
    void update(int idx, uint8_t v) { storage[idx] = v; }
    int length() const { return SIZE; }
};

extern EEPROMClass EEPROM;
