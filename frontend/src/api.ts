import type { JoystickEvent, RGB } from './types'

export const API_URL = (import.meta.env.VITE_API_URL || 'http://localhost:8000').replace(/\/$/, '')

export async function request(path: string, options?: RequestInit) {
  const response = await fetch(`${API_URL}${path}`, {
    ...options,
    headers: { 'Content-Type': 'application/json', ...options?.headers },
  })
  if (!response.ok) throw new Error((await response.text()) || `Request failed: ${response.status}`)
  return response.json()
}

export const sensorSocketUrl = `${API_URL.replace(/^http/, 'ws')}/ws/sensors`

export const api = {
  joystick: () => request('/api/joystick') as Promise<JoystickEvent[]>,
  message: (text: string, textColor: RGB, backgroundColor: RGB, scrollSpeed: number, repeat: boolean) =>
    request('/api/matrix/message', {
      method: 'POST',
      body: JSON.stringify({ text, text_color: textColor, background_color: backgroundColor, scroll_speed: scrollSpeed, repeat }),
    }),
  clear: () => request('/api/matrix/clear', { method: 'POST' }),
  rotate: (rotation: number) => request('/api/matrix/rotation', { method: 'POST', body: JSON.stringify({ rotation }) }),
  pixels: (pixels: RGB[]) => request('/api/matrix/pixels', { method: 'PUT', body: JSON.stringify({ pixels }) }),
}
