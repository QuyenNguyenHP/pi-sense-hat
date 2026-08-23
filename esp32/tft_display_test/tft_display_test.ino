#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS  5
#define TFT_RST 4
#define TFT_DC  2

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.println("ESP32");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 50);
  tft.println("TFT OK");
}

void loop() {
}
