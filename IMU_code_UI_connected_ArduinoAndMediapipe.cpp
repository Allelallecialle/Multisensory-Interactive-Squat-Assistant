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
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <EEPROM.h>
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
#define ANALOG_PERIOD_MICROSECS 1000
static uint32_t analog_last_read = 0;


uint16_t analog_input0_pin = 0;
uint16_t analog_input1_pin = 1;
uint16_t analog_input2_pin = 2;
uint16_t analog_input3_pin = 3;
uint16_t analog_input4_pin = 4;
uint16_t analog_input5_pin = 5;




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
IIRFilter lp_analog_input0(b_lp_50Hz, a_lp_50Hz);
IIRFilter lp_analog_input1(b_lp_50Hz, a_lp_50Hz);
IIRFilter lp_analog_input2(b_lp_50Hz, a_lp_50Hz);
IIRFilter lp_analog_input3(b_lp_50Hz, a_lp_50Hz);




//Thresholds for each sensor
uint16_t analog_input0_threshold = 75;
uint16_t analog_input1_threshold = 10;
uint16_t analog_input2_threshold = 5;
uint16_t analog_input3_threshold = 10;



/* IMU ***************************************************************************************************/

/* Set the delay between fresh samples */
static const unsigned long BNO055_PERIOD_MILLISECS = 100; // E.g. 4 milliseconds per sample for 250 Hz
//static const float BNO055_SAMPLING_FREQUENCY = 1.0e3f / PERIOD_MILLISECS;
#define BNO055_PERIOD_MICROSECS 100.0e3f //= 1000 * PERIOD_MILLISECS;
static uint32_t BNO055_last_read = 0;

//Here set the ID. In this case it is 55. In this sketch the ID must be different from 0 as 0 is used to reset the EEPROM
Adafruit_BNO055 bno_1(1, BNO055_ADDRESS_A); // 0x28
Adafruit_BNO055 bno_2(2, BNO055_ADDRESS_B); // 0x29

bool reset_calibration = false;  // set to true if you want to redo the calibration rather than using the values stored in the EEPROM
bool display_BNO055_info = true; // set to true if you want to print on the serial port the infromation about the status and calibration of the IMU


/* Set the correction factors for the three Euler angles according to the wanted orientation */
float  correction_x = 0; // -177.19;
float  correction_y = 0; // 0.5;
float  correction_z = 0; // 1.25;


// USER POSE CALIBRATION ***************************************************************************************************/
bool userPoseCalibrated = false;

bool setConfigured = false;
int repetitions_to_achieve = 0;
int squat_counter = 0;
bool repLocked = false;   // to avoid double rep counting


float refIMU1[3];   // x y z reference
float refIMU2[3];

unsigned long poseStableStart = 0;
const unsigned long POSE_STABLE_TIME = 2000; // ms required to maintain the concentric correct pose -> then trigger puredata reward sound
const float ANGLE_TOLERANCE = 6.0;           // degrees of tolerance of current squat pose from the calibrated one

//vars defined to detect if the user is squatting or not
bool isSquatting = false;   // bool to send to mediapipe to check valgus knees
#define SQUAT_ANGLE_THRESHOLD 12.0   // degrees to enter and exit squat movement from literature (Camomilla et al., Escamilla)
#define SQUAT_EXIT_THRESHOLD 6.0
#define SQUAT_MIN_TIME 200    // ms waited to correctly detect squat movement


// to handle serial inputs
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

UserCommand lastCommand = CMD_NONE;
int lastNumber = -1;



/* Displays some basic information on this sensor from the unified sensor API sensor_t type (see Adafruit_Sensor for more information) */
void displaySensorDetails(Adafruit_BNO055 &bno)
{
    sensor_t sensor;
    bno.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print("Sensor:       "); Serial.println(sensor.name);
    Serial.print("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
    Serial.print("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
    Serial.print("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}

/* Display some basic info about the sensor status */
void displaySensorStatus(Adafruit_BNO055 &bno)
{
    /* Get the system status values (mostly for debugging purposes) */
    uint8_t system_status, self_test_results, system_error;
    system_status = self_test_results = system_error = 0;
    bno.getSystemStatus(&system_status, &self_test_results, &system_error);

    /* Display the results in the Serial Monitor */
    Serial.println("");
    Serial.print("System Status: 0x");
    Serial.println(system_status, HEX);
    Serial.print("Self Test:     0x");
    Serial.println(self_test_results, HEX);
    Serial.print("System Error:  0x");
    Serial.println(system_error, HEX);
    Serial.println("");
    delay(500);
}

/* Display sensor calibration status */
void displayCalStatus(Adafruit_BNO055 &bno)
{
    /* Get the four calibration values (0..3) */
    /* Any sensor data reporting 0 should be ignored, */
    /* 3 means 'fully calibrated" */
    uint8_t system, gyro, accel, mag;
    system = gyro = accel = mag = 0;
    bno.getCalibration(&system, &gyro, &accel, &mag);

    /* The data should be ignored until the system calibration is > 0 */
    Serial.print("\t");
    if (!system)
    {
        Serial.print("! ");
    }

    /* Display the individual values */
    Serial.print("Sys:");
    Serial.print(system, DEC);
    Serial.print(" G:");
    Serial.print(gyro, DEC);
    Serial.print(" A:");
    Serial.print(accel, DEC);
    Serial.print(" M:");
    Serial.print(mag, DEC);
}

/* Display the raw calibration offset and radius data */
void displaySensorOffsets(const adafruit_bno055_offsets_t &calibData)
{
    Serial.print("Accelerometer: ");
    Serial.print(calibData.accel_offset_x); Serial.print(" ");
    Serial.print(calibData.accel_offset_y); Serial.print(" ");
    Serial.print(calibData.accel_offset_z); Serial.print(" ");

    Serial.print("\nGyro: ");
    Serial.print(calibData.gyro_offset_x); Serial.print(" ");
    Serial.print(calibData.gyro_offset_y); Serial.print(" ");
    Serial.print(calibData.gyro_offset_z); Serial.print(" ");

    Serial.print("\nMag: ");
    Serial.print(calibData.mag_offset_x); Serial.print(" ");
    Serial.print(calibData.mag_offset_y); Serial.print(" ");
    Serial.print(calibData.mag_offset_z); Serial.print(" ");

    Serial.print("\nAccel Radius: ");
    Serial.print(calibData.accel_radius);

    Serial.print("\nMag Radius: ");
    Serial.print(calibData.mag_radius);
}


/* Magnetometer calibration */
void performMagCal(Adafruit_BNO055 &bno) {
  
  /* Get the four calibration values (0..3) */
  /* Any sensor data reporting 0 should be ignored, */
  /* 3 means 'fully calibrated" */
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;
 
  while (mag != 3) {
    
    bno.getCalibration(&system, &gyro, &accel, &mag);
    if(display_BNO055_info){
      
      displayCalStatus(bno);
      Serial.println("");
    }
  }
  
  if(display_BNO055_info){

    Serial.println("\nMagnetometer calibrated!");
  }
}  

// function to calibrate one IMU. Pass the right params in setup() to calibrate the corresponding. Adapted calss code
void calibrateIMU(Adafruit_BNO055 &bno, int eeAddress, long &bnoID){
  sensor_t sensor;
  long eeBnoID;
  adafruit_bno055_offsets_t calibrationData;
  bool foundCalib = false;

  
  if(reset_calibration){// Reset the EEPROM so a new calibration can be made
    eeAddress += sizeof(long);
    EEPROM.put(eeAddress, 0);
    if(display_BNO055_info){
      Serial.println("EEPROM reset.");
      delay(10000);
    }
  }

  bno.getSensor(&sensor);
  bnoID = sensor.sensor_id;
  EEPROM.get(eeAddress, eeBnoID);

  //restore calibration if we don't want to recalibrate
  if (eeBnoID != bnoID) {

    if (display_BNO055_info) {
      Serial.println("\nNo Calibration Data for this sensor exists in EEPROM");
      delay(2000);
    }

  } else {

    if (display_BNO055_info) {
      Serial.println("\nFound Calibration for this sensor in EEPROM.");
    }

    eeAddress += sizeof(long);
    EEPROM.get(eeAddress, calibrationData);

    if (display_BNO055_info) {
      displaySensorOffsets(calibrationData);
      Serial.println("\n\nRestoring Calibration data to the BNO055...");
    }

    bno.setSensorOffsets(calibrationData);

    if (display_BNO055_info) {
      Serial.println("\n\nCalibration data loaded into BNO055");
      delay(2000);
    }

    foundCalib = true;
  }

  if (display_BNO055_info) {
    displaySensorDetails(bno);
    displaySensorStatus(bno);
  }

  bno.setExtCrystalUse(true);

  if (foundCalib) {

    performMagCal(bno);   // magnetometer only

  } else {

    if (display_BNO055_info) {
      Serial.println("Please Calibrate Sensor: ");
      delay(2000);
    }

    while (!bno.isFullyCalibrated()) {

      if (display_BNO055_info) {
        displayCalStatus(bno);
        Serial.println("");
      }

      delay(BNO055_PERIOD_MILLISECS);
    }

    adafruit_bno055_offsets_t newCalib;
    bno.getSensorOffsets(newCalib);

    if (display_BNO055_info) {
      Serial.println("\nFully calibrated!");
      delay(3000);
      Serial.println("--------------------------------");
      Serial.println("Calibration Results: ");
      displaySensorOffsets(newCalib);
      Serial.println("\n\nStoring calibration data to EEPROM...");
    }

    EEPROM.put(eeAddress, bnoID);
    EEPROM.put(eeAddress + sizeof(long), newCalib);

    if (display_BNO055_info) {
      Serial.println("Data stored to EEPROM.");
      Serial.println("\n--------------------------------\n");
      delay(3000);
    }
  }
}

/*Functions for IMUs for squat repetitions *************************************/

// to read the current IMUs angles
void readIMUAngles(float imu1[3], float imu2[3]) {

  sensors_event_t e1, e2;
  bno_1.getEvent(&e1, Adafruit_BNO055::VECTOR_EULER);
  bno_2.getEvent(&e2, Adafruit_BNO055::VECTOR_EULER);

  imu1[0] = e1.orientation.x;
  imu1[1] = e1.orientation.y;
  imu1[2] = e1.orientation.z;

  imu2[0] = e2.orientation.x;
  imu2[1] = e2.orientation.y;
  imu2[2] = e2.orientation.z;
}

// to check if the current pose is inside the tolerance angle set
bool checkPoseTolerance(float curr[3], float ref[3]) {
  for (int i = 0; i < 3; i++) {
    if (abs(curr[i] - ref[i]) > ANGLE_TOLERANCE) {
      return false;
    }
  }
  return true;
}

// function to check squat repetitions. Use the above functions defined
void checkCorrectPoseAndReward() {
  if (lastCommand == CMD_QUIT) {
    lastCommand = CMD_NONE;

    setConfigured = false;
    squat_counter = 0;
    repLocked = false;
    poseStableStart = 0;

    return;
  }

  if (!setConfigured) {
    return;   //wait until the number of desired reps are set
  }

  float imu1[3], imu2[3];
  readIMUAngles(imu1, imu2);

  bool imu1_ok = checkPoseTolerance(imu1, refIMU1);
  bool imu2_ok = checkPoseTolerance(imu2, refIMU2);

  if ((imu1_ok && imu2_ok) && !repLocked) {

    if (poseStableStart == 0) {
      poseStableStart = millis();
    }

    if (millis() - poseStableStart >= POSE_STABLE_TIME) {
      squat_counter++;
      repLocked = true;   // lock user pose until exits it

      // trigger reward sound from puredata -> 1 squat repetition correctly done
      Serial.print("REP_OK,");    // Message to send to python
      Serial.print(squat_counter);
      Serial.print(",");
      Serial.println(repetitions_to_achieve);

      if (squat_counter >= repetitions_to_achieve) {

        // trigger another puredata reward sound for end of squat set
        Serial.println("SET_OK");   // Message to send to python

        setConfigured = false;   // allow new set
        squat_counter = 0;
        repLocked = false;
        poseStableStart = 0;
        return;
      }

      poseStableStart = 0;
    }

  } else {
    poseStableStart = 0;
    repLocked = false;
  }
}

void squatSeriesCounter() {
  //static bool prompted = false;
  
  if (setConfigured){
    Serial.println("setConfigured:");
    Serial.println(setConfigured);
    return;
  }

  if (lastCommand == CMD_NUMBER) {
    int n = lastNumber;
    lastCommand = CMD_NONE;
    lastNumber = -1;

    if (n >= 1 && n <= 15) {
      repetitions_to_achieve = n;
      squat_counter = 0;
      setConfigured = true;
      //prompted = false;
      Serial.println("setConfigured:");
      Serial.println(setConfigured);
      Serial.println("Repetitions desired saved on arduino");
    }
  }
}

void setUserPose() {

  if (lastCommand == CMD_CONFIRM_POSE) {
    lastCommand = CMD_NONE;

    float imu1[3], imu2[3];
    readIMUAngles(imu1, imu2);

    for (int i = 0; i < 3; i++) {
      refIMU1[i] = imu1[i];
      refIMU2[i] = imu2[i];
    }

    // Reset of set logic
    setConfigured = false;
    repetitions_to_achieve = 0;
    squat_counter = 0;
    repLocked = false;
    poseStableStart = 0;

    userPoseCalibrated = true;
    Serial.println("Pose saved on arduino");
  }
}

// function to detect if the user is squatting 
bool detectSquatFromIMUs(float imu1[3], float imu2[3]) {
  float delta1 = abs(imu1[1] - refIMU1[1]); // Y plane
  float delta2 = abs(imu2[1] - refIMU2[1]);

  return (delta1 > SQUAT_ANGLE_THRESHOLD ||
          delta2 > SQUAT_ANGLE_THRESHOLD);
}

// function to call in the loop() that sets the bool to send to mediapipe
void setSquatStateMediapipe(){
  static unsigned long squatStateStart = 0;
  float imu1[3], imu2[3];
  readIMUAngles(imu1, imu2);

  bool squatDetected = detectSquatFromIMUs(imu1, imu2);

  if (squatDetected && !isSquatting) {
    if (squatStateStart == 0){
      squatStateStart = millis();
    }

    if (millis() - squatStateStart > SQUAT_MIN_TIME) {
      isSquatting = true;
      Serial.println("SQUATSTATE,1");  //is squatting
    }
  } else if (!squatDetected && isSquatting) {
    float delta1 = abs(imu1[1] - refIMU1[1]);
    float delta2 = abs(imu2[1] - refIMU2[1]);

    if (delta1 < SQUAT_EXIT_THRESHOLD &&
        delta2 < SQUAT_EXIT_THRESHOLD) {

      isSquatting = false;
      squatStateStart = 0;
      Serial.println("SQUATSTATE,0");  //is not squatting
    }
  } else {
    squatStateStart = 0;
  }

}

/** Functions for handling received messages ***********************************************************************/

void receive_message() {
  
    static boolean reception_in_progress = false;
    static byte ndx = 0;
    char rcv_char;

    while (Serial.available() > 0 && new_message_received == false) {
        // Added to not lose typed user input from serial monitor
        if (!reception_in_progress && Serial.peek() != START_MARKER) {
          return;   // leave input for other parts (space, numbers, ...)
        }

        rcv_char = Serial.read();

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
  char *all_tokens[2] = {NULL, NULL}; //NOTE: the message is composed by 2 tokens: command and value
  const char delimiters[5] = {START_MARKER, ',', ' ', END_MARKER, '\0'};
  int i = 0;

  char *token = strtok(received_message, delimiters);
  while (token != NULL && i < 2) {
    all_tokens[i++] = token;
    token = strtok(NULL, delimiters);
  }

  if (all_tokens[0] == NULL) return;

  if (strcmp(all_tokens[0], "RESET") == 0) {
    lastCommand = CMD_RESET_POSE;
    userPoseCalibrated = false;
    setConfigured = false;
    repetitions_to_achieve = 0;
    squat_counter = 0;
    repLocked = false;
    poseStableStart = 0;
    return;
  }

  if (strcmp(all_tokens[0], "QUIT") == 0) {
    lastCommand = CMD_QUIT;
    return;
  }

  if (strcmp(all_tokens[0], "INITIALIZE") == 0) {
    lastCommand = CMD_INITIALIZE;
    return;
  }


  if (strcmp(all_tokens[0], "SAVE_POSE") == 0) {
    lastCommand = CMD_CONFIRM_POSE;
    return;
  }

  if (strcmp(all_tokens[0], "SET_N_REPS") == 0 && all_tokens[1] != NULL) {
    lastNumber = atoi(all_tokens[1]);
    lastCommand = CMD_NUMBER;
    Serial.println("CMD_NUMBER received");
    return;
  }

  if (strcmp(all_tokens[0], "WRIST_UNBALANCED") == 0 && all_tokens[1] != NULL) {
    lastNumber = atoi(all_tokens[1]);
    lastCommand = CMD_WRISTS;
    return;
  }

  if (strcmp(all_tokens[0], "KNEE_VALGUS") == 0 && all_tokens[1] != NULL) {
    lastNumber = atoi(all_tokens[1]);
    lastCommand = CMD_KNEES;
    return;
  }
  

/*
  char *command = all_tokens[0]; 
  char *value = all_tokens[1];

  if (strcmp(command,"motor1_pattern1") == 0 && strcmp(value,"1") == 0) {

    
    Serial.print("activating message 1: ");
    Serial.print(command);
    Serial.print(" ");
    Serial.print(value);
    Serial.println(" ");
    
    
    analogWrite(digital_output3_pin, 200);
    
  }*/
  
  //if (strcmp(command,"motor1_pattern1") == 0 && strcmp(value,"0") == 0) {

    /*
    Serial.print("activating message 2: ");
    Serial.print(command);
    Serial.print(" ");
    Serial.print(value);
    Serial.println(" ");
    */
   // analogWrite(digital_output3_pin, 0);
    
  //}
  


  /*
  if (strcmp(command,"LED1_pattern1") == 0) {

    
    Serial.print("activating message 1: ");
    Serial.print(command);
    Serial.print(" ");
    Serial.print(value);
    Serial.println(" ");
    
    
    analogWrite(digital_output4_pin, atoi(value));
    
  }  */
} 











/**************************************************************************************************************/

void setup() {
  Serial.begin(BAUD_RATE);
  while(!Serial);

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
    analog_input1_lp_filtered =  (uint16_t)lp_analog_input1.filter((double)analog_input1);
    analog_input2 = analogRead(analog_input2_pin);
    analog_input2_lp_filtered =  (uint16_t)lp_analog_input2.filter((double)analog_input2);                
    analog_input3 = analogRead(analog_input3_pin);
    analog_input3_lp_filtered =  (uint16_t)lp_analog_input3.filter((double)analog_input3);
    

    // Apply thresholds to the filtered signal
    analog_input0_lp_filtered = (analog_input0_lp_filtered < analog_input0_threshold) ? 0 : analog_input0_lp_filtered;
    analog_input1_lp_filtered = (analog_input1_lp_filtered < analog_input1_threshold) ? 0 : analog_input1_lp_filtered;
    analog_input2_lp_filtered = (analog_input2_lp_filtered < analog_input2_threshold) ? 0 : analog_input2_lp_filtered;
    analog_input3_lp_filtered = (analog_input3_lp_filtered < analog_input3_threshold) ? 0 : analog_input3_lp_filtered;



    //Plot on the Serial Plotter the unfiltered sensors values 
    /*
    Serial.print(analog_input0);
    Serial.print(" ");
    Serial.print(analog_input1);
    Serial.print(" ");
    Serial.print(analog_input2);
    Serial.print(" ");
    Serial.println(analog_input3);
    */


    //Plot on the Serial Plotter the filtered sensors values 
    /*
    Serial.print(analog_input0_lp_filtered);
    Serial.print(" ");
    Serial.print(analog_input1_lp_filtered);
    Serial.print(" ");
    Serial.print(analog_input2_lp_filtered);
    Serial.print(" ");
    Serial.println(analog_input3_lp_filtered);
    */



    // Send the sensor value to the serial port only if it has changed
    /*
    if(analog_input0_lp_filtered != previous_analog_input0_lp_filtered){
      Serial.print("a0, ");
      Serial.println(analog_input0_lp_filtered);
      //Serial.print(analog_input0);
      //Serial.print(" ");
      //Serial.println(analog_input0_lp_filtered);
      previous_analog_input0_lp_filtered = analog_input0_lp_filtered;
    }
    if(analog_input1_lp_filtered != previous_analog_input1_lp_filtered){
      Serial.print("a1, ");
      Serial.println(analog_input1_lp_filtered);
      //Serial.print(analog_input1);
      //Serial.print(" ");
      //Serial.println(analog_input1_lp_filtered);
      previous_analog_input1_lp_filtered = analog_input1_lp_filtered;
    }
    if(analog_input2_lp_filtered != previous_analog_input2_lp_filtered){
      Serial.print("a2, ");
      Serial.println(analog_input2_lp_filtered);
      previous_analog_input2_lp_filtered = analog_input2_lp_filtered;
    }
    if(analog_input3_lp_filtered != previous_analog_input3_lp_filtered){
      Serial.print("a3, ");
      Serial.println(analog_input3_lp_filtered);
      previous_analog_input3_lp_filtered = analog_input3_lp_filtered;
    }
    */

        
  } // End of the section processing the analog sensors with the set sample rate for them. Such sample rate is different from that of the IMU



  /* Loop for the IMU BNO055 sensor ******************************************************************************/
  
  if (micros() - BNO055_last_read >= BNO055_PERIOD_MICROSECS) {
    BNO055_last_read += BNO055_PERIOD_MICROSECS;
  
    sensors_event_t orientationData1, angVelData1, linearAccelData1;
    bno_1.getEvent(&orientationData1, Adafruit_BNO055::VECTOR_EULER);
    bno_1.getEvent(&angVelData1, Adafruit_BNO055::VECTOR_GYROSCOPE);
    bno_1.getEvent(&linearAccelData1, Adafruit_BNO055::VECTOR_LINEARACCEL);

    sensors_event_t orientationData2, angVelData2, linearAccelData2;
    bno_2.getEvent(&orientationData2, Adafruit_BNO055::VECTOR_EULER);
    bno_2.getEvent(&angVelData2, Adafruit_BNO055::VECTOR_GYROSCOPE);
    bno_2.getEvent(&linearAccelData2, Adafruit_BNO055::VECTOR_LINEARACCEL);
    
    
    /*
     Note:
     x = Yaw, y = Roll, z = pitch 
     
     The Yaw values are between 0° to +360°
     The Roll values are between -90° and +90°
     The Pitch values are between -180° and +180°
    */ 
    
    /*
    Serial.print(": Euler x= ");
    Serial.print(orientationData.orientation.x + correction_x); // I add a correction value to get the 0 for the orientation I need 
    Serial.print(" | Euler y= ");
    Serial.print(orientationData.orientation.y + correction_y);
    Serial.print(" | Euler z= ");
    Serial.print(orientationData.orientation.z + correction_z);
    */
    /* UNCOMMENT TO SEE IMU DATA
    Serial.print("--- IMU 1 ---");
    Serial.print("x, ");
    Serial.println(orientationData1.orientation.x + correction_x); // I add a correction value to get the 0 for the orientation I need 
    Serial.print("y, ");
    Serial.println(orientationData1.orientation.y + correction_y); // I add a correction value to get the 0 for the orientation I need 
    Serial.print("z, ");
    Serial.println(orientationData1.orientation.z + correction_z); // I add a correction value to get the 0 for the orientation I need 
    Serial.print("--------------------------------------------------");
    Serial.print("\n --- IMU 2 ---");
    Serial.print("\n x, ");
    Serial.println(orientationData2.orientation.x + correction_x); // I add a correction value to get the 0 for the orientation I need 
    Serial.print("y, ");
    Serial.println(orientationData2.orientation.y + correction_y); // I add a correction value to get the 0 for the orientation I need 
    Serial.print("z, ");
    Serial.println(orientationData2.orientation.z + correction_z); // I add a correction value to get the 0 for the orientation I need 
    */
    /*
    Serial.print(": angVel x= ");
    Serial.print(angVelData.gyro.x);
    Serial.print(" | angVel y= ");
    Serial.print(angVelData.gyro.y);
    Serial.print(" | angVel z= ");
    Serial.print(angVelData.gyro.z);

    Serial.print(": linearAccel x= ");
    Serial.print(linearAccelData.acceleration.x);
    Serial.print(" | linearAccel y= ");
    Serial.print(linearAccelData.acceleration.y);
    Serial.print(" | linearAccel z= ");
    Serial.print(linearAccelData.acceleration.z);
    */

    
    //Serial.println("");
  

    // Initialize everything restarting the arduino loop() when the Python UI is launched. This avoids restarting manually the arduino
    if (lastCommand == CMD_INITIALIZE) {
      Serial.println("INITIALIZATION received");
      lastCommand = CMD_NONE;
      userPoseCalibrated = false;
      setConfigured = false;
      repetitions_to_achieve = 0;
      squat_counter = 0;
      repLocked = false;
      poseStableStart = 0;
      return;
    }


    // Perform the user calibration pose. When the user is in the correct squat position, press the SPACE key to set the current pose angles
    if (!userPoseCalibrated) {
      setUserPose();
      return;
    }
    
    // Call the function for the squat repetions check once we have the user pose reference
    if (userPoseCalibrated) {
      if (!setConfigured) {
        squatSeriesCounter();
      } else {
        setSquatStateMediapipe();
        checkCorrectPoseAndReward();
      }
    }

  }//End loop IMU
}
