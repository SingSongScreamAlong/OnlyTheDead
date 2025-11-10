# Only The Dead - Asset Specifications

**Version**: 1.0
**Last Updated**: November 10, 2025
**Purpose**: Complete specifications for all 3D, audio, and texture assets

---

## 3D MODEL SPECIFICATIONS

### General Requirements

**Format**: FBX, OBJ (export from DCC), USD (Houdini)
**Coordinate System**: Z-up, right-handed (UE5 standard)
**Units**: Centimeters
**Texture Coordinate**: 0-1 UV space, no overlaps for lightmaps
**Polygon Budget**: See specific categories below

---

### Characters

#### Player Character - Jean Moreau

**Poly Budget**: 50,000 triangles (body + equipment)
**LODs**:
- LOD0: 50,000 tris (first-person arms visible)
- LOD1: 25,000 tris (close range NPCs)
- LOD2: 10,000 tris (medium range)
- LOD3: 3,000 tris (far distance)
- LOD4: 500 tris (very far, silhouette)

**Rigging**:
- UE5 Mannequin skeleton compatible
- Additional bones for equipment (rifle, backpack)
- Facial rig: 52 blend shapes minimum

**Textures** (PBR workflow):
- BaseColor: 4096x4096
- Normal: 4096x4096
- ORM (Occlusion/Roughness/Metallic): 4096x4096
- Emissive: 2048x2048 (if needed)

**Variations**:
- Clean uniform (start of campaign)
- Dirty uniform (mid-campaign)
- Heavily damaged uniform (late campaign)
- 3 injury states (bandaged, bloodied, severe)

**Delivery Format**: Skeletal Mesh with animations

---

#### NPCs - Squad Members

**Count**: 4 unique models (Dubois, Laurent, Mercier, Rousseau)
**Poly Budget**: 30,000 tris per character (LOD0)
**LODs**: Same as player character
**Textures**: 2048x2048 (can share some textures)

**Specific Requirements**:
- **Dubois**: Older, grizzled, corporal stripes, war-weary face
- **Laurent**: Young, boyish, clean-shaven, fearful expression
- **Mercier**: Mediterranean features, shrewd eyes, slightly disheveled
- **Rousseau**: Military bearing, sergeant stripes, stern face

**Rigging**: UE5 Mannequin skeleton

---

#### NPCs - Generic Soldiers

**Count**: 10 variations (French), 10 variations (German)
**Poly Budget**: 20,000 tris (LOD0)
**Modular System**:
- Base body mesh (12,000 tris)
- Swappable heads (10 types, 3,000 tris each)
- Equipment variations (5,000 tris)

**Textures**: 2048x2048, shared where possible

---

### Weapons

#### Lebel Model 1886 Rifle

**Poly Budget**: 8,000 tris
**LODs**:
- LOD0: 8,000 tris (first-person view)
- LOD1: 3,000 tris (world space)
- LOD2: 1,000 tris (far view)

**Textures**: 2048x2048 PBR
**Variations**:
- Clean/new
- Worn/used
- Damaged/battlefield pickup

**Animations Required**:
- Reload (8 round clip)
- Bolt action
- Inspect
- Draw/holster
- Bayonet attach/detach

**Reference**: Historical photos, museum pieces

---

#### Other Weapons

| Weapon | Tris (LOD0) | Texture | Animations |
|--------|-------------|---------|------------|
| Berthier Rifle | 7,500 | 2048² | Reload, bolt, inspect |
| Mauser Gewehr 98 | 7,500 | 2048² | Reload, bolt, inspect |
| Hotchkiss M1914 MG | 12,000 | 2048² | Reload belt, fire, jam clear |
| MG 08 | 12,000 | 2048² | Reload belt, fire, water cool |
| F1 Grenade | 2,000 | 1024² | Pull pin, throw |
| Stielhandgranate | 2,500 | 1024² | Unscrew cap, throw |
| Bayonet | 1,500 | 1024² | Stab, slash |
| Trench Club | 1,000 | 1024² | Swing |

**Total Weapon Assets**: 20 unique models

---

### Environment - Props

#### Sandbags

**Variants**: 5 (full, half-full, torn, stacked-2, stacked-3)
**Poly Budget**: 500-1,500 tris per variant
**Textures**: 1024x1024 (tileable)
**Material**: Mud, worn canvas
**Modular**: Can be placed to build trench walls

---

#### Barbed Wire

**Variants**: 3 (coiled, stretched, tangled)
**Poly Budget**:
- Coiled: 2,000 tris per coil
- Stretched: 500 tris per meter
- Tangled: 3,000 tris per section

**Textures**: 1024x1024
**Material**: Rusted metal
**Special**: Destructible (can be cut by wire cutters)

---

#### Shell Craters

**Variants**: 10 sizes (from 75mm to 420mm)
**Poly Budget**: 1,000-5,000 tris (depending on size)
**Textures**: 2048x2048 (tileable dirt/mud)
**Procedural**: Can be spawned procedurally
**Water-filled**: Optional puddle material

**Sizes**:
- 75mm: 2m diameter, 1.5m deep
- 155mm: 5m diameter, 3m deep
- 210mm: 6m diameter, 4m deep
- 305mm: 10m diameter, 6m deep
- 420mm: 15m diameter, 10m deep

---

#### Destroyed Trees

**Variants**: 15 (various species, destruction states)
**Poly Budget**: 3,000-8,000 tris
**Textures**: 2048x2048
**States**:
- Shattered trunk (no branches)
- Half-destroyed
- Fallen log
- Splintered stump

---

### Environment - Buildings

#### Destroyed French House

**Variants**: 20 (various destruction levels)
**Poly Budget**: 10,000-30,000 tris per building
**Textures**: 4096x4096 (tileable where possible)
**Modular Pieces**:
- Walls (intact, damaged, destroyed)
- Roofs (collapsed variations)
- Floors (broken)
- Interior elements (furniture, debris)

**LODs**: 4 levels
**Collision**: Custom collision mesh (simplified)

---

#### Church Ruins

**Poly Budget**: 50,000 tris (landmark)
**Textures**: 4096x4096
**Features**:
- Fallen bell tower
- Broken stained glass
- Rubble piles
- Intact sections (partial walls)

**Special**: Can be used as landmark/shelter

---

#### Fort Douaumont Interior

**Sections**: 15 unique areas
**Total Poly Budget**: 500,000 tris (entire fort)
**Textures**: 4096x4096 (shared tileable + unique)

**Areas**:
- Entrance corridors
- Barracks rooms
- Command room
- Magazine
- Artillery casemates
- Stairwells
- Ventilation shafts
- Courtyard

**Materials**:
- Concrete (damaged, chipped)
- Steel doors (rusted, blown open)
- Debris (rubble, broken equipment)

**LODs**: Entire fort uses World Partition with streaming

---

## AUDIO ASSET SPECIFICATIONS

### Format Requirements

**Master Format**: 48kHz, 24-bit WAV
**Engine Format**: Converted to platform-optimized formats by UE5
**Spatialization**: All 3D sounds use HRTF
**Compression**: Platform-dependent (Opus, Vorbis)

---

### Artillery Audio

#### Shell Incoming Sounds

**Duration**: Variable (2-30 seconds depending on shell type)
**Channels**: Mono (will be spatialized in-engine)
**Recording Requirements**:
- Actual recordings preferred (range recordings, historical sound archives)
- If synthesized: Accurate frequency content per specifications

**Per Shell Type** (see Development Book for exact specs):
- 75mm: High-pitched whistle, 2-3 sec
- 155mm: Low rumble, 5-8 sec
- 210mm: Almost silent, 1-2 sec
- 305mm: Freight train, 15-20 sec
- 420mm: Apocalyptic roar, 20-30 sec

**Variations**: 5 per shell type (avoid repetition)

---

#### Shell Impact Sounds

**Duration**: 1-3 seconds + reverb tail
**Channels**: Mono
**Layers**:
- Initial blast (sharp transient)
- Explosion body (rumble)
- Debris (falling dirt, shrapnel)
- Reverb tail (environment-dependent)

**Variations**: 10 per shell type

---

### Small Arms Audio

**Weapons**: 10 types (see weapon list)
**Per Weapon**:
- Fire sound (close, 1P view)
- Fire sound (distant, 3P view)
- Reload sounds (foley, mechanical)
- Dry fire
- Jam/malfunction

**Variations**: 3 per sound type

---

### Voice Acting

**Languages**: French (primary), German (secondary), English (narration)
**Total Lines**: ~6,700 (see Character Database)
**Format**: 48kHz, 24-bit, Mono
**Processing**: Light EQ, no heavy compression

**Recording Requirements**:
- Professional native speakers
- Period-appropriate pronunciation
- Emotional range (calm to terrified)
- Multiple takes per line

**Actors Needed**:
- 10 French voice actors (male)
- 1 French voice actor (female) - Nurse Marie
- 5 German voice actors (male)
- 1 Narrator (English, French-accented preferred)

---

### Ambient Soundscapes

**Tracks**: 20 ambient loops
**Duration**: 2-5 minutes each (seamless loop)
**Channels**: Stereo or Ambisonics (for better spatialization)

**Types**:
- Distant artillery (various intensities)
- Trench ambience (water dripping, rats, wind)
- No-man's-land (wind, distant groans, wire creaking)
- Dugout interior (muffled explosions, breathing)
- Field hospital (medical sounds, groaning)

---

### Foley Sounds

**Count**: ~500 individual sounds
**Categories**:
- Footsteps (10 surface types x 5 variations)
- Equipment rattle (walking, running)
- Cloth movement (uniform)
- Metal clinks (helmet, rifle, tools)
- Breathing (normal, heavy, panicked)
- Mud squelching
- Door creaks
- Ladder climbing

---

### Music

**Tracks**: 4-5 pieces
**Duration**: 2-4 minutes each
**Style**: Solemn, mournful, minimal
**Instrumentation**: Strings-focused, period-appropriate

**Composer Direction**: "Restraint. Like Ravel's *Le Tombeau de Couperin*."

**Tracks**:
1. Main Menu Theme (melancholic, sets tone)
2. Victory Theme (bittersweet, not triumphant)
3. Death Theme (somber, respectful)
4. Dubois Death Scene (emotional, brief)
5. Credits (reflective, memorial)

---

## TEXTURE ASSET SPECIFICATIONS

### Material System

**Workflow**: PBR (Physically Based Rendering)
**Maps Required**:
- BaseColor (Albedo) - RGB
- Normal - RGB (DirectX or OpenGL format)
- ORM - Packed (Ambient Occlusion, Roughness, Metallic)
- Height (optional, for parallax)
- Emissive (optional)

**Format**: TGA, PNG (lossless)
**Resolution Tiers**:
- Hero assets: 4096x4096
- Standard assets: 2048x2048
- Small props: 1024x1024
- Trim sheets: 4096x256 or 8192x512

---

### Terrain Textures

**Mud** (dominant material at Verdun):
- 10 variations (wet, dry, churned)
- 2048x2048, tileable
- High detail (bootprints, shell holes visible in blend)

**Rock/Stone**:
- Limestone (local geology)
- 2048x2048, tileable

**Grass** (rare, only in rest areas):
- Dead/brown grass
- 2048x2048, tileable

**Special Blends**:
- Mud + Blood (trenches)
- Mud + Ash (bombardment zones)

---

### Character Textures

**Uniforms**:
- French Horizon Blue (3 dirt levels)
- German Field Grey (3 dirt levels)
- Colonial Infantry variations

**Skin**:
- 5 skin tones (pale to dark)
- 3 dirt/damage levels
- Wound overlays (burns, cuts, frostbite)

---

### UI Textures

**Resolution**: 1024x1024 (icons), 2048x2048 (backgrounds)
**Style**: Hand-drawn, WWI poster aesthetic
**Icons**: ~50 (items, abilities, objectives)

---

## ANIMATION SPECIFICATIONS

### Player Character Animations

**Total**: ~200 animations
**Framerate**: 30 FPS (motion capture quality)
**Format**: FBX with UE5 skeleton

**Categories**:

**Locomotion** (30 animations):
- Idle (5 variations)
- Walk (forward, back, strafe left/right)
- Run (forward, back, strafe left/right)
- Sprint (forward only)
- Crouch (idle, walk, run)
- Prone (idle, crawl forward/back)
- Jump
- Land (light, heavy)

**Combat** (50 animations):
- Aim (rifle, pistol)
- Fire (per weapon type)
- Reload (per weapon type)
- Throw grenade
- Melee (bayonet, club, knife)
- Take cover
- Lean (left/right)

**Interactions** (40 animations):
- Open door
- Climb ladder
- Carry wounded
- Use item
- Drink/eat
- Medical treatment
- Operate equipment

**Reactions** (30 animations):
- Hit reactions (front, back, left, right)
- Death (multiple variations)
- Shell shock (flinch, cower, freeze)
- Exhaustion (stumble, fall)
- Pain (injured locomotion)

**Facial** (50 blend shapes):
- Phonemes (for dialogue)
- Emotions (fear, pain, determination, relief)

---

## QUALITY ASSURANCE CHECKLIST

### 3D Models
- [ ] Correct scale (real-world measurements)
- [ ] Clean topology (no n-gons, minimal triangles)
- [ ] UVs within 0-1 space, no stretching
- [ ] LODs generated and functional
- [ ] Collision meshes optimized
- [ ] Materials assigned correctly
- [ ] Historical accuracy verified

### Textures
- [ ] Resolution appropriate for asset
- [ ] PBR values physically accurate
- [ ] Normal maps baked correctly (no seams)
- [ ] Tiling textures seamless
- [ ] File size optimized
- [ ] All maps present (BaseColor, Normal, ORM)

### Audio
- [ ] Sample rate: 48kHz
- [ ] Bit depth: 24-bit
- [ ] No clipping or distortion
- [ ] Proper fade in/out
- [ ] Loopable (if required)
- [ ] Metadata tagged
- [ ] Loudness normalized

### Animations
- [ ] Framerate consistent (30 FPS)
- [ ] Looping animations seamless
- [ ] Root motion baked (if required)
- [ ] Skeleton matches UE5 Mannequin
- [ ] No mesh penetration
- [ ] Motion is natural and weighted

---

## DELIVERY & INTEGRATION

### File Naming Conventions

**3D Models**: `SM_[Category]_[Name]_[Variant].fbx`
Example: `SM_Character_Jean_Dirty.fbx`

**Textures**: `T_[AssetName]_[MapType].tga`
Example: `T_Jean_BaseColor.tga`, `T_Jean_Normal.tga`

**Audio**: `SFX_[Category]_[Name]_[Variation].wav`
Example: `SFX_Shell_155mm_Incoming_01.wav`

**Animations**: `ANIM_[Action]_[Variant].fbx`
Example: `ANIM_Reload_Lebel_01.fbx`

### Folder Structure

```
Content/
├── Characters/
│   ├── Player/
│   ├── NPCs/
│   └── Enemies/
├── Weapons/
├── Props/
├── Environments/
│   ├── Buildings/
│   ├── Terrain/
│   └── Vegetation/
├── Audio/
│   ├── Artillery/
│   ├── SmallArms/
│   ├── Ambient/
│   └── Voice/
└── Animations/
    ├── Player/
    └── NPCs/
```

---

## PRODUCTION TIMELINE

**Phase 1: Blockout** (Weeks 1-4)
- Grey-box 3D models (basic shapes)
- Placeholder audio
- No textures yet

**Phase 2: First Pass** (Weeks 5-12)
- Full geometry, basic textures
- Basic audio implemented
- Key animations

**Phase 3: Polish** (Weeks 13-20)
- High-quality textures
- Final audio
- All animations
- LODs optimized

**Phase 4: Optimization** (Weeks 21-24)
- Performance profiling
- File size reduction
- Final quality pass

---

**Document Maintained By**: OnlyTheDead Art Director
**Last Updated**: November 10, 2025
**Version**: 1.0
