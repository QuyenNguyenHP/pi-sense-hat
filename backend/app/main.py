import asyncio
from contextlib import asynccontextmanager

from fastapi import BackgroundTasks, FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware

from .config import settings
from .models import PixelsCommand, RotationCommand, TextCommand
from .sense_service import SenseService

service = SenseService(settings.sense_hat_simulator)


@asynccontextmanager
async def lifespan(_: FastAPI):
    yield
    service.clear()


app = FastAPI(title="Pi Sense HAT API", version="1.0.0", lifespan=lifespan)
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.get("/api/health")
def health() -> dict[str, object]:
    return {"status": "ok", "simulated": service.simulated}


@app.get("/api/sensors")
def sensors():
    return service.read()


@app.get("/api/joystick")
def joystick():
    return service.events()


@app.post("/api/matrix/message", status_code=202)
def message(command: TextCommand, background_tasks: BackgroundTasks):
    background_tasks.add_task(
        service.show_message,
        command.text,
        command.text_color,
        command.background_color,
        command.scroll_speed,
    )
    return {"accepted": True}


@app.post("/api/matrix/clear")
def clear():
    service.clear()
    return {"ok": True}


@app.post("/api/matrix/rotation")
def rotation(command: RotationCommand):
    service.set_rotation(command.rotation)
    return {"ok": True, "rotation": command.rotation}


@app.put("/api/matrix/pixels")
def pixels(command: PixelsCommand):
    service.set_pixels(command.pixels)
    return {"ok": True}


@app.websocket("/ws/sensors")
async def sensor_stream(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            await websocket.send_json(service.read().model_dump())
            await asyncio.sleep(settings.sensor_interval_seconds)
    except WebSocketDisconnect:
        pass

