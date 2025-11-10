/**
 * KING OF BATTLE - ARTILLERY WEATHER SYSTEM
 *
 * Artillery as an Environmental Hazard
 *
 * This system treats artillery like weather - it ebbs and flows automatically.
 * The player doesn't control it, they SURVIVE it.
 *
 * Like a storm system:
 * - Intensity rises and falls
 * - Never truly stops (harassing fire is constant)
 * - Unpredictable surges
 * - Historical patterns (Feb 21 = hurricane, July = light rain)
 */

import {
  ArtilleryEngine,
  createArtilleryEngine,
  createArtilleryBattery,
  createFireMission,
  assignBatteryToMission
} from './artilleryEngine';
import {
  ShellType,
  BombardmentIntensity,
  FireMissionType,
  Position3D
} from './types';

// ============================================================================
// ARTILLERY WEATHER CONFIGURATION
// ============================================================================

export interface ArtilleryWeatherConfig {
  // Area being bombarded
  targetArea: {
    center: Position3D;
    radius: number;
  };

  // Base intensity (always present)
  baseIntensity: BombardmentIntensity;

  // Storm system
  stormsEnabled: boolean;
  stormMinInterval: number;      // seconds between storms
  stormMaxInterval: number;
  stormMinDuration: number;      // how long storms last
  stormMaxDuration: number;

  // Historical accuracy
  useHistoricalCurve: boolean;   // Follow actual Verdun intensity timeline
  historicalDate: Date;          // What date in battle (Feb 21 - Dec 18, 1916)

  // Randomness
  randomFluctuations: boolean;   // Add unpredictable surges
  fluctuationChance: number;     // % chance per minute
}

interface StormEvent {
  startTime: number;
  endTime: number;
  intensity: BombardmentIntensity;
  active: boolean;
}

// ============================================================================
// ARTILLERY WEATHER SYSTEM
// ============================================================================

export class ArtilleryWeatherSystem {
  private engine: ArtilleryEngine;
  private config: ArtilleryWeatherConfig;
  private currentStorm: StormEvent | null = null;
  private nextStormTime: number = 0;
  private germanBatteries: string[] = [];
  private currentIntensity: BombardmentIntensity;
  private timeSinceLastFluctuation: number = 0;

  constructor(engine: ArtilleryEngine, config: ArtilleryWeatherConfig) {
    this.engine = engine;
    this.config = config;
    this.currentIntensity = config.baseIntensity;

    this.initializeBatteries();
    this.scheduleNextStorm();
    this.startBaseBombardment();
  }

  /**
   * Initialize German artillery batteries that will bombard French positions
   */
  private initializeBatteries(): void {
    // Create multiple German batteries at various ranges
    const batteryConfigs = [
      // Light artillery (close range, high rate of fire)
      { type: ShellType.GERMAN_77MM, count: 6, distance: 3000, id: "German_77mm_Bty_1" },
      { type: ShellType.GERMAN_77MM, count: 6, distance: 3500, id: "German_77mm_Bty_2" },
      { type: ShellType.GERMAN_77MM, count: 6, distance: 4000, id: "German_77mm_Bty_3" },

      // Medium artillery
      { type: ShellType.HOWITZER_155MM, count: 4, distance: 6000, id: "German_155mm_Bty_1" },
      { type: ShellType.HOWITZER_155MM, count: 4, distance: 7000, id: "German_155mm_Bty_2" },

      // Heavy artillery
      { type: ShellType.GERMAN_210MM, count: 2, distance: 9000, id: "German_210mm_Bty_1" },

      // Super-heavy (rare, terrifying)
      { type: ShellType.GERMAN_420MM, count: 1, distance: 12000, id: "German_420mm" }
    ];

    for (const config of batteryConfigs) {
      const angle = Math.random() * Math.PI * 2;
      const battery = createArtilleryBattery(
        config.id,
        "german",
        config.type,
        config.count,
        {
          x: this.config.targetArea.center.x + Math.cos(angle) * config.distance,
          y: 0,
          z: this.config.targetArea.center.z + Math.sin(angle) * config.distance
        },
        this.getBatteryRange(config.type)
      );

      this.engine.addBattery(battery);
      this.germanBatteries.push(battery.id);
    }
  }

  private getBatteryRange(type: ShellType): number {
    const ranges = {
      [ShellType.GERMAN_77MM]: 7800,
      [ShellType.HOWITZER_155MM]: 12000,
      [ShellType.GERMAN_210MM]: 11000,
      [ShellType.GERMAN_420MM]: 14000
    };
    return ranges[type] || 10000;
  }

  /**
   * Start the base (constant) bombardment
   * This never stops - harassing fire is 24/7
   */
  private startBaseBombardment(): void {
    this.createBombardmentMission(this.config.baseIntensity, Infinity);
  }

  /**
   * Create a bombardment mission at given intensity
   */
  private createBombardmentMission(
    intensity: BombardmentIntensity,
    duration: number
  ): void {
    const mission = createFireMission(
      FireMissionType.HARASSING,
      this.config.targetArea.center,
      "French positions",
      3,
      duration,
      intensity
    );

    // Assign some batteries to this mission
    const batteriesToAssign = this.selectBatteriesForIntensity(intensity);
    const batteries = this.engine.getState().batteries;

    for (const batteryId of batteriesToAssign) {
      const battery = batteries.get(batteryId);
      if (battery) {
        assignBatteryToMission(battery, mission);
      }
    }

    this.engine.startMission(mission);
  }

  /**
   * Select which batteries fire based on intensity
   */
  private selectBatteriesForIntensity(intensity: BombardmentIntensity): string[] {
    switch (intensity) {
      case BombardmentIntensity.HARASSING_FIRE:
        // Just a few light guns
        return this.germanBatteries.slice(0, 2);

      case BombardmentIntensity.LIGHT:
        // Light artillery + some medium
        return this.germanBatteries.slice(0, 4);

      case BombardmentIntensity.HEAVY:
        // Everything except super-heavy
        return this.germanBatteries.slice(0, 6);

      case BombardmentIntensity.DRUMFIRE:
        // EVERYTHING including Big Bertha
        return [...this.germanBatteries];

      default:
        return this.germanBatteries.slice(0, 2);
    }
  }

  /**
   * Schedule when next artillery storm will occur
   */
  private scheduleNextStorm(): void {
    if (!this.config.stormsEnabled) return;

    const currentTime = this.engine.getState().currentTime;
    const interval = this.randomBetween(
      this.config.stormMinInterval,
      this.config.stormMaxInterval
    );

    this.nextStormTime = currentTime + interval;
  }

  /**
   * Check if a storm should start
   */
  private checkForStormStart(): void {
    if (!this.config.stormsEnabled) return;
    if (this.currentStorm?.active) return;

    const currentTime = this.engine.getState().currentTime;

    if (currentTime >= this.nextStormTime) {
      this.startStorm();
    }
  }

  /**
   * Start an artillery storm
   */
  private startStorm(): void {
    const currentTime = this.engine.getState().currentTime;
    const duration = this.randomBetween(
      this.config.stormMinDuration,
      this.config.stormMaxDuration
    );

    // Storms are HEAVY or DRUMFIRE intensity
    const stormIntensity = Math.random() > 0.5
      ? BombardmentIntensity.HEAVY
      : BombardmentIntensity.DRUMFIRE;

    this.currentStorm = {
      startTime: currentTime,
      endTime: currentTime + duration,
      intensity: stormIntensity,
      active: true
    };

    this.currentIntensity = stormIntensity;
    this.createBombardmentMission(stormIntensity, duration);

    console.log(`[Artillery Weather] STORM INCOMING! Intensity: ${stormIntensity}, Duration: ${duration}s`);
  }

  /**
   * Check if storm should end
   */
  private checkForStormEnd(): void {
    if (!this.currentStorm?.active) return;

    const currentTime = this.engine.getState().currentTime;

    if (currentTime >= this.currentStorm.endTime) {
      this.endStorm();
    }
  }

  /**
   * End the current storm
   */
  private endStorm(): void {
    console.log(`[Artillery Weather] Storm passing... returning to base intensity`);

    this.currentStorm = null;
    this.currentIntensity = this.config.baseIntensity;
    this.scheduleNextStorm();
  }

  /**
   * Random fluctuations - sudden surges in fire
   */
  private checkForFluctuations(deltaTime: number): void {
    if (!this.config.randomFluctuations) return;
    if (this.currentStorm?.active) return; // Don't fluctuate during storms

    this.timeSinceLastFluctuation += deltaTime;

    // Check every minute
    if (this.timeSinceLastFluctuation >= 60) {
      this.timeSinceLastFluctuation = 0;

      if (Math.random() * 100 < this.config.fluctuationChance) {
        // Temporary surge in fire
        console.log(`[Artillery Weather] Sudden surge in fire!`);
        this.createBombardmentMission(
          BombardmentIntensity.LIGHT,
          this.randomBetween(30, 120) // 30s to 2min surge
        );
      }
    }
  }

  /**
   * Get current intensity based on historical timeline
   */
  private getHistoricalIntensity(): BombardmentIntensity {
    if (!this.config.useHistoricalCurve) {
      return this.currentIntensity;
    }

    // Map historical dates to intensity levels
    const battleStart = new Date('1916-02-21');
    const daysSinceBattleStart = Math.floor(
      (this.config.historicalDate.getTime() - battleStart.getTime()) / (1000 * 60 * 60 * 24)
    );

    // Historical intensity curve
    if (daysSinceBattleStart === 0) {
      // Feb 21 - Opening bombardment
      return BombardmentIntensity.DRUMFIRE;
    } else if (daysSinceBattleStart < 7) {
      // First week - very heavy
      return BombardmentIntensity.HEAVY;
    } else if (daysSinceBattleStart < 60) {
      // Feb-March - sustained heavy
      return BombardmentIntensity.HEAVY;
    } else if (daysSinceBattleStart < 120) {
      // April-May - medium
      return BombardmentIntensity.LIGHT;
    } else if (daysSinceBattleStart < 180) {
      // June-July - heavy again
      return BombardmentIntensity.HEAVY;
    } else if (daysSinceBattleStart < 240) {
      // Aug-Sept - medium
      return BombardmentIntensity.LIGHT;
    } else if (daysSinceBattleStart < 260) {
      // Oct 20-24 - DRUMFIRE (French counteroffensive)
      return BombardmentIntensity.DRUMFIRE;
    } else {
      // Late battle - declining
      return BombardmentIntensity.LIGHT;
    }
  }

  /**
   * Main update loop - called every frame
   */
  public update(deltaTime: number): void {
    this.checkForStormStart();
    this.checkForStormEnd();
    this.checkForFluctuations(deltaTime);

    // Update base intensity if using historical curve
    if (this.config.useHistoricalCurve && !this.currentStorm?.active) {
      const historicalIntensity = this.getHistoricalIntensity();
      if (historicalIntensity !== this.currentIntensity) {
        this.currentIntensity = historicalIntensity;
        this.createBombardmentMission(historicalIntensity, Infinity);
      }
    }
  }

  /**
   * Get current weather status
   */
  public getStatus(): {
    currentIntensity: BombardmentIntensity;
    isStormActive: boolean;
    stormTimeRemaining?: number;
    nextStormIn?: number;
  } {
    const currentTime = this.engine.getState().currentTime;

    return {
      currentIntensity: this.currentIntensity,
      isStormActive: this.currentStorm?.active || false,
      stormTimeRemaining: this.currentStorm?.active
        ? this.currentStorm.endTime - currentTime
        : undefined,
      nextStormIn: !this.currentStorm?.active
        ? this.nextStormTime - currentTime
        : undefined
    };
  }

  private randomBetween(min: number, max: number): number {
    return min + Math.random() * (max - min);
  }
}

// ============================================================================
// FACTORY FUNCTIONS
// ============================================================================

/**
 * Create a typical Verdun artillery weather system
 */
export function createVerdunArtilleryWeather(
  playerArea: Position3D,
  historicalDate?: Date
): ArtilleryWeatherSystem {
  const engine = createArtilleryEngine({
    historicalShellMix: true,
    historicalCasualtyRates: true
  });

  const config: ArtilleryWeatherConfig = {
    targetArea: {
      center: playerArea,
      radius: 500 // 500m radius around player area
    },
    baseIntensity: BombardmentIntensity.HARASSING_FIRE,
    stormsEnabled: true,
    stormMinInterval: 600,      // 10 minutes min
    stormMaxInterval: 1800,     // 30 minutes max
    stormMinDuration: 300,      // 5 minutes min
    stormMaxDuration: 1800,     // 30 minutes max
    useHistoricalCurve: !!historicalDate,
    historicalDate: historicalDate || new Date('1916-02-21'),
    randomFluctuations: true,
    fluctuationChance: 20 // 20% chance per minute
  };

  const weather = new ArtilleryWeatherSystem(engine, config);

  return weather;
}

/**
 * Create February 21, 1916 scenario (apocalyptic)
 */
export function createFebruary21Weather(playerArea: Position3D): ArtilleryWeatherSystem {
  const engine = createArtilleryEngine({
    historicalShellMix: true,
    historicalCasualtyRates: true
  });

  const config: ArtilleryWeatherConfig = {
    targetArea: {
      center: playerArea,
      radius: 1000
    },
    baseIntensity: BombardmentIntensity.DRUMFIRE, // Constant drumfire
    stormsEnabled: false, // Already at max
    stormMinInterval: 0,
    stormMaxInterval: 0,
    stormMinDuration: 0,
    stormMaxDuration: 0,
    useHistoricalCurve: true,
    historicalDate: new Date('1916-02-21'),
    randomFluctuations: false, // Already apocalyptic
    fluctuationChance: 0
  };

  return new ArtilleryWeatherSystem(engine, config);
}

/**
 * Create quiet period scenario (relatively safe)
 */
export function createQuietPeriodWeather(playerArea: Position3D): ArtilleryWeatherSystem {
  const engine = createArtilleryEngine();

  const config: ArtilleryWeatherConfig = {
    targetArea: {
      center: playerArea,
      radius: 500
    },
    baseIntensity: BombardmentIntensity.HARASSING_FIRE,
    stormsEnabled: true,
    stormMinInterval: 1800,     // Rare storms
    stormMaxInterval: 3600,
    stormMinDuration: 180,      // Short storms
    stormMaxDuration: 600,
    useHistoricalCurve: false,
    historicalDate: new Date('1916-02-21'),
    randomFluctuations: true,
    fluctuationChance: 10 // Less frequent
  };

  return new ArtilleryWeatherSystem(engine, config);
}
