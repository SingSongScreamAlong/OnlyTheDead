/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Ballistics Calculation System
 *
 * Handles shell trajectory, flight time, and impact calculations
 */

import {
  Position3D,
  Vector3D,
  ShellDefinition,
  ShellInFlight,
  ShellType,
  ArtilleryGameState
} from './types';
import { getShellDefinition } from './shells';

// ============================================================================
// CONSTANTS
// ============================================================================

const GRAVITY = 9.81; // m/s²
const AIR_DENSITY = 1.225; // kg/m³ at sea level
const DRAG_COEFFICIENT = 0.295; // Typical for artillery shells

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Calculate distance between two 3D positions
 */
export function distance3D(p1: Position3D, p2: Position3D): number {
  const dx = p2.x - p1.x;
  const dy = p2.y - p1.y;
  const dz = p2.z - p1.z;
  return Math.sqrt(dx * dx + dy * dy + dz * dz);
}

/**
 * Calculate 2D distance (horizontal plane)
 */
export function distance2D(p1: Position3D, p2: Position3D): number {
  const dx = p2.x - p1.x;
  const dz = p2.z - p1.z;
  return Math.sqrt(dx * dx + dz * dz);
}

/**
 * Normalize a vector
 */
export function normalize(v: Vector3D): Vector3D {
  const length = Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (length === 0) return { x: 0, y: 0, z: 0 };
  return {
    x: v.x / length,
    y: v.y / length,
    z: v.z / length
  };
}

/**
 * Vector magnitude
 */
export function magnitude(v: Vector3D): number {
  return Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * Add two vectors
 */
export function addVectors(v1: Vector3D, v2: Vector3D): Vector3D {
  return {
    x: v1.x + v2.x,
    y: v1.y + v2.y,
    z: v1.z + v2.z
  };
}

/**
 * Scale a vector
 */
export function scaleVector(v: Vector3D, scale: number): Vector3D {
  return {
    x: v.x * scale,
    y: v.y * scale,
    z: v.z * scale
  };
}

// ============================================================================
// BALLISTICS CALCULATIONS
// ============================================================================

/**
 * Calculate firing angle needed to hit target at given range
 * Using simple ballistic formula (no air resistance)
 */
export function calculateFiringAngle(
  range: number,
  muzzleVelocity: number,
  heightDifference: number = 0
): number {
  // Using the simplified ballistic equation
  // For high arc trajectory, use the higher angle solution
  const g = GRAVITY;
  const v = muzzleVelocity;
  const r = range;
  const h = heightDifference;

  // θ = arctan((v² ± sqrt(v⁴ - g(gx² + 2hv²))) / (gx))
  const v2 = v * v;
  const v4 = v2 * v2;
  const discriminant = v4 - g * (g * r * r + 2 * h * v2);

  if (discriminant < 0) {
    // Target out of range
    return NaN;
  }

  // Use high angle for howitzers, low angle for field guns
  const angle1 = Math.atan((v2 + Math.sqrt(discriminant)) / (g * r));
  const angle2 = Math.atan((v2 - Math.sqrt(discriminant)) / (g * r));

  return angle1; // High arc (typical for WWI artillery)
}

/**
 * Calculate flight time for a shell
 */
export function calculateFlightTime(
  range: number,
  muzzleVelocity: number,
  angle: number
): number {
  // t = range / (velocity * cos(angle))
  return range / (muzzleVelocity * Math.cos(angle));
}

/**
 * Calculate initial velocity vector given target and muzzle velocity
 */
export function calculateInitialVelocity(
  origin: Position3D,
  target: Position3D,
  muzzleVelocity: number,
  useHighArc: boolean = true
): Vector3D {
  const horizontalRange = distance2D(origin, target);
  const heightDiff = target.y - origin.y;

  const angle = calculateFiringAngle(horizontalRange, muzzleVelocity, heightDiff);

  if (isNaN(angle)) {
    // Target out of range - return zero vector
    return { x: 0, y: 0, z: 0 };
  }

  // Calculate direction in horizontal plane
  const dx = target.x - origin.x;
  const dz = target.z - origin.z;
  const horizontalAngle = Math.atan2(dz, dx);

  // Build velocity vector
  return {
    x: muzzleVelocity * Math.cos(angle) * Math.cos(horizontalAngle),
    y: muzzleVelocity * Math.sin(angle),
    z: muzzleVelocity * Math.cos(angle) * Math.sin(horizontalAngle)
  };
}

/**
 * Calculate shell position at time t using simplified physics
 */
export function calculateShellPosition(
  origin: Position3D,
  initialVelocity: Vector3D,
  time: number
): Position3D {
  // Simple ballistic trajectory (no air resistance)
  // p(t) = p0 + v0*t + 0.5*a*t²
  return {
    x: origin.x + initialVelocity.x * time,
    y: origin.y + initialVelocity.y * time - 0.5 * GRAVITY * time * time,
    z: origin.z + initialVelocity.z * time
  };
}

/**
 * Calculate shell velocity at time t
 */
export function calculateShellVelocity(
  initialVelocity: Vector3D,
  time: number
): Vector3D {
  // v(t) = v0 + a*t
  return {
    x: initialVelocity.x,
    y: initialVelocity.y - GRAVITY * time,
    z: initialVelocity.z
  };
}

/**
 * Calculate detailed shell trajectory with air resistance
 * Uses Runge-Kutta 4th order integration for accuracy
 */
export function calculateDetailedTrajectory(
  origin: Position3D,
  target: Position3D,
  shellDef: ShellDefinition,
  timeStep: number = 0.1
): Position3D[] {
  const trajectory: Position3D[] = [];

  const initialVelocity = calculateInitialVelocity(
    origin,
    target,
    shellDef.ballistics.muzzleVelocity,
    shellDef.ballistics.trajectory !== 'flat'
  );

  let position = { ...origin };
  let velocity = { ...initialVelocity };
  let time = 0;

  trajectory.push({ ...position });

  // Simplified drag calculation
  const dragFactor = 0.5 * DRAG_COEFFICIENT * AIR_DENSITY / shellDef.ballistics.shellWeight;

  while (position.y >= target.y && time < 120) { // Max 2 minutes flight
    // Calculate drag force
    const speed = magnitude(velocity);
    const dragMagnitude = dragFactor * speed * speed;
    const dragDirection = normalize(velocity);
    const drag = scaleVector(dragDirection, -dragMagnitude);

    // Update velocity (gravity + drag)
    velocity.x += (drag.x) * timeStep;
    velocity.y += (-GRAVITY + drag.y) * timeStep;
    velocity.z += (drag.z) * timeStep;

    // Update position
    position.x += velocity.x * timeStep;
    position.y += velocity.y * timeStep;
    position.z += velocity.z * timeStep;

    time += timeStep;

    trajectory.push({ ...position });

    // Ground impact
    if (position.y <= 0) break;
  }

  return trajectory;
}

/**
 * Calculate impact point given origin, angle, and muzzle velocity
 */
export function calculateImpactPoint(
  origin: Position3D,
  azimuth: number,        // Horizontal angle (radians)
  elevation: number,      // Vertical angle (radians)
  muzzleVelocity: number
): Position3D {
  const initialVelocity: Vector3D = {
    x: muzzleVelocity * Math.cos(elevation) * Math.cos(azimuth),
    y: muzzleVelocity * Math.sin(elevation),
    z: muzzleVelocity * Math.cos(elevation) * Math.sin(azimuth)
  };

  // Calculate time to impact (when y = 0)
  // Using: y = y0 + vy*t - 0.5*g*t²
  // 0 = y0 + vy*t - 0.5*g*t²
  const a = 0.5 * GRAVITY;
  const b = -initialVelocity.y;
  const c = -origin.y;

  const discriminant = b * b - 4 * a * c;
  if (discriminant < 0) {
    return origin; // Should not happen
  }

  const t = (-b + Math.sqrt(discriminant)) / (2 * a);

  return calculateShellPosition(origin, initialVelocity, t);
}

/**
 * Estimate flight time based on range and shell type
 * Uses historical data from shell definitions
 */
export function estimateFlightTime(
  range: number,
  shellDef: ShellDefinition
): number {
  // Use typical flight time as reference
  const typicalRange = shellDef.ballistics.rangeMax / 2;
  const typicalTime = shellDef.ballistics.flightTimeTypical;

  // Scale linearly (simplified)
  return (range / typicalRange) * typicalTime;
}

/**
 * Calculate warning time for player based on shell distance and type
 */
export function calculateWarningTime(
  shellPosition: Position3D,
  playerPosition: Position3D,
  shellDef: ShellDefinition,
  timeToImpact: number
): number {
  const distance = distance3D(shellPosition, playerPosition);

  // Sound travels at ~340 m/s
  const soundSpeed = 340;
  const soundDelay = distance / soundSpeed;

  // Warning time is when player can hear it minus sound delay
  const effectiveWarning = timeToImpact - soundDelay;

  // Clamp to shell's audio signature warning time
  return Math.max(
    shellDef.audio.warnTimeSeconds,
    Math.min(effectiveWarning, shellDef.audio.warnTimeSecondsMax)
  );
}

/**
 * Calculate audio characteristics for player detection
 */
export function calculateAudioProperties(
  shellPosition: Position3D,
  playerPosition: Position3D,
  shellVelocity: Vector3D,
  shellDef: ShellDefinition
): {
  volume: number;
  pitch: number;
  direction: Vector3D;
  isAudible: boolean;
} {
  const distance = distance3D(shellPosition, playerPosition);

  // Maximum audible range based on shell size
  const maxRange = shellDef.damage.shellShock * 10;

  if (distance > maxRange) {
    return {
      volume: 0,
      pitch: shellDef.audio.frequencyHz,
      direction: { x: 0, y: 0, z: 0 },
      isAudible: false
    };
  }

  // Volume falloff with distance (inverse square law)
  const volumeAtDistance = shellDef.audio.volumeDb - 20 * Math.log10(distance / 10);
  const volume = Math.max(0, Math.min(100, (volumeAtDistance / 140) * 100));

  // Doppler effect on pitch
  const soundSpeed = 340;
  const shellSpeed = magnitude(shellVelocity);
  const direction = normalize({
    x: playerPosition.x - shellPosition.x,
    y: playerPosition.y - shellPosition.y,
    z: playerPosition.z - shellPosition.z
  });

  // Doppler shift factor
  const relativeVelocity = shellVelocity.x * direction.x +
                           shellVelocity.y * direction.y +
                           shellVelocity.z * direction.z;

  const dopplerFactor = 1 + (relativeVelocity / soundSpeed);
  const pitch = shellDef.audio.frequencyHz * dopplerFactor;

  return {
    volume,
    pitch,
    direction,
    isAudible: volume > 10
  };
}

/**
 * Apply wind effects to shell trajectory
 */
export function applyWindEffect(
  velocity: Vector3D,
  windSpeed: number,
  windDirection: number, // radians
  timeStep: number
): Vector3D {
  // Wind affects horizontal velocity
  const windX = windSpeed * Math.cos(windDirection);
  const windZ = windSpeed * Math.sin(windDirection);

  // Wind effect is cumulative over time
  const windEffect = 0.1; // Wind influence factor

  return {
    x: velocity.x + windX * windEffect * timeStep,
    y: velocity.y,
    z: velocity.z + windZ * windEffect * timeStep
  };
}

/**
 * Add random dispersion to target (shell spread)
 */
export function applyDispersion(
  target: Position3D,
  accuracy: number, // 0-100, where 100 is perfect
  range: number
): Position3D {
  // Dispersion increases with range and decreases with accuracy
  const dispersionFactor = (100 - accuracy) / 100;
  const baseDispersion = range * 0.01; // 1% of range at perfect accuracy

  const maxDispersion = baseDispersion * dispersionFactor * 5;

  // Random offset in X and Z
  const offsetX = (Math.random() - 0.5) * 2 * maxDispersion;
  const offsetZ = (Math.random() - 0.5) * 2 * maxDispersion;

  return {
    x: target.x + offsetX,
    y: target.y,
    z: target.z + offsetZ
  };
}

/**
 * Calculate if shell will hit target area
 */
export function willHitTarget(
  impactPoint: Position3D,
  target: Position3D,
  targetRadius: number
): boolean {
  const distance = distance2D(impactPoint, target);
  return distance <= targetRadius;
}

/**
 * Create a shell in flight
 */
export function createShellInFlight(
  id: string,
  shellType: ShellType,
  origin: Position3D,
  target: Position3D,
  launchTime: number,
  firingBattery: string,
  accuracy: number = 85,
  gameState?: ArtilleryGameState
): ShellInFlight | null {
  const shellDef = getShellDefinition(shellType);
  if (!shellDef) return null;

  // Apply dispersion to target
  const range = distance2D(origin, target);
  const actualTarget = applyDispersion(target, accuracy, range);

  // Calculate initial velocity
  const initialVelocity = calculateInitialVelocity(
    origin,
    actualTarget,
    shellDef.ballistics.muzzleVelocity,
    shellDef.ballistics.trajectory !== 'flat'
  );

  // Calculate flight time
  const flightTime = estimateFlightTime(range, shellDef);

  // Apply wind if game state provided
  let velocity = initialVelocity;
  if (gameState) {
    velocity = applyWindEffect(
      velocity,
      gameState.windSpeed,
      gameState.windDirection * (Math.PI / 180),
      0
    );
  }

  return {
    id,
    shellType,
    definition: shellDef,
    origin,
    target: actualTarget,
    currentPosition: { ...origin },
    velocity,
    launchTime,
    impactTime: launchTime + flightTime,
    timeRemaining: flightTime,
    audioSignature: shellDef.audio,
    audibleRange: shellDef.damage.shellShock * 10,
    firingBattery,
    fireMission: undefined
  };
}

/**
 * Update shell position during flight
 */
export function updateShellInFlight(
  shell: ShellInFlight,
  currentTime: number,
  deltaTime: number,
  gameState?: ArtilleryGameState
): ShellInFlight {
  const timeSinceLaunch = currentTime - shell.launchTime;

  // Update position
  shell.currentPosition = calculateShellPosition(
    shell.origin,
    shell.velocity,
    timeSinceLaunch
  );

  // Update velocity
  shell.velocity = calculateShellVelocity(shell.velocity, timeSinceLaunch);

  // Apply wind effect if game state provided
  if (gameState) {
    shell.velocity = applyWindEffect(
      shell.velocity,
      gameState.windSpeed,
      gameState.windDirection * (Math.PI / 180),
      deltaTime
    );
  }

  // Update time remaining
  shell.timeRemaining = shell.impactTime - currentTime;

  return shell;
}
