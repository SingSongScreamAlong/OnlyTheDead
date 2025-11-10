/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Player Detection & Audio Recognition System
 *
 * Handles player detection of incoming shells and audio-based recognition
 */

import {
  ShellInFlight,
  PlayerDetection,
  PlayerSkills,
  PlayerExperience,
  Position3D,
  ShellDefinition
} from './types';
import { distance3D, calculateWarningTime, calculateAudioProperties } from './ballistics';

// ============================================================================
// THREAT ASSESSMENT
// ============================================================================

/**
 * Assess threat level to player from incoming shell
 */
export function assessThreatLevel(
  shell: ShellInFlight,
  playerPosition: Position3D,
  playerSkills: PlayerSkills
): 'none' | 'low' | 'moderate' | 'high' | 'extreme' {
  const distanceToImpact = distance3D(shell.target, playerPosition);

  const shellDef = shell.definition;

  // Determine threat based on distance to impact point
  if (distanceToImpact <= shellDef.damage.lethal) {
    return 'extreme'; // Lethal range
  } else if (distanceToImpact <= shellDef.damage.severe) {
    return 'high'; // Severe wound range
  } else if (distanceToImpact <= shellDef.damage.moderate) {
    return 'moderate'; // Moderate wound range
  } else if (distanceToImpact <= shellDef.damage.light) {
    return 'low'; // Light wound range
  } else {
    return 'none'; // Outside danger zone
  }
}

/**
 * Recommend action to player based on threat
 */
export function recommendAction(
  threatLevel: 'none' | 'low' | 'moderate' | 'high' | 'extreme',
  timeToImpact: number,
  shellDef: ShellDefinition
): string {
  if (threatLevel === 'none') {
    return "Continue task";
  }

  if (timeToImpact < 1) {
    return "Brace for impact";
  }

  if (threatLevel === 'extreme') {
    if (timeToImpact < 3) {
      return "Drop prone immediately!";
    } else if (timeToImpact < 8) {
      return "Sprint to nearest deep cover!";
    } else {
      return "Seek deepest bunker or dugout!";
    }
  }

  if (threatLevel === 'high') {
    if (timeToImpact < 2) {
      return "Drop prone!";
    } else if (timeToImpact < 5) {
      return "Sprint to crater or trench!";
    } else {
      return "Move to dugout!";
    }
  }

  if (threatLevel === 'moderate') {
    if (timeToImpact < 3) {
      return "Take cover - crouch or prone!";
    } else {
      return "Move to nearest cover";
    }
  }

  if (threatLevel === 'low') {
    return "Stay alert, continue task";
  }

  return "Continue task";
}

/**
 * Detect shells audible to player
 */
export function detectAudibleShells(
  shellsInFlight: ShellInFlight[],
  playerPosition: Position3D,
  playerSkills: PlayerSkills,
  currentTime: number
): PlayerDetection[] {
  const detections: PlayerDetection[] = [];

  for (const shell of shellsInFlight) {
    // Check if shell is audible
    const audio = calculateAudioProperties(
      shell.currentPosition,
      playerPosition,
      shell.velocity,
      shell.definition
    );

    if (!audio.isAudible) continue;

    const distanceToPlayer = distance3D(shell.currentPosition, playerPosition);
    const distanceToImpact = distance3D(shell.target, playerPosition);
    const timeToImpact = shell.timeRemaining;

    // Calculate warning time
    const warningTime = calculateWarningTime(
      shell.currentPosition,
      playerPosition,
      shell.definition,
      timeToImpact
    );

    // Assess threat
    const threatLevel = assessThreatLevel(shell, playerPosition, playerSkills);

    // Determine if player can recognize shell type
    const recognitionDifficulty = calculateRecognitionDifficulty(
      shell.definition,
      distanceToPlayer,
      playerSkills
    );

    const isRecognized = attemptRecognition(recognitionDifficulty, playerSkills);

    // Get recommended action
    const recommendedAction = recommendAction(threatLevel, timeToImpact, shell.definition);

    detections.push({
      shellId: shell.id,
      shellType: shell.shellType,
      distanceToPlayer: distanceToImpact,
      timeToImpact,
      impactPosition: shell.target,
      isAudible: true,
      audioVolume: audio.volume,
      audioDirection: audio.direction,
      threatLevel,
      recommendedAction,
      recognitionDifficulty,
      isRecognized
    });
  }

  return detections;
}

/**
 * Calculate difficulty of recognizing shell type
 */
export function calculateRecognitionDifficulty(
  shellDef: ShellDefinition,
  distanceToShell: number,
  playerSkills: PlayerSkills
): number {
  // Base difficulty starts at 50
  let difficulty = 50;

  // Distance affects recognition (harder when farther)
  const distanceFactor = Math.min(100, distanceToShell / 10);
  difficulty += distanceFactor * 0.2;

  // Player experience reduces difficulty
  if (playerSkills.experience === PlayerExperience.VETERAN) {
    difficulty *= 0.4;
  } else if (playerSkills.experience === PlayerExperience.EXPERIENCED) {
    difficulty *= 0.7;
  }

  // Morale affects recognition (panic makes it harder)
  const moralePenalty = (100 - playerSkills.morale) / 100;
  difficulty *= (1 + moralePenalty * 0.5);

  // Alertness affects recognition
  const alertnessPenalty = (100 - playerSkills.alertness) / 100;
  difficulty *= (1 + alertnessPenalty * 0.3);

  // Shell shock makes recognition very difficult
  if (playerSkills.shellShock) {
    difficulty *= 2.0;
  }

  // Some shells are more distinctive
  if (shellDef.audio.recognizability.includes("Very distinctive")) {
    difficulty *= 0.8;
  } else if (shellDef.audio.recognizability.includes("Unmistakable")) {
    difficulty *= 0.6;
  }

  return Math.max(0, Math.min(100, difficulty));
}

/**
 * Attempt to recognize shell type
 */
export function attemptRecognition(
  difficulty: number,
  playerSkills: PlayerSkills
): boolean {
  // Recognition chance based on player accuracy and difficulty
  const baseChance = playerSkills.recognitionAccuracy;
  const adjustedChance = baseChance * (1 - difficulty / 100);

  return Math.random() * 100 < adjustedChance;
}

/**
 * Update player skills based on experience
 */
export function updatePlayerSkills(
  playerSkills: PlayerSkills,
  shellRecognized: boolean,
  shellSurvived: boolean
): PlayerSkills {
  const updated = { ...playerSkills };

  if (shellSurvived) {
    updated.shellsSurvived++;
  }

  if (shellRecognized) {
    updated.shellsRecognized++;
  }

  // Update recognition accuracy based on performance
  const totalAttempts = updated.shellsSurvived;
  const successRate = updated.shellsRecognized / Math.max(1, totalAttempts);
  updated.recognitionAccuracy = Math.min(95, successRate * 100);

  // Update recognition time (improves with experience)
  if (totalAttempts > 50 && updated.experience === PlayerExperience.NOVICE) {
    updated.experience = PlayerExperience.EXPERIENCED;
    updated.recognitionTime = 1.0;
    updated.panicChance = 40;
  }

  if (totalAttempts > 200 && updated.experience === PlayerExperience.EXPERIENCED) {
    updated.experience = PlayerExperience.VETERAN;
    updated.recognitionTime = 0.3;
    updated.panicChance = 15;
  }

  return updated;
}

/**
 * Calculate player reaction time based on experience
 */
export function calculateReactionTime(
  playerSkills: PlayerSkills,
  threatLevel: 'none' | 'low' | 'moderate' | 'high' | 'extreme'
): number {
  let baseReactionTime = playerSkills.recognitionTime;

  // Threat level affects reaction time
  if (threatLevel === 'extreme') {
    baseReactionTime *= 0.7; // Adrenaline speeds up reaction
  } else if (threatLevel === 'none') {
    baseReactionTime *= 1.5; // Less urgent, slower reaction
  }

  // Alertness affects reaction time
  const alertnessFactor = playerSkills.alertness / 100;
  baseReactionTime *= (2 - alertnessFactor); // Low alertness = slower

  // Stamina affects reaction time
  const staminaFactor = playerSkills.stamina / 100;
  baseReactionTime *= (2 - staminaFactor); // Low stamina = slower

  // Shell shock dramatically slows reaction
  if (playerSkills.shellShock) {
    baseReactionTime *= 2.5;
  }

  return baseReactionTime;
}

/**
 * Determine if player panics
 */
export function checkForPanic(
  playerSkills: PlayerSkills,
  threatLevel: 'none' | 'low' | 'moderate' | 'high' | 'extreme',
  multipleShells: boolean
): boolean {
  let panicChance = playerSkills.panicChance;

  // Extreme threats increase panic chance
  if (threatLevel === 'extreme') {
    panicChance *= 2.0;
  } else if (threatLevel === 'high') {
    panicChance *= 1.5;
  }

  // Multiple simultaneous shells increase panic
  if (multipleShells) {
    panicChance *= 1.5;
  }

  // Low morale increases panic
  const moraleFactor = (100 - playerSkills.morale) / 100;
  panicChance *= (1 + moraleFactor);

  return Math.random() * 100 < panicChance;
}

/**
 * Create audio cue for player
 */
export function createAudioCue(
  detection: PlayerDetection,
  playerSkills: PlayerSkills
): {
  soundDescription: string;
  volume: number;
  pitch: number;
  direction: string;
  warningLevel: 'info' | 'warning' | 'danger' | 'critical';
} {
  const shellDef = detection.isRecognized
    ? ` (${detection.shellType})`
    : "";

  let soundDescription = `Incoming shell${shellDef}`;

  let warningLevel: 'info' | 'warning' | 'danger' | 'critical' = 'info';

  if (detection.threatLevel === 'extreme') {
    soundDescription = `INCOMING HEAVY SHELL! ${detection.timeToImpact.toFixed(1)}s`;
    warningLevel = 'critical';
  } else if (detection.threatLevel === 'high') {
    soundDescription = `Incoming shell danger close! ${detection.timeToImpact.toFixed(1)}s`;
    warningLevel = 'danger';
  } else if (detection.threatLevel === 'moderate') {
    soundDescription = `Incoming shell nearby ${detection.timeToImpact.toFixed(1)}s`;
    warningLevel = 'warning';
  }

  // Direction description
  let direction = "unknown";
  const dir = detection.audioDirection;
  const angle = Math.atan2(dir.z, dir.x) * (180 / Math.PI);

  if (angle > -22.5 && angle <= 22.5) direction = "east";
  else if (angle > 22.5 && angle <= 67.5) direction = "northeast";
  else if (angle > 67.5 && angle <= 112.5) direction = "north";
  else if (angle > 112.5 && angle <= 157.5) direction = "northwest";
  else if (angle > 157.5 || angle <= -157.5) direction = "west";
  else if (angle > -157.5 && angle <= -112.5) direction = "southwest";
  else if (angle > -112.5 && angle <= -67.5) direction = "south";
  else if (angle > -67.5 && angle <= -22.5) direction = "southeast";

  return {
    soundDescription,
    volume: detection.audioVolume,
    pitch: 440, // Placeholder - would use actual audio signature
    direction,
    warningLevel
  };
}

/**
 * Filter detections by priority for UI display
 */
export function prioritizeDetections(
  detections: PlayerDetection[],
  maxDisplayed: number = 3
): PlayerDetection[] {
  // Sort by threat level and time to impact
  const threatPriority = {
    extreme: 5,
    high: 4,
    moderate: 3,
    low: 2,
    none: 1
  };

  return detections
    .sort((a, b) => {
      const threatDiff = threatPriority[b.threatLevel] - threatPriority[a.threatLevel];
      if (threatDiff !== 0) return threatDiff;

      // If same threat level, prioritize by time to impact
      return a.timeToImpact - b.timeToImpact;
    })
    .slice(0, maxDisplayed);
}

/**
 * Calculate player stress from bombardment
 */
export function calculateBombardmentStress(
  detections: PlayerDetection[],
  playerSkills: PlayerSkills,
  duration: number // seconds under bombardment
): {
  stressLevel: number; // 0-100
  moraleLoss: number;
  shellShockRisk: number;
} {
  let stressLevel = 0;
  let moraleLoss = 0;
  let shellShockRisk = 0;

  // Base stress from number of shells
  stressLevel += detections.length * 5;

  // High threat shells cause more stress
  for (const detection of detections) {
    if (detection.threatLevel === 'extreme') {
      stressLevel += 20;
      moraleLoss += 10;
      shellShockRisk += 5;
    } else if (detection.threatLevel === 'high') {
      stressLevel += 10;
      moraleLoss += 5;
      shellShockRisk += 2;
    }
  }

  // Prolonged bombardment increases stress
  const durationFactor = Math.min(2, duration / 3600); // Max 2x at 1 hour
  stressLevel *= durationFactor;

  // Veterans handle stress better
  if (playerSkills.experience === PlayerExperience.VETERAN) {
    stressLevel *= 0.5;
    shellShockRisk *= 0.5;
  } else if (playerSkills.experience === PlayerExperience.EXPERIENCED) {
    stressLevel *= 0.75;
    shellShockRisk *= 0.75;
  }

  return {
    stressLevel: Math.min(100, stressLevel),
    moraleLoss: Math.min(50, moraleLoss),
    shellShockRisk: Math.min(100, shellShockRisk)
  };
}

/**
 * Create initial player skills
 */
export function createPlayerSkills(experience: PlayerExperience = PlayerExperience.NOVICE): PlayerSkills {
  const baseSkills = {
    [PlayerExperience.NOVICE]: {
      recognitionAccuracy: 50,
      recognitionTime: 3.0,
      panicChance: 60
    },
    [PlayerExperience.EXPERIENCED]: {
      recognitionAccuracy: 80,
      recognitionTime: 1.0,
      panicChance: 40
    },
    [PlayerExperience.VETERAN]: {
      recognitionAccuracy: 95,
      recognitionTime: 0.3,
      panicChance: 15
    }
  };

  const base = baseSkills[experience];

  return {
    experience,
    shellsRecognized: 0,
    shellsSurvived: 0,
    recognitionAccuracy: base.recognitionAccuracy,
    recognitionTime: base.recognitionTime,
    panicChance: base.panicChance,
    morale: 80,
    stamina: 80,
    alertness: 70,
    shellShock: false
  };
}
