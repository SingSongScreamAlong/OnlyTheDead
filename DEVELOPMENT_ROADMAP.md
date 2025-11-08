# ONLY THE DEAD - DEVELOPMENT ROADMAP
**WWI Survival Game - Comprehensive Development Plan**

**Version**: 1.0
**Date**: November 8, 2025
**Status**: Pre-Production (Design Phase)

---

## EXECUTIVE SUMMARY

### Current Status
- **Research Anthology**: 65% complete (~190,000 words)
- **Development Book**: 15% complete (~30,000 words)
- **Implementation**: 0% (design phase only)
- **Total Documentation**: 220,000+ words across 85 files

### What We Have ✅
- World-class historical research foundation
- Complete survival & combat systems (implementation-ready)
- Production-ready terrain tools (80+ locations, SRTM data)
- Innovative rotation-based mission structure
- 45 fully developed characters
- 4 detailed mission examples

### Critical Gaps ❌
- **FEBRUARY 1916 TIMELINE** - Battle starts Feb 21! (BLOCKING)
- **30+ Mission Specifications** - Only 4 exist
- **Audio/Visual Design Bible** - 0% complete
- **Dialogue & Narrative Content** - 0% complete
- **Level Design Specs** - 0% complete
- **German Perspective** - Heavily underrepresented

### Timeline to Completion
- **Design Document Complete**: 6-12 months
- **Prototype Alpha**: 12-18 months (if development starts)
- **Full Release**: 24-36 months (with team)

---

## ROADMAP PHASES

### PHASE 0: FOUNDATION ✅ **COMPLETE**
**Duration**: 6 months (already done)
**Status**: Complete

**Deliverables**:
- ✅ Core vision & design pillars
- ✅ 7 survival systems (exact specifications)
- ✅ Complete artillery & combat systems
- ✅ Mission structure framework
- ✅ 10 months detailed timeline (Feb missing!)
- ✅ 7 survival system docs (141,000 lines)
- ✅ Geographic data package (80+ locations)
- ✅ Character rosters (45 characters)

**Achievement**: Solid foundation for an exceptional game.

---

### PHASE 1: CRITICAL BLOCKERS 🔥 **URGENT**
**Duration**: 4-6 weeks
**Priority**: HIGHEST - These block all mission development

#### Week 1-2: February 1916 Timeline
**File**: `/verdun_anthology/timeline/verdun_february_1916_detailed.md`

**Required Content**:
- **Feb 1-20**: Build-up (German preparations, French intelligence failures)
- **Feb 21**: THE MOST IMPORTANT DAY
  - Hour-by-hour: 0700-1700 (9-hour bombardment)
  - 1,000,000+ shells fired
  - Bois des Caures defense
  - French positions obliterated
- **Feb 22**: Col. Driant killed, German advance
- **Feb 23-24**: Fort Douaumont isolated
- **Feb 25**: Douaumont falls (catastrophic failure)
- **Feb 26**: Pétain arrives, "Ils ne passeront pas"
- **Feb 27**: Voie Sacrée established
- **Feb 28-29**: French line stabilizes

**Estimated Effort**: 20,000-25,000 lines (match other months)
**Research Sources**:
- Horne, Alistair. *The Price of Glory* (Chapter 8-10)
- Pétain archives (Service Historique de la Défense)
- German 5th Army war diary (Bundesarchiv)

---

#### Week 3-4: Tutorial Mission Specifications
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (add to Part III)

**Mission 1: "The Guns Begin" (Feb 21, 1916)**
- **Duration**: 2-3 hours gameplay
- **Location**: Bois des Caures, 2nd Line (Grid: 48.292°N, 5.412°E)
- **Playable Character**: Henri Moreau (FR002 - witness character)
- **Objectives**:
  1. Survive the 9-hour bombardment (0700-1700)
  2. Learn all 7 survival systems
  3. Assist wounded squadmates
  4. Receive orders from Col. Driant
  5. Survive until darkness (tutorial end)

**Tutorial Mechanics Introduced**:
- Shell recognition minigame (75mm, 77mm, 150mm)
- Shelter quality system
- Hunger/thirst management
- Morale degradation
- First aid basics
- Movement in trenches

**Mission 2: "Driant's Stand" (Feb 22, 1916)**
- **Duration**: 1-2 hours gameplay
- **Location**: Bois des Caures, final stand
- **Objectives**:
  1. Defend position with Col. Driant
  2. Witness Driant's death (historical event)
  3. Retreat to second line
  4. First combat encounter (defensive)
  5. Evacuate wounded

**Tutorial Mechanics Introduced**:
- Combat (Lebel rifle, F1 grenades)
- Tactical retreat
- Carrying wounded
- Death of named characters (emotional impact)
- Historical inevitability (can't save Driant)

**Estimated Effort**: 8,000-12,000 words per mission

---

#### Week 5-6: Climax Mission Specification
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (add to Part III)

**Mission: "Reclaiming Douaumont" (Oct 24, 1916)**
- **Duration**: 3-4 hours gameplay
- **Location**: Fort Douaumont & surroundings
- **Playable Character**: Henri Moreau (if survived) OR new character
- **Historical Context**: Most important French victory of battle

**Mission Structure**:

**Phase 1: The Approach (Night)**
- 0400-0600: Assembly in Ravin de la Dame
- Navigate no-man's-land in darkness
- Avoid German patrols
- Reach jump-off position

**Phase 2: The Barrage (Dawn)**
- 0600-1130: Wait through French artillery preparation
- Survival challenge: friendly fire risk
- Morale management (anticipatory fear)
- Final equipment check

**Phase 3: The Assault (Morning)**
- 1130-1200: Go over the top
- Cross 800m of cratered terrain
- Take casualties from machine guns
- Reach fort perimeter

**Phase 4: The Fort (Afternoon)**
- 1200-1500: Clear fort interior
- Tunnel fighting (claustrophobic)
- German counterattacks
- Raise French flag at 1500 (historical time)

**Phase 5: The Defense (Evening)**
- 1500-2000: Hold against German counter-barrage
- Evacuate wounded
- Consolidate position
- Survival until relief

**Unique Mechanics**:
- Large-scale assault (20+ friendlies on screen)
- Fort interior navigation (multi-level, dark)
- Victory music (only time in game)
- Emotional peak (avenging Feb 25 disaster)

**Estimated Effort**: 12,000-15,000 words

---

**PHASE 1 DELIVERABLES**:
- ✅ February 1916 timeline (20,000 lines)
- ✅ Tutorial Mission 1: "The Guns Begin" (10,000 words)
- ✅ Tutorial Mission 2: "Driant's Stand" (10,000 words)
- ✅ Climax Mission: "Reclaiming Douaumont" (15,000 words)

**Total**: ~45,000 words, 4-6 weeks effort

---

### PHASE 2: MISSION CAMPAIGN 📝 **HIGH PRIORITY**
**Duration**: 8-12 weeks
**Goal**: Complete 30-mission campaign specification

#### Mission List (30 Total Missions)

**Tutorial Arc (2 missions)** ✅ Done in Phase 1
1. The Guns Begin (Feb 21)
2. Driant's Stand (Feb 22)

**Early Campaign (Feb-Apr) - 6 missions**
3. "Fort Douaumont Falls" (Feb 25) - Witness disaster
4. "Pétain Arrives" (Feb 26-27) - Reorganization, new hope
5. "Mort-Homme Assault" (Mar 6-9) - Left Bank offensive begins
6. "Hill 304" (Mar 20-24) - Bloodiest fighting
7. "The Voie Sacrée" (Apr 1-7) - Supply convoy protection
8. "Tunnels of Death" (Apr 15-20) - Underground mine warfare

**Mid Campaign (May-Sep) - 10 missions**
9. "May Grinding" (May 1-7) - Standard rotation example
10. "Fort Vaux: The Siege Begins" (Jun 1-2) ✅ Already detailed
11. "Fort Vaux: The Cistern Runs Dry" (Jun 3-5)
12. "Fort Vaux: Surrender" (Jun 6-7)
13. "Fleury Village" (Jun 23-28) - Changes hands 16 times
14. "Thiaumont Farm" (Jul 12-18) - Redoubt fighting
15. "Gas Attack" (Jul 22-23) - Phosgene attack survival
16. "August Stalemate" (Aug 3-10) - Grinding attrition
17. "September Patrols" (Sep 5-12) - No-man's-land reconnaissance
18. "The Waiting" (Sep 20-27) - Pre-offensive tension

**Climax Arc (Oct-Nov) - 6 missions**
19. "Reclaiming Douaumont" (Oct 24) ✅ Done in Phase 1
20. "Holding Douaumont" (Oct 25-30) - German counter-barrage
21. "Fort Vaux Recapture" (Nov 2-3) - Second major victory
22. "The Push Continues" (Nov 10-15) - Exploitation
23. "Winter Returns" (Nov 20-25) - Cold and exhaustion
24. "November's End" (Nov 28-30) - Consolidation

**Final Arc (Dec) - 4 missions**
25. "December Offensive" (Dec 11-14) - Final push begins
26. "Louvemont Recaptured" (Dec 15-16) - Gains restored
27. "The Last Day" (Dec 18) - Battle officially ends
28. "Aftermath" (Dec 19-25) - Reflection and counting cost

**Special Missions - 2 additional**
29. "Night Courier" (any month) - Message delivery under fire
30. "Burial Detail" (any month) - Psychological horror

**Estimated Effort**: 8,000 words per mission × 28 new missions = **224,000 words**
**Timeline**: 10-12 weeks (2-3 missions per week)

---

#### Mission Specification Template

For each mission, include:

1. **Historical Context** (500 words)
   - Date and significance
   - Strategic situation
   - Weather and conditions
   - Casualty statistics

2. **Mission Overview** (300 words)
   - Playable character
   - Duration (gameplay hours)
   - Location (coordinates, map reference)
   - Primary objectives
   - Secondary objectives

3. **Phase Breakdown** (2,000 words)
   - Hour-by-hour structure
   - Key events and challenges
   - Survival meter priorities
   - Environmental hazards

4. **Unique Mechanics** (500 words)
   - Special gameplay for this mission
   - New systems introduced
   - Challenge variations

5. **Victory/Failure Conditions** (300 words)
   - Success criteria
   - Failure states
   - Player choices and consequences

6. **Narrative Beats** (1,000 words)
   - Character moments
   - Dialogue snippets
   - Emotional arc
   - Historical reverence

7. **Implementation Notes** (400 words)
   - Technical challenges
   - Asset requirements
   - AI behaviors needed

**Total per mission**: ~5,000-10,000 words

---

**PHASE 2 DELIVERABLES**:
- ✅ 28 additional mission specifications (224,000 words)
- ✅ Complete mission flowchart (campaign structure)
- ✅ Mission unlock tree (progression system)

**Total**: ~224,000 words, 8-12 weeks effort

---

### PHASE 3: NARRATIVE & DIALOGUE 🎭 **HIGH PRIORITY**
**Duration**: 6-8 weeks
**Goal**: Complete Part IV of Development Book

#### Week 1-2: Character Database (Section 16)
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (Part IV)

**Content Required**:
- **In-Game Character Profiles** (45 characters from rosters)
  - Game mechanics integration (stats, survival tendencies)
  - Voice casting notes (French/German authenticity)
  - Character interaction trees
  - Relationship systems (friendship, rivalry, mentorship)

- **Character Archetypes**:
  - The Survivor (Henri Moreau)
  - The Doomed Hero (Col. Driant)
  - The Cynic (Marcel Renard)
  - The Believer (Chaplain Bernard)
  - The Innocent (young conscripts)
  - The Veteran (scarred by experience)

**Estimated Effort**: 8,000-10,000 words

---

#### Week 3-4: Dialogue System & Scripts (Section 17)
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (Part IV)

**Content Required**:

1. **Dialogue Trees** (5,000 words)
   - Conversation system mechanics
   - Player choice integration
   - Relationship impact
   - Historical accuracy in speech patterns

2. **Combat Barks** (3,000 words)
   - French: "Attention!" "Obus!" "Masque à gaz!"
   - German: "Achtung!" "Granate!" "Gasalarm!"
   - Context-sensitive (under fire, retreating, charging)

3. **Survival Barks** (2,000 words)
   - Hunger: "J'ai faim..." "I'm starving..."
   - Thirst: "De l'eau, pour l'amour de Dieu!"
   - Cold: "Je ne sens plus mes doigts..."
   - Fear: "C'est la fin..." "This is the end..."

4. **Ambient Conversations** (5,000 words)
   - Trench small talk
   - Letters from home shared
   - Dark humor
   - Philosophical discussions
   - Period-accurate slang

5. **Command Voices** (2,000 words)
   - Officers giving orders
   - Radio chatter
   - Whistle signals
   - Code words

**Example Scripts**:
```
MOREAU (exhausted): "Encore combien de jours?" (How many more days?)
RENARD (cynical): "Autant qu'il faudra." (As many as it takes.)
MOREAU: "Et si on ne survit pas?"
RENARD (dark laugh): "Alors on sera avec les autres." (Then we'll be with the others.)
```

**Estimated Effort**: 17,000 words

---

#### Week 5-6: Environmental Storytelling (Section 18)
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (Part IV)

**Content Required**:
- **Found Letters** (50 examples, placement notes)
- **Body Positioning** (environmental narratives through corpse placement)
- **Graffiti & Trench Art** (50 examples, historical accuracy)
- **Destroyed Village Details** (what remains tells stories)
- **Personal Effects** (photos, talismans, letters)

**Estimated Effort**: 8,000-10,000 words

---

#### Week 7-8: Cutscene Specifications (Section 19)
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (Part IV)

**Required Cutscenes**:

1. **Opening Cutscene** (Feb 21, 0700)
   - 3-5 minute cinematic
   - Establish tone (horror, not heroism)
   - Introduce Moreau
   - First shells fall
   - Script: 2,000 words

2. **Mission Briefing Cutscenes** (30 × 200 words)
   - Template for each mission
   - Officer gives orders
   - Map overlay
   - Objectives stated
   - Script: 6,000 words total

3. **Character Death Scenes** (10 major deaths)
   - Driant (Feb 22)
   - Renard (varies by player choices)
   - Each with emotional weight
   - Script: 500 words each = 5,000 words

4. **Climax Cutscene** (Oct 24, 1500)
   - French flag raised over Douaumont
   - Only triumphant moment in game
   - 2-3 minutes
   - Script: 1,500 words

5. **Ending Sequence** (Dec 18)
   - Battle ends
   - Reflection on cost (700,000 casualties)
   - Memorial to the dead
   - "Only the dead have seen the end of war"
   - Script: 2,000 words

**Estimated Effort**: 16,500 words

---

**PHASE 3 DELIVERABLES**:
- ✅ Character Database (10,000 words)
- ✅ Dialogue System & Scripts (17,000 words)
- ✅ Environmental Storytelling (10,000 words)
- ✅ Cutscene Specifications (16,500 words)

**Total**: ~53,500 words, 6-8 weeks effort

---

### PHASE 4: AUDIO & VISUAL DESIGN 🎨 **HIGH PRIORITY**
**Duration**: 4-6 weeks
**Goal**: Complete Part V of Development Book

#### Week 1-2: Audio Design Bible (Section 20)
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (Part V)

**Content Required**:

1. **Shell Sound Signatures** (CRITICAL - gameplay mechanic)
   - 75mm: High-pitched whistle, 15-20 seconds warning
   - 77mm: Lower whistle, 12-18 seconds
   - 155mm: Deep rumble, 20-30 seconds
   - 210mm: Thunder-like roar, 25-30 seconds
   - 420mm: Earthquake sound, 30-45 seconds
   - Gas shells: Distinctive "plop" on impact
   - **Audio waveform descriptions**
   - **Frequency analysis** (for recognition minigame)

2. **3D Spatial Audio Specifications**
   - Distance attenuation curves
   - Occlusion (underground vs. surface)
   - Reverb in tunnels
   - Mud dampening effects

3. **Voice Processing**
   - French accent authenticity
   - German accent authenticity
   - Radio distortion
   - Gas mask muffling
   - Distance degradation

4. **Environmental Ambience**
   - Trench sounds (dripping water, rats, wind)
   - No-man's-land (silence punctuated by distant guns)
   - Bombardment layers (near/mid/far)
   - Underground ambience
   - Weather sounds (rain on tin, wind howling)

5. **Music Philosophy**
   - Minimal usage (let environmental audio dominate)
   - Period-appropriate instruments (if used)
   - Emotional moments only (Douaumont recapture)
   - Silence as musical element

6. **Sound Effect List** (estimated 2,000+ needed)
   - **Weapons**: Lebel rifle, Gewehr 98, MG08, grenades
   - **Artillery**: 12 shell types × (incoming, impact, shrapnel)
   - **Movement**: Mud squelch, boots on duckboards, climbing
   - **Survival**: Eating, drinking, shivering, coughing
   - **Environmental**: Rain, wind, explosions, screams
   - **UI**: Menu sounds, meter warnings, notifications

**Estimated Effort**: 15,000-20,000 words

---

#### Week 3-4: Visual Style Guide (Section 21)
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (Part V)

**Content Required**:

1. **Art Direction Philosophy**
   - Photorealistic environment (Nanite terrain)
   - Slightly stylized characters (avoid uncanny valley)
   - Balance: realism vs. playability
   - Gore approach (present but not exploitative)

2. **Color Palette**
   - **Primary**: Browns (mud), greys (stone), desaturated blues (sky)
   - **Accents**: French horizon blue uniforms, German feldgrau
   - **Atmospheric**: Orange (explosions), yellow-green (gas)
   - **Seasonal shifts**: Winter whites, spring greens (muted)

3. **Lighting Philosophy**
   - Overcast dominates (historically accurate)
   - Darkness important (night missions)
   - Flare light (temporary, dramatic)
   - Explosion lighting (critical for shell recognition)
   - Underground: lanterns, candles, darkness

4. **Period Photography References**
   - List of 50+ key reference photos
   - Photographers: Jules Gervais-Courtellemont, Paul Castelnau
   - Archives: Service Historique de la Défense
   - Banned references (avoid staged propaganda)

5. **Character Visual Design**
   - French uniforms (horizon blue, Adrian helmet)
   - German uniforms (feldgrau, Stahlhelm)
   - Dirt accumulation system
   - Injury visualization
   - Fatigue in animations

6. **Environmental Asset Style**
   - Trench construction (sandbags, duckboards, wire)
   - Craters (accurate morphology)
   - Destroyed trees (shattered, burnt)
   - Mud (viscous, deep, omnipresent)
   - Fortifications (concrete, steel, damage)

**Estimated Effort**: 12,000-15,000 words

---

#### Week 5-6: UI/UX Specifications (Section 22)
**File**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` (Part V)

**Content Required**:

1. **HUD Design**
   - Survival meters display (always visible but subtle)
   - Compass and objectives
   - Weapon/ammo counter
   - Context prompts
   - Shell warning indicator (critical!)
   - Minimal HUD philosophy (immersion)

2. **Inventory UI**
   - Grid-based (realistic capacity)
   - Weight system visualization
   - Equipment condition
   - Quick slots (water, food, bandages)

3. **Map System**
   - Period-accurate map style
   - Fog of war
   - Trench network overlay
   - Objective markers
   - Historical annotations

4. **Journal/Diary Interface**
   - Mission log
   - Character relationships
   - Historical context (educational)
   - Letters collected
   - Statistics (survival days, rotations)

5. **Settings Menu**
   - Difficulty options
   - Accessibility features
   - Audio/visual settings
   - Control remapping

6. **Accessibility Options UI**
   - Colorblind modes (shell recognition assist)
   - Subtitle size/contrast
   - UI scale
   - Tutorial toggles

**Mockups Needed**: 20-30 wireframe sketches (described in text)

**Estimated Effort**: 10,000-12,000 words

---

**PHASE 4 DELIVERABLES**:
- ✅ Audio Design Bible (20,000 words)
- ✅ Visual Style Guide (15,000 words)
- ✅ UI/UX Specifications (12,000 words)

**Total**: ~47,000 words, 4-6 weeks effort

---

### PHASE 5: WORLD BUILDING 🌍 **MEDIUM PRIORITY**
**Duration**: 4-6 weeks
**Goal**: Complete Part VI of Development Book

#### Week 1-2: Level Design Specifications (Section 23)

**Content Required**:

1. **Fort Interiors** (5,000 words)
   - Fort Douaumont (multi-level, ~400m long)
   - Fort Vaux (smaller, claustrophobic)
   - Fort Souville (never fell, defensive)
   - Room-by-room blockouts
   - Tunnel networks
   - Ventilation shafts

2. **Trench System Layouts** (5,000 words)
   - Front-line trenches (firing steps, parapets)
   - Support trenches (dugouts, command posts)
   - Communication trenches (narrow, winding)
   - Trench types (French vs. German)
   - Duckboards, sumps, latrines

3. **Destroyed Village Blockouts** (3,000 words)
   - Fleury-devant-Douaumont (obliterated)
   - Beaumont (ruins)
   - Louvemont (craters and rubble)
   - What remains vs. what's gone
   - Navigation through ruins

4. **No-Man's-Land Navigation** (4,000 words)
   - Crater density maps
   - Wire obstacles
   - Corpse fields
   - Shell holes as cover/hazard
   - Mud navigation

5. **Underground Tunnel Systems** (3,000 words)
   - Mine galleries
   - Listening posts
   - Counter-mine tactics
   - Claustrophobia mechanics

6. **Command Post Interiors** (2,000 words)
   - French: Souville command
   - German: Kronprinz HQ
   - Maps, communications, staff

**Estimated Effort**: 22,000 words

---

#### Week 3-4: AI Behavior Specifications (Section 24)

**Content Required**:

1. **Friendly AI** (5,000 words)
   - Squadmate behaviors (follow, take cover, panic)
   - Officer behaviors (give orders, inspire morale)
   - Medic behaviors (prioritize wounded)
   - AI survival (eat, drink, sleep when safe)
   - Relationship system (trust/distrust)

2. **Enemy AI** (3,000 words)
   - Limited (artillery is main threat)
   - Defensive positions (machine guns)
   - Patrols (predictable patterns)
   - Gas attacks (coordinated)
   - NOT superhuman (WWI tactics)

3. **Civilian AI** (2,000 words)
   - Rest area interactions
   - Medical staff
   - Supply depot workers
   - Scripted vs. ambient

4. **Animal AI** (2,000 words)
   - Rats (scavenging, attacks)
   - Horses (wounded, terrified)
   - Birds (rare, ominous when absent)

**Estimated Effort**: 12,000 words

---

#### Week 5-6: Dynamic Weather & Time System (Section 25)

**Content Required**:

1. **Weather Patterns by Month** (3,000 words)
   - February: Cold, occasional snow
   - March-April: Rain (mud season begins)
   - May-June: Warming, thunderstorms
   - July-August: Heat, dust
   - September-October: Rain returns
   - November-December: Cold, frost, snow

2. **Day/Night Cycle** (2,000 words)
   - Sunrise/sunset times by month
   - Darkness mechanics (vision range)
   - Moon phases (historical accuracy)
   - Flare illumination

3. **Visibility System** (3,000 words)
   - Fog effects (common)
   - Smoke from artillery
   - Gas clouds (deadly low-lying)
   - Distance rendering

4. **Mud Dynamic System** (4,000 words)
   - Saturation mechanics (rain accumulation)
   - Drying (sunshine required)
   - Movement penalties (deep mud = slow death)
   - Historical mud levels (April-May worst)

5. **Temperature Effects** (2,000 words)
   - Winter: Frostbite, hypothermia
   - Summer: Dehydration faster
   - Shelter warmth calculations

**Estimated Effort**: 14,000 words

---

**PHASE 5 DELIVERABLES**:
- ✅ Level Design Specifications (22,000 words)
- ✅ AI Behavior Specifications (12,000 words)
- ✅ Dynamic Weather & Time System (14,000 words)

**Total**: ~48,000 words, 4-6 weeks effort

---

### PHASE 6: TECHNICAL IMPLEMENTATION 🔧 **MEDIUM PRIORITY**
**Duration**: 3-4 weeks
**Goal**: Complete Part VII of Development Book

#### Week 1: Save System & Permadeath (Section 26)

**Content Required**:
- Checkpoint vs. autosave systems
- Permadeath implementation (Ironman mode)
- Character state persistence
- Rotation progress saving
- Cloud save integration

**Estimated Effort**: 5,000 words

---

#### Week 2: Performance Targets (Section 27)

**Content Required**:
- PC: 60 FPS @ 1440p (detailed specs)
- PS5/Xbox: Quality vs. Performance modes
- Optimization strategies (LOD, culling, streaming)
- Benchmark scenes
- Profiling approach

**Estimated Effort**: 5,000 words

---

#### Week 3: Localization Plan (Section 28)

**Content Required**:
- Voice: French/German (authentic)
- Subtitles: English + 10 languages
- UI translation approach
- Cultural sensitivity (German market)
- Text expansion ratios

**Estimated Effort**: 4,000 words

---

#### Week 4: Accessibility Features (Section 29)

**Content Required**:
- Colorblind modes (critical for shell recognition)
- Subtitle customization
- Difficulty assist options (meter depletion rates)
- Input remapping
- Screen reader support (menus)
- Photosensitivity warnings

**Estimated Effort**: 6,000 words

---

**PHASE 6 DELIVERABLES**:
- ✅ Save System & Permadeath (5,000 words)
- ✅ Performance Targets (5,000 words)
- ✅ Localization Plan (4,000 words)
- ✅ Accessibility Features (6,000 words)

**Total**: ~20,000 words, 3-4 weeks effort

---

### PHASE 7: PRODUCTION PLANNING 📋 **LOW PRIORITY**
**Duration**: 3-4 weeks
**Goal**: Complete Part VIII of Development Book

#### Week 1-2: Asset Lists (Section 30)

**Content Required**:

1. **3D Models** (estimated 5,000+ assets)
   - Characters (45 unique, variations)
   - Weapons (20 types)
   - Environment (trenches, craters, trees, rubble)
   - Props (equipment, furniture, debris)
   - Fortifications (walls, bunkers, doors)

2. **Textures & Materials**
   - Terrain (mud, stone, grass, snow)
   - Uniforms (French, German, variations)
   - Weathering (dirt, blood, wear)

3. **Audio Files**
   - SFX: 2,000+ individual sounds
   - Music: 10-15 tracks (minimal)
   - Voice: 50,000+ lines (estimated)

4. **Animations**
   - Character (1,000+ animations)
   - Weapons (reload, fire, melee)
   - Environmental (flags, cloth, debris)

5. **VFX**
   - Explosions (shell types, sizes)
   - Gas (phosgene green, mustard yellow)
   - Mud splatter, dirt, blood
   - Weather (rain, snow, fog)

**Estimated Effort**: 15,000 words

---

#### Week 3: Development Roadmap (Section 31)

**Content Required**:
- Milestones (Prototype → Alpha → Beta → Release)
- Team structure (ideal size: 20-40 people)
- Budget estimates ($5-15M indie, $20-50M AA)
- Timeline to release (24-36 months with team)
- Risk assessment

**Estimated Effort**: 8,000 words

---

#### Week 4: Testing & QA Plan (Section 32)

**Content Required**:
- Playtest goals (balance, pacing, emotional impact)
- Historical accuracy review process
- Balance testing (survival meter tuning)
- Bug tracking approach
- Beta test plan (historical communities, educators)

**Estimated Effort**: 7,000 words

---

**PHASE 7 DELIVERABLES**:
- ✅ Asset Lists (15,000 words)
- ✅ Development Roadmap (8,000 words)
- ✅ Testing & QA Plan (7,000 words)

**Total**: ~30,000 words, 3-4 weeks effort

---

### PHASE 8: ANTHOLOGY EXPANSION 📚 **MEDIUM PRIORITY**
**Duration**: 4-6 weeks (parallel with other phases)
**Goal**: Balance German perspective, add primary sources

#### Week 1-2: German Perspective

**Add to Anthology**:
- 5-10 German letters (common soldiers, officers, families)
- 2-3 German diaries (artillery observer, tunnel engineer)
- German newspaper examples (Berliner Tageblatt, etc.)
- German official documents
- German medical reports

**Estimated Effort**: 30,000-40,000 words

---

#### Week 3-4: Primary Sources Expansion

**Add to Anthology**:
- 10+ official orders (Joffre, Nivelle, Falkenhayn)
- 5+ speeches (parliamentary, military)
- Weather reports (daily logs)
- Casualty lists (verified)
- Court-martial records (2-3 examples)

**Estimated Effort**: 20,000-30,000 words

---

#### Week 5-6: Trench Journals & Flavor

**Add to Anthology**:
- Trench journals (soldier humor, satire)
- Postcards (50 examples)
- Graffiti catalog (100 examples)
- Personal effects descriptions
- Maps (tactical, annotated)

**Estimated Effort**: 15,000-20,000 words

---

**PHASE 8 DELIVERABLES**:
- ✅ Balanced German perspective (40,000 words)
- ✅ Expanded primary sources (30,000 words)
- ✅ Cultural/flavor content (20,000 words)

**Total**: ~90,000 words, 4-6 weeks effort

---

## TIMELINE SUMMARY

### Sequential Path (Full-Time Work)

| Phase | Duration | Cumulative | Words Added | Status |
|-------|----------|------------|-------------|--------|
| **Phase 0** | 6 months | 6 months | 220,000 | ✅ COMPLETE |
| **Phase 1** | 4-6 weeks | 7.5 months | 45,000 | 🔥 URGENT |
| **Phase 2** | 8-12 weeks | 10.5 months | 224,000 | High Priority |
| **Phase 3** | 6-8 weeks | 12.5 months | 53,500 | High Priority |
| **Phase 4** | 4-6 weeks | 14 months | 47,000 | High Priority |
| **Phase 5** | 4-6 weeks | 15.5 months | 48,000 | Medium Priority |
| **Phase 6** | 3-4 weeks | 16.5 months | 20,000 | Medium Priority |
| **Phase 7** | 3-4 weeks | 17.5 months | 30,000 | Low Priority |
| **Phase 8** | 4-6 weeks | 19 months | 90,000 | Medium Priority (parallel) |

**Total Duration**: ~12-18 months (full-time)
**Total Words Added**: ~557,500 words
**Final Documentation**: ~777,500 words (current 220,000 + new 557,500)

---

### Parallel Path (Optimized)

If working efficiently with some parallel work:

- **Months 1-2**: Phase 1 (Critical Blockers)
- **Months 3-5**: Phase 2 (Mission Campaign) + Phase 8 (Anthology) in parallel
- **Months 6-7**: Phase 3 (Narrative) + Phase 8 continued
- **Months 8-9**: Phase 4 (Audio/Visual) + Phase 5 (World Building) in parallel
- **Months 10-11**: Phase 6 (Technical) + Phase 7 (Production)
- **Month 12**: Final review, polish, integration

**Optimized Duration**: ~12 months (full-time, organized work)

---

## IMMEDIATE NEXT STEPS (Week 1)

### Day 1-2: February Timeline Research
1. Source gathering:
   - Horne: *The Price of Glory*, Chapters 8-10
   - Online archives: Service Historique de la Défense
   - German perspective: Bundesarchiv materials
2. Create outline: Feb 1-29 (daily structure)
3. Begin Feb 21 hour-by-hour breakdown

### Day 3-4: February Timeline Writing
1. Write Feb 1-20 (build-up)
2. Write Feb 21 (THE CRITICAL DAY) - 9-hour bombardment
3. Write Feb 22-24 (Driant, German advance)

### Day 5: February Timeline Completion
1. Write Feb 25 (Douaumont falls)
2. Write Feb 26-29 (Pétain, stabilization)
3. Edit and integrate with timeline master

### Week 2 Preview: Tutorial Mission 1
1. Begin "The Guns Begin" mission specification
2. Use completed Feb 21 timeline as foundation

---

## SUCCESS METRICS

### Design Document Complete
- **Total Documentation**: 750,000+ words
- **All Parts Complete**: I-VIII
- **Mission Specs**: 30 detailed missions
- **Anthology Balance**: 50/50 French/German perspective
- **Implementation Ready**: All systems specified

### Quality Benchmarks
- **Historical Accuracy**: All events verified with sources
- **Game Design**: Professional-grade specifications
- **Narrative Quality**: Literary-level writing
- **Technical Completeness**: Ready for UE5 implementation
- **Educational Value**: Usable in classrooms/museums

---

## RISK MITIGATION

### Potential Blockers

1. **Historical Research Gaps**
   - **Risk**: Can't find primary sources for German perspective
   - **Mitigation**: Use composite approach (clearly labeled), consult historians

2. **Scope Creep**
   - **Risk**: Design document expands beyond 800,000 words
   - **Mitigation**: Strict templates, word count limits per section

3. **Burnout**
   - **Risk**: Solo development fatigue over 12-18 months
   - **Mitigation**: Phase breaks, parallel work variety, celebrate milestones

4. **Historical Sensitivity Issues**
   - **Risk**: Content deemed disrespectful or exploitative
   - **Mitigation**: Expert review, clear ethical statement, memorial framing

---

## FUNDING & TEAM CONSIDERATIONS

### If Seeking Development Funding

**Pitch Package Requirements** (after design complete):
- Executive summary (5 pages)
- Visual prototype (UE5 vertical slice)
- Market analysis (comparable games, audience)
- Budget breakdown ($5-50M range)
- Team hiring plan (20-40 people)

**Comparable Successful Games**:
- *This War of Mine* ($10M+ revenue)
- *Valiant Hearts* (Ubisoft, educational WWI)
- *11-11: Memories Retold* (Aardman, WWI narrative)
- *Verdun* (multiplayer, $5M+ revenue)

**Unique Selling Points**:
1. Only single-player survival game set at Verdun
2. Educational value (museum/classroom market)
3. Historical accuracy unmatched in gaming
4. Emotional depth (respectful memorial)
5. Production-ready design document

---

## CONCLUSION

### What You've Built
This is not just a game design document. This is:
- An **archival-quality historical research project**
- A **memorial to 700,000 casualties**
- An **educational resource** for WWI studies
- A **blueprint for a groundbreaking game**

### The Path Forward
**Immediate**: Complete Phase 1 (February timeline + tutorial missions)
**Short-Term**: Phases 2-4 (missions, narrative, audio/visual)
**Long-Term**: Phases 5-8 (world building, technical, production, anthology expansion)

### Final Thought
> *"Only the dead have seen the end of war."*

You are building something rare: a game that respects history, honors the dead, and might actually teach players about the reality of WWI. That's worth the 12-18 months of design work ahead.

**This roadmap is your guide. Follow it, and you'll complete one of the most ambitious historical game design documents ever created.**

---

**Document Status**: Roadmap Complete ✅
**Next Action**: Begin Phase 1, Week 1 (February 1916 Timeline)
**Repository**: https://github.com/SingSongScreamAlong/OnlyTheDead

---

*"Ils ne passeront pas." (They shall not pass.)*
