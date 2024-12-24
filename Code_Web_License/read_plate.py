import time
import os
import cv2
import numpy as np
from lib_detection import load_model, detect_lp, im2single
import firebase_admin
from firebase_admin import credentials, db

# Path đến tệp JSON chứng thực mà bạn đã tải về
cred = credentials.Certificate("D:/DATKLLL/DA_OFFICIAL/TKLL_ParkingSystem/License_Web/License_Web/License_Web/firebase1.json")

# Khởi tạo Firebase với chứng thực và Realtime Database URL
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://parkingsystem-9d434-default-rtdb.firebaseio.com/',
})

# Create the "test" folder if it does not exist
if not os.path.exists("test"):
    os.makedirs("test")

# Load model LP detection
wpod_net_path = "wpod-net_update1.json"
wpod_net = load_model(wpod_net_path)

# Cấu hình tham số cho model SVM
digit_w = 30 # Kích thước ký tự
digit_h = 60 # Kích thước ký tự

model_svm = cv2.ml.SVM_load('svm.xml')

# Hàm sắp xếp contour từ trái sang phải
def sort_contours(cnts):
    reverse = False
    i = 0
    boundingBoxes = [cv2.boundingRect(c) for c in cnts]
    (cnts, boundingBoxes) = zip(*sorted(zip(cnts, boundingBoxes),
                                        key=lambda b: b[1][i], reverse=reverse))
    return cnts

# Dinh nghia các ký tự trên biển số
char_list = '0123456789ABCDEFGHKLMNPRSTUVXYZ-'

# Hàm fine tune biển số, loại bỏ các ký tự không hợp lý
def fine_tune(lp):
    newString = ""
    for i in range(len(lp)):
        if lp[i] in char_list:
            newString += lp[i]
    return newString


# Danh sách lưu biển số bên phải, chỉ số bắt đầu từ 1
right_licenses = []

def update_parking_data(carlicense, gate_side):
    """
    Cập nhật dữ liệu biển số xe dựa trên danh sách local, đẩy lên Firebase khi có khớp.
    """
    global right_licenses  # Danh sách lưu biển số bên phải

    ref = db.reference('ParkingData')  # Tham chiếu Firebase

    if gate_side == "right":
        # Lưu biển số bên phải vào danh sách
        right_licenses.append(carlicense)
        print(f"Added car license {carlicense} to right_licenses at position {len(right_licenses)}.")

        # Đẩy thông tin lên Firebase cho biển số bên phải
        node_id = len(right_licenses)  # Node trên Firebase bắt đầu từ 1
        ref.child(f'{node_id}/CarlicenseRight').set(carlicense)
        print(f"Pushed car license {carlicense} to Firebase at node {node_id}.")

    elif gate_side == "left":
        # Tìm trong danh sách bên phải
        try:
            matched_index = right_licenses.index(carlicense)  # Vị trí (index) trong danh sách, bắt đầu từ 0
            node_id = matched_index + 1  # Node trên Firebase bắt đầu từ 1

            # Đẩy thông tin lên Firebase
            ref.child(f'{node_id}/CarlicenseLeft').set(carlicense)
            print(f"Matched car license {carlicense} to node {node_id}.")

            # Xóa mục tương ứng khỏi danh sách
            right_licenses[matched_index] = None  # Hoặc dùng del để xóa
        except ValueError:
            print(f"No matching right license found for {carlicense} on the left gate.")


# Vòng lặp chính, chạy mã liên tục
while True:
    try:
        # Tham chiếu đến nút openGate_L trong Realtime Database
        openL = db.reference('ParkingData/Status/leftStatus')
        openR = db.reference('ParkingData/Status/rightStatus')
        openL_status = openL.get()  # Lấy giá trị từ nút
        openR_status = openR.get()
    #     print("Status of openR:", openR_status)
    # except Exception as e:
    #     print("Error occurred:", e)

        # Kiểm tra giá trị của nút openGate_L
        if openR_status == True or openL_status == True:
            open_gate_status = "false"
            print("openGate_L is True. Proceeding with license plate detection...")

            # Open the webcam
            cap = cv2.VideoCapture(0)

            # Check if the camera opened successfully
            if not cap.isOpened():
                print("Error: Could not open webcam.")
                break

            # Display the webcam feed
            print("Webcam opened. Waiting for 5 seconds before capturing the image...")
            start_time = time.time()

            while True:
                # Capture frame-by-frame
                ret, frame = cap.read()

                # Show the frame in a window named "Webcam Feed"
                cv2.imshow("Webcam Feed", frame)

                # Wait for 5 seconds
                if time.time() - start_time > 3:
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

            # Đọc file ảnh đầu vào
            img_path = "test/test1.jpg"

            # Đọc ảnh từ file
            Ivehicle = cv2.imread(img_path)

            # Kích thước lớn nhất và nhỏ nhất của 1 chiều ảnh
            Dmax = 608
            Dmin = 288

            # Lấy tỷ lệ giữa W và H của ảnh và tìm ra chiều nhỏ nhất
            ratio = float(max(Ivehicle.shape[:2])) / min(Ivehicle.shape[:2])
            side = int(ratio * Dmin)
            bound_dim = min(side, Dmax)

            _ , LpImg, lp_type = detect_lp(wpod_net, im2single(Ivehicle), bound_dim, lp_threshold=0.5)

            if len(LpImg):
                # Chuyển đổi ảnh biển số
                LpImg[0] = cv2.convertScaleAbs(LpImg[0], alpha=(255.0))

                roi = LpImg[0]

                # Chuyển ảnh biển số về gray
                gray = cv2.cvtColor(LpImg[0], cv2.COLOR_BGR2GRAY)

                # Áp dụng threshold để phân tách số và nền
                binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)[1]

                cv2.imshow("Ảnh biển số sau threshold", binary)
                cv2.waitKey()

                # Segment ký tự
                kernel3 = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
                thre_mor = cv2.morphologyEx(binary, cv2.MORPH_DILATE, kernel3)
                cont, _  = cv2.findContours(thre_mor, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

                plate_info = ""
                plate_rows = []  # Để lưu các dòng biển số

                # Sắp xếp các contours
                sorted_contours = sort_contours(cont)

                # Kiểm tra xem biển số có 1 dòng hay không
                total_height = sum([cv2.boundingRect(c)[3] for c in sorted_contours])
                avg_height = total_height / len(sorted_contours)

                # Nếu tổng chiều cao lớn hơn một ngưỡng nào đó, xử lý như biển số 2 hàng
                if avg_height > 30:  # Điều này có thể điều chỉnh dựa trên kích thước thực tế của biển số
                    # Chia thành 2 dòng
                    row1 = []
                    row2 = []
                    for c in sorted_contours:
                        (x, y, w, h) = cv2.boundingRect(c)
                        if y < roi.shape[0] / 2:
                            row1.append(c)
                        else:
                            row2.append(c)

                    plate_rows = [row1, row2]
                else:
                    # Biển số 1 hàng
                    plate_rows = [sorted_contours]

                # Xử lý biển số theo từng dòng
                for row in plate_rows:
                    for c in row:
                        (x, y, w, h) = cv2.boundingRect(c)
                        ratio = h / w
                        if 1.5 <= ratio <= 3.5:  # Chọn các contour có tỉ lệ hợp lý
                            if h / roi.shape[0] >= 0.6:  # Chọn các contour cao từ 35% biển số trở lên
                                # Vẽ khung chữ nhật quanh số
                                cv2.rectangle(roi, (x, y), (x + w, y + h), (0, 255, 0), 2)

                                # Tách số và dự đoán
                                curr_num = thre_mor[y:y+h, x:x+w]
                                curr_num = cv2.resize(curr_num, dsize=(digit_w, digit_h))
                                _, curr_num = cv2.threshold(curr_num, 30, 255, cv2.THRESH_BINARY)
                                curr_num = np.array(curr_num, dtype=np.float32)
                                curr_num = curr_num.reshape(-1, digit_w * digit_h)

                                # Dự đoán với model SVM
                                result = model_svm.predict(curr_num)[1]
                                result = int(result[0, 0])

                                if result <= 9:  # Nếu là số thì hiển thị luôn
                                    result = str(result)
                                else:  # Nếu là chữ thì chuyển từ ASCII
                                    result = chr(result)

                                plate_info += result

                cv2.imshow("Các contour tìm được", roi)
                cv2.waitKey()

                # Viết biển số lên ảnh
                #cv2.putText(Ivehicle, fine_tune(plate_info), (50, 50), cv2.FONT_HERSHEY_PLAIN, 3.0, (0, 0, 255), lineType=cv2.LINE_AA)

                # Hiển thị ảnh
                print("Biển số =", plate_info)
                cv2.imshow("Hình ảnh output", Ivehicle)
                cv2.waitKey()

                # Lưu ảnh roi vào file
                contour_image_path = "test/contour_output.jpg"
                cv2.imwrite(contour_image_path, roi)
                print(f"Contour image saved at {contour_image_path}")

                # if openR_status == "true":
                #     # Reference tới một vị trí trong Realtime Database
                #     ur = db.reference('ParkingData/UID_Right')
                #     ur1 = ur.get()  # Lấy giá trị từ nút
                #     ref = db.reference('UIDs/Right')
                #     ref.set({
                #         "carlicense": plate_info,
                #         "UID": ur1
                #     })
                
                # if openL_status == "true":
                #     # Reference tới một vị trí trong Realtime Database
                #     ul = db.reference('ParkingData/UID_Left')
                #     ul1 = ul.get()  # Lấy giá trị từ nút
                #     ref = db.reference('UIDs/Left')
                #     ref.set({
                #         "carlicense": plate_info,
                #         "UID": ul1
                #     })
                if openR_status:
                    # ur = db.reference('ParkingData/UID_Right')
                    # ur1 = ur.get()  # Lấy UID từ cổng phải
                    update_parking_data(carlicense=plate_info, gate_side="right")

                if openL_status:
                    # ul = db.reference('ParkingData/UID_Left')
                    # ul1 = ul.get()  # Lấy UID từ cổng trái
                    update_parking_data(carlicense=plate_info, gate_side="left")

        else:
            print("openGate_L is False. Skipping license plate detection.")

    except Exception as e:
        print(f"An error occurred: {e}")
        break

