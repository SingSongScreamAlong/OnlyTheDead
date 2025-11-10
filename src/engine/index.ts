/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Main Export Index
 *
 * "Artillery conquers, infantry occupies" - General Nivelle
 *
 * Complete WWI artillery simulation engine for the Battle of Verdun (1916)
 */

// ============================================================================
// MAIN ENGINE
// ============================================================================

export {
  ArtilleryEngine,
  createArtilleryEngine,
  createVerdunScenario
} from './artilleryEngine';

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

export type {
  // Core Types
  Position3D,
  Vector3D,

  // Shell Types
  ShellDefinition,
  AudioSignature,
  BallisticsData,
  DamageRadii,
  CraterData,
  Penetration,
  DamageByDistance,
  CoverModifier,

  // Bombardment
  BombardmentConfig,

  // Fire Missions
  FireMission,
  ArtilleryBattery,
  ArtilleryGun,

  // Player
  PlayerSkills,
  PlayerDetection,

  // Events
  ImpactEvent,
  ShellInFlight,

  // Game State
  ArtilleryGameState,
  ArtilleryEngineConfig
} from './types';

export {
  // Enums
  ShellType,
  ShellTier,
  CoverType,
  BombardmentIntensity,
  BombardmentPattern,
  FireMissionType,
  FireMissionStatus,
  PlayerExperience
} from './types';

// ============================================================================
// SHELL SYSTEM
// ============================================================================

export {
  // Shell Definitions
  FRENCH_75MM,
  GERMAN_77MM,
  HOWITZER_155MM,
  GERMAN_210MM,
  FRENCH_305MM,
  GERMAN_420MM,

  // Shell Registry
  SHELL_REGISTRY,
  getShellDefinition,
  getAllShellDefinitions,
  getShellsByTier,
  VERDUN_HISTORICAL_SHELL_MIX
} from './shells';

// ============================================================================
// BALLISTICS SYSTEM
// ============================================================================

export {
  // Calculations
  distance3D,
  distance2D,
  normalize,
  magnitude,
  addVectors,
  scaleVector,

  // Ballistics
  calculateFiringAngle,
  calculateFlightTime,
  calculateInitialVelocity,
  calculateShellPosition,
  calculateShellVelocity,
  calculateDetailedTrajectory,
  calculateImpactPoint,
  estimateFlightTime,
  calculateWarningTime,
  calculateAudioProperties,

  // Effects
  applyWindEffect,
  applyDispersion,
  willHitTarget,

  // Shell Management
  createShellInFlight,
  updateShellInFlight
} from './ballistics';

// ============================================================================
// DAMAGE SYSTEM
// ============================================================================

export {
  // Damage Calculations
  calculateDamage,
  determineSurvival,
  calculateShellShockProbability,
  calculateBurialProbability,

  // Environmental
  calculateCrater,
  calculateTrenchDestruction,
  calculateOverpressureEffects,
  calculateFragmentationDamage,

  // Medical
  determineMedicalRequirements,

  // Psychological
  calculatePsychologicalImpact,

  // Events
  createImpactEvent,
  calculateAreaCasualties
} from './damage';

// ============================================================================
// BOMBARDMENT SYSTEM
// ============================================================================

export {
  // Configuration
  INTENSITY_CONFIGS,

  // Shell Selection
  selectShellType,
  getHistoricalShellMix,
  createShellMix,

  // Pattern Generators
  generateRandomTargets,
  generateCreepingBarrageTargets,
  generateBoxBarrageTargets,
  generateConcentrationTargets,
  generateCounterBatteryTargets,

  // Execution
  createBombardmentSchedule,
  executeBombardmentStep,
  createStandardBombardment,

  // Historical
  HISTORICAL_BOMBARDMENTS,
  calculateExpectedCasualties
} from './bombardment';

// ============================================================================
// FIRE MISSION SYSTEM
// ============================================================================

export {
  // Mission Creation
  createFireMission,
  createCounterBatteryMission,
  createCreepingBarrageMission,
  createPreparatoryBombardment,

  // Battery Management
  createArtilleryBattery,
  canReachTarget,
  getBatteryFireRate,
  hasAmmoForMission,
  assignBatteryToMission,

  // Mission Control
  startFireMission,
  completeFireMission,
  cancelFireMission,

  // Coordination
  coordinateBatteriesForMission,
  calculateTimeOnTarget,
  synchronizeBatteries,
  distributeFire,

  // Logistics
  resupplyBattery,
  consumeAmmunition,
  updateBatteryStatus,
  calculateBatteryEffectiveness,

  // Observer
  adjustFire,
  calculateObserverAccuracy
} from './fireMission';

// ============================================================================
// PLAYER DETECTION SYSTEM
// ============================================================================

export {
  // Threat Assessment
  assessThreatLevel,
  recommendAction,
  detectAudibleShells,

  // Recognition
  calculateRecognitionDifficulty,
  attemptRecognition,

  // Skills
  updatePlayerSkills,
  calculateReactionTime,
  checkForPanic,
  createPlayerSkills,

  // Audio
  createAudioCue,
  prioritizeDetections,

  // Stress
  calculateBombardmentStress
} from './playerDetection';

// ============================================================================
// EXAMPLES
// ============================================================================

export {
  example1_BasicSetup,
  example2_SimpleBombardment,
  example3_PlayerDetection,
  example4_February21Bombardment,
  example5_CreepingBarrage,
  example6_CounterBattery,
  runAllExamples
} from './examples';

// ============================================================================
// VERSION
// ============================================================================

export const VERSION = '1.0.0';
export const ENGINE_NAME = 'King of Battle Artillery Engine';
export const HISTORICAL_PERIOD = 'Battle of Verdun, February-December 1916';

// ============================================================================
// QUICK START
// ============================================================================

/**
 * Quick start example
 *
 * ```typescript
 * import { createArtilleryEngine, createArtilleryBattery, ShellType } from '@engine';
 *
 * const engine = createArtilleryEngine();
 *
 * const battery = createArtilleryBattery(
 *   "French 75mm",
 *   "french",
 *   ShellType.FRENCH_75MM,
 *   6,
 *   { x: -1000, y: 0, z: 0 },
 *   11000
 * );
 *
 * engine.addBattery(battery);
 *
 * // Game loop
 * function update(deltaTime: number) {
 *   engine.update(deltaTime);
 *   const detections = engine.getPlayerDetections();
 *   // Handle detections...
 * }
 * ```
 */
