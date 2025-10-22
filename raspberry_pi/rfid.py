import serial
import time
from get_role_data import get_role_data
from send_to_role import send_to_role
from evdev import InputDevice, ecodes
import subprocess

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

def make_packet(data):
    start_byte = b'\x02'
    data_bytes = data.encode('utf-8')
    length_byte = bytes([len(data_bytes)])
    checksum = 0
    for b in data_bytes:
        checksum ^= b
    checksum_byte = bytes([checksum])
    return start_byte + length_byte + data_bytes + checksum_byte

def connect_wifi(ssid, pwd):
     cmd = ["sudo", "nmcli", "device", "wifi", "connect", ssid, "password", pwd]
     try:
         result = subprocess.run(cmd, capture_output=True, text=True, check=True)
         #print("Connect Sucess")
         #print(result.stdout)
         connect = True

     except subprocess.CalledProcessError as e:
         #print("Connect Failed")
         #print(e.stderr)
         connect = False

def check_wifi():
    cmd = ["nmcli", "-t", "-f", "active,ssid", "dev", "wifi"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        for line in result.stdout.splitlines():
            active, ssid = line.split(":", 1)
            #print(active.lower())
            if active.lower() == "是":
                return ssid
        return None
    except subprocess.CalledProcessError as e:
        print("Run nmcli falied:", e)
        return None

def main():
    print("Open Serial...")
    ser_send = connect_serial()

    connect = connect_wifi("ITALAB", "qwertyuio")


    while True:
        print("123")
        ssid = check_wifi()
        if ssid:
            print("Success Connect to Wifi:", ssid)
            try:
                dev = InputDevice('/dev/input/by-id/usb-RFID_Reader_RFID_Reader_HF5296500D-event-kbd')
                card_id = ""
                print("456")
                for event in dev.read_loop():
                                        #print("123")
                    if event.type == ecodes.EV_KEY and event.value == 1:
                        if event.code == ecodes.KEY_ENTER:
                            print("-----------------")
                            print("RFID:", card_id)

                            is_vip = get_role_data(card_id)
                            print("VIP:", is_vip)
                            if not is_vip:
                                send_to_role(card_id)
                                print(f"Already Send RFID: {card_id}")

                            packet = make_packet(card_id)
                            ser_send.write(packet)
                            print("Send Packet")
                            card_id = ""
                        else:
                            key = ecodes.KEY[event.code].replace("KEY_", "")
                            if key.isdigit():
                                card_id += key

            except serial.SerialException as e:
                print(f"Failed to Send: {e}, Trying to Reconnect")
                ser_send = connect_serial()

            except Exception as e:
                print(f"Another Error {e}")
                ser_send = connect_serial()

                time.sleep(1)

        else:
            print("Connect Wifi Falied, Trying to Reconnect Wifi")
            connect = connect_wifi("ITALAB", "qwertyuio")

if __name__ == "__main__":
    main()
