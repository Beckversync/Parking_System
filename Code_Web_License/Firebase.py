# import firebase_admin
# from firebase_admin import credentials, db

# # Khởi tạo ứng dụng Firebase với tệp JSON khóa riêng
# cred = credentials.Certificate("path/to/your/serviceAccountKey.json")
# firebase_admin.initialize_app(cred, {
#     "databaseURL": "https://parking-8d733-default-rtdb.firebaseio.com/"
# })

# # Ghi dữ liệu vào Realtime Database
# ref = db.reference("results")
# ref.push({
#     "key": "value",  # Thay "key" và "value" bằng dữ liệu bạn muốn lưu
#     "result": 12345  # Ví dụ: kết quả từ đoạn mã của bạn
# })
# print("Kết quả đã được lưu lên Firebase.")