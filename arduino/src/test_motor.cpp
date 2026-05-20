/**
* 
* This sketch reads multiple analog and digital sensors as well as the IMU, and sends the values on the serial port.
* This sketch also receives meessages from the serial port in the format [string,number], and then
* associates to this messages a certain behavior (e.g, triggering a vibration motor pattern)
* 
* The analog sensors values are filtered with a butterworth lowpass filter.
* The filtering is achieved by means of the library https://github.com/tttapa/Filters
* The coefficients for the filter are calculated using the tools: http://www.exstrom.com/journal/sigproc/
* 
* 
* Author: Luca Turchet
* Date: 30/05/2019
* 
* 
* 
* 
**/

#include <Wire.h>
#include <IIRFilter.h>
#include <string.h>

#define BAUD_RATE 115200 //NOTE: on the Teensy this is meaningless as the Teensy always transmits data at the full USB speed


/* Variables for incoming messages *************************************************************/

const byte MAX_LENGTH_MESSAGE = 64;
char received_message[MAX_LENGTH_MESSAGE];

char START_MARKER = '[';
char END_MARKER = ']';

    
boolean new_message_received = false;




/* Digital inputs *************************************************************/

const uint16_t digital_input0_pin = 0;
const uint16_t digital_input1_pin = 1;
const uint16_t digital_input2_pin = 2;

uint16_t digital_input0 = 0;
uint16_t digital_input1 = 0;
uint16_t digital_input2 = 0;

int digital_input0_button_state;
int digital_input1_button_state;
int digital_input2_button_state;

int digital_input0_last_button_state = LOW;
int digital_input1_last_button_state = LOW;
int digital_input2_last_button_state = LOW;   



/* Digital outputs *************************************************************/

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







/* Analog inputs ******************************************************************************************/

//#define ANALOG_BIT_RESOLUTION 12 // Only for Teensy

//static const unsigned long ANALOG_PERIOD_MILLISECS = 1; // E.g. 4 milliseconds per sample for 250 Hz
//static const unsigned long ANALOG_ANALOG_PERIOD_MICROSECS = 1000 * PERIOD_MILLISECS;
//static const float ANALOG_SAMPLING_FREQUENCY = 1.0e3f / PERIOD_MILLISECS;
#define ANALOG_PERIOD_MICROSECS 5000
static uint32_t analog_last_read = 0;


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

// 10 Hz Butterworth low-pass at 200Hz sampling rate (Period = 5000microsec)
double a_lp_10Hz[] = {1.000000000000, -3.180638548875, 3.861194348994, -2.112155355111, 0.438265142262};
double b_lp_10Hz[] = {0.000416599204407, 0.001666396817626, 0.002499595226440, 0.001666396817626, 0.000416599204407};

IIRFilter lp_analog_input0(b_lp_10Hz, a_lp_10Hz);
// IIRFilter lp_analog_input1(b_lp_50Hz, a_lp_50Hz);
// IIRFilter lp_analog_input2(b_lp_50Hz, a_lp_50Hz);
// IIRFilter lp_analog_input3(b_lp_50Hz, a_lp_50Hz);




//Thresholds for each sensor
uint16_t analog_input0_threshold = 0;
uint16_t analog_input1_threshold = 0;
uint16_t analog_input2_threshold = 0;
uint16_t analog_input3_threshold = 0;





/** Functions for handling received messages ***********************************************************************/

void handle_received_message(char *received_message);

void receive_message() {
  
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

  //Serial.print("received_message: ");
  //Serial.println(received_message);

  
  char *all_tokens[2] = {NULL,NULL}; //NOTE: the message is composed by 2 tokens: command and value
  const char delimiters[5] = {START_MARKER, ',', ' ', END_MARKER,'\0'};
  int i = 0;

  all_tokens[i] = strtok(received_message, delimiters);
  
  while (i < 1 && all_tokens[i] != NULL) {
    all_tokens[++i] = strtok(NULL, delimiters);
  }

  char *command = all_tokens[0]; 
  char *value = all_tokens[1];


  if (strcmp(command,"motor1_pattern1") == 0) {

    /*
    Serial.print("activating message 1: ");
    Serial.print(command);
    Serial.print(" ");
    Serial.print(value);
    Serial.println(" ");
    */
    
    analogWrite(digital_output3_pin, atoi(value));
    
  }
  
  
  if (strcmp(command,"LED1_pattern1") == 0) {
    
    analogWrite(digital_output4_pin, atoi(value));
    
  }  
} 

/**************************************************************************************************************/

void setup() {
  Serial.begin(BAUD_RATE);
  // while(!Serial);

  /* Setup of the digital sensors ******************************************************************************/
  
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

  /* Setup of the analog sensors ******************************************************************************/
 
  // analogReadResolution(ANALOG_BIT_RESOLUTION); // Only for Teensy
}
/****************************************************************************************************/

void loop() {


  receive_message();

  
  /* Loop for the analog and digital sensors ******************************************************************************/
  
  if (micros() - analog_last_read >= ANALOG_PERIOD_MICROSECS) {
    analog_last_read += ANALOG_PERIOD_MICROSECS;

    
    /* Loop for the analog sensors ******************************************************************************/

    analog_input0 = analogRead(analog_input0_pin);
    analog_input0_lp_filtered =  (uint16_t)lp_analog_input0.filter((double)analog_input0);
    analog_input1 = analogRead(analog_input1_pin);
    // analog_input1_lp_filtered =  (uint16_t)lp_analog_input1.filter((double)analog_input1);
    // analog_input2 = analogRead(analog_input2_pin);
    // analog_input2_lp_filtered =  (uint16_t)lp_analog_input2.filter((double)analog_input2);                
    // analog_input3 = analogRead(analog_input3_pin);
    // analog_input3_lp_filtered =  (uint16_t)lp_analog_input3.filter((double)analog_input3);
    

    // Apply thresholds to the filtered signal
    analog_input0_lp_filtered = (analog_input0_lp_filtered < analog_input0_threshold) ? 0 : analog_input0_lp_filtered;


    // Send the sensor value to the serial port only if it has changed
    if(analog_input0_lp_filtered != previous_analog_input0_lp_filtered){
      Serial.print("a0, ");
      Serial.println(analog_input0_lp_filtered);
      //Serial.print(analog_input0);
      //Serial.print(" ");
      //Serial.println(analog_input0_lp_filtered);
      previous_analog_input0_lp_filtered = analog_input0_lp_filtered;
    }
        
  } // End of the section processing the analog sensors with the set sample rate for them. Such sample rate is different from that of the IMU

}
