#include <Arduino.h>
#include "ESP_I2S.h"

// ESP32-S3-Touch-LCD-1.85C V1 digital microphone connections.
#define MIC_WS_PIN   GPIO_NUM_2
#define MIC_BCLK_PIN GPIO_NUM_15
#define MIC_DATA_PIN GPIO_NUM_39

#define MIC_SAMPLE_RATE 16000
#define FRAMES_PER_READ 256

I2SClass micI2S;
int32_t samples[FRAMES_PER_READ * 2];  // Interleaved left/right samples.
bool micReady = false;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("Starting V1 digital microphone test...");

  micI2S.setPins(MIC_BCLK_PIN, MIC_WS_PIN, -1, MIC_DATA_PIN, -1);
  if (!micI2S.begin(I2S_MODE_STD,
                    MIC_SAMPLE_RATE,
                    I2S_DATA_BIT_WIDTH_32BIT,
                    I2S_SLOT_MODE_STEREO,
                    I2S_STD_SLOT_BOTH)) {
    Serial.printf("Microphone I2S initialization failed, error=%d\n",
                  micI2S.lastError());
    return;
  }

  micReady = true;
  Serial.println("Microphone ready. Speak or tap near the microphone.");
  Serial.println("The active channel's level and peak should increase.");
}

void loop() {
  if (!micReady) {
    delay(1000);
    return;
  }

  size_t bytesRead = micI2S.readBytes(
      reinterpret_cast<char *>(samples), sizeof(samples));
  size_t framesRead = bytesRead / (sizeof(int32_t) * 2);

  if (framesRead == 0) {
    Serial.printf("No microphone data, error=%d\n", micI2S.lastError());
    delay(500);
    return;
  }

  uint64_t leftTotal = 0;
  uint64_t rightTotal = 0;
  uint32_t leftPeak = 0;
  uint32_t rightPeak = 0;

  for (size_t frame = 0; frame < framesRead; ++frame) {
    int64_t left = samples[frame * 2];
    int64_t right = samples[frame * 2 + 1];
    uint32_t leftMagnitude = left < 0 ? -left : left;
    uint32_t rightMagnitude = right < 0 ? -right : right;

    leftTotal += leftMagnitude;
    rightTotal += rightMagnitude;
    if (leftMagnitude > leftPeak) leftPeak = leftMagnitude;
    if (rightMagnitude > rightPeak) rightPeak = rightMagnitude;
  }

  // Shift away unused low bits to keep the displayed numbers readable.
  Serial.printf("L level=%lu peak=%lu | R level=%lu peak=%lu\n",
                (unsigned long)((leftTotal / framesRead) >> 8),
                (unsigned long)(leftPeak >> 8),
                (unsigned long)((rightTotal / framesRead) >> 8),
                (unsigned long)(rightPeak >> 8));

  delay(100);
}
