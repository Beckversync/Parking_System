# ESP32 License Plate Detection và WebApp Firebase  

## Giới thiệu  
Dự án này tích hợp ESP32 với ứng dụng web để quản lý bãi đỗ xe tự động. Hệ thống sử dụng ESP32 để kết nối các cảm biến, nhận diện biển số xe (license plate detection), và cập nhật trạng thái đỗ xe lên cơ sở dữ liệu Firebase. Webapp được phát triển để quản lý và hiển thị thông tin bãi đỗ theo thời gian thực.  

---

## Tính năng chính  

### ESP32  
- Đọc dữ liệu từ cảm biến (RFID, hồng ngoại).  
- Nhận diện biển số xe bằng thư viện hoặc API chuyên dụng.  
- Gửi và nhận dữ liệu qua Firebase.  

### WebApp  
- Hiển thị trạng thái bãi đỗ xe (số chỗ trống, xe hiện tại, v.v.).  
- Cập nhật trạng thái bãi đỗ và điều khiển barrier theo thời gian thực.  

### Firebase  
- Lưu trữ thông tin xe ra/vào.  
- Quản lý trạng thái barrier và slot đỗ.  

---

## Kiến trúc hệ thống  

### Phần cứng  
- ESP32 (ESP-WROOM-32).  
- Cảm biến hồng ngoại, RFID, và camera nhận diện biển số.  
- Barrier servo để kiểm soát lối vào/ra.  

### Phần mềm  
- WebApp phát triển bằng C#, tích hợp Firebase Realtime Database.  
- Firebase dùng để lưu trữ và xử lý dữ liệu.  
- ESP32 sử dụng FreeRTOS để quản lý các task.  

---  

