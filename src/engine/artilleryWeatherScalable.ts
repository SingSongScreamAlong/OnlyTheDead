/**
 * KING OF BATTLE - SCALABLE ARTILLERY WEATHER SYSTEM
 *
 * Integrates scalable artillery manager with weather system
 * Handles FULL Verdun intensity (February 21 = 23 shells per second)
 */

import {
  ScalableArtilleryManager,
  createScalableManager,
  calculateScaleConfig
} from './artilleryScale';
import {
  ArtilleryWeatherConfig,
  BombardmentIntensity,
  Position3D,
  ShellType
} from './types';
import { selectShellType } from './bombardment';
import { VERDUN_HISTORICAL_SHELL_MIX } from './shells';

// ============================================================================
// SCALABLE WEATHER SYSTEM
// ============================================================================

export class ScalableArtilleryWeather {
  private scaleManager: ScalableArtilleryManager;
  private config: ArtilleryWeatherConfig;
  private currentIntensity: BombardmentIntensity;
  private currentStorm: StormEvent | null = null;
  private nextStormTime: number = 0;
  private timeSinceLastFluctuation: number = 0;
  private currentTime: number = 0;

  // Battery positions (for shell origin calculation)
  private batteryPositions: Position3D[] = [];

  constructor(
    config: ArtilleryWeatherConfig,
    targetFPS: number = 60
  ) {
    this.config = config;
    this.currentIntensity = config.baseIntensity;

    // Create scalable manager with optimized config
    const scaleConfig = calculateScaleConfig(targetFPS, BombardmentIntensity.DRUMFIRE);
    this.scaleManager = createScalableManager(scaleConfig);

    this.initializeBatteryPositions();
    this.scheduleNextStorm();
  }

  /**
   * Initialize battery positions around target area
   */
  private initializeBatteryPositions(): void {
    const distances = [3000, 3500, 4000, 6000, 7000, 9000, 12000];

    for (const distance of distances) {
      const angle = Math.random() * Math.PI * 2;
      this.batteryPositions.push({
        x: this.config.targetArea.center.x + Math.cos(angle) * distance,
        y: 0,
        z: this.config.targetArea.center.z + Math.sin(angle) * distance
      });
    }
  }

  /**
   * Main update - handles shell generation and zone management
   */
  public update(deltaTime: number): void {
    this.currentTime += deltaTime;

    // Update scale manager
    this.scaleManager.update(this.currentTime, deltaTime);

    // Weather system logic
    this.checkForStormStart();
    this.checkForStormEnd();
    this.checkForFluctuations(deltaTime);

    // Generate shells based on current intensity
    this.generateShells(deltaTime);
  }

  /**
   * Generate shells based on current intensity
   * This is where the magic happens - automatic shell generation
   */
  private generateShells(deltaTime: number): void {
    const shellsPerSecond = this.getShellsPerSecond(this.currentIntensity);

    // Calculate how many shells to generate this frame
    const shellsThisFrame = shellsPerSecond * deltaTime;

    // Use probabilistic generation for fractional shells
    const wholeShells = Math.floor(shellsThisFrame);
    const fractional = shellsThisFrame - wholeShells;

    let totalShells = wholeShells;
    if (Math.random() < fractional) {
      totalShells++;
    }

    // Generate shells
    for (let i = 0; i < totalShells; i++) {
      this.generateSingleShell();
    }
  }

  /**
   * Generate a single shell
   */
  private generateSingleShell(): void {
    // Select shell type based on historical mix
    const shellType = selectShellType(VERDUN_HISTORICAL_SHELL_MIX);

    // Random target within area
    const angle = Math.random() * Math.PI * 2;
    const distance = Math.sqrt(Math.random()) * this.config.targetArea.radius;

    const target: Position3D = {
      x: this.config.targetArea.center.x + Math.cos(angle) * distance,
      y: 0,
      z: this.config.targetArea.center.z + Math.sin(angle) * distance
    };

    // Select random battery
    const battery = this.batteryPositions[
      Math.floor(Math.random() * this.batteryPositions.length)
    ];

    // Add to scalable manager (automatically sorted into zones)
    this.scaleManager.addIncomingShells(
      shellType,
      [target],
      this.currentTime,
      battery
    );
  }

  /**
   * Get shells per second for intensity level
   */
  private getShellsPerSecond(intensity: BombardmentIntensity): number {
    const rates = {
      [BombardmentIntensity.HARASSING_FIRE]: 0.05,  // ~3 per minute
      [BombardmentIntensity.LIGHT]: 0.25,            // ~15 per minute
      [BombardmentIntensity.HEAVY]: 0.83,            // ~50 per minute
      [BombardmentIntensity.DRUMFIRE]: 2.0           // ~120 per minute (realistic player experience)
    };

    return rates[intensity] || 0.05;
  }

  /**
   * Set player position (for zone calculation)
   */
  public setPlayerPosition(position: Position3D): void {
    this.scaleManager.setPlayerPosition(position);
  }

  /**
   * Get shells in detailed zone (for player detection)
   */
  public getDetailedShells() {
    return this.scaleManager.getDetailedShells();
  }

  /**
   * Get weather status
   */
  public getStatus() {
    const scaleStats = this.scaleManager.getStats();
    const backgroundStats = this.scaleManager.getBackgroundStats();

    return {
      currentIntensity: this.currentIntensity,
      isStormActive: this.currentStorm?.active || false,
      stormTimeRemaining: this.currentStorm?.active
        ? this.currentStorm.endTime - this.currentTime
        : undefined,
      nextStormIn: !this.currentStorm?.active
        ? this.nextStormTime - this.currentTime
        : undefined,

      // Scale statistics
      detailedShells: scaleStats.detailedShellsActive,
      simplifiedGroups: scaleStats.simplifiedGroupsActive,
      backgroundShellsPerSecond: backgroundStats?.shellsPerSecond || 0,
      totalShellsFired: scaleStats.totalShellsFired,
      performanceSavings: `${(scaleStats.performanceFactor * 100).toFixed(1)}%`
    };
  }

  /**
   * Get performance statistics
   */
  public getPerformanceStats() {
    return this.scaleManager.getStats();
  }

  // Storm system (same as before)
  private scheduleNextStorm(): void {
    if (!this.config.stormsEnabled) return;

    const interval = this.randomBetween(
      this.config.stormMinInterval,
      this.config.stormMaxInterval
    );

    this.nextStormTime = this.currentTime + interval;
  }

  private checkForStormStart(): void {
    if (!this.config.stormsEnabled) return;
    if (this.currentStorm?.active) return;

    if (this.currentTime >= this.nextStormTime) {
      this.startStorm();
    }
  }

  private startStorm(): void {
    const duration = this.randomBetween(
      this.config.stormMinDuration,
      this.config.stormMaxDuration
    );

    const stormIntensity = Math.random() > 0.5
      ? BombardmentIntensity.HEAVY
      : BombardmentIntensity.DRUMFIRE;

    this.currentStorm = {
      startTime: this.currentTime,
      endTime: this.currentTime + duration,
      intensity: stormIntensity,
      active: true
    };

    this.currentIntensity = stormIntensity;
    this.scaleManager.setIntensity(stormIntensity);

    console.log(`[Artillery Weather] Storm intensity: ${stormIntensity}`);
  }

  private checkForStormEnd(): void {
    if (!this.currentStorm?.active) return;

    if (this.currentTime >= this.currentStorm.endTime) {
      this.endStorm();
    }
  }

  private endStorm(): void {
    this.currentStorm = null;
    this.currentIntensity = this.config.baseIntensity;
    this.scaleManager.setIntensity(this.config.baseIntensity);
    this.scheduleNextStorm();
  }

  private checkForFluctuations(deltaTime: number): void {
    if (!this.config.randomFluctuations) return;
    if (this.currentStorm?.active) return;

    this.timeSinceLastFluctuation += deltaTime;

    if (this.timeSinceLastFluctuation >= 60) {
      this.timeSinceLastFluctuation = 0;

      if (Math.random() * 100 < this.config.fluctuationChance) {
        // Temporary surge
        const oldIntensity = this.currentIntensity;
        this.currentIntensity = BombardmentIntensity.LIGHT;

        setTimeout(() => {
          this.currentIntensity = oldIntensity;
        }, this.randomBetween(30, 120) * 1000);
      }
    }
  }

  private randomBetween(min: number, max: number): number {
    return min + Math.random() * (max - min);
  }
}

interface StormEvent {
  startTime: number;
  endTime: number;
  intensity: BombardmentIntensity;
  active: boolean;
}

// ============================================================================
// FACTORY FUNCTIONS FOR MASSIVE SCALE
// ============================================================================

/**
 * Create February 21, 1916 weather at FULL SCALE
 * Handles the actual 23 shells per second intensity
 */
export function createFebruary21ScalableWeather(
  playerArea: Position3D,
  targetFPS: number = 30  // Lower FPS target for drumfire
): ScalableArtilleryWeather {
  const config: ArtilleryWeatherConfig = {
    targetArea: {
      center: playerArea,
      radius: 2000  // 2km radius of death
    },
    baseIntensity: BombardmentIntensity.DRUMFIRE,
    stormsEnabled: false,  // Already at max
    stormMinInterval: 0,
    stormMaxInterval: 0,
    stormMinDuration: 0,
    stormMaxDuration: 0,
    useHistoricalCurve: true,
    historicalDate: new Date('1916-02-21'),
    randomFluctuations: false,
    fluctuationChance: 0
  };

  return new ScalableArtilleryWeather(config, targetFPS);
}

/**
 * Create typical Verdun weather with full scale support
 */
export function createScalableVerdunWeather(
  playerArea: Position3D,
  historicalDate?: Date,
  targetFPS: number = 60
): ScalableArtilleryWeather {
  const config: ArtilleryWeatherConfig = {
    targetArea: {
      center: playerArea,
      radius: 500
    },
    baseIntensity: BombardmentIntensity.HARASSING_FIRE,
    stormsEnabled: true,
    stormMinInterval: 600,
    stormMaxInterval: 1800,
    stormMinDuration: 300,
    stormMaxDuration: 1800,
    useHistoricalCurve: !!historicalDate,
    historicalDate: historicalDate || new Date('1916-02-21'),
    randomFluctuations: true,
    fluctuationChance: 20
  };

  return new ScalableArtilleryWeather(config, targetFPS);
}

/**
 * Create performance-optimized weather for low-end hardware
 */
export function createOptimizedWeather(
  playerArea: Position3D,
  targetFPS: number = 30
): ScalableArtilleryWeather {
  const config: ArtilleryWeatherConfig = {
    targetArea: {
      center: playerArea,
      radius: 300  // Smaller detailed zone
    },
    baseIntensity: BombardmentIntensity.LIGHT,
    stormsEnabled: true,
    stormMinInterval: 900,
    stormMaxInterval: 1800,
    stormMinDuration: 180,
    stormMaxDuration: 600,
    useHistoricalCurve: false,
    historicalDate: new Date('1916-02-21'),
    randomFluctuations: true,
    fluctuationChance: 15
  };

  return new ScalableArtilleryWeather(config, targetFPS);
}
