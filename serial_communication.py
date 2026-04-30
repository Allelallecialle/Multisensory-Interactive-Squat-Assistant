import queue

import serial
import threading

import puredata_communication


#Python controller that sends to arduino: [RESET], [SET_N_REPS], [SAVE_POSE], [QUIT], [WRIST_UNBALANCED], [KNEE_VALGUS]
#Viceversa python receives: SQUATSTATE (i.e. is_squatting bool to know when to check valgus knees), REP_OK,
# pressure values to draw on UI, current number of repetitions SET_OK
class SerialController:
    # "/dev/ttyACM0" o USB0
    def __init__(self, port="/dev/ttyACM0", baud=115200):
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.running = True

        self.is_squatting = False
        self.set_completed = False
        self.rep_completed = False
        self.current_reps = 0
        self.total_reps = 0
        self.left_pressure = [0, 0, 0]
        self.right_pressure = [0, 0, 0]
        self.write_queue = queue.Queue()

        self.read_thread = threading.Thread(target=self.read_loop, daemon=True)
        self.write_thread = threading.Thread(target=self.write_loop, daemon=True)

        self.read_thread.start()
        self.write_thread.start()

    # ----------------- Write to arduino -----------------
    def write_loop(self):
        while self.running:
            try:
                msg = self.write_queue.get(timeout=0.1)
                self.ser.write((msg).encode())
            except queue.Empty:
                continue
            except Exception as e:
                print("Serial write error:", e)

    # ----------------- Read from arduino -----------------
    def read_loop(self):
        while self.running:
            try:
                line = self.ser.readline().decode(errors="ignore").strip()
                if line:
                    print("FROM ARDUINO:", line)    #For debug
                    self.parse_message(line)
            except Exception as e:
                print("Serial error:", e)

    def parse_message(self, msg):
        print("PARSING:", msg)
        if msg.startswith("SQUATSTATE"):
            self.is_squatting = bool(int(msg.split(",")[1]))
            print(f"SQUATSTATE: {self.is_squatting}")
            send_to_puredata("SQUATSTATE", self.is_squatting)

        #TODO implement pressure data handling for UI
        elif msg.startswith("PRESSURE:"):
            vals = list(map(int, msg.split(":")[1].split(",")))
            # left foot 3 sensors, right foot 3 sensors
            self.left_pressure = vals[:3]
            self.right_pressure = vals[3:]

        elif msg.startswith("REP_OK"):
            parts = msg.split(",")
            if len(parts) == 3:
                current = int(parts[1])
                total = int(parts[2])
                print(f"Rep {current}/{total}")
                self.current_reps = current
                self.rep_completed = True
                send_to_puredata("rep_reward", 1)

        elif msg == "SET_OK":
            self.set_completed = True
            print("Set completed")
            send_to_puredata("set_reward", 1)

    # ----------------- Send to arduino -----------------
    def send(self, msg):
        self.write_queue.put('['+msg+']') # add the chars that arduino script wants to receive messages

    def reset_pose(self):
        self.send("RESET")

    def quit_set(self):
        self.set_completed = False
        self.current_reps = 0
        self.total_reps = 0
        self.send("QUIT")

    def set_reps(self, n):
        self.total_reps = n
        self.send(f"SET_N_REPS,{n}")

    def save_pose(self):
        self.send("SAVE_POSE")

    def send_wrist_unbalanced(self, unbalanced: bool):
        self.send(f"WRIST_UNBALANCED,{int(unbalanced)}")
        # send signal to puredata
        send_to_puredata("wrists", unbalanced)

    def send_knee_valgus(self, valgus: bool):
        self.send(f"KNEE_VALGUS,{int(valgus)}")
        #send signal to puredata
        send_to_puredata("knees", valgus)

    def send_startup_UI(self):
        self.send(f"INITIALIZE")

    def close(self):
        self.running = False
        self.ser.close()

