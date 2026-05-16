#include <Wire.h>
#include <Adafruit_INA219.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID    "12345678-1234-1234-1234-1234567890ac"

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

void setup() {
  Serial.begin(115200);

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

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
  if (connected) {

    
    float busvoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    float battery_voltage = busvoltage + (ina219.getShuntVoltage_mV() / 1000); 

    char buffer[64];
    
    snprintf(buffer, sizeof(buffer), "%.4f,%.4f", battery_voltage, current_mA / 1000.0);
    
    pCharacteristic->setValue(buffer);
    pCharacteristic->notify();
    Serial.println(buffer);
    delay(20); 
  } 
}