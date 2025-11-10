# ONLY THE DEAD - Audio Assets Library

**Project**: Only The Dead - WWI Survival Game (Battle of Verdun 1916)
**Purpose**: Comprehensive sound library for historically accurate WWI audio
**Era**: 1916, Western Front, Battle of Verdun

---

## 📁 DIRECTORY STRUCTURE

```
audio/
├── weapons/                    # Period-specific WWI weapons
│   ├── rifles/                 # Bolt-action rifles (primary infantry weapon)
│   ├── pistols/               # Officers' sidearms
│   ├── machine_guns/          # Heavy and light machine guns
│   ├── artillery/             # THE primary killer (70% of casualties)
│   ├── grenades/              # Hand grenades and rifle grenades
│   └── mechanical/            # Weapon mechanisms (bolt action, loading)
├── explosions/                # Impact and detonation sounds
│   ├── artillery_impacts/     # Shell impacts (various calibers)
│   ├── grenades/              # Grenade explosions
│   └── demolitions/           # Mines, saps, demolition charges
├── ambient/                   # Atmospheric soundscapes
│   ├── battlefield/           # Distant battle sounds
│   ├── environmental/         # Trenches, dugouts atmosphere
│   └── nature/                # Wind, birds (when present)
├── human/                     # Human sounds
│   ├── voices/                # French/German voices, commands, cries
│   ├── breathing/             # Exertion, fear, exhaustion
│   ├── pain/                  # Wounds, suffering (respectful)
│   └── movement/              # Human movement sounds
├── equipment/                 # Military equipment
│   ├── gas_masks/             # Canister filters, breathing through mask
│   ├── tools/                 # Shovels, wire cutters, picks
│   ├── loading/               # Ammunition, supplies handling
│   └── clothing/              # Uniform movement, equipment rattle
├── environmental/             # Natural world sounds
│   ├── weather/               # Rain, snow, wind, thunder
│   ├── footsteps/             # Mud, duckboards, stone, debris
│   └── terrain/               # Mud sounds, water, debris shifting
├── structures/                # Man-made structures
│   ├── trenches/              # Trench environment sounds
│   ├── dugouts/               # Underground shelter sounds
│   └── fortifications/        # Fort interiors, metal doors, concrete
├── vehicles/                  # Transport and machines
│   ├── trucks/                # Supply trucks (limited use)
│   ├── horses/                # Pack animals, cavalry (rare)
│   └── aircraft/              # Reconnaissance planes, observation balloons
└── ui/                        # User interface sounds
    ├── menu/                  # Menu navigation
    ├── alerts/                # Warnings, notifications
    └── objectives/            # Mission markers

```

---

## 🎯 PRIORITY CATEGORIES

### CRITICAL (Required for core gameplay)
1. **Artillery** - 75mm, 105mm, 155mm, 210mm, 305mm, 420mm shells
2. **Weather** - Rain (constant at Verdun), mud sounds
3. **Footsteps** - Mud, duckboards, debris
4. **Ambient** - Distant battle, trench atmosphere
5. **Human** - Breathing, exertion, French voices

### HIGH (Essential for authenticity)
6. **Rifles** - Lebel 1886, Berthier, Mauser 98
7. **Explosions** - Artillery impacts (various sizes)
8. **Environment** - Wind, mud, rats, flies
9. **Equipment** - Gas masks, shovels, wire cutters
10. **Grenades** - F1 "pineapple", stick grenades

### MEDIUM (Important for immersion)
11. **Machine guns** - Hotchkiss M1914, MG08, Chauchat
12. **Structures** - Trench collapses, dugout creaks
13. **Vehicles** - Trucks, horses, aircraft distant
14. **Pain/Wounds** - Respectful, not gratuitous
15. **Tools** - Digging, cutting wire, hammering stakes

### LOW (Polish and detail)
16. **UI sounds** - Menu navigation, alerts
17. **Pistols** - Ruby, Mauser C96 (officers only)
18. **Nature** - Birds (rare), wind variations
19. **Clothing** - Equipment rattle, uniform rustle
20. **Mechanical** - Bolt actions, loading clips

---

## 🔫 WWI WEAPONS REFERENCE

### FRENCH WEAPONS (Primary Player Arsenal)

#### Rifles
- **Lebel M1886/93** - 8mm tubular magazine, distinctive metallic action
  - Bolt cycling sound (distinct clank)
  - Single-load tubular magazine (slow)
  - Firing sound (smokeless powder era)
  - Empty chamber click

- **Berthier M1907/15** - 3-round en-bloc clip
  - Bolt action (smoother than Lebel)
  - Clip insertion/ejection (metallic ping)
  - Firing sound

#### Machine Guns
- **Hotchkiss M1914** - 8mm, gas-operated, strip-fed
  - Sustained firing (240-450 rpm)
  - Strip insertion (metallic rattle)
  - Gas vent hiss
  - Overheating sizzle

- **Chauchat M1915** - Light machine gun
  - Firing sound (long recoil, distinctive clatter)
  - Magazine insertion (20-round half-moon)
  - Prone to jamming (mechanism sounds)

#### Pistols (Officers)
- **Ruby pistol** - 7.65mm, semi-auto
- **Modèle 1892 revolver** - 8mm

#### Grenades
- **F1 "Pineapple" grenade** - Segmented body
  - Pin pull
  - Lever release
  - Throw whoosh
  - Explosion (5-second fuse)

#### Artillery (French)
- **75mm M1897** - "Soixante-Quinze" rapid-fire field gun
  - Iconic French weapon
  - Shell whistling (incoming)
  - Impact/explosion
  - Recoil mechanism (hydro-pneumatic)

- **155mm Schneider C** - Heavy howitzer
  - Deep firing sound
  - Heavier shell whistle
  - Massive explosion

- **Mortier de 58mm T N°2** - Trench mortar
  - Firing thump
  - High-arc trajectory whistle
  - Explosion in trenches

### GERMAN WEAPONS (Enemy Arsenal)

#### Rifles
- **Mauser Gewehr 98** - 7.92mm, 5-round internal magazine
  - Bolt action (smooth, precise)
  - Stripper clip loading
  - Firing sound

#### Machine Guns
- **MG08** - Based on Maxim gun, water-cooled
  - Sustained firing (500 rpm)
  - Belt feed rattle
  - Water cooling bubbling (when overheated)

- **MG08/15** - Light machine gun version
  - More portable, similar sound to MG08
  - Distinct from French Chauchat

#### Pistols
- **Luger P08** - 9mm, toggle-action
- **Mauser C96** - "Broomhandle" semi-auto

#### Grenades
- **Stielhandgranate** - "Stick grenade"/"Potato masher"
  - Pull-cord ignition
  - Throw whoosh
  - Explosion (5-second fuse, powerful blast)

#### Artillery (German)
- **77mm FK 16** - Field gun
- **105mm leFH 16** - Light field howitzer
- **150mm sFH 13** - Heavy field howitzer
- **210mm Mörser** - Heavy howitzer
- **305mm Beta-Gerät** - Coastal artillery adapted for Verdun
- **420mm "Big Bertha"** - Massive siege howitzer
  - Deep, earth-shaking firing sound
  - Long incoming whistle (terrifying)
  - Catastrophic explosion
  - Shockwave effects

---

## 💥 ARTILLERY SOUND DESIGN

**CRITICAL NOTE**: Artillery is the PRIMARY audio focus. 70% of Verdun casualties were from shells.

### Shell Types by Caliber

#### Light (75-77mm)
- **Whistle**: High-pitched, fast
- **Impact**: Sharp crack, smaller blast radius
- **Frequency**: Most common, rapid fire

#### Medium (105-155mm)
- **Whistle**: Mid-range pitch, moderate speed
- **Impact**: Heavy explosion, significant shockwave
- **Frequency**: Common barrage weapon

#### Heavy (210-305mm)
- **Whistle**: Deep, slower, ominous
- **Impact**: Massive explosion, crater-forming
- **Frequency**: Less common, devastating

#### Superheavy (420mm "Big Bertha")
- **Whistle**: Very deep, long duration, terrifying
- **Impact**: Catastrophic, fort-destroying
- **Frequency**: Rare, but memorable

### Shell Audio Stages
1. **Distant firing** (sometimes audible)
2. **Incoming whistle** (player warning - CRITICAL gameplay mechanic)
3. **Impact/Explosion** (varies by proximity to player)
4. **Debris/Shrapnel** (whistling fragments)
5. **Echo/Reverb** (battlefield acoustics)
6. **Aftermath** (ringing ears, muffled sound if close)

### Barrage Patterns
- **Preparatory barrage** - Rhythmic, sustained
- **Rolling barrage** - Moving wall of fire
- **Creeping barrage** - Slow advance with infantry
- **Harassing fire** - Random, unpredictable (most terrifying)
- **Box barrage** - Concentrated area saturation

---

## 🌧️ ENVIRONMENTAL SOUNDS

### Weather (Verdun-Specific)
- **Rain** - Near-constant feature
  - Light rain (patter on helmet)
  - Heavy rain (drumming on surfaces)
  - Rain on mud (splashing, pooling)
  - Rain in dugouts (dripping through timbers)

- **Wind**
  - Howling across no-man's land
  - Whistling through destroyed forests
  - Flapping tarps/canvas
  - Wind through barbed wire (eerie whistle)

- **Thunder** - Natural vs. artillery distinction

- **Snow** - February/March 1916
  - Crunching footsteps
  - Wind-driven snow
  - Muffled battlefield sounds

### Mud (Defining Feature)
- **Footsteps** - Deep, sucking sounds
- **Mud sliding** - Trench walls collapsing
- **Extraction** - Pulling boots from mud
- **Splashing** - Water-filled shell holes

### Battlefield Ambience
- **Distant battle** - Constant low rumble
- **Nearby fighting** - Sporadic gunfire, explosions
- **Night sounds** - Reduced activity, eerie quiet
- **Gas alarm** - Klaxons, bells, shouting

---

## 🎙️ HUMAN SOUNDS

### French Voices (Primary)
**CRITICAL**: All French dialogue must be authentic, period-appropriate French with regional accents (many Verdun defenders were from southern France).

#### Commands (Military)
- "En avant!" (Forward!)
- "À couvert!" (Take cover!)
- "Gaz! Gaz!" (Gas! Gas!)
- "Barrage!" (Barrage!)
- "Medic!" (French: "Brancardier!")
- "Ils ne passeront pas!" (They shall not pass!)

#### Survival/Distress
- Labored breathing
- Coughing (gas, cold, illness)
- Crying out (wounds - respectful, not gratuitous)
- Exhaustion gasps
- Shivering sounds

#### Human Interaction
- Quiet conversation (trench camaraderie)
- Prayers (Catholic - common among French soldiers)
- Singing (rare moments of normalcy)
- Dark humor/gallows humor

### German Voices (Enemy)
**IMPORTANT**: German soldiers are not caricatures - treat with same respect.

#### Commands
- "Feuer!" (Fire!)
- "Deckung!" (Cover!)
- "Gasangriff!" (Gas attack!)
- "Sturm!" (Assault!)
- "Sanitäter!" (Medic!)

---

## 🎧 TECHNICAL SPECIFICATIONS

### Format Requirements
- **Format**: WAV (uncompressed) or FLAC (lossless)
- **Sample Rate**: 48kHz (game industry standard)
- **Bit Depth**: 24-bit (professional quality)
- **Channels**:
  - Mono for point sources (weapons, footsteps)
  - Stereo for ambience
  - 3D spatial audio support (Unreal Engine 5)

### Naming Convention
```
[CATEGORY]_[SUBCATEGORY]_[SPECIFIC]_[VARIATION]_[DISTANCE/TYPE].wav

Examples:
WPN_RIFLE_LEBEL_FIRE_01.wav
WPN_RIFLE_LEBEL_BOLT_CYCLE_02.wav
EXP_ARTY_155MM_IMPACT_NEAR_03.wav
EXP_ARTY_155MM_WHISTLE_INCOMING_01.wav
AMB_BATTLEFIELD_DISTANT_BATTLE_01.wav
ENV_WEATHER_RAIN_HEAVY_01.wav
ENV_FOOTSTEPS_MUD_WALK_01.wav
HUM_BREATH_EXERTION_HEAVY_02.wav
```

### Variations Needed
- **Minimum**: 3-5 variations per sound (avoid repetition fatigue)
- **Weapons**: 5-10 variations (frequently heard)
- **Artillery**: 10+ variations (constant feature)
- **Footsteps**: 8-12 variations (continuous)
- **Ambient**: Long-duration loops (2-5 minutes)

---

## 📚 SOUND SOURCES & LICENSES

### Free/Royalty-Free Sources

#### 1. **Pixabay** (pixabay.com/sound-effects/)
- **License**: Royalty-free, no attribution required
- **Content**: Growing WWI collection
- **Format**: MP3/WAV downloads
- **Cost**: FREE

#### 2. **Freesound.org**
- **License**: Creative Commons (various), Public Domain
- **Content**: User-uploaded, massive library
- **Format**: Various (often WAV)
- **Cost**: FREE (attribution required for some)

#### 3. **BBC Sound Effects Archive** (via Archive.org)
- **License**: RemArc License (personal, educational, research use)
- **Content**: 33,000+ effects, historical recordings from 1920s+
- **Format**: WAV (high quality)
- **Cost**: FREE (non-commercial)
- **Note**: Includes WWII recordings, some adaptable to WWI

#### 4. **Sound-Fishing.eu**
- **License**: Various (check per sound)
- **Content**: Battlefield atmospheres, period sounds
- **Format**: WAV
- **Cost**: Mix of free and paid

#### 5. **SoundBible.com**
- **License**: Public Domain, Creative Commons
- **Content**: World War sounds collection
- **Format**: WAV/MP3
- **Cost**: FREE

### Professional/Paid Libraries (If Budget Available)

#### 1. **BOOM Library - "World War I Firearms"**
- **Price**: ~$399 (estimate)
- **Content**: Authentic WWI rifles, revolvers, shotguns
- **Recording**: Real WWI weapons, outdoor recordings
- **Details**: Full mechanical reality (cocking, reloading, dryfires, handling)
- **Format**: 24-bit/96kHz
- **Value**: HIGHEST authenticity for weapons

#### 2. **A Sound Effect - "Rifles & Pistols of The World Wars"**
- **Price**: Varies
- **Content**: 587+ sounds from WWI/WWII firearms
- **Format**: 24-bit/96kHz
- **License**: Royalty-free

#### 3. **Soundsnap** (soundsnap.com)
- **Price**: Subscription-based (~$29/month)
- **Content**: 500,000+ effects, WWI category
- **Format**: High quality
- **License**: Subscription-based usage

#### 4. **Pond5** (pond5.com)
- **Price**: Individual sounds ($2+)
- **Content**: 1,112+ WWI sounds
- **Format**: Various
- **License**: Royalty-free per purchase

### Historical Recordings (Public Domain)

#### Imperial War Museum (IWM) Sound Archive
- **Content**: Historical recordings, interviews, battle sounds
- **Era**: Includes WWI material
- **License**: Check per recording (many public domain)
- **Access**: Online archive, download available

#### Library of Congress Sound Archives
- **Content**: Historical recordings including WWI era
- **License**: Many public domain
- **Access**: Free downloads

---

## 🎬 SOUND DESIGN PHILOSOPHY

### Historical Authenticity
- **Prioritize accuracy** over Hollywood dramatization
- **Real weapons sound different** from movies (sharper cracks, not deep booms)
- **Artillery dominates** soundscape (not small arms)
- **Mud sounds** are constant (sucking, splashing, sliding)
- **Silence is terrifying** (use absence of sound for psychological effect)

### Respectful Portrayal
- **Human suffering** - Not gratuitous, but honest
- **Death sounds** - Understated, respectful
- **No glorification** - War sounds horrific, not exciting
- **Cultural sensitivity** - Accurate French/German voices, not stereotypes

### Gameplay Integration
- **Shell whistles** - PRIMARY survival mechanic (warning system)
- **Directional audio** - Essential for player awareness
- **Distance cues** - Help player judge threats
- **Dynamic mixing** - Proximity, obstruction, weather affect sound
- **Psychological effects** - Shell shock = muffled audio, tinnitus

### Immersion Priorities
1. **Artillery** - Constant, varied, terrifying
2. **Weather** - Rain, wind, mud (environmental oppression)
3. **Ambient battle** - Distant fighting (you're never safe)
4. **Footsteps/movement** - Player feedback
5. **Human presence** - Voices, breathing (you're not alone)

---

## 🚀 IMPLEMENTATION ROADMAP

### Phase 1: Core Essentials (Prototype)
- [ ] Artillery (5 calibers, 3 variations each)
- [ ] Lebel rifle (fire, bolt, reload)
- [ ] Rain sounds (3 intensities)
- [ ] Mud footsteps (8 variations)
- [ ] Distant battle ambient (2 loops)
- [ ] Heavy breathing (exertion)
- [ ] French voice commands (10 essential)

### Phase 2: Expanded Arsenal (Alpha)
- [ ] All French weapons (complete sets)
- [ ] German weapons (heard from enemy)
- [ ] Explosions (grenades, impacts, demolitions)
- [ ] Weather variations (wind, thunder, snow)
- [ ] Environmental sounds (rats, flies, debris)
- [ ] Gas masks (breathing, filters)
- [ ] Equipment sounds (tools, loading)

### Phase 3: Full Immersion (Beta)
- [ ] Complete voice sets (French/German)
- [ ] Human sounds (breathing, pain, movement)
- [ ] Structures (trench collapses, dugouts)
- [ ] Vehicles (trucks, horses, aircraft)
- [ ] Advanced ambience (night, fog, seasons)
- [ ] UI sounds (menu, alerts)
- [ ] All variations (3-10 per sound)

### Phase 4: Polish (Gold)
- [ ] Adaptive mixing system
- [ ] Shell shock audio effects
- [ ] Tinnitus simulation
- [ ] Obstruction/occlusion filters
- [ ] Reverb zones (trenches, forts, open)
- [ ] Wind variations (barbed wire whistles)
- [ ] Seasonal variations

---

## 📊 ESTIMATED ASSET COUNT

| Category | Sounds | Variations | Total Files |
|----------|--------|------------|-------------|
| Weapons | 40 | 5 each | 200 |
| Explosions | 25 | 8 each | 200 |
| Artillery | 7 calibers × 3 stages | 10 each | 210 |
| Ambient | 20 loops | 2 each | 40 |
| Human | 50 types | 5 each | 250 |
| Equipment | 30 items | 3 each | 90 |
| Environmental | 40 types | 6 each | 240 |
| Structures | 20 types | 3 each | 60 |
| Vehicles | 10 types | 4 each | 40 |
| UI | 15 types | 2 each | 30 |
| **TOTAL** | **~250 base sounds** | **Avg 4.5** | **~1,360 files** |

**Storage Estimate**:
- 1,360 files × 48kHz × 24-bit × avg 3 seconds = ~10-15 GB uncompressed
- Compressed in-engine: ~3-5 GB

---

## 🎯 NEXT STEPS

1. **Download free sources** - Start with Pixabay, Freesound, BBC Archive
2. **Catalog existing assets** - Document what we have vs. what we need
3. **Create placeholder sounds** - For testing/prototyping
4. **Test in-engine** - Import to UE5, test spatial audio
5. **Identify gaps** - What requires professional libraries?
6. **Budget allocation** - If purchasing (BOOM Library weapons highly recommended)
7. **Record custom sounds** - Mud, specific foley (if possible)
8. **Voice casting** - Native French/German speakers for authenticity

---

## 📖 REFERENCES

### Historical Audio Resources
- Imperial War Museum Sound Archive
- National WWI Museum audio collections
- "The Sounds of War" (historical audio analysis)
- French Military Archives (Service Historique de la Défense)
- German Federal Archives (Bundesarchiv)

### Technical References
- "Designing Sound" by Andy Farnell
- "The Sound Effects Bible" by Ric Viers
- Game Audio Implementation (Unreal Engine 5 documentation)
- WWI Firearms technical manuals

### Inspirational Games (Audio)
- *Verdun* (2015) - Authentic WWI soundscape
- *Battlefield 1* (2016) - High-quality WWI audio
- *Tannenberg* (1914-1918 Eastern Front)
- *Hell Let Loose* - WWII but similar design principles

---

## 📝 NOTES

- **All audio must respect the historical tragedy** - 700,000 casualties at Verdun
- **Authenticity over convenience** - Use period-correct sounds even if harder to source
- **Player experience over realism** - Shell whistles need to be audible (gameplay > simulation)
- **Cultural sensitivity** - French and German voices must be respectful, not caricatures
- **Educational value** - Audio should teach players what Verdun actually sounded like

---

**Document Version**: 1.0
**Last Updated**: November 10, 2025
**Author**: OnlyTheDead Development Team
**Status**: Sound Library Planning Document
