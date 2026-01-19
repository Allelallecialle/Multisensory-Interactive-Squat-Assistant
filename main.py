import time
import cv2

cap = cv2.VideoCapture(0)

start_time = time.time()

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    h, w, _ = frame.shape

    mp_image = vision.MpImage(
        image_format=vision.ImageFormat.SRGB,
        data=frame
    )

    # ✅ CORRECT timestamp (ms)
    timestamp_ms = int((time.time() - start_time) * 1000)

    result = landmarker.detect_for_video(mp_image, timestamp_ms)

    if result.pose_landmarks:
        landmarks = result.pose_landmarks[0]
        # your valgus logic + drawing here

    cv2.imshow("Pose", frame)
    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
