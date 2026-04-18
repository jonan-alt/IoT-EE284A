#include <QuickMedianLib.h>

const int adcPin = A2;
const int totalSamples = 5000;
const int filterSizeSmall = 50;
const int filterSizeLarge = 500;

float smaBufferSmall[filterSizeSmall];
float smaBufferLarge[filterSizeLarge];
int smaIndexSmall = 0, smaIndexLarge = 0;
float smaSumSmall = 0, smaSumLarge = 0;

float medBufferSmall[filterSizeSmall];
float medBufferLarge[filterSizeLarge];
int medIndexSmall = 0, medIndexLarge = 0;

float emaValueSmall = 0;
float emaValueLarge = 0;
const float alphaSmall = 2.0 / (filterSizeSmall + 1.0);
const float alphaLarge = 2.0 / (filterSizeLarge + 1.0);

struct FilterStats {
  int count = 0;
  float minVal = 999.0;
  float maxVal = -999.0;
  double meanVal = 0.0;
  double m2Val = 0.0;

  void updateStats(float newVal) {
    if (count == 0) {
      minVal = newVal;
      maxVal = newVal;
    } else {
      if (newVal < minVal) minVal = newVal;
      if (newVal > maxVal) maxVal = newVal;
    }
    count++;
    double delta1 = newVal - meanVal;
    meanVal += delta1 / count;
    double delta2 = newVal - meanVal;
    m2Val += delta1 * delta2;
  }

  float getStdDev() {
    if (count < 2) return 0.0;
    return sqrt(m2Val / (count - 1));
  }
};

FilterStats rawStats, smaSmallStats, smaLargeStats, medSmallStats, medLargeStats, emaSmallStats, emaLargeStats;

void printRow(const char* filterName, FilterStats& stats) {
  Serial.print(filterName);
  Serial.print("\t");
  Serial.print(stats.maxVal, 2);
  Serial.print("\t");
  Serial.print(stats.minVal, 2);
  Serial.print("\t");
  Serial.print(stats.meanVal, 2);
  Serial.print("\t");
  Serial.println(stats.getStdDev(), 4);
}

float getTemperature() {
  int milliVolts = analogReadMilliVolts(adcPin);
  return (milliVolts - 500.0) * 0.1;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  delay(50);

  float startingTemp = getTemperature();

  for (int i = 0; i < filterSizeLarge; i++) {
    if (i < filterSizeSmall) {
      smaBufferSmall[i] = startingTemp;
      smaSumSmall += startingTemp;
      medBufferSmall[i] = startingTemp;
    }
    smaBufferLarge[i] = startingTemp;
    smaSumLarge += startingTemp;
    medBufferLarge[i] = startingTemp;
  }
  emaValueSmall = startingTemp;
  emaValueLarge = startingTemp;

  Serial.println("Collecting 5000 readings...");
}

void loop() {
  static bool testFinished = false;

  if (!testFinished) {
    for (int i = 0; i < totalSamples; i++) {
      
      float currentTemp = getTemperature();
      rawStats.updateStats(currentTemp);

      smaSumSmall -= smaBufferSmall[smaIndexSmall];
      smaBufferSmall[smaIndexSmall] = currentTemp;
      smaSumSmall += currentTemp;
      smaIndexSmall = (smaIndexSmall + 1) % filterSizeSmall;
      smaSmallStats.updateStats(smaSumSmall / filterSizeSmall);

      smaSumLarge -= smaBufferLarge[smaIndexLarge];
      smaBufferLarge[smaIndexLarge] = currentTemp;
      smaSumLarge += currentTemp;
      smaIndexLarge = (smaIndexLarge + 1) % filterSizeLarge;
      smaLargeStats.updateStats(smaSumLarge / filterSizeLarge);

      medBufferSmall[medIndexSmall] = currentTemp;
      medIndexSmall = (medIndexSmall + 1) % filterSizeSmall;
      float tempArraySmall[filterSizeSmall];
      memcpy(tempArraySmall, medBufferSmall, sizeof(medBufferSmall));
      float medianSmall = QuickMedian<float>::GetMedian(tempArraySmall, filterSizeSmall);
      medSmallStats.updateStats(medianSmall);

      medBufferLarge[medIndexLarge] = currentTemp;
      medIndexLarge = (medIndexLarge + 1) % filterSizeLarge;
      float tempArrayLarge[filterSizeLarge];
      memcpy(tempArrayLarge, medBufferLarge, sizeof(medBufferLarge));
      float medianLarge = QuickMedian<float>::GetMedian(tempArrayLarge, filterSizeLarge);
      medLargeStats.updateStats(medianLarge);

      emaValueSmall = (alphaSmall * currentTemp) + ((1.0 - alphaSmall) * emaValueSmall);
      emaSmallStats.updateStats(emaValueSmall);

      emaValueLarge = (alphaLarge * currentTemp) + ((1.0 - alphaLarge) * emaValueLarge);
      emaLargeStats.updateStats(emaValueLarge);

      delay(1);
    }

    Serial.println("Method\t\tMax(C)\tMin(C)\tMean(C)\tStdDev");
    printRow("Raw Data  ", rawStats);
    printRow("SMA (N=50)", smaSmallStats);
    printRow("SMA (N=500)", smaLargeStats);
    printRow("Med (N=50)", medSmallStats);
    printRow("Med (N=500)", medLargeStats);
    printRow("EMA (N=50)", emaSmallStats);
    printRow("EMA (N=500)", emaLargeStats);
    
    testFinished = true;
  }
}