#include <Arduino.h>
#include "ESP_I2S.h"

#include "music.h"


#define I2S_NUM I2S_NUM_0
#define I2S_BCK_PIN GPIO_NUM_48
#define I2S_LRCK_PIN GPIO_NUM_38  
#define I2S_DOUT_PIN GPIO_NUM_47
 

#define EXAMPLE_SAMPLE_RATE (24000)

I2SClass i2s;
bool audio_ready = false;

bool setupI2S() {
  // V1 uses a PCM5101 DAC. It needs BCLK, LRCK and DOUT only; it has no
  // I2C control interface and does not require an ESP32-generated MCLK.
  i2s.setPins(I2S_BCK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN, -1, -1);
  if (!i2s.begin(I2S_MODE_STD, EXAMPLE_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("Failed to initialize I2S bus!");
    return false;
  }
  return true;
}

void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting V1 PCM5101 audio test...");

  if (!setupI2S()) {
    return;
  }
  Serial.println("PCM5101 I2S initialized");
  audio_ready = true;
}

void loop() {
  if (audio_ready) {
    i2s.write((uint8_t *)audio_data, AUDIO_SAMPLES * 2);
  } else {
    delay(1000);
  }
}
