#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "i2c_image.h"

#define TFT_CS 5
#define TFT_RST 4
#define TFT_DC 2

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);  // 160 x 128, landscape
  tft.fillScreen(ST77XX_BLACK);

  const int16_t x = (tft.width() - I2C_IMAGE_WIDTH) / 2;
  const int16_t y = (tft.height() - I2C_IMAGE_HEIGHT) / 2;
  tft.drawRGBBitmap(x, y, i2cImage, I2C_IMAGE_WIDTH, I2C_IMAGE_HEIGHT);

  Serial.println("Photo displayed.");
}

void loop() {
  // The image stays in the display's memory, so nothing is needed here.
}
