# ONLY THE DEAD - DEVELOPMENT BOOK
**WWI Survival Game Design Bible**

**Version**: 1.0.1
**Last Updated**: November 10, 2025
**Status**: In Development - Phase III Implementation Started

---

## 📚 DOCUMENT PURPOSE

This is the **game design bible** for "Only The Dead" - a WWI survival game set during the Battle of Verdun (February 21 - December 18, 1916, 303 days).

### Relationship to Research Anthology

This Development Book works alongside the **Verdun Research Anthology**:

- **Verdun Research Anthology** (`verdun_anthology/`) = Historical research, verified facts, primary sources
  - Timelines with documented events
  - Verified coordinates and terrain data
  - Historical survival mechanics documentation
  - Real casualty statistics and battle phases
  - **Purpose**: Historical accuracy foundation

- **Only The Dead Development Book** (this document) = Game design specifications
  - Mission designs using historical events
  - Game mechanics specifications (exact formulas)
  - Narrative content (dialogue, characters, scenes)
  - System designs (progression, UI/UX, audio, etc.)
  - **Purpose**: Playable game implementation

### Content Classification

**TIER 1: Historical Foundation (from Research Anthology)**
- Real events, dates, locations, statistics
- Verified from archives and academic sources
- Used as framework for missions and gameplay

**TIER 2: Narrative Reconstruction (clearly marked)**
- Characters created for game narrative
- Dialogue and scenes based on historical patterns
- Composite letters/diaries inspired by real sources
- **MARKED AS**: "Narrative content - recreated for game"

**TIER 3: Game Design (mechanical)**
- System specifications (hunger meters, damage calculations)
- Mission structures and objectives
- Progression systems
- UI/UX designs

---

## 📖 TABLE OF CONTENTS

### PART I: GAME VISION & DESIGN PILLARS
1. [Core Vision](#core-vision)
2. [Design Pillars](#design-pillars)
3. [Target Audience](#target-audience)
4. [Platform & Technical Specs](#technical-specs)

### PART II: GAME SYSTEMS
5. [Survival Systems](#survival-systems)
6. [Combat & Artillery Systems](#combat-systems)
7. [Progression & Meta-Game](#progression)
8. [Inventory & Equipment](#inventory)
9. [Medical & Injury System](#medical)
10. [Morale & Psychological System](#morale)

### PART III: MISSION DESIGN
11. [Mission Structure Overview](#mission-structure)
12. [Tutorial Missions (Feb 21-22)](#tutorial-missions)
13. [Core Missions (30+ mission specs)](#core-missions)
14. [Climax Mission: Douaumont Recapture](#climax-mission)
15. [Procedural Survival Mode](#survival-mode)

### PART IV: NARRATIVE CONTENT
16. [Character Database](#characters)
17. [Dialogue System & Scripts](#dialogue)
18. [Environmental Storytelling](#environmental-narrative)
19. [Cutscene Specifications](#cutscenes)

### PART V: AUDIO & VISUAL DESIGN
20. [Audio Design Bible](#audio-design)
21. [Visual Style Guide](#visual-style)
22. [UI/UX Specifications](#ui-ux)

### PART VI: WORLD BUILDING
23. [Level Design Specifications](#level-design)
24. [AI Behavior Specifications](#ai-behavior)
25. [Dynamic Weather & Time System](#weather-system)

### PART VII: TECHNICAL IMPLEMENTATION (**IN PROGRESS**)
26. [Camera & Cover System](#camera-cover-system) ✅ **IMPLEMENTED**
27. [Character & Movement System](#character-movement) ✅ **IMPLEMENTED**
28. [Survival Meters Component](#survival-component) ✅ **IMPLEMENTED**
29. [Save System & Permadeath](#save-system)
30. [Performance Targets](#performance)
31. [Localization Plan](#localization)
32. [Accessibility Features](#accessibility)

### PART VIII: PRODUCTION
30. [Asset Lists](#asset-lists)
31. [Development Roadmap](#roadmap)
32. [Testing & QA Plan](#testing)

---

## PART I: GAME VISION & DESIGN PILLARS

<a name="core-vision"></a>
### 1. CORE VISION

**Tagline**: "Victory is survival."

**Elevator Pitch**:
You are a French soldier at Verdun, February 1916. For 303 days, you will not fight to win. You will fight to survive. Every shell. Every night. Every hour. This is not a war game. This is a survival game where the enemy is artillery, mud, hunger, cold, fear, and time itself.

**Core Experience**:
- **Terror**: Random artillery that can kill you instantly with no warning
- **Exhaustion**: Constant management of hunger, thirst, stamina, sleep
- **Numbness**: Gradual psychological transformation from fear to hollow resilience
- **Humanity**: Moments of connection with fellow soldiers that keep you human
- **History**: Every location, battle, date is real - you are THERE

**What This Is NOT**:
- ❌ FPS shooter where you kill 100 enemies per mission
- ❌ Power fantasy where you become superhuman
- ❌ Heroic narrative where individuals change the battle's outcome
- ❌ Fast-paced action game

**What This IS**:
- ✅ Survival horror set in historical WWI
- ✅ Moment-to-moment survival (next hour, next day, next rotation)
- ✅ Psychological journey through trauma
- ✅ Educational experience that honors those who fought
- ✅ Slow, deliberate, methodical gameplay

<a name="design-pillars"></a>
### 2. DESIGN PILLARS

**Pillar 1: AUTHENTICITY**
- Every date, location, battle is historically verified
- Survival mechanics based on actual soldier experiences
- Weather, rations, artillery patterns match historical records
- Visual/audio design from period photographs and accounts

**Pillar 2: SURVIVAL OVER COMBAT**
- 70% of gameplay is managing survival meters (hunger, thirst, warmth, etc.)
- 20% is environmental navigation (mud, craters, trenches)
- 10% is combat (defensive, not aggressive)
- Victory = staying alive, not body count

**Pillar 3: PSYCHOLOGICAL REALISM**
- Morale system that degrades over time
- Shell shock mechanics (trauma accumulation)
- Coping mechanisms (letters home, rituals, dark humor)
- No "hero" transformation - just endurance

**Pillar 4: RESPECT FOR HISTORY**
- Game honors the 700,000 casualties
- Not glorifying war - showing horror and waste
- Educational component (historical context available)
- Avoids sensationalism or exploitation

**Pillar 5: PLAYER AGENCY WITHIN CONSTRAINTS**
- Player chooses HOW to survive, not WHETHER events happen
- Can't change history, but can change individual experience
- Choices matter (save squadmate or save self, eat now or ration)
- Multiple approaches to same situation

<a name="target-audience"></a>
### 3. TARGET AUDIENCE

**Primary Audience**:
- Age 25-45
- Fans of survival games (This War of Mine, The Long Dark)
- History enthusiasts (especially WWI)
- Players seeking meaningful/serious gaming experiences
- Educational institutions (history classes, museums)

**Secondary Audience**:
- Military history buffs
- Walking simulator / narrative game fans
- Realistic/tactical shooter players looking for depth
- French/German players with family WWI connections

**NOT Targeting**:
- Young FPS players seeking action
- Casual mobile gamers
- Players seeking power fantasy
- Those uncomfortable with historical horror/trauma

<a name="technical-specs"></a>
### 4. PLATFORM & TECHNICAL SPECIFICATIONS

**Engine**: Unreal Engine 5
- Reason: Nanite for terrain detail, Lumen for lighting, Blueprint for rapid iteration

**Target Platforms**:
- **Primary**: PC (Steam, Epic)
- **Secondary**: PlayStation 5, Xbox Series X|S
- **Possible**: VR (PSVR2, Quest 3) - post-launch if successful

**Performance Targets**:
- **PC**: 60 FPS @ 1440p (High settings), RTX 3070 or equivalent
- **Console**: 30 FPS @ 4K (Quality mode) or 60 FPS @ 1440p (Performance mode)
- **VR**: 90 FPS minimum (if VR version developed)

**Visual Style**:
- Photorealistic terrain and environment
- Slightly stylized characters (avoid uncanny valley, reduce gore)
- Muted color palette (browns, greys, desaturated blues)
- Film grain and chromatic aberration for period feel

**Audio**:
- Full 3D spatial audio (critical for shell recognition)
- Period-accurate sound effects
- Dynamic music (minimal, mood-driven)
- French/German voice acting with English subtitles

**Game Length**:
- **Main Story**: 15-20 hours (selected missions across 303 days)
- **Full Campaign**: 40-60 hours (all missions, all perspectives)
- **Survival Mode**: Infinite (procedural daily survival)

**Save System**: Checkpoint-based with optional Ironman permadeath mode

---

## PART II: GAME SYSTEMS

<a name="survival-systems"></a>
### 5. SURVIVAL SYSTEMS SPECIFICATIONS

**7 Core Survival Meters** (0-100 scale):

#### HUNGER
```json
{
  "max_value": 100,
  "depletion_rate_per_hour": {
    "resting": 3.0,
    "walking": 4.5,
    "working": 6.0,
    "combat": 8.0
  },
  "effects": {
    "100-70": {"penalties": "none"},
    "69-40": {"stamina_max": -15, "movement_speed": -5},
    "39-20": {"stamina_max": -30, "movement_speed": -15, "aim_shake": "moderate"},
    "19-1": {"stamina_max": -50, "movement_speed": -30, "aim_shake": "severe", "vision_blur": "light"},
    "0": {"collapse_chance_per_hour": 50, "death_hours": 168}
  },
  "food_restoration": {
    "bread_ration_700g": 40,
    "canned_beef_300g": 35,
    "biscuit_hard_100g": 15,
    "wine_250ml": 5,
    "chocolate_50g": 10,
    "cheese_100g": 20,
    "soup_hot_500ml": 30
  }
}
```

#### THIRST
```json
{
  "max_value": 100,
  "depletion_rate_per_hour": {
    "resting": 5.0,
    "walking": 7.0,
    "working": 10.0,
    "combat": 12.0,
    "hot_weather_multiplier": 1.5,
    "cold_weather_multiplier": 0.8
  },
  "effects": {
    "100-70": {"penalties": "none"},
    "69-40": {"stamina_regen": -25, "aim_shake": "light"},
    "39-20": {"stamina_regen": -50, "movement_speed": -15, "vision_blur": "moderate", "headache_vfx": true},
    "19-1": {"stamina_max": -40, "movement_speed": -30, "confusion": "severe", "hallucinations": "rare"},
    "0": {"collapse_immediate": true, "death_hours": 24}
  },
  "water_restoration": {
    "clean_water_1L": 50,
    "clean_water_500ml": 25,
    "rainwater_500ml": 20,
    "crater_water_500ml": 15,
    "crater_water_risk": {"disease_chance": 40, "vomiting_chance": 20}
  },
  "note": "Thirst is MORE critical than hunger - player dies faster from dehydration"
}
```

#### STAMINA
```json
{
  "max_value": 100,
  "base_regen_per_second": 2.0,
  "depletion_actions": {
    "sprinting": 15.0,
    "jumping": 8.0,
    "climbing": 12.0,
    "melee_attack": 10.0,
    "carrying_wounded": 20.0,
    "digging": 18.0
  },
  "effects_when_low": {
    "below_30": {"sprint_disabled": true, "movement_speed": -20},
    "below_10": {"crouch_walk_only": true, "aim_impossible": true},
    "0": {"collapse_for_10_seconds": true}
  },
  "modifiers": {
    "hunger_below_40": {"max_stamina": -30},
    "thirst_below_40": {"regen_rate": -50},
    "warmth_below_20": {"max_stamina": -40},
    "morale_below_30": {"max_stamina": -20}
  }
}
```

#### WARMTH
```json
{
  "max_value": 100,
  "depletion_rate_per_minute": {
    "base_winter": 0.5,
    "base_spring_fall": 0.2,
    "base_summer": 0.05,
    "rain_multiplier": 2.0,
    "wind_multiplier": 1.5,
    "wet_clothing_multiplier": 3.0
  },
  "restoration": {
    "fire_near": 2.0,
    "dugout_shelter": 0.5,
    "dry_clothing": 1.0,
    "hot_soup": 5.0,
    "alcohol_temporary": 10.0
  },
  "effects": {
    "100-70": {"penalties": "none"},
    "69-40": {"stamina_regen": -20, "movement_speed": -10, "shivering_vfx": true},
    "39-20": {"stamina_max": -30, "aim_shake": "severe", "frostbite_risk_per_hour": 10},
    "19-1": {"hypothermia": true, "confusion": "severe", "collapse_risk": 30},
    "0": {"unconscious": true, "death_hours": 4}
  },
  "frostbite_progression": {
    "stage_1": {"time_hours": 2, "effect": "numb extremities, -10% aim"},
    "stage_2": {"time_hours": 6, "effect": "permanent -20% movement, medical evacuation"},
    "stage_3": {"time_hours": 12, "effect": "amputation required, removed from combat"}
  }
}
```

#### HYGIENE
```json
{
  "max_value": 100,
  "depletion_rate_per_hour": 2.0,
  "depletion_multipliers": {
    "combat": 3.0,
    "trench_living": 2.0,
    "summer_heat": 1.5
  },
  "effects": {
    "100-70": {"penalties": "none"},
    "69-40": {"disease_risk_per_day": 5, "morale": -5, "lice_infestation": "light"},
    "39-20": {"disease_risk_per_day": 15, "morale": -15, "lice_infestation": "severe", "trench_fever_risk": 10},
    "19-1": {"dysentery_risk": 30, "trench_foot_risk": 40, "morale": -30},
    "0": {"illness_guaranteed": true}
  },
  "diseases": {
    "trench_fever": {"incubation_days": 10, "duration_days": 7, "effects": "high fever, hallucinations, -50% all stats"},
    "dysentery": {"incubation_days": 3, "duration_days": 5, "effects": "constant diarrhea, -60% stamina, dehydration +200%"},
    "lice": {"constant": true, "effects": "itching distraction, morale -10, trench fever vector"}
  },
  "hygiene_actions": {
    "wash_face_hands": 5,
    "full_wash": 15,
    "change_clothing": 20,
    "delousing": 10
  },
  "note": "Hygiene is SLOW to restore - intentionally frustrating"
}
```

#### MORALE
```json
{
  "max_value": 100,
  "base_depletion_per_hour": 0.5,
  "depletion_events": {
    "friendly_death_witnessed": -15,
    "artillery_barrage_survived": -5,
    "near_miss_shell": -8,
    "corpse_encountered": -3,
    "wounded_screaming_nearby": -2,
    "rat_attack": -5,
    "letter_from_home_received": 20,
    "hot_meal": 8,
    "rotation_to_rest": 25,
    "victory_small": 10,
    "dark_humor_moment": 5
  },
  "effects": {
    "100-70": {"penalties": "none"},
    "69-40": {"aim_shake": "light", "shell_shock_resistance": -20},
    "39-20": {"aim_shake": "moderate", "shell_shock_resistance": -50, "desertion_thoughts": true},
    "19-1": {"aim_shake": "severe", "panic_risk": 40, "combat_effectiveness": -60},
    "0": {"breakdown": true, "player_loses_control_temporarily": true}
  },
  "shell_shock_threshold": {
    "calculation": "morale < 30 AND (traumatic_event OR cumulative_stress > 100)",
    "symptoms": ["tremors", "mutism", "paralysis", "flashbacks", "panic_attacks"],
    "duration": "mission_end OR medical_evacuation"
  }
}
```

#### ALERTNESS
```json
{
  "max_value": 100,
  "depletion_rate_per_hour_awake": 4.0,
  "depletion_multipliers": {
    "night": 2.0,
    "sentry_duty": 1.5,
    "combat": 0.5
  },
  "restoration": {
    "sleep_1_hour": 25,
    "sleep_interrupted": 10,
    "coffee": 15,
    "adrenaline_in_combat": 30
  },
  "effects": {
    "100-70": {"penalties": "none"},
    "69-40": {"reaction_time": -15, "aim_speed": -10},
    "39-20": {"reaction_time": -30, "vision_blur": "light", "micro_sleep_risk": 10},
    "19-1": {"reaction_time": -60, "hallucinations": "moderate", "fall_asleep_risk": 40},
    "0": {"fall_asleep_immediately": true, "wake_only_if_shelled": true}
  },
  "sleep_mechanics": {
    "ideal_sleep_hours": 8,
    "typical_verdun_sleep": 2,
    "sleep_interruptions": ["artillery", "rats", "stand_to", "patrol_duty", "lice"],
    "cumulative_debt": "after_7_days_no_sleep_player_collapses"
  }
}
```

---

### SURVIVAL SYSTEM INTERACTIONS

**Cascading Failures**:
```
Low Hunger → Low Stamina → Can't work → Can't get food → Death spiral
Low Thirst → Confusion → Poor decisions → Drink crater water → Dysentery → More thirst → Death
Low Warmth → Frostbite → Medical evacuation → Mission failure
Low Morale → Shell shock → Combat ineffective → Death in battle
```

**Player Strategy**:
- Prioritization (which meter matters most RIGHT NOW?)
- Resource scarcity (limited food/water - ration or consume?)
- Risk/reward (dangerous water source or stay thirsty?)
- Long-term planning (save food for 7-day rotation or eat now?)

---

<a name="combat-systems"></a>
### 6. COMBAT & ARTILLERY SYSTEMS SPECIFICATIONS

**DESIGN PHILOSOPHY**: Artillery is the primary threat (70-75% of casualties), not enemy soldiers. Combat is defensive, not aggressive. Victory is survival, not body count.

---

#### ARTILLERY SYSTEM OVERVIEW

**Historical Foundation**:
- **40,000,000 shells fired** at Verdun over 303 days
- **~132,000 shells per day** average (varies by phase)
- **70-75% of all casualties** from artillery
- **Constant bombardment** - never truly stops, only varies in intensity

**Game Implementation**:
- Artillery is **RANDOM and UNPREDICTABLE** (by design - creates terror)
- Player must **recognize shells by SOUND** (1-30 seconds warning)
- Survival depends on **shelter quality, luck, and sound recognition**
- Direct hits are **instant death** (realistic, unforgiving)
- Near misses cause **wounds, burial, shell shock**

---

#### SHELL TYPES & CHARACTERISTICS

**TIER 1: LIGHT ARTILLERY (60% of shells fired)**

##### 75mm FRENCH FIELD GUN (Canon de 75 modèle 1897)
```json
{
  "designation": "75mm French field gun",
  "usage": "Primary French artillery - anti-personnel, counter-battery",
  "percentage_of_total": 35,
  "shells_fired_verdun": 14000000,

  "audio": {
    "incoming_sound": "Ffff-CRACK! (sharp, high-pitched whistle)",
    "warning_time_seconds": "1-3",
    "explosion_sound": "Sharp crack, metallic ring",
    "recognizability": "Very distinctive - fastest, highest pitch"
  },

  "ballistics": {
    "muzzle_velocity_m_s": 575,
    "range_max_m": 11000,
    "flight_time_typical_s": "8-15",
    "trajectory": "Relatively flat (direct fire capable)"
  },

  "damage": {
    "blast_radius_lethal_m": 10,
    "blast_radius_wound_m": 20,
    "blast_radius_shell_shock_m": 30,
    "crater_diameter_m": 2,
    "crater_depth_m": 0.5,
    "fragmentation": "High - thousands of steel fragments",
    "overpressure_psi": 15,
    "debris_radius_m": 50
  },

  "penetration": {
    "sandbag_wall": "Penetrates 2m thickness",
    "earth_dugout_roof": "Penetrates 0.5m earth",
    "concrete": "Superficial damage only",
    "trench_wall": "Destroys, causes collapse"
  },

  "effects_by_distance": {
    "0-5m": {
      "survival_chance": 0,
      "result": "Instant death (vaporization/massive trauma)",
      "player_experience": "Screen goes black instantly - no warning"
    },
    "5-10m": {
      "survival_chance": 10,
      "wounds": "Catastrophic - multiple shrapnel, internal injuries, amputation",
      "player_experience": "Knockdown, severe bleeding, unconsciousness, likely death",
      "medical_required": "Immediate evacuation or death in minutes"
    },
    "10-20m": {
      "survival_chance": 60,
      "wounds": "Severe - shrapnel wounds, concussion, hearing loss",
      "player_experience": "Knockdown, temporary deafness, vision blur, bleeding",
      "medical_required": "Field dressing + evacuation within hours"
    },
    "20-30m": {
      "survival_chance": 90,
      "wounds": "Light to moderate - shrapnel fragments, concussion",
      "player_experience": "Stagger, ringing ears, possible minor wounds",
      "medical_required": "Field dressing, can continue fighting"
    },
    "30-50m": {
      "survival_chance": 98,
      "wounds": "Minimal - possible fragment hits, shell shock",
      "player_experience": "Ears ring, morale drop, alertness spike",
      "medical_required": "None (unless unlucky fragment hit)"
    },
    "50m+": {
      "survival_chance": 100,
      "wounds": "None (unless very unlucky ricochet)",
      "player_experience": "Audio/visual scare, small morale drop",
      "medical_required": "None"
    }
  },

  "cover_modifiers": {
    "open_ground": "No protection - full damage",
    "shell_crater": "75% protection (below ground level)",
    "shallow_trench": "60% protection (2m deep)",
    "dugout_entrance": "85% protection (earth overhead)",
    "deep_dugout": "95% protection (3m+ earth)",
    "concrete_bunker": "99% protection (reinforced concrete)"
  },

  "gameplay_notes": {
    "frequency": "Very common - player hears these constantly",
    "reaction_window": "1-3 seconds - barely enough to drop prone",
    "primary_threat": "Shrapnel, not blast (fragments travel 50m+)",
    "counter_strategy": "Stay low, use craters, recognize sound fast"
  }
}
```

##### 77mm GERMAN FIELD GUN (Feldkanone 96 n.A.)
```json
{
  "designation": "77mm German field gun",
  "usage": "German equivalent to French 75mm",
  "percentage_of_total": 25,
  "shells_fired_verdun": 10000000,

  "audio": {
    "incoming_sound": "Ssssss-BANG! (sibilant hiss, sharp crack)",
    "warning_time_seconds": "2-4",
    "explosion_sound": "Slightly deeper than 75mm",
    "recognizability": "Distinctive hiss - German signature"
  },

  "damage": {
    "blast_radius_lethal_m": 12,
    "blast_radius_wound_m": 22,
    "blast_radius_shell_shock_m": 32,
    "crater_diameter_m": 2.5,
    "crater_depth_m": 0.6,
    "note": "Slightly more powerful than French 75mm"
  },

  "effects_by_distance": "Similar to 75mm, +10% damage at each range",

  "gameplay_notes": {
    "frequency": "Common when defending against German attacks",
    "distinguishing_feature": "Hissing sound vs. French whistle",
    "threat_level": "Comparable to 75mm"
  }
}
```

---

**TIER 2: MEDIUM ARTILLERY (25% of shells)**

##### 155mm HOWITZER (French/German)
```json
{
  "designation": "155mm howitzer (various models)",
  "usage": "Heavy bombardment, fortification destruction, trench clearing",
  "percentage_of_total": 20,
  "shells_fired_verdun": 8000000,

  "audio": {
    "incoming_sound": "Whummm...WHUMP (low rumble building to thunder)",
    "warning_time_seconds": "3-8",
    "explosion_sound": "Deep, ground-shaking BOOM",
    "recognizability": "Distinctive low rumble - 'freight train' sound"
  },

  "ballistics": {
    "muzzle_velocity_m_s": 450,
    "range_max_m": 12000,
    "flight_time_typical_s": "15-25",
    "trajectory": "High arc (indirect fire only)"
  },

  "damage": {
    "blast_radius_lethal_m": 20,
    "blast_radius_wound_m": 40,
    "blast_radius_shell_shock_m": 60,
    "blast_radius_buried_alive_m": 15,
    "crater_diameter_m": 6,
    "crater_depth_m": 1.5,
    "fragmentation": "Massive - house-sized kill zone",
    "overpressure_psi": 25,
    "debris_radius_m": 100,
    "trench_collapse_radius_m": 10
  },

  "penetration": {
    "sandbag_wall": "Destroys completely (3m+ thickness)",
    "earth_dugout_roof": "Penetrates 2m earth, causes collapse",
    "concrete": "Cracks 0.5m concrete, destroys thin walls",
    "trench_wall": "Obliterates 10m+ of trench"
  },

  "effects_by_distance": {
    "0-10m": {
      "survival_chance": 0,
      "result": "Instant death (complete disintegration)",
      "player_experience": "Screen black - no survival possible"
    },
    "10-20m": {
      "survival_chance": 2,
      "wounds": "Catastrophic - multiple amputations, internal rupture",
      "special": "Burial alive (50% chance) - oxygen timer starts",
      "player_experience": "Massive knockdown, possible burial, near-certain death"
    },
    "20-40m": {
      "survival_chance": 40,
      "wounds": "Severe - major shrapnel, broken bones, concussion",
      "special": "Burial alive (20% chance), temporary deafness (100%)",
      "player_experience": "Violent knockdown, bleeding, disorientation, medical evacuation needed"
    },
    "40-60m": {
      "survival_chance": 75,
      "wounds": "Moderate - shrapnel wounds, concussion, hearing damage",
      "special": "Shell shock risk (30%)",
      "player_experience": "Knockdown, ringing ears, vision blur, morale crash"
    },
    "60-100m": {
      "survival_chance": 95,
      "wounds": "Light - possible fragment hits, shell shock",
      "special": "Shell shock risk (10%)",
      "player_experience": "Ground shake, terror, morale drop, debris falling"
    },
    "100m+": {
      "survival_chance": 99,
      "wounds": "Minimal",
      "player_experience": "Distant boom, ground tremor, psychological impact"
    }
  },

  "cover_modifiers": {
    "open_ground": "No protection",
    "shell_crater": "50% protection (inadequate against this size)",
    "shallow_trench": "40% protection (can cause trench collapse)",
    "dugout_entrance": "70% protection",
    "deep_dugout": "90% protection (3m+ earth)",
    "concrete_bunker": "95% protection (thick concrete)"
  },

  "special_mechanics": {
    "burial_alive": {
      "trigger": "Within 20m and in trench/crater",
      "chance": "10-50% depending on distance",
      "effect": "Player buried under earth/debris",
      "oxygen_timer": "5-10 minutes to dig out or suffocate",
      "escape_mechanic": "Rapid button press + stamina cost",
      "ai_rescue": "Nearby soldiers may dig you out (if alive)"
    },
    "trench_collapse": {
      "radius": 10,
      "effect": "Trench walls collapse, burying occupants",
      "survivors": "Anyone not in direct blast but in collapse zone buried"
    }
  },

  "gameplay_notes": {
    "frequency": "Moderate - several per minute during bombardments",
    "reaction_window": "3-8 seconds - enough time to sprint to dugout",
    "primary_threat": "Blast overpressure + burial, not just shrapnel",
    "counter_strategy": "Deep dugout required, shallow cover insufficient",
    "terror_factor": "High - ground shakes, deafening, causes panic"
  }
}
```

---

**TIER 3: HEAVY ARTILLERY (10% of shells)**

##### 210mm HOWITZER (German)
```json
{
  "designation": "210mm heavy howitzer (21cm Mörser)",
  "usage": "Fort bombardment, deep dugout destruction",
  "percentage_of_total": 5,
  "shells_fired_verdun": 2000000,

  "audio": {
    "incoming_sound": "RRROOOOAAAARRRR (sustained roar, earthquake sound)",
    "warning_time_seconds": "8-15",
    "explosion_sound": "Earthquake-like BOOM, shockwave felt 500m+",
    "recognizability": "Unmistakable - sounds like incoming train"
  },

  "damage": {
    "blast_radius_lethal_m": 25,
    "blast_radius_wound_m": 50,
    "blast_radius_shell_shock_m": 80,
    "blast_radius_buried_alive_m": 20,
    "crater_diameter_m": 10,
    "crater_depth_m": 2.5,
    "overpressure_psi": 35,
    "debris_radius_m": 150,
    "trench_collapse_radius_m": 15
  },

  "penetration": {
    "earth_dugout_roof": "Penetrates 3m+ earth",
    "concrete": "Destroys 1m concrete",
    "fort_casemate": "Damages but rarely penetrates thick walls"
  },

  "effects_by_distance": {
    "0-25m": "Instant death, no survival",
    "25-50m": "5% survival, catastrophic wounds, burial",
    "50-80m": "50% survival, severe wounds, shell shock guaranteed",
    "80-150m": "90% survival, psychological trauma, debris danger"
  },

  "gameplay_notes": {
    "frequency": "Rare - a few per hour",
    "warning": "Long warning time allows retreat to deepest shelter",
    "threat": "Even deep dugouts can collapse",
    "psychological": "Single shell terrorizes entire sector"
  }
}
```

##### 305mm MORTAR (French Mortier de 305)
```json
{
  "designation": "305mm heavy mortar",
  "usage": "German position destruction, counter-battery",
  "percentage_of_total": 3,
  "shells_fired_verdun": 1200000,

  "audio": {
    "incoming_sound": "WHOOOOOM-WHOOOOOM (low bass wobble)",
    "warning_time_seconds": "10-20",
    "explosion_sound": "Ground-shaking concussion, felt 1km+"
  },

  "damage": {
    "blast_radius_lethal_m": 30,
    "blast_radius_wound_m": 60,
    "blast_radius_shell_shock_m": 100,
    "crater_diameter_m": 15,
    "crater_depth_m": 3,
    "note": "Creates craters visible from aircraft"
  },

  "gameplay_notes": {
    "frequency": "Very rare - occasional throughout day",
    "player_experience": "Apocalyptic - entire screen shakes",
    "survival": "Only deepest bunkers or being far away saves you"
  }
}
```

---

**TIER 4: SUPER-HEAVY ARTILLERY (2% of shells)**

##### 420mm GAMMA MORTAR / "Big Bertha" Type
```json
{
  "designation": "420mm super-heavy mortar (various German models)",
  "usage": "Fort destruction, symbolic terror bombardment",
  "percentage_of_total": 2,
  "shells_fired_verdun": 800000,

  "audio": {
    "incoming_sound": "RUUUUUMMMMBBBBLLLLLEEEEE (sustained earthquake rumble)",
    "warning_time_seconds": "20-40",
    "explosion_sound": "Apocalyptic blast heard 10km+ away",
    "recognizability": "Everyone within 1km knows it's coming"
  },

  "ballistics": {
    "shell_weight_kg": 800,
    "range_max_m": 14000,
    "flight_time_s": "40-60",
    "trajectory": "Very high arc (shell reaches 3000m altitude)"
  },

  "damage": {
    "blast_radius_lethal_m": 50,
    "blast_radius_wound_m": 100,
    "blast_radius_shell_shock_m": 150,
    "blast_radius_buried_alive_m": 30,
    "crater_diameter_m": 30,
    "crater_depth_m": 6,
    "overpressure_psi": 50,
    "debris_radius_m": 300,
    "trench_collapse_radius_m": 25,
    "seismic_effect": "Felt as earthquake 2-3km away"
  },

  "penetration": {
    "earth_dugout_roof": "Penetrates 5m+ earth (deep dugouts collapse)",
    "concrete": "Destroys 2m concrete",
    "fort_casemate": "Can penetrate fort roofs with direct hit"
  },

  "effects_by_distance": {
    "0-50m": {
      "survival_chance": 0,
      "result": "Obliteration - nothing remains",
      "player_experience": "Instant death cutscene - no escape"
    },
    "50-100m": {
      "survival_chance": 1,
      "wounds": "Total body trauma, mass burial",
      "player_experience": "Violent earthquake, burial, near-certain death"
    },
    "100-150m": {
      "survival_chance": 20,
      "wounds": "Catastrophic - blast injuries, burial, ruptured organs",
      "special": "Shell shock guaranteed, temporary blindness/deafness",
      "player_experience": "World goes white, then black, confusion, disorientation"
    },
    "150-300m": {
      "survival_chance": 70,
      "wounds": "Severe concussion, debris strikes, hearing loss",
      "special": "Shell shock 80% chance",
      "player_experience": "Knocked down, debris falling like rain, terror"
    },
    "300m+": {
      "survival_chance": 98,
      "wounds": "Psychological only",
      "player_experience": "Ground shakes, massive boom, see mushroom cloud of dirt"
    }
  },

  "cover_modifiers": {
    "open_ground": "Death within 50m",
    "shallow_trench": "Inadequate - burial/collapse",
    "deep_dugout": "70% protection (but can still collapse)",
    "fort_casemate": "90% protection (concrete/steel roof)",
    "note": "Even 'safe' cover can fail - pure luck element"
  },

  "gameplay_notes": {
    "frequency": "Extremely rare - maybe 1-3 per mission",
    "warning": "20-40 seconds - scramble for deepest shelter",
    "player_agency": "Run and pray - even deep cover can fail",
    "cinematic_moment": "Use sparingly for maximum terror impact",
    "psychological": "Entire sector panics - NPCs flee",
    "historical_note": "Germans used to destroy French morale, not just forts"
  }
}
```

---

#### GAS SHELLS (Special Category - 5% of shells)

##### PHOSGENE GAS SHELL (Most Common Gas Type)
```json
{
  "designation": "Phosgene gas shell (various calibers)",
  "usage": "Area denial, casualties without destroying positions",
  "percentage_of_total_gas": 60,

  "audio": {
    "incoming_sound": "Soft 'POP' or 'PFFFT' (distinctive lack of explosion)",
    "warning_time": "Immediate upon hearing pop",
    "recognizability": "Quiet landing = GAS WARNING",
    "alert": "Soldiers shout 'GAZ! GAZ! GAZ!'"
  },

  "dispersal": {
    "cloud_radius_m": 50,
    "cloud_height_m": 3,
    "duration_minutes": "10-30 depending on wind",
    "wind_drift": "Cloud moves with wind 2-5 m/s",
    "terrain_flow": "Sinks into trenches/craters (heavier than air)",
    "concentration": "Lethal for 5-10 min, dangerous for 20-30 min"
  },

  "effects_without_mask": {
    "0-30_seconds": {
      "exposure": "Light",
      "symptoms": "Coughing, eye irritation, chest tightness",
      "treatment": "Mask on + evacuate = full recovery",
      "survival": "95% if masked quickly"
    },
    "30-120_seconds": {
      "exposure": "Moderate",
      "symptoms": "Choking, vomiting, vision blur, lung burning",
      "treatment": "Mask on + medical evacuation + oxygen",
      "survival": "60% - may develop pneumonia later",
      "player_experience": "Vision blur, coughing animation, stamina crash"
    },
    "120-300_seconds": {
      "exposure": "Severe",
      "symptoms": "Fluid in lungs, choking on own fluids, collapse",
      "treatment": "Medical evacuation required, hospitalization",
      "survival": "20% - slow death over hours to days",
      "player_experience": "Cannot run, drowning in own fluids, screen fading"
    },
    "300+_seconds": {
      "exposure": "Lethal",
      "symptoms": "Unconsciousness, respiratory failure",
      "treatment": "None effective",
      "survival": "0-5% - death within 24 hours",
      "player_experience": "Collapse, fade to black, death"
    }
  },

  "gas_mask_mechanics": {
    "equip_time_seconds": 5,
    "player_input": "Hold button to put on mask (interrupts all actions)",
    "vision_penalty": "60% reduced FOV, fogged lenses, no peripheral vision",
    "stamina_penalty": "Cannot sprint, -50% stamina regen",
    "communication_penalty": "Muffled speech, hard to hear orders",
    "duration_max_minutes": 60,
    "filter_degradation": "Effectiveness drops 10% per 10 minutes in gas",
    "removal": "Cannot remove mask until gas clears (5-30 min)"
  },

  "gameplay_challenge": {
    "panic_factor": "High - must react in <5 seconds",
    "decision": "Drop everything and mask up vs. finish critical task",
    "movement": "Must evacuate gas zone while masked (can't see well)",
    "squad_impact": "NPCs without masks die - player must help or leave them",
    "moral_choice": "Save wounded soldier vs. save self from gas"
  }
}
```

##### MUSTARD GAS SHELL
```json
{
  "designation": "Mustard gas shell (Yperite)",
  "usage": "Persistent area denial (remains dangerous for days)",
  "percentage_of_total_gas": 30,

  "dispersal": {
    "cloud_radius_m": 40,
    "duration_hours": "24-72 (persists on ground)",
    "liquid_contamination": "Pools in shell holes, contaminates equipment",
    "note": "Can't see or smell it - insidious threat"
  },

  "effects": {
    "skin_contact": {
      "onset": "2-24 hours delayed (player won't know immediately)",
      "symptoms": "Blisters, chemical burns, skin sloughing off",
      "treatment": "Decontamination + hospitalization",
      "gameplay": "Delayed damage - player continues mission, then collapses hours later"
    },
    "inhalation": {
      "onset": "4-24 hours delayed",
      "symptoms": "Lung damage, blindness, internal bleeding",
      "survival": "50% with treatment"
    },
    "contaminated_equipment": {
      "effect": "Touching contaminated gear causes burns",
      "gameplay": "Must discard gear or suffer penalties"
    }
  },

  "gameplay_notes": {
    "insidious_threat": "Player may not realize they're exposed until later",
    "terrain_hazard": "Contaminated zones remain dangerous for days",
    "difficult_choice": "Retrieve wounded from gas zone or abandon them?"
  }
}
```

---

#### BOMBARDMENT PATTERNS & INTENSITY LEVELS

```json
{
  "intensity_levels": {
    "harassing_fire": {
      "description": "Sporadic shells to prevent rest/movement",
      "frequency": "1-3 shells per minute (mixed calibers)",
      "duration": "Constant (24 hours)",
      "purpose": "Psychological wear, prevent sleep",
      "player_experience": "Can't relax, constant background danger",
      "casualties_per_hour": "5-10 soldiers per 1000"
    },

    "light_bombardment": {
      "description": "Sustained shelling of sector",
      "frequency": "10-20 shells per minute",
      "duration": "1-4 hours",
      "purpose": "Suppress defenders, preparation for attack",
      "player_experience": "Must take cover, can't move freely",
      "casualties_per_hour": "20-40 per 1000"
    },

    "heavy_bombardment": {
      "description": "Intense shelling, trench destruction",
      "frequency": "40-60 shells per minute",
      "duration": "2-8 hours",
      "purpose": "Destroy trenches, kill defenders, break morale",
      "player_experience": "Shelter in deepest dugout, pray, shell shock risk high",
      "casualties_per_hour": "60-100 per 1000"
    },

    "drumfire": {
      "description": "Trommelfeuer - continuous roar (Feb 21, Oct 20-23)",
      "frequency": "100+ shells per minute (sounds like one continuous explosion)",
      "duration": "4-12 hours",
      "purpose": "Total annihilation, make assault possible",
      "player_experience": "Cannot think, cannot hear, world is ending, shell shock guaranteed",
      "casualties_per_hour": "150-200 per 1000",
      "historical_examples": [
        "Feb 21, 1916 07:15-16:00 (9 hours) - Battle starts",
        "Oct 20-23, 1916 (4 days) - 1.15 million shells before Douaumont recapture"
      ]
    }
  },

  "creeping_barrage": {
    "description": "Artillery fire that advances ahead of attacking infantry",
    "movement_rate": "50-100m per 3-4 minutes",
    "purpose": "Suppress defenders while infantry advances behind",
    "player_experience": {
      "defending": "Wall of explosions advancing toward you - terror",
      "attacking": "Must advance close behind explosions (50m) or lose support"
    },
    "danger": "Too close = hit by own artillery, too far = defenders recover",
    "gameplay_mechanic": "Player must maintain distance from moving wall of fire"
  },

  "box_barrage": {
    "description": "Artillery fired in square pattern to trap enemy",
    "purpose": "Isolate sector, prevent reinforcement/retreat",
    "player_experience": "Cannot leave area - surrounded by explosions",
    "escape": "Wait for barrage to end or risk running through it (low survival)"
  }
}
```

---

#### SHELL RECOGNITION MINIGAME

**Core Gameplay Loop**: Hear → Identify → React → Survive

```json
{
  "audio_recognition_system": {
    "player_skill": "Improves with experience (learning curve)",
    "veteran_bonus": "After 50 shells survived, reaction time +30%",

    "recognition_stages": {
      "novice": {
        "player_status": "First missions",
        "recognition_time": "2-4 seconds to identify shell type",
        "accuracy": "50% correct identification",
        "panic_chance": "High - may freeze or wrong reaction"
      },
      "experienced": {
        "player_status": "10+ hours gameplay",
        "recognition_time": "0.5-1 second",
        "accuracy": "80% correct",
        "panic_chance": "Moderate"
      },
      "veteran": {
        "player_status": "30+ hours, late campaign",
        "recognition_time": "Instant (<0.5 sec)",
        "accuracy": "95% correct",
        "panic_chance": "Low - numb, automatic response"
      }
    },

    "decision_tree": {
      "hear_shell": {
        "step_1": "Identify type (audio cue)",
        "step_2": "Estimate distance (volume/pitch)",
        "step_3": "Calculate time to impact",
        "step_4": "Choose reaction",
        "time_available": "1-30 seconds depending on shell"
      },

      "reactions": {
        "ignore": {
          "when": "Shell landing far away (100m+)",
          "risk": "Low (unless bad estimate)",
          "reward": "Continue task, no time lost"
        },
        "crouch": {
          "when": "75mm at 30-50m",
          "time_cost": "0 seconds",
          "protection": "Minimal, reduces shrapnel hits"
        },
        "drop_prone": {
          "when": "75mm/155mm at 20-40m",
          "time_cost": "1 second",
          "protection": "Moderate - reduces blast exposure"
        },
        "sprint_to_crater": {
          "when": "155mm at 10-30m",
          "time_cost": "2-5 seconds",
          "protection": "High - below ground level",
          "risk": "May not reach in time"
        },
        "sprint_to_dugout": {
          "when": "305mm+ or sustained bombardment",
          "time_cost": "5-15 seconds",
          "protection": "Very high - earth overhead",
          "risk": "Long exposure while running"
        },
        "freeze_and_pray": {
          "when": "No time to react or already in cover",
          "protection": "Luck-based",
          "player_experience": "Brace for impact, screen shake, hope"
        }
      }
    },

    "wrong_decision_consequences": {
      "overreaction": "Dive for 75mm landing 100m away - waste stamina, look foolish",
      "underreaction": "Ignore 155mm landing 15m away - severe wounds or death",
      "late_recognition": "Identify correctly but too slow - hit while running",
      "misidentification": "Think it's 75mm, actually 305mm - death"
    }
  },

  "audio_design_requirements": {
    "3d_spatial_audio": "CRITICAL - player must locate shell by sound",
    "doppler_effect": "Pitch change indicates shell passing overhead vs. incoming",
    "distance_attenuation": "Volume indicates proximity",
    "multiple_simultaneous": "During bombardments, 5-10 shells in air at once",
    "frequency_separation": "Each shell type has distinct frequency signature",
    "environmental_occlusion": "Dugout walls muffle external sounds"
  }
}
```

---

#### SMALL ARMS COMBAT (10-15% of gameplay)

**Design Philosophy**: Combat is defensive. Player rarely initiates. Shooting accurately while exhausted/terrified is HARD.

##### RIFLE COMBAT (French Lebel, German Gewehr 98)

```json
{
  "lebel_model_1886": {
    "type": "Bolt-action rifle",
    "caliber": "8mm Lebel",
    "capacity": 8,
    "reload_time_seconds": 12,
    "rate_of_fire": "15 rounds per minute (trained soldier)",

    "accuracy": {
      "base_accuracy": 75,
      "morale_modifier": "-30% if morale < 40",
      "stamina_modifier": "-20% if stamina < 30",
      "alertness_modifier": "-40% if alertness < 20",
      "hunger_modifier": "-15% if hunger < 30",
      "wounded_modifier": "-50% if wounded",
      "shell_shock_modifier": "-70% if in shell shock state",
      "practical_combat_accuracy": "30-50% for exhausted soldier (realistic)"
    },

    "effective_range": {
      "point_target": "100-300m (individual enemy)",
      "area_target": "300-600m (group of enemies)",
      "maximum_range": "2000m (mostly suppression)"
    },

    "damage": {
      "torso_hit": "Incapacitating wound, death in minutes to hours",
      "head_hit": "Instant death",
      "limb_hit": "Severe wound, bleeding, combat ineffective",
      "note": "One hit = out of action (realistic, not CoD)"
    },

    "gameplay_notes": {
      "aim_sway": "Heavy sway if survival meters low",
      "breath_hold": "3-5 seconds for steady shot (costs stamina)",
      "combat_is_rare": "Most missions have 0-10 shots fired by player",
      "defensive_posture": "Shooting from trench, not advancing",
      "suppression": "Enemy fire forces player to keep head down"
    }
  }
}
```

##### MACHINE GUN (Defensive)

```json
{
  "hotchkiss_m1914": {
    "type": "Heavy machine gun (defensive emplacement)",
    "rate_of_fire": "450 rounds per minute",
    "usage": "Defend trench from assault",

    "gameplay": {
      "player_role": "Man MG during German assault",
      "target_acquisition": "Groups of advancing infantry",
      "overheating": "Barrel overheats after 200 continuous rounds - must change or wait",
      "resupply": "Requires ammo bearer (NPC) to feed belts",
      "vulnerability": "Exposed position - artillery target, sniper target",
      "moral_weight": "Killing 20+ men in 30 seconds - morale impact"
    },

    "historical_accuracy": {
      "effectiveness": "Machine guns caused 30% of combat casualties",
      "psychological": "Defenders felt guilt - 'slaughter' not 'combat'",
      "vulnerability": "MG crews had high casualty rates (targeted)"
    }
  }
}
```

##### GRENADES

```json
{
  "grenade_f1": {
    "type": "French fragmentation grenade",
    "fuse_time": "5 seconds",
    "throw_range": "20-30m (stamina-dependent)",
    "blast_radius": "10m lethal, 20m wound",

    "usage": {
      "trench_clearing": "Throw into enemy trench section before assault",
      "defensive": "Throw at advancing Germans",
      "danger": "Can bounce back if hits wall - panic moment"
    },

    "gameplay_mechanic": {
      "throw_arc": "Physics-based (can miss, overshoot, undershoot)",
      "cook_grenade": "Hold to reduce fuse time (risky - can explode in hand)",
      "friendly_fire": "Can hit own soldiers if poorly thrown"
    }
  }
}
```

---

#### MELEE COMBAT (Rare, Brutal)

```json
{
  "melee_weapons": {
    "trench_knife": {
      "usage": "Last resort, trench raids, silent kills",
      "lethality": "Instant kill if successful",
      "risk": "Close quarters = high danger"
    },
    "shovel": {
      "usage": "Improvised weapon, common in trench fighting",
      "effectiveness": "Brutal but less reliable than knife"
    },
    "bayonet": {
      "usage": "Rifle-mounted, for assaults",
      "historical_note": "Rarely used despite iconic status (most killed by bullets/shells)"
    }
  },

  "melee_gameplay": {
    "frequency": "Very rare - 1-2 encounters per campaign",
    "player_experience": "Quick-time event OR skill-based parry system",
    "stakes": "You or them - binary outcome",
    "morale_cost": "Killing someone face-to-face = massive morale drop",
    "psychological": "Different from shooting - intimate violence, trauma"
  }
}
```

---

## [COMBAT SYSTEMS COMPLETE]

**Status**: Combat & Artillery Systems FULLY SPECIFIED

**Next Section**: Mission Design (30+ missions using these systems)

---

<a name="mission-structure"></a>
## PART III: MISSION DESIGN

### MISSION STRUCTURE: ROTATION-BASED CAMPAIGN

**Core Design Philosophy**: The player's goal is to **survive the entire 303-day Battle of Verdun** (February 21 - December 18, 1916) by rotating in and out of combat through the **Noria system**.

---

#### THE NORIA ROTATION SYSTEM

**Historical Foundation**:
- French divisions rotated through Verdun to prevent total breakdown
- Cycle: Front Line → Support Line → Rest Area → Repeat
- Each rotation lasted 4-7 days on average (varied by intensity)
- Over 70 French divisions cycled through Verdun during 303 days
- Prevented any single unit from being annihilated by prolonged exposure

**Game Implementation**:

```json
{
  "rotation_cycle": {
    "front_line": {
      "duration_days": "4-7 (varies by battle intensity)",
      "location": "First trench, direct enemy contact",
      "activities": [
        "Survive bombardments",
        "Repel German assaults",
        "Man defensive positions",
        "Night patrols",
        "Sentry duty"
      ],
      "survival_difficulty": "EXTREME",
      "death_risk": "High (70-80% of casualties occur here)",
      "player_experience": "Terror, exhaustion, constant danger"
    },

    "support_line": {
      "duration_days": "4-7",
      "location": "Second trench, 200-500m behind front",
      "activities": [
        "Resupply front line",
        "Repair trenches",
        "Burial details",
        "Reserve for counterattacks",
        "Receive wounded"
      ],
      "survival_difficulty": "HIGH",
      "death_risk": "Moderate (still within artillery range)",
      "player_experience": "Fatigue, ongoing danger, some respite"
    },

    "rest_area": {
      "duration_days": "7-14",
      "location": "Villages 5-10km behind lines (Dugny, Bras, etc.)",
      "activities": [
        "Sleep (real sleep, not interrupted)",
        "Receive mail from home",
        "Hot meals, baths, delousing",
        "Equipment repair",
        "Training replacements",
        "Write letters home"
      ],
      "survival_difficulty": "LOW",
      "death_risk": "Minimal (occasional long-range artillery)",
      "player_experience": "Relief, recovery, anticipatory dread of return"
    },

    "return_to_front": {
      "duration": "1 day (march back to trenches)",
      "player_experience": "Growing dread, final preparations, goodbyes",
      "psychological": "Hardest moment - knowing what's coming"
    }
  },

  "full_rotation_duration": "15-28 days total (varies)",
  "rotations_in_campaign": "10-20 rotations over 303 days",
  "player_sees_battle_evolve": "February desperation → June intensity → October climax → December relief"
}
```

---

#### CAMPAIGN STRUCTURE: 30+ MISSIONS

**Mission Types**:

1. **ROTATION MISSIONS** (20-25 missions)
   - Each rotation = 1 mission
   - Player must survive 4-7 days on front line
   - Victory = Survive until relief arrives
   - Failure = Death (checkpoint respawn OR permadeath mode)

2. **SPECIAL EVENT MISSIONS** (5-8 missions)
   - Fort Vaux Siege (7-day multi-stage mission)
   - Oct 24 Douaumont Recapture (climax mission)
   - Night patrol missions
   - Supply run missions (Voie Sacrée)

3. **TUTORIAL MISSIONS** (2-3 missions)
   - First Day (Feb 21) - Learn survival basics
   - First Rotation (Feb 22-28) - Complete rotation cycle

---

#### PROGRESSION SYSTEM

**Player Transformation Over 303 Days**:

```json
{
  "february_march": {
    "player_status": "NOVICE",
    "shell_recognition": "50% accuracy, 2-4 sec reaction",
    "morale": "High (80-100) - still believes in cause",
    "survival_meters": "Degrade quickly",
    "death_risk": "Very high (inexperience kills)",
    "psychological_state": "Fear, shock, disbelief"
  },

  "april_june": {
    "player_status": "EXPERIENCED",
    "shell_recognition": "80% accuracy, 0.5-1 sec reaction",
    "morale": "Moderate (50-70) - grim determination",
    "survival_meters": "More efficient management",
    "death_risk": "Moderate (competence improves odds)",
    "psychological_state": "Numbness setting in, autopilot mode"
  },

  "july_september": {
    "player_status": "VETERAN",
    "shell_recognition": "95% accuracy, <0.5 sec reaction",
    "morale": "Low (30-50) - hollow, enduring",
    "survival_meters": "Near-automatic management",
    "death_risk": "Lower (but never safe)",
    "psychological_state": "Dissociation, survivor guilt, 'living ghost'"
  },

  "october_december": {
    "player_status": "SURVIVOR",
    "shell_recognition": "Instant, unconscious",
    "morale": "Variable (hope returns as French advance)",
    "survival_meters": "Mastered but depleted",
    "death_risk": "Moderate (climax battles dangerous)",
    "psychological_state": "Cautious hope, fear of dying at the end",
    "narrative": "So close to surviving... don't die now"
  }
}
```

**Unlocks & Skills**:
- **Shell recognition** improves with experience (automatic skill gain)
- **Veteran bonuses**: +30% reaction time, -20% stamina drain from panic
- **Coping mechanisms unlock**: Dark humor, rituals, dissociation (morale management)
- **No weapon unlocks** - not a power fantasy, same rifle entire game

---

#### VICTORY CONDITIONS

**Mission-Level Victory** (per rotation):
- ✅ Survive until relieved (4-7 days)
- ✅ Complete assigned duties (sentry, patrol, defense)
- ✅ Optional: Save squadmates (moral choices)

**Campaign-Level Victory** (final goal):
- ✅ **SURVIVE 303 DAYS** - Reach December 18, 1916 alive
- ✅ Witness battle's end (final mission epilogue)
- ✅ Optional: Maximum squadmates saved (companion survival tracker)

**Permadeath Mode** (optional difficulty):
- ❌ No checkpoints - death = restart entire campaign
- 🎖️ For players seeking ultimate realism/challenge
- 📊 Campaign completion rate: Estimated <5% (realistic to actual survival odds)

---

#### FAILURE CONDITIONS

**Mission Failure** (checkpoint respawn):
- Death from artillery
- Death from combat
- Death from gas exposure
- Death from starvation/dehydration (survival meters at 0)
- Shell shock breakdown (evacuation = mission failure, rejoin next rotation)

**Campaign Failure** (permadeath mode only):
- Death = Start over from February 21
- OR: Play as new soldier (different regiment, continue from current date)

---

#### TIME PROGRESSION

**Real-Time vs. Game-Time**:
- **1 day in-game ≈ 20-40 minutes real-time** (compressed)
- **Front-line rotation (4-7 days) ≈ 2-4 hours gameplay**
- **Full campaign (303 days) ≈ 40-60 hours** for all missions

**Time Compression**:
- Daytime: Accelerated time (1 hour = 2-3 minutes)
- Critical events: Real-time (combat, patrols, bombardments)
- Rest periods: Fast-forward option (sleep, travel)
- Night: Slower time (more dangerous, player needs to be alert)

**Calendar System**:
- Always visible: Current date, days survived, days until relief
- Historical events noted: "Oct 24 - French counteroffensive begins"
- Morale affected by calendar: "Only 50 days left..." vs. "200 more days of this?"

---

#### HISTORICAL TIMELINE INTEGRATION

**Missions Follow Historical Progression**:

| **Phase** | **Dates** | **Historical Event** | **Player Experience** | **Rotations** |
|-----------|-----------|---------------------|----------------------|---------------|
| Phase I | Feb 21-Mar 6 | Initial German assault | Desperate defense, chaos | 1-2 |
| Phase II | Mar 6-Apr 9 | Left Bank offensive | Mort-Homme, Hill 304 fighting | 2-3 |
| Phase III | Apr 9-May 7 | Renewed attacks on forts | Grinding attrition | 2-3 |
| Phase IV | May 7-Jun 23 | Fort Vaux siege, German apex | **CLIMAX 1**: Vaux siege, Souville high-water mark | 2-4 |
| Phase V | Jun 23-Oct 24 | Stalemate, Somme drains Germans | Lower intensity, prep for counterattack | 3-5 |
| Phase VI | Oct 24-Nov 2 | French counteroffensive begins | **CLIMAX 2**: Douaumont recapture | 1-2 |
| Phase VII | Nov 2-Dec 18 | French recapture gains | Victory push, cautious hope | 2-3 |

**Key Historical Missions** (must include):
1. ✅ Feb 21 - First Day (tutorial)
2. ✅ Feb 25 - Douaumont falls (witness from distance)
3. ✅ Mar 6 - Mort-Homme assault begins (if playing Left Bank rotation)
4. ✅ Jun 1-7 - Fort Vaux siege (multi-day special mission)
5. ✅ Jun 23 - German high-water mark at Souville (desperate defense)
6. ✅ **Oct 24 - Douaumont recapture** (CENTERPIECE MISSION)
7. ✅ Nov 2 - Vaux recaptured
8. ✅ Dec 15-18 - Final offensive (victory, but exhausted)

---

#### PLAYER CHOICE & AGENCY

**What Player CAN Control**:
- ✅ How to manage survival meters (eat now or save rations?)
- ✅ How to react to artillery (which cover, when to run?)
- ✅ Whether to help wounded squadmates (risk self to save others?)
- ✅ How to cope psychologically (dark humor, letters, dissociation?)
- ✅ Combat tactics (when to shoot, conserve ammo, hold fire?)

**What Player CANNOT Control**:
- ❌ Historical outcomes (Douaumont falls Feb 25 no matter what player does)
- ❌ When they rotate in/out (orders determine schedule)
- ❌ The battle's timeline (events happen on historical dates)
- ❌ Whether squadmates live/die (some deaths scripted for narrative)

**Moral Choices** (impact morale, not history):
- Save wounded soldier vs. preserve self (morale vs. survival meters)
- Share rations vs. hoard (camaraderie vs. hunger)
- Risk self on patrol vs. stay safe (duty vs. survival instinct)
- Mercy-kill dying soldier vs. let suffer (humanity vs. trauma)

---

#### DIFFICULTY MODES

```json
{
  "recruit": {
    "description": "Experience the story with manageable challenge",
    "survival_meter_drain": "70% normal rate",
    "artillery_frequency": "70% normal frequency",
    "checkpoint_frequency": "Generous (every 6 hours in-game)",
    "permadeath": false,
    "aim_assist": "Moderate",
    "target_audience": "Story-focused players, history students"
  },

  "soldier": {
    "description": "Balanced challenge - intended experience",
    "survival_meter_drain": "100% normal rate",
    "artillery_frequency": "100% historical frequency",
    "checkpoint_frequency": "Standard (every 12 hours in-game)",
    "permadeath": false,
    "aim_assist": "Minimal",
    "target_audience": "Most players"
  },

  "veteran": {
    "description": "Brutal realism - high challenge",
    "survival_meter_drain": "130% normal rate",
    "artillery_frequency": "120% frequency (RNG nightmare)",
    "checkpoint_frequency": "Sparse (every 24 hours in-game)",
    "permadeath": false,
    "aim_assist": "None",
    "shell_recognition": "Player must learn audio cues (no UI hints)",
    "target_audience": "Hardcore survival game fans"
  },

  "ironman": {
    "description": "Permadeath - ultimate challenge",
    "survival_meter_drain": "100% normal rate",
    "artillery_frequency": "100% historical frequency",
    "checkpoint_frequency": "NONE",
    "permadeath": true,
    "death_consequence": "Restart from Feb 21 OR play as new soldier",
    "target_audience": "Masochists, achievement hunters",
    "estimated_completion_rate": "<5% (realistic to actual survival odds)"
  }
}
```

---

---

### DAY-IN-THE-LIFE MISSION EXAMPLES

**Purpose**: These examples show hour-by-hour gameplay during different rotation phases, demonstrating how survival systems, artillery mechanics, and mission structure combine into actual gameplay.

---

#### EXAMPLE 1: FRONT-LINE ROTATION MISSION

**MISSION**: "Les Éparges Sector - March Rotation"
- **Date**: March 15-21, 1916 (7 days)
- **Location**: Les Éparges, Right Bank
- **Player Status**: Experienced (1 month survival)
- **Weather**: Cold, rain, ankle-deep mud
- **Artillery Intensity**: Heavy (Phase II - Left Bank offensive)
- **Gameplay Duration**: 3-4 hours real-time
- **Victory Condition**: Survive until March 21 relief

---

##### DAY 1 - MARCH 15 (ARRIVAL)

**04:00 - Pre-Dawn March to Trenches**
```
Location: Communication trench from reserve area
Activity: March to front line (1.5km, 2 hours through mud)

Gameplay:
- Follow squad through winding communication trench
- Navigate mud (movement speed -40%, stamina drain +50%)
- Carry 30kg pack (rifle, ammo, rations, gas mask, gear)
- Hear distant artillery (builds dread)

Survival Meters:
  Stamina: 100 → 60 (heavy exertion)
  Warmth: 70 → 50 (cold rain)
  Morale: 60 → 55 (returning to front, anticipatory dread)
  Alertness: 70 (waking up)

Player Actions:
- Manage stamina (rest when possible, don't collapse)
- Stay with squad (getting lost = death)
- Listen to veteran's advice ("Stay low near the front")

Narrative Moments:
- Pass wounded being evacuated (morale -5)
- See shell crater with dead mule inside
- NPC dialogue: "Here we go again... At least it's only 7 days this time"
```

**06:00 - Arrival at Front Trench**
```
Location: First line trench, Bois d'Haumont sector
Activity: Relief of previous unit, take positions

Gameplay:
- Enter dugout (3m deep, earth walls, 8 men capacity)
- Receive briefing from officer
  - "German positions 200m east"
  - "Stand-to at 06:30 and 18:30 daily"
  - "No patrols today, just hold the line"
- Claim sleeping space (straw mat on earth)
- Store gear, check equipment

Survival Meters:
  Hunger: 80 → 70 (no breakfast yet)
  Thirst: 75 → 60 (exertion from march)
  Warmth: 50 → 60 (inside dugout, out of rain)
  Hygiene: 85 → 80 (muddy from march)

Environmental Observations:
- Trench walls partially collapsed (needs repair)
- Rats everywhere (dozens visible)
- Bodies partially buried in parapets (smell, visual horror)
- Shell craters 5-10m apart across no-man's-land

First Survival Decisions:
- Eat rations now (restore hunger) or save for later?
- Drink water now (thirst 60) or ration (only 1L canteen)?
- Change into dry socks (warmth +5, prevents trench foot) or save them?
```

**06:30 - Morning Stand-To**
```
Activity: All soldiers man firing positions, watch for dawn attack

Historical Basis: Standard WWI practice - most attacks came at dawn
Duration: 1 hour (until 07:30)

Gameplay:
- Climb fire-step, peer over parapet
- Hold rifle, watch German lines through periscope
- NPCs on left and right, spread 2m apart
- Tense silence (only sound: distant artillery, rain, mud squelching)

Artillery:
- Harassing fire (German 77mm, 1-2 shells per minute)
- Random impacts 20-100m away
- Shell recognition tutorial refresher:
  - "Ssss-BANG!" = 77mm, 3 seconds warning
  - One lands 30m away → Drop below parapet → Ears ring
  - Morale -3

Survival Meters (after 1 hour standing in rain):
  Stamina: 60 → 55 (standing in cold)
  Warmth: 60 → 50 (exposed to rain)
  Alertness: 70 → 75 (adrenaline from artillery)
  Morale: 55 → 52 (near-misses, corpses visible in no-man's-land)

Narrative Moment:
- Veteran NPC: "See that crater at 50m? That's where Beaumont died last week. 155mm, direct hit. Didn't find enough of him to bury."
- Morale -5
```

**07:30 - Stand-Down, Morning Rations**
```
Activity: Return to dugout, eat breakfast

Rations Issued:
- 350g bread (Hunger +20)
- 150g canned beef (Hunger +18)
- 125ml wine (Hunger +3, Morale +5, Warmth +3)
- Coffee (hot, if fire available) (Alertness +10, Warmth +5)

Player Decision:
- Eat full ration now (Hunger 70 → 100) but none until evening
- OR: Eat half (Hunger 70 → 85) and save rest for emergency

Survival Meters After Eating (if full ration):
  Hunger: 100
  Thirst: 60 → 75 (coffee)
  Warmth: 50 → 58 (hot coffee)
  Alertness: 75 → 85 (coffee)
  Morale: 52 → 57 (hot meal)

Squad Interaction:
- NPCs eat together in dugout
- Dialogue options:
  - Listen to stories (morale +2, learn about sector)
  - Write letter home (morale +5, time cost 10 min)
  - Sleep (restore alertness, but vulnerable during bombardment)
  - Inspect equipment (find issues before they become problems)

Rat Encounter:
- Rat steals bread if player looks away
- Choice: Chase rat (lose bread) or shoot rat (waste bullet, alert officer)
- Morale -2 (constant pest stress)
```

**08:00 - 12:00 - Daytime Duties**
```
Activity Rotation (player assigned one task, 2-4 hours):

TASK 1: Sentry Duty
- Stand at firing position with periscope
- Watch for German activity
- Report movement to officer
- Duration: 2 hours
- Stamina drain: Moderate
- Danger: High (exposed to snipers if careless)
- Boredom vs. Terror: Long tedium interrupted by artillery

TASK 2: Trench Repair
- Shovel collapsed earth back onto parapet
- Replace sandbags (carry 20kg bags)
- Repair duckboards (wooden planks over mud)
- Duration: 3 hours
- Stamina drain: Very high (-30 stamina)
- Danger: Moderate (artillery can hit while working)
- Player must balance: Work speed vs. conservation of stamina

TASK 3: Resupply Detail
- Carry ammo/water from support trench to front
- 200m round trip through communication trench
- 4-6 trips
- Duration: 2 hours
- Stamina drain: Extreme (-40 stamina)
- Danger: High (communication trenches are shelled)

TASK 4: Stand-By Reserve
- Wait in dugout, ready to respond to attack
- Can sleep, write, talk, clean weapon
- Duration: 4 hours
- Stamina: Resting (regen +20)
- Danger: Low (unless dugout hit)
- Best assignment (luck-based)

ARTILLERY DURING WORK PERIOD:
- Light bombardment (10-20 shells/min)
- Mix of 77mm and 155mm
- Player must react while working:
  - Hear shell → Identify type → Decide: Ignore vs. Take cover
  - Wrong decision: Death or wasted stamina
  - Right decision: Survive, continue working

Example Event (11:45):
- Player on trench repair duty
- Hear "WHUMMM..." (155mm incoming, 6 seconds warning)
- Drop shovel, sprint to dugout entrance (15m, 3 seconds)
- Dive inside
- WHUMP - Shell lands 12m away
- Trench wall partially collapses, must re-dig
- NPC buried in collapse → Must dig him out (save vs. abandon choice)

Survival Meters (after 4 hours work):
  Hunger: 100 → 76 (work burns calories)
  Thirst: 75 → 55 (sweating despite cold)
  Stamina: 55 → 30 (hard labor)
  Warmth: 58 → 50 (rain continues)
  Hygiene: 80 → 70 (mud, sweat, filth)
  Morale: 57 → 50 (constant stress, near-misses)
  Alertness: 85 → 70 (fatigue setting in)
```

**12:00 - Midday Break**
```
Activity: Rest period (1 hour)

No formal meal (rations exhausted from breakfast)
Player decisions:
- Sleep (Alertness +15, Stamina +10) but vulnerable
- Stay awake (can react to artillery, but more fatigued)
- Eat emergency rations (if hoarded from breakfast)
- Drink water (Thirst 55 → Critical decision: Drink clean water now or save it?)

Social Moment:
- NPC offers cigarette (Morale +3, Warmth +2, but coughing)
- Dialogue: Dark humor about rats, death, food
  - "At least the rats are well-fed here... plenty of meat around"
  - Player can engage (Morale +5) or stay silent (Morale -2, seen as unfriendly)

Critical Decision Point:
  THIRST at 55 (borderline critical)
  Options:
  1. Drink clean water now (Thirst → 80, but canteen empty until resupply tonight)
  2. Ration water (stay at 55, risky but preserves supply)
  3. Drink rainwater from shell crater (Thirst → 70, but 40% disease chance)

Recommended: Drink clean water (resupply tonight)
Risky: Drink crater water (desperate move if no resupply expected)
```

**13:00 - 18:00 - Afternoon Period**
```
Activity: Continued duties + increased artillery

ARTILLERY INTENSIFIES (German afternoon bombardment pattern):
- Heavy bombardment (40-60 shells/min) for 2 hours (14:00-16:00)
- Mix of 77mm, 155mm, occasional 210mm
- Purpose: Preparation for evening patrol/probe attack

Player Experience (14:00-16:00):
- MUST shelter in dugout (cannot work during heavy bombardment)
- Sit in darkness, earth shaking, deafening roar
- Dugout entrance partially collapses (NPCs dig it out)
- One 155mm hits 8m away → Buried under earth for 30 seconds
  - QTE: Rapid button press to dig out before suffocating
  - Success: Escape, Stamina -20, Morale -10, Shell shock risk +20%
  - Failure: Suffocation death OR NPC rescues you (if alive)

Survival Meters During Bombardment:
  Stamina: 30 → 20 (panic, digging out from burial)
  Morale: 50 → 35 (near-death, terror)
  Alertness: 70 → 90 (adrenaline surge)
  Hunger: 76 → 68
  Thirst: 80 → 70 (panic sweating)
  Warmth: 50 (unchanged, inside)
  Hygiene: 70 → 60 (covered in more mud)

Post-Bombardment (16:00):
- Emerge from dugout to assess damage
- Trench section destroyed (20m collapsed)
- 3 NPCs dead (bodies visible)
- 2 NPCs wounded (screaming, bleeding)

MORAL CHOICE:
  Wounded NPC #1: Leg blown off, bleeding out, will die in 10 minutes
    - Option A: Field dressing (save life, costs medical supplies, time)
    - Option B: Ignore (save supplies, morale -15, NPC dies)
    - Option C: Mercy kill (bullet, morale -20, but ends suffering)

  Wounded NPC #2: Arm wound, survivable
    - Option A: Field dressing (stabilize, NPC survives)
    - Option B: Ignore (NPC survives but in pain, morale -10)

Player Resources:
  - 2 field dressings available
  - 1 morphine syrette
  - If both used on NPC #1, none left for self if wounded later

Survival Calculus: Save others vs. preserve resources for self

Post-Decision:
- Resume duties (repair damage from bombardment)
- Work until 18:00 stand-to
```

**18:00 - Evening Stand-To**
```
Activity: Man firing positions, watch for dusk attack (1 hour)

Dusk = High danger period (visibility dropping, German attack likely)

Gameplay:
- All soldiers on fire-step
- Watch through periscope
- Rifle loaded, ready
- Tension high (quiet after bombardment = attack coming?)

NO ATTACK (this time)
- False alarm
- Tension remains (morale cost)

Survival Meters:
  Stamina: 20 → 15 (standing, exhausted)
  Alertness: 90 → 75 (adrenaline wearing off, exhaustion)
  Morale: 35 → 33 (stress, no relief)
```

**19:00 - Evening Rations & Resupply**
```
Activity: Dinner rations, water resupply

Rations:
- 350g bread (Hunger +20)
- 150g canned beef (Hunger +18)
- 200ml wine (Hunger +5, Morale +8, Warmth +5)
- Cheese 100g (if lucky) (Hunger +20)

Water Resupply:
- Canteen refilled (1L clean water)
- Thirst concern resolved (for now)

Survival Meters After Meal:
  Hunger: 68 → 100 (full meal)
  Thirst: 70 → 85 (water ration)
  Warmth: 50 → 58 (wine, inside dugout)
  Morale: 33 → 43 (hot food, wine, social time)
  Alertness: 75 → 70 (food coma setting in)
  Stamina: 15 → 20 (food provides some energy)

Squad Moment:
- NPCs gather, exhausted
- Dialogue: "One day down, six to go"
- Dark humor: "At least we're still alive... for now"
- Letter reading (NPC receives mail, reads aloud, everyone tears up)
- Morale +5 (vicarious connection to home)

Player Decision:
- Sleep now (restore alertness, but night sentry duty coming)
- Stay awake (prepare for night duty, but exhausted)
```

**20:00 - Night Sentry Duty (Rotation)**
```
Activity: Night watch (2-hour shifts, player draws 22:00-00:00 shift)

20:00-22:00: Sleep/Rest
- Alertness: 70 → 85 (2 hours sleep)
- Stamina: 20 → 35 (rest)
- Interrupted by rats, artillery, nightmares

22:00-00:00: Sentry Duty (Player's Shift)

Gameplay:
- Stand at firing position in darkness
- Near-total darkness (no moon, overcast)
- Visibility: 10-20m maximum
- Listen for German patrol activity

Night Conditions:
- Cold (0°C) + rain = misery
- Warmth: 58 → 40 (2 hours in cold)
- Alertness: 85 → 60 (fighting sleep)
- Stamina: 35 → 25 (standing in cold)

Hazards:
- Artillery continues (harassing fire, 1-2 shells/min)
- Must react to shells in near-darkness (harder to judge distance)
- Sniper risk (muzzle flash gives away position)
- German patrol risk (listening minigame)

Random Event (23:15):
- Hear movement in no-man's-land
- Is it: German patrol? Rat? Wounded man crawling back? Wind?
- Decision:
  - Fire flare (reveals area, but alerts Germans)
  - Stay silent (safer, but might miss patrol)
  - Alert officer (pass responsibility, but look uncertain)

If Fire Flare:
- 30 seconds of bright light
- Reveals: Dead bodies, rats, no Germans (this time)
- German artillery responds to flare (10 shells in 2 minutes)
  - Must take cover, sentry duty interrupted
- Morale -3 (wasted flare, caused shelling)

If Stay Silent:
- Nothing happens (this time)
- Uncertainty gnaws at morale (-2)
- But no consequences

End of Shift (00:00):
- Relieved by next sentry
- Stumble back to dugout, collapse on straw mat
- Survival Meters:
  - Alertness: 60 → 30 (exhausted)
  - Stamina: 25 → 15 (borderline collapse)
  - Warmth: 40 → CRITICAL (hypothermia risk)
  - Morale: 40 → 38 (night watch is psychologically draining)
```

**00:00 - 04:00 - Sleep (Interrupted)**
```
Activity: Attempt to sleep (4 hours available)

Sleep Quality: POOR
- Interrupted by artillery every 30-60 minutes
- Rats crawling on face (wake up, swat away)
- Nightmares (dead squadmates, burial alive)
- Cold (shivering prevents deep sleep)

Actual Rest Gained:
- Alertness: 30 → 55 (partial restoration)
- Stamina: 15 → 40 (partial restoration)
- Warmth: 40 → 50 (body heat in sleeping bag, inside dugout)

Random Event (02:30):
- 210mm shell lands 15m from dugout
- Massive explosion, dugout shakes violently
- Wake up in terror, panic
- Morale: 38 → 32 (shell shock risk +10%)
- Alertness: 55 → 85 (terror spike)
- Cannot sleep for rest of night

04:00 - Dawn Approaches:
- Stand-to in 2 hours
- Day 2 begins
- Player Status: Exhausted, hungry, stressed
- 6 more days of this...
```

**END OF DAY 1 - Survival Meter Summary:**
```
Starting Meters (06:00):          End of Day 1 (04:00):
  Hunger: 100                       Hunger: 85
  Thirst: 75                        Thirst: 75
  Stamina: 100                      Stamina: 40
  Warmth: 70                        Warmth: 50
  Hygiene: 85                       Hygiene: 60
  Morale: 60                        Morale: 32 (↓28 - major drop)
  Alertness: 70                     Alertness: 85 (terror spike)

Survival Status: STABLE but degrading
Critical Concerns:
  - Morale dropping fast (shell shock risk increasing)
  - Stamina/Alertness unsustainable (need real sleep)
  - Warmth borderline (hypothermia risk if continues)
  - 6 more days of this = death spiral risk

Player Strategy for Day 2:
  - MUST sleep during day (prioritize alertness/stamina)
  - Conserve energy (avoid heavy labor if possible)
  - Manage morale (engage with squad, humor, letters)
  - Ration food carefully (only 2 meals/day)
```

---

##### DAYS 2-6 - GRINDING SURVIVAL

**Compressed Gameplay (Time-Accelerated Sections + Critical Moments)**

**Day 2-3**: Routine sets in
- Same cycle: Stand-to → Work → Bombardment → Stand-to → Night watch → Sleep
- Survival meters slowly degrade
- Player learns rhythm, becomes more efficient
- NPC deaths (1-2 per day, replacements arrive)
- Small victories: Hot soup (+10 morale), mail from home (+15 morale), sunny afternoon (+10 warmth)

**Day 4**: CRISIS EVENT
- Heavy drumfire bombardment (4 hours, 100+ shells/min)
- Dugout partially collapses, trapped for 1 hour
- 6 NPCs killed, player wounded (shrapnel in arm, -15 health, -20 stamina)
- Field dressing used (1/2 remaining)
- Morale drops to 20 (shell shock threshold)
- Player must resist shell shock breakdown (QTE or skill check)
  - Success: Continue (morale stabilizes at 20)
  - Failure: Breakdown → Evacuated → Mission failure (OR skip to Day 7 relief)

**Day 5-6**: Endurance
- Survival meters critically low
- Hunger: 60 (rations insufficient)
- Stamina: 30 (chronic fatigue)
- Morale: 20-25 (holding on by thread)
- Alertness: 40 (sleep-deprived, micro-sleeps)
- Warmth: 45 (trench foot risk)
- Hygiene: 40 (lice infestation confirmed, -10 morale)

**Player Experience**: Numb, automatic, dissociated
- Actions become mechanical (player mastering systems through repetition)
- Less panic, more resignation
- "Just survive until relief"

---

##### DAY 7 - MARCH 21 (RELIEF)

**06:00 - Final Stand-To**
```
Activity: Last morning watch

Player Status: Barely functional
- All meters in yellow/red
- But alive
- 6 NPCs dead (of 12 starting), 6 wounded, 4 survivors (player + 3 NPCs)

Emotional State: Relief coming today... please don't die now
```

**10:00 - Relief Column Arrives**
```
Activity: New unit arrives, player's squad stands down

Gameplay:
- Fresh troops march into trench (clean uniforms, high morale)
- Player's squad: Filthy, exhausted, hollow-eyed
- Visual contrast (cutscene moment)
- New soldiers look at veterans with fear/respect

Dialogue:
- Fresh soldier: "Is it bad?"
- Veteran NPC: "You'll see."
- Player given option to respond:
  - "Good luck." (neutral)
  - "Just keep your head down." (helpful, +5 morale)
  - Silence (dissociated, realistic)

Handoff:
- Brief sector (German positions, recent activity)
- Transfer supplies
- Leave dugout (last look at the place that kept you alive)
```

**11:00 - March to Rest Area**
```
Activity: 2-hour march to Dugny (rest village)

Gameplay:
- Walk through communication trenches
- Pass resupply columns coming forward
- Emerge from trenches into open countryside (psychological relief)
- March 5km to village

Survival Meters During March:
  Stamina: 30 → 10 (exhausting march despite relief)
  Morale: 25 → 45 (relief + distance from front + survival joy)
  Alertness: 40 → 50 (walking, fresh air)

Arrival at Dugny (13:00):
- Billet assigned (barn, straw beds)
- Hot meal (Hunger → 100, Morale +10)
- Wash station (Hygiene 40 → 70, Morale +5)
- Sleep (finally, no artillery)

MISSION COMPLETE
Victory: Survived 7 days on front line
```

**MISSION DEBRIEFING SCREEN:**
```
================================
MISSION COMPLETE: Front Line Rotation
Les Éparges Sector, March 15-21, 1916
================================

SURVIVAL: 7 / 7 days
STATUS: Wounded (light)

SQUAD STATUS:
  Started with: 12 soldiers
  Killed: 6
  Wounded: 2 (evacuated)
  Survived: 4 (including player)
  Survival Rate: 33%

STATISTICS:
  Shells Survived: 18,247
  Closest Call: 8m (155mm, Day 4)
  Artillery Kills Witnessed: 6
  Times Buried Alive: 1
  Gas Attacks: 0
  Combat Engagements: 0
  Rations Consumed: 12
  Water Consumed: 8L
  Sleep: 18 hours (of 168 possible)

METERS AT END:
  Hunger: 100 ✅
  Thirst: 90 ✅
  Stamina: 50 ⚠️
  Warmth: 70 ✅
  Hygiene: 70 ⚠️
  Morale: 60 ⚠️ (recovered during march)
  Alertness: 85 ✅

NEXT ROTATION: March 29 (8 days rest)

PROGRESS: Day 24 / 303
         (279 days remaining)

"You survived. That's all that matters."
================================
```

#### EXAMPLE 2: SUPPORT-LINE ROTATION MISSION

**MISSION**: "Bras Support Sector - April Rotation"
- **Date**: April 10-16, 1916 (7 days)
- **Location**: Support trench, 400m behind front line
- **Player Status**: Experienced (7 weeks survival)
- **Weather**: Spring rain, improving conditions
- **Artillery Intensity**: Moderate (still dangerous, but less than front line)
- **Gameplay Duration**: 2-3 hours real-time
- **Victory Condition**: Survive until April 16 rotation to rest area

---

##### SUPPORT LINE - KEY DIFFERENCES FROM FRONT LINE

**Reduced Direct Combat:**
- No German infantry attacks
- No direct line-of-sight to enemy
- Lower small arms fire risk

**Still Within Artillery Range:**
- 40% of artillery casualties occur in support line
- Larger shells reach support trenches (155mm, 210mm common)
- Less frequent but still deadly

**Different Duties:**
- Resupply missions to front line
- Burial details (recover dead from no-man's-land)
- Trench construction/repair
- Receiving wounded from front
- Reserve for counterattacks

**Psychological Difference:**
- Less immediate terror (not under constant observation)
- But: Guilt (safer while comrades die 400m ahead)
- Anticipation: Will be sent forward if crisis occurs
- Sounds of battle (hearing friends die, unable to help)

---

##### TYPICAL DAY - APRIL 12 (Day 3 of Support Rotation)

**06:00 - Morning Stand-To (Reduced)**
```
Activity: 30 minutes (vs. 1 hour on front line)

Less tension:
- German attacks target front line first
- Support line is fallback position
- But still must be ready

Artillery:
- Harassing fire (1-2 shells every 5 minutes)
- Random long-range 155mm
- One lands 60m away → No reaction needed → Continue watch
```

**07:00 - Resupply Mission Assignment**
```
Mission: Carry ammunition to front line
Duration: 3 hours (multiple trips)
Danger: HIGH (communication trenches are targeted)

Loadout:
- 2 crates ammunition (20kg each)
- Personal weapon + gear (10kg)
- Total: 50kg load

Route:
- 400m through communication trench
- Narrow (1m wide), muddy, winding
- 6 trips required
- Total distance: 4.8km carrying heavy loads

Gameplay:
- Navigation challenge (trench maze, easy to get lost)
- Stamina management (cannot collapse with heavy load)
- Artillery timing (sprint between bombardments)
- NPC porter squad (4 soldiers, must stay together)

HAZARD - Communication Trench Artillery:
- Germans target supply routes
- 155mm shell lands in trench (Trip 3, 09:15)
  - 15m ahead of player
  - NPC porter killed instantly
  - Trench blocked by debris
  - Must find alternate route OR dig through
  - Morale -8 (witnessed death, knew the soldier)

Survival Meters (after 3 hours):
  Stamina: 85 → 30 (extreme exertion)
  Hunger: 90 → 65 (heavy labor)
  Thirst: 80 → 55 (sweating)
  Morale: 70 → 62 (death witnessed, exhaustion)
  Warmth: 65 → 60 (spring weather better)
  Hygiene: 75 → 65 (mud, sweat)

Decision Point (09:30):
- Continue mission (3 more trips) at Stamina 30
  - Risk: Collapse, become burden
- OR: Rest 30 minutes (restore Stamina +15, but delay delivery)
  - Risk: Front line runs low on ammo, officer angry
```

**10:00 - Burial Detail Assignment**
```
Mission: Recover bodies from no-man's-land (night mission prep)
Duration: 2 hours (planning + equipment prep)

Historical Basis:
- Bodies left in no-man's-land for days/weeks
- Disease risk, psychological toll on soldiers
- Burial parties went out at night (suicide mission status)

Preparation (Daytime):
- Scout locations (identify bodies through periscope)
- Plan routes (avoid German sightlines)
- Prepare stretchers, identification tags
- Write "goodbye letters" (common practice before burial missions)

Gameplay:
- Periscope minigame (identify 5 body locations)
- Map routes avoiding German MG positions
- Psychological preparation (Morale -10 anticipating mission)

NPC Dialogue:
- "I knew two of them... Beaumont and Laurent. Been out there 5 days."
- "Hope they don't shoot flares while we're out there."
- "If I don't come back, give this to my wife." (letter exchange)

Player Choice:
- Volunteer for burial party (duty, but dangerous)
- OR: Request different assignment (safer, but Morale -5, seen as coward)

If Volunteer: Night mission begins at 22:00 (see below)
```

**12:00 - Midday Meal & Rest**
```
Activity: Lunch + recovery time

Meal (Support line = better rations):
- Hot soup (Hunger +30, Warmth +10, Morale +10)
- Fresh bread (Hunger +20)
- Wine ration (Morale +5)
- Occasional luxury: Chocolate, cheese (if supply good)

Support Line Advantages:
- Hot food possible (field kitchens 200m behind)
- More rest opportunities
- Slightly better shelter (deeper dugouts)
- Mail delivery more reliable

Mail Call:
- Player receives letter from home (random event, 30% chance)
- IF received: Read letter (cutscene/text)
  - Content: Family news, longing, worry
  - Morale +20 (connection to home)
  - Hunger for normalcy increases
- IF NOT received: Morale -5 (disappointment, fear family forgotten)

Social Time:
- NPCs play cards, tell stories
- Player can join (Morale +5, learn about squadmates)
- OR: Sleep (Stamina +20, Alertness +15)
- OR: Write letter home (Morale +10, emotional release)

Survival Meters After Rest:
  Stamina: 30 → 50 (rest + food)
  Hunger: 65 → 100 (full meal)
  Thirst: 55 → 80 (water ration)
  Morale: 52 → 67 (food, mail, rest)
  Alertness: 70 → 80 (rest)
```

**14:00 - Wounded Receiving Station Duty**
```
Mission: Help medical officer with wounded from front line
Duration: 4 hours
Danger: LOW (physical), HIGH (psychological)

Location: Aid station (dugout converted to field hospital)
Role: Stretcher bearer, triage assistant, comfort provider

Gameplay:
- Carry wounded from arrival point to aid station (50m, 10-15 trips)
- Hold soldier down during surgery (no anesthetic available)
- Apply field dressings under medical officer supervision
- Comfort dying soldiers (dialogue choices)

Wounded Arrival Pattern:
- 14:00-15:00: Light (2-3 wounded)
- 15:00-16:00: HEAVY BOMBARDMENT on front line
- 16:00-18:00: SURGE (15-20 wounded arrive)

Triage System (historically accurate):
1. **Light wounds** - Field dressing, return to duty
2. **Serious wounds** - Stabilize, evacuate to rear hospital
3. **Mortal wounds** - Morphine, comfort, wait for death
4. **Dead on arrival** - Move to morgue area

MORAL CHOICE EVENT (16:30):

Wounded Soldier: Sergeant Moreau (NPC from front line, player knows him)
- Injury: Abdominal wound, intestines exposed
- Prognosis: Will die in 1-3 hours, no treatment possible
- Pain: Extreme, screaming
- Request: "Kill me... please... I can't..."

Options:
A. Give morphine (2 syrettes, entire supply)
   - Effect: Eases pain, soldier dies peacefully in 30 minutes
   - Consequence: No morphine left for other wounded (4 arrive later needing it)
   - Morale: -5 (hard choice) → +10 when soldier dies peacefully

B. Give 1 syrette (half dose)
   - Effect: Reduces pain, soldier still suffers, dies in 1 hour
   - Consequence: 1 syrette left for others
   - Morale: -15 (guilt, soldier suffers longer)

C. Refuse (save morphine for soldiers who can survive)
   - Effect: Soldier screams for 2 hours, dies in agony
   - Consequence: 2 syrettes available for survivable wounded (saves 2 lives)
   - Morale: -25 (trauma, screams haunt player)
   - NPC reaction: Some soldiers understand (utilitarian), others horrified

D. Mercy kill (bullet, if player has weapon)
   - Effect: Instant death, ends suffering
   - Consequence: Medical officer reports player, disciplinary hearing (cutscene)
   - Morale: -20 (killed friend) → -10 (court-martial stress)
   - Outcome: Reprimand (not executed, but on record)

Historical Accuracy Note:
- Mercy killings happened but were officially illegal
- Morphine rationing was real (agonizing choices)
- ~30% of wounded died before reaching hospital

Post-Mission Psychological Impact:
- If gave morphine: Haunted by screams of OTHER wounded who needed it
- If refused: Haunted by Moreau's screams (audio echoes for rest of mission)
- If mercy killed: Moral burden + legal consequences

Survival Meters After Duty:
  Morale: 67 → 40-50 (depending on choice)
  Stamina: 50 → 35 (carrying wounded, stress)
  Hygiene: 65 → 50 (blood, mud, infection risk)
  Alertness: 80 → 90 (adrenaline, trauma)

Shell Shock Risk: +15% (witnessing suffering)
```

**18:00 - Evening Meal & Brief Rest**
```
Evening rations (normal)
Rest: 2 hours before night burial mission

Player Status:
- Exhausted (Stamina 35)
- Traumatized (Morale 40-50)
- Dreading night mission (burial detail)

Preparation:
- Blackened face (reduce visibility)
- No metal items (silent movement)
- Stretchers prepared
- Routes memorized

NPCs on burial party (4 soldiers + player):
- All volunteers (duty-driven)
- All terrified (realistic)
- Dark humor: "If I die, leave me out there. Save yourselves the trip tomorrow."
```

**22:00 - Night Burial Detail (NO-MAN'S-LAND MISSION)**
```
Mission: Recover 5 bodies from no-man's-land
Duration: 2-3 hours (if survive)
Danger: EXTREME

Conditions:
- New moon (near-total darkness)
- 150m into no-man's-land
- German positions 200m away
- Random artillery continues (1-2 shells/min)
- Flare risk (if Germans suspicious)

Gameplay:
- STEALTH MISSION (completely different from normal gameplay)
- Crawl (cannot walk, too visible)
- Movement speed: 5m per minute (realistically slow)
- Navigate by landmarks (shell craters, bodies, wire)
- Absolute silence (no talking, hand signals only)

Route:
- Exit trench via sap (forward tunnel)
- Crawl 80m to first body
- Identify (check dog tags)
- Strap to stretcher
- Drag back (2 soldiers per stretcher)
- Repeat 5 times

HAZARDS:

1. Artillery (random, unavoidable)
- 77mm lands 40m away (22:45)
- Drop prone, hug earth, pray
- Explosion lights up no-man's-land for 2 seconds
- German MG fires at movement (suppression burst)
- Morale -10

2. German Flare (23:15)
- German suspicious of movement
- Fires illumination flare
- 30 seconds of daylight brightness
- Decision: FREEZE (don't move) or RUN (back to trench)

  If FREEZE:
  - Hold perfectly still (QTE: don't press any buttons)
  - Difficulty: Breathing, shaking, terror
  - Success: Germans don't spot (23% chance they shoot anyway)
  - Failure: Movement detected → MG fire → Death/wounding

  If RUN:
  - Sprint 80m to trench (10 seconds)
  - German MG fires (high hit chance)
  - Leave bodies behind (mission failure)
  - 40% survival chance

3. Corpse Horror
- First body: 5 days old, decomposing, rats feeding
- Player must touch, identify, strap to stretcher
- Morale -5 per body (total -25)
- Vomiting chance: 40% (if low Hygiene)
- If vomit: Noise → German suspicion → Flare risk +30%

4. Wounded Survivor Found
- Body #3 is ALIVE (barely)
- German soldier, gut shot, dying
- Conscious, staring at player
- Gurgling in German: "Hilfe..." (Help)

CHOICE:
A. Mercy kill (knife, silent)
   - Morale -15 (killed helpless enemy)
   - But prevents him alerting Germans

B. Give water, leave alive
   - Morale -10 (guilt, leaving to die)
   - Risk: He might call out (20% chance, alerts Germans)

C. Ignore completely
   - Morale -5
   - Same risk (20% alert chance)

D. Attempt rescue (take him back)
   - Morale +10 (humanity) → +20 if succeeds
   - Risk: Extra weight, slower, higher detection chance
   - Medical officer will treat enemy (historical practice)

Mission Outcome (if successful):

Return to trench (01:00):
- 5 bodies recovered (4 French, 1 German if rescued)
- All identified, prepared for burial
- Burial at dawn (05:00)

Survival Meters After Mission:
  Stamina: 35 → 10 (3 hours crawling, dragging)
  Morale: 50 → 30 (corpse horror, terror, exhaustion)
  Alertness: 90 → 95 (adrenaline sustained)
  Hygiene: 50 → 35 (decomposition fluids, mud, vomit)
  Warmth: 60 → 45 (3 hours prone in mud)

Shell Shock Risk: +25% (cumulative trauma)
```

**01:00-06:00 - Post-Mission Recovery**
```
Activity: Decontamination + Sleep (attempt)

Decontamination:
- Wash hands, face (Hygiene 35 → 45)
- Change clothes if available (Hygiene +10)
- Drink wine ration (Morale +5, Warmth +5)
- Medical check (infection risk from corpse contact)

Sleep:
- 4 hours available
- Quality: TERRIBLE (nightmares guaranteed)
- Flashbacks: Crawling, flares, corpses, rats
- Wake up multiple times
- Actual rest: Minimal (Alertness 95 → 75, Stamina 10 → 25)

06:00 - Burial Service:
- 5 bodies buried in cemetery behind lines
- Officer reads names
- Moment of silence
- Morale +5 (duty fulfilled, closure)
- But: Reminder of mortality (Morale -5)
```

**SUPPORT LINE DAYS 4-7:**
```
Continued rotation duties:
- More resupply missions
- Trench repair/construction
- Occasional shelling (1-2 casualties)
- Anticipation of rest rotation (morale improves)

Day 7 (April 16):
- Rotation to rest area
- March 10km to Dugny
- Hot bath, clean clothes, real sleep
- Survival Status: Degraded but stable

Mission Complete: 7 days support line survived
```

**MISSION DEBRIEFING:**
```
================================
MISSION COMPLETE: Support Line Rotation
Bras Sector, April 10-16, 1916
================================

SURVIVAL: 7 / 7 days
STATUS: Uninjured (physically)

SQUAD STATUS:
  Casualties: 2 killed (artillery), 1 wounded
  Survival Rate: 75% (better than front line)

STATISTICS:
  Shells Survived: 4,127 (fewer than front line)
  Resupply Missions: 6
  Burial Details: 1 (5 bodies recovered)
  Wounded Processed: 23
  Moral Choices: 3 (morphine, German soldier, burial risks)

METERS AT END:
  Hunger: 90 ✅
  Thirst: 85 ✅
  Stamina: 40 ⚠️
  Warmth: 65 ✅
  Hygiene: 55 ⚠️
  Morale: 45 ⚠️ (trauma accumulated)
  Alertness: 60 ⚠️

PSYCHOLOGICAL STATUS:
  Shell Shock Risk: 40% (cumulative)
  Trauma Events: 4 (burial detail, wounded station, deaths)

NEXT ROTATION: Rest area (7 days)

PROGRESS: Day 52 / 303
         (251 days remaining)

"The support line isn't safe. It's just less dangerous."
================================
```

---

#### EXAMPLE 3: REST AREA EXPERIENCE

**MISSION**: "Dugny Rest Period - May Rotation"
- **Date**: May 1-7, 1916 (7 days)
- **Location**: Dugny village, 8km behind front
- **Player Status**: Veteran (10 weeks survival)
- **Weather**: Spring, warm, sunny (contrast to trenches)
- **Artillery Intensity**: Minimal (occasional long-range)
- **Gameplay Duration**: 1-2 hours real-time (shorter, story-focused)
- **Victory Condition**: Recover meters, prepare mentally for return

---

##### REST AREA - PSYCHOLOGICAL PARADOX

**Relief vs. Dread:**
- First 2 days: Relief (joy, sleep, food, safety)
- Days 3-5: Normalcy (almost forget the war)
- Days 6-7: Dread (return to front approaching)

**Survival Meter Recovery:**
- Hunger/Thirst: Full restoration (hot meals, clean water)
- Stamina/Alertness: Full restoration (real sleep, no interruptions)
- Warmth: Full restoration (beds, shelter, spring weather)
- Hygiene: Significant improvement (baths, clean clothes, delousing)
- Morale: Partial recovery (50 → 70) but anticipatory anxiety prevents full recovery

**Different Gameplay:**
- No combat
- No survival urgency
- Focus on: Relationships, letters, recreation, mental preparation
- Philosophical moments (questioning war, purpose, survival guilt)

---

##### DAY 1 - MAY 1 (ARRIVAL)

**Morning - Arrival at Dugny (10:00)**
```
March from front line: 8km, 3 hours
Emerge from trenches into countryside

Psychological Shift:
- Colors (green grass, flowers - players haven't seen these in weeks)
- Silence (no artillery - uncanny, almost frightening)
- Civilians (old men, women, children - reminder of normal world)
- Birds singing (auditory contrast to explosions)

Arrival:
- Village square, intact buildings
- Billet assignment: Barn with real beds (straw mattresses)
- Squad of 8 (down from 12 at last rest period)

First Actions:
- Drop equipment (relief)
- Issued clean uniforms
- Directed to bath house
```

**Midday - Decontamination (12:00)**
```
Activity: Hot bath, delousing, medical check

Bath House:
- First hot water in 7 weeks
- Players can see character's body: Scars, wounds, lice bites, filth
- Visual reminder of degradation
- Hygiene: 45 → 85
- Morale: +15 (cleanliness, human dignity restored)

Delousing:
- Clothes burned (too infested)
- New uniform issued
- Anti-lice powder applied
- Relief from constant itching

Medical Check:
- Minor wounds treated (field dressings removed, proper care)
- Trench foot examined (stage 1 - treatable)
- Weight recorded: Player has lost 8kg since February (visual change)
- Doctor: "You're holding up better than most. Try to eat well this week."

Post-Bath:
- Clean, warm, human again
- But: Body aches now noticeable (adrenaline gone, injuries felt)
- Stamina: Still low (30) - will take days to recover
```

**Afternoon - Hot Meal & Sleep (14:00)**
```
Meal (proper restaurant, civilians serving):
- Hot soup, roasted chicken, potatoes, bread, wine, coffee, dessert
- Unlimited portions (first time in months)
- NPCs eat ravenously, then vomit (stomachs can't handle rich food)
- Hunger: → 100
- Morale: +10 (civilized meal, seated at table)

Post-Meal:
- Return to billet
- Collapse on bed (real mattress, no rats, no explosions)
- Sleep for 14 hours straight (14:30 - 04:30 next morning)
- First uninterrupted sleep in 10 weeks
- Nightmares: Yes, but can sleep through them
- Stamina: 30 → 80
- Alertness: 60 → 95
```

##### DAYS 2-3 - RECOVERY & RECREATION

**Day 2 - May 2**
```
Activities:
- Sleep late (08:00 wake-up, luxury)
- Three hot meals (Hunger maintained at 100)
- Light duties: Equipment repair, uniform maintenance
- Free time: 6 hours

Recreation Options (Player Choice):

1. Write Letters Home
   - Duration: 1-2 hours
   - Effect: Morale +10, emotional release
   - Content: Player writes to family (dialogue options)
     - Honest (tell them about horrors) - Cathartic but worries family
     - Reassuring (lie, say you're safe) - Protective but isolating
     - Vague (describe landscape, avoid war) - Common practice
   - Historical: Letters were censored (officer reads before sending)

2. Play Cards with Squad
   - Duration: 2 hours
   - Effect: Morale +5, camaraderie +10
   - Dialogue: Dark humor, shared trauma, bonding
   - Learn NPC backstories:
     - Corporal Duval: Wife and 2 children in Paris
     - Private Moreau: Was a teacher before war
     - Private Rousseau: 19 years old, joined February

3. Drink at Estaminet (Village Café)
   - Duration: 3 hours
   - Effect: Morale +15 (temporary), Alertness -20, Stamina -10 (hangover)
   - Cost: 5 francs (if player has money)
   - NPCs get drunk, sing songs, forget war briefly
   - Risk: Drunk arguments, fights, discipline issues

4. Church Service (Sunday, May 2)
   - Duration: 1 hour
   - Effect: Morale +10 (if player religious), philosophical reflection
   - Chaplain sermon: "God is with you, even in darkness"
   - Player reaction:
     - Believe (comfort, purpose)
     - Doubt (anger at God for allowing this)
     - Indifferent (hollow ritual)

5. Walk Countryside (Alone)
   - Duration: 2 hours
   - Effect: Morale +5, reflection, solitude
   - Gameplay: Walk through fields, woods (peaceful)
   - See: Normal world (farmers working, animals, spring flowers)
   - Philosophical moment:
     - "This still exists. While we die 8km away, life continues."
     - Bittersweet realization

6. Sleep More
   - Duration: 4-6 hours
   - Effect: Stamina → 100, Alertness → 100
   - Catch up on 10 weeks of sleep debt

Player Strategy:
- Balance: Social (Morale) vs. Physical (Stamina) vs. Emotional (Letters) recovery
```

**Day 3 - May 3**
```
Continued recovery
All meters improving:
  Hunger: 100 (maintained)
  Thirst: 100
  Stamina: 100 (fully recovered)
  Alertness: 100
  Warmth: 100
  Hygiene: 90 (maintained with daily washing)
  Morale: 70 (improved from 45)

Psychological State: Calm, almost normal
- Can think clearly
- Nightmares lessening
- Laughter feels possible again

NPC Interaction:
- Squad bonding deepens
- Shared photos of families
- Promises: "We'll survive this. We'll go home together."
- Player attachment to NPCs increases (makes future deaths harder)
```

##### DAYS 4-5 - NORMALITY & TEMPTATION

**Day 4 - May 4 - The Trap of Normalcy**
```
Gameplay: Normal life simulation
- No urgency, no danger
- Can explore village, talk to civilians, attend events
- Meters all at 100%

Psychological Shift:
- War feels distant (can almost forget it)
- But: Guilt (knowing comrades are dying at front while you rest)
- Temptation: What if I just... didn't go back?

Desertion Temptation Event (Optional, Player Discovery):
- Player finds NPC (Private Beaumont, 19) crying behind barn
- Dialogue:
  - Beaumont: "I can't go back. I can't. I'll die there."
  - "My mother's alone. If I die, she has no one."
  - "I could run. To Spain. Or just... disappear."

MORAL CHOICE:
A. Encourage Return (Duty)
   - "We all go back. Together. We'll watch out for each other."
   - Effect: Beaumont returns, Morale +5 (camaraderie)
   - Risk: Beaumont dies in next rotation (player guilt)

B. Support Desertion (Humanity)
   - "I won't tell anyone. Run. Save yourself."
   - Effect: Beaumont deserts, removed from squad
   - Player Morale -10 (complicit in desertion)
   - If caught: Player faces court-martial as accomplice (10% chance)

C. Report to Officer (Discipline)
   - Beaumont arrested, faces execution (desertion = death penalty)
   - Effect: Morale -20 (betrayed comrade)
   - NPC reaction: Squad hates player (Morale -15 more)
   - Historical: Executions were rare but did occur

D. Ignore (Avoid Responsibility)
   - Say nothing, walk away
   - Beaumont makes own choice (random outcome)
   - Morale -5 (cowardice, avoided hard choice)

Player's Own Temptation:
- After Beaumont event, player can consider desertion
- Game presents option (first time):
  - "You could run. The war would continue without you. Is survival enough?"
  - If player chooses: Game Over screen (not a failure, but an ending)
    - "You deserted. You survived the war. But could you live with yourself?"
    - No judgment, just reality of choice
```

**Day 5 - May 5 - Entertainment**
```
Event: Mobile theater troupe performs for troops

Performance:
- Comedy sketches (soldiers laugh, first time in months)
- Music (sentimental songs about home)
- Dancing girls (reminder of normal world, romance)

Player Experience:
- 2 hours of escape
- Morale +15
- But: Underlying sadness (this is temporary)

Post-Performance:
- Return to billet
- Quiet moment (cutscene):
  - NPCs discuss performance
  - "Almost forgot where we are."
  - "In two days, we go back."
  - Silence
  - "Let's not talk about it tonight."
```

##### DAYS 6-7 - ANTICIPATORY DREAD

**Day 6 - May 6 - The Shadow Returns**
```
Atmosphere Change:
- Joy of rest fading
- Return approaching (May 8)
- Meters still at 100%, but Morale begins dropping

Morale: 70 → 60 (anticipatory dread)
- Automatic decrease (unavoidable psychological reality)
- Sleep quality declines (nightmares return)
- Appetite decreases (anxiety)

Squad Behavior:
- Less laughter
- Longer silences
- More drinking
- Riskier behavior (fights, gambling)

Preparation Begins:
- Check equipment
- Clean weapons
- Pack gear
- Write "last letters" (to be mailed if killed)

Final Letters Event:
- Officer collects "death letters"
- Player can write one:
  - "If you're reading this, I'm dead. I want you to know..."
  - Morale -10 (confronting mortality)
  - But: Provides closure if player dies (cutscene plays letter reading)

Day 7 - May 7 - Last Day
```
Morning:
- Final hot meal
- Final bath
- Final sleep in real bed
- All meters at maximum (physically ready)

Afternoon:
- Pack gear (30kg load again)
- Issued rations (7 days)
- Ammunition resupply
- Gas mask check

Morale: 60 → 50 (dread intensifies)

Goodbye Event:
- Civilians wave goodbye
- Children give flowers to soldiers (French tradition)
- Old woman: "Thank you for protecting us. Come back alive."
- Morale -5 (burden of expectation) OR +5 (purpose renewed)

Final Free Time (2 hours):
- Player chooses final activity:
  - Sleep (futile, can't sleep)
  - Pray (if religious)
  - Write letter
  - Talk to squad
  - Walk alone (prepare mentally)

Evening (18:00) - March to Front Begins:
- 8km march through dusk
- Silence (no singing, no talking)
- Hear artillery in distance (growing louder)
- Smell smoke (front line approaching)
- Psychological shift: From human to soldier again

Arrival at Support Trench (21:00):
- Relief ends
- Next rotation begins
- Morale: 50 → 45 (return shock)

MISSION COMPLETE: Rest period ended
```

**REST AREA DEBRIEFING:**
```
================================
MISSION COMPLETE: Rest Period
Dugny Village, May 1-7, 1916
================================

SURVIVAL: 7 / 7 days (no combat danger)

RECOVERY STATUS:
  All survival meters restored to 100%
  Body weight: +2kg (still -6kg from campaign start)
  Wounds: Healed (minor)
  Infections: Treated

PSYCHOLOGICAL RECOVERY:
  Morale: 45 → 70 → 50 (brief recovery, then dread)
  Shell Shock Risk: 40% → 30% (reduced but not eliminated)
  Nightmares: Persistent (PTSD symptoms)

ACTIVITIES:
  Letters Written: 3
  Church Services Attended: 1
  Recreation Hours: 18
  Sleep Hours: 78 (of 168 available)
  Meals: 21 (hot, full rations)

SQUAD BONDING:
  Camaraderie: +25% (deeper friendships)
  Attachment: HIGH (makes future losses more painful)
  Squad Cohesion: 85% (strong unit)

MORAL CHOICES:
  Desertion Event: [Player's choice logged]

NEXT ROTATION: Front Line - Fleury Sector (May 8-14)

PROGRESS: Day 72 / 303
         (231 days remaining)

"Seven days of life. Now back to death."
================================
```

#### EXAMPLE 4: SPECIAL EVENT MISSION - FORT VAUX SIEGE

**MISSION**: "Fort Vaux - The Seven Days"
- **Date**: June 1-7, 1916 (7 days)
- **Location**: Fort Vaux (49.1989°N, 5.4494°E, 390m elevation)
- **Player Status**: Veteran (15 weeks survival)
- **Historical Context**: German attempt to capture last major fort before Verdun
- **Siege Intensity**: EXTREME - Most intense mission in campaign
- **Gameplay Duration**: 5-7 hours real-time (longest mission)
- **Victory Condition**: Survive until June 7 surrender
- **Historical Outcome**: Fort falls, but garrison honored for heroic defense

---

##### FORT VAUX - MISSION STRUCTURE DIFFERENCES

**This is NOT a normal rotation:**
- No relief coming
- No rest periods
- No resupply possible
- Completely surrounded, cut off
- Water runs out
- Ammunition depletes
- Victory = Survival until honorable surrender (not death)

**Multi-Stage Mission:**
1. **Stage 1** (Jun 1-2): Initial siege, fort intact
2. **Stage 2** (Jun 3-4): Fort breached, tunnel fighting
3. **Stage 3** (Jun 5-6): Water crisis, final defense
4. **Stage 4** (Jun 7): Surrender negotiations

**Historical Foundation:**
- Fort Vaux garrison: 600 soldiers
- Commander: Major Sylvain-Eugène Raynal (real person, BLIND from earlier injury)
- Casualties: 100 killed, 500 wounded/captured
- Germans: 2,700 attackers, 900 casualties
- Duration: 7 days (Jun 1-7, 1916)
- Result: Fort falls, but French inflicted heavy casualties
- Outcome: Raynal honored by both sides, paroled due to blindness

**Player Role:**
- Corporal (name chosen by player OR pre-set character)
- Part of garrison (not Raynal, who is NPC commander)
- Witness to historical events
- Participate in tunnel fighting, water rationing, surrender

---

##### STAGE 1: INITIAL SIEGE (June 1-2)

**JUNE 1 - 04:00: PRE-DAWN BOMBARDMENT**

```
Historical Context:
- German offensive to capture Vaux before summer
- 8-hour bombardment (04:00-12:00)
- Then infantry assault

Player Location: Inside fort casemate (thick concrete walls)
Garrison Status: 600 soldiers, supplies adequate, morale moderate

Gameplay - Bombardment Survival:
- Heaviest bombardment player has experienced (100+ shells/min)
- Mix of 210mm, 305mm, 420mm shells
- Fort shakes continuously (screen shake, audio overwhelm)
- Concrete dust fills air (visibility reduced)
- Lighting flickers (electrical damage)
- NPCs terrified (even veterans are afraid)

Duration: 8 hours compressed into 20-30 minutes gameplay
- Time acceleration during bombardment
- Punctuated by "critical moments" (real-time events)

Critical Moment 1 (06:30):
- 420mm shell hits fort roof (direct hit)
  - Massive explosion (screen white, deafening)
  - Casemate ceiling cracks (concrete chunks fall)
  - 3 soldiers killed instantly (debris crush)
  - Player must help dig out wounded
  - Morale -15 (terror, casualties)

Critical Moment 2 (09:00):
- Gas shells mixed into bombardment
  - Phosgene gas seeps into fort through cracks
  - "GAZ! GAZ! GAZ!" alarm
  - All must don gas masks (5-second equip window)
  - Player vision reduced (mask FOV)
  - Cannot eat/drink for next 2 hours (gas persists)
  - Thirst begins accumulating

Survival Meters During Bombardment:
  Hunger: 90 → 80 (stress, cannot eat during gas)
  Thirst: 85 → 65 (gas mask, panic sweating, cannot drink)
  Stamina: 80 → 60 (stress, digging out wounded)
  Morale: 70 → 50 (bombardment terror, casualties)
  Alertness: 90 → 95 (sustained adrenaline)
  Warmth: 75 (underground, stable)
  Hygiene: 80 → 70 (concrete dust, gas)

Psychological: "We're going to die here. This fort will be our tomb."
```

**12:00: GERMAN INFANTRY ASSAULT BEGINS**

```
Bombardment lifts, infantry attack starts

Commander Raynal (NPC):
- "To your positions! They're coming!"
- "Remember: Fort Vaux will NOT fall while we live!"
- Morale +10 (inspiring leadership)

Player Assignment: Machine gun position, north casemate
- Hotchkiss M1914 machine gun
- 2,000 rounds ammunition
- Firing port (narrow slit, limited view)
- NPC assistant (ammo feeder)

Assault Wave 1 (12:00-13:00):
- 400 German infantry advance across shell-cratered ground
- 200m approach distance
- Player must engage targets at 100-200m

Gameplay:
- MG operation (hold trigger, manage overheating)
- Target prioritization (closest threats first)
- Barrel changes (overheat every 200 rounds, 10-second swap)
- Ammo management (NPC feeds belts, but limited supply)

Combat Experience:
- Effective range: 100m (most kills occur here)
- First burst: 5-10 Germans fall (player's kills)
- Germans take cover in craters (harder to hit)
- Grenades thrown at fort (explosions near firing port)
- German flammenwerfer (flamethrower) team approaches (high priority target)

Moral Weight:
- Player kills 20-30 Germans in 1 hour
- Morale impact: -20 (mass killing, even in defense, is traumatic)
- NPC dialogue: "I can still see their faces... God forgive us."

Assault Result:
- Germans repelled (first wave)
- ~100 German casualties
- Fort casualties: 12 killed, 20 wounded
- Fort still secure

Player Status:
  Hunger: 80 → 70 (stress, adrenaline burns calories)
  Thirst: 65 → 50 (overheated MG position, smoke, yelling)
  Stamina: 60 → 40 (sustained combat, MG operation)
  Morale: 50 → 30 (mass killing, trauma)
  Alertness: 95 → 90 (combat fatigue setting in)

Post-Combat:
- Rations distributed (bread, water)
- Water: 1L per soldier (ample supply still)
- Rest period (2 hours)
- Prepare for next assault
```

**JUNE 1 - Night: German Infiltration Attempts**

```
Activity: Night watch, expect infiltration

22:00-04:00: Quiet warfare
- Germans probe defenses
- Attempt to place explosives on fort walls
- Player on sentry duty (2-hour shift, 00:00-02:00)

Stealth Encounter (01:15):
- Hear scraping noise outside fort wall
- German engineers placing explosive charges
- Decision:
  - Alert garrison (loud alarm, triggers firefight)
  - OR: Silent grenades (throw grenades through firing port, risky)
  - OR: Wait and observe (gather intel, but risk explosion)

If Alert:
- Garrison opens fire (grenades, rifles)
- Germans retreat, 5 killed
- Explosives not fully placed (defused)
- But: Alert draws artillery (20-minute bombardment)

If Silent Grenades:
- Player throws 2 grenades
- 3 Germans killed, rest flee
- Explosives partially placed (minor damage)
- No artillery response
- Morale +5 (successful stealth action)

If Wait:
- Germans complete placement
- Explosion at 02:30
  - Wall breached (5m hole)
  - 8 soldiers killed in explosion
  - Germans attempt entry through breach
  - Emergency defense (melee combat)
  - Breach eventually sealed with rubble
  - Morale -15 (casualties, breach)

Dawn June 2:
- Situation: Fort damaged but holding
- Garrison: 580 soldiers (20 casualties Day 1)
- Supplies: Adequate (Day 1 of 7)
- Water: 15,000L remaining (enough for ~4 days at current rate)
```

---

##### STAGE 2: FORT BREACHED - TUNNEL FIGHTING (June 3-4)

**JUNE 3: GERMANS BREACH FORT SUPERSTRUCTURE**

```
Historical Event: Germans enter fort via destroyed casemates
Now fighting INSIDE the fort - tunnel combat

Fort Layout (Gameplay):
- Main tunnel (100m long, 2m wide)
- Side galleries (sleeping quarters, storage, command post)
- Multiple levels (upper casemates, main tunnel, lower magazine)
- Now: Upper levels lost, defend main tunnel

Player Position: Barricade #1 (30m into tunnel from breach)
- Sandbag wall across tunnel
- 4 soldiers defending (player + 3 NPCs)
- 50m behind: Barricade #2 (fallback position)

Tunnel Combat Mechanics (NEW GAMEPLAY):
- Tight quarters (2m wide)
- Grenade danger (blast contained in tunnel = more lethal)
- Gas danger (no escape, tunnel fills with gas)
- Hand-to-hand risk (Germans can push through with numbers)
- Visibility: Torchlight only (electricity cut), smoke/dust

German Assault Pattern:
1. Grenade barrage (5-10 grenades)
2. Smoke/gas (obscure vision)
3. Infantry rush (10-15 soldiers)
4. Melee if they reach barricade

Player Combat:
- Rifle fire (bolt-action, 8 rounds, tight reload space)
- Throw back grenades (2-3 second window before detonation)
- Melee if breached (knife, bayonet, shovel)

Combat Intensity:
- Assault every 30-60 minutes
- 6 assaults over 8 hours (compressed to 40 min gameplay)
- Each assault: 5-10 Germans killed, 1-2 French casualties

Critical Event (14:30): BARRICADE #1 FALLS
- Major German push (30 soldiers)
- Overwhelming grenade barrage
- Barricade destroyed
- "FALL BACK! FALL BACK TO POSITION TWO!"
- Fighting retreat (30m sprint under fire)
- 5 NPCs killed in retreat (including friends)
- Player must leave wounded behind (moral choice):
  - Stay and help = likely death
  - Run = guilt, but survival
- Morale -25 (retreat, abandonment, lost position)

New Defense: Barricade #2 (deeper in fort)
- 40m from command post
- Last position before main garrison quarters
- Must hold here or lose fort

Survival Meters (June 3 end):
  Hunger: 70 → 55 (rations running low, combat stress)
  Thirst: 50 → 30 (CRITICAL - water rationing begins)
  Stamina: 40 → 25 (sustained combat, no sleep)
  Morale: 30 → 15 (retreat, losses, desperation)
  Alertness: 90 → 70 (exhaustion setting in despite danger)
  Hygiene: 70 → 50 (blood, smoke, filth)
  Warmth: 75 → 70 (underground, but exertion)

Garrison Status (June 3 night):
- 520 soldiers remaining (80 casualties in 3 days)
- Germans control upper fort
- French control main tunnel + command post
- Water: 6,000L (2-3 days remaining at rationing)
- Ammunition: 40% remaining
- No resupply possible (completely surrounded)
```

**JUNE 4: STALEMATE & WATER CRISIS BEGINS**

```
Situation: Germans cannot advance, French cannot counterattack
Fighting continues in tunnel, but no progress either side

Player Duty: Alternating combat shifts + rest (4 hours on, 4 hours off)

Water Rationing Introduced:
- Commander Raynal: "Water is now critical. Ration: 1 cup per man per day."
- 250ml per day (normally need 2-3L)
- This is SURVIVAL THRESHOLD

Water Crisis Gameplay:
- Thirst meter drains faster (tunnel heat, stress, combat)
- Only 1 ration per day (250ml)
- Must choose WHEN to drink:
  - Morning? (start day hydrated)
  - Before combat? (performance boost)
  - After combat? (recovery)
  - Ration throughout day? (stable but never comfortable)

Physical Degradation:
- Thirst 30 → 20 by end of Day 4
- Headaches (vision blur, -20% aim)
- Weakness (stamina max -40%)
- Confusion (dialogue becomes shorter, player less responsive)
- Morale -10 (suffering, hopelessness)

NPC Behavior:
- Some soldiers drink own urine (player can choose to do this)
  - Restore Thirst +5, but Morale -10, Hygiene -20
- Lick condensation from walls (Thirst +2, disease risk 20%)
- Beg for extra water (none available)
- Hallucinations (soldiers seeing things)

Combat Continues:
- German assaults every 2-3 hours
- Casualties: 2-3 per assault (both sides)
- Ammunition dwindling (now using captured German rifles)
- Garrison: 500 soldiers (20 more casualties Day 4)
```

---

##### STAGE 3: WATER CRISIS & FINAL DEFENSE (June 5-6)

**JUNE 5: NO WATER REMAINS**

```
Commander Raynal: "The water is gone. We are now fighting on will alone."

Water: 0L
Garrison: 480 soldiers (many wounded, dehydrated)
Ammunition: 20% remaining
Morale: CRITICAL

Player Status:
  Hunger: 55 → 45 (rations near-exhausted)
  Thirst: 20 → 5 (CRITICAL - near-collapse)
  Stamina: 25 → 10 (cannot function, weakness)
  Morale: 15 → 10 (despair, but duty remains)
  Alertness: 70 → 40 (dehydration confusion)
  Hygiene: 50 → 40
  Warmth: 70 (underground stable)

Physical State:
- Vision blurred
- Movement slowed (-50% speed)
- Cannot sprint (stamina too low)
- Hallucinations (see water where none exists)
- Lips cracked, tongue swollen

Gameplay Challenge:
- Must still defend barricade
- But: Aim impaired, reaction time slow, weakness
- German assaults continue (they know garrison is dying)

Moral Choice Event (June 5, 15:00):

Wounded NPC: Private Rousseau (age 19, player's friend since February)
- Gut wound (from grenade, 2 days ago)
- Infection (no medical supplies)
- Dehydrated (no water)
- Dying (1-2 hours remaining)
- Conscious, lucid
- Request: "Can I have your water ration? Please... I'm so thirsty..."

Player's water ration: 0ml (none exists)
But: Rousseau doesn't know that yet

Options:
A. Tell truth ("There's no water left. I'm sorry.")
   - Rousseau breaks down crying
   - Dies knowing there's no hope
   - Morale -15 (crushing truth)

B. Lie ("I'll get you water, hold on.")
   - Rousseau: "Thank you... I knew you wouldn't let me die thirsty."
   - Dies believing help is coming
   - Morale -10 (guilt, but merciful lie)

C. Give him your canteen (empty, but he doesn't notice)
   - Rousseau drinks, realizes it's empty
   - "You... tried. Thank you."
   - Dies with gratitude
   - Morale -5 (kindness despite futility)

D. Mercy kill (offer)
   - "I can end it. You won't suffer anymore."
   - Rousseau: "...Yes. Please."
   - Quick death (knife, behind privacy screen)
   - Morale -20 (killed friend) → +10 (ended suffering)

Historical Accuracy:
- Major Raynal wrote in memoir: "The worst was the thirst. Men begged for water that didn't exist. Some went mad. We couldn't help them."

June 5 Night:
- Garrison: 470 soldiers
- 10 died of dehydration (not combat)
- 30 wounded untreated (no water to clean wounds, infection spreading)
- Germans: Continue pressure
- French: Holding, but barely
```

**JUNE 6: CARRIER PIGEON - LAST MESSAGE**

```
Historical Event: Major Raynal sends last carrier pigeon

Morning: Situation desperate
- No water (Day 2 without)
- No ammunition (down to 50 rounds per man)
- No medical supplies
- Wounded dying (15 more dead overnight)
- Germans control 80% of fort
- French hold only command post + main gallery

Commander Raynal: Orders last message sent

Player witnesses (cutscene/interactive):
- Raynal dictates message (blind, cannot write)
  - "We are holding. But cannot hold much longer."
  - "Situation: Water exhausted. Ammunition critical."
  - "Garrison has done its duty."
  - "If relief does not come, we must surrender within 24 hours."
  - "Vive la France."

- Officer writes message, ties to pigeon's leg
- Pigeon released through ventilation shaft (only exit)
- Pigeon flies towards French lines (player watches through periscope)
- German soldiers see pigeon, open fire (rifle shots)
- Pigeon hit (falls), but continues flying (wounded)
- Disappears from view

Historical Note:
- Pigeon's name: "Vaillant" (Valiant)
- Reached French HQ 8km away despite wound
- Died upon arrival (honored, preserved in museum)
- Message confirmed: No relief possible (Somme offensive draining reserves)

Realization: We are on our own. No one is coming.

Morale: 10 → 5 (despair, but resolve remains)

Player Internal Monologue:
- "We'll die here. But we die with honor."
- "Make them pay for every meter. Every second."

Final Defense (June 6, 14:00-18:00):
- Germans launch final assault (200 soldiers, everything they have)
- 4-hour battle (compressed to 20 min intense gameplay)
- Hand-to-hand combat (Germans breach barricade)
- Fighting in command post (final position)
- Grenades, knives, fists, rocks
- Player kills 5 Germans in melee (most visceral combat of game)
- Bayonet charge, trench knife, bare hands

Casualties:
- German: 50 killed, 80 wounded (heavy)
- French: 30 killed, 50 wounded (catastrophic for depleted garrison)
- Garrison: 425 soldiers remaining (many wounded, all dehydrated)

German Commander (outside fort): Realizes garrison won't break
- "They fight like demons. We cannot take this fort by force."
- Decides: Siege, wait for thirst/starvation to force surrender

Nightfall June 6:
- Germans pull back, establish siege line (10m from command post)
- No assault
- Silence (eerie after 6 days of combat)
- French: Cannot attack (too weak)
- Stalemate

Player Status:
  Hunger: 45 → 35 (near-starvation)
  Thirst: 5 → 0 (COLLAPSED STATE)
  Stamina: 10 → 5 (cannot stand without support)
  Morale: 5 (despair, but dignity)
  Alertness: 40 → 30 (delirium)
  Hygiene: 40 → 30
  Warmth: 70

Physical State:
- Collapse (player can barely move)
- Vision: Tunnel vision, hallucinations
- Cannot fight (too weak)
- Lying on gallery floor
- Waiting to die or surrender
```

---

##### STAGE 4: HONORABLE SURRENDER (June 7)

**JUNE 7 - 07:00: SURRENDER NEGOTIATIONS**

```
Historical Event: Major Raynal decides to surrender

Morning:
- Garrison: 420 survivors (55 died overnight from dehydration/wounds)
- No water (Day 3 without)
- No ammunition (15 rounds remain total)
- No food (exhausted 2 days ago)
- Wounded: 150 (infection spreading, dying)

Commander Raynal (NPC cutscene):
- "Gentlemen... we have done all that honor demands."
- "We held for 7 days. We inflicted 900 casualties on the enemy."
- "Our duty is complete. To continue is to murder you all."
- "I will negotiate surrender terms."

Garrison Reaction:
- Silence (no protest)
- Relief (suffering will end)
- Pride (they held)
- Sadness (defeat)
- Morale +10 (honor restored by Raynal's words)

Surrender Process (Interactive):

1. White Flag Display (09:00)
   - French officer waves white cloth through firing port
   - German firing stops
   - Silence (first time in 7 days)

2. German Entry (09:30)
   - German officers enter command post (under truce)
   - See: Devastation (bodies, blood, exhausted soldiers)
   - German reaction: Respect (these men fought impossibly)

3. Surrender Terms Negotiated:
   - French: "We surrender the fort. We request water for wounded."
   - Germans: "Granted. Your garrison fought with honor. You will be treated as POWs, not criminals."
   - Raynal: "May I keep my sword?" (symbol of honor)
   - German: "Yes. You have earned it." (historically accurate - rare honor)

4. Water Arrives (10:00)
   - German soldiers bring water barrels
   - French drink (first water in 3 days)
   - Player drinks (interactive moment)
     - Thirst: 0 → 60 (massive restoration)
     - Vision clears
     - Relief (physical and emotional)
     - Tears (player character cries - automatic cutscene)
   - Germans also give bread (Hunger restoration)

5. Surrender Formality (11:00)
   - French garrison assembles
   - 420 survivors (from 600 starting)
   - Walk out of fort into daylight (first time in 7 days)
   - See: Devastation (fort is ruins, shell craters everywhere)
   - German soldiers line route (form honor guard - historical)
   - German commander salutes Raynal
     - "You and your men are heroes. History will remember."

Player Experience:
- Walk through German lines (cutscene + player control)
- German soldiers: Respectful, some salute
- No mockery, no violence (historical accuracy)
- March to POW collection point
- Medics treat wounded (French and German)

MISSION COMPLETE
Victory Type: Honorable Defeat
```

**FORT VAUX SIEGE DEBRIEFING:**

```
================================
MISSION COMPLETE: Fort Vaux Siege
June 1-7, 1916
================================

SURVIVAL: 7 / 7 days
STATUS: Survived, Prisoner of War

HISTORICAL OUTCOME:
  Fort Vaux falls to German forces
  But: Garrison's defense honored by both sides
  Major Raynal: Paroled due to blindness, honored by Germans
  Fortress garrison: Treated honorably as POWs

GARRISON CASUALTIES:
  Started with: 600 soldiers
  Killed: 100
  Wounded: 150
  Survived: 420 (including player)
  Survival Rate: 70% (remarkably high given circumstances)

GERMAN CASUALTIES:
  Attackers: 2,700
  Killed: 300
  Wounded: 600
  Casualty Rate: 33% (heavy cost for victory)

STATISTICS:
  Days Besieged: 7
  Days Without Water: 3
  Shells Fired at Fort: ~40,000
  Germans Killed by Player: 28 (MG + melee)
  Closest Death Call: Barricade #1 breach (June 3)
  Times Collapsed from Dehydration: 2

SURVIVAL METERS AT SURRENDER:
  Hunger: 35 → 70 (German rations) ⚠️
  Thirst: 0 → 60 (German water) ⚠️
  Stamina: 5 → 30 (rest + food) ⚠️
  Morale: 15 → 40 (honorable surrender, survival) ⚠️
  Alertness: 30 → 50 (recovery beginning) ⚠️
  Hygiene: 30 → 30 (filthy, POW camp ahead) ⚠️
  Warmth: 70 ✅

PSYCHOLOGICAL STATUS:
  Shell Shock Risk: 75% (sustained trauma)
  PTSD: Guaranteed (dehydration, melee combat, friend deaths)
  Trauma Events: 12 (bombardment, breaches, deaths, dehydration, surrender)

MORAL CHOICES MADE:
  - Infiltration response: [Logged]
  - Rousseau's death: [Logged]
  - Melee combat: [28 kills recorded]

CAMPAIGN IMPACT:
  Player now POW - Two ending paths available:

  PATH A: POW Camp Campaign
    - Player experiences German POW camps (1916-1918)
    - Different gameplay (survival in captivity)
    - Campaign continues to November 1918 Armistice
    - Victory: Survive to war's end, liberated

  PATH B: Return to Verdun (Optional)
    - Historical: Some POWs escaped or were exchanged
    - Player returns to Verdun (July 1916)
    - Campaign continues with original unit
    - Marked as "Fort Vaux Survivor" (NPC reactions changed)

PROGRESS: Day 132 / 303
         (171 days remaining until Armistice)

HISTORICAL NOTE:
Fort Vaux was recaptured by French forces on November 2, 1916
Your garrison's sacrifice was not in vain - you delayed the German advance by 7 critical days

"Ils ne passeront pas." They did not pass. Not while we lived.
================================

UNLOCKED:
- Achievement: "Vaillant" (Survive Fort Vaux Siege)
- Character Title: "Defender of Vaux"
- NPC Dialogue: Special recognition from soldiers who know your story
```

---

## [DAY-IN-THE-LIFE MISSION EXAMPLES COMPLETE]

**Summary**: Four detailed mission examples created:

1. ✅ **Front-Line Rotation** (March 15-21) - Hour-by-hour Day 1, grinding survival Days 2-7
2. ✅ **Support-Line Rotation** (April 10-16) - Resupply missions, burial details, wounded station, no-man's-land recovery
3. ✅ **Rest Area Experience** (May 1-7) - Recovery, normalcy trap, anticipatory dread, desertion temptation
4. ✅ **Special Event Mission: Fort Vaux Siege** (June 1-7) - Multi-stage siege, water crisis, tunnel combat, honorable surrender

**Mission Examples Show:**
- Complete gameplay loops (survival meters + combat + narrative)
- Psychological progression (fear → numbness → trauma)
- Moral choice integration (utilitarian vs. humanity)
- Historical accuracy within gameplay framework
- Different mission structures (rotation vs. special event)
- Victory conditions beyond "kill enemies" (survival, honor, duty)

**Total Mission Content**: ~8,000 words of detailed mission specifications

---

**Current Document Length**: ~30,000 words
**Target Final Length**: 50,000-100,000 words (comprehensive game bible)

---

## [MISSION STRUCTURE COMPLETE - Ready for individual mission list]

**Mission Categories to Create Next**:
1. Tutorial Missions (2-3) - Feb 21-22 first day
2. Early Rotations (Feb-Apr, 4-6 missions)
3. Mid-Campaign Rotations (May-Sep, 8-10 missions)
4. Climax Missions (Oct-Nov, 3-5 missions) - Douaumont recapture, etc.
5. Final Rotations (Nov-Dec, 3-4 missions)
6. Special Event Missions (5-7 missions) - Vaux (done), Douaumont fall, Mort-Homme, Souville, etc.

**Total Planned**: 30-35 missions

---

## DOCUMENT NOTES

This is the **LIVING GAME DESIGN DOCUMENT** for Only The Dead. It will be continuously expanded with:

✅ Completed:
- Core vision and design pillars
- 7 survival meters (fully specified with exact formulas)
- Combat & artillery systems (complete specifications)
  - All shell types (75mm, 155mm, 210mm, 305mm, 420mm, gas)
  - Exact damage/blast radius formulas
  - Bombardment patterns (harassing, light, heavy, drumfire)
  - Shell recognition minigame
  - Small arms, grenades, melee combat
- Mission structure & rotation framework
- **Technical Implementation (Phase III Started - Nov 2025)**
  - **Camera & Cover System** (~3,500 lines C++, UE5)
    - Hellblade 2-style hybrid camera (4 contextual modes)
    - Shoulder-swap OTS camera with smooth blending
    - Procedural artillery shelter detection (6 cover types, 60%-99% protection)
    - Exposure heatmap for AI targeting (attracts artillery/snipers)
    - Posture system (Standing/Crouching/Prone) integrated with stamina
    - 7 survival meters component (all formulas implemented)
    - Enhanced Input system (Keyboard/Mouse + Gamepad support)
    - Accessibility features (fixed horizon, camera lag, set-piece toggles)
  - **Documentation**: Complete integration guide + system summary
  - See: [`CAMERA_COVER_IMPLEMENTATION_GUIDE.md`](CAMERA_COVER_IMPLEMENTATION_GUIDE.md)

🔄 In Progress:
- Mission designs (4 detailed examples complete, 30+ planned)
- Combat system implementation (weapons, aiming, melee)
- AI & Artillery implementation (EQS targeting, shell impacts)

⏳ Planned:
- Complete system specifications
- Full dialogue database
- Audio/visual design bible
- Character database
- Mission design (30+ missions)
- AI behavior specs
- And much more...

---

**Document maintained by**: Claude (Verdun Historian → Game Designer)
**For**: "Only The Dead" Development Team
**Companion Document**: Verdun Research Anthology (historical foundation)
