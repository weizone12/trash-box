import json
import urllib.request

url = "http://140.133.74.174:8000/api/register-role/"

def send_to_role(rfid_code):
    data = {
        #"name": 1,
        "rfid_code": rfid_code,
        #"user_number": "ccc",
        #"created_at": ""
    }

    json_data = json.dumps(data).encode('utf-8')

    req = urllib.request.Request(
        url,
        data=json_data,
        headers={"Content-Type": "application/json"},
        method="POST"
    )

    with urllib.request.urlopen(req) as res:
        data = res.read().decode('utf-8')
        print("Send Data:", data)

'''
if __name__ == "__main__":
    send_to_role(input())
'''
