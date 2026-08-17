import { FormEvent, useEffect, useMemo, useState } from 'react'
import { api, sensorSocketUrl } from './api'
import type { JoystickEvent, RGB, SensorSnapshot, Vector3 } from './types'

const black: RGB = [0, 0, 0]
const emptyPixels = (): RGB[] => Array.from({ length: 64 }, () => [...black]) as RGB[]
const hexToRgb = (hex: string): RGB => [parseInt(hex.slice(1, 3), 16), parseInt(hex.slice(3, 5), 16), parseInt(hex.slice(5, 7), 16)]
const rgbToCss = ([r, g, b]: RGB) => `rgb(${r} ${g} ${b})`

function Metric({ label, value, unit, accent }: { label: string; value?: number; unit: string; accent: string }) {
  return <article className="metric" style={{ '--accent': accent } as React.CSSProperties}>
    <span>{label}</span><strong>{value === undefined ? '—' : value.toFixed(1)}</strong><small>{unit}</small>
  </article>
}

function Vector({ title, data, unit }: { title: string; data?: Vector3; unit: string }) {
  return <article className="panel vector"><h3>{title}</h3>{(['x', 'y', 'z'] as const).map(axis =>
    <div className="axis" key={axis}><b>{axis.toUpperCase()}</b><div><i style={{ width: `${Math.min(100, Math.abs(data?.[axis] ?? 0) * 65)}%` }} /></div><span>{data?.[axis].toFixed(3) ?? '—'} {unit}</span></div>
  )}</article>
}

function Matrix({ pixels, editable, color, onChange }: { pixels: RGB[]; editable?: boolean; color?: RGB; onChange?: (pixels: RGB[]) => void }) {
  const paint = (index: number) => {
    if (!editable || !onChange || !color) return
    const next = pixels.map(pixel => [...pixel] as RGB)
    next[index] = pixels[index].some(Boolean) ? [...black] : [...color]
    onChange(next)
  }
  return <div className="matrix" aria-label="8 by 8 LED matrix">{pixels.map((pixel, index) =>
    <button type="button" aria-label={`Pixel ${index + 1}`} key={index} onClick={() => paint(index)} style={{ background: rgbToCss(pixel), boxShadow: pixel.some(Boolean) ? `0 0 13px ${rgbToCss(pixel)}` : 'none' }} />
  )}</div>
}

export default function App() {
  const [data, setData] = useState<SensorSnapshot>()
  const [connected, setConnected] = useState(false)
  const [message, setMessage] = useState('Hello from Sense HAT!')
  const [textColor, setTextColor] = useState('#65fbd2')
  const [backgroundColor, setBackgroundColor] = useState('#000000')
  const [speed, setSpeed] = useState(.08)
  const [repeatMessage, setRepeatMessage] = useState(false)
  const [paintColor, setPaintColor] = useState('#ff4d8d')
  const [pixels, setPixels] = useState<RGB[]>(emptyPixels)
  const [events, setEvents] = useState<JoystickEvent[]>([])
  const [notice, setNotice] = useState('')

  useEffect(() => {
    let socket: WebSocket | undefined
    let retry: number
    let active = true
    const connect = () => {
      socket = new WebSocket(sensorSocketUrl)
      socket.onopen = () => setConnected(true)
      socket.onmessage = event => setData(JSON.parse(event.data))
      socket.onclose = () => { setConnected(false); if (active) retry = window.setTimeout(connect, 2000) }
      socket.onerror = () => socket?.close()
    }
    connect()
    const joystickTimer = window.setInterval(() => api.joystick().then(setEvents).catch(() => undefined), 120)
    return () => { active = false; clearTimeout(retry); clearInterval(joystickTimer); socket?.close() }
  }, [])

  useEffect(() => { if (data?.matrix?.length === 64) setPixels(data.matrix) }, [data?.matrix])
  const updated = useMemo(() => data ? new Date(data.timestamp).toLocaleTimeString() : 'waiting…', [data])
  const perform = async (job: () => Promise<unknown>, success: string) => {
    try { await job(); setNotice(success) } catch (error) { setNotice(error instanceof Error ? error.message : 'Request failed') }
    window.setTimeout(() => setNotice(''), 2500)
  }
  const submitMessage = (event: FormEvent) => {
    event.preventDefault()
    perform(() => api.message(message, hexToRgb(textColor), hexToRgb(backgroundColor), speed, repeatMessage), repeatMessage ? 'Message is looping' : 'Message sent to the matrix')
  }

  const activeDirection = events[0]?.action !== 'released' ? events[0]?.direction ?? '' : ''

  return <main>
    <header><div><p className="eyebrow">RASPBERRY PI / SENSE HAT</p><h1>Mission Control</h1></div><div className={`status ${connected ? 'online' : ''}`}><i />{connected ? 'Live' : 'Connecting'}<small>{updated}</small></div></header>
    {data?.simulated && <div className="banner">Simulator mode — connect this API on the Raspberry Pi to see real sensor data.</div>}

    <section className="metrics">
      <Metric label="Temperature" value={data?.temperature} unit="°C" accent="#ff7a59" />
      <Metric label="Humidity" value={data?.humidity} unit="% RH" accent="#55b9ff" />
      <Metric label="Pressure" value={data?.pressure} unit="hPa" accent="#9c7cff" />
      <Metric label="Compass" value={data?.compass} unit="degrees" accent="#65fbd2" />
    </section>

    <section className="dashboard-grid">
      <article className="panel orientation"><div><p className="eyebrow">ORIENTATION</p><h2>Spatial attitude</h2></div><div className="orb" style={{ transform: `rotateX(${data?.orientation.pitch ?? 0}deg) rotateZ(${data?.orientation.roll ?? 0}deg)` }}><span /></div><dl>{(['pitch', 'roll', 'yaw'] as const).map(k => <div key={k}><dt>{k}</dt><dd>{data?.orientation[k].toFixed(1) ?? '—'}°</dd></div>)}</dl></article>
      <Vector title="Accelerometer" data={data?.acceleration} unit="g" />
      <Vector title="Gyroscope" data={data?.gyroscope} unit="rad/s" />
    </section>

    <section className="control-grid">
      <article className="panel led-panel"><div><p className="eyebrow">LED MATRIX</p><h2>Pixel studio</h2><p>Click pixels to paint or erase, then send the frame.</p></div><Matrix pixels={pixels} editable color={hexToRgb(paintColor)} onChange={setPixels} /><div className="matrix-tools"><label>Paint <input type="color" value={paintColor} onChange={e => setPaintColor(e.target.value)} /></label><button onClick={() => setPixels(emptyPixels())}>Reset canvas</button><button className="primary" onClick={() => perform(() => api.pixels(pixels), 'Pixel frame sent')}>Send pixels</button></div></article>
      <article className="panel controls"><p className="eyebrow">MESSAGE</p><h2>Write to the Pi</h2><form onSubmit={submitMessage}><label>Text<input value={message} maxLength={160} onChange={e => setMessage(e.target.value)} /></label><div className="field-row"><label>Text color<input type="color" value={textColor} onChange={e => setTextColor(e.target.value)} /></label><label>Background<input type="color" value={backgroundColor} onChange={e => setBackgroundColor(e.target.value)} /></label></div><label>Scroll speed <output>{speed.toFixed(2)}s</output><input type="range" min=".01" max=".3" step=".01" value={speed} onChange={e => setSpeed(Number(e.target.value))} /></label><label className="check"><input type="checkbox" checked={repeatMessage} onChange={e => setRepeatMessage(e.target.checked)} /><span>Run continuously</span></label><button className="primary" type="submit">Scroll message</button></form><div className="actions"><button onClick={() => perform(api.clear, 'Matrix cleared')}>Clear matrix</button>{[0, 90, 180, 270].map(r => <button key={r} onClick={() => perform(() => api.rotate(r), `Rotated to ${r}°`)}>{r}°</button>)}</div></article>
    </section>

    <section className="panel joystick"><div><p className="eyebrow">JOYSTICK</p><h2>Live controller</h2><div className="dpad" aria-label={`Joystick ${activeDirection || 'idle'}`}><i className={`up ${activeDirection === 'up' ? 'active' : ''}`}>▲</i><i className={`left ${activeDirection === 'left' ? 'active' : ''}`}>◀</i><i className={`middle ${activeDirection === 'middle' ? 'active' : ''}`}>●</i><i className={`right ${activeDirection === 'right' ? 'active' : ''}`}>▶</i><i className={`down ${activeDirection === 'down' ? 'active' : ''}`}>▼</i></div></div>{events.length ? <ol>{events.slice(0, 8).map((event, index) => <li key={`${event.timestamp}-${index}`}><b>{event.direction}</b><span>{event.action}</span><time>{new Date(event.timestamp).toLocaleTimeString()}</time></li>)}</ol> : <p className="muted">Move or press the Sense HAT joystick to see events here.</p>}</section>
    {notice && <div className="toast">{notice}</div>}
  </main>
}
