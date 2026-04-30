#pragma once

#include "Arduino.h"
#include <cstdint>

#define SENSOR_NAME_LEN 12

typedef struct {
    int32_t version;
    int32_t sensor_id;
    int32_t type;
    int32_t reserved;
    char name[SENSOR_NAME_LEN];
    int32_t min_delay;
    float max_value;
    float min_value;
    float resolution;
} sensor_t;

typedef struct {
    int32_t version;
    int32_t sensor_id;
    int32_t type;
    int32_t reserved0;
    int32_t timestamp;
    union {
        float data[4];
        struct {
            float x;
            float y;
            float z;
        } orientation;
        struct {
            float x;
            float y;
            float z;
        } gyro;
        struct {
            float x;
            float y;
            float z;
        } acceleration;
    };
} sensors_event_t;

class Adafruit_Sensor {
public:
    virtual ~Adafruit_Sensor() {}
    virtual void getSensor(sensor_t*) = 0;
    virtual bool getEvent(sensors_event_t*) = 0;
};
