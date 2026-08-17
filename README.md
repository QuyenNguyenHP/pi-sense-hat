# Pi Sense HAT Dashboard

A full-stack dashboard for a Raspberry Pi Sense HAT:

- **FastAPI backend** runs on the Raspberry Pi and reads the Sense HAT.
- **React frontend** can be developed or built on any laptop and talks to the Pi over HTTP/WebSocket.
- A built-in simulator lets you develop without Raspberry Pi hardware.

## Features

- Temperature, humidity, and pressure
- Pitch, roll, yaw, compass heading
- Accelerometer and gyroscope
- Joystick event stream
- Live 8×8 LED matrix preview
- Scroll custom text with foreground/background color and speed controls
- Rotate or clear the matrix
- Paint pixels directly and send the entire 8×8 image

## 1. Run the backend on the Pi

```bash
cd backend
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
pip install -r requirements-pi.txt
cp .env.example .env
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

If `sense-hat` is unavailable, the backend automatically uses simulated readings. To require real hardware, set `SENSE_HAT_SIMULATOR=false` in `.env`.

Check the API at `http://<pi-ip>:8000/docs`.

## 2. Run the frontend on the laptop

```bash
cd frontend
npm install
cp .env.example .env
# Edit VITE_API_URL to contain the Pi's IP address.
npm run dev
```

Open the URL printed by Vite. For a production build:

```bash
npm run build
```

The static output is written to `frontend/dist` and can be hosted by any static web server.

## Network notes

- The laptop and Pi must be reachable on the same network.
- Allow TCP port `8000` through the Pi firewall.
- Set `CORS_ORIGINS` to the frontend origins in production, separated by commas.
- If the frontend is served over HTTPS, put the API behind HTTPS too; browsers block insecure API/WebSocket calls from secure pages.

