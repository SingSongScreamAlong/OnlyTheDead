/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Main Engine Coordinator
 *
 * "Artillery conquers, infantry occupies" - General Nivelle
 *
 * This is the main artillery simulation engine for WWI Verdun battle.
 * Historically accurate artillery system based on 40 million shells fired
 * over 303 days at Verdun, 1916.
 */

import {
  ArtilleryGameState,
  ArtilleryEngineConfig,
  ShellInFlight,
  FireMission,
  ArtilleryBattery,
  ImpactEvent,
  PlayerSkills,
  PlayerDetection,
  Position3D,
  CoverType,
  ShellType,
  BombardmentConfig,
  FireMissionStatus
} from './types';
import { updateShellInFlight } from './ballistics';
import { createImpactEvent } from './damage';
import { detectAudibleShells, updatePlayerSkills } from './playerDetection';
import { createBombardmentSchedule, executeBombardmentStep } from './bombardment';
import { startFireMission, completeFireMission } from './fireMission';

// ============================================================================
// ARTILLERY ENGINE CLASS
// ============================================================================

export class ArtilleryEngine {
  private state: ArtilleryGameState;
  private config: ArtilleryEngineConfig;
  private lastUpdate: number;
  private bombardmentSchedules: Map<string, any[]> = new Map();

  constructor(config: ArtilleryEngineConfig) {
    this.config = config;
    this.lastUpdate = 0;

    // Initialize game state
    this.state = {
      currentTime: 0,
      shellsInFlight: new Map(),
      activeMissions: new Map(),
      batteries: new Map(),
      recentImpacts: [],
      stats: {
        totalShellsFired: 0,
        shellsFiredByType: {},
        totalCasualties: 0,
        cratersCreated: 0
      },
      playerPosition: { x: 0, y: 0, z: 0 },
      playerSkills: {
        experience: 'novice',
        shellsRecognized: 0,
        shellsSurvived: 0,
        recognitionAccuracy: 50,
        recognitionTime: 3,
        panicChance: 60,
        morale: 80,
        stamina: 80,
        alertness: 70,
        shellShock: false
      },
      playerCover: CoverType.OPEN_GROUND,
      windSpeed: 2,
      windDirection: 90,
      temperature: 15,
      visibility: 1000
    };
  }

  // ========================================================================
  // CORE UPDATE LOOP
  // ========================================================================

  /**
   * Main update loop - call this every frame
   */
  public update(deltaTime: number): void {
    this.state.currentTime += deltaTime;

    // Update shells in flight
    this.updateShells(deltaTime);

    // Update active fire missions
    this.updateFireMissions();

    // Check for shell impacts
    this.checkImpacts();

    // Clean up old impacts
    this.cleanupOldImpacts();

    this.lastUpdate = this.state.currentTime;
  }

  /**
   * Update all shells in flight
   */
  private updateShells(deltaTime: number): void {
    const shellsToRemove: string[] = [];

    for (const [id, shell] of this.state.shellsInFlight) {
      // Update shell position
      updateShellInFlight(shell, this.state.currentTime, deltaTime, this.state);

      // Check if shell has impacted
      if (shell.currentPosition.y <= 0 || shell.timeRemaining <= 0) {
        shellsToRemove.push(id);
        this.handleImpact(shell);
      }
    }

    // Remove impacted shells
    for (const id of shellsToRemove) {
      this.state.shellsInFlight.delete(id);
    }
  }

  /**
   * Update active fire missions
   */
  private updateFireMissions(): void {
    for (const [id, mission] of this.state.activeMissions) {
      if (mission.status === FireMissionStatus.IN_PROGRESS) {
        // Execute bombardment
        this.executeMissionStep(mission);

        // Check if mission is complete
        if (this.state.currentTime >= mission.endTime) {
          completeFireMission(mission);
        }
      }
    }
  }

  /**
   * Execute a fire mission step
   */
  private executeMissionStep(mission: FireMission): void {
    // Get or create bombardment schedule
    let schedule = this.bombardmentSchedules.get(mission.id);

    if (!schedule) {
      // Get first battery assigned to mission
      const batteryId = mission.batteries[0];
      const battery = this.state.batteries.get(batteryId);
      if (!battery) return;

      schedule = createBombardmentSchedule(
        mission.bombardment,
        battery.position,
        mission.startTime
      );

      this.bombardmentSchedules.set(mission.id, schedule);
    }

    // Execute this step
    const batteryId = mission.batteries[0];
    const battery = this.state.batteries.get(batteryId);
    if (!battery) return;

    const newShells = executeBombardmentStep(
      schedule,
      this.state.currentTime,
      battery.position,
      batteryId
    );

    // Add shells to game state
    for (const shell of newShells) {
      this.state.shellsInFlight.set(shell.id, shell);
      this.state.stats.totalShellsFired++;

      const typeCount = this.state.stats.shellsFiredByType[shell.shellType] || 0;
      this.state.stats.shellsFiredByType[shell.shellType] = typeCount + 1;
    }
  }

  /**
   * Check for impacts that should occur this frame
   */
  private checkImpacts(): void {
    for (const shell of this.state.shellsInFlight.values()) {
      if (shell.impactTime <= this.state.currentTime) {
        this.handleImpact(shell);
      }
    }
  }

  /**
   * Handle shell impact
   */
  private handleImpact(shell: ShellInFlight): void {
    const impactEvent = createImpactEvent(
      `impact_${this.state.currentTime}_${Math.random()}`,
      shell.shellType,
      shell.target,
      this.state.currentTime,
      [] // Entities would be populated by game logic
    );

    this.state.recentImpacts.push(impactEvent);
    this.state.stats.cratersCreated++;

    // Check if player was affected
    this.checkPlayerImpact(impactEvent);
  }

  /**
   * Check if impact affects player
   */
  private checkPlayerImpact(impact: ImpactEvent): void {
    const distance = this.getDistance2D(impact.impactPosition, this.state.playerPosition);

    if (distance <= impact.damageRadii.light) {
      // Player is in affected area - game would handle damage here
      console.log(`Player in impact zone! Distance: ${distance.toFixed(1)}m`);
    }
  }

  /**
   * Clean up old impact events (keep last 100)
   */
  private cleanupOldImpacts(): void {
    if (this.state.recentImpacts.length > 100) {
      this.state.recentImpacts = this.state.recentImpacts.slice(-100);
    }
  }

  // ========================================================================
  // PUBLIC API
  // ========================================================================

  /**
   * Start a fire mission
   */
  public startMission(mission: FireMission): void {
    startFireMission(mission, this.state.currentTime);
    this.state.activeMissions.set(mission.id, mission);
  }

  /**
   * Add a battery to the game
   */
  public addBattery(battery: ArtilleryBattery): void {
    this.state.batteries.set(battery.id, battery);
  }

  /**
   * Get player detections (what shells can player hear/see)
   */
  public getPlayerDetections(): PlayerDetection[] {
    return detectAudibleShells(
      Array.from(this.state.shellsInFlight.values()),
      this.state.playerPosition,
      this.state.playerSkills,
      this.state.currentTime
    );
  }

  /**
   * Update player position
   */
  public setPlayerPosition(position: Position3D): void {
    this.state.playerPosition = position;
  }

  /**
   * Update player cover type
   */
  public setPlayerCover(cover: CoverType): void {
    this.state.playerCover = cover;
  }

  /**
   * Update player skills (e.g., after recognizing a shell)
   */
  public updatePlayerSkills(recognized: boolean, survived: boolean): void {
    this.state.playerSkills = updatePlayerSkills(
      this.state.playerSkills,
      recognized,
      survived
    );
  }

  /**
   * Get current game state
   */
  public getState(): Readonly<ArtilleryGameState> {
    return this.state;
  }

  /**
   * Get statistics
   */
  public getStats() {
    return {
      ...this.state.stats,
      shellsInFlight: this.state.shellsInFlight.size,
      activeMissions: this.state.activeMissions.size,
      batteries: this.state.batteries.size,
      gameTime: this.state.currentTime
    };
  }

  /**
   * Get all active shells
   */
  public getShellsInFlight(): ShellInFlight[] {
    return Array.from(this.state.shellsInFlight.values());
  }

  /**
   * Get recent impacts
   */
  public getRecentImpacts(limit: number = 10): ImpactEvent[] {
    return this.state.recentImpacts.slice(-limit);
  }

  /**
   * Set environmental conditions
   */
  public setEnvironment(
    windSpeed: number,
    windDirection: number,
    temperature: number,
    visibility: number
  ): void {
    this.state.windSpeed = windSpeed;
    this.state.windDirection = windDirection;
    this.state.temperature = temperature;
    this.state.visibility = visibility;
  }

  /**
   * Reset the engine
   */
  public reset(): void {
    this.state.currentTime = 0;
    this.state.shellsInFlight.clear();
    this.state.activeMissions.clear();
    this.state.recentImpacts = [];
    this.bombardmentSchedules.clear();
    this.state.stats = {
      totalShellsFired: 0,
      shellsFiredByType: {},
      totalCasualties: 0,
      cratersCreated: 0
    };
  }

  // ========================================================================
  // HELPER METHODS
  // ========================================================================

  private getDistance2D(p1: Position3D, p2: Position3D): number {
    const dx = p2.x - p1.x;
    const dz = p2.z - p1.z;
    return Math.sqrt(dx * dx + dz * dz);
  }
}

// ============================================================================
// FACTORY FUNCTIONS
// ============================================================================

/**
 * Create default artillery engine
 */
export function createArtilleryEngine(
  options: Partial<ArtilleryEngineConfig> = {}
): ArtilleryEngine {
  const defaultConfig: ArtilleryEngineConfig = {
    tickRate: 60,
    maxShellsInFlight: 1000,
    maxSimulationDistance: 10000,
    audioEnabled: true,
    spatialAudioEnabled: true,
    maxAudioSources: 32,
    playerSkillProgression: true,
    shellShockEnabled: true,
    permadeathMode: false,
    historicalShellMix: true,
    historicalCasualtyRates: true,
    detailedPhysics: false,
    particleEffects: true,
    ...options
  };

  return new ArtilleryEngine(defaultConfig);
}

/**
 * Create historical Verdun scenario
 */
export function createVerdunScenario(): ArtilleryEngine {
  const engine = createArtilleryEngine({
    historicalShellMix: true,
    historicalCasualtyRates: true
  });

  // Set environment to typical Verdun conditions (February 1916)
  engine.setEnvironment(
    3, // 3 m/s wind
    90, // East wind
    2, // 2°C (cold)
    500 // 500m visibility (winter fog)
  );

  return engine;
}

// ============================================================================
// EXPORTS
// ============================================================================

export * from './types';
export * from './shells';
export * from './ballistics';
export * from './damage';
export * from './bombardment';
export * from './fireMission';
export * from './playerDetection';
