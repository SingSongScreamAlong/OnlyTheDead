# ARTILLERY SOUND SPECIFICATIONS - PRIORITY CATEGORY

**Project**: Only The Dead - Battle of Verdun 1916
**Category**: Artillery Audio (CRITICAL)
**Priority**: HIGHEST - Artillery is the primary audio experience
**Historical Context**: 70% of Verdun casualties were from artillery

---

## ⚠️ CRITICAL IMPORTANCE

**Artillery is not just another weapon category - it IS the game.**

- **70% of casualties** at Verdun were from artillery
- **Primary survival mechanic** = recognizing and reacting to shell whistles
- **Constant presence** = artillery sounds are heard 80% of gameplay time
- **Psychological impact** = most terrifying aspect of the experience
- **Educational value** = Players learn why artillery dominated WWI

**Audio Design Priority**: Artillery > All other sounds combined

---

## 🎯 ARTILLERY CALIBERS AT VERDUN

### FRENCH ARTILLERY

#### 1. **75mm M1897 "Soixante-Quinze"** (PRIMARY)
- **Type**: Rapid-fire field gun
- **Role**: French iconic weapon, direct fire, anti-infantry
- **Rate**: 15-20 rounds per minute (extremely fast for era)
- **Range**: 8.5 km effective
- **Sound characteristics**:
  - Sharp, high-pitched report
  - Fast-cycling recoil system (distinctive)
  - Rapid succession possible
  - Lighter shell whistle (higher pitch)

#### 2. **155mm Schneider C M1917**
- **Type**: Heavy howitzer
- **Role**: Counter-battery, fortification attack
- **Rate**: 2-3 rounds per minute
- **Range**: 11.3 km
- **Sound characteristics**:
  - Deep firing boom
  - Heavier shell whistle
  - Massive explosion on impact

#### 3. **220mm Mortier Schneider**
- **Type**: Heavy mortar/howitzer
- **Role**: Fort destruction, deep trenches
- **Rate**: Slow (1-2 per minute)
- **Range**: 10.7 km
- **Sound characteristics**:
  - Very deep firing thud
  - Long arcing trajectory whistle
  - Enormous explosion

#### 4. **58mm T N°2 Trench Mortar**
- **Type**: Light trench mortar
- **Role**: Close support, trench-to-trench
- **Rate**: Variable
- **Range**: 500m
- **Sound characteristics**:
  - Distinctive "thump" launch
  - High-arc, short whistle
  - Explosion in confined trench space

#### 5. **370mm Mortier Filloux**
- **Type**: Super-heavy railway gun
- **Role**: Strategic bombardment, fort destruction
- **Rate**: Very slow
- **Range**: 15 km
- **Sound characteristics**:
  - Earth-shaking firing boom
  - Long, terrifying incoming whistle
  - Catastrophic explosion

---

### GERMAN ARTILLERY

#### 1. **77mm FK 16** (PRIMARY)
- **Type**: Field gun
- **Role**: Standard German field artillery
- **Rate**: 10-15 rounds per minute
- **Range**: 9 km
- **Sound characteristics**:
  - Similar to French 75mm
  - Sharp crack
  - High-pitched whistle

#### 2. **105mm leFH 16**
- **Type**: Light field howitzer
- **Role**: General bombardment
- **Rate**: 4-6 rounds per minute
- **Range**: 9.5 km
- **Sound characteristics**:
  - Medium boom
  - Mid-range whistle
  - Significant explosion

#### 3. **150mm sFH 13**
- **Type**: Heavy field howitzer
- **Role**: Heavy bombardment, trenches
- **Rate**: 2-4 rounds per minute
- **Range**: 8.5 km
- **Sound characteristics**:
  - Heavy boom
  - Deep whistle
  - Large explosion

#### 4. **210mm Mörser**
- **Type**: Heavy howitzer/mortar
- **Role**: Fort attack, deep trenches
- **Rate**: 1-2 rounds per minute
- **Range**: 11 km
- **Sound characteristics**:
  - Very deep boom
  - Ominous long whistle
  - Massive explosion

#### 5. **305mm Beta-Gerät (Kurze Marinekanone)**
- **Type**: Coastal artillery adapted for land
- **Role**: Fort destruction at Verdun
- **Rate**: Very slow
- **Range**: 15 km
- **Sound characteristics**:
  - Thunderous firing boom
  - Long, terrifying whistle
  - Enormous explosion

#### 6. **420mm "Big Bertha" (M-Gerät)** (ICONIC)
- **Type**: Super-heavy howitzer
- **Role**: Fort destruction (used at Verdun early)
- **Rate**: 1 round per ~10 minutes
- **Range**: 9.3 km
- **Sound characteristics**:
  - **EARTH-SHAKING firing boom** (heard kilometers away)
  - **EXTREMELY long, terrifying incoming whistle**
  - **CATASTROPHIC explosion** (crater 10m deep, 20m wide)
  - **Shockwave effect** (felt far from impact)
  - **Psychological terror** (soldiers could hear it coming for seconds)

---

## 🎧 ARTILLERY AUDIO STAGES

Every artillery shell goes through multiple audio stages:

### STAGE 1: FIRING (Sometimes Heard)
**Player Distance from Gun**: 0.5-2 km to hear clearly

**Characteristics**:
- Muzzle blast (initial bang)
- Propellant explosion
- Recoil mechanism (if close enough)
- Echo across battlefield

**Variations Needed**:
- Per caliber (75mm through 420mm)
- 3-5 variations each
- Distance variants (near/medium/far)

**Files Needed**:
```
ARTY_FIRE_75MM_NEAR_01.wav (through _05.wav)
ARTY_FIRE_75MM_MED_01.wav (through _03.wav)
ARTY_FIRE_75MM_FAR_01.wav (through _03.wav)
[Repeat for each caliber: 105mm, 150mm, 155mm, 210mm, 305mm, 420mm]
```

**Total Firing Sounds**: 7 calibers × 3 distances × 4 variations = ~84 files

---

### STAGE 2: INCOMING WHISTLE (CRITICAL GAMEPLAY)
**This is THE most important sound in the game**

**Characteristics**:
- Doppler effect (pitch changes as shell approaches)
- Volume increases dramatically
- Pitch correlates to caliber
- Duration correlates to distance
- Terror increases with duration

**Gameplay Function**:
- **Primary warning system** - Player must recognize and react
- **Caliber identification** - Experienced players learn to judge shell size
- **Direction estimation** - Spatial audio shows trajectory
- **Time to impact** - Longer whistle = more time to take cover

**Whistle Types by Caliber**:

**75-77mm (Light)**
- Pitch: High (1000-2000 Hz range)
- Duration: 0.5-2 seconds
- Characteristics: Fast, sharp, "zipping" quality
- Warning time: MINIMAL (fast shells)

**105-155mm (Medium)**
- Pitch: Medium (500-1000 Hz range)
- Duration: 1-4 seconds
- Characteristics: Clear whistle, moderate warning
- Warning time: 2-3 seconds average

**210mm (Heavy)**
- Pitch: Lower (300-600 Hz range)
- Duration: 3-7 seconds
- Characteristics: Ominous, building terror
- Warning time: 4-5 seconds average

**305mm (Superheavy)**
- Pitch: Deep (200-400 Hz range)
- Duration: 5-10 seconds
- Characteristics: Terrifying, unmistakable
- Warning time: 6-8 seconds

**420mm "Big Bertha" (Ultimate Terror)**
- Pitch: Very deep (100-250 Hz range)
- Duration: 8-15 seconds
- Characteristics: **Nightmare fuel**, everyone hears it coming
- Warning time: 10+ seconds (but may not save you)
- **Psychological effect**: Entire battlefield falls silent listening

**Distance Variants**:
- **Overhead** - Passes over player position (may not hit here)
- **Incoming Close** - Will land nearby (DANGER)
- **Incoming Very Close** - Will land on/near player (CRITICAL DANGER)

**Files Needed**:
```
ARTY_WHISTLE_75MM_OVERHEAD_01.wav (through _10.wav)
ARTY_WHISTLE_75MM_CLOSE_01.wav (through _10.wav)
ARTY_WHISTLE_75MM_VERYCLOSE_01.wav (through _10.wav)

ARTY_WHISTLE_105MM_OVERHEAD_01.wav (through _10.wav)
ARTY_WHISTLE_105MM_CLOSE_01.wav (through _10.wav)
ARTY_WHISTLE_105MM_VERYCLOSE_01.wav (through _10.wav)

ARTY_WHISTLE_155MM_OVERHEAD_01.wav (through _10.wav)
ARTY_WHISTLE_155MM_CLOSE_01.wav (through _10.wav)
ARTY_WHISTLE_155MM_VERYCLOSE_01.wav (through _10.wav)

ARTY_WHISTLE_210MM_OVERHEAD_01.wav (through _10.wav)
ARTY_WHISTLE_210MM_CLOSE_01.wav (through _10.wav)
ARTY_WHISTLE_210MM_VERYCLOSE_01.wav (through _10.wav)

ARTY_WHISTLE_305MM_OVERHEAD_01.wav (through _10.wav)
ARTY_WHISTLE_305MM_CLOSE_01.wav (through _10.wav)
ARTY_WHISTLE_305MM_VERYCLOSE_01.wav (through _10.wav)

ARTY_WHISTLE_420MM_OVERHEAD_01.wav (through _10.wav)
ARTY_WHISTLE_420MM_CLOSE_01.wav (through _10.wav)
ARTY_WHISTLE_420MM_VERYCLOSE_01.wav (through _10.wav)
```

**Total Whistle Sounds**: 6 calibers × 3 distances × 10 variations = 180 files
**PRIORITY**: These 180 files are the HIGHEST priority in the entire audio library

---

### STAGE 3: IMPACT/EXPLOSION (DEATH OR SURVIVAL)

**Characteristics**:
- Initial blast (sharp crack or deep boom)
- Shockwave (pressure wave, depends on proximity)
- Debris/shrapnel (whistling fragments)
- Crater formation (ground impact, dirt spray)
- Echo/reverb (across battlefield)
- Lingering effects (dust settling, debris falling)

**Distance Variants**:

**NEAR (0-5m)** - CRITICAL DAMAGE/DEATH RANGE
- **Sound**: Overwhelming, potentially distorted
- **Effects**: Muffled hearing, tinnitus, ear ringing
- **Game effect**: Audio occlusion, player disorientation
- **Survival**: Unlikely for large calibers

**CLOSE (5-20m)** - DANGER ZONE
- **Sound**: Extremely loud, clear shockwave
- **Effects**: Ear ringing, temporary deafness
- **Debris**: Shrapnel impacts audible
- **Game effect**: Reduced hearing for 10-30 seconds

**MEDIUM (20-50m)** - SAFE(R) ZONE
- **Sound**: Loud boom, clear explosion
- **Effects**: Pressure wave felt
- **Debris**: Some fragments may reach
- **Game effect**: Brief audio ducking

**FAR (50-200m)** - OBSERVATION DISTANCE
- **Sound**: Clear explosion, manageable volume
- **Effects**: Observable, not directly threatening
- **Context**: Barrage elsewhere

**VERY FAR (200m+)** - DISTANT BATTLE
- **Sound**: Dull thump, part of ambient soundscape
- **Effects**: Background noise
- **Context**: Fighting on other sectors

**Files Needed**:
```
ARTY_IMPACT_75MM_NEAR_01.wav (through _08.wav)
ARTY_IMPACT_75MM_CLOSE_01.wav (through _08.wav)
ARTY_IMPACT_75MM_MEDIUM_01.wav (through _06.wav)
ARTY_IMPACT_75MM_FAR_01.wav (through _05.wav)
ARTY_IMPACT_75MM_VERYFAR_01.wav (through _03.wav)

[Repeat for each caliber: 105mm, 155mm, 210mm, 305mm, 420mm]
```

**Special: 420mm "Big Bertha" Impact**
```
ARTY_IMPACT_420MM_NEAR_01.wav (through _05.wav) - Catastrophic
ARTY_IMPACT_420MM_CLOSE_01.wav (through _05.wav) - Earth-shaking
ARTY_IMPACT_420MM_MEDIUM_01.wav (through _05.wav) - Fort-destroying
ARTY_IMPACT_420MM_FAR_01.wav (through _05.wav) - Still terrifying
ARTY_IMPACT_420MM_VERYFAR_01.wav (through _03.wav) - Distant thunder
```

**Total Impact Sounds**: 6 calibers × 5 distances × 6 variations = 180 files

---

### STAGE 4: SHRAPNEL/DEBRIS (Post-Explosion)

**Characteristics**:
- Shell fragments whistling through air
- Debris falling (dirt, wood, metal)
- Secondary impacts (fragments hitting ground/objects)
- Delayed effects (delayed fuse shrapnel)

**Types**:

**Shrapnel Whistles**
- High-velocity fragments (supersonic cracks)
- Tumbling metal (chaotic pitch changes)
- Multiple fragments (layered sounds)

**Debris Impacts**
- Dirt raining down (patter)
- Large debris falling (thuds)
- Metal fragments hitting objects (pings, clangs)

**Files Needed**:
```
ARTY_SHRAPNEL_WHISTLE_CLOSE_01.wav (through _10.wav)
ARTY_SHRAPNEL_WHISTLE_PASS_01.wav (through _10.wav)
ARTY_SHRAPNEL_IMPACT_GROUND_01.wav (through _08.wav)
ARTY_SHRAPNEL_IMPACT_METAL_01.wav (through _08.wav)
ARTY_SHRAPNEL_IMPACT_WOOD_01.wav (through _08.wav)

ARTY_DEBRIS_DIRT_RAIN_01.wav (through _05.wav)
ARTY_DEBRIS_LARGE_FALL_01.wav (through _05.wav)
ARTY_DEBRIS_SETTLING_01.wav (through _05.wav)
```

**Total Debris Sounds**: ~60 files

---

### STAGE 5: AFTERMATH (Psychological)

**Characteristics**:
- Tinnitus (ear ringing) after close calls
- Muffled hearing (temporary deafness)
- Dust/smoke acoustics (sound dampening)
- Echo decay
- Silence (eerie after bombardment)

**Game Effects**:

**Tinnitus System**
- Triggered by near misses
- High-pitched ringing (8000-12000 Hz)
- Fades over 10-60 seconds
- Intensity based on proximity

**Hearing Damage**
- Temporary frequency filtering
- Muffled dialogue
- Reduced spatial awareness
- Gradual recovery

**Files Needed**:
```
ARTY_AFTERMATH_TINNITUS_MILD_01.wav
ARTY_AFTERMATH_TINNITUS_MODERATE_01.wav
ARTY_AFTERMATH_TINNITUS_SEVERE_01.wav
ARTY_AFTERMATH_MUFFLE_LIGHT_01.wav (audio filter)
ARTY_AFTERMATH_MUFFLE_HEAVY_01.wav (audio filter)
```

**Total Aftermath Sounds**: ~10 files

---

## 🎯 BARRAGE PATTERNS

Artillery rarely fires single shots - barrages are coordinated patterns.

### 1. **Preparatory Barrage** (Pre-Attack)
**Characteristics**:
- Hours or days of sustained fire
- Rhythmic, predictable pattern
- Multiple calibers firing together
- Intended to destroy defenses

**Audio Design**:
- Overlapping shell whistles (constant)
- Regular explosion pattern
- No silence (relentless)
- Mix of calibers

**Files Needed**:
```
ARTY_BARRAGE_PREPARATORY_LIGHT_LOOP.wav (2-3 min loop)
ARTY_BARRAGE_PREPARATORY_MEDIUM_LOOP.wav
ARTY_BARRAGE_PREPARATORY_HEAVY_LOOP.wav
ARTY_BARRAGE_PREPARATORY_INTENSE_LOOP.wav
```

---

### 2. **Rolling Barrage** (Moving Wall of Fire)
**Characteristics**:
- Artillery fire moving forward
- Synchronized with infantry advance
- Shells land progressively closer/farther
- Precisely timed (soldiers follow the barrage)

**Audio Design**:
- Explosions moving spatially
- Predictable timing (every 30-60 seconds advance)
- Consistent caliber (usually 75mm)
- Clear direction of movement

**Files Needed**:
```
ARTY_BARRAGE_ROLLING_APPROACHING.wav
ARTY_BARRAGE_ROLLING_OVERHEAD.wav
ARTY_BARRAGE_ROLLING_DEPARTING.wav
```

---

### 3. **Harassing Fire** (Random Terror)
**Characteristics**:
- **MOST TERRIFYING FOR SOLDIERS**
- Unpredictable timing
- Random targets
- Psychological warfare
- No pattern to predict

**Audio Design**:
- Irregular intervals (15 seconds to 5 minutes)
- Variable calibers
- Keeps players on edge
- No safe rhythm

**Files Needed**:
```
ARTY_HARASS_SINGLE_DISTANT.wav (ambient, random trigger)
ARTY_HARASS_SINGLE_NEAR.wav (danger, random trigger)
ARTY_HARASS_SEQUENCE_2_3.wav (short sequence)
```

---

### 4. **Box Barrage** (Encirclement)
**Characteristics**:
- Shells land in square pattern
- Traps target area
- Prevents reinforcement/retreat
- Concentrated fire

**Audio Design**:
- Explosions surrounding player
- All directions threatened
- No escape route
- Claustrophobic audio

**Files Needed**:
```
ARTY_BARRAGE_BOX_SURROUND_LOOP.wav
ARTY_BARRAGE_BOX_EDGE.wav (player near perimeter)
ARTY_BARRAGE_BOX_CENTER.wav (player in kill zone)
```

---

### 5. **Counter-Battery Fire** (Artillery Duel)
**Characteristics**:
- Artillery targeting enemy artillery
- Often reciprocal
- Heavy calibers
- Overhead trajectories (may pass over player)

**Audio Design**:
- Shells passing overhead (whistles without impact nearby)
- Distant explosions
- May trigger response fire (enemy shooting back)
- Background to other action

**Files Needed**:
```
ARTY_COUNTERBATTERY_OVERHEAD_LOOP.wav
ARTY_COUNTERBATTERY_DISTANT_IMPACTS.wav
```

---

## 📊 TOTAL ARTILLERY SOUND COUNT

| Stage | Types | Variations | Total Files |
|-------|-------|------------|-------------|
| Firing | 7 calibers × 3 distances | 4 each | 84 |
| **Whistles** | **6 calibers × 3 proximities** | **10 each** | **180** |
| **Impacts** | **6 calibers × 5 distances** | **6 each** | **180** |
| Shrapnel | 8 types | 8 avg | 64 |
| Aftermath | 5 types | 2 avg | 10 |
| Barrages | 12 patterns | 1 each | 12 |
| **TOTAL** | **~100 types** | **Varies** | **~530 files** |

**Storage Estimate**:
- 530 files × 48kHz × 24-bit × avg 5 seconds = ~3-4 GB uncompressed
- Compressed in-engine: ~800 MB - 1.2 GB

**Priority Files** (Minimum Viable Product):
- **80 whistle files** (top 4 calibers, 2 distances, 10 variations)
- **80 impact files** (top 4 calibers, 3 distances, 6 variations)
- **20 barrage loops**
- **TOTAL MVP**: ~180 files

---

## 🎤 RECORDING REAL ARTILLERY (If Possible)

### Sources for Authentic Recordings:

1. **Military Exercises**
   - Some armies conduct live-fire exercises
   - Recording permissions may be available
   - Modern artillery sounds different (warning)

2. **Historical Reenactments**
   - WWI reenactors with period artillery
   - Authentic weapons fired (blank or live)
   - Imperial War Museum (UK) does demonstrations

3. **Museums with Live Demonstrations**
   - Musée de l'Armée (Paris)
   - Imperial War Museum (London)
   - US Army Artillery Museum (Fort Sill)

4. **Historical Audio Archives**
   - Imperial War Museum Sound Archive
   - Library of Congress
   - French military archives (SHD)
   - German Bundesarchiv

### Challenges:
- WWI artillery no longer commonly fired
- Smokeless powder sounds different than modern propellant
- Shell velocities differ (modern faster = different whistle)
- Safe recording distances vs. authentic close sounds

### Solutions:
- **Professional sound libraries** (BOOM Library, A Sound Effect)
- **Synthesis** (create whistles from scratch using audio design)
- **Layering** (combine multiple sources for authentic effect)
- **Film sound** (WWI movies may have reference audio)

---

## 🔊 SOUND DESIGN TECHNIQUES

### Creating Authentic Whistles

**Method 1: Synthesis**
1. Start with sine wave generator
2. Apply Doppler pitch shift (increases over time)
3. Add harmonics based on caliber
4. Layer with wind noise (shell turbulence)
5. Apply distance filtering

**Method 2: Recording**
1. Bottle rockets / fireworks (scaled down)
2. Arrows/projectiles (small scale reference)
3. Pitch shift down for larger calibers
4. Time stretch for longer trajectories

**Method 3: Sampling**
1. Use existing artillery recordings
2. Isolate whistle portion
3. Pitch shift for different calibers
4. Time stretch for various distances
5. Layer multiple takes

### Creating Authentic Impacts

**Method 1: Layering**
1. **Core explosion** - Deep bass thump
2. **Initial crack** - Sharp transient
3. **Air displacement** - Whoosh/rumble
4. **Debris** - Dirt/metal impacts
5. **Echo** - Environmental reverb

**Method 2: Real Explosions**
1. Controlled demolitions (construction sites)
2. Quarry blasts (available on Freesound)
3. Movie sound libraries
4. Fireworks (large mortars)
5. Thunder (natural explosions)

**Processing**:
- Heavy bass emphasis (20-200 Hz)
- Sharp transient attack
- Long decay (3-10 seconds)
- Environmental reverb
- Distance filtering (low-pass for far sounds)

---

## 🎧 SPATIAL AUDIO IMPLEMENTATION

### 3D Audio Requirements (Unreal Engine 5)

**Spatialization**:
- Full 3D positioning (azimuth + elevation)
- Distance attenuation curves
- Doppler effect for whistles
- Occlusion (terrain/structures blocking sound)
- Reverb zones (trenches vs. open field)

**Shell Trajectory Audio**:
1. Spawn audio source at shell position
2. Attach whistle sound to moving projectile
3. Update 3D position every frame
4. Trigger impact sound at impact location
5. Spawn shrapnel sounds radiating from impact

**Player Hearing Damage**:
- Dynamic EQ (frequency filtering)
- Tinnitus layer (high-frequency ring)
- Volume ducking (temporary deafness)
- Recovery over time

---

## 🎯 GAMEPLAY INTEGRATION

### Artillery as Core Mechanic

**Player Learning Curve**:
1. **First hour**: All shells sound terrifying
2. **5 hours**: Player learns to distinguish calibers
3. **10 hours**: Player accurately judges impact location
4. **20+ hours**: Player becomes "veteran" - calm under fire

**Audio Cues for Survival**:
- **Whistle pitch** = Shell size
- **Whistle duration** = Distance
- **Whistle direction** = Impact location
- **Volume increase rate** = Danger level

**Tutorial Elements**:
- Off-map shells (safe learning)
- Caliber introduction (learn each size)
- Close calls (teach survival)
- Barrage survival (apply skills)

---

## 📚 REFERENCES

### Historical Sources
- "Artillery of the Verdun Battle" (French Military Archives)
- "German Artillery at Verdun" (Bundesarchiv documents)
- Imperial War Museum Sound Archive
- "The Guns of August" (audio documentary)

### Technical References
- "Artillery Sounds: Physics and Acoustics"
- "Shell Trajectory and Doppler Effect"
- WWI artillery firing tables (for authentic trajectories)

### Sound Design References
- BOOM Library "World War I Firearms"
- *Battlefield 1* sound design talks (GDC)
- *Saving Private Ryan* artillery sound design
- *Apocalypse Now* explosive sound layering

### Historical Audio
- BBC WW1 Sound Archive
- "Sounds of War" (historical compilation)
- French newsreel audio (1916)
- German military recordings

---

## ✅ PRIORITY CHECKLIST

### Phase 1: Minimum Viable Product
- [ ] 75mm whistle (10 variations, 2 distances)
- [ ] 75mm impact (6 variations, 3 distances)
- [ ] 155mm whistle (10 variations, 2 distances)
- [ ] 155mm impact (6 variations, 3 distances)
- [ ] 210mm whistle (10 variations, 2 distances)
- [ ] 210mm impact (6 variations, 3 distances)
- [ ] Harassing fire ambience loop
- [ ] Tinnitus effect (3 intensities)

**MVP Total**: ~90 files

### Phase 2: Full Arsenal
- [ ] Complete all calibers (75mm through 420mm)
- [ ] All distance variants (5 per caliber)
- [ ] Shrapnel sounds
- [ ] Debris sounds
- [ ] Barrage patterns
- [ ] Aftermath effects

**Full Total**: ~530 files

### Phase 3: Polish
- [ ] Additional variations (15+ per sound)
- [ ] Seasonal variations (snow dampening, rain effects)
- [ ] Environmental variations (trenches, forts, open field)
- [ ] Dynamic mixing system
- [ ] Shell shock simulation

---

**Document Version**: 1.0
**Last Updated**: November 10, 2025
**Status**: Artillery Audio Specification - Complete
**Next Steps**: Begin sourcing/creating MVP artillery sounds
