#include <Wire.h>
#include <IIRFilter.h>
#include <string.h>
#include <Arduino.h>

#include "0_config.h"
#include "2_utils.h"



//incoming msg
char received_message[MAX_LENGTH_MESSAGE];
boolean new_message_received = false;



void setup(){
    Serial.begin(BAUD_RATE);

    //setup digital sensors  
    pinMode(digital_input0_pin, INPUT);
    pinMode(digital_input1_pin, INPUT);
    pinMode(digital_input2_pin, INPUT);
    pinMode(digital_output0_pin, OUTPUT);
    pinMode(digital_output1_pin, OUTPUT);
    pinMode(digital_output2_pin, OUTPUT);
    pinMode(digital_output3_pin, OUTPUT);
    pinMode(digital_output4_pin, OUTPUT);


    digitalWrite(digital_output0_pin, HIGH);
    digitalWrite(digital_output1_pin, HIGH);
    digitalWrite(digital_output2_pin, HIGH);
    digitalWrite(digital_output3_pin, LOW);
    digitalWrite(digital_output4_pin, LOW);

}

void loop(){

    receive_message(new_message_received, received_message);

    analog_digital_loop();
}