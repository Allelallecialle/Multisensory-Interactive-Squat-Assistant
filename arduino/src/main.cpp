#include <Wire.h>
#include <IIRFilter.h>
#include <string.h>
#include <Arduino.h>

#include "config.h"
#include "utils.h"



//incoming msg
//boolean new_message_received = false;



void setup(){
    Serial.begin(BAUD_RATE);
    while(!Serial);

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

    //setup pressure sensors


      /* Setup of the IMU BNO055 sensor ******************************************************************************/
  
  /* Initialise the IMU BNO055 sensor */
  delay(1000);
  if (!bno_1.begin()){
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 1 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }
  if (!bno_2.begin()){
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 2 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  int eeAddress_1 = 0;
  int eeAddress_2 = sizeof(long) + sizeof(adafruit_bno055_offsets_t); // beacause we have IMU1_id + IMU1 calibration data + IMU2 id + IMU2 calibration data
  long bnoID_1 = 1;
  long bnoID_2 = 2;


  calibrateIMU(bno_1, eeAddress_1, bnoID_1);
  calibrateIMU(bno_2, eeAddress_2, bnoID_2);

}

void loop(){

    receive_message();

    readPressureSensors(14, 15, 16, "RIGHT");
    readPressureSensors(23, 22, 21, "LEFT");

    analog_digital_loop();
}