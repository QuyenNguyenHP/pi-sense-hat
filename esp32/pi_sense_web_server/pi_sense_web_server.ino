#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS  5
#define TFT_RST 4
#define TFT_DC  2

// Wi-Fi network used by both the ESP32 and Raspberry Pi.
const char* WIFI_SSID = "NOKIA-C6D1";
const char* WIFI_PASSWORD = "FpbtePuB5t";

// Replace this address with the Raspberry Pi's IP address.
const char* PI_SENSOR_API =
    "http://192.168.18.50:8000/api/sensors";

WebServer server(80);
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

float temperature = 0.0;
float humidity = 0.0;
bool sensorDataValid = false;

unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL_MS = 3000;

void showTftStatus(const char* message, uint16_t color) {
  tft.fillRect(0, 130, tft.width(), 30, ST77XX_BLACK);
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.setCursor(5, 136);
  tft.print(message);
}

void showTftReadings() {
  // Clear only the changing part of the screen to reduce flicker.
  tft.fillRect(0, 25, tft.width(), 103, ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(8, 32);
  tft.print("TEMPERATURE");

  tft.setTextSize(3);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(8, 48);
  if (sensorDataValid) {
    tft.print(temperature, 1);
    tft.setTextSize(2);
    tft.print(" C");
  } else {
    tft.print("--");
  }

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(8, 83);
  tft.print("HUMIDITY");

  tft.setTextSize(3);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(8, 99);
  if (sensorDataValid) {
    tft.print(humidity, 1);
    tft.setTextSize(2);
    tft.print(" %");
  } else {
    tft.print("--");
  }
}

bool updateFromPi() {
  if (WiFi.status() != WL_CONNECTED) {
    sensorDataValid = false;
    showTftReadings();
    showTftStatus("Wi-Fi disconnected", ST77XX_RED);
    return false;
  }

  HTTPClient http;
  http.setTimeout(3000);
  http.begin(PI_SENSOR_API);

  int status = http.GET();

  if (status != HTTP_CODE_OK) {
    Serial.printf("Pi API error: %d\n", status);
    http.end();
    sensorDataValid = false;
    showTftReadings();
    showTftStatus("Pi API unavailable", ST77XX_RED);
    return false;
  }

  String json = http.getString();
  http.end();

  // The Pi response also contains orientation, motion data, and a 64-pixel
  // matrix. Filter those fields out to keep ESP32 memory usage small.
  JsonDocument filter;
  filter["temperature"] = true;
  filter["humidity"] = true;

  JsonDocument document;
  DeserializationError error = deserializeJson(
      document,
      json,
      DeserializationOption::Filter(filter));

  if (error) {
    Serial.print("JSON error: ");
    Serial.println(error.c_str());
    sensorDataValid = false;
    showTftReadings();
    showTftStatus("Invalid API JSON", ST77XX_RED);
    return false;
  }

  if (!document["temperature"].is<float>() ||
      !document["humidity"].is<float>()) {
    Serial.println("Pi API response is missing temperature or humidity");
    sensorDataValid = false;
    showTftReadings();
    showTftStatus("Sensor data missing", ST77XX_RED);
    return false;
  }

  temperature = document["temperature"].as<float>();
  humidity = document["humidity"].as<float>();
  sensorDataValid = true;

  Serial.printf(
      "Temperature: %.1f C, Humidity: %.1f %%\n",
      temperature,
      humidity);

  showTftReadings();
  String ipStatus = "IP: " + WiFi.localIP().toString();
  showTftStatus(ipStatus.c_str(), ST77XX_GREEN);

  return true;
}

String createPage() {
  String tempText = sensorDataValid ? String(temperature, 1) : "--";
  String humidityText = sensorDataValid ? String(humidity, 1) : "--";

  String html = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="3">
  <title>Pi Sense HAT</title>
  <style>
    body {
      margin: 0;
      min-height: 100vh;
      display: grid;
      place-items: center;
      font-family: Arial, sans-serif;
      color: white;
      background: #0f172a;
    }
    .panel {
      width: min(85%, 420px);
      padding: 28px;
      text-align: center;
      border-radius: 20px;
      background: #1e293b;
      box-shadow: 0 12px 35px #0008;
    }
    .reading {
      margin: 18px 0;
      padding: 20px;
      border-radius: 14px;
      background: #0f172a;
    }
    .label {
      color: #94a3b8;
      font-size: 18px;
    }
    .value {
      margin-top: 8px;
      color: #38bdf8;
      font-size: 42px;
      font-weight: bold;
    }
    .status {
      color: #94a3b8;
      font-size: 14px;
    }
  </style>
</head>
<body>
  <main class="panel">
    <h1>Pi Sense HAT</h1>
    <section class="reading">
      <div class="label">Temperature</div>
      <div class="value">)HTML";

  html += tempText;
  html += R"HTML(&deg;C</div>
    </section>
    <section class="reading">
      <div class="label">Humidity</div>
      <div class="value">)HTML";

  html += humidityText;
  html += R"HTML(%</div>
    </section>
    <div class="status">Data supplied by Raspberry Pi</div>
  </main>
</body>
</html>
)HTML";

  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", createPage());
}

void handleApi() {
  JsonDocument response;
  response["connected"] = sensorDataValid;

  if (sensorDataValid) {
    response["temperature"] = temperature;
    response["humidity"] = humidity;
  }

  String output;
  serializeJson(response, output);
  server.send(200, "application/json", output);
}

void setup() {
  Serial.begin(115200);

  // Initialize the ST7735 using the same settings as TFT_display.ino.
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(8, 5);
  tft.print("PI SENSE");
  showTftReadings();
  showTftStatus("Connecting Wi-Fi...", ST77XX_YELLOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  String ipStatus = "IP: " + WiFi.localIP().toString();
  showTftStatus(ipStatus.c_str(), ST77XX_GREEN);

  updateFromPi();

  server.on("/", handleRoot);
  server.on("/api/data", handleApi);
  server.begin();

  Serial.println("ESP32 web server started");
}

void loop() {
  server.handleClient();

  if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
    lastUpdate = millis();
    updateFromPi();
  }
}
