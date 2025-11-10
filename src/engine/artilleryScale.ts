/**
 * KING OF BATTLE - SCALABLE ARTILLERY SYSTEM
 *
 * Handles MASSIVE SCALE bombardment (40 million shells over 303 days)
 *
 * Three-tier system:
 * 1. DETAILED ZONE (player proximity) - Full physics, audio, individual shells
 * 2. SIMPLIFIED ZONE (nearby) - Grouped impacts, statistical effects
 * 3. BACKGROUND ZONE (distant) - Pure statistics, atmosphere only
 *
 * This allows Feb 21 drumfire (23 shells/sec) without performance death.
 */

import {
  ArtilleryGameState,
  Position3D,
  ShellType,
  ShellInFlight,
  ImpactEvent,
  BombardmentIntensity,
  PlayerDetection
} from './types';
import { distance2D } from './ballistics';
import { selectShellType } from './bombardment';
import { createShellInFlight } from './ballistics';

// ============================================================================
// SCALE CONFIGURATION
// ============================================================================

export interface ScaleConfig {
  // Zone radii (meters from player)
  detailedZoneRadius: number;      // Full simulation (0-500m typically)
  simplifiedZoneRadius: number;    // Grouped impacts (500-2000m)
  backgroundZoneRadius: number;    // Pure stats (2000m+)

  // Performance limits
  maxDetailedShells: number;       // Max individual shells in detailed zone
  maxSimplifiedGroups: number;     // Max shell groups in simplified zone

  // Update rates (Hz)
  detailedUpdateRate: number;      // Full physics updates per second
  simplifiedUpdateRate: number;    // Grouped updates per second
  backgroundUpdateRate: number;    // Statistical updates per second

  // Audio limits
  maxAudioSources: number;         // Max simultaneous sounds
  audioPooling: boolean;           // Reuse audio sources
}

export const DEFAULT_SCALE_CONFIG: ScaleConfig = {
  detailedZoneRadius: 500,
  simplifiedZoneRadius: 2000,
  backgroundZoneRadius: 10000,
  maxDetailedShells: 100,          // Player can only track ~100 at once
  maxSimplifiedGroups: 50,
  detailedUpdateRate: 60,          // 60 Hz
  simplifiedUpdateRate: 10,        // 10 Hz
  backgroundUpdateRate: 1,         // 1 Hz
  maxAudioSources: 32,             // Human can perceive ~32 simultaneous sounds
  audioPooling: true
};

// ============================================================================
// SHELL GROUP (Simplified Zone)
// ============================================================================

/**
 * Group of shells treated as a statistical batch
 * Used in simplified zone to reduce individual tracking
 */
interface ShellGroup {
  id: string;
  shellType: ShellType;
  count: number;                   // Number of shells in group
  centerPosition: Position3D;
  spreadRadius: number;            // Dispersion radius
  impactTime: number;
  impactInterval: number;          // Seconds between impacts in group
}

// ============================================================================
// BACKGROUND BOMBARDMENT (Distant Zone)
// ============================================================================

/**
 * Pure statistical bombardment in distant zone
 * No individual shells tracked - just atmosphere and stats
 */
interface BackgroundBombardment {
  intensity: BombardmentIntensity;
  shellsPerSecond: number;
  affectedArea: {
    center: Position3D;
    radius: number;
  };
  lastUpdate: number;
  accumulatedShells: number;       // Shells since last update
}

// ============================================================================
// SCALABLE ARTILLERY MANAGER
// ============================================================================

export class ScalableArtilleryManager {
  private config: ScaleConfig;
  private playerPosition: Position3D;

  // Three tiers of simulation
  private detailedShells: Map<string, ShellInFlight> = new Map();
  private simplifiedGroups: Map<string, ShellGroup> = new Map();
  private backgroundBombardment: BackgroundBombardment | null = null;

  // Update timing
  private lastDetailedUpdate: number = 0;
  private lastSimplifiedUpdate: number = 0;
  private lastBackgroundUpdate: number = 0;

  // Statistics
  private stats = {
    totalShellsFired: 0,
    detailedShellsActive: 0,
    simplifiedGroupsActive: 0,
    backgroundShellsPerSecond: 0,
    performanceSavings: 0           // Shells NOT individually tracked
  };

  constructor(config: Partial<ScaleConfig> = {}) {
    this.config = { ...DEFAULT_SCALE_CONFIG, ...config };
    this.playerPosition = { x: 0, y: 0, z: 0 };
  }

  /**
   * Set player position (determines simulation zones)
   */
  public setPlayerPosition(position: Position3D): void {
    this.playerPosition = position;
    this.redistributeShells();
  }

  /**
   * Add incoming shells - automatically sorted into zones
   */
  public addIncomingShells(
    shellType: ShellType,
    targetPositions: Position3D[],
    currentTime: number,
    batteryPosition: Position3D
  ): void {
    for (const target of targetPositions) {
      const distanceToPlayer = distance2D(target, this.playerPosition);

      if (distanceToPlayer <= this.config.detailedZoneRadius) {
        // DETAILED ZONE - Full simulation
        this.addDetailedShell(shellType, target, currentTime, batteryPosition);
      } else if (distanceToPlayer <= this.config.simplifiedZoneRadius) {
        // SIMPLIFIED ZONE - Group it
        this.addToSimplifiedGroup(shellType, target, currentTime);
      } else {
        // BACKGROUND ZONE - Pure statistics
        this.addToBackground(shellType, currentTime);
      }

      this.stats.totalShellsFired++;
    }
  }

  /**
   * Add shell to detailed zone (full physics)
   */
  private addDetailedShell(
    shellType: ShellType,
    target: Position3D,
    currentTime: number,
    batteryPosition: Position3D
  ): void {
    // Check capacity limit
    if (this.detailedShells.size >= this.config.maxDetailedShells) {
      // Drop oldest shell or demote to simplified
      const oldest = this.getOldestDetailedShell();
      if (oldest) {
        this.detailedShells.delete(oldest);
      }
    }

    const shell = createShellInFlight(
      `detailed_${currentTime}_${Math.random()}`,
      shellType,
      batteryPosition,
      target,
      currentTime,
      'battery_auto',
      85
    );

    if (shell) {
      this.detailedShells.set(shell.id, shell);
      this.stats.detailedShellsActive = this.detailedShells.size;
    }
  }

  /**
   * Add shell to simplified group
   */
  private addToSimplifiedGroup(
    shellType: ShellType,
    target: Position3D,
    currentTime: number
  ): void {
    // Find or create group for this type/area
    const groupKey = this.getGroupKey(shellType, target);
    let group = this.simplifiedGroups.get(groupKey);

    if (!group) {
      // Create new group
      group = {
        id: groupKey,
        shellType,
        count: 0,
        centerPosition: target,
        spreadRadius: 50,
        impactTime: currentTime + 15, // Estimated
        impactInterval: 1.0
      };
      this.simplifiedGroups.set(groupKey, group);
    }

    group.count++;
    this.stats.simplifiedGroupsActive = this.simplifiedGroups.size;
    this.stats.performanceSavings++;
  }

  /**
   * Add shell to background statistics
   */
  private addToBackground(shellType: ShellType, currentTime: number): void {
    if (!this.backgroundBombardment) {
      this.backgroundBombardment = {
        intensity: BombardmentIntensity.LIGHT,
        shellsPerSecond: 0.5,
        affectedArea: {
          center: this.playerPosition,
          radius: 5000
        },
        lastUpdate: currentTime,
        accumulatedShells: 0
      };
    }

    this.backgroundBombardment.accumulatedShells++;
    this.stats.backgroundShellsPerSecond =
      this.backgroundBombardment.accumulatedShells /
      Math.max(1, currentTime - this.backgroundBombardment.lastUpdate);
    this.stats.performanceSavings++;
  }

  /**
   * Update all zones at appropriate rates
   */
  public update(currentTime: number, deltaTime: number): void {
    // Detailed zone - high frequency
    if (currentTime - this.lastDetailedUpdate >= 1.0 / this.config.detailedUpdateRate) {
      this.updateDetailedZone(currentTime, deltaTime);
      this.lastDetailedUpdate = currentTime;
    }

    // Simplified zone - medium frequency
    if (currentTime - this.lastSimplifiedUpdate >= 1.0 / this.config.simplifiedUpdateRate) {
      this.updateSimplifiedZone(currentTime);
      this.lastSimplifiedUpdate = currentTime;
    }

    // Background zone - low frequency
    if (currentTime - this.lastBackgroundUpdate >= 1.0 / this.config.backgroundUpdateRate) {
      this.updateBackgroundZone(currentTime);
      this.lastBackgroundUpdate = currentTime;
    }

    // Redistribute shells based on player movement
    this.redistributeShells();
  }

  /**
   * Update detailed zone (full physics)
   */
  private updateDetailedZone(currentTime: number, deltaTime: number): void {
    const shellsToRemove: string[] = [];

    for (const [id, shell] of this.detailedShells) {
      // Check if shell left detailed zone
      const distanceToPlayer = distance2D(shell.target, this.playerPosition);

      if (distanceToPlayer > this.config.detailedZoneRadius) {
        // Demote to simplified zone
        this.demoteToSimplified(shell);
        shellsToRemove.push(id);
        continue;
      }

      // Check for impact
      if (shell.timeRemaining <= 0) {
        shellsToRemove.push(id);
        // Create impact event (handled by main engine)
      }
    }

    for (const id of shellsToRemove) {
      this.detailedShells.delete(id);
    }

    this.stats.detailedShellsActive = this.detailedShells.size;
  }

  /**
   * Update simplified zone (grouped impacts)
   */
  private updateSimplifiedZone(currentTime: number): void {
    const groupsToRemove: string[] = [];

    for (const [id, group] of this.simplifiedGroups) {
      // Check if group should start impacting
      if (currentTime >= group.impactTime) {
        // Process group impacts
        this.processGroupImpacts(group);
        groupsToRemove.push(id);
      }

      // Check if group entered detailed zone
      const distanceToPlayer = distance2D(group.centerPosition, this.playerPosition);
      if (distanceToPlayer <= this.config.detailedZoneRadius) {
        // Promote to detailed
        this.promoteToDetailed(group, currentTime);
        groupsToRemove.push(id);
      }
    }

    for (const id of groupsToRemove) {
      this.simplifiedGroups.delete(id);
    }

    this.stats.simplifiedGroupsActive = this.simplifiedGroups.size;
  }

  /**
   * Update background zone (pure statistics)
   */
  private updateBackgroundZone(currentTime: number): void {
    if (!this.backgroundBombardment) return;

    // Update statistics
    const timeSinceLastUpdate = currentTime - this.backgroundBombardment.lastUpdate;
    if (timeSinceLastUpdate >= 1.0) {
      this.stats.backgroundShellsPerSecond =
        this.backgroundBombardment.accumulatedShells / timeSinceLastUpdate;

      this.backgroundBombardment.accumulatedShells = 0;
      this.backgroundBombardment.lastUpdate = currentTime;
    }
  }

  /**
   * Redistribute shells when player moves
   */
  private redistributeShells(): void {
    // Check detailed shells
    const toSimplify: ShellInFlight[] = [];
    for (const shell of this.detailedShells.values()) {
      const dist = distance2D(shell.target, this.playerPosition);
      if (dist > this.config.detailedZoneRadius) {
        toSimplify.push(shell);
      }
    }

    for (const shell of toSimplify) {
      this.demoteToSimplified(shell);
      this.detailedShells.delete(shell.id);
    }

    // Check simplified groups
    const toDetail: ShellGroup[] = [];
    for (const group of this.simplifiedGroups.values()) {
      const dist = distance2D(group.centerPosition, this.playerPosition);
      if (dist <= this.config.detailedZoneRadius) {
        toDetail.push(group);
      }
    }

    for (const group of toDetail) {
      this.promoteToDetailed(group, Date.now());
      this.simplifiedGroups.delete(group.id);
    }
  }

  /**
   * Demote detailed shell to simplified group
   */
  private demoteToSimplified(shell: ShellInFlight): void {
    const groupKey = this.getGroupKey(shell.shellType, shell.target);
    let group = this.simplifiedGroups.get(groupKey);

    if (!group) {
      group = {
        id: groupKey,
        shellType: shell.shellType,
        count: 0,
        centerPosition: shell.target,
        spreadRadius: 50,
        impactTime: shell.impactTime,
        impactInterval: 1.0
      };
      this.simplifiedGroups.set(groupKey, group);
    }

    group.count++;
  }

  /**
   * Promote simplified group to detailed shells
   */
  private promoteToDetailed(group: ShellGroup, currentTime: number): void {
    // Only promote a subset to avoid flooding detailed zone
    const promotionCount = Math.min(group.count, 10);

    for (let i = 0; i < promotionCount; i++) {
      // Create detailed shell from group
      const offset = (Math.random() - 0.5) * group.spreadRadius * 2;
      const target = {
        x: group.centerPosition.x + offset,
        y: group.centerPosition.y,
        z: group.centerPosition.z + offset
      };

      const batteryPos = {
        x: target.x - 3000,
        y: 0,
        z: target.z
      };

      this.addDetailedShell(group.shellType, target, currentTime, batteryPos);
    }

    // Reduce group count
    group.count -= promotionCount;
  }

  /**
   * Process group impacts (simplified zone)
   */
  private processGroupImpacts(group: ShellGroup): void {
    // Generate area effect instead of individual impacts
    // This is handled by game logic - we just report the group impact
  }

  /**
   * Get group key for shell grouping
   */
  private getGroupKey(shellType: ShellType, position: Position3D): string {
    // Group shells by type and 100m grid
    const gridX = Math.floor(position.x / 100);
    const gridZ = Math.floor(position.z / 100);
    return `${shellType}_${gridX}_${gridZ}`;
  }

  /**
   * Get oldest detailed shell (for culling)
   */
  private getOldestDetailedShell(): string | null {
    let oldest: string | null = null;
    let oldestTime = Infinity;

    for (const [id, shell] of this.detailedShells) {
      if (shell.launchTime < oldestTime) {
        oldestTime = shell.launchTime;
        oldest = id;
      }
    }

    return oldest;
  }

  /**
   * Get detailed shells (for player detection)
   */
  public getDetailedShells(): ShellInFlight[] {
    return Array.from(this.detailedShells.values());
  }

  /**
   * Get simplified groups (for visual effects at distance)
   */
  public getSimplifiedGroups(): ShellGroup[] {
    return Array.from(this.simplifiedGroups.values());
  }

  /**
   * Get background bombardment stats
   */
  public getBackgroundStats(): {
    intensity: BombardmentIntensity;
    shellsPerSecond: number;
  } | null {
    if (!this.backgroundBombardment) return null;

    return {
      intensity: this.backgroundBombardment.intensity,
      shellsPerSecond: this.stats.backgroundShellsPerSecond
    };
  }

  /**
   * Get performance statistics
   */
  public getStats() {
    return {
      ...this.stats,
      totalActive: this.detailedShells.size +
                   this.simplifiedGroups.size +
                   (this.backgroundBombardment?.accumulatedShells || 0),
      performanceFactor: this.stats.performanceSavings /
                        Math.max(1, this.stats.totalShellsFired)
    };
  }

  /**
   * Set bombardment intensity (affects background generation)
   */
  public setIntensity(intensity: BombardmentIntensity): void {
    if (!this.backgroundBombardment) return;

    this.backgroundBombardment.intensity = intensity;

    // Map intensity to shells per second
    const intensityMap = {
      [BombardmentIntensity.HARASSING_FIRE]: 0.5,
      [BombardmentIntensity.LIGHT]: 2,
      [BombardmentIntensity.HEAVY]: 10,
      [BombardmentIntensity.DRUMFIRE]: 30
    };

    this.backgroundBombardment.shellsPerSecond = intensityMap[intensity] || 1;
  }
}

// ============================================================================
// INTEGRATION WITH ARTILLERY WEATHER
// ============================================================================

/**
 * Create scalable artillery manager for weather system
 */
export function createScalableManager(
  config: Partial<ScaleConfig> = {}
): ScalableArtilleryManager {
  return new ScalableArtilleryManager(config);
}

/**
 * Calculate optimal scale config based on target performance
 */
export function calculateScaleConfig(
  targetFPS: number,
  maxIntensity: BombardmentIntensity
): ScaleConfig {
  // Adjust zones based on target FPS
  const fpsMultiplier = 60 / targetFPS;

  return {
    detailedZoneRadius: 500 / fpsMultiplier,
    simplifiedZoneRadius: 2000,
    backgroundZoneRadius: 10000,
    maxDetailedShells: Math.floor(100 / fpsMultiplier),
    maxSimplifiedGroups: 50,
    detailedUpdateRate: targetFPS,
    simplifiedUpdateRate: Math.max(10, targetFPS / 6),
    backgroundUpdateRate: 1,
    maxAudioSources: 32,
    audioPooling: true
  };
}

/**
 * Get performance recommendations based on intensity
 */
export function getPerformanceRecommendations(
  intensity: BombardmentIntensity
): {
  detailedZoneRadius: number;
  expectedShellsPerSecond: number;
  recommendedFPS: number;
} {
  const intensityData = {
    [BombardmentIntensity.HARASSING_FIRE]: {
      detailedZoneRadius: 500,
      expectedShellsPerSecond: 2,
      recommendedFPS: 60
    },
    [BombardmentIntensity.LIGHT]: {
      detailedZoneRadius: 400,
      expectedShellsPerSecond: 15,
      recommendedFPS: 60
    },
    [BombardmentIntensity.HEAVY]: {
      detailedZoneRadius: 300,
      expectedShellsPerSecond: 50,
      recommendedFPS: 30
    },
    [BombardmentIntensity.DRUMFIRE]: {
      detailedZoneRadius: 200,
      expectedShellsPerSecond: 120,
      recommendedFPS: 30
    }
  };

  return intensityData[intensity];
}
