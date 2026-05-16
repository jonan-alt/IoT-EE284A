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
void runFanState(){
  Serial.print("State: ");
  Serial.println("%");  

  unsigned long stateStartTime = millis();

  while (millis() - stateStartTime < 600000) {
    float busvoltage = ina219.getBusVoltage_V();
    float shuntvoltage = ina219.getShuntVoltage_mV();
    float voltage = busvoltage + (shuntvoltage / 1000.0);
    float current = ina219.getCurrent_mA() / 1000.0;
    int cycle = 0;

    Serial.println(current);
    Serial.println(voltage);
    
    //Control Strategy Here

    if (voltage >= 3.50) {
      cycle = 100;
      ledcAttach(fanPin, freq, resolution); 
      ledcWrite(fanPin, 255); 
    } 
    else if (voltage < 3.50 && voltage > 3.30) {
      cycle = 50;
      ledcAttach(fanPin, freq, resolution);
      ledcWrite(fanPin, 127);
    } 
    else {
      cycle = 0;
      ledcDetach(fanPin); 
      pinMode(fanPin, OUTPUT);
      digitalWrite(fanPin, LOW);
    }
    
    if (connected) {
      float busvoltage = ina219.getBusVoltage_V();
      float shuntvoltage = ina219.getShuntVoltage_mV();
      float voltage = busvoltage + (shuntvoltage / 1000);
      float current = ina219.getCurrent_mA() / 1000.0;

      char buffer[64];
      snprintf(buffer, sizeof(buffer), "%.4f,%.4f,%.4f,%d", voltage, current, voltage * current, cycle);
      
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
  //Starts out at 0 spinning
  ledcWrite(fanPin, 0);

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

  if(connected){
    runFanState();
    while(1) {delay(1000);}
  }
}
  

  /*
  Note in my findings I found that running this at these conditions will still lower the system voltage
  when coming in contact w/ the shade, it requires complete shut off 
  */
  /*
  if (current_mA < 0) {
    Serial.println("Solar Surplus - Fan Duty Cycle Max");
    runFanState(100); 
  } 
  else if (current_mA > 0 && current_mA < 50) {
    Serial.println("Low Harvest - Fan Duty Cycle Lowered");
    runFanState(50);
  }
  else {
    Serial.println("No harvest - Fan OFF");
    runFanState(0);
  }
  */


