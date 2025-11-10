/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Core Type Definitions
 *
 * Historical Artillery System for WWI Verdun Battle (1916)
 * "Artillery conquers, infantry occupies" - General Nivelle
 */

// ============================================================================
// CORE TYPES
// ============================================================================

export interface Position3D {
  x: number;  // meters
  y: number;  // meters (elevation)
  z: number;  // meters
}

export interface Vector3D {
  x: number;
  y: number;
  z: number;
}

// ============================================================================
// SHELL TYPES
// ============================================================================

export enum ShellType {
  // Light Artillery (60% of shells)
  FRENCH_75MM = "75mm_french",
  GERMAN_77MM = "77mm_german",

  // Medium Artillery (25% of shells)
  HOWITZER_155MM = "155mm_howitzer",

  // Heavy Artillery (10% of shells)
  GERMAN_210MM = "210mm_german",
  FRENCH_305MM = "305mm_french",

  // Super-Heavy Artillery (2% of shells)
  GERMAN_420MM = "420mm_german",

  // Gas Shells (5% of shells)
  PHOSGENE_GAS = "phosgene_gas",
  MUSTARD_GAS = "mustard_gas",

  // Special
  SHRAPNEL = "shrapnel",
  SMOKE = "smoke",
  ILLUMINATION = "illumination"
}

export enum ShellTier {
  LIGHT = 1,      // 60% of shells
  MEDIUM = 2,     // 25% of shells
  HEAVY = 3,      // 10% of shells
  SUPER_HEAVY = 4 // 2% of shells
}

// ============================================================================
// AUDIO CHARACTERISTICS
// ============================================================================

export interface AudioSignature {
  incomingSound: string;          // Description of incoming sound
  warnTimeSeconds: number;        // Warning time in seconds (min)
  warnTimeSecondsMax: number;     // Warning time in seconds (max)
  explosionSound: string;         // Description of explosion sound
  recognizability: string;        // How distinctive the sound is
  frequencyHz: number;            // Dominant frequency for audio system
  volumeDb: number;               // Volume at source
}

// ============================================================================
// BALLISTICS
// ============================================================================

export interface BallisticsData {
  muzzleVelocity: number;     // m/s
  rangeMax: number;           // meters
  flightTimeTypical: number;  // seconds
  trajectory: 'flat' | 'high_arc' | 'very_high_arc';
  shellWeight: number;        // kg
}

// ============================================================================
// DAMAGE SYSTEM
// ============================================================================

export interface DamageRadii {
  lethal: number;           // meters - 0% survival
  catastrophic: number;     // meters - <10% survival
  severe: number;           // meters - 40-60% survival
  moderate: number;         // meters - 75-90% survival
  light: number;            // meters - 95-98% survival
  shellShock: number;       // meters - psychological impact
  buriedAlive?: number;     // meters - burial risk
  debris: number;           // meters - debris falls
}

export interface CraterData {
  diameter: number;  // meters
  depth: number;     // meters
}

export interface Penetration {
  sandbagWall: string;    // Description of penetration
  earthDugout: string;    // Description of penetration
  concrete: string;       // Description of penetration
  trenchWall: string;     // Description of penetration
}

export interface DamageByDistance {
  distance: number;          // meters from impact
  survivalChance: number;    // 0-100 percentage
  wounds: string;            // Description of wounds
  playerExperience: string;  // Game effect description
  medicalRequired: string;   // Medical attention needed
  specialEffects?: string[]; // e.g., burial, shell shock
}

// ============================================================================
// COVER TYPES
// ============================================================================

export enum CoverType {
  OPEN_GROUND = "open_ground",
  SHELL_CRATER = "shell_crater",
  SHALLOW_TRENCH = "shallow_trench",
  DUGOUT_ENTRANCE = "dugout_entrance",
  DEEP_DUGOUT = "deep_dugout",
  CONCRETE_BUNKER = "concrete_bunker",
  WOOD_BUNKER = "wood_bunker",
  SANDBAG_WALL = "sandbag_wall"
}

export interface CoverModifier {
  coverType: CoverType;
  protection: number;  // 0-100 percentage reduction in damage
  description: string;
}

// ============================================================================
// COMPLETE SHELL DEFINITION
// ============================================================================

export interface ShellDefinition {
  designation: string;
  type: ShellType;
  tier: ShellTier;
  usage: string;
  percentageOfTotal: number;      // What % of all shells fired
  shellsFiredVerdun: number;      // Historical total at Verdun

  audio: AudioSignature;
  ballistics: BallisticsData;
  damage: DamageRadii;
  crater: CraterData;
  penetration: Penetration;
  damageByDistance: DamageByDistance[];
  coverModifiers: CoverModifier[];

  // Special mechanics
  specialMechanics?: {
    burialAlive?: {
      trigger: string;
      chance: number;
      effect: string;
      oxygenTimer: number;
      escapeMechanic: string;
    };
    gasCloud?: {
      radius: number;
      durationMinutes: number;
      effectsWithoutMask: any;
    };
    trenchCollapse?: {
      radius: number;
      effect: string;
    };
  };

  gameplayNotes: {
    frequency: string;
    reactionWindow: string;
    primaryThreat: string;
    counterStrategy: string;
    terrorFactor?: string;
  };
}

// ============================================================================
// BOMBARDMENT PATTERNS
// ============================================================================

export enum BombardmentIntensity {
  HARASSING_FIRE = "harassing_fire",      // 1-3 shells/min, constant
  LIGHT = "light_bombardment",             // 10-20 shells/min
  HEAVY = "heavy_bombardment",             // 40-60 shells/min
  DRUMFIRE = "drumfire",                   // 100+ shells/min (Trommelfeuer)
}

export enum BombardmentPattern {
  RANDOM = "random",                       // Random shell placement
  CREEPING_BARRAGE = "creeping_barrage",   // Advancing wall of fire
  BOX_BARRAGE = "box_barrage",            // Square pattern to trap
  CONCENTRATION = "concentration",         // Multiple batteries on one point
  COUNTER_BATTERY = "counter_battery"      // Targeting enemy guns
}

export interface BombardmentConfig {
  intensity: BombardmentIntensity;
  pattern: BombardmentPattern;
  duration: number;              // seconds
  targetArea: {
    center: Position3D;
    radius: number;              // meters
    length?: number;             // for creeping barrage
    width?: number;              // for box barrage
  };
  shellMix: {
    [key in ShellType]?: number; // percentage of each shell type
  };
  shellsPerMinute: number;
  casualtiesPerHourPer1000: number; // Historical casualty rate
}

// ============================================================================
// FIRE MISSION
// ============================================================================

export enum FireMissionType {
  HARASSING = "harassing",
  PREPARATORY = "preparatory",         // Before assault
  SUPPRESSION = "suppression",
  INTERDICTION = "interdiction",       // Block reinforcements
  COUNTER_BATTERY = "counter_battery",
  FINAL_PROTECTIVE = "final_protective", // Emergency defense
  CREEPING_BARRAGE = "creeping_barrage"
}

export enum FireMissionStatus {
  PLANNED = "planned",
  IN_PROGRESS = "in_progress",
  COMPLETE = "complete",
  CANCELLED = "cancelled"
}

export interface FireMission {
  id: string;
  type: FireMissionType;
  status: FireMissionStatus;

  target: {
    position: Position3D;
    description: string;
    priority: number;  // 1-5, 5 being highest
  };

  batteries: string[];  // IDs of batteries assigned
  bombardment: BombardmentConfig;

  startTime: number;    // Game timestamp
  endTime: number;      // Game timestamp

  observer?: {
    id: string;
    position: Position3D;
    accuracy: number;   // 0-100 percentage
  };
}

// ============================================================================
// ARTILLERY BATTERY
// ============================================================================

export interface ArtilleryGun {
  id: string;
  type: ShellType;
  position: Position3D;
  ammunition: number;
  rateOfFire: number;  // rounds per minute
  crew: number;        // number of crew members
  operational: boolean;
  lastFired: number;   // timestamp
}

export interface ArtilleryBattery {
  id: string;
  name: string;
  nationality: 'french' | 'german';
  guns: ArtilleryGun[];
  position: Position3D;
  range: number;       // maximum range in meters
  ammunition: {
    [key in ShellType]?: number;
  };
  currentMission?: string;  // FireMission ID
  commandPost: {
    position: Position3D;
    telephone: boolean;
    radio: boolean;
  };
}

// ============================================================================
// IMPACT EVENT
// ============================================================================

export interface ImpactEvent {
  id: string;
  shellType: ShellType;
  impactPosition: Position3D;
  impactTime: number;

  // Calculated data
  crater: CraterData;
  damageRadii: DamageRadii;

  // Affected entities
  casualties: {
    killed: string[];      // Entity IDs
    wounded: string[];     // Entity IDs
    shellShocked: string[]; // Entity IDs
    buried: string[];      // Entity IDs
  };

  // Environmental effects
  trenchesDestroyed: string[];
  structuresDestroyed: string[];

  // Audio/visual
  audioRange: number;    // meters - how far sound travels
  visualRange: number;   // meters - how far flash/smoke visible
}

// ============================================================================
// SHELL IN FLIGHT
// ============================================================================

export interface ShellInFlight {
  id: string;
  shellType: ShellType;
  definition: ShellDefinition;

  // Trajectory
  origin: Position3D;
  target: Position3D;
  currentPosition: Position3D;
  velocity: Vector3D;

  // Timing
  launchTime: number;
  impactTime: number;
  timeRemaining: number;

  // Audio
  audioSignature: AudioSignature;
  audibleRange: number;  // meters - how far it can be heard

  // Source
  firingBattery: string;  // Battery ID
  fireMission?: string;   // FireMission ID
}

// ============================================================================
// PLAYER DETECTION
// ============================================================================

export interface PlayerDetection {
  shellId: string;
  shellType: ShellType;

  // Distance/timing
  distanceToPlayer: number;      // meters
  timeToImpact: number;          // seconds
  impactPosition: Position3D;

  // Audio detection
  isAudible: boolean;
  audioVolume: number;           // 0-100
  audioDirection: Vector3D;      // 3D direction to shell

  // Threat assessment
  threatLevel: 'none' | 'low' | 'moderate' | 'high' | 'extreme';
  recommendedAction: string;

  // Player skill-based
  recognitionDifficulty: number; // 0-100, based on player experience
  isRecognized: boolean;         // Did player identify shell type?
}

// ============================================================================
// PLAYER EXPERIENCE LEVELS
// ============================================================================

export enum PlayerExperience {
  NOVICE = "novice",           // First missions
  EXPERIENCED = "experienced", // 10+ hours
  VETERAN = "veteran"          // 30+ hours, late campaign
}

export interface PlayerSkills {
  experience: PlayerExperience;
  shellsRecognized: number;
  shellsSurvived: number;
  recognitionAccuracy: number;     // 0-100 percentage
  recognitionTime: number;         // seconds to identify
  panicChance: number;             // 0-100 percentage

  // Modifiers
  morale: number;                  // 0-100
  stamina: number;                 // 0-100
  alertness: number;               // 0-100
  shellShock: boolean;
}

// ============================================================================
// GAME STATE
// ============================================================================

export interface ArtilleryGameState {
  currentTime: number;             // Game timestamp (seconds)

  // Active elements
  shellsInFlight: Map<string, ShellInFlight>;
  activeMissions: Map<string, FireMission>;
  batteries: Map<string, ArtilleryBattery>;

  // Impact history
  recentImpacts: ImpactEvent[];

  // Statistics
  stats: {
    totalShellsFired: number;
    shellsFiredByType: { [key in ShellType]?: number };
    totalCasualties: number;
    cratersCreated: number;
  };

  // Player
  playerPosition: Position3D;
  playerSkills: PlayerSkills;
  playerCover: CoverType;

  // Environmental
  windSpeed: number;               // m/s
  windDirection: number;           // degrees
  temperature: number;             // celsius
  visibility: number;              // meters
}

// ============================================================================
// CONFIGURATION
// ============================================================================

export interface ArtilleryEngineConfig {
  // Simulation
  tickRate: number;                // Updates per second
  maxShellsInFlight: number;
  maxSimulationDistance: number;   // meters - shells beyond this aren't simulated

  // Audio
  audioEnabled: boolean;
  spatialAudioEnabled: boolean;
  maxAudioSources: number;

  // Difficulty
  playerSkillProgression: boolean; // Does player improve over time?
  shellShockEnabled: boolean;
  permadeathMode: boolean;

  // Historical accuracy
  historicalShellMix: boolean;     // Use historical percentages?
  historicalCasualtyRates: boolean;

  // Performance
  detailedPhysics: boolean;        // Full ballistics vs. simplified
  particleEffects: boolean;
}
