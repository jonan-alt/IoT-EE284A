#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);

  bme.setTemperatureOversampling(BME680_OS_1X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_0);
}

void loop() {
  analogSetPinAttenuation(ADC_PIN, ADC_6db); 
  delay(50);

  int rawADC = analogRead(ADC_PIN);

  float voltage = (rawADC/4095.0) * Vref;

  float tempsC = (voltage - 0.5)*100.0;

  String comparison = (tempsC > 23.0) ? "above" : "below";

  Serial.print("The raw ADC value is ");
  Serial.print(rawADC);
  Serial.print(", which converts to ");
  Serial.print(voltage, 3);
  Serial.print("V. The temperature is ");
  Serial.print(comparison);
  Serial.print(" 23C, in fact it is ");
  Serial.print(tempsC);
  Serial.println("C");

  delay(1000);
}