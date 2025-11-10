/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Shell Definitions
 *
 * Historical shell data from Verdun 1916
 * Based on French and German artillery specifications
 */

import {
  ShellDefinition,
  ShellType,
  ShellTier,
  CoverType,
  CoverModifier,
  DamageByDistance
} from './types';

// ============================================================================
// TIER 1: LIGHT ARTILLERY (60% of shells fired)
// ============================================================================

export const FRENCH_75MM: ShellDefinition = {
  designation: "75mm Canon de 75 modèle 1897",
  type: ShellType.FRENCH_75MM,
  tier: ShellTier.LIGHT,
  usage: "Primary French artillery - anti-personnel, counter-battery",
  percentageOfTotal: 35,
  shellsFiredVerdun: 14000000,

  audio: {
    incomingSound: "Ffff-CRACK! (sharp, high-pitched whistle)",
    warnTimeSeconds: 1,
    warnTimeSecondsMax: 3,
    explosionSound: "Sharp crack, metallic ring",
    recognizability: "Very distinctive - fastest, highest pitch",
    frequencyHz: 2500,
    volumeDb: 150
  },

  ballistics: {
    muzzleVelocity: 575,
    rangeMax: 11000,
    flightTimeTypical: 12,
    trajectory: 'flat',
    shellWeight: 6.2
  },

  damage: {
    lethal: 10,
    catastrophic: 10,
    severe: 20,
    moderate: 30,
    light: 50,
    shellShock: 30,
    debris: 50
  },

  crater: {
    diameter: 2,
    depth: 0.5
  },

  penetration: {
    sandbagWall: "Penetrates 2m thickness",
    earthDugout: "Penetrates 0.5m earth",
    concrete: "Superficial damage only",
    trenchWall: "Destroys, causes collapse"
  },

  damageByDistance: [
    {
      distance: 5,
      survivalChance: 0,
      wounds: "Instant death (vaporization/massive trauma)",
      playerExperience: "Screen goes black instantly - no warning",
      medicalRequired: "None - instant death"
    },
    {
      distance: 10,
      survivalChance: 10,
      wounds: "Catastrophic - multiple shrapnel, internal injuries, amputation",
      playerExperience: "Knockdown, severe bleeding, unconsciousness, likely death",
      medicalRequired: "Immediate evacuation or death in minutes"
    },
    {
      distance: 20,
      survivalChance: 60,
      wounds: "Severe - shrapnel wounds, concussion, hearing loss",
      playerExperience: "Knockdown, temporary deafness, vision blur, bleeding",
      medicalRequired: "Field dressing + evacuation within hours"
    },
    {
      distance: 30,
      survivalChance: 90,
      wounds: "Light to moderate - shrapnel fragments, concussion",
      playerExperience: "Stagger, ringing ears, possible minor wounds",
      medicalRequired: "Field dressing, can continue fighting"
    },
    {
      distance: 50,
      survivalChance: 98,
      wounds: "Minimal - possible fragment hits, shell shock",
      playerExperience: "Ears ring, morale drop, alertness spike",
      medicalRequired: "None (unless unlucky fragment hit)"
    }
  ],

  coverModifiers: [
    { coverType: CoverType.OPEN_GROUND, protection: 0, description: "No protection - full damage" },
    { coverType: CoverType.SHELL_CRATER, protection: 75, description: "75% protection (below ground level)" },
    { coverType: CoverType.SHALLOW_TRENCH, protection: 60, description: "60% protection (2m deep)" },
    { coverType: CoverType.DUGOUT_ENTRANCE, protection: 85, description: "85% protection (earth overhead)" },
    { coverType: CoverType.DEEP_DUGOUT, protection: 95, description: "95% protection (3m+ earth)" },
    { coverType: CoverType.CONCRETE_BUNKER, protection: 99, description: "99% protection (reinforced concrete)" }
  ],

  gameplayNotes: {
    frequency: "Very common - player hears these constantly",
    reactionWindow: "1-3 seconds - barely enough to drop prone",
    primaryThreat: "Shrapnel, not blast (fragments travel 50m+)",
    counterStrategy: "Stay low, use craters, recognize sound fast"
  }
};

export const GERMAN_77MM: ShellDefinition = {
  designation: "77mm Feldkanone 96 n.A.",
  type: ShellType.GERMAN_77MM,
  tier: ShellTier.LIGHT,
  usage: "German equivalent to French 75mm",
  percentageOfTotal: 25,
  shellsFiredVerdun: 10000000,

  audio: {
    incomingSound: "Ssssss-BANG! (sibilant hiss, sharp crack)",
    warnTimeSeconds: 2,
    warnTimeSecondsMax: 4,
    explosionSound: "Slightly deeper than 75mm",
    recognizability: "Distinctive hiss - German signature",
    frequencyHz: 2200,
    volumeDb: 152
  },

  ballistics: {
    muzzleVelocity: 465,
    rangeMax: 7800,
    flightTimeTypical: 14,
    trajectory: 'flat',
    shellWeight: 6.8
  },

  damage: {
    lethal: 12,
    catastrophic: 12,
    severe: 22,
    moderate: 32,
    light: 52,
    shellShock: 32,
    debris: 52
  },

  crater: {
    diameter: 2.5,
    depth: 0.6
  },

  penetration: {
    sandbagWall: "Penetrates 2.2m thickness",
    earthDugout: "Penetrates 0.6m earth",
    concrete: "Superficial damage only",
    trenchWall: "Destroys, causes collapse"
  },

  damageByDistance: [
    {
      distance: 5,
      survivalChance: 0,
      wounds: "Instant death",
      playerExperience: "Screen goes black instantly",
      medicalRequired: "None - instant death"
    },
    {
      distance: 12,
      survivalChance: 8,
      wounds: "Catastrophic - multiple shrapnel, internal injuries",
      playerExperience: "Knockdown, severe bleeding, unconsciousness",
      medicalRequired: "Immediate evacuation or death"
    },
    {
      distance: 22,
      survivalChance: 55,
      wounds: "Severe - shrapnel wounds, concussion, hearing loss",
      playerExperience: "Knockdown, temporary deafness, vision blur",
      medicalRequired: "Field dressing + evacuation"
    },
    {
      distance: 32,
      survivalChance: 88,
      wounds: "Light to moderate - shrapnel fragments",
      playerExperience: "Stagger, ringing ears, minor wounds",
      medicalRequired: "Field dressing"
    },
    {
      distance: 52,
      survivalChance: 97,
      wounds: "Minimal - possible fragment hits",
      playerExperience: "Ears ring, morale drop",
      medicalRequired: "None"
    }
  ],

  coverModifiers: [
    { coverType: CoverType.OPEN_GROUND, protection: 0, description: "No protection" },
    { coverType: CoverType.SHELL_CRATER, protection: 73, description: "Good protection" },
    { coverType: CoverType.SHALLOW_TRENCH, protection: 58, description: "Moderate protection" },
    { coverType: CoverType.DUGOUT_ENTRANCE, protection: 83, description: "High protection" },
    { coverType: CoverType.DEEP_DUGOUT, protection: 94, description: "Very high protection" },
    { coverType: CoverType.CONCRETE_BUNKER, protection: 99, description: "Near-complete protection" }
  ],

  gameplayNotes: {
    frequency: "Common when defending against German attacks",
    reactionWindow: "2-4 seconds - enough time to drop prone",
    primaryThreat: "Shrapnel (slightly more powerful than 75mm)",
    counterStrategy: "Recognize hissing sound, take cover immediately",
    terrorFactor: "The sound of German attacks"
  }
};

// ============================================================================
// TIER 2: MEDIUM ARTILLERY (25% of shells)
// ============================================================================

export const HOWITZER_155MM: ShellDefinition = {
  designation: "155mm Howitzer (French/German)",
  type: ShellType.HOWITZER_155MM,
  tier: ShellTier.MEDIUM,
  usage: "Heavy bombardment, fortification destruction, trench clearing",
  percentageOfTotal: 20,
  shellsFiredVerdun: 8000000,

  audio: {
    incomingSound: "Whummm...WHUMP (low rumble building to thunder)",
    warnTimeSeconds: 3,
    warnTimeSecondsMax: 8,
    explosionSound: "Deep, ground-shaking BOOM",
    recognizability: "Distinctive low rumble - 'freight train' sound",
    frequencyHz: 800,
    volumeDb: 165
  },

  ballistics: {
    muzzleVelocity: 450,
    rangeMax: 12000,
    flightTimeTypical: 20,
    trajectory: 'high_arc',
    shellWeight: 43
  },

  damage: {
    lethal: 20,
    catastrophic: 20,
    severe: 40,
    moderate: 60,
    light: 100,
    shellShock: 60,
    buriedAlive: 15,
    debris: 100
  },

  crater: {
    diameter: 6,
    depth: 1.5
  },

  penetration: {
    sandbagWall: "Destroys completely (3m+ thickness)",
    earthDugout: "Penetrates 2m earth, causes collapse",
    concrete: "Cracks 0.5m concrete, destroys thin walls",
    trenchWall: "Obliterates 10m+ of trench"
  },

  damageByDistance: [
    {
      distance: 10,
      survivalChance: 0,
      wounds: "Instant death (complete disintegration)",
      playerExperience: "Screen black - no survival possible",
      medicalRequired: "None - instant death"
    },
    {
      distance: 20,
      survivalChance: 2,
      wounds: "Catastrophic - multiple amputations, internal rupture",
      playerExperience: "Massive knockdown, possible burial, near-certain death",
      medicalRequired: "Immediate evacuation (unlikely to survive)",
      specialEffects: ["burial_alive_50%", "temporary_deafness_100%"]
    },
    {
      distance: 40,
      survivalChance: 40,
      wounds: "Severe - major shrapnel, broken bones, concussion",
      playerExperience: "Violent knockdown, bleeding, disorientation, medical evacuation needed",
      medicalRequired: "Urgent evacuation",
      specialEffects: ["burial_alive_20%", "temporary_deafness_100%"]
    },
    {
      distance: 60,
      survivalChance: 75,
      wounds: "Moderate - shrapnel wounds, concussion, hearing damage",
      playerExperience: "Knockdown, ringing ears, vision blur, morale crash",
      medicalRequired: "Field hospital treatment",
      specialEffects: ["shell_shock_30%"]
    },
    {
      distance: 100,
      survivalChance: 95,
      wounds: "Light - possible fragment hits, shell shock",
      playerExperience: "Ground shake, terror, morale drop, debris falling",
      medicalRequired: "Field dressing if hit",
      specialEffects: ["shell_shock_10%"]
    }
  ],

  coverModifiers: [
    { coverType: CoverType.OPEN_GROUND, protection: 0, description: "No protection" },
    { coverType: CoverType.SHELL_CRATER, protection: 50, description: "Inadequate against this size" },
    { coverType: CoverType.SHALLOW_TRENCH, protection: 40, description: "Can cause trench collapse" },
    { coverType: CoverType.DUGOUT_ENTRANCE, protection: 70, description: "Moderate protection" },
    { coverType: CoverType.DEEP_DUGOUT, protection: 90, description: "Good protection (3m+ earth)" },
    { coverType: CoverType.CONCRETE_BUNKER, protection: 95, description: "Very high protection" }
  ],

  specialMechanics: {
    burialAlive: {
      trigger: "Within 20m and in trench/crater",
      chance: 50,
      effect: "Player buried under earth/debris",
      oxygenTimer: 300,
      escapeMechanic: "Rapid button press + stamina cost"
    },
    trenchCollapse: {
      radius: 10,
      effect: "Trench walls collapse, burying occupants"
    }
  },

  gameplayNotes: {
    frequency: "Moderate - several per minute during bombardments",
    reactionWindow: "3-8 seconds - enough time to sprint to dugout",
    primaryThreat: "Blast overpressure + burial, not just shrapnel",
    counterStrategy: "Deep dugout required, shallow cover insufficient",
    terrorFactor: "High - ground shakes, deafening, causes panic"
  }
};

// ============================================================================
// TIER 3: HEAVY ARTILLERY (10% of shells)
// ============================================================================

export const GERMAN_210MM: ShellDefinition = {
  designation: "210mm Heavy Howitzer (21cm Mörser)",
  type: ShellType.GERMAN_210MM,
  tier: ShellTier.HEAVY,
  usage: "Fort bombardment, deep dugout destruction",
  percentageOfTotal: 5,
  shellsFiredVerdun: 2000000,

  audio: {
    incomingSound: "RRROOOOAAAARRRR (sustained roar, earthquake sound)",
    warnTimeSeconds: 8,
    warnTimeSecondsMax: 15,
    explosionSound: "Earthquake-like BOOM, shockwave felt 500m+",
    recognizability: "Unmistakable - sounds like incoming train",
    frequencyHz: 400,
    volumeDb: 175
  },

  ballistics: {
    muzzleVelocity: 393,
    rangeMax: 11000,
    flightTimeTypical: 25,
    trajectory: 'high_arc',
    shellWeight: 113
  },

  damage: {
    lethal: 25,
    catastrophic: 25,
    severe: 50,
    moderate: 80,
    light: 150,
    shellShock: 80,
    buriedAlive: 20,
    debris: 150
  },

  crater: {
    diameter: 10,
    depth: 2.5
  },

  penetration: {
    sandbagWall: "Obliterates any sandbag fortification",
    earthDugout: "Penetrates 3m+ earth",
    concrete: "Destroys 1m concrete",
    trenchWall: "Fort casemate - damages but rarely penetrates thick walls"
  },

  damageByDistance: [
    {
      distance: 25,
      survivalChance: 0,
      wounds: "Instant death, no survival",
      playerExperience: "Obliteration",
      medicalRequired: "None"
    },
    {
      distance: 50,
      survivalChance: 5,
      wounds: "Catastrophic wounds, burial",
      playerExperience: "Massive trauma, likely death",
      medicalRequired: "Immediate evacuation (unlikely to survive)"
    },
    {
      distance: 80,
      survivalChance: 50,
      wounds: "Severe wounds, shell shock guaranteed",
      playerExperience: "Extreme trauma, disorientation",
      medicalRequired: "Urgent medical evacuation"
    },
    {
      distance: 150,
      survivalChance: 90,
      wounds: "Psychological trauma, debris danger",
      playerExperience: "Terror, falling debris",
      medicalRequired: "Psychological support"
    }
  ],

  coverModifiers: [
    { coverType: CoverType.OPEN_GROUND, protection: 0, description: "Death within 25m" },
    { coverType: CoverType.SHELL_CRATER, protection: 35, description: "Inadequate" },
    { coverType: CoverType.SHALLOW_TRENCH, protection: 30, description: "Inadequate - burial/collapse" },
    { coverType: CoverType.DUGOUT_ENTRANCE, protection: 60, description: "Minimal protection" },
    { coverType: CoverType.DEEP_DUGOUT, protection: 85, description: "Can still collapse" },
    { coverType: CoverType.CONCRETE_BUNKER, protection: 92, description: "Best protection" }
  ],

  gameplayNotes: {
    frequency: "Rare - a few per hour",
    reactionWindow: "8-15 seconds - long warning allows retreat to deepest shelter",
    primaryThreat: "Even deep dugouts can collapse",
    counterStrategy: "Run to deepest shelter immediately",
    terrorFactor: "Single shell terrorizes entire sector"
  }
};

export const FRENCH_305MM: ShellDefinition = {
  designation: "305mm Heavy Mortar (Mortier de 305)",
  type: ShellType.FRENCH_305MM,
  tier: ShellTier.HEAVY,
  usage: "German position destruction, counter-battery",
  percentageOfTotal: 3,
  shellsFiredVerdun: 1200000,

  audio: {
    incomingSound: "WHOOOOOM-WHOOOOOM (low bass wobble)",
    warnTimeSeconds: 10,
    warnTimeSecondsMax: 20,
    explosionSound: "Ground-shaking concussion, felt 1km+",
    recognizability: "Unmistakable bass rumble",
    frequencyHz: 200,
    volumeDb: 180
  },

  ballistics: {
    muzzleVelocity: 350,
    rangeMax: 11000,
    flightTimeTypical: 30,
    trajectory: 'very_high_arc',
    shellWeight: 340
  },

  damage: {
    lethal: 30,
    catastrophic: 30,
    severe: 60,
    moderate: 100,
    light: 200,
    shellShock: 100,
    buriedAlive: 25,
    debris: 200
  },

  crater: {
    diameter: 15,
    depth: 3
  },

  penetration: {
    sandbagWall: "Complete obliteration",
    earthDugout: "Penetrates 4m+ earth",
    concrete: "Destroys 1.5m concrete",
    trenchWall: "Creates massive crater visible from aircraft"
  },

  damageByDistance: [
    {
      distance: 30,
      survivalChance: 0,
      wounds: "Complete obliteration",
      playerExperience: "Instant death",
      medicalRequired: "None"
    },
    {
      distance: 60,
      survivalChance: 3,
      wounds: "Catastrophic - mass burial",
      playerExperience: "Earthquake, burial, near-certain death",
      medicalRequired: "None - unlikely to survive"
    },
    {
      distance: 100,
      survivalChance: 40,
      wounds: "Severe trauma, shell shock",
      playerExperience: "World shakes, disorientation, terror",
      medicalRequired: "Urgent evacuation"
    },
    {
      distance: 200,
      survivalChance: 85,
      wounds: "Psychological trauma",
      playerExperience: "Apocalyptic - entire screen shakes",
      medicalRequired: "Psychological support"
    }
  ],

  coverModifiers: [
    { coverType: CoverType.OPEN_GROUND, protection: 0, description: "No protection" },
    { coverType: CoverType.SHELL_CRATER, protection: 30, description: "Inadequate" },
    { coverType: CoverType.SHALLOW_TRENCH, protection: 25, description: "Inadequate" },
    { coverType: CoverType.DUGOUT_ENTRANCE, protection: 55, description: "Minimal" },
    { coverType: CoverType.DEEP_DUGOUT, protection: 80, description: "Only deepest bunkers survive" },
    { coverType: CoverType.CONCRETE_BUNKER, protection: 90, description: "Best chance" }
  ],

  gameplayNotes: {
    frequency: "Very rare - occasional throughout day",
    reactionWindow: "10-20 seconds - seek deepest shelter",
    primaryThreat: "Apocalyptic blast - creates massive craters",
    counterStrategy: "Only deepest bunkers or being far away saves you",
    terrorFactor: "Entire sector hears and feels impact"
  }
};

// ============================================================================
// TIER 4: SUPER-HEAVY ARTILLERY (2% of shells)
// ============================================================================

export const GERMAN_420MM: ShellDefinition = {
  designation: "420mm Super-Heavy Mortar (Big Bertha type)",
  type: ShellType.GERMAN_420MM,
  tier: ShellTier.SUPER_HEAVY,
  usage: "Fort destruction, symbolic terror bombardment",
  percentageOfTotal: 2,
  shellsFiredVerdun: 800000,

  audio: {
    incomingSound: "RUUUUUMMMMBBBBLLLLLEEEEE (sustained earthquake rumble)",
    warnTimeSeconds: 20,
    warnTimeSecondsMax: 40,
    explosionSound: "Apocalyptic blast heard 10km+ away",
    recognizability: "Everyone within 1km knows it's coming",
    frequencyHz: 100,
    volumeDb: 190
  },

  ballistics: {
    muzzleVelocity: 425,
    rangeMax: 14000,
    flightTimeTypical: 50,
    trajectory: 'very_high_arc',
    shellWeight: 820
  },

  damage: {
    lethal: 50,
    catastrophic: 50,
    severe: 100,
    moderate: 150,
    light: 300,
    shellShock: 150,
    buriedAlive: 30,
    debris: 300
  },

  crater: {
    diameter: 30,
    depth: 6
  },

  penetration: {
    sandbagWall: "Obliterates everything",
    earthDugout: "Penetrates 5m+ earth (deep dugouts collapse)",
    concrete: "Destroys 2m concrete",
    trenchWall: "Can penetrate fort roofs with direct hit"
  },

  damageByDistance: [
    {
      distance: 50,
      survivalChance: 0,
      wounds: "Obliteration - nothing remains",
      playerExperience: "Instant death cutscene - no escape",
      medicalRequired: "None"
    },
    {
      distance: 100,
      survivalChance: 1,
      wounds: "Total body trauma, mass burial",
      playerExperience: "Violent earthquake, burial, near-certain death",
      medicalRequired: "None - death likely"
    },
    {
      distance: 150,
      survivalChance: 20,
      wounds: "Catastrophic - blast injuries, burial, ruptured organs",
      playerExperience: "World goes white, then black, confusion, disorientation",
      medicalRequired: "Immediate evacuation (low survival)",
      specialEffects: ["shell_shock_100%", "temporary_blindness", "temporary_deafness"]
    },
    {
      distance: 300,
      survivalChance: 70,
      wounds: "Severe concussion, debris strikes, hearing loss",
      playerExperience: "Knocked down, debris falling like rain, terror",
      medicalRequired: "Medical evacuation",
      specialEffects: ["shell_shock_80%"]
    }
  ],

  coverModifiers: [
    { coverType: CoverType.OPEN_GROUND, protection: 0, description: "Death within 50m" },
    { coverType: CoverType.SHELL_CRATER, protection: 20, description: "Inadequate - burial/collapse" },
    { coverType: CoverType.SHALLOW_TRENCH, protection: 15, description: "Inadequate - burial/collapse" },
    { coverType: CoverType.DUGOUT_ENTRANCE, protection: 40, description: "Minimal protection" },
    { coverType: CoverType.DEEP_DUGOUT, protection: 70, description: "Can still collapse - luck element" },
    { coverType: CoverType.CONCRETE_BUNKER, protection: 90, description: "Best protection (can still fail)" }
  ],

  gameplayNotes: {
    frequency: "Extremely rare - maybe 1-3 per mission",
    reactionWindow: "20-40 seconds - scramble for deepest shelter",
    primaryThreat: "Even deep cover can fail - pure luck element",
    counterStrategy: "Run and pray - seek fort casemate or deepest bunker",
    terrorFactor: "Entire sector panics - NPCs flee. Use sparingly for maximum terror impact"
  }
};

// ============================================================================
// SHELL REGISTRY
// ============================================================================

export const SHELL_REGISTRY: Map<ShellType, ShellDefinition> = new Map([
  [ShellType.FRENCH_75MM, FRENCH_75MM],
  [ShellType.GERMAN_77MM, GERMAN_77MM],
  [ShellType.HOWITZER_155MM, HOWITZER_155MM],
  [ShellType.GERMAN_210MM, GERMAN_210MM],
  [ShellType.FRENCH_305MM, FRENCH_305MM],
  [ShellType.GERMAN_420MM, GERMAN_420MM]
]);

/**
 * Get shell definition by type
 */
export function getShellDefinition(type: ShellType): ShellDefinition | undefined {
  return SHELL_REGISTRY.get(type);
}

/**
 * Get all shell definitions
 */
export function getAllShellDefinitions(): ShellDefinition[] {
  return Array.from(SHELL_REGISTRY.values());
}

/**
 * Get shells by tier
 */
export function getShellsByTier(tier: ShellTier): ShellDefinition[] {
  return getAllShellDefinitions().filter(shell => shell.tier === tier);
}

/**
 * Historical shell mix for Verdun (based on percentages)
 */
export const VERDUN_HISTORICAL_SHELL_MIX: { [key in ShellType]?: number } = {
  [ShellType.FRENCH_75MM]: 35,
  [ShellType.GERMAN_77MM]: 25,
  [ShellType.HOWITZER_155MM]: 20,
  [ShellType.GERMAN_210MM]: 5,
  [ShellType.FRENCH_305MM]: 3,
  [ShellType.GERMAN_420MM]: 2,
  [ShellType.PHOSGENE_GAS]: 5,
  [ShellType.MUSTARD_GAS]: 5
};
