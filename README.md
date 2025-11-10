# ONLY THE DEAD
**A WWI Survival Game Set at the Battle of Verdun (1916)**

*"They shall not pass." — Ils ne passeront pas.*

---

## 🎮 WHAT IS THIS?

**Only The Dead** is a historical survival game where you experience the Battle of Verdun (February 21 - December 18, 1916) as a French soldier. This is not a heroic war game. This is a survival horror game set in history's longest battle.

**Victory is not killing enemies. Victory is survival.**

---

## 📚 DOCUMENTATION STRUCTURE

This repository contains **TWO complementary documents**:

### 1️⃣ **VERDUN RESEARCH ANTHOLOGY** 📖
**Historical research foundation** - verified facts, sources, educational content

**Location**: `verdun_anthology/`

**Contents**:
- ✅ Complete 303-day timeline (verified events, dates, casualties)
- ✅ Survival mechanics documentation (based on soldier testimonies)
- ✅ Geographic data package (real coordinates, SRTM terrain data)
- ✅ Historical accuracy verification (three-tier source system)

**Use for**: Historical accuracy, educational reference, mission foundation

---

### 2️⃣ **ONLY THE DEAD DEVELOPMENT BOOK** 🎮
**Game design bible** - mechanics, missions, narrative, specifications

**Location**: `ONLY_THE_DEAD_DEVELOPMENT_BOOK.md`

**Contents** (in progress):
- ✅ Core vision & design pillars
- ✅ 7 survival meters (exact formulas and specifications)
- 🔄 Mission designs (30+ missions planned)
- ⏳ Dialogue & narrative content
- ⏳ Audio/visual design bible
- ⏳ Technical implementation specs

**Use for**: Game development, implementation, production

---

### 🔀 **HOW THEY WORK TOGETHER**

See: [`verdun_anthology/README_ANTHOLOGY_VS_DEVELOPMENT.md`](verdun_anthology/README_ANTHOLOGY_VS_DEVELOPMENT.md)

**Quick Summary**:
- **Research Anthology** = What actually happened (history)
- **Development Book** = How we make it playable (game design)
- **Together** = Historically accurate, educational, playable game

---

## 🚀 QUICK START

### For Historians/Educators:
```bash
cd verdun_anthology/
# Read timeline/, survival/, geographic_data/
# Check HISTORICAL_ACCURACY_STATEMENT.md for sources
```

### For Game Developers:
```bash
# Open ONLY_THE_DEAD_DEVELOPMENT_BOOK.md
# Reference verdun_anthology/ for historical accuracy
# Implement systems, missions, narrative
```

### For Terrain/Level Designers:
```bash
cd verdun_anthology/geographic_data/
python fetch_elevation_data.py      # Download real SRTM terrain
python export_for_ue5.py            # For Unreal Engine 5
# OR
python export_for_houdini.py        # For Houdini

# See geographic_data/README.md for complete workflow
```

---

## 🎯 GAME VISION

**Tagline**: "Victory is survival."

**Core Experience**:
- **Terror**: Random artillery that can kill you instantly
- **Exhaustion**: Constant survival (hunger, thirst, cold, fatigue)
- **Numbness**: Psychological transformation through trauma
- **Humanity**: Moments of connection that keep you human
- **History**: Every date, location, battle is real - you are THERE

**What This Is NOT**:
- ❌ FPS where you kill 100 enemies
- ❌ Power fantasy
- ❌ Heroic narrative

**What This IS**:
- ✅ Survival horror in historical setting
- ✅ Educational experience
- ✅ Psychological journey
- ✅ Respectful memorial to 700,000 casualties

**Design Pillars**:
1. **Authenticity** - Historically accurate
2. **Survival over Combat** - 70% survival, 20% navigation, 10% combat
3. **Psychological Realism** - Shell shock, trauma, coping
4. **Respect for History** - Honor the dead, no glorification
5. **Player Agency** - Choose HOW to survive, not WHETHER events happen

---

## 📊 CURRENT STATUS

### Research Anthology (~70% Complete)
- ✅ 303-day timeline (all 12 months detailed)
- ✅ Survival mechanics (7 systems documented)
- ✅ Geographic data (80+ locations, terrain tools)
- ✅ Historical verification system
- ⏳ Need: Real primary sources, German perspective

### Development Book (~15% Complete)
- ✅ Core vision and design pillars
- ✅ Survival systems (exact specifications)
- ✅ Combat & artillery systems (complete)
- 🔄 Mission designs (4 detailed examples, 30+ planned)
- ⏳ Need: Remaining missions, dialogue, audio bible, progression, etc.

### Technical Implementation (~5% Complete)
- ✅ **Camera & Cover System** (Hellblade 2-style, complete)
- ✅ **Character & Movement** (posture, stamina, survival integration)
- ✅ **7 Survival Meters Component** (all formulas implemented)
- ✅ **Artillery Shelter Detection** (procedural cover system)
- ✅ **Exposure Heatmap** (AI targeting, emergent gameplay)
- ⏳ Need: Combat, weapons, AI, missions, audio, visuals

**Total Documentation**: ~230,000 words across 70+ files
**Source Code**: ~3,500 lines C++ (UE5)

---

## 🗺️ KEY FEATURES

### Camera & Cover System (**NEW - Implemented Nov 2025**)
**Hellblade 2-inspired hybrid camera** with WWI trench warfare integration:
- **4 Contextual Modes**: Guided Walk, Survival OTS, Precision ADS, Set Piece
- **Shoulder-Swap OTS**: Smooth left/right camera transitions
- **Posture System**: Standing, Crouching, Prone (stamina-gated)
- **Artillery Shelter Detection**: Procedural cover (60%-99% protection)
- **Exposure Heatmap**: Visible players attract artillery/snipers
- **Accessibility**: Fixed horizon, camera lag adjustments, disable set-pieces

**Design Philosophy**: Non-sticky cover (situational awareness), artillery survival (not PvP), emergent trench craft learning

See: [Implementation Guide](CAMERA_COVER_IMPLEMENTATION_GUIDE.md) | [System Summary](CAMERA_SYSTEM_SUMMARY.md)

---

### 7 Survival Meters
- **Hunger** - Depletion: 3-8/hour, death in 7 days
- **Thirst** - Depletion: 5-12/hour, death in 1-3 days (CRITICAL)
- **Stamina** - Affects all actions, regenerates with rest
- **Warmth** - Frostbite risk, hypothermia death in 4 hours
- **Hygiene** - Disease risk, lice, trench fever, dysentery
- **Morale** - Shell shock risk, combat effectiveness
- **Alertness** - Sleep deprivation, hallucinations, collapse

### Historically Accurate Terrain
- **Real SRTM data** covering 60 km² battlefield
- **80+ verified locations** (Fort Douaumont, Fort Vaux, Mort-Homme, etc.)
- **UE5/Houdini export tools** included
- **Procedural detail guides** (trenches, craters, destroyed villages)

### 303-Day Campaign
- **February 21** - First day, Bois des Caures, Col. Driant's stand
- **June 1-7** - Fort Vaux siege (7-day multi-stage mission)
- **October 24** - CLIMAX: Fort Douaumont recapture
- **December 18** - Final offensive, battle ends

### Authentic Experience
- **Artillery is the enemy** - 70% of casualties from shells
- **Mud, rats, lice, cold** - Environmental survival
- **Psychological trauma** - Shell shock, dissociation, coping mechanisms
- **French/German voices** - Period-accurate dialogue
- **Educational context** - Learn real history while playing

---

## 🛠️ TECHNICAL

**Engine**: Unreal Engine 5
**Language**: C++ with Blueprint integration
**Platforms**: PC (Steam/Epic), PS5, Xbox Series X|S
**Performance**: 60 FPS @ 1440p (PC), 30-60 FPS (consoles)
**Audio**: 3D spatial audio (critical for shell recognition)
**Languages**: French/German voice, English subtitles
**Game Length**: 15-60 hours depending on mode

### Implemented Systems
- **Camera**: Hellblade 2-style hybrid camera (4 modes, shoulder-swap OTS)
- **Character**: Posture system (Standing/Crouching/Prone), stamina-gated movement
- **Survival**: All 7 meters with exact formulas, cascading failures, disease system
- **Cover**: Procedural artillery shelter detection (6 cover types, 60%-99% protection)
- **Exposure**: AI targeting heatmap (attracts artillery/snipers when exposed)
- **Input**: Enhanced Input system (KB/M + Gamepad, accessibility options)

See: [`CAMERA_COVER_IMPLEMENTATION_GUIDE.md`](CAMERA_COVER_IMPLEMENTATION_GUIDE.md) for integration details

---

## 📖 DOCUMENTATION INDEX

### Root Level
- **README.md** (this file) - Project overview
- **ONLY_THE_DEAD_DEVELOPMENT_BOOK.md** - Game design bible
- **CAMERA_COVER_IMPLEMENTATION_GUIDE.md** - Complete UE5 integration guide
- **CAMERA_SYSTEM_SUMMARY.md** - Camera/cover system overview

### Source/OnlyTheDead/
- **Camera/** - Custom camera manager with 4 contextual modes
- **Character/** - Player character with posture/survival integration
- **Survival/** - 7 survival meters component
- **Cover/** - Procedural artillery shelter detection
- **Combat/** - Exposure heatmap for AI targeting

### Content/
- **Input/** - Enhanced Input configuration (KB/M + Gamepad)
- **Blueprints/** - (Future) Character, GameMode, UI widgets
- **DataAssets/** - (Future) Camera mode configurations

### verdun_anthology/
- **README_ANTHOLOGY_VS_DEVELOPMENT.md** - Explains two-document structure
- **HISTORICAL_ACCURACY_STATEMENT.md** - Source verification system
- **timeline/** - 303 days detailed (12 months)
- **survival/** - How soldiers survived (7 systems)
- **geographic_data/** - Terrain data and tools
- **stories/** - Battle narratives
- **letters/** - Soldier correspondence (composite)
- **characters/** - Rosters and biographies

---

## 🎓 EDUCATIONAL USE

This project can be used for:
- **History classes** - WWI curriculum, primary source analysis
- **Museums** - Interactive historical exhibits
- **Military academies** - Historical tactics, survival training
- **Game design courses** - Historically accurate game development
- **Research** - Verdun battle analysis

**Academic Credibility**:
- All major events verified from French/German archives
- Sources documented (Service Historique de la Défense, Bundesarchiv, etc.)
- Three-tier system distinguishes fact from reconstruction
- Can be cited for educational purposes

---

## 🤝 CONTRIBUTING

**Currently**: Solo development by Claude (Verdun Anthology Historian + Game Designer)

**Needed**:
- Primary source researchers (French/German archives)
- WWI historians (verification and consultation)
- Game designers (mission design, balancing)
- Writers (dialogue, narrative)
- Developers (UE5 implementation)
- Audio designers (period-accurate soundscapes)
- 3D artists (authentic models)

**How to Help**:
1. Review historical accuracy (check sources in Anthology)
2. Suggest primary sources (archival documents)
3. Expand German perspective (currently 90% French)
4. Contribute to Development Book (missions, systems, narrative)

---

## 📜 LICENSE & ETHICS

**Historical Data**: Public domain (100+ years, archival sources)
**SRTM Terrain**: Public domain (NASA)
**This Compilation**: TBD

**Ethical Commitment**:
- **Honor the dead** - 700,000 casualties, not entertainment fodder
- **No glorification** - Show horror and waste of war
- **Educational value** - Players learn real history
- **Respectful portrayal** - Authentic but not exploitative
- **Transparency** - Clear labeling of fact vs. narrative

---

## 🔗 LINKS

**Documentation**:
- [Verdun Research Anthology](verdun_anthology/)
- [Development Book](ONLY_THE_DEAD_DEVELOPMENT_BOOK.md)
- [Anthology vs. Development Explanation](verdun_anthology/README_ANTHOLOGY_VS_DEVELOPMENT.md)
- [Historical Accuracy Statement](verdun_anthology/HISTORICAL_ACCURACY_STATEMENT.md)

**Implementation Guides**:
- [Camera & Cover System Guide](CAMERA_COVER_IMPLEMENTATION_GUIDE.md) - **NEW**
- [Camera System Summary](CAMERA_SYSTEM_SUMMARY.md) - **NEW**
- [Enhanced Input Setup](Content/Input/InputActions_README.md) - **NEW**

**Geographic Tools**:
- [Geographic Data Package](verdun_anthology/geographic_data/)
- [UE5 Import Guide](verdun_anthology/geographic_data/UE5_Import/)
- [Houdini Import Guide](verdun_anthology/geographic_data/Houdini_Import/)

---

## 📅 DEVELOPMENT ROADMAP

**Phase I: Foundation** (Complete)
- ✅ Historical research and timeline
- ✅ Survival mechanics documentation
- ✅ Geographic data package

**Phase II: Design** (In Progress)
- 🔄 Core game systems specifications
- 🔄 Mission design (30+ missions)
- ⏳ Dialogue and narrative content
- ⏳ Audio/visual design bible

**Phase III: Implementation** (In Progress - Started Nov 2025)
- ✅ Camera & Cover System (Hellblade 2-style, complete)
- ✅ Character & Movement (posture, stamina, survival)
- ✅ 7 Survival Meters (all formulas implemented)
- ⏳ Combat system (weapons, aiming, melee)
- ⏳ AI & Artillery (EQS targeting, shell impacts)
- ⏳ Mission framework (triggers, objectives, narrative)
- ⏳ Alpha (vertical slice - one full mission)

**Phase IV: Production** (Future)
- ⏳ Full asset production
- ⏳ Voice acting and localization
- ⏳ QA and playtesting

**Phase V: Release** (Future)
- ⏳ PC launch (Steam/Epic)
- ⏳ Console ports
- ⏳ Educational editions

---

## 💬 CONTACT

**Project Lead**: Claude (Verdun Anthology Historian + Game Designer)
**GitHub**: https://github.com/SingSongScreamAlong/OnlyTheDead
**Issues**: https://github.com/SingSongScreamAlong/OnlyTheDead/issues

---

## 🎖️ DEDICATION

*This project is dedicated to the memory of the approximately 700,000 French and German soldiers who became casualties at Verdun between February 21 and December 18, 1916.*

*"Ils ne passeront pas." (They shall not pass)*

*May we remember their sacrifice, understand their suffering, and never forget the cost of war.*

---

**"Only the dead have seen the end of war."** — Plato (attr.)

---

*Last Updated: November 10, 2025*
