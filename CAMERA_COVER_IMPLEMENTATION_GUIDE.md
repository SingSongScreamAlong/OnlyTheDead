# Only The Dead - Camera & Cover System Implementation Guide

## Overview

This document provides a comprehensive guide to implementing and integrating the Hellblade 2-inspired camera system with the WWI trench warfare survival mechanics for Only The Dead.

**System Highlights:**
- **Hybrid Camera Stack**: 4 contextual modes (Guided Walk, Survival OTS, Precision ADS, Set Piece)
- **Shoulder-Swap OTS**: Smooth left/right shoulder transitions with collision handling
- **Artillery-Focused Cover**: Non-sticky cover detection for shell shelter (not PvP combat)
- **Exposure Heatmap**: Visible players attract artillery and sniper fire
- **7 Survival Meters**: Hunger, Thirst, Stamina, Warmth, Hygiene, Morale, Alertness
- **Posture System**: Standing, Crouching, Prone with stamina integration

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Setup & Integration Steps](#setup--integration-steps)
3. [Component Reference](#component-reference)
4. [Enhanced Input Configuration](#enhanced-input-configuration)
5. [Blueprint Integration](#blueprint-integration)
6. [AI & Gameplay Integration](#ai--gameplay-integration)
7. [Tuning & Balancing](#tuning--balancing)
8. [Testing Checklist](#testing-checklist)

---

## Architecture Overview

### Core Components

```
AOTDCharacter (Player Character)
├── USpringArmComponent (CameraBoom)
│   └── UCameraComponent (FollowCamera)
├── UOTDSurvivalComponent (7 meters)
├── UOTDCoverDetectionComponent (Artillery shelter detection)
└── UOTDExposureHeatmapComponent (AI targeting)

AOTDCameraManager (Custom Camera Manager)
├── Camera Mode Management (4 modes)
├── Shoulder Swap Logic
├── Suppression Effects
└── Artillery Shake

Data Assets
├── UOTDCameraDataAsset (Per-mode camera configs)
└── Enhanced Input Assets (IMC_Character, IA_* actions)
```

### Data Flow

```
Player Input → Enhanced Input System
    ↓
Character Movement & Actions
    ↓
Survival Component Updates (meters drain/regen)
    ↓
Cover Detection (continuous traces)
    ↓
Exposure Heatmap (visibility tracking)
    ↓
Camera Manager (mode selection & blending)
    ↓
Final Camera Transform
```

---

## Setup & Integration Steps

### Step 1: Project Configuration

1. **Create UE5 Project** (if not already created)
   - Use Third Person template as starting point
   - Enable Enhanced Input plugin
   - Set target platform(s): PC, Console

2. **Add Source Files**
   - Copy all files from `/Source/OnlyTheDead/` to your project
   - Regenerate Visual Studio project files
   - Compile C++ code

3. **Directory Structure**
```
Source/OnlyTheDead/
├── Public/
│   ├── Camera/
│   │   ├── OTDCameraManager.h
│   │   └── OTDCameraDataAsset.h
│   ├── Character/
│   │   └── OTDCharacter.h
│   ├── Survival/
│   │   └── OTDSurvivalComponent.h
│   ├── Cover/
│   │   └── OTDCoverDetectionComponent.h
│   └── Combat/
│       └── OTDExposureHeatmapComponent.h
└── Private/ (matching .cpp files)

Content/
├── Blueprints/
│   ├── BP_OTDCharacter (Blueprint child of AOTDCharacter)
│   └── BP_OTDGameMode
├── DataAssets/
│   └── Camera/
│       ├── DA_CameraMode_GuidedWalk
│       ├── DA_CameraMode_SurvivalOTS
│       ├── DA_CameraMode_PrecisionADS
│       └── DA_CameraMode_SetPiece
└── Input/
    ├── IMC_Character (Input Mapping Context)
    └── IA_* (Input Actions)
```

### Step 2: Create Camera Data Assets

1. **In Unreal Editor:**
   - Content Browser → Right Click → Miscellaneous → Data Asset
   - Choose `OTDCameraDataAsset` as parent class
   - Name: `DA_CameraMode_SurvivalOTS`

2. **Configure Camera State:**
```
Camera State:
  Mode: Survival OTS
  FOV: 75.0
  Camera Offset: (-150, 60, 60)
  Camera Lag: 1.5
  Camera Rotation Lag: 5.0
  Blend Time: 0.25

Post Process Settings:
  (Optional - leave default for now)
```

3. **Repeat for all 4 modes:**
   - Guided Walk (FOV 80, slower lag)
   - Survival OTS (FOV 75, balanced)
   - Precision ADS (FOV 60, minimal lag)
   - Lock-Framed Set Piece (FOV 85, no lag)

### Step 3: Setup Enhanced Input

See [Enhanced Input Configuration](#enhanced-input-configuration) section below.

### Step 4: Create Character Blueprint

1. **Create Blueprint:**
   - Content Browser → Blueprints → Blueprint Class
   - Parent: `OTDCharacter`
   - Name: `BP_OTDCharacter`

2. **Configure Components:**
   - Camera Boom:
     - Target Arm Length: 150.0
     - Socket Offset: (0, 60, 60)
     - Enable Camera Lag: true
     - Enable Camera Rotation Lag: true
     - Do Collision Test: true (important for trenches!)

   - Follow Camera:
     - Field of View: 75.0

   - Survival Component:
     - All default values from design doc

   - Cover Detection:
     - Detection Range: 100.0 cm
     - Detection Interval: 0.1 sec
     - Trace Fan Count: 5
     - Debug Draw: true (for testing)

   - Exposure Heatmap:
     - Exposure Accumulation Rate: 10.0/sec
     - Exposure Decay Rate: 5.0/sec
     - Artillery Attraction Threshold: 3.0 sec

3. **Set Mesh & Animation:**
   - Skeletal Mesh: (Your soldier mesh)
   - Anim Class: (Your animation blueprint)

4. **Configure Collision:**
   - Capsule Component:
     - Radius: 40.0
     - Half Height: 88.0 (standing)

### Step 5: Setup Game Mode

1. **Create BP_OTDGameMode:**
   - Default Pawn Class: `BP_OTDCharacter`
   - Player Controller Class: (Default or custom)
   - HUD Class: (Your HUD)

2. **Set Camera Manager Override:**
   - In Player Controller:
     - Player Camera Manager Class: `OTDCameraManager`

3. **World Settings:**
   - Override Game Mode: `BP_OTDGameMode`

### Step 6: Tag Level Geometry

For cover detection to work properly, tag your level actors:

```cpp
// In level editor, select actor and add tags:
"Trench"          - Shallow trench walls, sandbags
"ShellCrater"     - Bomb craters
"DugoutEntrance"  - Covered entrances
"DeepDugout"      - Underground shelters
"ConcreteBunker"  - Reinforced bunkers
"DugoutRoof"      - Overhead protection
"Overhead"        - Any overhead shelter
```

**Tagging Workflow:**
1. Select actor in viewport
2. Details Panel → Tags → Add Component Tag
3. Add appropriate tag from list above

---

## Component Reference

### AOTDCharacter

Main player character with integrated systems.

**Key Functions:**
```cpp
// Posture
void SetPosture(EOTDPosture NewPosture);
void ForcePosture(EOTDPosture NewPosture, float TransitionTime);
bool CanTransitionToPosture(EOTDPosture TargetPosture) const;

// Camera
void SwapShoulder();
void SetAiming(bool bIsAiming);

// Movement
void StartSprint();
void StopSprint();
bool CanSprint() const;
float GetMovementSpeedModifier() const;

// Leaning
void SetLeaning(float LeanAmount); // -1.0 to 1.0
```

**Blueprint Events to Implement:**
- `OnStaminaDepleted` - Notify player
- `OnPostureChanged` - Update UI
- `OnShoulderSwapped` - Optional feedback

### UOTDSurvivalComponent

Manages all 7 survival meters with exact formulas from design doc.

**Key Functions:**
```cpp
// Getters
float GetHunger() const;
float GetThirst() const;
float GetStamina() const;
float GetWarmth() const;
float GetHygiene() const;
float GetMorale() const;
float GetAlertness() const;

// Modification
void RestoreHunger(float Amount);
void RestoreThirst(float Amount);
void DrainStamina(float Amount);
void ModifyMorale(float Amount); // Can be + or -

// Activity Costs
void ApplySprintCost(float DeltaTime);  // 15/sec
void ApplyMeleeCost(float DeltaTime);   // 10/sec
void ApplyClimbCost(float DeltaTime);   // 12/sec

// Status
bool HasDisease(EOTDDisease Disease) const;
float GetMovementSpeedModifier() const;
float GetAimAccuracyModifier() const;
```

**Events:**
- `OnHungerChanged(float NewValue, float MaxValue)`
- `OnThirstChanged(float NewValue, float MaxValue)`
- `OnStaminaChanged(float NewValue, float MaxValue)`
- `OnDeath(FString Cause)`

**Integration:**
- Hook these events to UI widgets
- Display warnings when meters < 20%
- Show death screen on OnDeath event

### UOTDCoverDetectionComponent

Procedural cover detection for artillery shelter.

**Key Functions:**
```cpp
// Query
FOTDCoverInfo GetCurrentCover() const;
bool IsNearCover() const;
float GetProtectionValue() const; // 0.0 to 1.0
bool CanPeekFromCover() const;
FVector GetCoverDirection() const;

// Manual Query
FOTDCoverInfo DetectCoverAtLocation(const FVector& Location, const FRotator& Facing);
bool HasOverheadProtection(const FVector& Location) const;
```

**Events:**
- `OnEnteredCover(FOTDCoverInfo)`
- `OnExitedCover(FOTDCoverInfo)`
- `OnCoverChanged(FOTDCoverInfo)`

**Cover Types & Protection:**
```
Open Ground:     0% protection
Shallow Trench:  60% protection
Shell Crater:    75% protection
Dugout Entrance: 85% protection
Deep Dugout:     95% protection
Concrete Bunker: 99% protection
```

**Integration:**
- Use protection value to modify artillery damage
- Show cover indicator UI when near cover
- Tutorial: teach players to seek cover during bombardment

### UOTDExposureHeatmapComponent

Tracks player visibility for AI targeting.

**Key Functions:**
```cpp
// Query
float GetExposureValue() const; // 0.0 to 100.0
EOTDExposureLevel GetExposureLevel() const;
bool IsCriticallyExposed() const;
float GetTimeAtCurrentExposure() const;

// AI Scores (for EQS)
float GetArtilleryAttractionScore() const; // 0.0 to 1.0
float GetSniperAttractionScore() const;

// Manual Control
void AddExposure(float Amount);
void ResetExposure();
```

**Events:**
- `OnExposureLevelChanged(EOTDExposureLevel Old, EOTDExposureLevel New)`

**Exposure Levels:**
```
None:     0-20   (Hidden, safe)
Low:      20-40  (Minimal exposure)
Medium:   40-60  (Moderate risk)
High:     60-80  (Artillery may respond)
Critical: 80-100 (Imminent danger)
```

**Integration:**
- Display exposure indicator in UI (subtle, corner)
- Audio cue when reaching High/Critical
- Use AI scores in EQS queries for artillery targeting

### AOTDCameraManager

Custom camera manager with 4 contextual modes.

**Key Functions:**
```cpp
// Mode Management
void SetCameraMode(EOTDCameraMode NewMode, float BlendTimeOverride);
EOTDCameraMode GetCameraMode() const;
bool IsBlending() const;

// Shoulder
void SwapShoulder();
void SetShoulderSide(EOTDShoulderSide NewSide);

// Effects
void ApplySuppressionEffect(float SuppressionLevel); // 0.0 to 1.0
void ApplyBreathSway(float StaminaPercent);
void ApplyArtilleryShake(const FVector& ImpactLocation, float ShellCaliberMM);
```

**Camera Modes:**
```cpp
GuidedWalk          - Cinematic exploration
SurvivalOTS         - Default gameplay
PrecisionADS        - Aiming down sights
LockFramedSetPiece  - Scripted moments
```

**Accessibility:**
- `bAllowSetPieceLocks` - Disable locked cameras
- `bFixedHorizon` - Reduce motion sickness
- `CameraLagMultiplier` - Adjust responsiveness

---

## Enhanced Input Configuration

### Input Actions (Create in Editor)

1. **IA_Move** (Axis2D)
2. **IA_Look** (Axis2D)
3. **IA_Sprint** (Digital)
4. **IA_Crouch** (Digital)
5. **IA_Prone** (Digital)
6. **IA_Aim** (Digital)
7. **IA_SwapShoulder** (Digital)
8. **IA_LeanLeft** (Digital)
9. **IA_LeanRight** (Digital)
10. **IA_Lean** (Axis1D, -1.0 to 1.0)

### Input Mapping Context

Create **IMC_Character** with these mappings:

**Keyboard & Mouse:**
```
W/A/S/D       → IA_Move (2D Composite)
Mouse XY      → IA_Look (2D Composite, multiply by sensitivity)
Left Shift    → IA_Sprint (hold)
C / Ctrl      → IA_Crouch (toggle / hold)
X / Z         → IA_Prone (toggle)
Right Mouse   → IA_Aim (hold)
Middle Mouse  → IA_SwapShoulder (press)
Q             → IA_LeanLeft (hold)
E             → IA_LeanRight (hold)
```

**Gamepad:**
```
Left Stick    → IA_Move
Right Stick   → IA_Look
L3 (Click)    → IA_Sprint
B / Circle    → IA_Crouch
D-Pad Down    → IA_Prone
Left Trigger  → IA_Aim
R3 (Click)    → IA_SwapShoulder
Left Bumper   → IA_LeanLeft
Right Bumper  → IA_LeanRight
```

### Character Input Binding (C++)

```cpp
void AOTDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind actions
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOTDCharacter::Move);
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOTDCharacter::Look);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AOTDCharacter::StartSprint);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AOTDCharacter::StopSprint);
        // ... etc
    }
}
```

See `Content/Input/InputActions_README.md` for full details.

---

## Blueprint Integration

### UI Widgets

Create these widgets for player feedback:

1. **WBP_SurvivalMeters**
   - 7 progress bars for each meter
   - Color-coded: Green (>60%), Yellow (30-60%), Red (<30%)
   - Bind to OnMeterChanged events

2. **WBP_CoverIndicator**
   - Show when near cover
   - Display protection percentage
   - Arrow pointing to cover direction

3. **WBP_ExposureWarning**
   - Subtle indicator in corner
   - Pulses when High/Critical
   - Audio cue: "Take cover!"

4. **WBP_PostureIndicator**
   - Icon showing current posture (standing/crouching/prone)
   - Stamina bar below

### Example Blueprint Event

**BP_OTDCharacter Event Graph:**

```
Event BeginPlay
    → Get Player Controller
    → Cast to Player Controller
    → Get Player Camera Manager
    → Cast to OTDCameraManager
    → Set Camera Manager Reference

Event OnHighExposure
    → Show WBP_ExposureWarning
    → Play Audio Cue "Take Cover"
    → Camera Shake (slight)

Event OnEnteredCover
    → Show WBP_CoverIndicator
    → Set Text (Protection %)
    → Fade in over 0.2s

Event OnExitedCover
    → Fade out WBP_CoverIndicator
```

---

## AI & Gameplay Integration

### Artillery System Integration

When artillery impacts near player:

```cpp
// In your artillery shell actor:
void AArtilleryShell::Explode()
{
    const FVector ImpactLocation = GetActorLocation();
    const float Caliber = ShellCaliberMM; // 75, 155, 210, etc.

    // Apply camera shake to player
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (AOTDCharacter* OTDChar = Cast<AOTDCharacter>(PlayerChar))
    {
        if (AOTDCameraManager* CamManager = OTDChar->GetCameraManager())
        {
            CamManager->ApplyArtilleryShake(ImpactLocation, Caliber);
        }

        // Apply damage modified by cover
        UOTDCoverDetectionComponent* CoverDetection = OTDChar->FindComponentByClass<UOTDCoverDetectionComponent>();
        const float Protection = CoverDetection ? CoverDetection->GetProtectionValue() : 0.0f;

        const float BaseDamage = CalculateDamageForCaliber(Caliber, Distance);
        const float FinalDamage = BaseDamage * (1.0f - Protection);

        UGameplayStatics::ApplyDamage(OTDChar, FinalDamage, nullptr, this, UDamageType::StaticClass());
    }
}
```

### EQS Integration for Artillery Targeting

Create EQS Query **EQS_ArtilleryTarget**:

1. **Generator:** ActorsOfClass (Player)
2. **Tests:**
   - Distance (prefer 30-100m from artillery)
   - **Exposure Score** (custom test):
     ```cpp
     float Score = Player->ExposureHeatmap->GetArtilleryAttractionScore();
     return Score > 0.5f ? Score : 0.0f; // Filter low exposure
     ```
   - Line of Sight (prefer visible)
   - Previous target timeout (don't spam same location)

3. **Scoring:**
   - Weight: Exposure Score × 0.7
   - Weight: Distance (inverse) × 0.3

### Sniper AI Integration

In sniper AI behavior tree:

```
Selector: Choose Target
├── Sequence: High-Value Target
│   ├── Check: Player Exposure > Medium
│   ├── Check: Player Stationary > 2 seconds
│   ├── Check: Distance < 300m
│   └── Task: Aim at Player
├── Sequence: Opportunity Target
│   ├── Check: Player Exposure > Low
│   └── Task: Aim at Player
└── Task: Patrol
```

---

## Tuning & Balancing

### Camera Feel

Adjust these for different feels:

**Responsive (arcade-like):**
```
Camera Lag: 0.5
Camera Rotation Lag: 2.0
Blend Time: 0.15s
```

**Weighty (realistic):**
```
Camera Lag: 2.0
Camera Rotation Lag: 8.0
Blend Time: 0.35s
```

**Balanced (recommended):**
```
Camera Lag: 1.5
Camera Rotation Lag: 5.0
Blend Time: 0.25s
```

### Survival Meter Balance

From design doc, these are tuned for realism:

- **Hunger:** 3-8/hr → ~7 days survival
- **Thirst:** 5-12/hr → ~1-3 days survival (MORE critical)
- **Stamina:** Regen 2.0/sec, sprint drains 15/sec
- **Warmth:** 0.5-3.0/min → hypothermia in 4 hours
- **Morale:** Shell shock at < 30

Test by simulating 24-hour gameplay loop and ensure meters feel right.

### Cover Detection

Adjust these if cover feels too sensitive/insensitive:

```cpp
CoverDetectionRange = 100.0f;  // Increase if not detecting cover in time
DetectionInterval = 0.1f;       // Lower for more responsive (costs performance)
TraceFanCount = 5;              // Increase for accuracy, decrease for performance
```

### Exposure Heatmap

Tune artillery attraction:

```cpp
ExposureAccumulationRate = 10.0f;  // How fast exposure builds (lower = more forgiving)
ExposureDecayRate = 5.0f;          // How fast it decays in cover (higher = faster forgiveness)
ArtilleryAttractionThreshold = 3.0f; // Seconds at high exposure before artillery (lower = more punishing)
```

**Balancing Goal:** Players should have 5-10 seconds to react when exposure reaches High before artillery starts ranging.

---

## Testing Checklist

### Camera System
- [ ] All 4 camera modes work and blend smoothly
- [ ] Shoulder swap transitions smoothly (both sides)
- [ ] ADS zoom feels responsive (<0.2s)
- [ ] Camera collision pushes in when close to walls
- [ ] No clipping through trench walls
- [ ] Set-piece locks (if used) can be disabled in options
- [ ] Fixed horizon mode works for accessibility
- [ ] Camera lag feels good at all movement speeds

### Character & Movement
- [ ] Standing/Crouching/Prone transitions smoothly
- [ ] Capsule height adjusts correctly per posture
- [ ] Can't stand when overhead obstruction
- [ ] Sprint drains stamina at 15/sec
- [ ] Sprint disabled when stamina < 30
- [ ] Auto-crouch when stamina < 30
- [ ] Movement speed modified by hunger/cold/stamina
- [ ] Leaning works (Q/E or bumpers)

### Survival Meters
- [ ] All 7 meters deplete at correct rates
- [ ] Stamina regenerates at 2.0/sec (modified by thirst/hunger)
- [ ] Death triggers correctly (thirst, hunger, cold, exhaustion)
- [ ] Diseases apply correctly (low hygiene → trench fever, etc.)
- [ ] Morale < 30 triggers shell shock
- [ ] All meter events fire and update UI
- [ ] Cascading effects work (low thirst → stamina regen penalty)

### Cover Detection
- [ ] Detects trenches correctly (tagged and geometry-based)
- [ ] Detects shell craters (below ground level)
- [ ] Detects dugout entrances and bunkers (tagged)
- [ ] Overhead protection detection works
- [ ] Protection values match design doc (60%, 75%, 85%, 95%, 99%)
- [ ] Events fire when entering/exiting cover
- [ ] Cover indicator UI shows correct protection %
- [ ] Can peek from high cover

### Exposure & AI
- [ ] Exposure builds when in the open
- [ ] Exposure decays when in cover (faster with better cover)
- [ ] Exposure level transitions trigger events
- [ ] High/Critical exposure attracts artillery (after 3s threshold)
- [ ] Artillery damage modified by cover protection
- [ ] Camera shake scales with shell caliber and distance
- [ ] Suppression effects apply (blur/grain)
- [ ] EQS queries exposure score correctly

### Input & Controls
- [ ] All keyboard inputs work
- [ ] All gamepad inputs work
- [ ] Shoulder swap on Middle Mouse / R3
- [ ] Lean on Q/E / bumpers
- [ ] Aim on Right Mouse / Left Trigger
- [ ] Sprint on Shift / L3
- [ ] Posture on C/X / buttons
- [ ] Input rebinding works (if implemented)

### Performance
- [ ] Cover detection doesn't tank FPS (check with 5+ traces/update)
- [ ] Exposure heatmap visibility checks aren't too expensive
- [ ] Camera blends don't cause hitches
- [ ] 60 FPS maintained on target hardware

### Polish
- [ ] Audio cues for exposure warnings
- [ ] UI indicators for all systems
- [ ] Tutorial messages for cover/exposure
- [ ] Death screen shows cause (starvation, hypothermia, etc.)
- [ ] Options menu for accessibility (camera lag, set-piece locks, etc.)

---

## Troubleshooting

### Camera clipping through walls
- Increase `CameraBoom->ProbeSize`
- Enable `bDoCollisionTest` on SpringArm
- Adjust socket offset Z value

### Cover not detected
- Check actor tags in level
- Enable `bDebugDrawTraces` on CoverDetection component
- Increase `CoverDetectionRange`
- Check trace fan is hitting geometry

### Exposure doesn't build
- Check for enemies with "Enemy" tag in level
- Verify `IsVisibleToEnemies()` returns true
- Check `VisibilityCheckInterval` isn't too high
- Enable logging in ExposureHeatmap component

### Stamina drains too fast
- Reduce activity costs (SprintStaminaDrain, etc.)
- Increase `StaminaRegenRate`
- Check for cascading effects (low thirst reducing regen)

### Artillery shake not working
- Verify player controller has `OTDCameraManager` set
- Check `GetCameraManager()` returns valid pointer
- Test with different calibers (75, 155, 305)

---

## Next Steps

After implementing this system:

1. **Add Hand-to-Hand Combat:**
   - Create melee weapon actors
   - Implement hit detection
   - Add stamina costs (10/sec from design doc)
   - Bayonet charges, trench knife, etc.

2. **Dynamic Cover Degradation:**
   - Shell impacts deform trench collision
   - Sandbags can be destroyed
   - Rain affects mud/height
   - Rebuilding with sandbag props

3. **Advanced AI:**
   - Full EQS integration for artillery
   - Sniper behavior trees
   - Enemy exposure tracking
   - Squad AI with cover awareness

4. **Mission Integration:**
   - Tie into mission structure from design doc
   - Tutorial mission teaching systems
   - Bombardment scenarios
   - Fort Vaux siege implementation

5. **Polish:**
   - Particle effects for shell impacts
   - Screen effects for suppression
   - Detailed UI with meter predictions
   - Full accessibility options

---

## References

- **Design Document:** `ONLY_THE_DEAD_DEVELOPMENT_BOOK.md`
- **Survival Systems:** Section 6.1-6.7
- **Combat Systems:** Section 7
- **Mission Structure:** Section 10
- **Historical Data:** `verdun_anthology/` directory
- **UE5 Enhanced Input:** https://docs.unrealengine.com/5.0/en-US/enhanced-input-in-unreal-engine/
- **UE5 Camera System:** https://docs.unrealengine.com/5.0/en-US/camera-system-in-unreal-engine/

---

## Credits

**System Design:** Based on Hellblade II camera work + WWI trench warfare survival
**Implementation:** C++ with Blueprint integration
**Historical Research:** Based on Verdun 1916 primary sources

---

## Version History

- **v1.0** (2025-11-10): Initial implementation
  - Hybrid camera stack (4 modes)
  - Shoulder-swap OTS
  - 7 survival meters
  - Procedural cover detection
  - Exposure heatmap
  - Posture system
  - Enhanced Input configuration

---

**For questions or issues, consult the design document or create a GitHub issue.**
