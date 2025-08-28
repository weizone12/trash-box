struct Recv_Result {
  String text;
  bool flag;
};

#include "HX711.h"

unsigned long start_time = 0;
bool timing = false;

const int trash_ir = A3;   // 垃圾紅外線
const int people_ir = A4;  // 人體紅外線
const int led = A5;        // 滿溢燈
const int but = 5;        // 垃圾桶按鈕
const int close = 7;      // 關垃圾桶
const int open = 8;       // 開垃圾桶
const int DT_Pin1 = 10;    // 秤重1 DT
const int SCK_Pin1 = 11;  // 秤重1 SCK
const int DT_Pin2 = 12;   // 秤重2 DT
const int SCK_Pin2 = 13;  // 秤重2 SCK
bool box_status = true;

const int scale_factor = 49;  // 校正參數
HX711 scale1;
HX711 scale2;

void sendPacket(String data) {
  byte startByte = 0x02;
  byte lengthByte = data.length();
  byte checksum = 0;

  // 計算 checksum
  for (int i = 0; i < data.length(); i++) {
    checksum ^= data[i];
  }

  // 發送封包
  Serial.write(startByte);       // Start
  Serial.write(lengthByte);      // Length
  Serial.write((const uint8_t*)data.c_str(), data.length());  // Data
  Serial.write(checksum);        // Checksum
}

Recv_Result recv_packet() {
  Recv_Result res;
  if (Serial.available() >= 3) { // 至少有 起始符 + 長度 + 一個資料
    if (Serial.read() == 0x02) { // 確認起始符
      int length = Serial.read();
      char data[50];
      int checksum = 0;

      for (int i = 0; i < length; i++) {
        while (!Serial.available());
        data[i] = Serial.read();
        checksum ^= data[i];
      }
      data[length] = '\0';

      while (!Serial.available());
      int recv_checksum = Serial.read();

      if (recv_checksum == checksum) {
        res.text = data;
        res.flag = true;
        return res;
      } else {
        Serial.println("檢查碼錯誤");
      }
    }
  }
  res.text = "";
  res.flag = false;
  return res;
}

void setup() {
  Serial.begin(9600);
  pinMode(trash_ir, INPUT);
  pinMode(people_ir, INPUT);
  pinMode(led, OUTPUT);
  pinMode(open, OUTPUT);
  pinMode(close, OUTPUT);
  pinMode(but, OUTPUT);
  digitalWrite(led, LOW);
  digitalWrite(but, LOW);

  // Serial.println("Initializing the scale");
  scale1.begin(DT_Pin1, SCK_Pin1);
  scale2.begin(DT_Pin2, SCK_Pin2);
  // Serial.println("Before setting up the scale:"); 
  // Serial.println(scale1.get_units(5), 0);	//未設定校正參數前的數值
  // Serial.println(scale2.get_units(5), 0);	//未設定校正參數前的數值
  scale1.set_scale(scale_factor);          // 設定校正參數
  scale2.set_scale(scale_factor);
  scale1.tare();				                    // 歸零
  scale2.tare();
  // Serial.println("After setting up the scale:"); 
  // Serial.println(scale.get_units(5), 0);  //設定比例參數後的數值
  // Serial.println("Readings:");            //在這個訊息之前都不要放東西在電子稱

  digitalWrite(open, LOW);
  digitalWrite(close, HIGH);
  delay(2000);
  digitalWrite(close, LOW);
}

bool cheated = false;
void loop() {
  // 垃圾滿溢
  if(digitalRead(trash_ir) == HIGH) {
    digitalWrite(led, LOW);
    bool cheated = false;

    // 解包
    Recv_Result res = recv_packet();
    String rfid_code = res.text;
    bool login = res.flag;

    // 開蓋
    if(login) {
      // Serial.println("收到資料，開啟上蓋");
      //Serial.print("rfid_code: ");
      //Serial.println(rfid_code);
      digitalWrite(open, HIGH);
      delay(3000);
      digitalWrite(open, LOW);
      box_status = true;
      sendPacket(rfid_code + "|" + box_status + "|" + NULL + "|" + cheated);

      while(box_status) {
        if(!timing) {
          timing = true;
          start_time = millis();  
        }
        if(timing) {
          unsigned long current_time = millis();
          if(digitalRead(people_ir) == LOW) {  // 偵測有人
            start_time = current_time;
          }
          // 檢查滿溢
          if(digitalRead(trash_ir) != HIGH) {
            cheated = true;
            digitalWrite(led, HIGH);
          }

          if(current_time - start_time >= 10000) {
            digitalWrite(close, HIGH);
            delay(2000);
            digitalWrite(close, LOW);
            box_status = false;
            timing = false;

            // 檢查滿溢
            if(digitalRead(trash_ir) != HIGH) {
              cheated = true;
              digitalWrite(led, HIGH);
            }

            // 測量重量
            float weight = scale1.get_units(10) + scale2.get_units(10);
            String str_weight = String(weight, 2);

            sendPacket(rfid_code + "|" + box_status + "|" + str_weight + "|" + cheated);
          }
        }
      }

      // 清除 Serial 緩衝
      while(Serial.available() >0) {
        Serial.read();
      }
      delay(10);
      // }
    } 

  } else {
    digitalWrite(led, HIGH);
    bool cheated = true;

    // 解包
    Recv_Result res = recv_packet();
    String rfid_code = res.text;
    bool login = res.flag;

  } delay(10);

  
  
  // // Serial.print("scale: ");
  // // Serial.println(scale1.get_units(10) + scale2.get_units(10), 0);
  // scale1.power_down();			        // 進入睡眠模式
  // scale2.power_down();
  // delay(500);
  // scale1.power_up();               // 結束睡眠模式
  // scale2.power_up();
}
