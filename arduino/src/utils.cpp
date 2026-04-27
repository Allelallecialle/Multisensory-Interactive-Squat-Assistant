#include "utils.h"
#include <IIRFilter.h>


void analog_digital_loop() {
    //Loop for the analog and digital sensors
  if(micros() - analog_last_read >= ANALOG_PERIOD_MICROSECS){
      analog_last_read += ANALOG_PERIOD_MICROSECS;

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
  
    }

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

      }
}

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

/**** IMU */
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

/** Pressure sensors ************************************************************************************************/

void readPressureSensors(uint16_t H, uint16_t FL, uint16_t FR, String name) {

  pinMode(FL, INPUT);
  pinMode(FR, INPUT);
  pinMode(H, INPUT);

  int fsrFL = analogRead(FL);
  int fsrFR = analogRead(FR);
  int fsrH = analogRead(H);

  if(fsrFL > 1.0 || fsrFR > 1.0 || fsrH > 1.0){

    // Convert the raw readings to voltages
    float fsrFL_V = fsrFL * VCC / 1023.0;
    float fsrH_V = fsrH * VCC / 1023.0;
    float fsrFR_V = fsrFR * VCC / 1023.0;

    //calculate resistance fsr
    float fsrFL_R = R_DIV * (VCC / fsrFL_V - 1.0);
    float fsrH_R = R_DIV * (VCC / fsrH_V - 1.0);
    float fsrFR_R = R_DIV * (VCC / fsrFR_V - 1.0);

    //guess force based on slops
    float forceFL;
    float fsrG_FL = 1.0 / fsrFL_R;
    //break parabolic curve down into two linear slopes
    if(fsrFL_R <= 400)
      forceFL = (fsrG_FL - 0.00075) / 0.00000032639;
    else
      forceFL = fsrG_FL / 0.000000642857;

    // Serial.println("Force FL: " + String(forceFL) + " g\r");

    float forceH;
    float fsrG_H = 1.0 / fsrH_R;
    if(fsrH_R <= 400)
      forceH = (fsrG_H - 0.00075) / 0.00000032639;
    else
      forceH = fsrG_H / 0.000000642857;

    // Serial.println("Force H: " + String(forceH) + " g\r");

    float forceFR;
    float fsrG_FR = 1.0 / fsrFR_R;
    if(fsrFR_R <= 400)
      forceFR = (fsrG_FR - 0.00075) / 0.00000032639;
    else
      forceFR = fsrG_FR / 0.000000642857;

    // Serial.println("Force FR: " + String(forceFR) + " g\r");


    //showing percentage on each fsr based on the total amount of force detected on the three fsr
    float totalForce = forceFL + forceFR + forceH;
    float percentageFL = (forceFL / totalForce) * 100.0;
    float percentageH = (forceH / totalForce) * 100.0;
    float percentageFR = (forceFR / totalForce) * 100.0;
    
    Serial.print(name + " - ");
    Serial.println("Percentage FL: " + String(percentageFL) + " %\r");
    Serial.println("Percentage H: " + String(percentageH) + " %\r");
    Serial.println("Percentage FR: " + String(percentageFR) + " %\r");

    delay(500); // Delay to avoid flooding the serial output

  }

  else 
  {
    Serial.println("No significant force detected on the sensors.\n");
    delay(500); // Delay to avoid flooding the serial output
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

} 
