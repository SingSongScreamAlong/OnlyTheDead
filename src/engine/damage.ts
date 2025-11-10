/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Damage Calculation System
 *
 * Handles damage calculations, cover effects, and casualty determination
 */

import {
  Position3D,
  ShellDefinition,
  DamageByDistance,
  CoverType,
  ImpactEvent,
  PlayerSkills,
  ShellType
} from './types';
import { distance3D, distance2D } from './ballistics';
import { getShellDefinition } from './shells';

// ============================================================================
// DAMAGE CALCULATION
// ============================================================================

/**
 * Calculate damage for an entity at given distance from impact
 */
export function calculateDamage(
  impactPosition: Position3D,
  targetPosition: Position3D,
  shellDef: ShellDefinition,
  coverType: CoverType = CoverType.OPEN_GROUND,
  targetElevation: number = 0
): {
  distance: number;
  survivalChance: number;
  wounds: string;
  isLethal: boolean;
  isCatastrophic: boolean;
  shellShock: boolean;
  buriedAlive: boolean;
  coverProtection: number;
  finalSurvivalChance: number;
} {
  // Calculate distance to impact
  const distance = distance2D(impactPosition, targetPosition);

  // Get cover modifier
  const coverMod = shellDef.coverModifiers.find(c => c.coverType === coverType);
  const coverProtection = coverMod ? coverMod.protection : 0;

  // Determine damage tier based on distance
  let baseSurvivalChance = 100;
  let wounds = "None";
  let shellShock = false;
  let buriedAlive = false;

  if (distance <= shellDef.damage.lethal) {
    baseSurvivalChance = 0;
    wounds = "Instant death (vaporization/massive trauma)";
  } else if (distance <= shellDef.damage.catastrophic) {
    baseSurvivalChance = 5;
    wounds = "Catastrophic - multiple amputations, internal rupture";
  } else if (distance <= shellDef.damage.severe) {
    baseSurvivalChance = 50;
    wounds = "Severe - major shrapnel, broken bones, concussion";
  } else if (distance <= shellDef.damage.moderate) {
    baseSurvivalChance = 80;
    wounds = "Moderate - shrapnel wounds, concussion";
  } else if (distance <= shellDef.damage.light) {
    baseSurvivalChance = 95;
    wounds = "Light - possible fragment hits";
  }

  // Check for shell shock
  if (distance <= shellDef.damage.shellShock) {
    shellShock = true;
  }

  // Check for burial alive
  if (shellDef.damage.buriedAlive && distance <= shellDef.damage.buriedAlive) {
    if (coverType === CoverType.SHALLOW_TRENCH || coverType === CoverType.SHELL_CRATER) {
      const burialChance = ((shellDef.damage.buriedAlive - distance) / shellDef.damage.buriedAlive) * 100;
      buriedAlive = Math.random() * 100 < burialChance;
    }
  }

  // Apply cover protection
  const coverFactor = coverProtection / 100;
  const damageReduction = (100 - baseSurvivalChance) * coverFactor;
  const finalSurvivalChance = Math.min(100, baseSurvivalChance + damageReduction);

  return {
    distance,
    survivalChance: baseSurvivalChance,
    wounds,
    isLethal: distance <= shellDef.damage.lethal,
    isCatastrophic: distance <= shellDef.damage.catastrophic,
    shellShock,
    buriedAlive,
    coverProtection,
    finalSurvivalChance
  };
}

/**
 * Determine if entity survives based on survival chance
 */
export function determineSurvival(survivalChance: number): boolean {
  return Math.random() * 100 < survivalChance;
}

/**
 * Calculate shell shock probability
 */
export function calculateShellShockProbability(
  distance: number,
  shellDef: ShellDefinition,
  currentMorale: number,
  isVeteran: boolean
): number {
  if (distance > shellDef.damage.shellShock) {
    return 0;
  }

  // Base probability increases as distance decreases
  const distanceFactor = 1 - (distance / shellDef.damage.shellShock);
  let baseProbability = distanceFactor * 50; // Up to 50% at ground zero

  // Morale affects shell shock resistance
  const moraleFactor = (100 - currentMorale) / 100;
  baseProbability *= (1 + moraleFactor);

  // Veterans have better resistance
  if (isVeteran) {
    baseProbability *= 0.6;
  }

  return Math.min(100, baseProbability);
}

/**
 * Calculate burial alive probability
 */
export function calculateBurialProbability(
  distance: number,
  shellDef: ShellDefinition,
  coverType: CoverType
): number {
  if (!shellDef.damage.buriedAlive || distance > shellDef.damage.buriedAlive) {
    return 0;
  }

  // Only certain cover types can result in burial
  const burialProneCover = [
    CoverType.SHALLOW_TRENCH,
    CoverType.SHELL_CRATER,
    CoverType.DUGOUT_ENTRANCE
  ];

  if (!burialProneCover.includes(coverType)) {
    return 0;
  }

  // Probability increases with shell size and proximity
  const distanceFactor = 1 - (distance / shellDef.damage.buriedAlive);
  const baseProbability = distanceFactor * 60; // Up to 60% at closest range

  // Adjust by cover type
  let coverMultiplier = 1.0;
  if (coverType === CoverType.SHALLOW_TRENCH) {
    coverMultiplier = 1.2; // Trenches more prone to collapse
  } else if (coverType === CoverType.SHELL_CRATER) {
    coverMultiplier = 0.8; // Craters less prone
  }

  return Math.min(100, baseProbability * coverMultiplier);
}

/**
 * Calculate crater created by shell impact
 */
export function calculateCrater(
  impactPosition: Position3D,
  shellDef: ShellDefinition
): {
  position: Position3D;
  diameter: number;
  depth: number;
  volume: number;
} {
  return {
    position: impactPosition,
    diameter: shellDef.crater.diameter,
    depth: shellDef.crater.depth,
    volume: Math.PI * Math.pow(shellDef.crater.diameter / 2, 2) * shellDef.crater.depth
  };
}

/**
 * Calculate trench destruction from shell impact
 */
export function calculateTrenchDestruction(
  impactPosition: Position3D,
  shellDef: ShellDefinition,
  trenchPositions: Position3D[]
): Position3D[] {
  const destroyedSections: Position3D[] = [];

  // Destruction radius based on shell size
  const destructionRadius = shellDef.crater.diameter * 2;

  for (const trenchPos of trenchPositions) {
    const distance = distance2D(impactPosition, trenchPos);
    if (distance <= destructionRadius) {
      destroyedSections.push(trenchPos);
    }
  }

  return destroyedSections;
}

/**
 * Calculate overpressure effects (for enclosed spaces)
 */
export function calculateOverpressureEffects(
  distance: number,
  shellDef: ShellDefinition,
  isEnclosed: boolean
): {
  overpressure: number; // PSI
  rupturedEardrums: boolean;
  lungDamage: boolean;
  lethal: boolean;
} {
  // Overpressure formula (simplified)
  // P = P0 * (R0/R)^3 where P0 is base overpressure at R0

  const baseOverpressure = 50; // PSI at 1m for typical shell
  const basePressure = baseOverpressure * Math.pow(1 / Math.max(1, distance), 3);

  // Enclosed spaces amplify overpressure
  const overpressure = isEnclosed ? basePressure * 2 : basePressure;

  return {
    overpressure,
    rupturedEardrums: overpressure > 5,
    lungDamage: overpressure > 15,
    lethal: overpressure > 35
  };
}

/**
 * Calculate fragmentation effects
 */
export function calculateFragmentationDamage(
  distance: number,
  shellDef: ShellDefinition,
  coverType: CoverType
): {
  fragmentHits: number;
  penetration: boolean;
  wounds: string;
} {
  // Fragment density decreases with distance squared
  const maxFragments = 1000; // Typical for medium shell
  const fragmentDensity = maxFragments / (4 * Math.PI * distance * distance);

  // Average fragments that could hit a human-sized target (1.8m x 0.5m = 0.9 m²)
  let expectedHits = fragmentDensity * 0.9;

  // Cover reduces fragment hits
  const coverMod = shellDef.coverModifiers.find(c => c.coverType === coverType);
  if (coverMod) {
    expectedHits *= (1 - coverMod.protection / 100);
  }

  let wounds = "None";
  if (expectedHits > 20) {
    wounds = "Catastrophic - multiple fragment wounds across body";
  } else if (expectedHits > 10) {
    wounds = "Severe - numerous fragment wounds";
  } else if (expectedHits > 5) {
    wounds = "Moderate - several fragment wounds";
  } else if (expectedHits > 1) {
    wounds = "Light - one or more fragment wounds";
  }

  return {
    fragmentHits: Math.floor(expectedHits),
    penetration: distance < shellDef.damage.severe,
    wounds
  };
}

/**
 * Determine wound severity and medical requirements
 */
export function determineMedicalRequirements(
  survivalChance: number,
  distance: number,
  shellDef: ShellDefinition,
  buriedAlive: boolean
): {
  severity: 'none' | 'light' | 'moderate' | 'severe' | 'critical' | 'fatal';
  treatment: string;
  evacuationRequired: boolean;
  survivalTime: number; // minutes until death without treatment
} {
  let severity: 'none' | 'light' | 'moderate' | 'severe' | 'critical' | 'fatal' = 'none';
  let treatment = "None required";
  let evacuationRequired = false;
  let survivalTime = Infinity;

  if (survivalChance === 0) {
    severity = 'fatal';
    treatment = "None - instant death";
    survivalTime = 0;
  } else if (survivalChance < 10) {
    severity = 'critical';
    treatment = "Immediate surgical intervention required";
    evacuationRequired = true;
    survivalTime = 10; // 10 minutes
  } else if (survivalChance < 40) {
    severity = 'severe';
    treatment = "Field surgery and evacuation within 1 hour";
    evacuationRequired = true;
    survivalTime = 60;
  } else if (survivalChance < 80) {
    severity = 'moderate';
    treatment = "Field dressing, evacuation recommended";
    evacuationRequired = true;
    survivalTime = 240; // 4 hours
  } else if (survivalChance < 98) {
    severity = 'light';
    treatment = "Field dressing, can continue fighting";
    evacuationRequired = false;
    survivalTime = Infinity;
  }

  if (buriedAlive) {
    severity = 'critical';
    treatment = "Immediate excavation required";
    survivalTime = 10; // 10 minutes of oxygen
    evacuationRequired = true;
  }

  return {
    severity,
    treatment,
    evacuationRequired,
    survivalTime
  };
}

/**
 * Calculate psychological impact (morale and stress)
 */
export function calculatePsychologicalImpact(
  distance: number,
  shellDef: ShellDefinition,
  witnessed: boolean,
  currentMorale: number,
  playerSkills: PlayerSkills
): {
  moraleLoss: number;
  stressGain: number;
  shellShockRisk: number;
  panicChance: number;
} {
  // Base impact based on shell size and proximity
  const proximityFactor = Math.max(0, 1 - (distance / shellDef.damage.shellShock));
  const shellSizeFactor = shellDef.tier / 4; // 0.25 to 1.0

  let baseMoraleLoss = proximityFactor * shellSizeFactor * 20;
  let baseStressGain = proximityFactor * shellSizeFactor * 30;

  // Witnessing casualties increases impact
  if (witnessed) {
    baseMoraleLoss *= 1.5;
    baseStressGain *= 1.5;
  }

  // Experience reduces psychological impact
  if (playerSkills.experience === 'veteran') {
    baseMoraleLoss *= 0.5;
    baseStressGain *= 0.6;
  } else if (playerSkills.experience === 'experienced') {
    baseMoraleLoss *= 0.75;
    baseStressGain *= 0.8;
  }

  // Current morale affects panic chance
  const panicChance = Math.max(0, (100 - currentMorale) * proximityFactor);

  // Shell shock risk
  const shellShockRisk = calculateShellShockProbability(
    distance,
    shellDef,
    currentMorale,
    playerSkills.experience === 'veteran'
  );

  return {
    moraleLoss: baseMoraleLoss,
    stressGain: baseStressGain,
    shellShockRisk,
    panicChance
  };
}

/**
 * Create impact event from shell impact
 */
export function createImpactEvent(
  id: string,
  shellType: ShellType,
  impactPosition: Position3D,
  impactTime: number,
  affectedEntities: Array<{
    id: string;
    position: Position3D;
    coverType: CoverType;
  }>
): ImpactEvent {
  const shellDef = getShellDefinition(shellType);
  if (!shellDef) {
    throw new Error(`Unknown shell type: ${shellType}`);
  }

  const casualties = {
    killed: [] as string[],
    wounded: [] as string[],
    shellShocked: [] as string[],
    buried: [] as string[]
  };

  // Calculate casualties
  for (const entity of affectedEntities) {
    const damage = calculateDamage(
      impactPosition,
      entity.position,
      shellDef,
      entity.coverType
    );

    const survived = determineSurvival(damage.finalSurvivalChance);

    if (!survived) {
      casualties.killed.push(entity.id);
    } else {
      if (damage.finalSurvivalChance < 80) {
        casualties.wounded.push(entity.id);
      }
      if (damage.shellShock) {
        casualties.shellShocked.push(entity.id);
      }
      if (damage.buriedAlive) {
        casualties.buried.push(entity.id);
      }
    }
  }

  return {
    id,
    shellType,
    impactPosition,
    impactTime,
    crater: shellDef.crater,
    damageRadii: shellDef.damage,
    casualties,
    trenchesDestroyed: [],
    structuresDestroyed: [],
    audioRange: shellDef.damage.shellShock * 10,
    visualRange: shellDef.damage.debris
  };
}

/**
 * Calculate area of effect casualties (simplified for AI units)
 */
export function calculateAreaCasualties(
  impactPosition: Position3D,
  shellDef: ShellDefinition,
  entityDensity: number, // entities per 100m²
  coverDistribution: { [key in CoverType]?: number } // percentage of entities in each cover type
): {
  estimatedKilled: number;
  estimatedWounded: number;
  estimatedShellShocked: number;
} {
  const affectedArea = Math.PI * Math.pow(shellDef.damage.severe, 2);
  const entitiesInArea = (affectedArea / 100) * entityDensity;

  let killed = 0;
  let wounded = 0;
  let shellShocked = 0;

  // Sample casualties based on cover distribution
  for (const [coverTypeStr, percentage] of Object.entries(coverDistribution)) {
    const coverType = coverTypeStr as CoverType;
    const entitiesInCover = entitiesInArea * (percentage / 100);

    // Average distance in affected area
    const avgDistance = shellDef.damage.severe * 0.6;

    const damage = calculateDamage(
      impactPosition,
      { ...impactPosition, x: impactPosition.x + avgDistance },
      shellDef,
      coverType
    );

    const survivalRate = damage.finalSurvivalChance / 100;
    killed += entitiesInCover * (1 - survivalRate);
    wounded += entitiesInCover * survivalRate * 0.6; // 60% of survivors wounded
    shellShocked += entitiesInCover * survivalRate * 0.3; // 30% shell shocked
  }

  return {
    estimatedKilled: Math.round(killed),
    estimatedWounded: Math.round(wounded),
    estimatedShellShocked: Math.round(shellShocked)
  };
}
