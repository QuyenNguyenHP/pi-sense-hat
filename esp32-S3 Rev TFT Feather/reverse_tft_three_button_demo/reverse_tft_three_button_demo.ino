/*
  Adafruit ESP32-S3 Reverse TFT Feather
  Built-in TFT + three-button demonstration

  Controls:
    D0: previous screen (active LOW)
    D1: perform the action shown on screen (active HIGH)
    D2: next screen (active HIGH)
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#if !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVTFT)
#warning "Select 'Adafruit Feather ESP32-S3 Reverse TFT' in Tools > Board."
#endif

// The current Espressif board package defines these names when the Reverse TFT
// board is selected.  Keep explicit fallbacks so the sketch also compiles with
// older board packages or the generic ESP32-S3 Dev Module selection.
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

constexpr uint8_t BUTTON_D0 = 0;
constexpr uint8_t BUTTON_D1 = 1;
constexpr uint8_t BUTTON_D2 = 2;
constexpr uint16_t DEBOUNCE_MS = 30;
constexpr uint8_t SCREEN_COUNT = 3;

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

struct Button {
  uint8_t pin;
  bool activeHigh;
  bool stablePressed;
  bool lastRawPressed;
  uint32_t changedAt;
};

Button buttons[] = {
    {BUTTON_D0, false, false, false, 0},
    {BUTTON_D1, true, false, false, 0},
    {BUTTON_D2, true, false, false, 0},
};

uint8_t currentScreen = 0;
uint32_t counter = 0;
bool timerRunning = false;
uint32_t timerStartedAt = 0;
uint32_t timerAccumulatedMs = 0;
uint32_t lastTimerDraw = 0;

void centerText(const char *text, int16_t y, uint8_t size, uint16_t color) {
  int16_t x1, y1;
  uint16_t width, height;
  tft.setTextSize(size);
  tft.setTextColor(color);
  tft.getTextBounds(text, 0, y, &x1, &y1, &width, &height);
  tft.setCursor((tft.width() - width) / 2, y);
  tft.print(text);
}

void drawHeader(const char *title, uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0, 0, tft.width(), 25, color);
  centerText(title, 6, 2, ST77XX_BLACK);
}

void drawFooter(const char *action) {
  tft.drawFastHLine(0, 111, tft.width(), ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(5, 119);
  tft.print("D0 <");
  tft.setCursor(95, 119);
  tft.print("D1 ");
  tft.print(action);
  tft.setCursor(202, 119);
  tft.print("> D2");
}

void drawCounterValue() {
  char value[16];
  snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(counter));
  tft.fillRect(0, 38, tft.width(), 55, ST77XX_BLACK);
  centerText(value, 48, 4, ST77XX_GREEN);
}

uint32_t elapsedTimerMs() {
  return timerAccumulatedMs + (timerRunning ? millis() - timerStartedAt : 0);
}

void drawTimerValue() {
  const uint32_t elapsed = elapsedTimerMs();
  const uint32_t minutes = elapsed / 60000;
  const uint32_t seconds = (elapsed / 1000) % 60;
  const uint32_t tenths = (elapsed / 100) % 10;
  char value[20];
  snprintf(value, sizeof(value), "%02lu:%02lu.%lu",
           static_cast<unsigned long>(minutes),
           static_cast<unsigned long>(seconds),
           static_cast<unsigned long>(tenths));
  tft.fillRect(0, 38, tft.width(), 55, ST77XX_BLACK);
  centerText(value, 52, 3, ST77XX_CYAN);
}

void drawButtonMonitor() {
  tft.fillRect(0, 34, tft.width(), 70, ST77XX_BLACK);
  const char *labels[] = {"D0", "D1", "D2"};
  for (uint8_t i = 0; i < 3; ++i) {
    const int16_t x = 18 + i * 76;
    const uint16_t color = buttons[i].stablePressed ? ST77XX_GREEN : ST77XX_BLUE;
    tft.fillRoundRect(x, 43, 52, 43, 7, color);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(x + 14, 57);
    tft.print(labels[i]);
  }
  centerText("Press all three buttons", 94, 1, ST77XX_YELLOW);
}

void drawScreen() {
  switch (currentScreen) {
    case 0:
      drawHeader("COUNTER", ST77XX_YELLOW);
      drawCounterValue();
      drawFooter("+1");
      break;
    case 1:
      drawHeader("STOPWATCH", ST77XX_CYAN);
      drawTimerValue();
      drawFooter(timerRunning ? "PAUSE" : "START");
      break;
    default:
      drawHeader("BUTTON TEST", ST77XX_MAGENTA);
      drawButtonMonitor();
      drawFooter("CLEAR");
      break;
  }
}

bool updateButton(Button &button) {
  const bool rawPressed = digitalRead(button.pin) == (button.activeHigh ? HIGH : LOW);
  const uint32_t now = millis();

  if (rawPressed != button.lastRawPressed) {
    button.lastRawPressed = rawPressed;
    button.changedAt = now;
  }

  if (now - button.changedAt >= DEBOUNCE_MS && rawPressed != button.stablePressed) {
    button.stablePressed = rawPressed;
    return button.stablePressed;  // Report only a new press, not a release.
  }
  return false;
}

void handleAction() {
  if (currentScreen == 0) {
    ++counter;
    drawCounterValue();
  } else if (currentScreen == 1) {
    if (timerRunning) {
      timerAccumulatedMs += millis() - timerStartedAt;
    } else {
      timerStartedAt = millis();
    }
    timerRunning = !timerRunning;
    drawScreen();
  } else {
    // On this page D1 still lights while held; its press redraws the monitor.
    drawButtonMonitor();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_D0, INPUT_PULLUP);  // Pressed = LOW.
  pinMode(BUTTON_D1, INPUT_PULLDOWN); // Pressed = HIGH.
  pinMode(BUTTON_D2, INPUT_PULLDOWN); // Pressed = HIGH.

  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // Select the display's actual SPI pins even if a generic board is selected.
  SPI.begin(TFT_SCLK_PIN, -1, TFT_MOSI_PIN, TFT_CS);
  tft.init(135, 240);
  tft.setRotation(3);
  tft.setTextWrap(false);
  drawScreen();

  Serial.println("ESP32-S3 Reverse TFT three-button demo ready");
}

void loop() {
  const bool d0Pressed = updateButton(buttons[0]);
  const bool d1Pressed = updateButton(buttons[1]);
  const bool d2Pressed = updateButton(buttons[2]);

  if (currentScreen == 2) {
    // Show held/released state as well as debounced presses.
    static bool oldStates[3] = {};
    bool changed = false;
    for (uint8_t i = 0; i < 3; ++i) {
      if (oldStates[i] != buttons[i].stablePressed) {
        oldStates[i] = buttons[i].stablePressed;
        changed = true;
      }
    }
    if (changed) drawButtonMonitor();
  }

  if (d0Pressed) {
    currentScreen = (currentScreen + SCREEN_COUNT - 1) % SCREEN_COUNT;
    drawScreen();
    Serial.println("D0: previous screen");
  } else if (d2Pressed) {
    currentScreen = (currentScreen + 1) % SCREEN_COUNT;
    drawScreen();
    Serial.println("D2: next screen");
  } else if (d1Pressed) {
    handleAction();
    Serial.println("D1: action");
  }

  if (currentScreen == 1 && timerRunning && millis() - lastTimerDraw >= 100) {
    lastTimerDraw = millis();
    drawTimerValue();
  }
}
