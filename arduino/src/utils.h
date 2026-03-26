#include "config.h"

#ifndef UTILS_H
#define UTILS_H

/**
 * The function `receive_message` reads characters from Serial and constructs a message until an end
 * marker is received, then calls `handle_received_message` with the complete message.
 * 
 * @param new_message_received The `new_message_received` parameter is a boolean variable that
 * indicates whether a new message has been received. It is used to control the flow of the message
 * reception process in the `receive_message` function.
 * @param received_message The `received_message` parameter is a pointer to a character array where the
 * received message characters are stored. The function `receive_message` reads characters from the
 * Serial input and populates this array until it encounters an `END_MARKER` character, at which point
 * it terminates the string and sets `new_message
 */
void handle_received_message(char *received_message);

/**
 * The function `handle_received_message` parses a received message to extract a command and value,
 * then performs an action based on the command.
 * 
 * @param received_message The `handle_received_message` function takes a `char* received_message` as a
 * parameter. This message is expected to contain a command and a value separated by specific
 * delimiters. The function then extracts the command and value from the message and performs a
 * specific action based on the command.
 */
void receive_message();

/**
 * The function `analog_digital_loop` reads analog sensor values, filters them, and sends them to the
 * serial port if they have changed.
 */
void analog_digital_loop();

/**
 * Displays some basic information on this sensor from the unified sensor API sensor_t type (see Adafruit_Sensor for more information)
 */
void displaySensorDetails(Adafruit_BNO055 &bno);

/** 
 * Display some basic info about the sensor status 
 */ 
void displaySensorStatus(Adafruit_BNO055 &bno);

/**
 *  Display sensor calibration status 
*/
void displayCalStatus(Adafruit_BNO055 &bno);

void checkCorrectPoseAndReward();

/* Magnetometer calibration */
void performMagCal(Adafruit_BNO055 &bno);

// function to calibrate one IMU. Pass the right params in setup() to calibrate the corresponding. Adapted calss code
void calibrateIMU(Adafruit_BNO055 &bno, int eeAddress, long &bnoID);

// to read the current IMUs angles
void readIMUAngles(float imu1[3], float imu2[3]);


// to check if the current pose is inside the tolerance angle set
bool checkPoseTolerance(float curr[3], float ref[3]);

void squatSeriesCounter();

void setUserPose();

// function to detect if the user is squatting 
bool detectSquatFromIMUs(float imu1[3], float imu2[3]);

void setSquatStateMediapipe();


#endif



