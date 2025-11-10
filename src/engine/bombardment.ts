/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Bombardment Pattern System
 *
 * Handles bombardment patterns: random, creeping barrage, box barrage, etc.
 */

import {
  Position3D,
  BombardmentConfig,
  BombardmentIntensity,
  BombardmentPattern,
  ShellType,
  ShellInFlight
} from './types';
import { createShellInFlight } from './ballistics';
import { VERDUN_HISTORICAL_SHELL_MIX } from './shells';

// ============================================================================
// BOMBARDMENT INTENSITY CONFIGURATIONS
// ============================================================================

export const INTENSITY_CONFIGS = {
  [BombardmentIntensity.HARASSING_FIRE]: {
    shellsPerMinute: 2,
    duration: 3600, // 1 hour typical
    casualtiesPerHourPer1000: 7,
    description: "Sporadic shells to prevent rest/movement"
  },
  [BombardmentIntensity.LIGHT]: {
    shellsPerMinute: 15,
    duration: 7200, // 2 hours typical
    casualtiesPerHourPer1000: 30,
    description: "Sustained shelling of sector"
  },
  [BombardmentIntensity.HEAVY]: {
    shellsPerMinute: 50,
    duration: 14400, // 4 hours typical
    casualtiesPerHourPer1000: 80,
    description: "Intense shelling, trench destruction"
  },
  [BombardmentIntensity.DRUMFIRE]: {
    shellsPerMinute: 120,
    duration: 28800, // 8 hours typical
    casualtiesPerHourPer1000: 175,
    description: "Trommelfeuer - continuous roar"
  }
};

// ============================================================================
// SHELL SELECTION
// ============================================================================

/**
 * Select random shell type based on mix percentages
 */
export function selectShellType(shellMix: { [key in ShellType]?: number }): ShellType {
  const types = Object.keys(shellMix) as ShellType[];
  const weights = types.map(t => shellMix[t] || 0);
  const totalWeight = weights.reduce((a, b) => a + b, 0);

  let random = Math.random() * totalWeight;
  for (let i = 0; i < types.length; i++) {
    random -= weights[i];
    if (random <= 0) {
      return types[i];
    }
  }

  return types[0]; // Fallback
}

/**
 * Get historical shell mix for Verdun
 */
export function getHistoricalShellMix(): { [key in ShellType]?: number } {
  return { ...VERDUN_HISTORICAL_SHELL_MIX };
}

/**
 * Create custom shell mix
 */
export function createShellMix(
  lightPercent: number = 60,
  mediumPercent: number = 25,
  heavyPercent: number = 10,
  superHeavyPercent: number = 5
): { [key in ShellType]?: number } {
  return {
    [ShellType.FRENCH_75MM]: lightPercent * 0.6,
    [ShellType.GERMAN_77MM]: lightPercent * 0.4,
    [ShellType.HOWITZER_155MM]: mediumPercent,
    [ShellType.GERMAN_210MM]: heavyPercent * 0.5,
    [ShellType.FRENCH_305MM]: heavyPercent * 0.5,
    [ShellType.GERMAN_420MM]: superHeavyPercent
  };
}

// ============================================================================
// BOMBARDMENT PATTERN GENERATORS
// ============================================================================

/**
 * Generate random shell target positions within area
 */
export function generateRandomTargets(
  center: Position3D,
  radius: number,
  count: number
): Position3D[] {
  const targets: Position3D[] = [];

  for (let i = 0; i < count; i++) {
    // Random position within circle
    const angle = Math.random() * Math.PI * 2;
    const distance = Math.sqrt(Math.random()) * radius;

    targets.push({
      x: center.x + Math.cos(angle) * distance,
      y: center.y,
      z: center.z + Math.sin(angle) * distance
    });
  }

  return targets;
}

/**
 * Generate creeping barrage targets
 * Artillery advances in a line ahead of infantry
 */
export function generateCreepingBarrageTargets(
  startLine: Position3D,
  endLine: Position3D,
  width: number,
  advanceRate: number, // meters per minute
  duration: number, // seconds
  shellsPerMinute: number
): Position3D[] {
  const targets: Position3D[] = [];

  const totalDistance = Math.sqrt(
    Math.pow(endLine.x - startLine.x, 2) +
    Math.pow(endLine.z - startLine.z, 2)
  );

  const steps = Math.floor(duration / 60); // Number of minute intervals
  const distancePerStep = advanceRate;

  for (let step = 0; step < steps; step++) {
    const progress = (step * distancePerStep) / totalDistance;
    if (progress > 1) break;

    // Current barrage line position
    const lineX = startLine.x + (endLine.x - startLine.x) * progress;
    const lineZ = startLine.z + (endLine.z - startLine.z) * progress;

    // Generate shells along the line width
    const shellsThisMinute = shellsPerMinute;
    for (let i = 0; i < shellsThisMinute; i++) {
      const offset = (Math.random() - 0.5) * width;

      // Perpendicular offset
      const angle = Math.atan2(endLine.z - startLine.z, endLine.x - startLine.x);
      const perpAngle = angle + Math.PI / 2;

      targets.push({
        x: lineX + Math.cos(perpAngle) * offset,
        y: startLine.y,
        z: lineZ + Math.sin(perpAngle) * offset
      });
    }
  }

  return targets;
}

/**
 * Generate box barrage targets
 * Creates a rectangular "box" of artillery fire to trap enemy
 */
export function generateBoxBarrageTargets(
  center: Position3D,
  width: number,
  length: number,
  shellsPerMinute: number,
  duration: number // seconds
): Position3D[] {
  const targets: Position3D[] = [];

  const totalShells = Math.floor((duration / 60) * shellsPerMinute);
  const shellsPerSide = Math.floor(totalShells / 4);

  // Four sides of the box
  const halfWidth = width / 2;
  const halfLength = length / 2;

  // North side
  for (let i = 0; i < shellsPerSide; i++) {
    targets.push({
      x: center.x + (Math.random() - 0.5) * width,
      y: center.y,
      z: center.z + halfLength + (Math.random() - 0.5) * 20
    });
  }

  // South side
  for (let i = 0; i < shellsPerSide; i++) {
    targets.push({
      x: center.x + (Math.random() - 0.5) * width,
      y: center.y,
      z: center.z - halfLength + (Math.random() - 0.5) * 20
    });
  }

  // East side
  for (let i = 0; i < shellsPerSide; i++) {
    targets.push({
      x: center.x + halfWidth + (Math.random() - 0.5) * 20,
      y: center.y,
      z: center.z + (Math.random() - 0.5) * length
    });
  }

  // West side
  for (let i = 0; i < shellsPerSide; i++) {
    targets.push({
      x: center.x - halfWidth + (Math.random() - 0.5) * 20,
      y: center.y,
      z: center.z + (Math.random() - 0.5) * length
    });
  }

  return targets;
}

/**
 * Generate concentration fire targets
 * All shells aimed at single point (for fort bombardment)
 */
export function generateConcentrationTargets(
  target: Position3D,
  shellsPerMinute: number,
  duration: number, // seconds
  dispersionRadius: number = 25 // Natural dispersion
): Position3D[] {
  const targets: Position3D[] = [];
  const totalShells = Math.floor((duration / 60) * shellsPerMinute);

  for (let i = 0; i < totalShells; i++) {
    // Add natural dispersion
    const angle = Math.random() * Math.PI * 2;
    const distance = Math.random() * dispersionRadius;

    targets.push({
      x: target.x + Math.cos(angle) * distance,
      y: target.y,
      z: target.z + Math.sin(angle) * distance
    });
  }

  return targets;
}

/**
 * Generate counter-battery fire targets
 * Target enemy artillery positions
 */
export function generateCounterBatteryTargets(
  batteryPositions: Position3D[],
  shellsPerMinute: number,
  duration: number // seconds
): Position3D[] {
  const targets: Position3D[] = [];
  const totalShells = Math.floor((duration / 60) * shellsPerMinute);
  const shellsPerBattery = Math.floor(totalShells / batteryPositions.length);

  for (const batteryPos of batteryPositions) {
    // Distribute shells around battery position
    for (let i = 0; i < shellsPerBattery; i++) {
      const angle = Math.random() * Math.PI * 2;
      const distance = Math.random() * 50; // 50m dispersion

      targets.push({
        x: batteryPos.x + Math.cos(angle) * distance,
        y: batteryPos.y,
        z: batteryPos.z + Math.sin(angle) * distance
      });
    }
  }

  return targets;
}

// ============================================================================
// BOMBARDMENT EXECUTION
// ============================================================================

/**
 * Create bombardment schedule
 * Returns array of shell impacts with timing
 */
export function createBombardmentSchedule(
  config: BombardmentConfig,
  batteryPosition: Position3D,
  startTime: number
): Array<{
  shellType: ShellType;
  target: Position3D;
  launchTime: number;
}> {
  const schedule: Array<{
    shellType: ShellType;
    target: Position3D;
    launchTime: number;
  }> = [];

  // Generate targets based on pattern
  let targets: Position3D[] = [];

  switch (config.pattern) {
    case BombardmentPattern.RANDOM:
      targets = generateRandomTargets(
        config.targetArea.center,
        config.targetArea.radius,
        Math.floor((config.duration / 60) * config.shellsPerMinute)
      );
      break;

    case BombardmentPattern.CREEPING_BARRAGE:
      if (!config.targetArea.length || !config.targetArea.width) {
        throw new Error("Creeping barrage requires length and width");
      }
      const endPoint: Position3D = {
        x: config.targetArea.center.x + config.targetArea.length,
        y: config.targetArea.center.y,
        z: config.targetArea.center.z
      };
      targets = generateCreepingBarrageTargets(
        config.targetArea.center,
        endPoint,
        config.targetArea.width,
        50, // 50m per minute advance rate
        config.duration,
        config.shellsPerMinute
      );
      break;

    case BombardmentPattern.BOX_BARRAGE:
      if (!config.targetArea.length || !config.targetArea.width) {
        throw new Error("Box barrage requires length and width");
      }
      targets = generateBoxBarrageTargets(
        config.targetArea.center,
        config.targetArea.width,
        config.targetArea.length,
        config.shellsPerMinute,
        config.duration
      );
      break;

    case BombardmentPattern.CONCENTRATION:
      targets = generateConcentrationTargets(
        config.targetArea.center,
        config.shellsPerMinute,
        config.duration,
        25
      );
      break;

    default:
      targets = generateRandomTargets(
        config.targetArea.center,
        config.targetArea.radius,
        Math.floor((config.duration / 60) * config.shellsPerMinute)
      );
  }

  // Create schedule with timing
  const timeInterval = 60 / config.shellsPerMinute; // seconds between shells

  targets.forEach((target, index) => {
    schedule.push({
      shellType: selectShellType(config.shellMix),
      target,
      launchTime: startTime + index * timeInterval
    });
  });

  return schedule;
}

/**
 * Execute bombardment step
 * Returns shells to be launched this tick
 */
export function executeBombardmentStep(
  schedule: Array<{
    shellType: ShellType;
    target: Position3D;
    launchTime: number;
  }>,
  currentTime: number,
  batteryPosition: Position3D,
  batteryId: string
): ShellInFlight[] {
  const shellsToLaunch: ShellInFlight[] = [];

  for (const scheduled of schedule) {
    if (scheduled.launchTime <= currentTime && scheduled.launchTime > currentTime - 0.1) {
      const shell = createShellInFlight(
        `shell_${currentTime}_${Math.random()}`,
        scheduled.shellType,
        batteryPosition,
        scheduled.target,
        scheduled.launchTime,
        batteryId,
        85 // Accuracy
      );

      if (shell) {
        shellsToLaunch.push(shell);
      }
    }
  }

  return shellsToLaunch;
}

/**
 * Create standard bombardment configurations
 */
export function createStandardBombardment(
  intensity: BombardmentIntensity,
  pattern: BombardmentPattern,
  targetCenter: Position3D,
  targetRadius: number = 500
): BombardmentConfig {
  const intensityConfig = INTENSITY_CONFIGS[intensity];

  return {
    intensity,
    pattern,
    duration: intensityConfig.duration,
    targetArea: {
      center: targetCenter,
      radius: targetRadius
    },
    shellMix: getHistoricalShellMix(),
    shellsPerMinute: intensityConfig.shellsPerMinute,
    casualtiesPerHourPer1000: intensityConfig.casualtiesPerHourPer1000
  };
}

/**
 * Historical bombardments from Verdun
 */
export const HISTORICAL_BOMBARDMENTS = {
  /**
   * February 21, 1916 - Opening bombardment
   * 1 million shells in 12 hours
   */
  FEBRUARY_21_OPENING: {
    intensity: BombardmentIntensity.DRUMFIRE,
    pattern: BombardmentPattern.RANDOM,
    duration: 32400, // 9 hours
    shellsPerMinute: 170,
    description: "Opening bombardment of Battle of Verdun",
    shellMix: {
      [ShellType.GERMAN_77MM]: 30,
      [ShellType.HOWITZER_155MM]: 25,
      [ShellType.GERMAN_210MM]: 20,
      [ShellType.GERMAN_420MM]: 15,
      [ShellType.PHOSGENE_GAS]: 10
    }
  },

  /**
   * October 20-23, 1916 - French counteroffensive
   * 1.15 million shells before Douaumont recapture
   */
  OCTOBER_DOUAUMONT: {
    intensity: BombardmentIntensity.DRUMFIRE,
    pattern: BombardmentPattern.CREEPING_BARRAGE,
    duration: 259200, // 3 days
    shellsPerMinute: 150,
    description: "Preparation for Douaumont recapture",
    shellMix: {
      [ShellType.FRENCH_75MM]: 40,
      [ShellType.HOWITZER_155MM]: 30,
      [ShellType.FRENCH_305MM]: 20,
      [ShellType.PHOSGENE_GAS]: 10
    }
  },

  /**
   * Typical harassing fire
   */
  HARASSING_FIRE: {
    intensity: BombardmentIntensity.HARASSING_FIRE,
    pattern: BombardmentPattern.RANDOM,
    duration: 86400, // 24 hours
    shellsPerMinute: 2,
    description: "Constant harassment to prevent rest",
    shellMix: getHistoricalShellMix()
  }
};

/**
 * Calculate expected casualties from bombardment
 */
export function calculateExpectedCasualties(
  config: BombardmentConfig,
  troopsInArea: number
): {
  killed: number;
  wounded: number;
  shellShocked: number;
} {
  const hours = config.duration / 3600;
  const casualtyRate = config.casualtiesPerHourPer1000;

  const totalCasualties = (troopsInArea / 1000) * casualtyRate * hours;

  // Historical breakdown: ~30% killed, 50% wounded, 20% shell shocked
  return {
    killed: Math.round(totalCasualties * 0.3),
    wounded: Math.round(totalCasualties * 0.5),
    shellShocked: Math.round(totalCasualties * 0.2)
  };
}
