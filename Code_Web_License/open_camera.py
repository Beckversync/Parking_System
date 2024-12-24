# import cv2
# cap = cv2.VideoCapture(0)
# while True:
#     ret, frame = cap.read()
#     print(ret)
#     cv2.imshow("Cua so cam", frame)
#     if cv2.waitKey(1) == ord("q"):
#         break
# cap.realease()
# cv2.destroyAllWindows()

import cv2
import os
import time

# Create the "test" folder if it does not exist
if not os.path.exists("test"):
    os.makedirs("test")

# Open the webcam
cap = cv2.VideoCapture(0)

# Check if the camera opened successfully
if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

# Display the webcam feed
print("Webcam opened. Waiting for 5 seconds before capturing the image...")
start_time = time.time()

while True:
    # Capture frame-by-frame
    ret, frame = cap.read()

    # Show the frame in a window named "Webcam Feed"
    cv2.imshow("Webcam Feed", frame)

    # Wait for 5 seconds
    if time.time() - start_time > 5:
        # Save the frame after 5 seconds
        cv2.imwrite("test/test1.jpg", frame)
        print("Image saved successfully in the 'test' folder.")
        break

    # Close the window if 'q' is pressed
    if cv2.waitKey(1) == ord("q"):
        break

# Release the webcam and close all OpenCV windows
cap.release()
cv2.destroyAllWindows()




