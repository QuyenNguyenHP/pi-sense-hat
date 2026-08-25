# ESP32 Force Sensor

This Arduino sketch reads a force-sensitive resistor (FSR) through an ESP32
analog input. It prints the raw reading and an estimated force in newtons to the
Serial Monitor. It is configured for a nominal sensor range of 20 g to 20 kg,
which corresponds to approximately 0.20 N to 196.13 N.

## Parts

- ESP32 development board
- Force-sensitive resistor (FSR)
- 10 kΩ resistor
- Breadboard and jumper wires

## Wiring

Build a voltage divider as follows:

```text
3.3V ---- FSR ----+---- GPIO 35
                  |
                 10 kΩ
                  |
                 GND
```

Use `3.3V`, not `5V`, because ESP32 GPIO pins are not 5 V tolerant. GPIO 35 is
input-only, which is appropriate for this analog measurement. It does not have
an internal pull-up or pull-down resistor, so the external resistor is required.

## Uploading

1. Open `force_sensor.ino` in Arduino IDE.
2. Install the ESP32 board package if it is not already installed.
3. Select the correct ESP32 board and serial port.
4. Upload the sketch.
5. Open the Serial Monitor and set its baud rate to `115200`.
6. Press the FSR and observe the values.

## ADC readings and calibration

The sketch configures a 12-bit ADC, giving raw values from `0` to `4095`. The
included calibration values are only starting points because readings depend on
the FSR, fixed resistor, wiring, and applied force.

1. Apply a known 20 g load and record the raw ADC reading.
2. Apply a known 20 kg load, if it is safe for the sensor and mounting, and
   record the raw ADC reading.
3. Enter the readings in `force_sensor.ino`:

```cpp
const int ADC_AT_20_G = 50;
const int ADC_AT_20_KG = 4000;
```

Force is calculated using `F = m × 9.80665`. The sketch linearly interpolates
between the two calibration points and limits output to about `196.13 N`.

The sensor is sampled every 50 ms (20 readings per second). If the raw ADC value
reacts quickly but the force value is incorrect, update `ADC_AT_20_G` and
`ADC_AT_20_KG`; the default values are placeholders, not factory calibration.

FSRs are generally nonlinear and vary between units, so two-point linear
calibration only provides an estimate. For better accuracy, collect several
known-load measurements across the range and create a multi-point calibration
curve. Do not apply 20 kg unless the sensor and its mechanical mounting are
rated to handle that load safely.
