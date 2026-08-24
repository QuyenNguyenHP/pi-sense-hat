# Display a photo on the ESP32 ST7735

Open `display_photo/display_photo.ino` in Arduino IDE, select the ESP32 board and port, then
upload it. The sketch displays `i2c.png` on the 160×128 ST7735 screen using the
same pins as the other examples in this repository:

| ST7735 | ESP32 |
| --- | --- |
| CS | GPIO 5 |
| RST | GPIO 4 |
| DC | GPIO 2 |
| SCK | GPIO 18 (default SPI) |
| MOSI | GPIO 23 (default SPI) |

Install the **Adafruit GFX Library** and **Adafruit ST7735 and ST7789 Library**
through Arduino IDE's Library Manager first.

The generated `display_photo/i2c_image.h` embeds a 128×128 RGB565 version of the photo in
flash. If `i2c.png` changes, regenerate the header with Python 3 (no additional
packages are required):

```bash
python3 generate_image_header.py
```
