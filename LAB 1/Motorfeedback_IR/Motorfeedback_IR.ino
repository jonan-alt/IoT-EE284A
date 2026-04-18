const int IR_SENSOR = 27;
const int MOTOR_PIN = 32;
const int PWM_BITS = 16;
const int PWM_HZ = 50;

uint32_t pulseLow = 1550;
uint32_t pulseMid = 1650;
uint32_t pulseHigh = 1750;

float desiredRPM = 60.0;
float actualRPM = 0.0;
float pwmCommand = 1600.0;
float gain = 1.0;

int previousBeam = HIGH;
uint32_t lastPulseTime = 0;

uint32_t usToDuty(uint32_t us) {
  const uint32_t maxVal = (1UL << PWM_BITS) - 1;
  return (us * maxVal) / 20000UL;
}

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR, INPUT_PULLUP);
  
  ledcAttach(MOTOR_PIN, PWM_HZ, PWM_BITS);
  delay(100);
  ledcWrite(MOTOR_PIN, usToDuty(pulseHigh));
}

void loop() {
  int beamNow = digitalRead(IR_SENSOR);
  uint32_t now = micros();

  if (previousBeam == HIGH && beamNow == LOW) {
    uint32_t revolutionTime = now - lastPulseTime;
    
    if (revolutionTime > 0 && lastPulseTime != 0) { 
      actualRPM = 60000000.0 / revolutionTime;
      
      float difference = desiredRPM - actualRPM;
      pwmCommand = pwmCommand + (gain * difference);

      if (pwmCommand > pulseHigh) pwmCommand = pulseHigh;
      if (pwmCommand < pulseLow) pwmCommand = pulseLow;

      ledcWrite(MOTOR_PIN, usToDuty((uint32_t)pwmCommand));
      
      Serial.print("Desired:");
      Serial.print(desiredRPM);
      Serial.print(", Current:");
      Serial.print(actualRPM);
      Serial.print(", PWM:");
      Serial.println(pwmCommand);
    }

    lastPulseTime = now;
  }

  previousBeam = beamNow;
}