#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

const char* AP_NAME = "ESP32-I2C-Lesson";
const char* AP_PASSWORD = "12345678";  // Minimum eight characters.

const byte DNS_PORT = 53;
const char* IMAGE_PATH = "/i2c.png";

DNSServer dnsServer;
WebServer server(80);

const char PAGE_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="vi">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>I2C là gì?</title>
  <style>
    * { box-sizing: border-box; }
    html, body {
      margin: 0;
      min-height: 100%;
      background: #101318;
    }
    body {
      display: flex;
      justify-content: center;
      align-items: flex-start;
    }
    img {
      display: block;
      width: 100%;
      max-width: 1080px;
      height: auto;
    }
    .error {
      margin: 2rem;
      color: white;
      font: 18px sans-serif;
    }
  </style>
</head>
<body>
  <img src="/i2c.png" alt="Tập 6: I2C là gì?">
</body>
</html>
)HTML";

void sendPage() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", PAGE_HTML);
}

void sendImage() {
  File image = LittleFS.open(IMAGE_PATH, "r");

  if (!image) {
    server.send(
        500,
        "text/plain; charset=utf-8",
        "Image missing. Upload the LittleFS data folder before starting.");
    return;
  }

  server.sendHeader("Cache-Control", "public, max-age=3600");
  server.streamFile(image, "image/png");
  image.close();
}

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin(false)) {
    Serial.println("LittleFS mount failed. Upload the data folder first.");
  }

  WiFi.mode(WIFI_AP);

  if (!WiFi.softAP(AP_NAME, AP_PASSWORD)) {
    Serial.println("Unable to start the Wi-Fi access point.");
    return;
  }

  IPAddress apIP = WiFi.softAPIP();

  // Resolve every hostname to the ESP32 to behave like a captive portal.
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, sendPage);
  server.on("/i2c.png", HTTP_GET, sendImage);

  // Common Android, Apple, and Windows captive-portal checks.
  server.on("/generate_204", HTTP_ANY, sendPage);
  server.on("/gen_204", HTTP_ANY, sendPage);
  server.on("/hotspot-detect.html", HTTP_ANY, sendPage);
  server.on("/library/test/success.html", HTTP_ANY, sendPage);
  server.on("/connecttest.txt", HTTP_ANY, sendPage);
  server.on("/ncsi.txt", HTTP_ANY, sendPage);
  server.onNotFound(sendPage);

  server.begin();

  Serial.println();
  Serial.println("ESP32 image access point started");
  Serial.print("Wi-Fi name: ");
  Serial.println(AP_NAME);
  Serial.print("Open: http://");
  Serial.println(apIP);
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  delay(2);
}
