# ESP32-S3 Reverse TFT Feather: TFT + 3 Buttons

This Arduino project demonstrates all of the built-in user-interface hardware on
the **Adafruit ESP32-S3 Reverse TFT Feather**:

- 1.14-inch, 240 x 135 ST7789 color TFT
- D0 / BOOT button on GPIO 0
- D1 button on GPIO 1
- D2 button on GPIO 2

No external wiring is required. D0 is active-low; D1 and D2 are active-high.
The sketch handles this difference and debounces all three buttons.

## Demo controls

| Button | Function |
| --- | --- |
| D0 | Previous screen |
| D1 | Screen action: increment counter or start/pause stopwatch |
| D2 | Next screen |

The three screens are a counter, a stopwatch, and a live button-state test.

## Arduino IDE setup

1. In Boards Manager, install/update **esp32 by Espressif Systems**.
2. In Library Manager, install **Adafruit GFX Library** and
   **Adafruit ST7735 and ST7789 Library** (accept their dependencies).
3. Open `reverse_tft_three_button_demo.ino`.
4. Select **Tools > Board > ESP32 Arduino > Adafruit Feather ESP32-S3 Reverse TFT**.
5. Select the board's USB port and upload.
6. If the new program does not start immediately after upload, press Reset once.

The exact board selection is important because it supplies the built-in TFT pin
definitions (`TFT_CS`, `TFT_DC`, `TFT_RST`, power, backlight, and SPI pins).
The sketch also contains the Reverse TFT's explicit pin values as fallbacks for
older ESP32 board packages that do not provide those names.

If upload mode is needed, hold D0/BOOT, press and release Reset, then release
D0/BOOT. Select the newly appearing port and upload again.

## Vietnam flag image sketch

Open `display_vietnam_flag/display_vietnam_flag.ino` to display the included
Vietnam flag source image full-screen. The generated RGB565 image is embedded
in flash, so no SD card or runtime image-decoding library is needed.

To regenerate the header after replacing the source image, install Pillow and
run `python3 generate_vietnam_flag_header.py` from this directory.

## Hardware notes

The project targets the **Reverse TFT** model. The similarly named ESP32-S3 TFT
Feather has its screen on the other side and does not have the same three-button
layout.
