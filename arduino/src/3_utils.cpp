#include "2_utils.h"
#include <IIRFilter.h>

// 10 Hz Butterworth low-pass at 200Hz sampling rate (Period = 5000microsec)
double a_lp_10Hz[] = {1.000000000000, -3.180638548875, 3.861194348994, -2.112155355111, 0.438265142262};
double b_lp_10Hz[] = {0.000416599204407, 0.001666396817626, 0.002499595226440, 0.001666396817626, 0.000416599204407};

IIRFilter lp_analog_input0(b_lp_10Hz, a_lp_10Hz);


void receive_message(boolean new_message_received, char* received_message) {
  
    static boolean reception_in_progress = false;
    static byte ndx = 0;
    char rcv_char;

    while (Serial.available() > 0 && new_message_received == false) {
        rcv_char = Serial.read();
        Serial.println(rcv_char);

        if (reception_in_progress == true) {
            if (rcv_char!= END_MARKER) {
                received_message[ndx] = rcv_char;
                ndx++;
                if (ndx >= MAX_LENGTH_MESSAGE) {
                    ndx = MAX_LENGTH_MESSAGE - 1;
                }
            }
            else {
                received_message[ndx] = '\0'; // terminate the string
                reception_in_progress = false;
                ndx = 0;
                new_message_received = true;
            }
        }
        else if (rcv_char == START_MARKER) {
            reception_in_progress = true;
        }
    }

    if (new_message_received) {
      handle_received_message(received_message);
      new_message_received = false;
    }
}


void handle_received_message(char *received_message) {
    
    char *all_tokens[2] = {NULL, NULL};
    const char delimiters[5] = {START_MARKER, ',', ' ' , END_MARKER, '\0'};
    int i = 0;

    all_tokens[i] = strtok(received_message, delimiters);

    while (i<1 && all_tokens[i] != NULL){
        all_tokens[++i] = strtok(NULL, delimiters);
    }

    char *command = all_tokens[0];
    char *value = all_tokens[1];

    if (strcmp(command, "motor1_pattern1")==0){
        analogWrite(digital_output3_pin, atoi(value));
    }
}



void analog_digital_loop() {
    //Loop for the analog and digital sensors
    if(micros() - analog_last_read >= ANALOG_PERIOD_MICROSECS){
        analog_last_read += ANALOG_PERIOD_MICROSECS;

        //Loop for the analog sensors
        analog_input0 = analogRead(analog_input0_pin);
        analog_input0_lp_filtered =  (uint16_t)lp_analog_input0.filter((double)analog_input0);
        analog_input1 = analogRead(analog_input1_pin);
        analog_input0_lp_filtered = (analog_input0_lp_filtered < analog_input0_threshold) ? 0 : analog_input0_lp_filtered;

        //Send the sensor value to the serial port only if it has changed
        if(analog_input0_lp_filtered != previous_analog_input0_lp_filtered){
          Serial.print("a0, ");
          Serial.println(analog_input0_lp_filtered);
          previous_analog_input0_lp_filtered = analog_input0_lp_filtered;
          previous_analog_input0_lp_filtered = analog_input0_lp_filtered;
        }
    
    }
}