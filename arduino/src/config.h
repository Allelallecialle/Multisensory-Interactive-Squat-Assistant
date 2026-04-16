#include <Wire.h>
#include <IIRFilter.h>
#include <string.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>
#include <utility/imumaths.h>
#include <EEPROM.h>

#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#define BAUD_RATE 115200 //NOTE: on the Teensy this is meaningless as the Teensy always transmits data at the full USB speed
#define ANALOG_PERIOD_MICROSECS 5000
#define END_MARKER ']'
#define START_MARKER '['

#define SQUAT_ANGLE_THRESHOLD 12.0   // degrees to enter and exit squat movement from literature (Camomilla et al., Escamilla)
#define SQUAT_EXIT_THRESHOLD 6.0
#define SQUAT_MIN_TIME 200    // ms waited to correctly detect squat movement
#define BNO055_PERIOD_MICROSECS 100.0e3f //= 1000 * PERIOD_MILLISECS;
#define PERIOD_MICROSECS 1000 // 1 ms loop period for reading sensors and sending data to PureData

enum UserCommand {
  CMD_NONE,
  CMD_INITIALIZE,
  CMD_QUIT,
  CMD_RESET_POSE,
  CMD_CONFIRM_POSE,
  CMD_NUMBER,
  CMD_WRISTS,
  CMD_KNEES
};

//digital inputs
extern const uint16_t digital_input0_pin;
extern const uint16_t digital_input1_pin;
extern const uint16_t digital_input2_pin;

extern int digital_input0_button_state;
extern int digital_input1_button_state;
extern int digital_input2_button_state;

extern int digital_input0_last_button_state;
extern int digital_input1_last_button_state;
extern int digital_input2_last_button_state;  

/* Digital outputs*/

extern const uint16_t digital_output0_pin;
extern const uint16_t digital_output1_pin;
extern const uint16_t digital_output2_pin;
extern const uint16_t digital_output3_pin;
extern const uint16_t digital_output4_pin;
extern int digital_output0_LED_state;
extern int digital_output1_LED_state;
extern int digital_output2_LED_state;


extern unsigned long digital_input0_last_debounce_time;  
extern unsigned long digital_input1_last_debounce_time;  
extern unsigned long digital_input2_last_debounce_time;  

extern unsigned long debounce_delay;    // the debounce time; increase if the output flickers

//analog inputs
extern uint32_t analog_last_read;

extern uint16_t analog_input0_pin;
extern uint16_t analog_input1_pin;
extern uint16_t analog_input2_pin;
extern uint16_t analog_input3_pin;

extern uint16_t analog_input0;
extern uint16_t analog_input1;
extern uint16_t analog_input2;
extern uint16_t analog_input3;

extern uint16_t analog_input0_lp_filtered;
extern uint16_t analog_input1_lp_filtered;
extern uint16_t analog_input2_lp_filtered;
extern uint16_t analog_input3_lp_filtered;

extern uint16_t previous_analog_input0_lp_filtered;
extern uint16_t previous_analog_input1_lp_filtered;
extern uint16_t previous_analog_input2_lp_filtered;
extern uint16_t previous_analog_input3_lp_filtered;

// 50 Hz Butterworth low-pass
extern double a_lp_50Hz[];
extern double b_lp_50Hz[];

// IIR filters for each analog input
extern IIRFilter lp_analog_input0;
extern IIRFilter lp_analog_input1;
extern IIRFilter lp_analog_input2;
extern IIRFilter lp_analog_input3;

//Thresholds for each sensor
extern uint16_t analog_input0_threshold;
extern uint16_t analog_input1_threshold;
extern uint16_t analog_input2_threshold;
extern uint16_t analog_input3_threshold;

//user pose calibration
extern bool userPoseCalibrated;
extern bool setConfigured;
extern bool repLocked;   // to avoid double rep counting
extern bool isSquatting; 

extern int repetitions_to_achieve;
extern int squat_counter;


extern float refIMU1[3];   // x y z reference
extern float refIMU2[3];

extern unsigned long poseStableStart;
extern const unsigned long POSE_STABLE_TIME; // ms required to maintain the concentric correct pose -> then trigger puredata reward sound
extern const float ANGLE_TOLERANCE;           // degrees of tolerance of current squat pose from the calibrated one
extern UserCommand lastCommand;
extern int lastNumber;
extern boolean new_message_received;
constexpr byte MAX_LENGTH_MESSAGE = 64;
extern char received_message[MAX_LENGTH_MESSAGE];

//IMU
/* Set the delay between fresh samples */
static const unsigned long BNO055_PERIOD_MILLISECS = 100; // E.g. 4 milliseconds per sample for 250 Hz
static uint32_t BNO055_last_read = 0;


extern Adafruit_BNO055 bno_1;
extern Adafruit_BNO055 bno_2;

extern bool reset_calibration;
extern bool display_BNO055_info;


/* Set the correction factors for the three Euler angles according to the wanted orientation */
extern float  correction_x;
extern float  correction_y;
extern float  correction_z;
// Pressure sensors (Teensy 3.6 analog pins)
extern const uint16_t pressure_sensor0_pin;
extern const uint16_t pressure_sensor1_pin;
extern const uint16_t pressure_sensor2_pin;

extern uint16_t pressure_sensor0_value;
extern uint16_t pressure_sensor1_value;
extern uint16_t pressure_sensor2_value;

extern uint16_t pressure_sensor_threshold;
extern uint32_t pressure_last_print;


// FSR values
extern const uint16_t FSR_FL; // Front Left
extern const uint16_t FSR_FR; // Front Right
extern const uint16_t FSR_H;  // Heel

extern const float VCC;      // Supply voltage for the FSR circuit
extern const float R_DIV;    // Resistance of the fixed resistor in the FSR voltage divider

extern float analog_FSR_H_filtered;
extern float analog_FSR_FL_filtered;
extern float analog_FSR_FR_filtered;
extern float EMA_alpha; // Lower = more smoothing. Adjust between 0.05 (heavy) and 0.3 (light)


#define PRESSURE_PRINT_PERIOD_MS 200

#endif


