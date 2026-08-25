#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

const uint8_t TFT_CS = 5;
const uint8_t TFT_RST = 4;
const uint8_t TFT_DC = 2;
const uint8_t FORCE_SENSOR_PIN = 35;
// Update at 20 Hz so changes in force are shown quickly.
const unsigned long SAMPLE_INTERVAL_MS = 500;

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// Replace these defaults with ADC readings measured at known loads.
const int ADC_AT_20_G = 50;
const int ADC_AT_20_KG = 4000;
const float MIN_MASS_KG = 0.020f;
const float MAX_MASS_KG = 20.0f;
const float GRAVITY_M_S2 = 9.80665f;

void printCentered(const char *text, int16_t y, uint8_t textSize,
                   uint16_t color) {
  int16_t x1;
  int16_t y1;
  uint16_t textWidth;
  uint16_t textHeight;

  tft.setTextSize(textSize);
  tft.setTextColor(color);
  tft.getTextBounds(text, 0, y, &x1, &y1, &textWidth, &textHeight);
  tft.setCursor((tft.width() - textWidth) / 2, y);
  tft.print(text);
}

void drawScreenLayout() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_BLUE);
  printCentered("FORCE SENSOR", 10, 2, ST77XX_YELLOW);
  tft.drawFastHLine(8, 32, tft.width() - 16, ST77XX_BLUE);
  printCentered("Range: 0.20 - 196.13 N", 106, 1, ST77XX_WHITE);
}

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

void displayForce(int adcReading, float forceNewtons) {
  char forceText[20];
  char adcText[20];

  if (forceNewtons == 0.0f) {
    snprintf(forceText, sizeof(forceText), "< 0.20 N");
  } else {
    snprintf(forceText, sizeof(forceText), "%.2f N", forceNewtons);
  }
  snprintf(adcText, sizeof(adcText), "ADC: %d", adcReading);

  tft.fillRect(2, 38, tft.width() - 4, 63, ST77XX_BLACK);
  printCentered(forceText, 48, 3, ST77XX_GREEN);
  printCentered(adcText, 83, 1, ST77XX_CYAN);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(FORCE_SENSOR_PIN, ADC_11db);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  drawScreenLayout();

  Serial.println("ESP32 force sensor and TFT started");
}

void loop() {
  if (ADC_AT_20_G >= ADC_AT_20_KG) {
    Serial.println("Calibration error: ADC_AT_20_G must be below ADC_AT_20_KG");
    tft.fillRect(2, 38, tft.width() - 4, 63, ST77XX_BLACK);
    printCentered("CALIBRATION ERROR", 60, 1, ST77XX_RED);
    delay(SAMPLE_INTERVAL_MS);
    return;
  }

  const int adcReading = analogRead(FORCE_SENSOR_PIN);
  const float forceNewtons = readingToNewtons(adcReading);

  displayForce(adcReading, forceNewtons);

  Serial.print("ADC = ");
  Serial.print(adcReading);
  Serial.print(" | Force = ");
  Serial.print(forceNewtons, 2);
  Serial.println(" N");

  delay(SAMPLE_INTERVAL_MS);
}
