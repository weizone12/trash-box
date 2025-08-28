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

void setup() {
  Serial.begin(9600);
  pinMode(trash_ir, INPUT);
  pinMode(people_ir, INPUT);
  pinMode(led, OUTPUT);
  pinMode(open, OUTPUT);
  pinMode(close, OUTPUT);
  pinMode(but, OUTPUT);
  digitalWrite(led, HIGH);
  digitalWrite(but, LOW);

  Serial.println("Initializing the scale");
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
  Serial.println("Readings:");            //在這個訊息之前都不要放東西在電子稱

  digitalWrite(open, LOW);
  digitalWrite(close, HIGH);
  delay(2000);
  digitalWrite(close, LOW);
}

void loop() {
  // 垃圾滿溢
  if(digitalRead(trash_ir) == HIGH) {
    digitalWrite(led, LOW);
  } else {
    digitalWrite(led, HIGH);
  } delay(10);
  
  if(digitalRead(but) == HIGH) {
    digitalWrite(open, HIGH);
    delay(3000);
    digitalWrite(open, LOW);
    box_status = true;

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
        if(current_time - start_time >= 3000) {
          digitalWrite(close, HIGH);
          delay(2000);
          digitalWrite(close, LOW);
          box_status = false;
          timing = false;
        }
      }
    }
  }

  // 測量重量
  Serial.print("scale: ");
  Serial.println(scale1.get_units(10) + scale2.get_units(10), 0);
  scale1.power_down();			        // 進入睡眠模式
  scale2.power_down();
  delay(500);
  scale1.power_up();               // 結束睡眠模式
  scale2.power_up();
}
