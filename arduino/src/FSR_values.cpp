/******************************************************************************

Author: Luca Turchet 17/08/2019
Modified version from 
https://learn.sparkfun.com/tutorials/force-sensitive-resistor-hookup-guide/all


Example sketch for SparkFun's force sensitive resistors
  (https://www.sparkfun.com/products/9375)
Jim Lindblom @ SparkFun Electronics
April 28, 2016

Create a voltage divider circuit combining an FSR with a 10k resistor.
- The resistor should connect from A0 to GND.
- The FSR should connect from A0 to 3.3V
As the resistance of the FSR decreases (meaning an increase in pressure), the
voltage at A0 should increase.

//calcu


******************************************************************************/
#include <Arduino.h>
#include <IIRFilter.h>

#define BAUD_RATE 9600
#define PERIOD_MICROSECS 1000

static uint32_t last_read = 0;

const int FSR_FL = 14; // Pin connected to FSR/resistor divider
const int FSR_FR = 15; // Pin connected to FSR/resistor divider
const int FSR_H = 16; // Pin connected to FSR/resistor divider

// Measure the voltage at 5V and resistance of your 3.3k resistor, and enter
// their value's below:
const float VCC = 4.99; // Measured voltage of Arduno 5V line
const float R_DIV = 10000.0; // Measured resistance of 10k resistor

double a_lp_50Hz[] = {1.000000000000, -3.180638548875, 3.861194348994, -2.112155355111, 0.438265142262};
double b_lp_50Hz[] = {0.000416599204407, 0.001666396817626, 0.002499595226440, 0.001666396817626, 0.000416599204407};
IIRFilter lp_analog_input(b_lp_50Hz, a_lp_50Hz);

double a_lp_100Hz[] = {1.000000000000, -1.561018075800, 0.641351538057};
double b_lp_100Hz[] = {0.020083365564, 0.040166731128, 0.020083365564};

float analog_FSR_H_filtered = 0;
float analog_FSR_FL_filtered = 0;
float analog_FSR_FR_filtered = 0;
float EMA_alpha = 0.1; // Lower = more smoothing. Adjust between 0.05 (heavy) and 0.3 (light)

// Exponential Moving Average low-pass filter for FSR readings
// y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
float low_pass_EMA(float x, float ym1, float alpha){
  return alpha * x + (1.0f - alpha) * ym1;
}


void setup() 
{
  Serial.begin(9600);
  pinMode(FSR_FL, INPUT);
  pinMode(FSR_FR, INPUT);
  pinMode(FSR_H, INPUT);
}

void loop()
{
  int fsrADC = analogRead(FSR_FL);
  int fsrADC2 = analogRead(FSR_FR);
  int fsrADC3 = analogRead(FSR_H);

  // Always apply the low-pass filter (including zero readings for smooth decay)
  analog_FSR_FL_filtered = low_pass_EMA(fsrADC, analog_FSR_FL_filtered, EMA_alpha);
  analog_FSR_FR_filtered = low_pass_EMA(fsrADC2, analog_FSR_FR_filtered, EMA_alpha);
  analog_FSR_H_filtered = low_pass_EMA(fsrADC3, analog_FSR_H_filtered, EMA_alpha);

  if (analog_FSR_FL_filtered > 1.0 || analog_FSR_FR_filtered > 1.0 || analog_FSR_H_filtered > 1.0)
  {
    // Use filtered ADC reading to calculate voltage:
    float fsrV = analog_FSR_FL_filtered * VCC / 1023.0;
    float fsrV2 = analog_FSR_FR_filtered * VCC / 1023.0;
    float fsrV3 = analog_FSR_H_filtered * VCC / 1023.0;
    //Serial.println("Voltage: " + String(fsrV) + " V, Voltage2: " + String(fsrV2) + " V, Voltage3: " + String(fsrV3) + " V\n");
    // Use voltage and static resistor value to 
    // calculate FSR resistance:
    float fsrR = R_DIV * (VCC / fsrV - 1.0);
    //Serial.println("Resistance: " + String(fsrR) + " ohms\n");

    float fsrR2 = R_DIV * (VCC / fsrV2 - 1.0);
    //Serial.println("Resistance2: " + String(fsrR2) + " ohms\n");

    float fsrR3 = R_DIV * (VCC / fsrV3 - 1.0);


    //Serial.println("Resistance3: " + String(fsrR3) + " ohms");
    // Guesstimate force based on slopes in figure 3 of
    // FSR datasheet:
    float forceFL;
    float fsrG = 1.0 / fsrR; // Calculate conductance
    // Break parabolic curve down into two linear slopes:
    if (fsrR <= 400) 
      forceFL = (fsrG - 0.00075) / 0.00000032639;
    else
      forceFL =  fsrG / 0.000000642857;
    Serial.println("Force FL: " + String(forceFL) + " g\n");


    float forceFR;
    float fsrG2 = 1.0 / fsrR2; // Calculate conductance
    // Break parabolic curve down into two linear slopes:
    if (fsrR2 <= 400)      forceFR = (fsrG2 - 0.00075) / 0.00000032639;
    else                    forceFR =  fsrG2 / 0.000000642857;
    Serial.println("Force FR: " + String(forceFR) + " g\n");


    float forceH;
    float fsrG3 = 1.0 / fsrR3; // Calculate conductance
    // Break parabolic curve down into two linear slopes:
    if (fsrR3 <= 400)      forceH = (fsrG3 - 0.00075) / 0.00000032639;
    else                    forceH =  fsrG3 / 0.000000642857;
    Serial.println("Force H: " + String(forceH) + " g\n");  
    Serial.println();


    //showing the percentage on each fsr based on the total amount of force detected on the three fsrs
    float totalForce = forceFL + forceFR + forceH;
    float percentageFL = (forceFL / totalForce) * 100.0;
    float percentageFR = (forceFR / totalForce) * 100.0;
    float percentageH = (forceH / totalForce) * 100.0;
    Serial.println("Percentage FL: " + String(percentageFL) + " %");
    Serial.println("Percentage FR: " + String(percentageFR) + " %");
    Serial.println("Percentage H: " + String(percentageH) + " %");

    //calculate the total weight on the three fsrs based on the total amount of force detected and the percentage on each fsr
    float totalWeight = (forceFL + forceFR + forceH) / 1000.0; // Convert from grams to kilograms
    Serial.println("Total weight: " + String(totalWeight) + " kg\n");

    delay(500);
  }
  else
  {
    Serial.println("No pressure detected");
    delay(500);
  }
}
