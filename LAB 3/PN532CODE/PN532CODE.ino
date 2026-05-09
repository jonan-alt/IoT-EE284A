#include <Wire.h> //The package for I2C
#include <Adafruit_PN532.h>

// -----------------------------
// PN532
// -----------------------------
// We do not connect these pins but they must be input into the PN532 object
#define PN532_IRQ   -1
#define PN532_RESET -1   

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);

const uint32_t POLL_TIMEOUT_MS = 250;
const uint32_t INTER_TRIAL_DELAY_MS = 80;
const int N_TRIALS = 30;

bool onePoll(uint32_t &ttf_ms) {

  uint8_t uid[7];
  uint8_t uidLength = 0;

  uint32_t t0 = millis();

  bool ok = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, POLL_TIMEOUT_MS);
  
  ttf_ms = millis() - t0;
  
  return ok;
}

void printUID(const uint8_t *uid, uint8_t uidLength) {
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) Serial.print("0");
    Serial.print(uid[i], HEX);
    if (i + 1 < uidLength) Serial.print(" ");
  }
}

bool readPageSpan(uint8_t startPage, uint8_t endPage) {
  uint8_t buf[4];

  if (startPage > endPage || endPage > 0x2C) {
    Serial.println("OUT OF RANGE");
    return false;
  }

  for (uint8_t page = startPage; page <= endPage; page++) {
    if (page < 0x10) Serial.print("0");
    Serial.print(page, HEX);
    Serial.print(": ");

    if (nfc.ntag2xx_ReadPage(page, buf)) {
      for (uint8_t i = 0; i < 4; i++) {
        if (buf[i] < 0x10) Serial.print("0");
        Serial.print(buf[i], HEX);
        if (i < 3) Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println("READ FAIL");
      return false;
    }
  }

  return true;
}

void clearNTAG213UserArea() {
  uint8_t blank[4] = {0x00, 0x00, 0x00, 0x00};
  for (uint8_t page = 0x04; page <= 0x27; page++) {
    bool ok = nfc.ntag2xx_WritePage(page, blank);
    if (page < 0x10) Serial.print("0");
    Serial.print(page, HEX);
    Serial.print(": ");
    
    if (ok) Serial.println("CLEARED");
    else Serial.println("WRITE FAIL");
  }
}

void setup() {

  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  nfc.begin(); 

  uint32_t versiondata = nfc.getFirmwareVersion();  
  if (!versiondata) {
    Serial.println("ERROR: PN532 not found. Check I2C mode + wiring (and IRQ pin if using Adafruit lib I2C).");
    while (1) delay(10);
  }

  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print(".");
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  
  nfc.SAMConfig();

  Serial.println("Waiting for tag...");
  uint8_t uid[7];
  uint8_t uidLength;

  while (!nfc.readPassiveTargetID(
      PN532_MIFARE_ISO14443A,
      uid,
      &uidLength
  )) {

    delay(100);
  }

  Serial.println("Tag detected!");


  clearNTAG213UserArea();

  uint8_t data[4] = {0xAB, 0xCD, 0xEF, 0x01};
  nfc.ntag2xx_WritePage(0x0A, data);  // page 10 decimal
  nfc.ntag2xx_WritePage(0x18, data);  // page 24 decimal

}

void loop() {

  /*int successes = 0;
  uint32_t totalSuccessTTF = 0;
  
  Serial.println("Running 30 polls...");
  Serial.println();

  for (int i = 0; i < N_TRIALS; i++) {

    uint32_t ttf = 0;
    bool ok = onePoll(ttf);

    Serial.print("Trial ");
    Serial.print(i + 1);
    Serial.print(": ");

    if (ok) {
      successes++;

      totalSuccessTTF += ttf;

      Serial.print("SUCCESS | ");
      Serial.print(ttf);
      Serial.println(" ms");
    } else {

      Serial.print("FAIL | ");
      Serial.print(ttf);
      Serial.println(" ms");
    }
    delay(INTER_TRIAL_DELAY_MS);
  } 

  float successRate = 100.0 * successes / N_TRIALS;

  float avgTTF = 0;

  if (successes > 0) {
    avgTTF = (float) totalSuccessTTF / successes;
  }

  Serial.println();
  Serial.println("Results");

  Serial.print("Successes: ");
  Serial.print(successes);
  Serial.print(" / ");
  Serial.println(N_TRIALS);

  Serial.print("Success Rate: ");
  Serial.print(successRate);
  Serial.println("%");

  Serial.print("Average Successful TTF: ");
  Serial.print(avgTTF);
  Serial.println(" ms");

  Serial.println();
  
  delay(5000);
  */
  uint8_t uid[7];  
  uint8_t uidLength = 0;

  bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);

  if (success) {
    Serial.print("UID length: ");
    Serial.print(uidLength);
    Serial.print(" bytes | UID: ");
    printUID(uid, uidLength);
    Serial.println();
    Serial.println("Pages:");
    readPageSpan(0x00, 0x27); 
    Serial.println();


    delay(500);
  }
  
}