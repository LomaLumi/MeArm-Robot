import requests
import time

# URL ของ Google Apps Script Web App (แก้เป็นของคุณเอง)
url = "https://script.google.com/macros/s/yourdeploymentid/exec"

def read_last_line(filename="data.txt"):
    with open(filename, "r", encoding="utf-8") as f:
        lines = f.readlines()
    return lines[-1].strip() if lines else ""

while True:
    try:
        # อ่านบรรทัดสุดท้ายจากไฟล์
        last_line = read_last_line("log.txt")
        
        if last_line:
            parts = last_line.split(",")
            if len(parts) == 4:
                x, y, z, status = parts

                # เตรียมข้อมูลส่งไป Google Sheets
                data = {
                    "sts": "write",
                    "x": x,
                    "y": y,
                    "z": z,
                    "analog": "0",   # ค่า default ถ้าไฟล์ไม่มี
                    "status": status
                }

                # ส่งไป Google Sheets
                r = requests.get(url, params=data)
                print("Sent:", data, "| Response:", r.text)
            else:
                print("Format error:", last_line)

    except Exception as e:
        print("Error:", e)

    # รอ 0.5 วินาที
    time.sleep(0.1)
