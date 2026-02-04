import numpy as np
import threading

is_squatting = False
latest_result = None
rep_success = False
result_lock = threading.Lock()


# ----------------- MediaPipe callback -----------------
def pose_callback(result, output_image, timestamp_ms):
    global latest_result
    with result_lock:
        latest_result = result


# ----------------- Mediapipe squat functions -----------------
def lm_xy(lm, w, h):
    return np.array([lm.x * w, lm.y * h])


def check_knee_valgus(landmarks, w, h, threshold=0.90):
    if not is_squatting:
        return False

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


def barbell_bad_form(landmarks, w, h):
    if not is_squatting:
        return False

    LW, RW = 15, 16
    return abs(landmarks[LW].y - landmarks[RW].y) > 0.03
