/**
 * KING OF BATTLE - IMPACT-DRIVEN WEATHER INTEGRATION
 *
 * Combines:
 * - Scalable artillery weather (generates bombardment)
 * - Impact-driven system (no visible projectiles)
 * - Delayed audio (speed of sound)
 * - Player awareness (threat detection)
 *
 * Flow:
 * 1. Weather system generates impact schedule
 * 2. Impacts happen (ground explodes)
 * 3. Sound arrives on delay
 * 4. Player hears/feels aftermath
 */

import {
  ImpactScheduler,
  AudioDelayManager,
  PlayerAwarenessSystem,
  ScheduledImpact,
  ImpactResult,
  ImpactThreat,
  createVerdunImpactScheduler,
  createAudioDelayManager,
  createPlayerAwareness
} from './impactDriven';
import {
  Position3D,
  ShellType,
  BombardmentIntensity,
  CoverType
} from './types';
import { selectShellType } from './bombardment';
import { VERDUN_HISTORICAL_SHELL_MIX } from './shells';

// ============================================================================
// IMPACT-DRIVEN WEATHER SYSTEM
// ============================================================================

export class ImpactDrivenWeather {
  private impactScheduler: ImpactScheduler;
  private audioManager: AudioDelayManager;
  private awarenessSystem: PlayerAwarenessSystem;

  private currentIntensity: BombardmentIntensity;
  private targetArea: {
    center: Position3D;
    radius: number;
  };

  private batteryPositions: Position3D[] = [];
  private currentTime: number = 0;

  // Storm system
  private stormActive: boolean = false;
  private nextStormTime: number = 0;
  private stormEndTime: number = 0;

  // Statistics
  private stats = {
    totalScheduled: 0,
    totalImpacted: 0,
    soundsPlayed: 0
  };

  constructor(
    targetArea: Position3D,
    targetRadius: number = 500,
    baseIntensity: BombardmentIntensity = BombardmentIntensity.HARASSING_FIRE
  ) {
    this.impactScheduler = createVerdunImpactScheduler();
    this.audioManager = createAudioDelayManager();
    this.awarenessSystem = createPlayerAwareness();

    this.currentIntensity = baseIntensity;
    this.targetArea = {
      center: targetArea,
      radius: targetRadius
    };

    this.initializeBatteries();
    this.scheduleNextStorm();
  }

  /**
   * Initialize battery positions
   */
  private initializeBatteries(): void {
    const distances = [3000, 3500, 4000, 6000, 7000, 9000, 12000];

    for (const distance of distances) {
      const angle = Math.random() * Math.PI * 2;
      this.batteryPositions.push({
        x: this.targetArea.center.x + Math.cos(angle) * distance,
        y: 0,
        z: this.targetArea.center.z + Math.sin(angle) * distance
      });
    }
  }

  /**
   * Main update loop
   */
  public update(deltaTime: number): {
    impacts: ImpactResult[];
    sounds: Array<{ position: Position3D; shellType: ShellType; volume: number; direction: Position3D }>;
    threats: ImpactThreat[];
  } {
    this.currentTime += deltaTime;

    // Weather system
    this.updateWeather();

    // Generate new impacts based on current intensity
    this.generateImpacts(deltaTime);

    // Process impacts that happen this frame
    const impacts = this.impactScheduler.update(deltaTime);
    this.stats.totalImpacted += impacts.length;

    // Schedule audio for each impact
    for (const impact of impacts) {
      this.audioManager.scheduleAudio(impact, this.currentTime);
    }

    // Get sounds that play this frame (delayed from impacts)
    const sounds = this.audioManager.update(deltaTime);
    this.stats.soundsPlayed += sounds.length;

    // Player threat awareness
    const scheduledImpacts = this.impactScheduler.getScheduledImpacts();
    const threats = this.awarenessSystem.assessThreats(scheduledImpacts, this.currentTime);

    return { impacts, sounds, threats };
  }

  /**
   * Update weather system (storms, fluctuations)
   */
  private updateWeather(): void {
    // Check for storm start
    if (!this.stormActive && this.currentTime >= this.nextStormTime) {
      this.startStorm();
    }

    // Check for storm end
    if (this.stormActive && this.currentTime >= this.stormEndTime) {
      this.endStorm();
    }
  }

  /**
   * Start artillery storm
   */
  private startStorm(): void {
    const duration = this.randomBetween(300, 1800); // 5-30 minutes

    const stormIntensity = Math.random() > 0.5
      ? BombardmentIntensity.HEAVY
      : BombardmentIntensity.DRUMFIRE;

    this.stormActive = true;
    this.stormEndTime = this.currentTime + duration;
    this.currentIntensity = stormIntensity;

    console.log(`[Impact Weather] Storm! Intensity: ${stormIntensity}, Duration: ${duration}s`);
  }

  /**
   * End artillery storm
   */
  private endStorm(): void {
    this.stormActive = false;
    this.currentIntensity = BombardmentIntensity.HARASSING_FIRE;
    this.scheduleNextStorm();
  }

  /**
   * Schedule next storm
   */
  private scheduleNextStorm(): void {
    const interval = this.randomBetween(600, 1800); // 10-30 minutes
    this.nextStormTime = this.currentTime + interval;
  }

  /**
   * Generate impacts based on current intensity
   */
  private generateImpacts(deltaTime: number): void {
    const shellsPerSecond = this.getShellsPerSecond(this.currentIntensity);

    // Calculate impacts to schedule this frame
    const impactsThisFrame = shellsPerSecond * deltaTime;

    // Probabilistic generation
    const wholeImpacts = Math.floor(impactsThisFrame);
    const fractional = impactsThisFrame - wholeImpacts;

    let totalImpacts = wholeImpacts;
    if (Math.random() < fractional) {
      totalImpacts++;
    }

    // Schedule each impact
    for (let i = 0; i < totalImpacts; i++) {
      this.scheduleRandomImpact();
    }
  }

  /**
   * Schedule a single random impact
   */
  private scheduleRandomImpact(): void {
    // Select shell type
    const shellType = selectShellType(VERDUN_HISTORICAL_SHELL_MIX);

    // Random target within area
    const angle = Math.random() * Math.PI * 2;
    const distance = Math.sqrt(Math.random()) * this.targetArea.radius;

    const target: Position3D = {
      x: this.targetArea.center.x + Math.cos(angle) * distance,
      y: 0,
      z: this.targetArea.center.z + Math.sin(angle) * distance
    };

    // Random battery
    const battery = this.batteryPositions[
      Math.floor(Math.random() * this.batteryPositions.length)
    ];

    // Schedule impact (not projectile!)
    this.impactScheduler.scheduleImpact(
      shellType,
      target,
      battery,
      this.currentTime
    );

    this.stats.totalScheduled++;
  }

  /**
   * Get shells per second for intensity
   */
  private getShellsPerSecond(intensity: BombardmentIntensity): number {
    const rates = {
      [BombardmentIntensity.HARASSING_FIRE]: 0.05,
      [BombardmentIntensity.LIGHT]: 0.25,
      [BombardmentIntensity.HEAVY]: 0.83,
      [BombardmentIntensity.DRUMFIRE]: 2.0
    };

    return rates[intensity] || 0.05;
  }

  /**
   * Set player position
   */
  public setPlayerPosition(position: Position3D): void {
    this.impactScheduler.setPlayerPosition(position);
    this.awarenessSystem.setPlayerPosition(position);
  }

  /**
   * Set player skill level (affects threat recognition)
   */
  public setPlayerSkill(skill: number): void {
    this.awarenessSystem.setSkillLevel(skill);
  }

  /**
   * Get weather status
   */
  public getStatus() {
    return {
      currentIntensity: this.currentIntensity,
      stormActive: this.stormActive,
      stormTimeRemaining: this.stormActive ? this.stormEndTime - this.currentTime : undefined,
      nextStormIn: !this.stormActive ? this.nextStormTime - this.currentTime : undefined,
      ...this.stats,
      ...this.impactScheduler.getStats()
    };
  }

  /**
   * Get upcoming sounds (for pre-cue system)
   */
  public getUpcomingSounds(lookaheadSeconds: number = 0.5) {
    return this.audioManager.getUpcomingSounds(lookaheadSeconds);
  }

  private randomBetween(min: number, max: number): number {
    return min + Math.random() * (max - min);
  }
}

// ============================================================================
// GAME INTEGRATION EXAMPLE
// ============================================================================

/**
 * Example showing how to integrate with game engine
 */
export class ArtilleryGameIntegration {
  private weather: ImpactDrivenWeather;
  private playerPosition: Position3D = { x: 0, y: 0, z: 0 };
  private playerCover: CoverType = CoverType.SHALLOW_TRENCH;

  constructor() {
    this.weather = new ImpactDrivenWeather(
      { x: 0, y: 0, z: 0 },
      500,
      BombardmentIntensity.HARASSING_FIRE
    );

    this.weather.setPlayerPosition(this.playerPosition);
  }

  /**
   * Game update loop
   */
  public update(deltaTime: number): void {
    // Update weather system
    const { impacts, sounds, threats } = this.weather.update(deltaTime);

    // Process impacts (ground explodes NOW)
    for (const impact of impacts) {
      this.handleImpact(impact);
    }

    // Process sounds (delayed audio)
    for (const sound of sounds) {
      this.playDelayedSound(sound);
    }

    // Process threats (player awareness)
    for (const threat of threats) {
      this.handleThreat(threat);
    }

    // Pre-cue for upcoming sounds (rumble before the boom)
    const upcoming = this.weather.getUpcomingSounds(0.3);
    for (const preCue of upcoming) {
      if (preCue.timeUntil < 0.1) {
        this.playPreRumble(preCue.position);
      }
    }
  }

  /**
   * Handle impact (visual + physical)
   */
  private handleImpact(impact: ImpactResult): void {
    // 1. Spawn explosion VFX
    this.spawnExplosion(impact.position, impact.shellType);

    // 2. Create crater (terrain deformation)
    if (impact.craterDiameter > 2) {
      this.createCrater(impact.position, impact.craterDiameter, impact.craterDepth);
    }

    // 3. Apply gameplay effects (damage, suppression)
    this.applyBlastEffects(impact);

    // 4. Camera shake
    this.applyCameraShake(impact);

    // 5. Show rare visual streak (if applicable)
    if (impact.showStreak) {
      // Brief (1-2 frame) streak effect
      // Only shown for very close small caliber
    }

    // NO SOUND YET - sound arrives later based on distance
  }

  /**
   * Play delayed sound (arrives after impact)
   */
  private playDelayedSound(sound: {
    position: Position3D;
    shellType: ShellType;
    volume: number;
    direction: Position3D;
  }): void {
    // This is the "boom" you hear AFTER seeing the explosion
    // Play 3D positional audio
    // Use sound.direction for panning
    // Use sound.volume for attenuation

    console.log(`BOOM at ${sound.position.x}, ${sound.position.z} (volume: ${sound.volume})`);

    // Example: game engine audio call
    // AudioEngine.Play3DSound(
    //   "explosion_" + sound.shellType,
    //   sound.position,
    //   sound.volume,
    //   sound.direction
    // );
  }

  /**
   * Handle player threat awareness
   */
  private handleThreat(threat: ImpactThreat): void {
    if (threat.threatLevel === 'extreme' && threat.isRecognized) {
      // Show warning UI
      console.log(`⚠️ DANGER! ${threat.recommendedAction}`);
      // UI.ShowWarning(threat.recommendedAction);
    }

    if (threat.threatLevel === 'high' && threat.timeUntilImpact < 5) {
      // Audio cue (distant incoming sound)
      // This is BEFORE impact, BEFORE explosion sound
      // Just a faint whistle or roar if player is skilled
    }
  }

  /**
   * Play pre-rumble (low-freq cue before explosion sound)
   */
  private playPreRumble(position: Position3D): void {
    // Low-frequency rumble 100-300ms before main explosion sound
    // Sells pressure wave and dread
    // AudioEngine.PlayLFE("pre_rumble", 0.3);
  }

  /**
   * Spawn explosion VFX
   */
  private spawnExplosion(position: Position3D, shellType: ShellType): void {
    // Niagara system
    // Size based on shell type
    // Ground interaction (mud spray, dirt)
    console.log(`💥 Explosion at ${position.x}, ${position.z} (${shellType})`);
  }

  /**
   * Create crater (terrain deformation)
   */
  private createCrater(position: Position3D, diameter: number, depth: number): void {
    // GPU-based heightfield stamp
    // Time-sliced to avoid hitches
    // Queue for navmesh rebuild
    console.log(`🕳️  Crater ${diameter}m at ${position.x}, ${position.z}`);
  }

  /**
   * Apply blast effects (damage, suppression)
   */
  private applyBlastEffects(impact: ImpactResult): void {
    // Query entities in blast radius
    // Apply damage based on distance
    // Apply suppression
    // Check player distance and cover
  }

  /**
   * Apply camera shake
   */
  private applyCameraShake(impact: ImpactResult): void {
    const distanceToPlayer = Math.sqrt(
      Math.pow(impact.position.x - this.playerPosition.x, 2) +
      Math.pow(impact.position.z - this.playerPosition.z, 2)
    );

    if (distanceToPlayer < impact.blastRadius * 2) {
      // Camera shake intensity based on distance
      const intensity = 1 - (distanceToPlayer / (impact.blastRadius * 2));
      console.log(`📷 Camera shake: ${intensity.toFixed(2)}`);
    }
  }
}

// ============================================================================
// FACTORY FUNCTIONS
// ============================================================================

/**
 * Create February 21, 1916 impact weather
 */
export function createFebruary21ImpactWeather(
  playerArea: Position3D
): ImpactDrivenWeather {
  return new ImpactDrivenWeather(
    playerArea,
    2000, // Large area
    BombardmentIntensity.DRUMFIRE // Constant apocalypse
  );
}

/**
 * Create typical Verdun impact weather
 */
export function createVerdunImpactWeather(
  playerArea: Position3D
): ImpactDrivenWeather {
  return new ImpactDrivenWeather(
    playerArea,
    500,
    BombardmentIntensity.HARASSING_FIRE
  );
}
