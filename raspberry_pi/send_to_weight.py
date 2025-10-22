import json
import urllib.request

url = "http://140.133.74.174:8000/api/delivery-records/"

def send_to_weight(rfid_code, status, weight, is_cheated):
    data = {
        "id": 3,
        "rfid_code": rfid_code,
        "user_name": "3",
        "device_id": "07高雄",
        "device_code": "01",
        "status": status,
        "weight": weight,
        "score": 2,
        #"delivered_at": "2222-02-22T11:11:00Z",
        #"created_at": "2025-08-09T08:54:56.838000Z",
        "is_cheated": is_cheated,
        #"user": None
    }

    json_data = json.dumps(data).encode('utf-8')

    req = urllib.request.Request(
        url,
        data=json_data,
        headers={"Content-Type": "application/json"},
        method="POST"
    )
    try:
        with urllib.request.urlopen(req) as res:
            print(res.read().decode('utf-8'))
    except urllib.error.HTTPError as e:
        error_body = e.read()
        print("Error code:", e.code)
        print("Error message:", e.reason)
        #print("Error body:", error_body.decode())
