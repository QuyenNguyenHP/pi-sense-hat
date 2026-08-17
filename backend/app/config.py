import os
from dataclasses import dataclass
from pathlib import Path


def _load_env() -> None:
    env_file = Path(__file__).resolve().parents[1] / ".env"
    if not env_file.exists():
        return
    for raw_line in env_file.read_text().splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        os.environ.setdefault(key.strip(), value.strip().strip("\"'"))


_load_env()


@dataclass(frozen=True)
class Settings:
    sense_hat_simulator: str = os.getenv("SENSE_HAT_SIMULATOR", "auto")
    cors_origins: str = os.getenv("CORS_ORIGINS", "http://localhost:5173")
    sensor_interval_seconds: float = float(os.getenv("SENSOR_INTERVAL_SECONDS", "0.5"))

    @property
    def origins(self) -> list[str]:
        return [origin.strip() for origin in self.cors_origins.split(",") if origin.strip()]


settings = Settings()
