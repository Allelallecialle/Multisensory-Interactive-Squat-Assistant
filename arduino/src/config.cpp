#include "config.h"

/*digital inputs*/
const uint16_t digital_input0_pin = 0;
const uint16_t digital_input1_pin = 1;
const uint16_t digital_input2_pin = 2;

uint16_t digital_input0 = 0;
uint16_t digital_input1 = 0;
uint16_t digital_input2 = 0;

int digital_input0_last_button_state = LOW;
int digital_input1_last_button_state = LOW;
int digital_input2_last_button_state = LOW;  

/* Digital outputs*/

const uint16_t digital_output0_pin = 3; 
const uint16_t digital_output1_pin = 4; 
const uint16_t digital_output2_pin = 5; 
const uint16_t digital_output3_pin = 9; 
const uint16_t digital_output4_pin = 10; 

int digital_output0_LED_state = HIGH;
int digital_output1_LED_state = HIGH;
int digital_output2_LED_state = HIGH;


unsigned long digital_input0_last_debounce_time = 0;  
unsigned long digital_input1_last_debounce_time = 0;  
unsigned long digital_input2_last_debounce_time = 0;  

unsigned long debounce_delay = 50;    // the debounce time; increase if the output flickers

//analog inputs
uint32_t analog_last_read = 0;

uint16_t analog_input0_pin = 0;
uint16_t analog_input1_pin = 1;
uint16_t analog_input2_pin = 2;
uint16_t analog_input3_pin = 3;

uint16_t analog_input0 = 0;
uint16_t analog_input1 = 0;
uint16_t analog_input2 = 0;
uint16_t analog_input3 = 0;

uint16_t analog_input0_lp_filtered = 0;
uint16_t analog_input1_lp_filtered = 0;
uint16_t analog_input2_lp_filtered = 0;
uint16_t analog_input3_lp_filtered = 0;

uint16_t previous_analog_input0_lp_filtered = 0;
uint16_t previous_analog_input1_lp_filtered = 0;
uint16_t previous_analog_input2_lp_filtered = 0;
uint16_t previous_analog_input3_lp_filtered = 0;

// 50 Hz Butterworth low-pass
double a_lp_50Hz[] = {1.000000000000, -3.180638548875, 3.861194348994, -2.112155355111, 0.438265142262};
double b_lp_50Hz[] = {0.000416599204407, 0.001666396817626, 0.002499595226440, 0.001666396817626, 0.000416599204407};

float refIMU1[3] = {0, 0, 0};
float refIMU2[3] = {0, 0, 0};

IIRFilter lp_analog_input0(b_lp_50Hz, a_lp_50Hz);
IIRFilter lp_analog_input1(b_lp_50Hz, a_lp_50Hz);
IIRFilter lp_analog_input2(b_lp_50Hz, a_lp_50Hz);
IIRFilter lp_analog_input3(b_lp_50Hz, a_lp_50Hz);


//Thresholds for each sensor
uint16_t analog_input0_threshold = 75;
uint16_t analog_input1_threshold = 10;
uint16_t analog_input2_threshold = 5;
uint16_t analog_input3_threshold = 10;

//user calibration
bool userPoseCalibrated = false;
bool setConfigured = false;
bool repLocked = false;   // to avoid double rep counting
bool isSquatting = false;
int repetitions_to_achieve = 0;
int squat_counter = 0;

unsigned long poseStableStart = 0;
const unsigned long POSE_STABLE_TIME = 2000; // ms required to maintain the concentric correct pose -> then trigger puredata reward sound
const float ANGLE_TOLERANCE = 6.0;           // degrees of tolerance of current squat pose from the calibrated one

boolean new_message_received = false;
char received_message[64];

// IMU
Adafruit_BNO055 bno_1(1, BNO055_ADDRESS_A); // 0x28 pin 18
Adafruit_BNO055 bno_2(2, BNO055_ADDRESS_B); // 0x29 pin 19

bool reset_calibration = false;  // set to true if you want to redo the calibration rather than using the values stored in the EEPROM
bool display_BNO055_info = true; // set to true if you want to print on the serial port the information about the status and calibration of the IMU

/* Set the correction factors for the three Euler angles according to the wanted orientation */
float correction_x = 0; // -177.19;
float correction_y = 0; // 0.5;
float correction_z = 0; // 1.25;

UserCommand lastCommand = CMD_NONE;
int lastNumber = -1;

// Pressure sensors

uint16_t pressure_sensor_threshold = 50;
uint32_t pressure_last_print = 0;

const float VCC = 3.3;      // Supply voltage for the FSR circuit
const float R_DIV = 10000;  // Resistance of the fixed resistor in the FSR voltage divider

float analog_FSR_H_filtered = 0;
float analog_FSR_FL_filtered = 0;
float analog_FSR_FR_filtered = 0;
float EMA_alpha = 0.1; // Lower = more smoothing. Adjust between 0.05 (heavy) and 0.3 (light)


