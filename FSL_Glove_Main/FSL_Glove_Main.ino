#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ============ PIN DEFINITIONS ============
// Flex Sensors (CD74HC4067 Multiplexer)
#define MUX_S0 4
#define MUX_S1 5
#define MUX_S2 6
#define MUX_S3 7
#define MUX_OUT A0

// MPU-6050 (I2C: SDA=A4, SCL=A5)
#define MPU_ADDR 0x68

// HC-05 Bluetooth Module
#define BT_RX 2
#define BT_TX 3

// DFPlayer Mini
#define DF_RX 8
#define DF_TX 9

// Speaker
#define SPEAKER_PIN 10

// ============ OBJECT DECLARATIONS ============
MPU6050 mpu;
SoftwareSerial btSerial(BT_RX, BT_TX);      // RX, TX for HC-05
SoftwareSerial dfSerial(DF_RX, DF_TX);      // RX, TX for DFPlayer Mini
DFRobotDFPlayerMini dfPlayer;

// ============ SENSOR VARIABLES ============
int flexValues[10];           // 10 flex sensors
float accelX, accelY, accelZ; // Accelerometer
float gyroX, gyroY, gyroZ;    // Gyroscope
float tempC;                  // Temperature

// Flex sensor calibration
int flexMin[10] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200};  // Adjust based on your sensors
int flexMax[10] = {800, 800, 800, 800, 800, 800, 800, 800, 800, 800};  // Adjust based on your sensors

// ============ SETUP ============
void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);       // HC-05 baud rate
  dfSerial.begin(9600);       // DFPlayer Mini baud rate
  
  // Initialize multiplexer pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_OUT, INPUT);
  
  // Initialize speaker pin
  pinMode(SPEAKER_PIN, OUTPUT);
  
  // Initialize I2C for MPU-6050
  Wire.begin();
  delay(100);
  
  // Initialize MPU-6050
  if (!mpu.begin(MPU_ADDR)) {
    Serial.println("MPU-6050 connection failed!");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("MPU-6050 initialized!");
  
  // Initialize DFPlayer Mini
  if (!dfPlayer.begin(dfSerial)) {
    Serial.println("DFPlayer Mini initialization failed!");
  } else {
    Serial.println("DFPlayer Mini initialized!");
    dfPlayer.setTimeOut(500);
    dfPlayer.volume(20);  // Volume: 0-30
  }
  
  delay(500);
  Serial.println("FSL Glove System Initialized!");
}

// ============ LOOP ============
void loop() {
  // Read flex sensors
  readFlexSensors();
  
  // Read MPU-6050
  readMPU6050();
  
  // Send data via Bluetooth
  sendBluetoothData();
  
  // Gesture recognition (optional)
  recognizeGesture();
  
  delay(100);  // 100ms sampling rate
}

// ============ READ FLEX SENSORS ============
void readFlexSensors() {
  for (int i = 0; i < 10; i++) {
    selectMuxChannel(i);
    delay(10);  // Give sensor time to stabilize
    flexValues[i] = analogRead(MUX_OUT);
  }
}

// ============ SELECT MUX CHANNEL ============
void selectMuxChannel(int channel) {
  digitalWrite(MUX_S0, channel & 0x01);
  digitalWrite(MUX_S1, (channel >> 1) & 0x01);
  digitalWrite(MUX_S2, (channel >> 2) & 0x01);
  digitalWrite(MUX_S3, (channel >> 3) & 0x01);
}

// ============ READ MPU-6050 ============
void readMPU6050() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);
  
  accelX = accel.acceleration.x;
  accelY = accel.acceleration.y;
  accelZ = accel.acceleration.z;
  
  gyroX = gyro.gyro.x;
  gyroY = gyro.gyro.y;
  gyroZ = gyro.gyro.z;
  
  tempC = temp.temperature;
}

// ============ SEND BLUETOOTH DATA ============
void sendBluetoothData() {
  // Format: FLEX1,FLEX2,...,FLEX10,ACCEL_X,ACCEL_Y,ACCEL_Z,GYRO_X,GYRO_Y,GYRO_Z
  
  String data = "";
  
  // Add flex sensor values
  for (int i = 0; i < 10; i++) {
    data += flexValues[i];
    if (i < 9) data += ",";
  }
  
  // Add accelerometer data
  data += "," + String(accelX, 2);
  data += "," + String(accelY, 2);
  data += "," + String(accelZ, 2);
  
  // Add gyro data
  data += "," + String(gyroX, 2);
  data += "," + String(gyroY, 2);
  data += "," + String(gyroZ, 2);
  
  // Send to Bluetooth
  btSerial.println(data);
  
  // Also print to Serial Monitor for debugging
  Serial.println(data);
}

// ============ GESTURE RECOGNITION ============
void recognizeGesture() {
  // Example: Recognize a "thumbs up" gesture
  // Thumb flex (Flex 1) is high, other fingers are low
  
  int thumbFlexed = flexValues[0] > (flexMin[0] + flexMax[0]) / 2;
  int indexFlexed = flexValues[1] > (flexMin[1] + flexMax[1]) / 2;
  int middleFlexed = flexValues[2] > (flexMin[2] + flexMax[2]) / 2;
  int ringFlexed = flexValues[3] > (flexMin[3] + flexMax[3]) / 2;
  int pinkyFlexed = flexValues[4] > (flexMin[4] + flexMax[4]) / 2;
  
  // Thumbs Up: Thumb extended, others flexed
  if (!thumbFlexed && indexFlexed && middleFlexed && ringFlexed && pinkyFlexed) {
    Serial.println("GESTURE: Thumbs Up!");
    playSound(1);  // Play sound 001
    btSerial.println("GESTURE:THUMBS_UP");
  }
  
  // Peace/Victory: Thumb, Index, Middle extended, others flexed
  if (!thumbFlexed && !indexFlexed && !middleFlexed && ringFlexed && pinkyFlexed) {
    Serial.println("GESTURE: Peace!");
    playSound(2);  // Play sound 002
    btSerial.println("GESTURE:PEACE");
  }
  
  // Fist: All fingers flexed
  if (thumbFlexed && indexFlexed && middleFlexed && ringFlexed && pinkyFlexed) {
    Serial.println("GESTURE: Fist!");
    playSound(3);  // Play sound 003
    btSerial.println("GESTURE:FIST");
  }
  
  // Open Hand: All fingers extended
  if (!thumbFlexed && !indexFlexed && !middleFlexed && !ringFlexed && !pinkyFlexed) {
    Serial.println("GESTURE: Open Hand!");
    playSound(4);  // Play sound 004
    btSerial.println("GESTURE:OPEN_HAND");
  }
}

// ============ PLAY SOUND ============
void playSound(int trackNumber) {
  if (dfPlayer.available()) {
    dfPlayer.play(trackNumber);  // Play track number (001.mp3, 002.mp3, etc.)
  }
}

// ============ CALIBRATION FUNCTION (Optional) ============
void calibrateFlexSensors() {
  Serial.println("Calibrating flex sensors...");
  Serial.println("Keep all fingers extended:");
  
  delay(3000);
  
  for (int i = 0; i < 10; i++) {
    selectMuxChannel(i);
    delay(10);
    flexMin[i] = analogRead(MUX_OUT);
  }
  
  Serial.println("Now make a fist:");
  delay(3000);
  
  for (int i = 0; i < 10; i++) {
    selectMuxChannel(i);
    delay(10);
    flexMax[i] = analogRead(MUX_OUT);
  }
  
  Serial.println("Calibration complete!");
  for (int i = 0; i < 10; i++) {
    Serial.print("Flex ");
    Serial.print(i + 1);
    Serial.print(" - Min: ");
    Serial.print(flexMin[i]);
    Serial.print(" Max: ");
    Serial.println(flexMax[i]);
  }
}