/*
  ESP32 force-sensitive resistor (FSR) example

  Voltage-divider wiring:
    3.3V --- FSR ---+--- GPIO 35
                    |
                   10 kOhm
                    |
                   GND
*/

const uint8_t FORCE_SENSOR_PIN = 35;
// Update at 20 Hz so changes in force are shown quickly.
const unsigned long SAMPLE_INTERVAL_MS = 500;

// Replace these values with ADC readings measured using known 20 g and 20 kg
// loads. The defaults span most of the ESP32's 12-bit ADC range.
const int ADC_AT_20_G = 50;
const int ADC_AT_20_KG = 4000;

const float MIN_MASS_KG = 0.020f;
const float MAX_MASS_KG = 20.0f;
const float GRAVITY_M_S2 = 9.80665f;

float readingToNewtons(int reading) {
  if (reading < ADC_AT_20_G) {
    return 0.0f;
  }

  const int limitedReading = constrain(reading, ADC_AT_20_G, ADC_AT_20_KG);
  const float fraction =
      float(limitedReading - ADC_AT_20_G) /
      float(ADC_AT_20_KG - ADC_AT_20_G);
  const float massKg = MIN_MASS_KG + fraction * (MAX_MASS_KG - MIN_MASS_KG);

  return massKg * GRAVITY_M_S2;
}

void setup() {
  Serial.begin(115200);

  // Use the ESP32's 12-bit ADC range: 0 to 4095.
  analogReadResolution(12);
  analogSetPinAttenuation(FORCE_SENSOR_PIN, ADC_11db);

  Serial.println("ESP32 force sensor started");
}

void loop() {
  const int forceValue = analogRead(FORCE_SENSOR_PIN);

  if (ADC_AT_20_G >= ADC_AT_20_KG) {
    Serial.println("Calibration error: ADC_AT_20_G must be below ADC_AT_20_KG");
    delay(SAMPLE_INTERVAL_MS);
    return;
  }

  const float forceNewtons = readingToNewtons(forceValue);
  Serial.print("ADC = ");
  Serial.print(forceValue);
  Serial.print(" | Force = ");

  if (forceNewtons == 0.0f) {
    Serial.println("0.00 N (below 20 g range)");
  } else {
    Serial.print(forceNewtons, 2);
    Serial.println(" N");
  }

  delay(SAMPLE_INTERVAL_MS);
}
