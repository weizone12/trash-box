import json
import urllib.request

url = "http://140.133.74.174:8000/api/register-role/"

def get_role_data(rfid_code):
    with urllib.request.urlopen(url) as res:
            data = res.read().decode('utf-8')
            data = json.loads(data)

    #print("\nVIPS: ")
    for row in data:
        #print(f"ID: {row['id']}, Name: {row['name']}, RFID: {row['rfid_code']}, Created: {row['created_at']}")
        if rfid_code == row['rfid_code']:
            return True
    return False

#print(get_role_data(input()))
