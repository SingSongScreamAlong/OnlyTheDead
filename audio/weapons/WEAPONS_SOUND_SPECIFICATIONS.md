# WWI WEAPONS SOUND SPECIFICATIONS

**Project**: Only The Dead - Battle of Verdun 1916
**Category**: Weapons Audio Reference
**Purpose**: Detailed specifications for acquiring/creating period-accurate weapon sounds

---

## 🎯 PRIORITY WEAPONS

### Priority 1: Player Weapons (Essential)
1. Lebel M1886/93 rifle
2. Berthier M1907/15 rifle
3. F1 "Pineapple" grenade
4. French trench knife/bayonet

### Priority 2: Common Weapons (High)
5. Hotchkiss M1914 machine gun
6. Chauchat M1915 light machine gun
7. Ruby pistol (officers)
8. 58mm trench mortar

### Priority 3: Enemy Weapons (Medium)
9. Mauser Gewehr 98 rifle (German)
10. MG08 machine gun (German)
11. Stielhandgranate "Stick grenade" (German)

---

## 📋 FRENCH RIFLES - DETAILED SPECS

### LEBEL M1886/93 RIFLE
**Primary French infantry weapon** - Tubular magazine, 8mm Lebel cartridge

#### Required Sounds:

**1. Firing**
- **Variations needed**: 8-10
- **Characteristics**: Sharp crack (smokeless powder), not deep boom
- **Distance**: Need near/medium/far versions
- **Environment**: Open field vs. trench (reverb difference)
- **Technical notes**:
  - 8mm Lebel = smaller than modern rifle rounds
  - Smokeless powder = sharper, cleaner sound
  - Long barrel (798mm) = distinctive report

**Files needed**:
```
WPN_RIFLE_LEBEL_FIRE_NEAR_01.wav (through _10.wav)
WPN_RIFLE_LEBEL_FIRE_MED_01.wav (through _05.wav)
WPN_RIFLE_LEBEL_FIRE_FAR_01.wav (through _05.wav)
WPN_RIFLE_LEBEL_FIRE_TRENCH_01.wav (with reverb, through _05.wav)
```

**2. Bolt Action**
- **Variations needed**: 5-6
- **Characteristics**: Metallic clank, distinctive "clack-clack"
- **Timing**: Two-stage sound (open, close)
- **Technical notes**:
  - Heavier action than Mauser (French design)
  - Audible when operating (not silent)
  - Different sound than German rifles

**Files needed**:
```
WPN_RIFLE_LEBEL_BOLT_OPEN_01.wav (through _05.wav)
WPN_RIFLE_LEBEL_BOLT_CLOSE_01.wav (through _05.wav)
WPN_RIFLE_LEBEL_BOLT_CYCLE_FULL_01.wav (through _05.wav)
```

**3. Loading (Tubular Magazine)**
- **Variations needed**: 3-4
- **Characteristics**: One round at a time through loading gate
- **Process sounds**:
  - Loading gate opening (metallic click)
  - Round insertion (cartridge sliding on metal)
  - Magazine spring compression (as rounds load)
  - Loading gate closing
- **Technical notes**:
  - SLOW process (8 rounds takes time)
  - Distinctive tubular magazine sound
  - Different from clip-fed rifles

**Files needed**:
```
WPN_RIFLE_LEBEL_LOAD_GATE_OPEN_01.wav
WPN_RIFLE_LEBEL_LOAD_INSERT_ROUND_01.wav (through _05.wav)
WPN_RIFLE_LEBEL_LOAD_SPRING_COMPRESS_01.wav
WPN_RIFLE_LEBEL_LOAD_GATE_CLOSE_01.wav
WPN_RIFLE_LEBEL_LOAD_COMPLETE_SEQUENCE_01.wav
```

**4. Empty/Dry Fire**
- **Variations needed**: 3
- **Characteristics**: Firing pin strike on empty chamber
- **Technical notes**: Metallic click, no explosion

**Files needed**:
```
WPN_RIFLE_LEBEL_DRYFIRE_01.wav (through _03.wav)
WPN_RIFLE_LEBEL_EMPTY_CLICK_01.wav (through _03.wav)
```

**5. Handling/Foley**
- **Variations needed**: 5-6
- **Characteristics**: Weapon movement, sling, wood stock
- **Situations**:
  - Raising to shoulder
  - Lowering weapon
  - Sling movement
  - Stock against body/gear

**Files needed**:
```
WPN_RIFLE_LEBEL_RAISE_01.wav (through _05.wav)
WPN_RIFLE_LEBEL_LOWER_01.wav (through _05.wav)
WPN_RIFLE_LEBEL_MOVEMENT_01.wav (through _05.wav)
```

---

### BERTHIER M1907/15 RIFLE
**Clip-fed rifle** - 3-round en-bloc clip, 8mm Lebel cartridge

#### Required Sounds:

**1. Firing**
- Similar to Lebel (same cartridge)
- **Variations needed**: 6-8
- **Note**: Slightly different than Lebel due to shorter barrel

**Files needed**:
```
WPN_RIFLE_BERTHIER_FIRE_NEAR_01.wav (through _08.wav)
WPN_RIFLE_BERTHIER_FIRE_MED_01.wav (through _04.wav)
WPN_RIFLE_BERTHIER_FIRE_FAR_01.wav (through _04.wav)
```

**2. Bolt Action**
- **Variations needed**: 4-5
- **Characteristics**: Smoother action than Lebel
- **Note**: Faster cycling (better design)

**Files needed**:
```
WPN_RIFLE_BERTHIER_BOLT_CYCLE_01.wav (through _05.wav)
```

**3. Clip Loading (En-Bloc)**
- **Variations needed**: 3-4
- **Characteristics**: 3-round clip insertion
- **Process sounds**:
  - Clip insertion (metallic slide)
  - Clip locking in place (click)
  - Clip ejection when empty (ping/clatter)
- **Technical notes**:
  - MUCH faster than Lebel loading
  - Distinctive clip ejection sound
  - Similar to M1 Garand "ping" but quieter

**Files needed**:
```
WPN_RIFLE_BERTHIER_CLIP_INSERT_01.wav (through _04.wav)
WPN_RIFLE_BERTHIER_CLIP_LOCK_01.wav
WPN_RIFLE_BERTHIER_CLIP_EJECT_01.wav (through _04.wav)
```

---

## 🔫 FRENCH MACHINE GUNS

### HOTCHKISS M1914
**Primary French machine gun** - 8mm Lebel, gas-operated, strip-fed, 240-450 rpm

#### Required Sounds:

**1. Firing**
- **Variations needed**: 6-8 for different burst lengths
- **Characteristics**:
  - Sustained fire sound (not single shots)
  - Gas-operated cycling
  - Distinct from water-cooled MG08
- **Burst types**:
  - Short burst (3-5 rounds)
  - Medium burst (10-15 rounds)
  - Long burst (20-30 rounds)
  - Sustained fire (60+ rounds)

**Files needed**:
```
WPN_MG_HOTCHKISS_FIRE_SHORT_01.wav (through _05.wav)
WPN_MG_HOTCHKISS_FIRE_MEDIUM_01.wav (through _05.wav)
WPN_MG_HOTCHKISS_FIRE_LONG_01.wav (through _05.wav)
WPN_MG_HOTCHKISS_FIRE_SUSTAINED_01.wav (through _03.wav)
```

**2. Mechanical Sounds**
- **Variations needed**: 4-5
- **Characteristics**:
  - Gas vent hiss
  - Strip feeding (24-round strips, metallic rattle)
  - Cooling fins heat dissipation
  - Overheating effects

**Files needed**:
```
WPN_MG_HOTCHKISS_GAS_VENT_01.wav (through _03.wav)
WPN_MG_HOTCHKISS_STRIP_FEED_01.wav (through _05.wav)
WPN_MG_HOTCHKISS_STRIP_INSERT_01.wav (through _03.wav)
WPN_MG_HOTCHKISS_OVERHEAT_SIZZLE_01.wav (through _03.wav)
```

**3. Stopping/Cycling**
- **Variations needed**: 3
- **Characteristics**:
  - Last round fired (distinctive end)
  - Bolt forward (charging)
  - Safety engagement

**Files needed**:
```
WPN_MG_HOTCHKISS_STOP_FIRE_01.wav (through _03.wav)
WPN_MG_HOTCHKISS_BOLT_FORWARD_01.wav (through _03.wav)
```

---

### CHAUCHAT M1915
**Light machine gun** - Long recoil, half-moon magazine, notorious reliability issues

#### Required Sounds:

**1. Firing**
- **Variations needed**: 6-8
- **Characteristics**:
  - Distinctive clatter (long recoil system)
  - Less smooth than other machine guns
  - Lower rate of fire (~240 rpm)
- **IMPORTANT**: Should sound "rougher" than Hotchkiss

**Files needed**:
```
WPN_MG_CHAUCHAT_FIRE_BURST_01.wav (through _08.wav)
WPN_MG_CHAUCHAT_FIRE_AUTO_01.wav (through _05.wav)
```

**2. Jamming/Malfunctions**
- **Variations needed**: 5-6
- **Characteristics**:
  - Jam during firing (stoppage)
  - Clearing jam (bolt manipulation)
  - Feed failure sounds
- **IMPORTANT**: Chauchat was prone to jamming (especially with mud)

**Files needed**:
```
WPN_MG_CHAUCHAT_JAM_01.wav (through _06.wav)
WPN_MG_CHAUCHAT_CLEAR_JAM_01.wav (through _04.wav)
WPN_MG_CHAUCHAT_FEED_FAIL_01.wav (through _03.wav)
```

**3. Magazine**
- **Variations needed**: 4-5
- **Characteristics**:
  - 20-round half-moon magazine insertion
  - Magazine removal
  - Magazine drop (metallic clatter)

**Files needed**:
```
WPN_MG_CHAUCHAT_MAG_INSERT_01.wav (through _05.wav)
WPN_MG_CHAUCHAT_MAG_REMOVE_01.wav (through _05.wav)
WPN_MG_CHAUCHAT_MAG_DROP_01.wav (through _03.wav)
```

---

## 💣 GRENADES

### F1 "PINEAPPLE" GRENADE (French)
**Defensive fragmentation grenade** - Cast iron segmented body, 5-second fuse

#### Required Sounds:

**1. Handling**
- **Variations needed**: 4-5
- **Characteristics**:
  - Removing from pouch (canvas rustle)
  - Pin removal (metallic pull)
  - Lever release (spring-loaded "spoon")
  - Fuse ignition (hiss)

**Files needed**:
```
WPN_GREN_F1_POUCH_REMOVE_01.wav (through _03.wav)
WPN_GREN_F1_PIN_PULL_01.wav (through _05.wav)
WPN_GREN_F1_LEVER_RELEASE_01.wav (through _05.wav)
WPN_GREN_F1_FUSE_IGNITE_01.wav (through _04.wav)
```

**2. Throwing**
- **Variations needed**: 5-6
- **Characteristics**:
  - Throwing motion (whoosh)
  - Trajectory whoosh (Doppler effect)
  - Landing impact (varies by surface)

**Files needed**:
```
WPN_GREN_F1_THROW_01.wav (through _06.wav)
WPN_GREN_F1_FLIGHT_SHORT_01.wav (through _04.wav)
WPN_GREN_F1_FLIGHT_LONG_01.wav (through _04.wav)
WPN_GREN_F1_IMPACT_MUD_01.wav (through _04.wav)
WPN_GREN_F1_IMPACT_WOOD_01.wav (through _03.wav)
WPN_GREN_F1_IMPACT_METAL_01.wav (through _03.wav)
```

**3. Explosion**
- **Variations needed**: 8-10
- **Characteristics**:
  - Sharp, high-pitched blast (defensive grenade)
  - Fragmentation (shrapnel whistling)
  - Distance variations (near/medium/far)
- **IMPORTANT**: Should sound DIFFERENT from artillery

**Files needed**:
```
WPN_GREN_F1_EXPLODE_NEAR_01.wav (through _08.wav)
WPN_GREN_F1_EXPLODE_MED_01.wav (through _05.wav)
WPN_GREN_F1_EXPLODE_FAR_01.wav (through _04.wav)
WPN_GREN_F1_SHRAPNEL_01.wav (through _06.wav)
```

---

### STIELHANDGRANATE "STICK GRENADE" (German)
**Offensive blast grenade** - Wooden handle, pull-cord ignition

#### Required Sounds:

**1. Handling**
- **Variations needed**: 4-5
- **Characteristics**:
  - Removing cap (wooden twist)
  - Pull-cord extraction (porcelain ball and string)
  - Fuse ignition (hiss)

**Files needed**:
```
WPN_GREN_STICK_CAP_TWIST_01.wav (through _04.wav)
WPN_GREN_STICK_CORD_PULL_01.wav (through _05.wav)
WPN_GREN_STICK_FUSE_IGNITE_01.wav (through _04.wav)
```

**2. Throwing**
- **Variations needed**: 5-6
- **Characteristics**:
  - Distinctive whoosh (heavier, handle creates drag)
  - Tumbling sound (handle rotates in flight)
  - Landing impact (wood/metal)

**Files needed**:
```
WPN_GREN_STICK_THROW_01.wav (through _06.wav)
WPN_GREN_STICK_FLIGHT_01.wav (through _05.wav)
WPN_GREN_STICK_IMPACT_01.wav (through _04.wav)
```

**3. Explosion**
- **Variations needed**: 8-10
- **Characteristics**:
  - Deeper blast than F1 (offensive, blast-focused)
  - Less fragmentation sound
  - Powerful shockwave

**Files needed**:
```
WPN_GREN_STICK_EXPLODE_NEAR_01.wav (through _08.wav)
WPN_GREN_STICK_EXPLODE_MED_01.wav (through _05.wav)
WPN_GREN_STICK_EXPLODE_FAR_01.wav (through _04.wav)
```

---

## 🔫 PISTOLS

### RUBY PISTOL (French)
**Semi-automatic pistol** - 7.65mm, officer's sidearm

#### Required Sounds:

**1. Firing**
- **Variations needed**: 5-6
- **Characteristics**:
  - Sharp crack (small caliber)
  - Semi-auto cycling sound
  - Lighter than rifle fire

**Files needed**:
```
WPN_PISTOL_RUBY_FIRE_01.wav (through _06.wav)
WPN_PISTOL_RUBY_FIRE_SUPPRESSED_01.wav (for enclosed spaces)
```

**2. Mechanical**
- **Variations needed**: 4-5
- **Characteristics**:
  - Slide cycling
  - Magazine insertion/removal
  - Slide release
  - Hammer cock

**Files needed**:
```
WPN_PISTOL_RUBY_SLIDE_01.wav (through _05.wav)
WPN_PISTOL_RUBY_MAG_INSERT_01.wav (through _04.wav)
WPN_PISTOL_RUBY_MAG_REMOVE_01.wav (through _04.wav)
WPN_PISTOL_RUBY_SLIDE_RELEASE_01.wav (through _03.wav)
```

---

## 🎯 GERMAN WEAPONS (Enemy Audio)

### MAUSER GEWEHR 98 RIFLE
**Primary German infantry weapon** - 7.92×57mm, 5-round stripper clip

#### Required Sounds:

**1. Firing**
- **Variations needed**: 6-8
- **Characteristics**:
  - Sharp crack (7.92mm Mauser)
  - Slightly different pitch than French 8mm
  - Heard from enemy positions (medium/far distance primarily)

**Files needed**:
```
WPN_RIFLE_MAUSER98_FIRE_MED_01.wav (through _06.wav)
WPN_RIFLE_MAUSER98_FIRE_FAR_01.wav (through _05.wav)
WPN_RIFLE_MAUSER98_FIRE_NEAR_01.wav (through _04.wav) [less common]
```

**2. Bolt Action**
- **Variations needed**: 4-5
- **Characteristics**:
  - Smooth, precise action
  - Different sound than French rifles
  - "Ka-chunk" distinctive to Mauser

**Files needed**:
```
WPN_RIFLE_MAUSER98_BOLT_01.wav (through _05.wav)
```

**3. Stripper Clip Loading**
- **Variations needed**: 3-4
- **Characteristics**:
  - 5-round stripper clip insertion
  - Metallic slide into magazine
  - Clip removal/discard

**Files needed**:
```
WPN_RIFLE_MAUSER98_CLIP_INSERT_01.wav (through _04.wav)
WPN_RIFLE_MAUSER98_CLIP_DISCARD_01.wav (through _03.wav)
```

---

### MG08 MACHINE GUN (German)
**Water-cooled heavy machine gun** - 7.92mm, belt-fed, ~500 rpm

#### Required Sounds:

**1. Firing**
- **Variations needed**: 6-8
- **Characteristics**:
  - Sustained fire (very reliable weapon)
  - Belt-fed cycling (continuous rattle)
  - Water-cooling bubbling (when heard close)
- **IMPORTANT**: MG08 was extremely reliable and feared

**Files needed**:
```
WPN_MG_MG08_FIRE_BURST_01.wav (through _06.wav)
WPN_MG_MG08_FIRE_SUSTAINED_01.wav (through _04.wav)
WPN_MG_MG08_FIRE_DISTANT_01.wav (through _05.wav)
```

**2. Mechanical**
- **Variations needed**: 4-5
- **Characteristics**:
  - Belt feed mechanism (metallic links)
  - Water jacket sounds
  - Steam venting (when overheated)

**Files needed**:
```
WPN_MG_MG08_BELT_FEED_01.wav (through _05.wav)
WPN_MG_MG08_WATER_BUBBLE_01.wav (through _03.wav)
WPN_MG_MG08_STEAM_VENT_01.wav (through _03.wav)
```

---

## 🔪 MELEE WEAPONS

### FRENCH TRENCH KNIFE / BAYONET
**Close combat weapons** - Silent but deadly

#### Required Sounds:

**1. Deployment**
- **Variations needed**: 4-5
- **Characteristics**:
  - Unsheathing (metal on leather)
  - Bayonet attachment (socket lock)
  - Quick draw

**Files needed**:
```
WPN_MELEE_KNIFE_DRAW_01.wav (through _05.wav)
WPN_MELEE_BAYONET_ATTACH_01.wav (through _04.wav)
WPN_MELEE_BAYONET_DETACH_01.wav (through _03.wav)
```

**2. Impact/Usage**
- **Variations needed**: 5-6
- **Characteristics**:
  - Stabbing impact (fabric/flesh - respectful)
  - Slashing whoosh
  - Metal on metal (parrying)

**Files needed**:
```
WPN_MELEE_KNIFE_STAB_01.wav (through _05.wav)
WPN_MELEE_KNIFE_SLASH_01.wav (through _05.wav)
WPN_MELEE_KNIFE_PARRY_01.wav (through _04.wav)
```

---

## 📦 TOTAL WEAPON SOUNDS NEEDED

### Summary by Weapon Type

| Weapon | Base Sounds | Variations | Total Files |
|--------|-------------|------------|-------------|
| Lebel Rifle | 12 types | 5 avg | 60 |
| Berthier Rifle | 8 types | 5 avg | 40 |
| Hotchkiss MG | 10 types | 4 avg | 40 |
| Chauchat MG | 9 types | 5 avg | 45 |
| F1 Grenade | 10 types | 5 avg | 50 |
| Stick Grenade | 9 types | 5 avg | 45 |
| Ruby Pistol | 6 types | 4 avg | 24 |
| Mauser 98 | 6 types | 5 avg | 30 |
| MG08 | 8 types | 5 avg | 40 |
| Melee | 6 types | 5 avg | 30 |
| **TOTAL** | **84 types** | **Avg 4.8** | **~404 files** |

---

## 🎤 RECORDING SPECIFICATIONS

### If Recording Custom Sounds

**Equipment**:
- High-quality field recorder (Zoom H6, Sound Devices MixPre)
- Windscreen (outdoor recording essential)
- Multiple microphone positions

**Recording Settings**:
- 48kHz, 24-bit WAV
- Multiple takes (minimum 5 per sound)
- Vary intensity/timing
- Record room tone for each session

**Environment**:
- Outdoor range (authentic reverb)
- Dry indoor range (for processing flexibility)
- Distance recordings (near/medium/far)

**Safety**:
- Hearing protection (separate from microphones)
- Range safety protocols
- Live fire only with proper supervision

---

## 🔍 AUTHENTICITY CHECKLIST

### Verification Points

✅ **Correct cartridge sounds** - 8mm Lebel ≠ modern .308
✅ **Period mechanisms** - Bolt actions, not modern semi-autos
✅ **Appropriate reverb** - Outdoor vs. trench acoustics
✅ **Historical rate of fire** - Not too fast (bolt actions are slow)
✅ **Distinct French vs. German** - Weapons should sound different
✅ **No modern sounds** - Avoid polymer, modern suppressors, etc.
✅ **Appropriate echo** - Battlefield scale, not indoor range

---

## 📚 REFERENCES

### Technical Manuals
- "French Infantry Weapons of WWI" by Ian Sumner
- "German Infantry Weapons of WWI" by Stephen Bull
- Forgotten Weapons (YouTube - Ian McCollum) - Excellent WWI weapons coverage
- C&Rsenal (YouTube) - In-depth WWI firearms history and firing

### Sound References
- BOOM Library "World War I Firearms" - Gold standard
- A Sound Effect "Rifles & Pistols of The World Wars"
- Historical recordings (Imperial War Museum)

### Gameplay References
- *Verdun* (2015) - Authentic weapon sounds
- *Battlefield 1* (2016) - High-quality WWI audio
- *Tannenberg* - Similar weapon set

---

**Document Version**: 1.0
**Last Updated**: November 10, 2025
**Status**: Weapon Audio Specification - Complete
