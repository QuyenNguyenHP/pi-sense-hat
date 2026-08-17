export type RGB = [number, number, number]
export type Vector3 = { x: number; y: number; z: number }

export type SensorSnapshot = {
  timestamp: string
  simulated: boolean
  temperature: number
  humidity: number
  pressure: number
  orientation: { pitch: number; roll: number; yaw: number }
  acceleration: Vector3
  gyroscope: Vector3
  compass: number
  matrix: RGB[]
}

export type JoystickEvent = {
  timestamp: string
  direction: string
  action: string
}

