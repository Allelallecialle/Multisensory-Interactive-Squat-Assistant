import cv2
import mediapipe as mp
import numpy as np
import serial
import time
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
from mediapipe_pose_utils import draw_landmarks_on_image
import threading


SERIAL_PORT = "/dev/ttyACM0"
BAUDRATE = 9600
#
# try:
#     arduino = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=0.01)
#     time.sleep(2)
#     print("Arduino connected")
# except:
#     arduino = None
#     print("Arduino NOT connected")

latest_result = None
result_lock = threading.Lock()
rep_success = False
def pose_callback(result, output_image, timestamp_ms):
    global latest_result
    with result_lock:
        latest_result = result


MODEL_PATH = "./mediapipe_models/pose_landmarker_lite.task"

base_options = python.BaseOptions(model_asset_path=MODEL_PATH)
options = vision.PoseLandmarkerOptions(
    base_options=base_options,
    running_mode=vision.RunningMode.LIVE_STREAM,
    num_poses=1,
    result_callback=pose_callback
)

landmarker = vision.PoseLandmarker.create_from_options(options)

def lm_xy(lm, w, h):
    return np.array([lm.x * w, lm.y * h])

def check_knee_valgus(landmarks, w, h, threshold=0.90):
    LK, RK, LA, RA = 25, 26, 27, 28  # mediapipe indices

    left_knee = lm_xy(landmarks[LK], w, h)
    right_knee = lm_xy(landmarks[RK], w, h)
    left_ankle = lm_xy(landmarks[LA], w, h)
    right_ankle = lm_xy(landmarks[RA], w, h)

    knee_dist = abs(left_knee[0] - right_knee[0])
    ankle_dist = abs(left_ankle[0] - right_ankle[0])

    if ankle_dist == 0:
        return False
    # if not is_squatting:
    #     return False

    valgus_ratio = knee_dist / ankle_dist

    return valgus_ratio < threshold

#---- Video loop ------------------------------------
# external camera. Set 0 for webcam
cam= cv2.VideoCapture(2)
rep_success = False
timestamp = 0

while cam.isOpened():
    ret, frame = cam.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    h, w, _ = frame.shape

    mp_image = mp.Image(
        image_format=mp.ImageFormat.SRGB,
        data=frame
    )
    landmarker.detect_async(mp_image, timestamp)

    timestamp += 33

    valgus = False

    # Arduino feedback
    # if arduino and arduino.in_waiting:
    #     msg = arduino.readline().decode(errors="ignore").strip()
    #     if msg == "OK":
    #         rep_success = True
    #     elif msg == "RESET":
    #         rep_success = False

    with result_lock:
        result = latest_result

    if result and result.pose_landmarks:
        landmarks = result.pose_landmarks[0]
        valgus = check_knee_valgus(landmarks, w, h)

        # Color logic
        color = None
        if rep_success:
            color = (0, 255, 0)
        elif valgus:
            color = (0, 0, 255)


        annotated = draw_landmarks_on_image(
            frame.copy(),
            result
        )
        # Apply color overlay logic
        if color == (0, 0, 255) or color == (0, 255, 0):  # yellow = default
            overlay = annotated.copy()
            overlay[:] = color
            annotated = cv2.addWeighted(annotated, 0.6, overlay, 0.4, 0)

        frame[:] = annotated


        if valgus:
            cv2.putText(frame, "KNEE VALGUS", (30, 40),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        if rep_success:
            cv2.putText(frame, "REP OK", (30, 80),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    cv2.imshow("Squat Pose Monitor", frame)

    if cv2.waitKey(1) & 0xFF == 27:
        break

cam.release()
cv2.destroyAllWindows()

