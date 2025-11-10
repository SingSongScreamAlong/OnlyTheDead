# Only The Dead - Technical Setup Guide

**Version**: 1.0
**Last Updated**: November 10, 2025
**For**: Developers setting up the project for the first time

---

## Prerequisites

### Required Software

#### Unreal Engine 5.3+
- Download from: https://www.unrealengine.com/download
- Install location: Set `UE5_ROOT` environment variable to installation directory
- License: Free for development (royalties on revenue > $1M)

#### Visual Studio 2022 (Windows)
- Community Edition or higher
- Workloads:
  - Game development with C++
  - .NET desktop development
- Install from: https://visualstudio.microsoft.com/

#### CMake 3.20+
- Download from: https://cmake.org/download/
- Add to PATH during installation

#### Git & Git LFS
- Git: https://git-scm.com/
- Git LFS: https://git-lfs.github.com/
- Required for large asset files

#### Python 3.10+
- Download from: https://python.org/
- Required for terrain tools and build scripts

---

## Initial Setup

### 1. Clone Repository

```bash
git clone https://github.com/SingSongScreamAlong/OnlyTheDead.git
cd OnlyTheDead
git lfs pull  # Pull large files
```

### 2. Set Environment Variables

**Windows**:
```cmd
setx UE5_ROOT "C:\Program Files\Epic Games\UE_5.3"
```

**Linux/Mac**:
```bash
export UE5_ROOT="/path/to/UnrealEngine"
echo 'export UE5_ROOT="/path/to/UnrealEngine"' >> ~/.bashrc
```

### 3. Generate Project Files

**Windows**:
```cmd
"%UE5_ROOT%\Engine\Build\BatchFiles\GenerateProjectFiles.bat" OnlyTheDead.uproject
```

**Linux/Mac**:
```bash
"$UE5_ROOT/Engine/Build/BatchFiles/GenerateProjectFiles.sh" OnlyTheDead.uproject
```

### 4. Open Project

**Method A: Using Unreal Editor**
1. Open Epic Games Launcher
2. Navigate to Library > UE 5.3
3. Click "Launch"
4. Open Project > Browse to `OnlyTheDead.uproject`

**Method B: Direct Launch**
```cmd
"%UE5_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" OnlyTheDead.uproject
```

### 5. Initial Build

In Unreal Editor:
1. Tools > Refresh Visual Studio Project
2. Close Editor
3. Open `OnlyTheDead.sln` in Visual Studio
4. Build > Build Solution (Ctrl+Shift+B)
5. Start with Debugging (F5)

---

## Project Structure

```
OnlyTheDead/
├── OnlyTheDead.uproject          # Unreal project file
├── Config/                       # Engine configuration
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   └── DefaultInput.ini
├── Content/                      # Game assets (UE5 assets)
│   ├── Blueprints/
│   ├── Data/                     # Data tables
│   ├── Maps/
│   ├── Materials/
│   ├── Audio/
│   └── UI/
├── Source/                       # C++ source (UE5 build system)
│   ├── OnlyTheDead/
│   └── OnlyTheDeadEditor/
├── src/                          # Our C++ headers/implementations
│   ├── core/                     # Core game types
│   ├── systems/                  # Game systems
│   ├── ai/                       # AI behaviors
│   ├── audio/                    # Audio system
│   └── data/                     # Data files
├── verdun_anthology/             # Historical research
├── docs/                         # Documentation
├── Binaries/                     # Compiled binaries (generated)
├── Intermediate/                 # Build artifacts (generated)
└── Saved/                        # Saved files, logs (generated)
```

---

## Development Workflow

### Daily Development

1. **Pull Latest Changes**
   ```bash
   git pull origin main
   git lfs pull
   ```

2. **Open Project**
   - Launch Unreal Editor
   - Wait for shaders to compile (first time: 30-60 minutes)

3. **Code Changes**
   - Edit C++ in Visual Studio
   - Edit Blueprints in UE5 Editor
   - Compile in VS or use Hot Reload in Editor

4. **Test Changes**
   - Play In Editor (Alt+P)
   - Standalone Game (Alt+S)
   - Check logs: `Saved/Logs/`

5. **Commit Changes**
   ```bash
   git add .
   git commit -m "Description of changes"
   git push origin your-branch-name
   ```

### Building for Distribution

#### Development Build
```bash
"%UE5_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
    -project="%CD%\OnlyTheDead.uproject" ^
    -platform=Win64 ^
    -clientconfig=Development ^
    -build -cook -stage -pak
```

#### Shipping Build (Final Release)
```bash
"%UE5_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
    -project="%CD%\OnlyTheDead.uproject" ^
    -platform=Win64 ^
    -clientconfig=Shipping ^
    -build -cook -stage -pak -archive ^
    -archivedirectory="Builds/Windows"
```

---

## System-Specific Setup

### Survival System

**Data Tables**:
- `Content/Data/DT_FoodItems.json` - Food/water items
- `Content/Data/DT_SurvivalMeters.json` - Meter configurations

**C++ Files**:
- `src/systems/SurvivalSystem.h` - Header
- `src/systems/SurvivalSystem.cpp` - Implementation

**Blueprints**:
- `Content/Blueprints/Systems/BP_SurvivalComponent.uasset`

**Testing**:
```cpp
// In-game console commands
DebugSurvival.SetMeter Hunger 50
DebugSurvival.SetMeter Thirst 0
DebugSurvival.PrintAllMeters
```

### Artillery System

**Data Tables**:
- `Content/Data/DT_ShellTypes.json` - Shell specifications

**Audio**:
- Place MetaSounds in `Content/Audio/Artillery/`
- Naming convention: `MS_Shell_[Type]_[State].uasset`
  - Example: `MS_Shell_155mm_Incoming.uasset`

**Testing**:
```cpp
// Console commands
DebugArtillery.FireShell 155mm
DebugArtillery.StartBombardment Heavy 300
DebugArtillery.TestShellRecognition
```

### Mission System

**Data Tables**:
- `Content/Data/DT_Missions.json` - All mission data

**Maps**:
- Each mission has dedicated map
- Naming: `MAP_M[##]_[MissionName].umap`
  - Example: `MAP_M01_TheGunsBegin.umap`

**Testing**:
```cpp
// Console commands
DebugMission.Load M01_TheGunsBegin
DebugMission.CompleteObjective M01_OBJ01
DebugMission.SkipToDate 1916-06-01
```

---

## Debugging

### Common Issues

#### Issue: Project won't open
**Solution**:
1. Check UE5_ROOT is set correctly
2. Verify UE5.3+ installed
3. Delete `Intermediate/` and `Binaries/` folders
4. Regenerate project files

#### Issue: Compile errors
**Solution**:
1. Clean solution in Visual Studio
2. Rebuild all
3. Check for missing headers
4. Verify all data tables are valid JSON

#### Issue: Shaders compiling forever
**Solution**:
- This is normal for first build (30-60 min)
- Subsequent builds are incremental
- Close other applications to free RAM

#### Issue: Hot reload fails
**Solution**:
- Close editor
- Build in Visual Studio
- Relaunch editor

### Debug Logging

**C++ Logging**:
```cpp
UE_LOG(LogTemp, Log, TEXT("Normal log message"));
UE_LOG(LogTemp, Warning, TEXT("Warning message"));
UE_LOG(LogTemp, Error, TEXT("Error message: %s"), *ErrorString);
```

**View Logs**:
- In-editor: Window > Developer Tools > Output Log
- File: `Saved/Logs/OnlyTheDead.log`

### Performance Profiling

**In-Editor**:
- Window > Developer Tools > Session Frontend
- Start profiling session
- Analyze CPU, GPU, memory usage

**Stat Commands**:
```
stat fps           # Show FPS
stat unit          # Show frame time breakdown
stat memory        # Show memory usage
stat audio         # Show audio stats
stat game          # Show game thread stats
```

---

## Testing

### Automated Tests

**Run All Tests**:
```bash
"%UE5_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
    "%CD%\OnlyTheDead.uproject" ^
    -ExecCmds="Automation RunTests Now Project" ^
    -TestExit="Automation Test Queue Empty" ^
    -log
```

**Test Categories**:
- Unit Tests: Individual system tests
- Integration Tests: System interaction tests
- Gameplay Tests: Mission flow tests

### Manual Testing Checklist

**Survival Systems**:
- [ ] All 7 meters deplete correctly
- [ ] Food items restore hunger
- [ ] Water items restore thirst
- [ ] Death occurs at correct thresholds
- [ ] Cascading effects work

**Artillery Systems**:
- [ ] Shell audio is 3D spatialized
- [ ] Different shell types distinguishable
- [ ] Blast radius damage correct
- [ ] Craters appear at impact
- [ ] Morale impacts work

**Mission Systems**:
- [ ] Missions load correctly
- [ ] Objectives track properly
- [ ] Victory/failure conditions trigger
- [ ] Save/load preserves state

---

## Performance Targets

### PC (Development)
- **Target**: 60 FPS @ 1440p
- **Min Spec**: GTX 1060, 16GB RAM, SSD
- **Recommended**: RTX 3070, 32GB RAM, NVMe SSD

### Consoles
- **PS5**: 1440p @ 60 FPS (Performance) or 4K @ 30 FPS (Quality)
- **Xbox Series X**: Same as PS5
- **Steam Deck**: 800p @ 30 FPS

### Optimization Tips
- Use World Partition for large maps
- Enable Nanite for static meshes
- Use Lumen for dynamic lighting
- Profile regularly with Unreal Insights

---

## Contributing

See: `CONTRIBUTING.md` (to be created)

**Coding Standards**:
- C++17 standard
- UE5 coding conventions
- Comprehensive comments
- Unit tests for new systems

**Branch Strategy**:
- `main`: Stable releases
- `develop`: Active development
- `feature/*`: New features
- `bugfix/*`: Bug fixes
- `claude/*`: AI-assisted development

---

## Support & Resources

**Documentation**:
- Game Design: `ONLY_THE_DEAD_DEVELOPMENT_BOOK.md`
- Historical Research: `verdun_anthology/`
- API Docs: `docs/api/` (generate with Doxygen)

**External Resources**:
- UE5 Docs: https://docs.unrealengine.com/5.3/
- UE5 Forums: https://forums.unrealengine.com/
- UE5 AnswerHub: https://answers.unrealengine.com/

**Contact**:
- GitHub Issues: https://github.com/SingSongScreamAlong/OnlyTheDead/issues
- Discord: (to be created)

---

**Last Updated**: November 10, 2025
**Maintained By**: OnlyTheDead Development Team
