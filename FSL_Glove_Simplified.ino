// ============ SIMPLIFIED VERSION FOR BEGINNERS ============
// This is a more basic version with fewer gestures but easier to understand

#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>

// ============ PIN DEFINITIONS ============
#define MUX_S0 4
#define MUX_S1 5
#define MUX_S2 6
#define MUX_S3 7
#define MUX_OUT A0

#define BT_RX 2
#define BT_TX 3

#define LED_PIN 10  // Optional LED indicator

// ============ OBJECTS ============
MPU6050 mpu;
SoftwareSerial btSerial(BT_RX, BT_TX);

// ============ VARIABLES ============
int flexValues[10];
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;

int flexMin[10] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200};
int flexMax[10] = {800, 800, 800, 800, 800, 800, 800, 800, 800, 800};

unsigned long lastGestureTime = 0;

// ============ SETUP ============
void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  
  // Multiplexer pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_OUT, INPUT);
  
  // LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // I2C & MPU
  Wire.begin();
  delay(100);
  
  if (!mpu.begin(MPU_ADDR)) {
    Serial.println("MPU ERROR");
    while (1);
  }
  
  Serial.println("SYSTEM READY");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

// ============ MAIN LOOP ============
void loop() {
  readFlexSensors();
  readMPU6050();
  sendData();
  checkGestures();
  delay(100);
}

// ============ READ MULTIPLEXER ============
void readFlexSensors() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(MUX_S0, i & 1);
    digitalWrite(MUX_S1, (i >> 1) & 1);
    digitalWrite(MUX_S2, (i >> 2) & 1);
    digitalWrite(MUX_S3, (i >> 3) & 1);
    delay(2);
    flexValues[i] = analogRead(MUX_OUT);
  }
}

// ============ READ MPU ============
void readMPU6050() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);
  
  accelX = accel.acceleration.x;
  accelY = accel.acceleration.y;
  accelZ = accel.acceleration.z;
  gyroX = gyro.gyro.x;
  gyroY = gyro.gyro.y;
  gyroZ = gyro.gyro.z;
}

// ============ SEND DATA ============
void sendData() {
  String msg = "";
  for (int i = 0; i < 10; i++) {
    msg += flexValues[i];
    if (i < 9) msg += ",";
  }
  msg += "," + String(accelX, 1);
  msg += "," + String(accelY, 1);
  msg += "," + String(accelZ, 1);
  
  btSerial.println(msg);
  Serial.println(msg);
}

// ============ SIMPLE GESTURE CHECK ============
void checkGestures() {
  if (millis() - lastGestureTime < 500) return;  // Cooldown
  
  int threshold = (flexMin[0] + flexMax[0]) / 2;
  
  // Fist: all fingers flexed
  bool allFlexed = true;
  for (int i = 0; i < 5; i++) {
    if (flexValues[i] < threshold) allFlexed = false;
  }
  if (allFlexed) {
    btSerial.println("GESTURE:FIST");
    Serial.println(">> FIST <<");
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    lastGestureTime = millis();
  }
  
  // Open: all fingers extended
  bool allExtended = true;
  for (int i = 0; i < 5; i++) {
    if (flexValues[i] > threshold) allExtended = false;
  }
  if (allExtended) {
    btSerial.println("GESTURE:OPEN");
    Serial.println(">> OPEN <<");
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    lastGestureTime = millis();
  }
}
