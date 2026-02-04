#include "0_config.h"


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

unsigned long debounce_delay;    // the debounce time; increase if the output flickers

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

//Thresholds for each sensor
uint16_t analog_input0_threshold = 0;
uint16_t analog_input1_threshold = 0;
uint16_t analog_input2_threshold = 0;
uint16_t analog_input3_threshold = 0;

// 10 Hz Butterworth low-pass at 200Hz sampling rate (Period = 5000microsec)


