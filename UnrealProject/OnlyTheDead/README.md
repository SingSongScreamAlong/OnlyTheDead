# Only The Dead - Unreal Engine 5 Project

**WWI Survival Game - Battle of Verdun 1916**

## Project Overview

This is the Unreal Engine 5 implementation of "Only The Dead", a historically accurate survival game set during the 303-day Battle of Verdun (February 21 - December 18, 1916).

## Project Structure

```
OnlyTheDead/
├── Config/                      # Engine and game configuration files
│   ├── DefaultEngine.ini        # Engine settings (rendering, physics, etc.)
│   ├── DefaultGame.ini          # Game-specific settings
│   └── DefaultInput.ini         # Input configuration (Enhanced Input)
│
├── Content/                     # Blueprint and asset content (created in editor)
│   ├── Maps/                    # Level files
│   ├── Blueprints/              # Blueprint classes
│   ├── UI/                      # UI widgets and materials
│   ├── Characters/              # Character blueprints and animations
│   ├── Systems/                 # Game system blueprints
│   └── Data/                    # Data tables and assets
│
├── Source/OnlyTheDead/          # C++ source code
│   ├── Public/                  # Header files
│   │   ├── Core/                # Core game classes
│   │   ├── Systems/             # Game systems (survival, combat, etc.)
│   │   ├── Characters/          # Character classes
│   │   ├── UI/                  # UI components
│   │   ├── World/               # World actors and components
│   │   └── Weapons/             # Weapon classes
│   │
│   ├── Private/                 # Implementation files
│   │   ├── Core/
│   │   ├── Systems/
│   │   ├── Characters/
│   │   ├── UI/
│   │   ├── World/
│   │   └── Weapons/
│   │
│   ├── OnlyTheDead.Build.cs     # Build configuration
│   ├── OnlyTheDead.h            # Main module header
│   └── OnlyTheDead.cpp          # Main module implementation
│
├── Plugins/                     # Third-party plugins
├── Saved/                       # Temporary files and logs
└── OnlyTheDead.uproject         # Unreal project file
```

## Implemented Systems (C++)

### ✅ Core Systems

#### 1. Survival System (`SurvivalComponent`)
- **7 Survival Meters** with exact specifications from design document:
  - **Hunger**: Depletes over time, death after 7 days at 0
  - **Thirst**: Depletes faster, death after 3 days at 0
  - **Stamina**: Regenerates when resting, consumed by actions
  - **Warmth**: Affected by weather, hypothermia mechanics
  - **Hygiene**: Disease risk, trench foot, infections
  - **Morale**: Psychological state, shell shock risk
  - **Alertness**: Sleep deprivation, hallucinations

**Features**:
- Real-time meter depletion with exact formulas from design doc
- Critical thresholds and warning states
- Death countdown timers for lethal meters
- Blueprint-accessible functions and events
- Cascading failure mechanics

**Location**: `Source/OnlyTheDead/Public/Systems/SurvivalComponent.h`

#### 2. Character System (`VerdunSoldierCharacter`)
- French soldier character class
- Integrated survival component
- Movement speed affected by survival meters
- Prone/sprint/cover mechanics
- Character stats tracking (days survived, rotations, etc.)

**Location**: `Source/OnlyTheDead/Public/Characters/VerdunSoldierCharacter.h`

#### 3. Game Mode (`OnlyTheDeadGameMode`)
- Historical timeline system (303 days)
- 6 battle phases (Feb - Dec 1916)
- Rotation system (Front Line → Support → Rest, 7-day cycles)
- Permadeath mechanics
- Mission management
- Time-of-day and date progression
- Autosave system
- Statistics tracking

**Location**: `Source/OnlyTheDead/Public/Core/OnlyTheDeadGameMode.h`

## Design Specifications

All game systems are based on detailed specifications in:
- **Development Book**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md`
- **Research Anthology**: `/verdun_anthology/`

### Survival Meter Specifications

All depletion rates, restoration values, and effects are implemented exactly as specified in the Development Book (Part II, Section 5).

**Examples**:
- Hunger: 3.0/hour resting → death in 168 hours (7 days)
- Thirst: 5.0/hour resting → death in 72 hours (3 days)
- Stamina: 2.0/second regeneration when resting
- Warmth: 0.5/minute winter depletion → death in 4 hours

## Building the Project

### Requirements
- Unreal Engine 5.5 (or later)
- Visual Studio 2022 (Windows) or Xcode (Mac)
- Minimum 32GB RAM recommended
- GPU: RTX 3060 or equivalent (for Lumen/Nanite)

### Setup Instructions

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd OnlyTheDead/UnrealProject/OnlyTheDead
   ```

2. **Generate project files**:
   - Right-click `OnlyTheDead.uproject` → "Generate Visual Studio project files"

3. **Open in Visual Studio**:
   - Open `OnlyTheDead.sln`
   - Set build configuration to "Development Editor"
   - Build solution (Ctrl+Shift+B)

4. **Launch Editor**:
   - Double-click `OnlyTheDead.uproject`
   - Or launch from Visual Studio (F5)

## Development Status

### ✅ Completed
- [x] UE5 project structure and configuration
- [x] Survival system (7 meters with exact formulas)
- [x] Player character class with survival integration
- [x] Game Mode with timeline and rotation systems
- [x] Permadeath mechanics
- [x] Core movement and state systems

### 🔄 In Progress
- [ ] Artillery system implementation
- [ ] Combat mechanics
- [ ] Inventory & equipment system
- [ ] Medical & injury system
- [ ] AI behavior system
- [ ] Mission framework
- [ ] UI/UX implementation

### ⏳ Planned
- [ ] Level design and terrain import
- [ ] Audio system
- [ ] Visual effects (artillery, weather, etc.)
- [ ] Multiplayer/co-op mode (stretch goal)

## Key Features

### Historical Accuracy
- **303-day timeline**: Feb 21 - Dec 18, 1916
- **Real locations**: 80+ verified coordinates from SRTM elevation data
- **Authentic mechanics**: Based on historical soldier experiences
- **6 battle phases**: Matches actual Verdun timeline

### Survival Mechanics
- **7 interconnected meters**: Complex survival simulation
- **Cascading failures**: Low hunger → low stamina → can't work → death spiral
- **Realistic timescales**: Thirst kills in 3 days, hunger in 7 days
- **Environmental factors**: Weather, season, clothing affect warmth

### Rotation System
- **21-day cycle**: 7 days front line, 7 support, 7 rest
- **Dynamic difficulty**: Front line is most dangerous
- **Authentic experience**: Based on French trench rotation schedules

### Permadeath
- **One life**: When you die, playthrough ends
- **Statistics tracking**: See how long you survived
- **Cause of death**: Detailed death attribution
- **Restart option**: Begin new playthrough from Feb 21

## Geographic Data Integration

The project includes Python tools for importing real Verdun terrain data:

**Location**: `/verdun_anthology/geographic_data/`

- `fetch_elevation_data.py`: Downloads SRTM elevation data
- `export_for_ue5.py`: Converts to UE5 heightmap format
- `export_for_houdini.py`: Alternative Houdini workflow

**Usage**:
```bash
cd /verdun_anthology/geographic_data
python fetch_elevation_data.py
python export_for_ue5.py
# Import generated heightmap into UE5 Landscape tool
```

## Documentation

### Design Documents
- **Game Design Bible**: `/ONLY_THE_DEAD_DEVELOPMENT_BOOK.md`
- **Historical Research**: `/verdun_anthology/` (70+ documents)
- **Timeline**: `/verdun_anthology/timeline/` (12 months detailed)
- **Survival Guides**: `/verdun_anthology/survival_mechanics/` (7 guides)
- **Mission Examples**: See Development Book Part III

### Code Documentation
- All C++ classes have detailed header documentation
- Public functions are BlueprintCallable where appropriate
- Enums and structs are BlueprintType for designer access

## Contributing

This is currently a solo development project. Contributions are welcome in:
- Historical research (primary sources, fact-checking)
- 3D asset creation (WWI equipment, trenches, fortifications)
- Audio (period-appropriate sound effects, music)
- Translation (French, German localization)

## Credits

**Design & Development**: See main project README
**Historical Research**: Based on academic sources and archives (see `/verdun_anthology/`)
**Engine**: Unreal Engine 5.5 by Epic Games

## License

[TBD - Awaiting license decision]

## Contact

For questions about implementation or design:
- See main project repository
- Reference the Development Book for design decisions
- Check `/verdun_anthology/` for historical sources

---

**"Only the dead have seen the end of war."** - Plato (disputed)

*Victory is survival.*
