/**
 * KING OF BATTLE - IMPACT-DRIVEN ARTILLERY SYSTEM
 *
 * Reality: You don't see shells flying. You see impacts.
 *
 * Flow:
 * 1. Calculate impact location (ballistics)
 * 2. Schedule impact event at T + flight_time
 * 3. Impact happens: explosion, crater, physics
 * 4. Sound arrives T + (distance / speed_of_sound) later
 * 5. Player hears/feels after the fact
 *
 * No projectile tracking. No visual shells (except rare close-range streaks).
 * Just impacts and delayed audio. Realistic + Performant.
 */

import {
  Position3D,
  ShellType,
  ShellDefinition,
  CoverType
} from './types';
import { getShellDefinition } from './shells';
import { distance2D, estimateFlightTime } from './ballistics';
import { calculateDamage } from './damage';

// ============================================================================
// IMPACT EVENT (The Only Thing That Matters)
// ============================================================================

export interface ScheduledImpact {
  id: string;

  // What and where
  shellType: ShellType;
  impactPosition: Position3D;

  // When
  scheduleTime: number;      // When we scheduled it
  impactTime: number;        // When it actually hits

  // Origin (for sound direction and rare visual streak)
  batteryPosition: Position3D;

  // Visual cues
  showStreak: boolean;       // Rare: only close, small caliber

  // Fuze type
  fuzeType: 'impact' | 'delay' | 'airburst' | 'smoke' | 'gas';
}

export interface ImpactResult {
  id: string;
  position: Position3D;
  shellType: ShellType;
  actualImpactTime: number;

  // Effects
  craterDiameter: number;
  craterDepth: number;
  blastRadius: number;

  // Audio (delayed from impact)
  soundDelay: number;        // seconds until player hears it
  soundDirection: Position3D; // 3D direction to impact
  soundVolume: number;       // 0-100

  // Visual
  showExplosion: boolean;
  showStreak: boolean;
  streakDuration: number;    // frames (1-2)

  // Gameplay
  affectedEntities: string[];
}

// ============================================================================
// ENVIRONMENTAL CONDITIONS
// ============================================================================

export interface EnvironmentalConditions {
  temperature: number;       // Celsius
  humidity: number;          // 0-100%
  windSpeed: number;         // m/s
  terrain: 'mud' | 'rock' | 'wood' | 'water';
}

/**
 * Calculate speed of sound based on temperature
 * c ≈ 331.3 + 0.606 * T (°C)
 */
export function calculateSpeedOfSound(temperatureCelsius: number): number {
  return 331.3 + 0.606 * temperatureCelsius;
}

/**
 * Get speed of sound from environmental conditions
 */
export function getSpeedOfSound(conditions: EnvironmentalConditions): number {
  return calculateSpeedOfSound(conditions.temperature);
}

// ============================================================================
// IMPACT SCHEDULER
// ============================================================================

export class ImpactScheduler {
  private scheduledImpacts: Map<string, ScheduledImpact> = new Map();
  private recentImpacts: ImpactResult[] = [];
  private currentTime: number = 0;
  private playerPosition: Position3D = { x: 0, y: 0, z: 0 };
  private conditions: EnvironmentalConditions;

  constructor(conditions: EnvironmentalConditions) {
    this.conditions = conditions;
  }

  /**
   * Schedule an impact
   * This is the ONLY way artillery enters the system
   */
  public scheduleImpact(
    shellType: ShellType,
    impactPosition: Position3D,
    batteryPosition: Position3D,
    currentTime: number
  ): string {
    const shellDef = getShellDefinition(shellType);
    if (!shellDef) return '';

    // Calculate when it will hit
    const range = distance2D(batteryPosition, impactPosition);
    const flightTime = estimateFlightTime(range, shellDef);

    // Should we show a streak? (rare)
    const distanceToPlayer = distance2D(impactPosition, this.playerPosition);
    const showStreak = this.shouldShowStreak(shellType, distanceToPlayer);

    const impact: ScheduledImpact = {
      id: `impact_${currentTime}_${Math.random()}`,
      shellType,
      impactPosition,
      scheduleTime: currentTime,
      impactTime: currentTime + flightTime,
      batteryPosition,
      showStreak,
      fuzeType: this.determineFuze(shellType)
    };

    this.scheduledImpacts.set(impact.id, impact);
    return impact.id;
  }

  /**
   * Determine if we should show a visual streak
   * Only for close, small caliber rounds
   */
  private shouldShowStreak(shellType: ShellType, distanceToPlayer: number): boolean {
    // Only small caliber
    if (shellType !== ShellType.FRENCH_75MM && shellType !== ShellType.GERMAN_77MM) {
      return false;
    }

    // Only if very close
    if (distanceToPlayer > 120) {
      return false;
    }

    // Random chance (1 in 20)
    return Math.random() < 0.05;
  }

  /**
   * Determine fuze type based on shell type
   */
  private determineFuze(shellType: ShellType): 'impact' | 'delay' | 'airburst' | 'smoke' | 'gas' {
    if (shellType === ShellType.PHOSGENE_GAS || shellType === ShellType.MUSTARD_GAS) {
      return 'gas';
    }

    if (shellType === ShellType.SMOKE) {
      return 'smoke';
    }

    // Large shells often have delay fuzes to penetrate before exploding
    if (shellType === ShellType.GERMAN_420MM || shellType === ShellType.FRENCH_305MM) {
      return Math.random() < 0.3 ? 'delay' : 'impact';
    }

    return 'impact';
  }

  /**
   * Update scheduler - process impacts that should happen this frame
   */
  public update(deltaTime: number): ImpactResult[] {
    this.currentTime += deltaTime;

    const impactsThisFrame: ImpactResult[] = [];
    const toRemove: string[] = [];

    for (const [id, impact] of this.scheduledImpacts) {
      if (impact.impactTime <= this.currentTime) {
        // IMPACT HAPPENS NOW
        const result = this.executeImpact(impact);
        impactsThisFrame.push(result);
        this.recentImpacts.push(result);
        toRemove.push(id);
      }
    }

    // Remove executed impacts
    for (const id of toRemove) {
      this.scheduledImpacts.delete(id);
    }

    // Keep only recent 100 impacts
    if (this.recentImpacts.length > 100) {
      this.recentImpacts = this.recentImpacts.slice(-100);
    }

    return impactsThisFrame;
  }

  /**
   * Execute an impact - calculate all effects
   */
  private executeImpact(impact: ScheduledImpact): ImpactResult {
    const shellDef = getShellDefinition(impact.shellType);
    if (!shellDef) throw new Error('Invalid shell type');

    // Calculate sound delay
    const distanceToPlayer = distance2D(impact.impactPosition, this.playerPosition);
    const speedOfSound = getSpeedOfSound(this.conditions);
    const soundDelay = distanceToPlayer / speedOfSound;

    // Sound direction (for 3D audio)
    const dx = impact.impactPosition.x - this.playerPosition.x;
    const dz = impact.impactPosition.z - this.playerPosition.z;
    const dist = Math.sqrt(dx * dx + dz * dz);
    const soundDirection: Position3D = {
      x: dx / dist,
      y: 0,
      z: dz / dist
    };

    // Sound volume (inverse square law with distance)
    const baseVolume = shellDef.audio.volumeDb;
    const volumeAtDistance = baseVolume - 20 * Math.log10(distanceToPlayer / 10);
    const soundVolume = Math.max(0, Math.min(100, (volumeAtDistance / 140) * 100));

    // Visual effects
    const showExplosion = distanceToPlayer <= shellDef.damage.debris;

    return {
      id: impact.id,
      position: impact.impactPosition,
      shellType: impact.shellType,
      actualImpactTime: this.currentTime,
      craterDiameter: shellDef.crater.diameter,
      craterDepth: shellDef.crater.depth,
      blastRadius: shellDef.damage.severe,
      soundDelay,
      soundDirection,
      soundVolume,
      showExplosion,
      showStreak: impact.showStreak,
      streakDuration: 2, // frames
      affectedEntities: []
    };
  }

  /**
   * Set player position (for distance calculations)
   */
  public setPlayerPosition(position: Position3D): void {
    this.playerPosition = position;
  }

  /**
   * Get scheduled impacts (for UI/debugging)
   */
  public getScheduledImpacts(): ScheduledImpact[] {
    return Array.from(this.scheduledImpacts.values());
  }

  /**
   * Get recent impacts
   */
  public getRecentImpacts(limit: number = 10): ImpactResult[] {
    return this.recentImpacts.slice(-limit);
  }

  /**
   * Get impacts that player will hear soon (warning system)
   */
  public getIncomingImpacts(lookaheadSeconds: number = 5): ScheduledImpact[] {
    return Array.from(this.scheduledImpacts.values())
      .filter(impact => {
        const timeUntilImpact = impact.impactTime - this.currentTime;
        return timeUntilImpact > 0 && timeUntilImpact <= lookaheadSeconds;
      })
      .sort((a, b) => a.impactTime - b.impactTime);
  }

  /**
   * Get performance statistics
   */
  public getStats() {
    return {
      scheduledImpacts: this.scheduledImpacts.size,
      recentImpacts: this.recentImpacts.length,
      currentTime: this.currentTime
    };
  }
}

// ============================================================================
// AUDIO DELAY MANAGER
// ============================================================================

/**
 * Manages delayed audio playback
 * Sounds arrive AFTER impacts based on distance and speed of sound
 */
export class AudioDelayManager {
  private scheduledSounds: Array<{
    id: string;
    playTime: number;
    position: Position3D;
    shellType: ShellType;
    volume: number;
    direction: Position3D;
  }> = [];

  private currentTime: number = 0;

  /**
   * Schedule audio to play after delay
   */
  public scheduleAudio(
    impactResult: ImpactResult,
    currentTime: number
  ): void {
    this.scheduledSounds.push({
      id: impactResult.id,
      playTime: currentTime + impactResult.soundDelay,
      position: impactResult.position,
      shellType: impactResult.shellType,
      volume: impactResult.soundVolume,
      direction: impactResult.soundDirection
    });
  }

  /**
   * Update - play sounds that are due
   */
  public update(deltaTime: number): Array<{
    position: Position3D;
    shellType: ShellType;
    volume: number;
    direction: Position3D;
  }> {
    this.currentTime += deltaTime;

    const soundsToPlay: Array<{
      position: Position3D;
      shellType: ShellType;
      volume: number;
      direction: Position3D;
    }> = [];

    const toRemove: number[] = [];

    for (let i = 0; i < this.scheduledSounds.length; i++) {
      const sound = this.scheduledSounds[i];

      if (sound.playTime <= this.currentTime) {
        soundsToPlay.push({
          position: sound.position,
          shellType: sound.shellType,
          volume: sound.volume,
          direction: sound.direction
        });
        toRemove.push(i);
      }
    }

    // Remove played sounds (reverse order to maintain indices)
    for (let i = toRemove.length - 1; i >= 0; i--) {
      this.scheduledSounds.splice(toRemove[i], 1);
    }

    return soundsToPlay;
  }

  /**
   * Get upcoming sounds (for pre-cueing, pre-rumble)
   */
  public getUpcomingSounds(lookaheadSeconds: number = 0.5): Array<{
    playTime: number;
    timeUntil: number;
    position: Position3D;
  }> {
    return this.scheduledSounds
      .filter(s => {
        const timeUntil = s.playTime - this.currentTime;
        return timeUntil > 0 && timeUntil <= lookaheadSeconds;
      })
      .map(s => ({
        playTime: s.playTime,
        timeUntil: s.playTime - this.currentTime,
        position: s.position
      }));
  }
}

// ============================================================================
// PLAYER AWARENESS SYSTEM
// ============================================================================

/**
 * Player awareness of incoming impacts
 * Different from tracking shells - this is about recognizing patterns
 */
export interface ImpactThreat {
  impactPosition: Position3D;
  timeUntilImpact: number;
  distanceToPlayer: number;
  shellType: ShellType;
  isRecognized: boolean;       // Did player "sense" this one coming?
  threatLevel: 'none' | 'low' | 'moderate' | 'high' | 'extreme';
  recommendedAction: string;
}

export class PlayerAwarenessSystem {
  private playerPosition: Position3D = { x: 0, y: 0, z: 0 };
  private playerSkillLevel: number = 50; // 0-100

  public setPlayerPosition(position: Position3D): void {
    this.playerPosition = position;
  }

  public setSkillLevel(skill: number): void {
    this.playerSkillLevel = Math.max(0, Math.min(100, skill));
  }

  /**
   * Assess threats from scheduled impacts
   * Player can "sense" patterns - repetition, creeping barrages, etc.
   */
  public assessThreats(
    scheduledImpacts: ScheduledImpact[],
    currentTime: number
  ): ImpactThreat[] {
    const threats: ImpactThreat[] = [];

    for (const impact of scheduledImpacts) {
      const distanceToPlayer = distance2D(impact.impactPosition, this.playerPosition);
      const timeUntilImpact = impact.impactTime - currentTime;

      // Only assess impacts in near future (player can't predict too far ahead)
      if (timeUntilImpact > 30 || timeUntilImpact <= 0) continue;

      const shellDef = getShellDefinition(impact.shellType);
      if (!shellDef) continue;

      // Determine threat level based on distance to impact point
      let threatLevel: 'none' | 'low' | 'moderate' | 'high' | 'extreme' = 'none';
      if (distanceToPlayer <= shellDef.damage.lethal) {
        threatLevel = 'extreme';
      } else if (distanceToPlayer <= shellDef.damage.severe) {
        threatLevel = 'high';
      } else if (distanceToPlayer <= shellDef.damage.moderate) {
        threatLevel = 'moderate';
      } else if (distanceToPlayer <= shellDef.damage.light) {
        threatLevel = 'low';
      }

      // Player "recognizes" threat based on skill and distance
      const recognitionChance = this.playerSkillLevel * (1 - distanceToPlayer / 500);
      const isRecognized = Math.random() * 100 < recognitionChance;

      threats.push({
        impactPosition: impact.impactPosition,
        timeUntilImpact,
        distanceToPlayer,
        shellType: impact.shellType,
        isRecognized,
        threatLevel,
        recommendedAction: this.getRecommendedAction(threatLevel, timeUntilImpact)
      });
    }

    return threats.sort((a, b) => a.timeUntilImpact - b.timeUntilImpact);
  }

  private getRecommendedAction(
    threatLevel: 'none' | 'low' | 'moderate' | 'high' | 'extreme',
    timeUntil: number
  ): string {
    if (threatLevel === 'none') return "Continue task";

    if (threatLevel === 'extreme') {
      if (timeUntil < 3) return "Drop prone immediately!";
      if (timeUntil < 8) return "Sprint to deep cover!";
      return "Seek deepest bunker!";
    }

    if (threatLevel === 'high') {
      if (timeUntil < 5) return "Take cover now!";
      return "Move to dugout";
    }

    if (threatLevel === 'moderate') {
      return "Stay alert, be ready to move";
    }

    return "Be aware";
  }
}

// ============================================================================
// FACTORY FUNCTIONS
// ============================================================================

/**
 * Create impact scheduler with typical Verdun conditions
 */
export function createVerdunImpactScheduler(): ImpactScheduler {
  const conditions: EnvironmentalConditions = {
    temperature: 5,        // Cold February morning
    humidity: 80,
    windSpeed: 3,
    terrain: 'mud'
  };

  return new ImpactScheduler(conditions);
}

/**
 * Create audio delay manager
 */
export function createAudioDelayManager(): AudioDelayManager {
  return new AudioDelayManager();
}

/**
 * Create player awareness system
 */
export function createPlayerAwareness(): PlayerAwarenessSystem {
  return new PlayerAwarenessSystem();
}
