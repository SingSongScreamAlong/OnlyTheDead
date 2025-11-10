# ONLY THE DEAD - COMPLETE TASK LIST

## CORE SYSTEMS

### Survival Meters
- [ ] Hunger meter (depletion rates, thresholds, death conditions)
- [ ] Thirst meter (depletion rates, thresholds, death conditions)
- [ ] Stamina meter (depletion rates, regeneration, cascading effects)
- [ ] Warmth meter (hypothermia stages, frostbite progression)
- [ ] Hygiene meter (disease system, lice, trench foot)
- [ ] Morale meter (shell shock mechanics, breakdown states)
- [ ] Alertness meter (sleep deprivation, hallucinations, micro-sleep)
- [ ] JSON configuration system for all meter values
- [ ] Cascading failure system (low meters affect others)
- [ ] Death screens for each meter failure
- [ ] Save/load integration for all meters

### Inventory System
- [ ] Item data structure (BaseItem class)
- [ ] Food items (12 French rations + restoration values)
- [ ] Water items (bottles, canteens, disease risk)
- [ ] Medical items (bandages, morphine, antiseptic)
- [ ] Equipment items (gas mask, filters, coat, blanket)
- [ ] Weapon items (rifle, grenades, bayonet, ammo)
- [ ] Inventory UI (grid or list display)
- [ ] Hotkey system (1-9 quick access)
- [ ] Weight/capacity system
- [ ] Item durability/degradation
- [ ] Consumption animations
- [ ] Item pickup/drop mechanics
- [ ] Inventory persistence (save/load)

### Artillery System
- [ ] Shell projectile class (physics-based ballistics)
- [ ] 75mm shell (sound, flight time, damage, blast radius)
- [ ] 155mm shell (sound, flight time, damage, blast radius)
- [ ] 210mm shell (sound, flight time, damage, blast radius)
- [ ] 305mm shell (sound, flight time, damage, blast radius)
- [ ] 420mm shell (sound, flight time, damage, blast radius)
- [ ] Sound-based warning system (1-30 sec by shell type)
- [ ] Shell recognition minigame UI
- [ ] Explosion VFX (dirt, debris, shockwave)
- [ ] Blast damage calculation
- [ ] Shrapnel spread system
- [ ] Crater generation/deformation
- [ ] Screen shake/camera effects
- [ ] Tinnitus/deafness audio effects
- [ ] Bombardment pattern AI (harassing, light, heavy, drumfire)

### Combat System
- [ ] First-person shooting mechanics
- [ ] Lebel 1886 rifle (accuracy, recoil, reload)
- [ ] Rifle aiming/sights
- [ ] Ammo management
- [ ] Grenade throwing (physics arc)
- [ ] Bayonet melee combat
- [ ] Trench club melee
- [ ] Hit detection (body parts)
- [ ] Damage system
- [ ] Enemy AI (cover, suppression, flanking)
- [ ] Friendly AI (squadmates)
- [ ] Death/respawn system
- [ ] Checkpoint system

### Gas Warfare
- [ ] Gas cloud simulation (volumetric fog)
- [ ] Chlorine gas (yellow-green, effects)
- [ ] Phosgene gas (colorless, effects)
- [ ] Mustard gas (yellowish, blistering)
- [ ] Wind direction/speed system
- [ ] Gas dispersion physics
- [ ] Gas mask equipment
- [ ] Filter degradation system
- [ ] Gas damage over time
- [ ] Gas detection (visual, audio cues)
- [ ] Alarm bell system
- [ ] Gas mask FOV restriction
- [ ] Breathing sounds (mask on/off)

### Character Controller
- [ ] First-person camera
- [ ] Walk/run movement
- [ ] Sprint (drains stamina)
- [ ] Crouch
- [ ] Prone
- [ ] Lean (left/right)
- [ ] Jump/climb
- [ ] Ladder climbing
- [ ] Wade through mud (slowed movement)
- [ ] Stamina affects movement speed
- [ ] Alertness affects camera shake
- [ ] Injury affects movement
- [ ] Look/mouse sensitivity settings

### Weather & Time
- [ ] Day/night cycle (accurate Verdun sunrise/sunset)
- [ ] Time acceleration system
- [ ] Monthly weather states (Feb-Dec 1916)
- [ ] Rain system
- [ ] Snow system
- [ ] Fog system
- [ ] Wind system
- [ ] Overcast sky (default state)
- [ ] Temperature system
- [ ] Mud accumulation
- [ ] Flooding mechanics
- [ ] Weather affects survival meters

---

## 3D ASSETS

### Terrain
- [ ] 2 km² heightmap (from SRTM data)
- [ ] Landscape material (mud, chalk, grass, rock)
- [ ] Mud texture (wet, dry, frozen variations)
- [ ] Chalk texture (Verdun-specific white chalk)
- [ ] Crater meshes (various sizes)
- [ ] Shell hole decals
- [ ] Terrain deformation system
- [ ] Trench floor materials
- [ ] No-man's-land ground cover
- [ ] Destroyed vegetation
- [ ] Water puddles/flooding

### Trenches
- [ ] Front-line trench segments (firestep, parapet)
- [ ] Communication trench segments
- [ ] Support trench segments
- [ ] Traverse sections (zigzag)
- [ ] Dugout interiors (small, medium, large)
- [ ] Command bunker
- [ ] Aid station
- [ ] Latrine
- [ ] Ammunition storage
- [ ] Observation post
- [ ] Machine gun nest
- [ ] Sniper position
- [ ] Duckboards (wooden walkways)
- [ ] Sandbag walls
- [ ] Wooden support beams
- [ ] Corrugated iron roofing
- [ ] Trench ladders
- [ ] Periscope stand

### Fort Douaumont
- [ ] Exterior concrete structure
- [ ] Turret emplacements
- [ ] Entrance/exit points
- [ ] Damaged sections (shell impacts)
- [ ] Rubble piles
- [ ] Interior corridors (if accessible)
- [ ] Underground tunnels (if accessible)
- [ ] Barbed wire perimeter
- [ ] Defensive positions

### Props - Military
- [ ] Barbed wire (coiled, stretched)
- [ ] Wire stakes
- [ ] Sandbags (stacked, loose)
- [ ] Ammunition crates
- [ ] Rifle racks
- [ ] Grenade boxes
- [ ] Artillery shells (stacked)
- [ ] Shell casings
- [ ] Helmets (Adrian, damaged)
- [ ] Gas masks (hanging, worn)
- [ ] Stretchers
- [ ] Medical crates
- [ ] Water cans
- [ ] Food crates
- [ ] Fuel drums
- [ ] Periscopes
- [ ] Binoculars
- [ ] Maps/papers
- [ ] Field telephones
- [ ] Signal lamps
- [ ] Flare pistols

### Props - Environmental
- [ ] Destroyed trees (various sizes)
- [ ] Tree stumps (shattered)
- [ ] Shattered wood debris
- [ ] Brick rubble
- [ ] Concrete chunks
- [ ] Mud puddles
- [ ] Rats (animated)
- [ ] Corpses (respectful, covered)
- [ ] Wooden crates (generic)
- [ ] Metal debris
- [ ] Unexploded shells
- [ ] Barrels
- [ ] Chairs
- [ ] Tables
- [ ] Beds/cots
- [ ] Braziers (fire source)
- [ ] Lanterns
- [ ] Candles
- [ ] Letters/photographs (props)

### Weapons
- [ ] Lebel 1886 rifle (French)
- [ ] Gewehr 98 rifle (German)
- [ ] Hotchkiss M1914 machine gun
- [ ] MG08 machine gun (German)
- [ ] F1 grenade (French)
- [ ] Stielhandgranate (German)
- [ ] Bayonet (Lebel)
- [ ] Trench club
- [ ] Entrenching tool
- [ ] Signal flare gun
- [ ] Flamethrower (if included)

### Food & Consumables
- [ ] Bread loaf
- [ ] Hard tack biscuits
- [ ] Canned beef (singe)
- [ ] Canned sardines
- [ ] Cheese wedge
- [ ] Chocolate bar
- [ ] Wine bottle
- [ ] Coffee tin
- [ ] Sugar cubes
- [ ] Water bottle/canteen
- [ ] Cigarettes
- [ ] Bandages
- [ ] Morphine syringe
- [ ] Iodine bottle
- [ ] Alcohol flask

### Characters - French
- [ ] Jean-Baptiste Moreau (protagonist)
- [ ] Paul Durand (best friend)
- [ ] Sergeant Claude Renard
- [ ] Médecin-Major Leclerc
- [ ] Additional squadmate #5
- [ ] Background soldiers (variations)
- [ ] French officers
- [ ] French uniform (horizon blue)
- [ ] Adrian helmet
- [ ] Greatcoat
- [ ] Puttees (leg wraps)
- [ ] Boots
- [ ] Facial variations (age, features)
- [ ] Dirt/blood decals
- [ ] Wounded variations

### Characters - German
- [ ] German soldiers (enemy models)
- [ ] German officers
- [ ] Feldgrau uniform
- [ ] Pickelhaube helmet
- [ ] Stahlhelm (if 1916 late)
- [ ] Facial variations

### Animations - Character
- [ ] Idle (standing, tired variations)
- [ ] Walk
- [ ] Run
- [ ] Sprint
- [ ] Crouch
- [ ] Prone
- [ ] Crouch walk
- [ ] Crawl
- [ ] Jump/climb
- [ ] Ladder climb
- [ ] Lean left/right
- [ ] Aim rifle
- [ ] Fire rifle
- [ ] Reload rifle
- [ ] Throw grenade
- [ ] Bayonet attack
- [ ] Melee attack
- [ ] Eat food
- [ ] Drink water
- [ ] Put on gas mask
- [ ] Take off gas mask
- [ ] Bandage self
- [ ] Inject morphine
- [ ] Take cover (duck)
- [ ] Flinch (near miss)
- [ ] Stumble (exhausted)
- [ ] Fall (wounded)
- [ ] Death (various types)
- [ ] Surrender
- [ ] Talking (conversation)
- [ ] Smoking
- [ ] Writing letter
- [ ] Sleeping

---

## AUDIO

### Artillery Sounds
- [ ] 75mm incoming whistle (high pitch, 10-15 sec)
- [ ] 155mm incoming rumble (medium, 15-20 sec)
- [ ] 210mm incoming roar (low, 20-25 sec)
- [ ] 305mm incoming thunder (very low, 25-30 sec)
- [ ] 420mm incoming earthquake (ultra low, 30+ sec)
- [ ] Explosion - 75mm (close, medium, far)
- [ ] Explosion - 155mm (close, medium, far)
- [ ] Explosion - 210mm (close, medium, far)
- [ ] Explosion - 305mm (close, medium, far)
- [ ] Explosion - 420mm (close, medium, far)
- [ ] Shrapnel whizzing sounds
- [ ] Shrapnel impacts (ground, metal, wood)
- [ ] Shockwave thump
- [ ] Debris falling
- [ ] Tinnitus/ear ringing

### Weapons
- [ ] Lebel rifle fire
- [ ] Lebel reload sounds
- [ ] Rifle bolt action
- [ ] Bullet whizz-by
- [ ] Bullet impacts (flesh, dirt, wood, metal)
- [ ] Machine gun bursts (Hotchkiss, MG08)
- [ ] Grenade pin pull
- [ ] Grenade throw
- [ ] Grenade explosion
- [ ] Bayonet stab
- [ ] Melee swing/impact
- [ ] Empty rifle click

### Environmental Ambience
- [ ] Distant artillery (constant background)
- [ ] Distant machine gun fire
- [ ] Distant rifle shots
- [ ] Wind (light, moderate, strong)
- [ ] Rain (light, heavy)
- [ ] Thunder
- [ ] Mud squelching (footsteps)
- [ ] Water dripping
- [ ] Wood creaking
- [ ] Metal clanging
- [ ] Rats squeaking
- [ ] Flies buzzing
- [ ] Fire crackling (braziers)
- [ ] Gas alarm bells

### Character Vocals
- [ ] Jean-Baptiste breathing (normal, heavy, panicked)
- [ ] Jean-Baptiste pain sounds
- [ ] Jean-Baptiste death sounds
- [ ] Paul Durand voice lines (500+ lines)
- [ ] Sergeant Renard voice lines (300+ lines)
- [ ] Médecin-Major Leclerc voice lines (200+ lines)
- [ ] Squadmate #5 voice lines (200+ lines)
- [ ] Background soldier chatter
- [ ] French officer commands
- [ ] German soldier shouts (enemy)
- [ ] Wounded cries (French, German)
- [ ] Death screams
- [ ] Coughing (gas, illness)
- [ ] Vomiting sounds
- [ ] Eating/drinking sounds
- [ ] Footsteps (mud, wood, concrete)

### UI/System Sounds
- [ ] Menu button clicks
- [ ] Menu navigation
- [ ] Item pickup
- [ ] Item use
- [ ] Inventory open/close
- [ ] Health warning (critical meter)
- [ ] Shell warning indicator
- [ ] Mission objective complete
- [ ] Save game sound
- [ ] Achievement unlock

### Music (Optional)
- [ ] Main menu theme
- [ ] Mission complete theme
- [ ] Death/game over theme
- [ ] Period French songs (La Marseillaise, etc.)
- [ ] Ambient score (minimal, mournful)

---

## MISSIONS

### Mission 1: Opening Bombardment (Feb 21, 1916)
- [ ] Level design (rear trench sector)
- [ ] Mission script (timeline, events)
- [ ] Tutorial prompts (controls, meters)
- [ ] NPC spawn points (Paul, Renard, others)
- [ ] Dialogue triggers (5 characters)
- [ ] Dialogue writing (500-1,000 lines)
- [ ] Bombardment sequence (0715 hours start)
- [ ] Scripted artillery strikes
- [ ] Survival objective (24 hours)
- [ ] Safe zones (dugouts)
- [ ] Food/water pickup locations
- [ ] Success/failure conditions
- [ ] Mission end transition

### Mission 2: Fort Douaumont Falls (Feb 25, 1916)
- [ ] Level design (support trench, view of fort)
- [ ] Mission script (witness event)
- [ ] Fort Douaumont visible in distance
- [ ] Distant explosion VFX
- [ ] Runner NPC (reports fort lost)
- [ ] Dialogue (shock, disbelief)
- [ ] Morale meter drop event
- [ ] Night retreat sequence
- [ ] German positions visible
- [ ] Success/failure conditions
- [ ] Mission end transition

### Mission 3: Burial Detail (March 3-6, 1916)
- [ ] Level design (no-man's-land at night)
- [ ] Mission script (body recovery)
- [ ] Corpse prop placement (respectful)
- [ ] Paul breakdown sequence
- [ ] Comfort/ignore choice dialogue
- [ ] Hygiene meter tank event
- [ ] Morale meter drop event
- [ ] Artillery hazards
- [ ] Sniper hazards
- [ ] Getting lost mechanic
- [ ] Success/failure conditions
- [ ] Mission end transition

### Mission 4: Night Raid (March 10-12, 1916)
- [ ] Level design (no-man's-land patrol)
- [ ] Mission script (repair barbed wire)
- [ ] Shell recognition tests (5-10 instances)
- [ ] Shell warning UI
- [ ] Cover positions
- [ ] Optional German patrol encounter
- [ ] Stealth mechanics
- [ ] Detection system (alertness-based)
- [ ] Engage/hide choice
- [ ] Success/failure conditions
- [ ] Mission end transition

### Mission 5: Gas Attack (March 18-21, 1916)
- [ ] Level design (front-line trench)
- [ ] Mission script (routine → gas → counterattack)
- [ ] Gas cloud approach VFX
- [ ] Gas alarm sequence
- [ ] Gas mask requirement check
- [ ] Gas survival section (30 min)
- [ ] Filter degradation mechanic
- [ ] Replacement filter locations
- [ ] German counterattack AI
- [ ] Defense objectives
- [ ] Success/failure conditions
- [ ] Mission end transition

---

## UI/UX

### HUD
- [ ] Survival meter display (7 meters)
- [ ] Meter icons (hunger, thirst, stamina, warmth, hygiene, morale, alertness)
- [ ] Meter bars/fill indicators
- [ ] Color coding (green, yellow, red)
- [ ] Critical warning flashes
- [ ] Stamina bar (below meters)
- [ ] Health/damage indicator
- [ ] Ammo counter
- [ ] Grenade count
- [ ] Compass/direction indicator
- [ ] Shell warning indicator (directional)
- [ ] Shell recognition timer
- [ ] Objective text display
- [ ] Interaction prompts (context-sensitive)
- [ ] Crosshair (minimal, iron sights)

### Menus
- [ ] Main menu background (historical photo)
- [ ] Main menu buttons (New Game, Continue, Options, Codex, Credits, Quit)
- [ ] Chapter/mission select screen
- [ ] Mission briefing screen (date, location, objectives)
- [ ] Pause menu (Resume, Save, Load, Settings, Quit)
- [ ] Settings menu (Graphics, Audio, Controls)
- [ ] Graphics settings (resolution, quality presets, V-sync, FPS cap)
- [ ] Audio settings (master, music, SFX, dialogue volumes)
- [ ] Control settings (mouse sensitivity, keybindings, controller)
- [ ] Accessibility options (subtitles, colorblind modes)
- [ ] Save/load menu (multiple slots)
- [ ] Inventory screen (grid layout, item details)
- [ ] Character status screen (meter details, injuries)
- [ ] Map screen (trench network, current location)

### Historical Codex
- [ ] Codex menu structure (categories)
- [ ] Battles category (20+ entries)
- [ ] Locations category (80+ entries)
- [ ] Weapons category (15+ entries)
- [ ] People category (45+ character bios)
- [ ] Historical documents (letters, orders, photos)
- [ ] Unlock conditions per entry
- [ ] Entry text (500 words each, ~15,000 words total)
- [ ] Historical photos (public domain)
- [ ] Source citations
- [ ] Entry read/unread tracking

### Dialogue System
- [ ] Dialogue box UI
- [ ] Character name display
- [ ] Dialogue text (with line breaks)
- [ ] Subtitle display (always on for accessibility)
- [ ] Dialogue choice buttons (2-4 options)
- [ ] Choice consequence preview (if applicable)
- [ ] Relationship status indicator (trust/fear)
- [ ] Skip dialogue button
- [ ] Auto-advance option

---

## VFX (Visual Effects)

### Explosions
- [ ] 75mm explosion (small, dirt/debris)
- [ ] 155mm explosion (medium, larger debris cloud)
- [ ] 210mm explosion (large, shockwave ring)
- [ ] 305mm explosion (very large, ground shake)
- [ ] 420mm explosion (massive, delayed shockwave)
- [ ] Dirt/mud ejection particles
- [ ] Smoke plume (rising, dissipating)
- [ ] Fire/embers
- [ ] Shrapnel sparks
- [ ] Screen shake (distance-based)
- [ ] Camera blur (on nearby hit)
- [ ] Screen dirt/mud splatter

### Gas Clouds
- [ ] Chlorine gas (yellow-green volumetric fog)
- [ ] Phosgene gas (colorless, subtle)
- [ ] Mustard gas (yellowish, ground-hugging)
- [ ] Gas dispersion (wind-driven)
- [ ] Gas density variation
- [ ] Gas mask FOV vignette
- [ ] Gas mask lens fogging

### Weather
- [ ] Rain particles (light, moderate, heavy)
- [ ] Raindrops on camera
- [ ] Snow particles
- [ ] Snowflakes on camera
- [ ] Fog (volumetric)
- [ ] Fog density animation
- [ ] Lightning flashes
- [ ] Overcast sky (dynamic clouds)

### Combat
- [ ] Muzzle flash (rifle, machine gun)
- [ ] Bullet tracers
- [ ] Bullet impact sparks (metal)
- [ ] Bullet impact dust (concrete)
- [ ] Blood splatter (minimal, respectful)
- [ ] Grenade explosion
- [ ] Smoke grenade

### Environmental
- [ ] Fire (braziers, burning debris)
- [ ] Smoke (fire, cigarettes)
- [ ] Water splash (puddles)
- [ ] Mud splash (footsteps)
- [ ] Dust clouds (collapse, movement)
- [ ] Flare light (parachute flare)
- [ ] Lantern light (flickering)
- [ ] Breath mist (cold weather)

### Post-Processing
- [ ] Shell shock effect (desaturation, blur, chromatic aberration)
- [ ] Exhaustion effect (tunnel vision, slow-motion)
- [ ] Hypothermia effect (blue tint, vignette)
- [ ] Starvation effect (blur, desaturation)
- [ ] Normal state (grim, desaturated baseline)
- [ ] Night vision adjustment (darkness, flare bloom)
- [ ] Gas vision (through mask, restricted FOV)

---

## SYSTEMS & TECHNICAL

### Game Progression
- [ ] Campaign calendar (Feb 21 - March 21, 1916)
- [ ] Date/time tracking system
- [ ] Mission unlock logic
- [ ] Mission complete tracking
- [ ] Story state tracking
- [ ] Character relationship values
- [ ] Player choices tracking
- [ ] Statistics tracking (deaths, shells survived, etc.)

### Save System
- [ ] Save game data structure
- [ ] Auto-save triggers (end of mission, checkpoints)
- [ ] Manual save (pause menu)
- [ ] Multiple save slots (3-5)
- [ ] Save slot info display (date, mission, playtime)
- [ ] Load game functionality
- [ ] Delete save functionality
- [ ] Save file validation/corruption handling

### AI Systems
- [ ] Squadmate AI (follow player, take cover, fire at enemies)
- [ ] Enemy AI (patrol, detect player, engage, take cover)
- [ ] NPC conversation AI (idle behaviors, responses)
- [ ] Panic behavior (low morale)
- [ ] Rally behavior (recovering morale)
- [ ] Wounded behavior (calling for help, crawling)
- [ ] Death behavior (ragdoll, final words)

### Performance Optimization
- [ ] LOD system (terrain, meshes, characters)
- [ ] Occlusion culling
- [ ] Draw call optimization
- [ ] Texture streaming
- [ ] Audio occlusion
- [ ] Particle budgeting
- [ ] Target: 60 FPS @ 1440p on GTX 1660 / Ryzen 5

### Project Setup
- [ ] UE5 project creation (5.4+)
- [ ] Enable Nanite
- [ ] Enable Lumen
- [ ] Git repository setup
- [ ] Git LFS configuration
- [ ] .gitignore for UE5
- [ ] Folder structure (Content/OTD/)
- [ ] Asset naming conventions
- [ ] Blueprint/C++ organization

---

## CONTENT WRITING

### Dialogue
- [ ] Mission 1 dialogue (500-1,000 lines)
- [ ] Mission 2 dialogue (300-500 lines)
- [ ] Mission 3 dialogue (500-800 lines)
- [ ] Mission 4 dialogue (300-500 lines)
- [ ] Mission 5 dialogue (500-800 lines)
- [ ] Ambient barks (100+ per character)
- [ ] Combat barks (50+ variations)
- [ ] Pain/wounded lines (50+ variations)
- [ ] Idle conversation (100+ lines)
- [ ] Total: 5,000-8,000 lines

### Codex Entries
- [ ] Opening Bombardment article (500 words)
- [ ] Fort Douaumont article (500 words)
- [ ] Battle of Verdun overview (1,000 words)
- [ ] Trench warfare tactics (500 words)
- [ ] Artillery article (500 words)
- [ ] Gas warfare article (500 words)
- [ ] Survival in trenches (500 words)
- [ ] French Army 1916 (500 words)
- [ ] German Army 1916 (500 words)
- [ ] Jean-Baptiste Moreau bio (300 words)
- [ ] Paul Durand bio (300 words)
- [ ] Sergeant Renard bio (300 words)
- [ ] Médecin-Major Leclerc bio (300 words)
- [ ] Lebel rifle entry (300 words)
- [ ] Gewehr 98 entry (300 words)
- [ ] Additional 15+ entries (300-500 words each)
- [ ] Total: ~15,000 words

### Mission Briefs
- [ ] Mission 1 briefing text (300 words)
- [ ] Mission 2 briefing text (300 words)
- [ ] Mission 3 briefing text (300 words)
- [ ] Mission 4 briefing text (300 words)
- [ ] Mission 5 briefing text (300 words)

---

## TESTING & QA

### Functional Testing
- [ ] All survival meters work correctly
- [ ] Meters drain at correct rates
- [ ] Death conditions trigger properly
- [ ] Inventory system works (add, remove, use items)
- [ ] Item consumption restores correct meter values
- [ ] Artillery sounds play with correct timing
- [ ] Shell recognition minigame works
- [ ] Explosions damage player correctly
- [ ] Cover reduces damage
- [ ] Gas system works (masks, filters, damage)
- [ ] Combat mechanics work (shooting, grenades, melee)
- [ ] AI behaves correctly (squadmates, enemies)
- [ ] All 5 missions completable start-to-finish
- [ ] Save/load works without corruption
- [ ] No game-breaking bugs

### Balance Testing
- [ ] Survival meters balanced (not too hard/easy)
- [ ] Food/water availability appropriate
- [ ] Artillery frequency balanced
- [ ] Shell recognition difficulty fair
- [ ] Combat difficulty appropriate
- [ ] Gas attack survivable but challenging
- [ ] Missions completable by average player

### Performance Testing
- [ ] 60 FPS on target hardware (GTX 1660, Ryzen 5)
- [ ] No significant frame drops
- [ ] Loading times acceptable (<30 sec)
- [ ] Memory usage within bounds (12-16 GB)
- [ ] No memory leaks
- [ ] Audio performance stable
- [ ] Particle systems optimized

### Compatibility Testing
- [ ] Works on Windows 10/11
- [ ] Works with various GPUs (NVIDIA, AMD)
- [ ] Works with various CPUs (Intel, AMD)
- [ ] Keyboard/mouse controls work
- [ ] Controller support works (Xbox, PlayStation)
- [ ] Different resolutions work (1080p, 1440p, 4K)
- [ ] Ultrawide monitor support
- [ ] Multi-monitor handling

---

## STEAM RELEASE

### Steam Setup
- [ ] Register Steamworks account ($100 fee)
- [ ] Create app ID
- [ ] Fill out store page info
- [ ] Upload build to Steam (depot setup)
- [ ] Configure Early Access settings
- [ ] Set up achievements (if any)
- [ ] Configure cloud saves
- [ ] Set up DLC structure (for future)

### Store Page Content
- [ ] Game title
- [ ] Tagline/short description
- [ ] Long description (500-1,000 words)
- [ ] Key features list (5-10 bullet points)
- [ ] Early Access disclaimer
- [ ] System requirements (minimum, recommended)
- [ ] 10-15 screenshots (1920x1080)
- [ ] Gameplay trailer (2-3 minutes)
- [ ] Announcement trailer (1-2 minutes)
- [ ] Header capsule image
- [ ] Small capsule image
- [ ] Main capsule image
- [ ] Store page background
- [ ] Logo
- [ ] Tags (Survival, Horror, Historical, WWI, etc.)
- [ ] Genre classifications
- [ ] ESRB/PEGI rating submission

### Marketing Assets
- [ ] Trailer script/storyboard
- [ ] Trailer footage capture (gameplay)
- [ ] Trailer editing
- [ ] Trailer music/sound design
- [ ] Screenshot selection (dramatic moments)
- [ ] Screenshot captions
- [ ] Press kit (logo, screenshots, fact sheet)
- [ ] Developer bio
- [ ] Game fact sheet

### Marketing Activities
- [ ] Create Twitter/X account
- [ ] Create subreddit or join r/indiegaming
- [ ] Create Discord server
- [ ] Create TikTok account
- [ ] Post regular dev updates (weekly)
- [ ] Create dev diary videos (10 episodes)
- [ ] Reach out to gaming press (RPS, PC Gamer, Kotaku)
- [ ] Reach out to history YouTubers
- [ ] Post in WW1 history forums
- [ ] Post in military sim communities
- [ ] Email educators/museum contacts
- [ ] Create presskit.zip for journalists
- [ ] Prepare review codes

### Launch Preparation
- [ ] Set launch date
- [ ] Set price ($14.99-19.99 Early Access)
- [ ] Plan launch discount (10-15% week 1?)
- [ ] Prepare day-1 patch
- [ ] Monitor reviews strategy
- [ ] Community management plan
- [ ] Support/bug report system
- [ ] Update schedule communication

---

## POST-LAUNCH

### Immediate Support
- [ ] Monitor Steam reviews
- [ ] Monitor Discord feedback
- [ ] Track bug reports
- [ ] Hotfix critical bugs (days 1-3)
- [ ] Respond to community questions
- [ ] Post launch thank-you message

### Patch 1.1 (Weeks 2-4)
- [ ] Fix reported bugs
- [ ] Balance adjustments based on telemetry
- [ ] QOL improvements from feedback
- [ ] Performance optimizations
- [ ] Patch notes

### Patch 1.2 (Months 2-3)
- [ ] Additional bug fixes
- [ ] Potential new mission (if funded)
- [ ] Additional codex entries
- [ ] Community-requested features

### Full Release (v2.0)
- [ ] Exit Early Access
- [ ] Add 5 more missions (total 10)
- [ ] Polish pass on all content
- [ ] Final bug sweep
- [ ] Price increase to $29.99
- [ ] Marketing push for 1.0 launch

### DLC Considerations
- [ ] German campaign (flip perspective)
- [ ] Battle of the Somme expansion
- [ ] 1917 Mutinies story DLC
- [ ] Additional missions packs

---

## TOTAL COUNTS

**Systems**: ~20 major systems
**3D Assets**: 500+ individual models/props
**Audio Files**: 1,000+ sound effects, 5,000-8,000 voice lines
**Missions**: 5 missions (3-5 hours gameplay)
**Dialogue**: 5,000-8,000 lines
**Written Content**: ~20,000 words (codex + briefs + dialogue)
**UI Screens**: 20+ unique screens/menus
**VFX**: 50+ unique effects
**Animations**: 60+ character animations

---

**TOTAL TASKS: ~800+ individual items**

Check off what you've completed. I'll help with whatever's next.
