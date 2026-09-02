/* Display an embedded Vietnam flag image on the built-in 240x135 TFT. */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include "vietnam_flag_image.h"

// Explicit fallbacks support older ESP32 Arduino board packages.
#ifndef TFT_CS
#define TFT_CS 42
#endif
#ifndef TFT_DC
#define TFT_DC 40
#endif
#ifndef TFT_RST
#ifdef TFT_RESET
#define TFT_RST TFT_RESET
#else
#define TFT_RST 41
#endif
#endif
#ifndef TFT_I2C_POWER
#define TFT_I2C_POWER 7
#endif
#ifndef TFT_BACKLITE
#ifdef TFT_BACKLIGHT
#define TFT_BACKLITE TFT_BACKLIGHT
#else
#define TFT_BACKLITE 45
#endif
#endif

constexpr int8_t TFT_SCLK_PIN = 36;
constexpr int8_t TFT_MOSI_PIN = 35;

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  SPI.begin(TFT_SCLK_PIN, -1, TFT_MOSI_PIN, TFT_CS);
  tft.init(135, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRGBBitmap(0, 0, vietnamFlagImage,
                    VIETNAM_FLAG_WIDTH, VIETNAM_FLAG_HEIGHT);

  Serial.println("Vietnam flag displayed.");
}

void loop() {
  // The ST7789 keeps the image in display memory.
}
