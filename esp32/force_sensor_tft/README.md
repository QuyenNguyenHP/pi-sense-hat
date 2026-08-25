# ESP32 Force Sensor with ST7735 TFT

This separate sketch combines the force measurement from `force_sensor` with
the ST7735 configuration from `tft_display_test`. It displays force in newtons
and the raw ADC reading on the TFT and Serial Monitor.

## Pin connections

| Device | ESP32 pin |
| --- | --- |
| Force sensor voltage-divider output | GPIO 35 |
| TFT CS | GPIO 5 |
| TFT RST/RES | GPIO 4 |
| TFT DC/A0 | GPIO 2 |
| TFT SDA/MOSI | GPIO 23 |
| TFT SCK/CLK | GPIO 18 |

Power the force-sensor voltage divider and TFT logic from 3.3 V and connect all
grounds together.

## Required libraries

Install these through Arduino IDE Library Manager:

- Adafruit GFX Library
- Adafruit ST7735 and ST7789 Library

Open `force_sensor_tft.ino`, select the correct ESP32 board and port, and upload
the sketch. Calibrate `ADC_AT_20_G` and `ADC_AT_20_KG` using known loads before
relying on the displayed estimate.

The sensor and display update every 50 ms (20 times per second). The default ADC
calibration values are placeholders and will not produce accurate newton values
until they are replaced with measurements from your sensor and circuit.
