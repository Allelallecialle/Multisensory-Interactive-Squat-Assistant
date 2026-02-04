from PySide6.QtCore import QTimer, Qt
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import (
    QMainWindow, QWidget,
    QLabel, QPushButton, QSpinBox,
    QVBoxLayout, QHBoxLayout, QSizePolicy)
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
from mediapipe_knees import *
import cv2
from mediapipe_pose_utils import draw_landmarks_on_image


# ----------------- Qt Main Window -----------------
class SquatUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Multisensory Interactive Squat System")
        self.resize(1200, 800)

        self.timestamp = 0

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
        self.timer.start(30)  # ~30 FPS

    # ----------------- Actions -----------------
    def save_pose(self):
        print("Pose saved (hook logic here)")
        # UI feedback
        self.statusBar().showMessage("Pose saved", 2000)

    def reset_pose(self):
        global rep_success
        rep_success = False
        print("Pose reset")
        # UI feedback
        self.statusBar().showMessage("Pose reset", 2000)

    def confirm_repetitions(self):
        n = self.reps_spin.value()

        self.repetitions_to_achieve = n
        self.squat_counter = 0
        self.set_configured = True

        print(f"Set configured: {n} reps")

        # send to Arduino (example to CHANGE)
        # if self.arduino:
        #     self.arduino.write(f"[REPS,{n}]".encode())

        # UI feedback
        self.statusBar().showMessage(f"Set configured: {n} reps", 2000)

    def quit_set(self):
        print("SET INTERRUPTED.")

        self.set_configured = False
        self.squat_counter = 0

        # inform Arduino (example to CHANGE)
        # if self.arduino:
        #     self.arduino.write(b"[QUIT,1]")

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
        self.landmarker.detect_async(mp_image, self.timestamp)
        self.timestamp += 33

        with result_lock:
            result = latest_result

        if result and result.pose_landmarks:
            landmarks = result.pose_landmarks[0]
            valgus = check_knee_valgus(landmarks, w, h)

            annotated = draw_landmarks_on_image(frame.copy(), result)

            if valgus:
                overlay = annotated.copy()
                overlay[:] = (0, 0, 255)
                annotated = cv2.addWeighted(annotated, 0.6, overlay, 0.4, 0)
                cv2.putText(
                    annotated, "KNEE VALGUS", (30, 40),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2
                )

            if barbell_bad_form(landmarks, w, h):
                cv2.putText(
                    annotated, "BARBELL OUT OF BALANCE", (30, 80),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2
                )

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

