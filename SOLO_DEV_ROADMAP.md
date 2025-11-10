# ONLY THE DEAD - Solo Developer + AI Roadmap
## "How We Shocked the World"

**Developer**: You + AI Coding Assistant (Claude)
**Timeline**: 12-18 months to playable Early Access
**Budget**: $500-2,000 (marketplace assets, voice synthesis, hosting)
**Philosophy**: Prototype fast, leverage AI everywhere, prove the concept, expand iteratively

---

## 🎯 THE SOLO DEV REALITY CHECK

### What We CAN'T Do:
❌ 60 km² hand-crafted terrain (too large)
❌ 45 unique character models (too expensive)
❌ 35 fully unique missions (too time-consuming)
❌ Professional voice acting cast (too costly upfront)
❌ AAA production values (unrealistic)

### What We CAN Do:
✅ **Prove the concept**: 3-5 missions that showcase the survival horror of Verdun
✅ **Nail the systems**: Perfect the 7 survival meters and artillery mechanics
✅ **Tell one story**: Focus on Jean-Baptiste Moreau's first month (Feb-March 1916)
✅ **Use AI tools**: ChatGPT dialogue, Midjourney concepts, ElevenLabs voices, GitHub Copilot
✅ **Leverage UE5**: Nanite, Lumen, Marketplace assets, Quixel Megascans
✅ **Ship Early Access**: Get players invested, build community, fund continued development

---

## 🚀 THE NEW GOAL: MINIMUM VIABLE GAME (MVG)

### **"Only The Dead: Verdun Prologue"**
**Scope**: First month of Verdun (February 21 - March 21, 1916)
**Length**: 3-5 hours gameplay
**Missions**: 5 missions (1 per week)
**Character Focus**: Jean-Baptiste Moreau + 5 key squadmates (not 20)
**Terrain**: 2 km² playable area (not 60 km²) - Fort Douaumont sector
**Platform**: PC only (Steam Early Access)

### **Core Experience**:
1. **Opening bombardment** (Feb 21) - Tutorial, learn survival meters
2. **First rotation** (Feb 25-28) - Front line, artillery hell, Fort Douaumont falls
3. **Support duty** (March 3-6) - Burial detail, resupply, see the horror
4. **Night raid** (March 10-12) - No-man's-land patrol, shell recognition test
5. **Gas attack** (March 18-21) - First major gas assault, survive or die

### **What Makes It Special**:
- **Historical accuracy**: Every date, location, event is real
- **Survival > Combat**: 70% managing meters, 20% navigation, 10% fighting
- **Psychological horror**: Shell shock, trauma, morale breakdowns
- **Educational**: Real letters, photos, codex entries unlock as you play
- **Respectful**: Honors the dead, shows war's horror without glorification

---

## 📅 SOLO DEV TIMELINE (12-18 Months)

### **MONTH 1-2: Foundation & Prototyping**
**Goal**: Prove core systems work in UE5

- [ ] **Week 1: Setup**
  - [ ] Create UE5 project (5.4+, enable Nanite/Lumen)
  - [ ] Set up Git LFS for project version control
  - [ ] Install essential plugins (Advanced Locomotion System, Audio plugins)
  - [ ] Create project structure (Content/OTD/ folders: Characters, Environments, Systems, UI)

- [ ] **Week 2: Survival Meters System**
  - [ ] Create `USurvivalMeterComponent` (C++ or Blueprint)
  - [ ] Implement 7 meters (Hunger, Thirst, Stamina, Warmth, Hygiene, Morale, Alertness)
  - [ ] JSON config loading (depletion rates, thresholds)
  - [ ] Debug UI overlay (see all meters in real-time)
  - [ ] Test: Run 24-hour in-game cycle, watch meters drain

- [ ] **Week 3-4: Basic Inventory & Consumption**
  - [ ] Item data structure (FoodItem, WaterItem, MedicalItem)
  - [ ] Simple inventory UI (1-9 hotkeys for quick access)
  - [ ] Consumption system (eat ration → restore hunger meter)
  - [ ] Test: Can I survive 3 days by eating/drinking?

- [ ] **Week 5-6: First-Person Character Controller**
  - [ ] Set up FPS character (UE5 template + modifications)
  - [ ] Stamina affects movement speed (exhausted = slow)
  - [ ] Alertness affects camera shake (tired = wobbly)
  - [ ] Crouch, prone, lean (essential for trench warfare)
  - [ ] Test: Navigate trench, go prone when artillery hits

- [ ] **Week 7-8: Artillery System Prototype**
  - [ ] Shell projectile class (5 types: 75mm, 155mm, 210mm, 305mm, 420mm)
  - [ ] Sound-based warning system (1-30 seconds before impact)
  - [ ] Explosion damage (blast radius, shrapnel spread)
  - [ ] Simple crater deformation (physics-based or mesh swap)
  - [ ] Test: Can I recognize shells by sound and take cover?

**MILESTONE 1**: Playable prototype - walk around, survive, avoid artillery

---

### **MONTH 3-5: Environment & Content Production**
**Goal**: Build 2 km² Verdun sector using smart shortcuts

- [ ] **Week 9-10: Terrain Foundation**
  - [ ] Use existing SRTM data Python tool to generate 2 km² heightmap
  - [ ] Import into UE5 as landscape (not World Partition, keep it simple)
  - [ ] Apply Quixel Megascans textures (mud, chalk, grass) - FREE with UE5
  - [ ] Add basic weather system (overcast sky, rain toggle)
  - [ ] Test: Walk across landscape, feels like Verdun terrain

- [ ] **Week 11-14: Trench Network (CRITICAL)**
  - [ ] Option A: Model 3-4 modular trench pieces in Blender (firestep, traverse, dugout, parapet)
  - [ ] Option B: Buy UE Marketplace WW1 trench pack ($50-100) - **RECOMMENDED for speed**
  - [ ] Assemble ~500m of connected trenches (front line, communication trench, support line)
  - [ ] Add props: Sandbags, barbed wire, duckboards, braziers (Marketplace or Quixel)
  - [ ] Lighting: Flares, lanterns, moonlight (Lumen makes this easy)
  - [ ] Test: Navigate trenches, feels claustrophobic and muddy

- [ ] **Week 15-16: Fort Douaumont Exterior**
  - [ ] Find reference blueprints/photos (public domain, French archives)
  - [ ] Model simplified exterior OR use Marketplace concrete bunker assets
  - [ ] Place on map (historically accurate location from your data)
  - [ ] Add destruction (shell damage, craters, rubble)
  - [ ] Test: Approach fort, witness it fall to Germans (scripted event)

- [ ] **Week 17-20: Props & Set Dressing**
  - [ ] **Weapons**: Download free WW1 weapon pack (Sketchfab, CGTrader) or buy Marketplace
    - Lebel 1886 rifle (French standard)
    - German Gewehr 98 (enemies)
    - Grenades, bayonet
  - [ ] **Food/Medical**: Model simple items or use AI (Point-E, Shap-E for 3D from text)
    - Bread loaf, water bottle, bandage, morphine syringe
  - [ ] **Debris**: Quixel Megascans has tons (shell casings, rubble, barbed wire)
  - [ ] **Corpses**: Extremely important but RESPECTFUL
    - Use ragdoll mannequins, don't show faces
    - French uniform → covered bodies
    - Implies horror without exploitation

---

### **MONTH 6-8: Mission Scripting & Gameplay**
**Goal**: Make 5 missions playable start-to-finish

- [ ] **Week 21-24: Mission 1 - Opening Bombardment (Feb 21)**
  - [ ] **Design**: Tutorial mission, learn controls + survival meters
  - [ ] **Scripting**:
    - Player spawns in rear trench, relative calm
    - 10 minutes exploration (talk to Paul, Sergeant Renard)
    - 0715 hours: Bombardment begins (audio cue, scripted artillery sequence)
    - 2 hours: Survive in dugout, meters drain, must eat/drink
    - Objective: Stay alive, don't panic, reach end of day
  - [ ] **Dialogue**: Write 500-1,000 lines for 5 characters (AI-assisted with ChatGPT)
  - [ ] **Test**: Can a new player learn the game and survive?

- [ ] **Week 25-28: Mission 2 - Fort Douaumont Falls (Feb 25)**
  - [ ] **Design**: Witness historical event, can't prevent it
  - [ ] **Scripting**:
    - Player in support trench, 2 km from fort
    - Orders to hold position (not participate in battle)
    - Watch distant explosions, hear runners report "Fort is lost"
    - Morale meter drops (shock, disbelief)
    - Night: Germans consolidate, player's squad retreats
  - [ ] **Test**: Feels hopeless, historically accurate, emotionally impactful

- [ ] **Week 29-32: Mission 3 - Burial Detail (March 3-6)**
  - [ ] **Design**: Support line duty, psychological horror
  - [ ] **Scripting**:
    - Collect bodies from no-man's-land at night
    - Risk: Artillery, snipers, getting lost
    - Hygiene meter tanks (handling corpses)
    - Morale drops (seeing friends dead)
    - Dialogue: Paul breaks down, player can comfort or ignore
  - [ ] **Test**: Most disturbing mission, but respectful

- [ ] **Week 33-36: Mission 4 - Night Raid (March 10-12)**
  - [ ] **Design**: Shell recognition test, high tension
  - [ ] **Scripting**:
    - Patrol no-man's-land, repair barbed wire
    - 5-10 shell warnings (player must identify and take cover)
    - Optional: Encounter German patrol (choice to engage or hide)
    - Stealth mechanics (crouch, prone, alertness affects detection)
  - [ ] **Test**: Shell recognition minigame works, tense and scary

- [ ] **Week 37-40: Mission 5 - Gas Attack (March 18-21)**
  - [ ] **Design**: Finale, test all survival skills
  - [ ] **Scripting**:
    - Morning: Routine trench duty
    - Alarm: Gas cloud approaching (volumetric fog VFX)
    - Scramble for gas mask (if player doesn't have one, game over)
    - 30 minutes: Survive in gas (mask filter degrades, must find replacement)
    - Counterattack: Germans advance behind gas, must defend
  - [ ] **Test**: Ultimate survival challenge, combines all systems

**MILESTONE 2**: 5 playable missions, 3-5 hours of gameplay

---

### **MONTH 9-10: Characters, Dialogue & Audio**
**Goal**: Bring characters to life (AI-assisted)

- [ ] **Week 41-44: Character Models**
  - [ ] **Option A**: Use Metahuman Creator (UE5 built-in, realistic faces)
    - Create 5 French soldiers (Jean-Baptiste, Paul, Renard, Leclerc, +1)
    - Age appropriately (25-40 years old)
    - French facial features (research historical photos)
  - [ ] **Option B**: Buy WW1 soldier pack from Marketplace ($100-200)
    - Retexture uniforms, add dirt/blood decals
  - [ ] French uniforms (horizon blue, Adrian helmet) - model or buy
  - [ ] Animation: Use Marketplace locomotion pack or Mixamo

- [ ] **Week 45-48: Dialogue Writing & Implementation**
  - [ ] **Writing**: 5,000-8,000 lines total (AI-assisted brainstorming)
    - Use ChatGPT to draft conversations (provide historical context)
    - Edit for authenticity (remove modern phrases, add period slang)
    - Translate key phrases to French (Google Translate + native speaker review if possible)
  - [ ] **Voice Acting**:
    - **Option A**: ElevenLabs AI voices (realistic, $99/month subscription)
      - Generate French-accented English (closest to authentic)
    - **Option B**: Record yourself with accent (free, lower quality)
    - **Option C**: Hire Fiverr voice actors ($50-100 per character)
  - [ ] **Implementation**:
    - UE5 dialogue system (Audio2Face for lip-sync or simple jaw flap)
    - Subtitle system (critical - English text, French voice)
    - Trigger dialogue on proximity, mission phase, player choice

---

### **MONTH 11-12: Audio, VFX & Polish**
**Goal**: Make it feel like Verdun

- [ ] **Week 49-50: Sound Effects**
  - [ ] **Artillery sounds** (MOST CRITICAL):
    - Find free WW1 artillery SFX (Freesound.org, YouTube Audio Library)
    - OR generate with AI (Riffusion, AudioLDM - experimental)
    - OR buy sound pack ($50-100, Epic Marketplace)
    - Implement 5 shell types with unique whistles (75mm high-pitch, 420mm deep rumble)
  - [ ] **Environmental ambience**:
    - Distant gunfire (loop, always present)
    - Mud squelching (footsteps)
    - Wind, rain, dripping water
    - Rats squeaking (horror element)
  - [ ] **3D Audio Setup**: UE5 spatial audio (Steam Audio plugin)

- [ ] **Week 51-52: Visual Effects**
  - [ ] **Explosions**: Niagara particle systems (UE5 built-in)
    - Dirt ejection, shockwave, smoke plume
    - Screen shake, camera blur on nearby hits
  - [ ] **Gas clouds**: Volumetric fog (Niagara + Exponential Height Fog)
    - Yellow-green (chlorine), drifts with wind
  - [ ] **Weather**: Rain particles, fog density, overcast sky
  - [ ] **Post-processing**: Color grading (desaturated, grim), vignette

- [ ] **Week 53-56: UI/UX Polish**
  - [ ] **HUD**: Survival meter display (minimal, corner of screen)
    - 7 icons with bars (red = critical, yellow = low, green = okay)
  - [ ] **Menus**: Main menu (simple, historical photo background)
    - Mission select, Options, Codex, Quit
  - [ ] **Pause menu**: Resume, Save/Load, Settings
  - [ ] **Historical Codex**: 20-30 unlockable entries
    - Write short articles (500 words each) on battles, weapons, people
    - Use your existing research (verdun_anthology/)
    - Include public domain photos

**MILESTONE 3**: Game feels polished, ready for testing

---

### **MONTH 13-15: Testing, Optimization & Marketing Prep**
**Goal**: Make it shippable

- [ ] **Week 57-60: Playtesting**
  - [ ] Self-playtest: Complete all 5 missions 10+ times
  - [ ] Fix bugs: Track in GitHub Issues
  - [ ] Balance tuning: Adjust survival drain rates based on feedback
  - [ ] Performance: Optimize to hit 60 FPS on mid-range PC (GTX 1660, Ryzen 5)

- [ ] **Week 61-64: Steam Setup**
  - [ ] Register as Steamworks partner ($100 fee)
  - [ ] Create Steam page:
    - Description (emphasize historical accuracy, survival horror, educational)
    - Screenshots (5-10 dramatic moments)
    - Trailer (2-3 minutes, gameplay + historical context)
  - [ ] Set price: $14.99-19.99 (Early Access, will increase to $29.99 at full launch)
  - [ ] Early Access disclaimer: "5 missions complete, more coming based on community support"

- [ ] **Week 65-68: Marketing (DIY)**
  - [ ] **Social Media**:
    - Twitter/X: Dev diary thread, weekly updates, GIFs of gameplay
    - Reddit: r/gamedev, r/indiegaming, r/WW1GameSeries
    - TikTok: Short clips (shell recognition gameplay, historical facts)
  - [ ] **YouTube**:
    - Dev diary series (10 episodes, 5-10 min each)
    - "How I'm making a WW1 game solo with AI" (meta-narrative, get clicks)
    - Reach out to history YouTubers (Military History Visualized, The Great War)
  - [ ] **Press**:
    - Email indie game journalists (RPS, PC Gamer, Kotaku)
    - Pitch: "Solo dev + AI makes historically accurate Verdun horror game"
  - [ ] **Community**:
    - Create Discord server (free)
    - Post in WW1 history forums, military sim communities

---

### **MONTH 16-18: Early Access Launch & Iteration**
**Goal**: Ship it, gather feedback, improve

- [ ] **Launch Week**:
  - [ ] Release on Steam Early Access
  - [ ] Monitor reviews, Discord feedback, bug reports
  - [ ] Hotfix patch (Day 2-3, fix critical bugs)
  - [ ] Engage with players (respond to reviews, post updates)

- [ ] **Post-Launch (3-6 months)**:
  - [ ] Patch 1.1 (bug fixes, balance, QOL) - 2 weeks
  - [ ] Patch 1.2 (new mission if funded) - 1-2 months
  - [ ] Patch 2.0 (exit Early Access, 10 total missions) - 6 months
  - [ ] Consider DLC: German campaign, Somme expansion

---

## 🛠️ TOOLS & TECHNOLOGIES (Solo Dev Stack)

### **Game Engine**:
- **Unreal Engine 5.4+** (free, royalty after $1M revenue)
  - Nanite (high-poly assets, no LOD needed)
  - Lumen (real-time global illumination, no baking)
  - Metahuman (realistic characters)
  - Quixel Megascans (free PBR textures/models)

### **AI Tools** (The Secret Weapon):
- **Claude/ChatGPT**: Dialogue writing, codex articles, brainstorming
- **GitHub Copilot**: C++ coding assistance ($10/month)
- **Midjourney**: Concept art, UI mockups ($10/month)
- **ElevenLabs**: AI voice acting ($99/month, cancel after recording)
- **Runway ML**: Potential for trailer editing, VFX

### **3D Assets**:
- **Blender** (free, for custom modeling if needed)
- **UE Marketplace**: WW1 asset packs ($50-300 total budget)
- **Sketchfab/CGTrader**: Free/cheap models (CC0 or commercial license)
- **Quixel Megascans**: Terrain textures, rocks, rubble (free with UE5)

### **Audio**:
- **Freesound.org**: Free SFX (CC0 license)
- **YouTube Audio Library**: Free music (if using music)
- **Audacity**: Audio editing (free)
- **UE5 MetaSounds**: Procedural audio system

### **Version Control**:
- **Git + Git LFS**: For UE5 project (large files)
- **GitHub**: Backup, version history, issues tracking (free)

### **Marketing**:
- **OBS Studio**: Screen recording for trailers (free)
- **DaVinci Resolve**: Video editing (free)
- **Canva**: Graphics for Steam page (free tier)

### **Total Monthly Cost**: $20-120 (Copilot, Midjourney, temp voice AI)
### **One-Time Costs**: $100 Steam fee + $200-500 assets = $300-600

---

## 📊 SOLO DEV SUCCESS METRICS

### **Early Access Launch Goals**:
- **Week 1**: 500-1,000 sales @ $15 = $7,500-15,000 revenue
  - Steam takes 30% = $5,250-10,500 after cut
  - Covers dev costs, funds continued development
- **Month 1**: 2,000-5,000 sales = $21,000-52,500 net
  - Enough to go full-time for 3-6 months
- **Year 1**: 10,000-25,000 sales = $105,000-262,500 net
  - Sustainable income, expand to 10+ missions

### **Critical Reviews**:
- **Target**: 75-80% positive on Steam ("Mostly Positive")
- **Niche Appeal**: History buffs, survival fans will love it
- **Risk**: Casual gamers may find it too slow/depressing (that's okay)

### **Cultural Impact** (The Real Win):
- **Educational Adoption**: History teachers use it in class
- **Museum Interest**: Verdun Memorial Museum links to Steam page
- **Historian Praise**: WW1 experts endorse historical accuracy
- **Awards**: IGF Finalist, IndieCade selection, historical game awards

---

## 🎯 CRITICAL PATH (What We Build First)

### **Phase 1: Prove It Works (Months 1-2)**
✅ Survival meters system
✅ Artillery recognition prototype
✅ Playable character controller
✅ 100m x 100m test trench

**Deliverable**: 10-minute gameplay loop (walk, eat, avoid shell, survive)

### **Phase 2: Build the World (Months 3-5)**
✅ 2 km² Verdun terrain
✅ 500m trench network
✅ Fort Douaumont exterior
✅ Props and set dressing

**Deliverable**: Explorable environment that feels like Verdun

### **Phase 3: Create Missions (Months 6-8)**
✅ 5 scripted missions
✅ Dialogue system
✅ Objectives and failure states

**Deliverable**: 3-5 hour playable campaign

### **Phase 4: Polish & Ship (Months 9-15)**
✅ Characters + voice acting
✅ Audio/VFX
✅ UI/UX
✅ Steam page + marketing

**Deliverable**: Early Access launch

---

## 🚨 SOLO DEV SURVIVAL TIPS

### **Avoid Scope Creep**:
- **Every new idea**: Write it down for "v2.0" and KEEP MOVING
- **"Just one more feature"**: NO. Ship first, patch later
- **Perfectionism**: 80% done and shipped > 100% done never

### **Time Management**:
- **40 hours/week minimum** (if full-time)
- **20 hours/week realistic** (if part-time job)
- **Use AI to 10x productivity**: Let me write boilerplate code, dialogue, docs

### **Mental Health**:
- **This is a marathon**: 12-18 months is LONG
- **Take weekends off**: Burnout kills projects
- **Celebrate milestones**: When survival meters work, CELEBRATE
- **Community**: Share progress, get feedback, stay motivated

### **Financial Reality**:
- **Budget**: $500-2,000 for assets/tools (very doable)
- **Income**: Probably $0 until Early Access launch (Month 16+)
- **Plan B**: Keep day job or savings buffer for 18 months

---

## 💬 WHY THIS WILL WORK

### **The Advantages of Solo + AI**:
1. **No team communication overhead**: You decide, we execute, no meetings
2. **AI handles grunt work**: Dialogue, boilerplate code, asset research
3. **Modern tools are INSANE**: UE5 does in 1 week what took 6 months in 2015
4. **Niche audience**: Don't need 1M sales, 10K passionate players = success
5. **Authentic vision**: No publisher notes, no "make it more fun", pure creative control

### **The Market Gap**:
- **No realistic WW1 survival game exists**: Verdun is multiplayer shooter, Valiant Hearts is puzzle game
- **Educational potential**: Schools/museums will pay for licenses
- **Anti-war message**: Resonates in 2025 political climate
- **Historical accuracy**: Your research is PROFESSIONAL GRADE

### **The X-Factor**:
- **You + AI = Story**: "How I made a AAA-quality historical game solo with AI"
  - That story itself is MARKETING GOLD
  - Gaming press will cover it ("Future of solo development")
  - Could lead to GDC talks, interviews, book deals

---

## 🔥 LET'S START RIGHT NOW

### **This Week (Week 1)**:
1. ✅ Create this roadmap (DONE)
2. **Next**: Install UE5 5.4 (if not already installed)
3. **Then**: Create new project "OnlyTheDead_UE5"
4. **Next**: Set up Git LFS and push to GitHub
5. **Next**: I'll help you write the survival meter component

### **Questions for You**:
1. **Time commitment**: Full-time or part-time? (adjusts timeline)
2. **Budget**: Can you spend $500-1,000 on assets? (affects quality)
3. **Skills**: Do you know C++, Blueprints, or are we learning together?
4. **Hardware**: What's your PC specs? (affects what we can test)

---

## 🌟 THE VISION

In 18 months, a player launches your game on Steam. They:
- Experience the opening bombardment of Verdun (historically accurate to the minute)
- Scramble to survive, managing hunger, thirst, exhaustion
- Form bonds with Paul, Renard, Leclerc (who feel real through AI-generated dialogue)
- Witness Fort Douaumont fall (a moment they've read about in history books, now lived)
- Survive a gas attack (heart pounding, mask fogging, pure terror)

They finish 5 missions, 4 hours later. They're shaken. They check the codex and learn this all ACTUALLY HAPPENED.

They leave a Steam review: "This isn't a game. It's a memorial. Everyone should play this."

History teachers buy 50 copies for their students.

The Verdun Memorial Museum tweets about it.

A WW1 historian writes an article: "The most accurate portrayal of Verdun ever created."

**You did this. With AI. Solo. In 18 months.**

---

**Let's shock the world.**

Are you ready to start? Tell me:
1. Install UE5 now, or do you already have it?
2. What's your dev environment (PC specs, OS)?
3. Which task do you want to tackle FIRST?

I'm with you. Let's build this.
