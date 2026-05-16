#include <Wire.h>
#include <Adafruit_INA219.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID    "12345678-1234-1234-1234-1234567890ac"

const int fanPin = 13;
const int freq = 5000;
const int resolution = 8; //8 bit calculations for duty cycesl

Adafruit_INA219 ina219;
BLECharacteristic *pCharacteristic;
bool connected = false;

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
       connected = true; 
    };
    void onDisconnect(BLEServer* pServer) { 
      connected = false;
      BLEDevice::startAdvertising(); 
    }
};

//Input integer like percentages: 100% --> 100 input, 50% --> 50 input
void runFanState(int cycle){
  Serial.print("State: ");
  Serial.println(cycle);
  Serial.println("%");

  int bitCycle = round((cycle/100.0)*255);

  ledcWrite(fanPin, bitCycle);

  unsigned long stateStartTime = millis();

  while (millis() - stateStartTime < 60000) {
    if (connected) {
      float busvoltage = ina219.getBusVoltage_V();
      float shuntvoltage = ina219.getShuntVoltage_mV();
      float voltage = busvoltage + (shuntvoltage / 1000);
      float current = ina219.getCurrent_mA() / 1000.0;

      char buffer[64];
      snprintf(buffer, sizeof(buffer), "%.4f,%.4f", voltage, current);
      
      pCharacteristic->setValue(buffer);
      pCharacteristic->notify();
      Serial.println(buffer);
    }
    delay(20); 
  }
}

void setup() {
  Serial.begin(115200);

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

  ledcAttach(fanPin, freq, resolution);

  BLEDevice::init("Balegyr86"); 
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHAR_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  /*runFanState(0);
  runFanState(100);
  runFanState(50);
  runFanState(25);
  */
}
