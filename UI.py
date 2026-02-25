import time
from PySide6.QtCore import QTimer, Qt
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import (
    QMainWindow, QWidget,
    QLabel, QPushButton, QSpinBox,
    QVBoxLayout, QHBoxLayout, QSizePolicy, QApplication)
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
from mediapipe_knees import *
import cv2
import threading
from mediapipe_pose_utils import draw_landmarks_on_image


# ----------------- MediaPipe callback -----------------
result_lock = threading.Lock()
def pose_callback(result, output_image, timestamp_ms):
    global latest_result
    with result_lock:
        latest_result = result

# ----------------- Mediapipe squat functions -----------------
latest_result = None
def lm_xy(lm, w, h):
    return np.array([lm.x * w, lm.y * h])


def check_knee_valgus(landmarks, w, h, threshold=0.90):
    # if not is_squatting:
    #     return False

    LK, RK, LA, RA = 25, 26, 27, 28
    left_knee = lm_xy(landmarks[LK], w, h)
    right_knee = lm_xy(landmarks[RK], w, h)
    left_ankle = lm_xy(landmarks[LA], w, h)
    right_ankle = lm_xy(landmarks[RA], w, h)

    ankle_dist = abs(left_ankle[0] - right_ankle[0])
    if ankle_dist == 0:
        return False

    knee_dist = abs(left_knee[0] - right_knee[0])
    return (knee_dist / ankle_dist) < threshold


def barbell_bad_form(landmarks):
    # if not is_squatting:
    #     return False

    LW, RW = 15, 16
    return abs(landmarks[LW].y - landmarks[RW].y) > 0.03


# ----------------- Qt Main Window -----------------
class SquatUI(QMainWindow):
    def __init__(self, arduino):
        super().__init__()
        self.setWindowTitle("Multisensory Interactive Squat System")
        self.resize(1200, 800)

        self.timestamp = 0
        self.arduino = arduino
        # ---- Camera ----
        self.cam = cv2.VideoCapture(0)

        # ---- MediaPipe ----
        base_options = python.BaseOptions(
            model_asset_path="./mediapipe_models/pose_landmarker_lite.task"
        )
        options = vision.PoseLandmarkerOptions(
            base_options=base_options,
            running_mode=vision.RunningMode.LIVE_STREAM,
            num_poses=1,
            result_callback=pose_callback
        )
        self.landmarker = vision.PoseLandmarker.create_from_options(options)

        # ---- UI ----
        self.video_label = QLabel()
        self.video_label.setAlignment(Qt.AlignCenter)
        self.video_label.setMinimumSize(640, 480)
        self.video_label.setSizePolicy(
            QSizePolicy.Expanding,
            QSizePolicy.Expanding
        )

        self.save_btn = QPushButton("Save pose")
        self.reset_btn = QPushButton("Reset pose")
        self.quit_btn = QPushButton("Quit")
        self.confirm_reps_btn = QPushButton("Confirm repetitions")
        self.status = self.statusBar()
        self.status.setFixedHeight(28)  # small bottom box to show UI messages
        self.status.showMessage("Ready")

        self.repetitions_to_achieve = 0
        self.squat_counter = 0

        self.reps_spin = QSpinBox()
        self.reps_spin.setRange(1, 20)
        self.reps_spin.setValue(10)

        controls = QVBoxLayout()
        controls.addWidget(self.save_btn)
        controls.addWidget(self.reset_btn)
        controls.addWidget(QLabel("Repetitions"))
        controls.addWidget(self.reps_spin)
        controls.addWidget(self.confirm_reps_btn)
        controls.addStretch()
        controls.addWidget(self.quit_btn)

        bottom = QHBoxLayout()
        bottom.addLayout(controls)

        main = QVBoxLayout()
        main.addWidget(self.video_label, stretch=4)
        main.addLayout(bottom, stretch=1)

        container = QWidget()
        container.setLayout(main)
        self.setCentralWidget(container)

        # ---- Signals ----
        self.quit_btn.clicked.connect(self.quit_set)
        self.save_btn.clicked.connect(self.save_pose)
        self.reset_btn.clicked.connect(self.reset_pose)
        self.confirm_reps_btn.clicked.connect(self.confirm_repetitions)

        # ---- Timer ----
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_frame)
        self.timer.start(30)

    # ---- Stop UI ----
    def closeEvent(self, event):
        print("Closing UI...")
        self.timer.stop()

        if self.cam.isOpened():
            self.cam.release()

        self.arduino.close()  # stop thread and close serial communication
        QApplication.quit()
        event.accept()

    # ----------------- Actions -----------------
    def save_pose(self):
        self.arduino.save_pose()
        print("Pose saved")
        # UI feedback
        self.statusBar().showMessage("Pose saved", 2000)

    def reset_pose(self):
        global rep_success
        rep_success = False
        self.arduino.reset_pose()
        print("Pose reset")
        # UI feedback
        self.statusBar().showMessage("Pose reset", 2000)

    def confirm_repetitions(self):
        n = self.reps_spin.value()

        self.repetitions_to_achieve = n
        self.squat_counter = 0
        self.set_configured = True

        self.arduino.set_reps(n)
        print(f"Set configured: {n} reps")
        # UI feedback
        self.statusBar().showMessage(f"Set configured: {n} reps", 2000)

    def quit_set(self):
        self.arduino.quit_set()
        print("SET INTERRUPTED.")

        self.set_configured = False
        self.squat_counter = 0

        # UI feedback
        self.statusBar().showMessage("Set interrupted. Choose repetitions for next set.")

    # ----------------- Video loop -----------------
    def update_frame(self):
        ret, frame = self.cam.read()
        if not ret:
            return

        frame = cv2.flip(frame, 1)
        h, w, _ = frame.shape

        mp_image = mp.Image(
            image_format=mp.ImageFormat.SRGB,
            data=frame
        )
        #self.landmarker.detect_async(mp_image, self.timestamp)
        #self.timestamp += 33
        timestamp_ms = int(time.time() * 1000)
        self.landmarker.detect_async(mp_image, timestamp_ms)

        with result_lock:
            result = latest_result
        if result is None:
            self.display_frame(frame)
            return

        if result and result.pose_landmarks:
            landmarks = result.pose_landmarks[0]
            annotated = draw_landmarks_on_image(frame.copy(), result)

            #check valgus knees only if the arduino sends that the user is actually squatting
            if self.arduino.is_squatting:
                print("squatting")
                valgus = check_knee_valgus(landmarks, w, h)
                if valgus:
                    overlay = annotated.copy()
                    overlay[:] = (0, 0, 255)
                    annotated = cv2.addWeighted(annotated, 0.6, overlay, 0.4, 0)
                    cv2.putText(
                        annotated, "KNEE VALGUS", (30, 40),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2
                    )

                unbalanced_wrists = barbell_bad_form(landmarks)
                if unbalanced_wrists:
                    cv2.putText(
                        annotated, "BARBELL OUT OF BALANCE", (30, 80),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2
                    )
                #send serial boolean for barbell balance: 1 if unbalanced, 0 balanced
                self.arduino.send_wrist_unbalanced(unbalanced_wrists)

            frame = annotated

        self.display_frame(frame)

    # ----------------- Opencv frame to Qt -----------------
    def display_frame(self, frame):
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        h, w, ch = rgb.shape
        img = QImage(rgb.data, w, h, ch * w, QImage.Format_RGB888)

        pix = QPixmap.fromImage(img)

        pix = pix.scaled(
            self.video_label.size(),
            Qt.KeepAspectRatio,
            Qt.SmoothTransformation
        )

        self.video_label.setPixmap(pix)

