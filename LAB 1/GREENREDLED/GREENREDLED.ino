#include <Wire.h>
#include <Adafruit_BME680.h>

// Sensor Pins
const int TMP36_PIN = A2;
const int BTN_PIN = 38;

// LED Pins – Two for normal, two for alert
const int LED1_NORMAL = 15;  // Green (ON when diff is LOW)
const int LED2_NORMAL = 32;  // Green (ON when diff is LOW)
const int LED1_ALERT  = 14;  // Red (ON when diff is HIGH)
const int LED2_ALERT  = 13;  // Built-in Red (ON when diff is HIGH)

const float threshold = 3.0;
const float alpha = 2.0 / (50 + 1.0);

Adafruit_BME680 bme(&Wire);

float emaTemp36 = 0;
float emaBme = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin(22, 20);

  pinMode(BTN_PIN, INPUT);

  // Configure ALL LED pins
  pinMode(LED1_NORMAL, OUTPUT);
  pinMode(LED2_NORMAL, OUTPUT);
  pinMode(LED1_ALERT, OUTPUT);
  pinMode(LED2_ALERT, OUTPUT);

  // Start with all LEDs OFF
  digitalWrite(LED1_NORMAL, LOW);
  digitalWrite(LED2_NORMAL, LOW);
  digitalWrite(LED1_ALERT, LOW);
  digitalWrite(LED2_ALERT, LOW);

  analogReadResolution(12);

  if (!bme.begin()) {
    Serial.println("BME680 not found!");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  Serial.println("Press button to start.");

  while (digitalRead(BTN_PIN) == HIGH) {
    delay(10);
  }
  delay(50);

  Serial.println("System starting...");

  float voltage = analogReadMilliVolts(TMP36_PIN);
  emaTemp36 = (voltage - 500.0) / 10.0;

  bme.performReading();
  emaBme = bme.temperature;
}

void loop() {
  // Read and filter TMP36
  float voltage = analogReadMilliVolts(TMP36_PIN);
  float temp36Temp = (voltage - 500.0) / 10.0;
  emaTemp36 = (alpha * temp36Temp) + ((1.0 - alpha) * emaTemp36);

  // Read and filter BME680
  if (bme.performReading()) {
    float rawBme = bme.temperature;
    emaBme = (alpha * rawBme) + ((1.0 - alpha) * emaBme);
  }

  float difference = abs(emaTemp36 - emaBme);

  // Control ALL LEDs based on difference
  if (difference > threshold) {
    // ALERT STATE – Two LEDs ON
    digitalWrite(LED1_NORMAL, LOW);
    digitalWrite(LED2_NORMAL, LOW);
    digitalWrite(LED1_ALERT, HIGH);
    digitalWrite(LED2_ALERT, HIGH);
  } else {
    // NORMAL STATE – Two LEDs ON
    digitalWrite(LED1_NORMAL, HIGH);
    digitalWrite(LED2_NORMAL, HIGH);
    digitalWrite(LED1_ALERT, LOW);
    digitalWrite(LED2_ALERT, LOW);
  }

  Serial.print("TMP36: "); Serial.print(emaTemp36);
  Serial.print(" C | BME: "); Serial.print(emaBme);
  Serial.print(" C | Diff: "); Serial.print(difference);
  Serial.print(" C | LEDs: ");
  Serial.println((difference > threshold) ? "ALERT (2 LEDs)" : "NORMAL (2 LEDs)");

  delay(50);
}
