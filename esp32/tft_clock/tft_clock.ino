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
const unsigned long SAMPLE_INTERVAL_MS = 500;

void setup() {
  Serial.begin(115200);

  // Use the ESP32's 12-bit ADC range: 0 to 4095.
  analogReadResolution(12);
  analogSetPinAttenuation(FORCE_SENSOR_PIN, ADC_11db);

  Serial.println("ESP32 force sensor started");
}

void loop() {
  const int forceValue = analogRead(FORCE_SENSOR_PIN);

  Serial.print("Force sensor value = ");
  Serial.print(forceValue);

  if (forceValue < 50) {
    Serial.println(" -> no pressure");
  } else if (forceValue < 500) {
    Serial.println(" -> light touch");
  } else if (forceValue < 1500) {
    Serial.println(" -> light squeeze");
  } else if (forceValue < 2800) {
    Serial.println(" -> medium squeeze");
  } else {
    Serial.println(" -> big squeeze");
  }

  delay(SAMPLE_INTERVAL_MS);
}
