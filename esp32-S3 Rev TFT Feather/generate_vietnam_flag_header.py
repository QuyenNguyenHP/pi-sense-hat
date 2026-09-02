#!/usr/bin/env python3
"""Center-crop and convert the source image to a 240x135 RGB565 C header."""

from pathlib import Path
from PIL import Image, ImageOps


HERE = Path(__file__).resolve().parent
SOURCE = HERE / "vietnam-flag-png-xl.png"
OUTPUT = HERE / "display_vietnam_flag" / "vietnam_flag_image.h"
SIZE = (240, 135)


def rgb565(red: int, green: int, blue: int) -> int:
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)


with Image.open(SOURCE) as source:
    image = ImageOps.fit(source.convert("RGB"), SIZE, Image.Resampling.LANCZOS)

pixels = [rgb565(red, green, blue) for red, green, blue in image.getdata()]
lines = [
    "#pragma once",
    "",
    "#include <Arduino.h>",
    "",
    f"constexpr uint16_t VIETNAM_FLAG_WIDTH = {SIZE[0]};",
    f"constexpr uint16_t VIETNAM_FLAG_HEIGHT = {SIZE[1]};",
    "",
    "const uint16_t vietnamFlagImage[] PROGMEM = {",
]

for start in range(0, len(pixels), 12):
    values = ", ".join(f"0x{value:04X}" for value in pixels[start : start + 12])
    lines.append(f"  {values},")

lines.extend(["};", ""])
OUTPUT.parent.mkdir(parents=True, exist_ok=True)
OUTPUT.write_text("\n".join(lines), encoding="utf-8")
print(f"Wrote {OUTPUT} ({SIZE[0]}x{SIZE[1]}, {len(pixels) * 2} bytes)")
