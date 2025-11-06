# ONLY THE DEAD - DEVELOPMENT BOOK
**WWI Survival Game Design Bible**

**Version**: 1.0.0
**Last Updated**: November 6, 2025
**Status**: In Development

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

### PART VII: TECHNICAL IMPLEMENTATION
26. [Save System & Permadeath](#save-system)
27. [Performance Targets](#performance)
28. [Localization Plan](#localization)
29. [Accessibility Features](#accessibility)

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

## [MISSION STRUCTURE COMPLETE - Individual missions to follow]

**Mission Categories to Create**:
1. Tutorial Missions (2-3)
2. Early Rotations (Feb-Apr, 4-6 missions)
3. Mid-Campaign Rotations (May-Sep, 8-10 missions)
4. Climax Missions (Oct-Nov, 3-5 missions)
5. Final Rotations (Nov-Dec, 3-4 missions)
6. Special Event Missions (5-7 missions)

**Total**: 30-35 missions covering full 303-day campaign

---

**Current Document Length**: ~15,000 words
**Target Final Length**: 50,000-100,000 words (comprehensive game bible)

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
- Technical specifications

🔄 In Progress:
- Mission designs (starting next)

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
