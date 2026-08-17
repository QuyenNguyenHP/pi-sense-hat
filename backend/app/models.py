from typing import Annotated, Literal

from pydantic import BaseModel, Field, field_validator


Channel = Annotated[int, Field(ge=0, le=255)]
RGB = tuple[Channel, Channel, Channel]


class Vector3(BaseModel):
    x: float
    y: float
    z: float


class Orientation(BaseModel):
    pitch: float
    roll: float
    yaw: float


class SensorSnapshot(BaseModel):
    timestamp: str
    simulated: bool
    temperature: float
    humidity: float
    pressure: float
    orientation: Orientation
    acceleration: Vector3
    gyroscope: Vector3
    compass: float
    matrix: list[RGB]


class TextCommand(BaseModel):
    text: str = Field(min_length=1, max_length=160)
    text_color: RGB = (255, 255, 255)
    background_color: RGB = (0, 0, 0)
    scroll_speed: float = Field(default=0.08, ge=0.01, le=1.0)


class RotationCommand(BaseModel):
    rotation: Literal[0, 90, 180, 270]


class PixelsCommand(BaseModel):
    pixels: list[RGB]

    @field_validator("pixels")
    @classmethod
    def exactly_64_pixels(cls, value: list[RGB]) -> list[RGB]:
        if len(value) != 64:
            raise ValueError("The Sense HAT matrix requires exactly 64 pixels")
        return value


class JoystickEvent(BaseModel):
    timestamp: str
    direction: str
    action: str
