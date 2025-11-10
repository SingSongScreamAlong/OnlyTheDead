/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Fire Mission System
 *
 * Handles fire missions, battery coordination, and artillery command
 */

import {
  FireMission,
  FireMissionType,
  FireMissionStatus,
  ArtilleryBattery,
  ArtilleryGun,
  Position3D,
  BombardmentConfig,
  BombardmentIntensity,
  BombardmentPattern,
  ShellType
} from './types';
import { distance2D } from './ballistics';
import { createStandardBombardment } from './bombardment';
import { v4 as uuidv4 } from 'uuid';

// Note: uuid v4 is used for unique IDs. In production, replace with actual implementation

function uuidv4(): string {
  return `${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
}

// ============================================================================
// FIRE MISSION CREATION
// ============================================================================

/**
 * Create a new fire mission
 */
export function createFireMission(
  type: FireMissionType,
  targetPosition: Position3D,
  targetDescription: string,
  priority: number = 3,
  duration: number = 600, // 10 minutes default
  intensity: BombardmentIntensity = BombardmentIntensity.LIGHT
): FireMission {
  const bombardmentPattern = determinePatternForMissionType(type);
  const bombardment = createStandardBombardment(
    intensity,
    bombardmentPattern,
    targetPosition,
    500 // Default radius
  );

  bombardment.duration = duration;

  return {
    id: uuidv4(),
    type,
    status: FireMissionStatus.PLANNED,
    target: {
      position: targetPosition,
      description: targetDescription,
      priority
    },
    batteries: [],
    bombardment,
    startTime: 0,
    endTime: 0
  };
}

/**
 * Determine bombardment pattern based on mission type
 */
function determinePatternForMissionType(type: FireMissionType): BombardmentPattern {
  switch (type) {
    case FireMissionType.CREEPING_BARRAGE:
      return BombardmentPattern.CREEPING_BARRAGE;
    case FireMissionType.COUNTER_BATTERY:
      return BombardmentPattern.CONCENTRATION;
    case FireMissionType.HARASSING:
      return BombardmentPattern.RANDOM;
    case FireMissionType.FINAL_PROTECTIVE:
      return BombardmentPattern.BOX_BARRAGE;
    default:
      return BombardmentPattern.RANDOM;
  }
}

/**
 * Create counter-battery fire mission
 */
export function createCounterBatteryMission(
  enemyBatteryPosition: Position3D,
  observerAccuracy: number = 80
): FireMission {
  const mission = createFireMission(
    FireMissionType.COUNTER_BATTERY,
    enemyBatteryPosition,
    "Enemy artillery battery",
    5, // High priority
    1800, // 30 minutes
    BombardmentIntensity.HEAVY
  );

  // Counter-battery uses concentration fire
  mission.bombardment.pattern = BombardmentPattern.CONCENTRATION;

  // Adjust for observer accuracy
  if (observerAccuracy < 70) {
    mission.bombardment.targetArea.radius = 150; // Larger area if uncertain
  } else {
    mission.bombardment.targetArea.radius = 75; // Tighter grouping
  }

  return mission;
}

/**
 * Create creeping barrage mission
 */
export function createCreepingBarrageMission(
  startLine: Position3D,
  endLine: Position3D,
  barrageWidth: number,
  advanceDuration: number = 3600 // 1 hour
): FireMission {
  const mission = createFireMission(
    FireMissionType.CREEPING_BARRAGE,
    startLine,
    "Creeping barrage support",
    5,
    advanceDuration,
    BombardmentIntensity.HEAVY
  );

  mission.bombardment.pattern = BombardmentPattern.CREEPING_BARRAGE;
  mission.bombardment.targetArea.width = barrageWidth;
  mission.bombardment.targetArea.length = distance2D(startLine, endLine);

  return mission;
}

/**
 * Create preparatory bombardment mission
 */
export function createPreparatoryBombardment(
  targetArea: Position3D,
  targetRadius: number,
  duration: number = 7200 // 2 hours
): FireMission {
  const mission = createFireMission(
    FireMissionType.PREPARATORY,
    targetArea,
    "Preparatory bombardment before assault",
    4,
    duration,
    BombardmentIntensity.DRUMFIRE
  );

  mission.bombardment.targetArea.radius = targetRadius;

  return mission;
}

// ============================================================================
// BATTERY MANAGEMENT
// ============================================================================

/**
 * Create an artillery battery
 */
export function createArtilleryBattery(
  name: string,
  nationality: 'french' | 'german',
  gunType: ShellType,
  numberOfGuns: number,
  position: Position3D,
  range: number
): ArtilleryBattery {
  const guns: ArtilleryGun[] = [];

  for (let i = 0; i < numberOfGuns; i++) {
    guns.push({
      id: `${name}_gun_${i}`,
      type: gunType,
      position: {
        x: position.x + (Math.random() - 0.5) * 50,
        y: position.y,
        z: position.z + (Math.random() - 0.5) * 50
      },
      ammunition: 1000,
      rateOfFire: getRateOfFire(gunType),
      crew: 6,
      operational: true,
      lastFired: 0
    });
  }

  return {
    id: uuidv4(),
    name,
    nationality,
    guns,
    position,
    range,
    ammunition: {
      [gunType]: numberOfGuns * 1000
    },
    commandPost: {
      position: {
        x: position.x + 100,
        y: position.y,
        z: position.z
      },
      telephone: true,
      radio: false
    }
  };
}

/**
 * Get rate of fire for shell type
 */
function getRateOfFire(shellType: ShellType): number {
  switch (shellType) {
    case ShellType.FRENCH_75MM:
      return 15; // 15 rounds per minute
    case ShellType.GERMAN_77MM:
      return 12;
    case ShellType.HOWITZER_155MM:
      return 4;
    case ShellType.GERMAN_210MM:
      return 2;
    case ShellType.FRENCH_305MM:
      return 1;
    case ShellType.GERMAN_420MM:
      return 0.125; // 1 round per 8 minutes
    default:
      return 5;
  }
}

/**
 * Check if battery can reach target
 */
export function canReachTarget(
  battery: ArtilleryBattery,
  target: Position3D
): boolean {
  const distance = distance2D(battery.position, target);
  return distance <= battery.range;
}

/**
 * Get battery fire rate (total rounds per minute)
 */
export function getBatteryFireRate(battery: ArtilleryBattery): number {
  return battery.guns
    .filter(gun => gun.operational)
    .reduce((total, gun) => total + gun.rateOfFire, 0);
}

/**
 * Check if battery has enough ammunition for mission
 */
export function hasAmmoForMission(
  battery: ArtilleryBattery,
  mission: FireMission
): boolean {
  const shellType = battery.guns[0]?.type;
  if (!shellType) return false;

  const requiredShells = Math.ceil(
    (mission.bombardment.duration / 60) * mission.bombardment.shellsPerMinute
  );

  const availableAmmo = battery.ammunition[shellType] || 0;
  return availableAmmo >= requiredShells;
}

/**
 * Assign battery to fire mission
 */
export function assignBatteryToMission(
  battery: ArtilleryBattery,
  mission: FireMission
): boolean {
  // Check if battery can execute mission
  if (!canReachTarget(battery, mission.target.position)) {
    return false;
  }

  if (!hasAmmoForMission(battery, mission)) {
    return false;
  }

  // Assign battery
  if (!mission.batteries.includes(battery.id)) {
    mission.batteries.push(battery.id);
  }

  battery.currentMission = mission.id;

  return true;
}

/**
 * Start fire mission
 */
export function startFireMission(
  mission: FireMission,
  currentTime: number
): void {
  mission.status = FireMissionStatus.IN_PROGRESS;
  mission.startTime = currentTime;
  mission.endTime = currentTime + mission.bombardment.duration;
}

/**
 * Complete fire mission
 */
export function completeFireMission(mission: FireMission): void {
  mission.status = FireMissionStatus.COMPLETE;
}

/**
 * Cancel fire mission
 */
export function cancelFireMission(mission: FireMission): void {
  mission.status = FireMissionStatus.CANCELLED;
}

// ============================================================================
// BATTERY COORDINATION
// ============================================================================

/**
 * Coordinate multiple batteries for a mission
 */
export function coordinateBatteriesForMission(
  mission: FireMission,
  availableBatteries: ArtilleryBattery[]
): ArtilleryBattery[] {
  const assignedBatteries: ArtilleryBattery[] = [];

  // Calculate required fire rate
  const requiredFireRate = mission.bombardment.shellsPerMinute;

  // Sort batteries by suitability
  const suitableBatteries = availableBatteries
    .filter(battery => canReachTarget(battery, mission.target.position))
    .filter(battery => hasAmmoForMission(battery, mission))
    .sort((a, b) => {
      // Prefer batteries closer to target
      const distA = distance2D(a.position, mission.target.position);
      const distB = distance2D(b.position, mission.target.position);
      return distA - distB;
    });

  // Assign batteries until fire rate requirement is met
  let currentFireRate = 0;
  for (const battery of suitableBatteries) {
    if (currentFireRate >= requiredFireRate) break;

    if (assignBatteryToMission(battery, mission)) {
      assignedBatteries.push(battery);
      currentFireRate += getBatteryFireRate(battery);
    }
  }

  return assignedBatteries;
}

/**
 * Calculate time on target (when shells will impact)
 */
export function calculateTimeOnTarget(
  battery: ArtilleryBattery,
  target: Position3D,
  launchTime: number
): number {
  const distance = distance2D(battery.position, target);

  // Simplified flight time calculation
  // Actual calculation would use shell ballistics
  const flightTime = distance / 200; // ~200 m/s average horizontal velocity

  return launchTime + flightTime;
}

/**
 * Synchronize multiple batteries for simultaneous impact
 */
export function synchronizeBatteries(
  batteries: ArtilleryBattery[],
  target: Position3D,
  desiredImpactTime: number
): Map<string, number> {
  const launchTimes = new Map<string, number>();

  for (const battery of batteries) {
    const timeOnTarget = calculateTimeOnTarget(battery, target, 0);
    const launchTime = desiredImpactTime - timeOnTarget;
    launchTimes.set(battery.id, launchTime);
  }

  return launchTimes;
}

/**
 * Distribute fire across multiple targets (priority queue)
 */
export function distributeFire(
  batteries: ArtilleryBattery[],
  missions: FireMission[]
): Map<string, string> {
  // Maps battery ID to mission ID
  const assignments = new Map<string, string>();

  // Sort missions by priority
  const sortedMissions = [...missions]
    .filter(m => m.status === FireMissionStatus.PLANNED)
    .sort((a, b) => b.target.priority - a.target.priority);

  // Assign batteries to highest priority missions
  const availableBatteries = [...batteries].filter(b => !b.currentMission);

  for (const mission of sortedMissions) {
    const assigned = coordinateBatteriesForMission(mission, availableBatteries);

    for (const battery of assigned) {
      assignments.set(battery.id, mission.id);
      // Remove from available list
      const index = availableBatteries.indexOf(battery);
      if (index > -1) {
        availableBatteries.splice(index, 1);
      }
    }
  }

  return assignments;
}

/**
 * Resupply battery ammunition
 */
export function resupplyBattery(
  battery: ArtilleryBattery,
  shellType: ShellType,
  amount: number
): void {
  const current = battery.ammunition[shellType] || 0;
  battery.ammunition[shellType] = current + amount;
}

/**
 * Consume ammunition from battery
 */
export function consumeAmmunition(
  battery: ArtilleryBattery,
  shellType: ShellType,
  amount: number
): boolean {
  const current = battery.ammunition[shellType] || 0;

  if (current < amount) {
    return false; // Not enough ammo
  }

  battery.ammunition[shellType] = current - amount;
  return true;
}

/**
 * Update battery operational status based on casualties
 */
export function updateBatteryStatus(
  battery: ArtilleryBattery,
  casualtiesPercent: number
): void {
  // If casualties exceed threshold, guns become non-operational
  const gunsToDisable = Math.floor(battery.guns.length * (casualtiesPercent / 100));

  for (let i = 0; i < gunsToDisable; i++) {
    if (battery.guns[i]) {
      battery.guns[i].operational = false;
    }
  }
}

/**
 * Calculate battery effectiveness
 */
export function calculateBatteryEffectiveness(battery: ArtilleryBattery): number {
  const operationalGuns = battery.guns.filter(g => g.operational).length;
  const totalGuns = battery.guns.length;

  if (totalGuns === 0) return 0;

  return (operationalGuns / totalGuns) * 100;
}

// ============================================================================
// OBSERVER SYSTEM
// ============================================================================

/**
 * Adjust fire based on observer correction
 */
export function adjustFire(
  mission: FireMission,
  correction: {
    direction: 'left' | 'right' | 'add' | 'drop';
    meters: number;
  }
): void {
  const target = mission.bombardment.targetArea.center;

  switch (correction.direction) {
    case 'left':
      target.x -= correction.meters;
      break;
    case 'right':
      target.x += correction.meters;
      break;
    case 'add':
      target.z += correction.meters;
      break;
    case 'drop':
      target.z -= correction.meters;
      break;
  }
}

/**
 * Calculate observer accuracy based on conditions
 */
export function calculateObserverAccuracy(
  observerPosition: Position3D,
  targetPosition: Position3D,
  visibility: number, // 0-100
  experience: 'novice' | 'experienced' | 'veteran'
): number {
  const distance = distance2D(observerPosition, targetPosition);

  // Base accuracy decreases with distance
  let accuracy = 100 - (distance / 100);

  // Visibility affects accuracy
  accuracy *= (visibility / 100);

  // Experience modifier
  const experienceModifier = {
    novice: 0.7,
    experienced: 0.85,
    veteran: 1.0
  };

  accuracy *= experienceModifier[experience];

  return Math.max(0, Math.min(100, accuracy));
}
