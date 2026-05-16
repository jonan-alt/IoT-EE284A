#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID    "12345678-1234-1234-1234-1234567890ac"

BLECharacteristic *characteristic;

bool connected = false;
bool restartAdvertising = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) {
    connected = true;
    Serial.println("Client connected");
  }
  void onDisconnect(BLEServer*) {
    connected = false;
    restartAdvertising = true;
    Serial.println("Client disconnected");
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  BLEDevice::init("<Your Bluetooth Device Name>");
  BLEDevice::setPower(ESP_PWR_LVL_P9);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  BLEService *service = server->createService(SERVICE_UUID);

  characteristic = service->createCharacteristic(
      CHAR_UUID,
      BLECharacteristic::PROPERTY_NOTIFY
  );

  characteristic->addDescriptor(new BLE2902());
  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("BLE advertising started");
}

void loop() {

  if (restartAdvertising) {
    delay(200);
    BLEDevice::startAdvertising();
    Serial.println("Advertising restarted");
    restartAdvertising = false;
  }
  if (connected) {
    characteristic->setValue("hello world");
    characteristic->notify();
    delay(1000);
  }
}