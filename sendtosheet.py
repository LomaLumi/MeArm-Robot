import requests
import time

# URL ของ Google Apps Script Web App (ต้องแก้เป็นของเราเอง)
url = "https://script.google.com/macros/s/yourID/exec"

# ฟังก์ชันอ่านบรรทัดสุดท้ายจากไฟล์
def read_last_line(filename="log.txt"):
    with open(filename, "r", encoding="utf-8") as f:
        lines = f.readlines()
    return lines[-1].strip() if lines else ""   # คืนค่าบรรทัดสุดท้าย

while True:
    try:
        # อ่านข้อมูลจาก log.txt (บรรทัดล่าสุด)
        last_line = read_last_line("log.txt")
        
        if last_line:
            parts = last_line.split(",")   # แยกค่าด้วย comma
            if len(parts) == 4:            # ต้องมี 4 ค่า: x, y, z, status
                x, y, z, status = parts

                # จัดรูปแบบข้อมูลสำหรับส่งไป Google Sheets
                data = {
                    "sts": "write",
                    "x": x,
                    "y": y,
                    "z": z,
                    "analog": "0",   # กำหนดค่าเริ่มต้น
                    "status": status
                }

                # ส่งข้อมูลไปยัง Google Sheets ผ่าน Web App
                r = requests.get(url, params=data)
                print("Sent:", data, "| Response:", r.text)
            else:
                print("Format error:", last_line)

    except Exception as e:   # ถ้ามี error ให้แสดงผล
        print("Error:", e)

    time.sleep(0.1)          # หน่วงเวลา 0.1 วินาที (ปรับได้)
