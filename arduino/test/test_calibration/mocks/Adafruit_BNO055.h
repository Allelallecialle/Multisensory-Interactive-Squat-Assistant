#pragma once

#include "Adafruit_Sensor.h"
#include "Wire.h"
#include <cstdint>

#define BNO055_ADDRESS_A 0x28
#define BNO055_ADDRESS_B 0x29

typedef struct {
    uint16_t accel_offset_x;
    uint16_t accel_offset_y;
    uint16_t accel_offset_z;
    uint16_t mag_offset_x;
    uint16_t mag_offset_y;
    uint16_t mag_offset_z;
    uint16_t gyro_offset_x;
    uint16_t gyro_offset_y;
    uint16_t gyro_offset_z;
    uint16_t accel_radius;
    uint16_t mag_radius;
} adafruit_bno055_offsets_t;

class Adafruit_BNO055 {
public:
    enum adafruit_vector_type_t {
        VECTOR_ACCELEROMETER,
        VECTOR_MAGNETOMETER,
        VECTOR_GYROSCOPE,
        VECTOR_EULER,
        VECTOR_LINEARACCEL,
        VECTOR_GRAVITY
    };

    int32_t mock_sensor_id = 1;
    bool mock_fully_calibrated = true;
    uint8_t mock_mag_cal = 3;
    adafruit_bno055_offsets_t mock_returned_offsets = {};
    bool mock_begin_returns = true;

    bool begin_called = false;
    bool ext_crystal_set = false;
    bool set_offsets_called = false;
    int set_offsets_call_count = 0;
    int get_offsets_call_count = 0;
    int put_calls_observed = 0;
    adafruit_bno055_offsets_t last_offsets_set = {};

    Adafruit_BNO055(int32_t id = -1, uint8_t addr = BNO055_ADDRESS_A,
                    TwoWire* wire = nullptr)
        : _id(id), _addr(addr), _wire(wire) {}

    bool begin() {
        begin_called = true;
        return mock_begin_returns;
    }

    void getSensor(sensor_t* s) {
        s->version = 1;
        s->sensor_id = mock_sensor_id;
        s->type = 0;
        s->min_delay = 0;
        s->max_value = 0;
        s->min_value = 0;
        s->resolution = 0;
    }

    void getCalibration(uint8_t* sys, uint8_t* gyro, uint8_t* accel,
                        uint8_t* mag) {
        if (sys) *sys = 3;
        if (gyro) *gyro = 3;
        if (accel) *accel = 3;
        if (mag) *mag = mock_mag_cal;
    }

    void setSensorOffsets(const adafruit_bno055_offsets_t& d) {
        set_offsets_called = true;
        set_offsets_call_count++;
        last_offsets_set = d;
    }

    void setExtCrystalUse(bool v) { ext_crystal_set = v; }

    bool isFullyCalibrated() { return mock_fully_calibrated; }

    void getSensorOffsets(adafruit_bno055_offsets_t& d) {
        get_offsets_call_count++;
        d = mock_returned_offsets;
    }

    bool getEvent(sensors_event_t*) { return true; }
    bool getEvent(sensors_event_t*, adafruit_vector_type_t) { return true; }

    void getSystemStatus(uint8_t* sys, uint8_t* st, uint8_t* err) {
        if (sys) *sys = 0;
        if (st) *st = 0;
        if (err) *err = 0;
    }

private:
    int32_t _id;
    uint8_t _addr;
    TwoWire* _wire;
};
