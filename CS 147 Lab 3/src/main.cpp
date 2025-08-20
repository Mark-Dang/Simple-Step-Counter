#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <SparkFunLSM6DSO.h>
#include <Wire.h>
#include <math.h>
#include <stdio.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


void setupBLEServer();
void calibrateLSMD6S0();

unsigned long steps = 0;
float lastX = 0;
float lastY = 0;
float xAccel = 0;
float yAccel = 0;
LSM6DSO myIMU;

class MyCallbacks: public BLECharacteristicCallbacks {
void onWrite(BLECharacteristic *pCharacteristic) {
  std::string value = pCharacteristic->getValue();
  if (value == "1") digitalWrite(33, HIGH); // Turn On
  else if (value == "0") digitalWrite(33, LOW); // Turn Off
  if (value.length() > 0) {
    Serial.println("*********");
    Serial.print("New value: ");
    for (int i = 0; i < value.length(); i++)
    Serial.print(value[i]);
    Serial.println();
    Serial.println("*********");
  }
}
void onRead(BLECharacteristic *pCharacteristic) {
  char str[20];
  sprintf(str, "%d", steps);
  pCharacteristic->setValue(str);
}
};


void setup() {
  Serial.begin(9600);
  setupBLEServer();
  pinMode(33, OUTPUT);

  Wire.begin();
  delay(10);
  if (myIMU.begin()) Serial.println("Ready");
  else {
    Serial.println("Could not connect to IMU");
    Serial.println("Freezing");
  }

  if (myIMU.initialize(BASIC_SETTINGS)) Serial.println("Loaded Settings.");
}


// When pulling from bluetooth, it will pull the last thing in the stream, including what we sent
void loop() {
  xAccel = myIMU.readFloatAccelX();
  yAccel = myIMU.readFloatAccelY();
  if (abs(lastX - xAccel) + abs(lastY - yAccel) >= 0.2) {
    steps += 1;
    Serial.println(steps);
    delay(650);
  }
  lastX = xAccel;
  lastY = yAccel;
  delay(20);
}


void setupBLEServer() {
  Serial.println("Starting BLE Server");
  BLEDevice::init("Group17ESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Hello World");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising -> setScanResponse(true);
  pAdvertising -> setMinPreferred(0x0);
  pAdvertising -> setMinPreferred(0x02);
  BLEDevice::startAdvertising();
  Serial.println("Good to go! Now we can read and write!");
}