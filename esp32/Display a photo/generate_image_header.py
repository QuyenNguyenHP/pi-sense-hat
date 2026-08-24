#!/usr/bin/env python3
"""Convert i2c.png to the RGB565 header used by display_photo.ino."""

from pathlib import Path
import struct
import zlib


HERE = Path(__file__).resolve().parent
SOURCE = HERE / "i2c.png"
OUTPUT = HERE / "display_photo" / "i2c_image.h"
SIZE = (128, 128)


def rgb565(red: int, green: int, blue: int) -> int:
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)


def paeth(left: int, above: int, upper_left: int) -> int:
    estimate = left + above - upper_left
    distances = (abs(estimate - left), abs(estimate - above), abs(estimate - upper_left))
    return (left, above, upper_left)[distances.index(min(distances))]


def read_rgba_png(path: Path) -> tuple[int, int, list[bytes]]:
    """Read a non-interlaced, 8-bit RGB/RGBA PNG using only the standard library."""
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("Input is not a PNG file")

    position = 8
    compressed = bytearray()
    width = height = channels = 0
    while position < len(data):
        length = struct.unpack(">I", data[position : position + 4])[0]
        kind = data[position + 4 : position + 8]
        payload = data[position + 8 : position + 8 + length]
        position += length + 12
        if kind == b"IHDR":
            width, height, depth, color, _, _, interlace = struct.unpack(">IIBBBBB", payload)
            if depth != 8 or color not in (2, 6) or interlace != 0:
                raise ValueError("Only non-interlaced 8-bit RGB/RGBA PNG files are supported")
            channels = 3 if color == 2 else 4
        elif kind == b"IDAT":
            compressed.extend(payload)
        elif kind == b"IEND":
            break

    raw = zlib.decompress(compressed)
    stride = width * channels
    rows: list[bytes] = []
    previous = bytearray(stride)
    offset = 0
    for _ in range(height):
        filter_type = raw[offset]
        source = raw[offset + 1 : offset + 1 + stride]
        offset += stride + 1
        row = bytearray(stride)
        for index, value in enumerate(source):
            left = row[index - channels] if index >= channels else 0
            above = previous[index]
            upper_left = previous[index - channels] if index >= channels else 0
            predictors = (0, left, above, (left + above) // 2, paeth(left, above, upper_left))
            if filter_type > 4:
                raise ValueError(f"Unsupported PNG filter: {filter_type}")
            row[index] = (value + predictors[filter_type]) & 0xFF
        rows.append(bytes(row))
        previous = row
    return width, height, rows


source_width, source_height, source_rows = read_rgba_png(SOURCE)
source_channels = len(source_rows[0]) // source_width
pixels = []
for target_y in range(SIZE[1]):
    source_y = target_y * source_height // SIZE[1]
    for target_x in range(SIZE[0]):
        source_x = target_x * source_width // SIZE[0]
        start = source_x * source_channels
        red, green, blue = source_rows[source_y][start : start + 3]
        pixels.append(rgb565(red, green, blue))

lines = [
    "#pragma once",
    "",
    "#include <Arduino.h>",
    "",
    f"constexpr uint16_t I2C_IMAGE_WIDTH = {SIZE[0]};",
    f"constexpr uint16_t I2C_IMAGE_HEIGHT = {SIZE[1]};",
    "",
    "const uint16_t i2cImage[] PROGMEM = {",
]

for start in range(0, len(pixels), 12):
    row = ", ".join(f"0x{value:04X}" for value in pixels[start : start + 12])
    lines.append(f"  {row},")

lines.extend(["};", ""])
OUTPUT.write_text("\n".join(lines), encoding="utf-8")
print(f"Wrote {OUTPUT} ({SIZE[0]}x{SIZE[1]}, {len(pixels) * 2} bytes)")
