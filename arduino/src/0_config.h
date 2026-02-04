#include <Wire.h>
#include <IIRFilter.h>
#include <string.h>

#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#define BAUD_RATE 115200 //NOTE: on the Teensy this is meaningless as the Teensy always transmits data at the full USB speed
#define ANALOG_PERIOD_MICROSECS 5000
#define END_MARKER ']'
#define START_MARKER '['
#define MAX_LENGTH_MESSAGE 64
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

//Thresholds for each sensor
extern uint16_t analog_input0_threshold;
extern uint16_t analog_input1_threshold;
extern uint16_t analog_input2_threshold;
extern uint16_t analog_input3_threshold;
#endif


