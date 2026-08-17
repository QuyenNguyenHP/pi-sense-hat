import asyncio
import math
import random
import threading
import time
from collections import deque
from datetime import datetime, timezone

from .models import JoystickEvent, SensorSnapshot


class SenseService:
    def __init__(self, simulator_setting: str = "auto") -> None:
        self.simulated = True
        self._sense = None
        self._pixels = [(0, 0, 0)] * 64
        self._rotation = 0
        self._events: deque[JoystickEvent] = deque(maxlen=50)
        self._lock = threading.Lock()

        if simulator_setting.lower() not in {"true", "1", "yes"}:
            try:
                from sense_hat import SenseHat

                self._sense = SenseHat()
                self.simulated = False
                self._sense.stick.direction_any = self._on_joystick
            except (ImportError, OSError, RuntimeError):
                if simulator_setting.lower() in {"false", "0", "no"}:
                    raise RuntimeError("Sense HAT hardware is required but could not be initialized")

    @staticmethod
    def _now() -> str:
        return datetime.now(timezone.utc).isoformat()

    def _on_joystick(self, event: object) -> None:
        self._events.append(
            JoystickEvent(
                timestamp=self._now(),
                direction=str(getattr(event, "direction", "unknown")),
                action=str(getattr(event, "action", "unknown")),
            )
        )

    def read(self) -> SensorSnapshot:
        if self._sense is not None:
            orientation = self._sense.get_orientation_degrees()
            acceleration = self._sense.get_accelerometer_raw()
            gyroscope = self._sense.get_gyroscope_raw()
            pixels = [tuple(pixel) for pixel in self._sense.get_pixels()]
            return SensorSnapshot(
                timestamp=self._now(),
                simulated=False,
                temperature=round(self._sense.get_temperature(), 2),
                humidity=round(self._sense.get_humidity(), 2),
                pressure=round(self._sense.get_pressure(), 2),
                orientation={key: round(orientation[key], 2) for key in ("pitch", "roll", "yaw")},
                acceleration={key: round(acceleration[key], 4) for key in ("x", "y", "z")},
                gyroscope={key: round(gyroscope[key], 4) for key in ("x", "y", "z")},
                compass=round(self._sense.get_compass(), 2),
                matrix=pixels,
            )

        t = time.monotonic()
        jitter = random.uniform(-0.15, 0.15)
        return SensorSnapshot(
            timestamp=self._now(),
            simulated=True,
            temperature=round(24 + math.sin(t / 12) * 2 + jitter, 2),
            humidity=round(55 + math.sin(t / 18) * 8, 2),
            pressure=round(1012 + math.sin(t / 25) * 4, 2),
            orientation={"pitch": (t * 7) % 360, "roll": 180 + math.sin(t / 3) * 30, "yaw": (t * 4) % 360},
            acceleration={"x": math.sin(t) * 0.05, "y": math.cos(t) * 0.05, "z": 0.98},
            gyroscope={"x": math.sin(t / 2) * 0.02, "y": math.cos(t / 2) * 0.02, "z": 0.0},
            compass=(t * 4) % 360,
            matrix=self._pixels.copy(),
        )

    async def show_message(self, text: str, text_color: tuple[int, int, int], background_color: tuple[int, int, int], scroll_speed: float) -> None:
        if self._sense is not None:
            await asyncio.to_thread(self._sense.show_message, text, scroll_speed, text_color, background_color)
            return

        # Keep the simulator responsive while visually acknowledging the command.
        with self._lock:
            color = tuple(text_color)
            self._pixels = [color if (i + len(text)) % 3 == 0 else tuple(background_color) for i in range(64)]
        await asyncio.sleep(min(len(text) * scroll_speed, 2.0))

    def clear(self) -> None:
        if self._sense is not None:
            self._sense.clear()
        with self._lock:
            self._pixels = [(0, 0, 0)] * 64

    def set_rotation(self, rotation: int) -> None:
        if self._sense is not None:
            self._sense.set_rotation(rotation, redraw=True)
        self._rotation = rotation

    def set_pixels(self, pixels: list[tuple[int, int, int]]) -> None:
        normalized = [tuple(pixel) for pixel in pixels]
        if self._sense is not None:
            self._sense.set_pixels(normalized)
        with self._lock:
            self._pixels = normalized

    def events(self) -> list[JoystickEvent]:
        return list(reversed(self._events))

