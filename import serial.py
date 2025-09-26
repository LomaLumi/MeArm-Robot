import serial
import time

# ===== ตั้งค่า Serial Port =====
PORT = "COM3"      # เปลี่ยนเป็น COM port ของ ESP32 (Windows)
# PORT = "/dev/ttyUSB0"  # Linux/Mac
BAUD = 115200

# ===== เปิด Serial =====
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)  # รอ ESP32 รีเซ็ต

# ===== เปิดไฟล์ log.txt ไว้เก็บ =====
with open("log.txt", "a") as f:
    print("Start logging... (กด Ctrl+C เพื่อหยุด)")
    while True:
        try:
            line = ser.readline().decode().strip()
            if line:
                f.write(line + "\n")
                f.flush()
                print("Saved:", line)
        except KeyboardInterrupt:
            print("Stopped by user")
            break
