#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <WiFi.h>
#include <time.h>

#define TFT_CS  5
#define TFT_RST 4
#define TFT_DC  2

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

const char *WIFI_NAME = "dsing";
const char *WIFI_PASSWORD = "pioneer128";

// Singapore time is UTC+8 and has no daylight-saving adjustment.
const long GMT_OFFSET_SECONDS = 8 * 3600;
const int DAYLIGHT_OFFSET_SECONDS = 0;
const char *NTP_SERVER = "pool.ntp.org";

int lastDrawnSecond = -1;

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

void showMessage(const char *message, uint16_t color) {
  tft.fillRect(0, 47, tft.width(), 30, ST77XX_BLACK);
  printCentered(message, 57, 1, color);
}

void drawClock(const tm &timeInfo) {
  char timeText[9];
  strftime(timeText, sizeof(timeText), "%H:%M:%S", &timeInfo);

  // Clear only the clock area to avoid flashing the whole screen.
  tft.fillRect(0, 47, tft.width(), 30, ST77XX_BLACK);
  printCentered(timeText, 54, 2, ST77XX_CYAN);
}

bool connectToWiFi() {
  showMessage("Connecting to WiFi...", ST77XX_YELLOW);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  const uint32_t timeout = millis() + 20000UL;
  while (WiFi.status() != WL_CONNECTED && (int32_t)(millis() - timeout) < 0) {
    delay(250);
  }

  return WiFi.status() == WL_CONNECTED;
}

void setup() {
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_BLUE);
  printCentered("CLOCK", 15, 2, ST77XX_YELLOW);

  tft.drawFastHLine(10, 39, tft.width() - 20, ST77XX_BLUE);

  printCentered("ESP32 + ST7735", 95, 1, ST77XX_WHITE);

  if (!connectToWiFi()) {
    showMessage("Wi-Fi connection failed", ST77XX_RED);
    Serial.println("Wi-Fi connection failed. Check WIFI_NAME and WIFI_PASSWORD.");
    return;
  }

  showMessage("Getting network time...", ST77XX_YELLOW);
  configTime(GMT_OFFSET_SECONDS, DAYLIGHT_OFFSET_SECONDS, NTP_SERVER);

  tm timeInfo;
  if (!getLocalTime(&timeInfo, 15000)) {
    showMessage("NTP synchronization failed", ST77XX_RED);
    Serial.println("Unable to get time from the NTP server.");
    return;
  }

  Serial.print("Wi-Fi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  tm timeInfo;
  if (!getLocalTime(&timeInfo, 100)) {
    delay(500);
    return;
  }

  if (timeInfo.tm_sec != lastDrawnSecond) {
    lastDrawnSecond = timeInfo.tm_sec;
    drawClock(timeInfo);
  }

  delay(20);
}
