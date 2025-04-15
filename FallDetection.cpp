#define BLYNK_TEMPLATE_ID "TMPL3YVP6AVqC"
#define BLYNK_TEMPLATE_NAME "Fall Detection"
#define BLYNK_AUTH_TOKEN "pZRAELszOy5jRLLMAQiwQPauKG53smq2"

#include <Wire.h>
#include <math.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Dhanush";
char pass[] = "zorozoro";

#define ADXL345_ADDR 0x53
#define BUZZER_PIN 4

#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

String latitude = "12.973150";
String longitude = "79.165500";

float accX, accY, accZ;
float pitch, roll;
bool isOnGround = false;

const int numReadings = 5;
float accXArray[numReadings], accYArray[numReadings], accZArray[numReadings];
int readIndex = 0;
float accXAvg, accYAvg, accZAvg;

HardwareSerial gpsSerial(2);

// Buzzer state
bool beepStarted = false;
unsigned long beepStartTime = 0;
int beepCount = 0;
bool buzzerOn = false;
unsigned long lastBeepTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(0x2D);
  Wire.write(0x08);
  Wire.endTransmission();

  for (int i = 0; i < numReadings; i++) {
    accXArray[i] = 0;
    accYArray[i] = 0;
    accZArray[i] = 0;
  }

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("System Initialized");
}

void loop() {
  Blynk.run();

  readAccelerometer();
  calculateOrientation();
  applySmoothing();

  bool newGroundState = checkIfOnGround();

  if (newGroundState && !isOnGround) {
    Serial.println("Fall detected");
    beepStarted = true;
    beepStartTime = millis();
    beepCount = 0;
    buzzerOn = false;
  }

  handleBeep();

  isOnGround = newGroundState;

  getGPSData();

  String status = isOnGround ? "On Ground" : "Not on Ground";
  Serial.println("Lat: " + latitude + " | Lon: " + longitude + " | Status: " + status);

  Blynk.virtualWrite(V0, latitude);
  Blynk.virtualWrite(V1, longitude);

  delay(100);
}

void handleBeep() {
  if (!beepStarted) return;

  if (millis() - beepStartTime >= 2000) { // Wait 2 sec before starting beeps
    if (beepCount < 5) {
      if (millis() - lastBeepTime >= 300) {
        buzzerOn = !buzzerOn;
        digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
        lastBeepTime = millis();
        if (!buzzerOn) beepCount++;
      }
    } else {
      beepStarted = false;
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}

void getGPSData() {
  String gpsData = "";
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    gpsData += c;
    if (c == '\n') break;
  }

  if (gpsData.startsWith("$GPGGA")) {
    int commaIndex[15], idx = 0;
    for (int i = 0; i < gpsData.length(); i++) {
      if (gpsData[i] == ',') commaIndex[idx++] = i;
    }

    if (idx > 5) {
      String lat = gpsData.substring(commaIndex[1] + 1, commaIndex[2]);
      String latDir = gpsData.substring(commaIndex[2] + 1, commaIndex[3]);
      String lon = gpsData.substring(commaIndex[3] + 1, commaIndex[4]);
      String lonDir = gpsData.substring(commaIndex[4] + 1, commaIndex[5]);

      if (lat.length() > 0 && lon.length() > 0) {
        latitude = convertToDecimal(lat, latDir);
        longitude = convertToDecimal(lon, lonDir);
      }
    }
  }
}

String convertToDecimal(String raw, String dir) {
  float value = raw.toFloat();
  int degrees = int(value / 100);
  float minutes = value - (degrees * 100);
  float decimal = degrees + (minutes / 60.0);

  if (dir == "S" || dir == "W") decimal *= -1;
  return String(decimal, 6);
}

void applySmoothing() {
  accXAvg -= accXArray[readIndex] / numReadings;
  accYAvg -= accYArray[readIndex] / numReadings;
  accZAvg -= accZArray[readIndex] / numReadings;

  accXArray[readIndex] = accX;
  accYArray[readIndex] = accY;
  accZArray[readIndex] = accZ;

  accXAvg += accX / numReadings;
  accYAvg += accY / numReadings;
  accZAvg += accZ / numReadings;

  readIndex = (readIndex + 1) % numReadings;
}

bool checkIfOnGround() {
  static bool groundState = false;
  static unsigned long lastChangeTime = 0;

  bool zCondition = accZAvg < -0.8;
  bool xyCondition = (abs(accXAvg) < 0.5) && (abs(accYAvg) < 0.5);
  bool angleCondition = (abs(pitch) < 15) && (abs(roll) < 15);

  if (!groundState) {
    if (zCondition && xyCondition && angleCondition) {
      if (millis() - lastChangeTime > 300) {
        groundState = true;
        lastChangeTime = millis();
      }
    } else {
      lastChangeTime = millis();
    }
  } else {
    if (!zCondition || !xyCondition || !angleCondition) {
      if (millis() - lastChangeTime > 300) {
        groundState = false;
        lastChangeTime = millis();
      }
    } else {
      lastChangeTime = millis();
    }
  }

  return groundState;
}

void readAccelerometer() {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345_ADDR, 6, true);

  int16_t x = Wire.read() | (Wire.read() << 8);
  int16_t y = Wire.read() | (Wire.read() << 8);
  int16_t z = Wire.read() | (Wire.read() << 8);

  accX = x * 0.0039;
  accY = y * 0.0039;
  accZ = z * 0.0039;
}

void calculateOrientation() {
  pitch = atan2(accY, sqrt(accX * accX + accZ * accZ)) * 180.0 / PI;
  roll = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI;
}