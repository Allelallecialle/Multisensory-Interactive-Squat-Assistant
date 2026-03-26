# Multisensory Interactive Squat Assistant
The system was developed for the final project of the **Multisensory and Interactive Systems** course of DISI Unitn.

A real-time squat analysis system that combines sensors, actuators, computer vision, and a responsive UI to monitor squat executions. 
It analyzes body posture adapting to the user, detects repetitions, and provides visual and haptic feedback to the user through a graphical interface and motors.
## Features
- Interactive UI
- Balance, pose and knees position monitoring
- Feet pressure monitoring
- Adaptive pose to reach during squat execution
- Personalized sensitivity thresholds
- Automatized repetitions count
- Python-Arduino communication with serial port

## Materials and Components
**Hardware**: Teensy or Arduino, webcam or external camera, 2 IMUs, 6 pressure sensors, 8 vibration motors. 

**Software**: Mediapipe, PyQt, PySerial, Puredata

## Project structure

## Running the code
- Load the arduino code to the microcontroller
- Connect the microcontroller to the chosen device to execute the python code
- Execute python code running `main.py`
- Interact with the UI to set the desired squat pose and the number of repetitions

### Setup python environment
#### With venv
- Create the virtual env `python3 -m venv .venv`
- Activate it `source .venv/bin/activate`
- Install the dependencies running `pip install requirements.txt`

#### With conda


#### To load the arduino code


## Authors
Alessandra Benassi, Stefan Razvan Manole

