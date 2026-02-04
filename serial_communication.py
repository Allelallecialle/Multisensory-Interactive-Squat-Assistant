import serial
import threading

#Python controller that sends to arduino: [RESET], [SET_N_REPS], [SAVE_POSE], [QUIT], [WRIST_UNBALANCED]
#Viceversa python receives: SQUATSTATE (i.e. is_squatting bool to know when to check valgus knees),
# pressure values to draw on UI, current number of repetitions
class SerialController:
    def __init__(self, port="/dev/ttyACM0", baud=115200):
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.running = True

        self.is_squatting = False

        self.thread = threading.Thread(target=self.read_loop, daemon=True)
        self.thread.start()

    # ----------------- Read from arduino -----------------
    def read_loop(self):
        while self.running:
            try:
                line = self.ser.readline().decode().strip()
                if line:
                    self.parse_message(line)
            except Exception as e:
                print("Serial error:", e)

    def parse_message(self, msg):
        if msg.startswith("SQUATSTATE:"):
            self.is_squatting = bool(int(msg.split(":")[1]))

        #TODO implement pressure data handling for UI
        # elif msg.startswith("PRESSURE:"):
        #     l, r, h = map(int, msg.split(":")[1].split(","))


        elif msg == "REP_OK":
            print("1 Rep completed")

        elif msg == "SET_OK":
            print("Set completed")

    # ----------------- Send to arduino -----------------
    def send(self, msg):
        self.ser.write((msg + "\n").encode())

    def reset_pose(self):
        self.send("RESET")

    def quit_set(self):
        self.send("QUIT")

    def set_reps(self, n):
        self.send(f"SET_N_REPS:{n}")

    def save_pose(self):
        self.send("SAVE_POSE")

    def send_wrist_unbalanced(self, unbalanced: bool):
        self.send(f"WRIST_UNBALANCED:{int(unbalanced)}")

    def close(self):
        self.running = False
        self.ser.close()

