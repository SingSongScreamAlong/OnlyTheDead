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

## [CONTINUATION MARKER]

**Status**: Part II (Game Systems) started
**Next Sections Needed**:
- Combat & Artillery Systems (detailed shell mechanics)
- Medical & Injury System (20+ wound types)
- Progression & Meta-Game (how player advances through 303 days)
- Mission Design (30+ mission specifications)
- Dialogue System
- Audio Bible
- And 20+ more sections...

**Current Document Length**: ~3,500 words
**Target Final Length**: 50,000-100,000 words (comprehensive game bible)

---

## DOCUMENT NOTES

This is the **LIVING GAME DESIGN DOCUMENT** for Only The Dead. It will be continuously expanded with:

✅ Completed:
- Core vision and design pillars
- 7 survival meters (fully specified with exact formulas)
- Technical specifications

🔄 In Progress:
- Combat/artillery systems
- Mission designs

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
