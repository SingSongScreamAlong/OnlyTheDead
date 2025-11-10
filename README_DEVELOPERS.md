# Only The Dead - Developer Quick Start

**For developers joining the project mid-development**

---

## What Is This Project?

"Only The Dead" is a WWI survival game set at the Battle of Verdun (Feb 21 - Dec 18, 1916). 
This is NOT a typical war FPS. This is survival horror in a historical setting.

**Victory condition**: Survive 303 days. Not "kill 100 enemies."

---

## Current Status: 95% Complete (Design Phase)

### What Exists ✅
- Complete game design (55,000 words)
- All 35 missions designed
- All core systems specified (C++ headers)
- Historical research foundation (135,000 words)
- UE5 project structure
- Build system and CI/CD
- Complete documentation

### What's Missing ⏳
- UE5 implementation (C++ .cpp files need full implementations)
- All game assets (3D models, audio, textures)
- Blueprint logic
- Actual playable game

**You're joining at the perfect time** - design is complete, implementation begins now.

---

## First Day Checklist

### 1. Clone and Setup (30 minutes)
```bash
git clone https://github.com/SingSongScreamAlong/OnlyTheDead.git
cd OnlyTheDead
git lfs pull  # Important!

# Set environment variable
export UE5_ROOT="/path/to/UnrealEngine"  # Or setx on Windows

# Generate project files
"$UE5_ROOT/Engine/Build/BatchFiles/GenerateProjectFiles.sh" OnlyTheDead.uproject
```

### 2. Read Core Docs (2 hours)
1. **This file** (you're reading it)
2. `PROJECT_STATUS.md` - Complete project overview
3. `docs/TECHNICAL_SETUP.md` - Detailed setup instructions
4. `ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` - Game design bible (skim, read sections you'll work on)

### 3. Open Project (1 hour first time)
```bash
# Launch UE5 Editor
"$UE5_ROOT/Engine/Binaries/[Platform]/UnrealEditor" OnlyTheDead.uproject

# Wait for shaders to compile (30-60 min first time)
# Grab coffee, read more docs
```

### 4. Choose Your Path

Pick ONE area to start contributing:

#### Path A: Programmer
**Goal**: Implement survival system
**Start**: `src/systems/SurvivalSystem.cpp` (already has implementation!)
**Task**: Review implementation, add missing features, test in-engine
**Duration**: 1-2 weeks to full functionality

#### Path B: Level Designer
**Goal**: Build Mission 01 environment
**Start**: Create `Content/Maps/MAP_M01_TheGunsBegin.umap`
**Task**: Blockout Bois des Caures trenches
**Reference**: `verdun_anthology/geographic_data/` + historical photos
**Duration**: 2-3 weeks to blockout

#### Path C: Game Designer
**Goal**: Implement Mission 01 gameplay
**Start**: Create Blueprint `BP_Mission_M01`
**Task**: Script mission objectives, triggers, events
**Reference**: `ONLY_THE_DEAD_DEVELOPMENT_BOOK.md` Lines 1838-1927
**Duration**: 1-2 weeks to basic playable

#### Path D: Audio Designer
**Goal**: Create artillery audio system
**Start**: MetaSounds for shell incoming audio
**Task**: Implement 75mm shell audio (test case)
**Reference**: `docs/ASSET_SPECIFICATIONS.md` Artillery Audio section
**Duration**: 1-2 weeks to prototype

---

## File Structure Tour (5 minutes)

**Most Important Files**:
```
OnlyTheDead/
├── OnlyTheDead.uproject           # Open this in UE5
├── PROJECT_STATUS.md              # READ THIS FIRST
├── ONLY_THE_DEAD_DEVELOPMENT_BOOK.md  # Game design bible
│
├── src/                           # C++ code
│   ├── core/GameTypes.h           # Core types, enums, structs
│   └── systems/                   # All game systems
│       ├── SurvivalSystem.h/.cpp  # 7 survival meters
│       ├── ArtillerySystem.h      # Shell recognition
│       └── MissionSystem.h        # 303-day campaign
│
├── Content/
│   ├── Data/                      # Data tables (JSON)
│   ├── Maps/                      # Levels (create these)
│   └── Blueprints/                # Blueprint logic (create these)
│
├── verdun_anthology/              # Historical research
│   ├── timeline/                  # 303 days detailed
│   └── geographic_data/           # Real terrain data
│
└── docs/                          # Documentation
    ├── TECHNICAL_SETUP.md         # Setup guide
    └── ASSET_SPECIFICATIONS.md    # Asset requirements
```

---

## Key Systems Overview (10 minutes)

### Survival System
**What**: 7 meters (Hunger, Thirst, Stamina, Warmth, Hygiene, Morale, Alertness)
**Code**: `src/systems/SurvivalSystem.h/.cpp`
**Status**: Header + implementation complete, needs testing
**Critical**: Thirst is most important (death in 24 hours at 0)

### Artillery System
**What**: Shell recognition by audio (core gameplay mechanic)
**Code**: `src/systems/ArtillerySystem.h`
**Status**: Header only, needs implementation
**Critical**: 3D spatial audio is essential

### Mission System
**What**: 303-day campaign with rotation system
**Code**: `src/systems/MissionSystem.h`
**Status**: Header only, needs implementation
**Critical**: Drives entire game structure

---

## Your First Task (Choose One, ~1 week)

### For Programmers
**Task**: "Make Thirst Meter Work in UE5"
1. Finish `SurvivalSystem.cpp` implementation if needed
2. Create Blueprint Component: `BP_SurvivalComponent`
3. Add UI widget showing thirst meter
4. Test: Thirst depletes over time, player can drink water, death at 0 for 24 hours
5. **Success Criteria**: Can watch player die of thirst in PIE (Play In Editor)

### For Level Designers
**Task**: "Build Opening Trench"
1. Create map: `MAP_M01_TheGunsBegin.umap`
2. Import terrain data from `verdun_anthology/geographic_data/`
3. Blockout first trench section (100m)
4. Add: sandbags, duckboards, dugout entrance
5. **Success Criteria**: Can walk through trench, looks like Verdun

### For Game Designers
**Task**: "Script First 5 Minutes of Mission 01"
1. Create Blueprint: `BP_Mission_M01_TheGunsBegin`
2. Script: Game starts, Dubois speaks (text for now), bombardment begins
3. Objective markers appear
4. Player tutorial prompts
5. **Success Criteria**: Can play first 5 minutes without manual input

---

## Common Gotchas

### Problem: "Project won't open"
**Solution**: 
- Check UE5_ROOT environment variable
- Delete `Intermediate/` and `Binaries/` folders
- Regenerate project files

### Problem: "Can't find header files"
**Solution**:
- Our headers are in `src/`, UE5 expects `Source/`
- Add `src/` to include paths in IDE
- Or create symlinks

### Problem: "Data tables not loading"
**Solution**:
- JSON files need to be converted to UE5 DataTable assets
- Right-click in Content Browser > Miscellaneous > Data Table
- Import JSON

### Problem: "Shaders compiling forever"
**Solution**:
- Normal for first time (30-60 min)
- Subsequent builds are fast (incremental)
- Don't close editor during shader compilation

---

## Testing Your Work

### In-Editor Testing
```
Play In Editor (Alt+P)  - Quick test
Standalone Game (Alt+S) - More accurate performance
```

### Console Commands (Use ~ key)
```cpp
// Survival System
DebugSurvival.SetMeter Thirst 0        // Test death
DebugSurvival.SetMeter Hunger 25       // Test critical state
DebugSurvival.PrintAllMeters           // Show all values

// Artillery System
DebugArtillery.FireShell 155mm         // Spawn shell
DebugArtillery.StartBombardment Heavy  // Test bombardment

// Mission System
DebugMission.Load M01_TheGunsBegin     // Load mission
DebugMission.CompleteObjective OBJ01   // Skip objective
```

---

## Communication

### Daily Standup (Async)
Post in Discord #dev-standup:
- What you did yesterday
- What you're doing today
- Any blockers

### Ask Questions
- **Technical**: Discord #dev-help or GitHub Issues
- **Design**: Discord #design-discussion
- **Historical**: Discord #history (verify accuracy)

### Code Review
- All PRs need 1 approval
- Respond to review comments within 48 hours
- Be respectful (we're all learning)

---

## Coding Style Quick Reference

```cpp
// Classes: PascalCase with UE prefix
class ONLYTHEDEAD_API USurvivalSystem : public UActorComponent

// Functions: PascalCase
void SetMeterValue(ESurvivalMeterType MeterType, float NewValue);

// Variables: camelCase
float currentHunger = 100.0f;

// Constants: UPPER_SNAKE_CASE
static constexpr float MAX_HUNGER = 100.0f;

// Comments: Explain "why", not "what"
// Decrease morale when comrade dies (historical: unit cohesion critical)
ModifyMeter(ESurvivalMeterType::Morale, -20.0f);
```

---

## Performance Targets

- **PC**: 60 FPS @ 1440p (mid-range hardware)
- **Console**: 60 FPS @ 1440p (Performance) or 30 FPS @ 4K (Quality)
- **Always profile before optimizing**

---

## Historical Accuracy Rule

**If you add anything historical:**
1. Cite your source in code comments
2. Verify dates/facts against `verdun_anthology/`
3. If unsure, ask in #history channel
4. Mark fictional content clearly

```cpp
// HISTORICAL FACT - Fort Douaumont captured Feb 25, 1916
// Source: French official history (Service Historique de la Défense)
SetMissionDate(1916, 2, 25);

// COMPOSITE - Typical daily ration (verified pattern)
// Source: French Army supply regulations 1916
AddFoodRation("Bread", 700g);

// GAME DESIGN - Balanced for gameplay
// Hunger depletes at 5/hour (historical: soldiers could go days without food)
SetDepletionRate(5.0f);
```

---

## When You Get Stuck

1. **Check docs**: Most answers are in `docs/` or Development Book
2. **Search code**: Use IDE search for similar code
3. **Ask**: Discord #dev-help (no stupid questions!)
4. **GitHub Issue**: If it's a bug or missing feature
5. **Pair program**: Schedule with another dev

---

## Milestones

**Near-term (3-6 months)**: Vertical slice (Mission 01 playable)
**Mid-term (12 months)**: Alpha (10 missions playable)
**Long-term (24-33 months)**: Release (all 35 missions)

**Your contribution matters** - every line of code, every asset, every design decision brings us closer to honoring the 700,000 casualties of Verdun through this game.

---

## Resources

- **Game Design**: `ONLY_THE_DEAD_DEVELOPMENT_BOOK.md`
- **Setup**: `docs/TECHNICAL_SETUP.md`
- **Assets**: `docs/ASSET_SPECIFICATIONS.md`
- **Contributing**: `CONTRIBUTING.md`
- **UE5 Docs**: https://docs.unrealengine.com/5.3/

---

**Welcome to the team. Let's build something meaningful.**

*"Ils ne passeront pas."*
