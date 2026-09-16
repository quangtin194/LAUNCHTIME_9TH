// === LIB ===
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

// === VARIABLE ===
// PIN
  int STBY = 21;
  int IN1 = 12;
  int IN2 = 13;
  int IN3 = 14;
  int IN4 = 27;
  int ENA = 25;
  int ENB = 26;
  int LED_PIN = 2;

// PWM
  const int pwmFreq = 1000;
  const int pwmResolution = 8;
  const int channelENA = 0;
  const int channelENB = 1;

// TUNING
  int speedMax  = 255;
  int spinSpeed = 120;
  float turnGain = 0.45;

// DATA
String rxData = "";

// === SETUP ===
void setup() {
  Serial.begin(115200);
  SerialBT.begin("Xe_ESP32");

  // Enable driver
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // PWM Setup
  ledcSetup(channelENA, pwmFreq, pwmResolution);
  ledcSetup(channelENB, pwmFreq, pwmResolution);

  ledcAttachPin(ENA, channelENA);
  ledcAttachPin(ENB, channelENB);

  Serial.println("San sang nhan lenh...");
}

// === SUPER LOOP ===
void loop() {

  while (SerialBT.available()) {
    char c = SerialBT.read();

    if (c == '\n' || c == '\r') 
    {
      if (rxData.length() > 0) 
      {
        processData(rxData);
        rxData = "";
      }
    } else rxData += c;
  }
}

// === USER-DEFINED FUNCTION ===
// XỬ LÝ 
void processData(String data) {

  Serial.println("Nhan: " + data);
  int forward = 0;
  int turn = 0;

  for (int i = 0; i < data.length(); i++) 
  {
    if (data[i] == 'F') forward = map(data.substring(i + 1, i + 3).toInt(), 0, 99, 0, speedMax);

    if (data[i] == 'B') forward = -map(data.substring(i + 1, i + 3).toInt(), 0, 99, 0, speedMax);

    if (data[i] == 'R') turn = map(data.substring(i + 1, i + 3).toInt(), 0, 99, 0, speedMax) * turnGain;

    if (data[i] == 'L') turn = -map(data.substring(i + 1, i + 3).toInt(), 0, 99, 0, speedMax) * turnGain;
  }

  // TRỘN TỐC ĐỘ
  int speedLeft;
  int speedRight;

  // TIẾN
  if (forward >= 0) 
  {
    speedLeft  = forward + turn;
    speedRight = forward - turn;
  }

  // LÙI
  else 
  {
    speedLeft  = forward - turn;
    speedRight = forward + turn;
  }

  // SCALE CHỐNG VƯỢT
  int maxVal = max(abs(speedLeft), abs(speedRight));

  if (maxVal > speedMax) 
  {
    speedLeft  = speedLeft  * speedMax / maxVal;
    speedRight = speedRight * speedMax / maxVal;
  }

  // XOAY TẠI CHỖ
  if (forward == 0 && turn != 0) 
  {
    speedLeft  = constrain(speedLeft,  -spinSpeed, spinSpeed);
    speedRight = constrain(speedRight, -spinSpeed, spinSpeed);
  }

  driveMotor(speedLeft, speedRight);
}

// MOTOR
void driveMotor(int left, int right) {

  // MOTOR TRÁI
  if (left > 0) 
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    ledcWrite(channelENA, left);

  } else if (left < 0) 
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    ledcWrite(channelENA, -left);

  } else 
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    ledcWrite(channelENA, 0);
  }

  // MOTOR PHẢI
  if (right > 0) 
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    ledcWrite(channelENB, right);

  } else if (right < 0) 
  {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    ledcWrite(channelENB, -right);

  } else 
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    ledcWrite(channelENB, 0);
  }
}