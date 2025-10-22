import serial
import time
from send_to_weight import send_to_weight

def connect_serial():
    ports = ["/dev/ttyACM0", "/dev/ttyUSB0", "/dev/ttyACM1", "/dev/ttyUSB1"]
    while True:
        for port in ports:
            try:
                ser = serial.Serial(port, 9600, timeout=1)
                time.sleep(2)
                print(f"Sucess Connect Arduino: {port}")
                return ser
            except serial.SerialException as e:
                print(f"Can't Connect {port}, Trying Another Port")
        print("Can't Find Useful Port, Retry in 3 second")
        time.sleep(3)

def recv_packet(ser):
    # 尋找起始符
    while True:
        start = ser.read(1)
        if not start:
            return None
        if start[0] == 0x02:
            break

    # 讀 長度
    length_bytes = ser.read(1)
    if not length_bytes:
        return None
    length = length_bytes[0]

    # 讀資料
    data_bytes = ser.read(length)
    if len(data_bytes) != length:
        return None

    # 讀 checksum
    checksum_bytes = ser.read(1)
    if not checksum_bytes:
        return None
    checksum = checksum_bytes[0]

    # 驗證 checksum
    calc_checksum = 0
    for b in data_bytes:
        calc_checksum ^= b

    if checksum == calc_checksum:
        try:
            data_str = data_bytes.decode('utf-8')
        except UnicodeDecodeError:
            data_str = data_bytes.hex()  # 無法解碼時顯示十六進位
        return data_str
    else:
        print("Checksum error")
        return None


def main():
    ser = connect_serial()

    while True:
        datas = []
        while True:
            data = ser.readline().decode('utf-8').strip()
            if data == "END":
                break
            if data:
                datas.append(data)
        if datas[2] == "0":
            weight = None
        else:
            weight = datas[2]
        print("RFID:", datas[0], ",", "Weight:", weight)
        send_to_weight(datas[0], datas[1], weight, datas[3])

'''
        try:
            packet = recv_packet(ser)
            print(packet)
            if packet != None:
                datas = packet.split("|")
                rfid_code = datas[0]
                status = datas[1]
                weight = datas[2]
                cheated = datas[3]
                if weight == "0":
                    weight = None
                print("RFID:", rfid_code, ",", "Weight:", weight)
                send_to_weight(rfid_code, status, weight, cheated)
        except Exception as e:
            print("Error:", e)
            ser = connect_serial()
'''

if __name__ == "__main__":
    main()
