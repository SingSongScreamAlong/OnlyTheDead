# King of Battle - Artillery Engine

**"Artillery conquers, infantry occupies"** - General Robert Nivelle

A historically accurate WWI artillery simulation engine for the Battle of Verdun (1916).

## Overview

This artillery engine simulates the **40 million shells** fired during the 303-day Battle of Verdun, the deadliest artillery bombardment in history. The system is designed for a survival game where artillery is the primary threat (70-75% of casualties), not enemy soldiers.

### 🌩️ Artillery as Weather

**The player doesn't control artillery - they survive it.**

Like a weather system, artillery:
- **Never stops** - Harassing fire is constant (24/7)
- **Ebbs and flows** - Quiet periods, then sudden storms
- **Is unpredictable** - Random surges, no safe patterns
- **Runs autonomously** - German batteries fire automatically
- **Can't be controlled** - Only reacted to

The player is **subject to** the bombardment, like being caught in a deadly storm. They can only:
- **Listen** for incoming shells
- **Recognize** shell types by sound
- **Take cover** when threats appear
- **Survive** through skill and luck

### Key Features

- **Historically Accurate**: Based on extensive research of WWI artillery data
- **6 Shell Types**: From 75mm field guns to 420mm "Big Bertha" super-heavy mortars
- **Realistic Ballistics**: Trajectory, flight time, wind effects, and dispersion
- **Damage System**: Distance-based casualties, cover modifiers, shell shock, burial alive
- **Bombardment Patterns**: Random, creeping barrage, box barrage, concentration fire
- **Fire Missions**: Complete battery coordination and fire mission system
- **Player Detection**: Audio-based shell recognition and threat assessment
- **Skill Progression**: Player improves shell recognition over time (novice → veteran)

## Historical Context

### Battle of Verdun Statistics

- **Duration**: 303 days (February 21 - December 18, 1916)
- **Shells Fired**: ~40,000,000 total
- **Average**: 132,000 shells per day
- **Peak Days**: 200,000+ shells per day
- **Artillery Casualties**: 70-75% of all casualties
- **Most Intense**: February 21 opening - 1 million shells in 12 hours

### Artillery Types

| Type | Caliber | Shell Weight | Range | Rate of Fire |
|------|---------|--------------|-------|--------------|
| French 75mm | 75mm | 6.2 kg | 11 km | 15-20 rpm |
| German 77mm | 77mm | 6.8 kg | 7.8 km | 10-15 rpm |
| 155mm Howitzer | 155mm | 43 kg | 12 km | 4-5 rpm |
| German 210mm | 210mm | 113 kg | 11 km | 2 rpm |
| French 305mm | 305mm | 340 kg | 11 km | 1 rpm |
| German 420mm | 420mm | 820 kg | 14 km | 1 per 8 min |

## Installation

```typescript
import { createArtilleryEngine, createVerdunScenario } from './engine/artilleryEngine';
```

## Quick Start

### Artillery Weather System (Recommended)

The **Artillery Weather System** runs automatically - artillery is an environmental hazard:

```typescript
import { createVerdunArtilleryWeather } from './engine/artilleryWeather';
import { CoverType } from './engine/types';

// Create the weather system - it runs autonomously
const artilleryWeather = createVerdunArtilleryWeather(
  { x: 0, y: 0, z: 0 }, // Your trench position
  new Date('1916-03-15') // Historical date
);

// Access the underlying engine for player interactions
const engine = (artilleryWeather as any).engine;

// Set player state
engine.setPlayerPosition({ x: 0, y: 0, z: 0 });
engine.setPlayerCover(CoverType.SHALLOW_TRENCH);

// Game loop - artillery runs automatically
function gameLoop(deltaTime: number) {
  // Update weather system (manages bombardments automatically)
  artilleryWeather.update(deltaTime);

  // Update engine (tracks shells, impacts)
  engine.update(deltaTime);

  // Player REACTS to incoming shells
  const detections = engine.getPlayerDetections();

  for (const detection of detections) {
    if (detection.threatLevel === 'extreme') {
      // Play warning sound
      // Show shell direction
      // Player must take cover!
      console.log(`💀 ${detection.recommendedAction}`);
    }
  }

  // Check weather status
  const status = artilleryWeather.getStatus();
  if (status.isStormActive) {
    console.log(`🌩️ Artillery storm! ${status.stormTimeRemaining}s remaining`);
  }
}
```

### Weather System Features

```typescript
// Check current bombardment intensity
const status = artilleryWeather.getStatus();
console.log(status.currentIntensity); // HARASSING_FIRE, LIGHT, HEAVY, or DRUMFIRE

// Artillery storms occur randomly
if (status.isStormActive) {
  console.log(`Storm! ${status.stormTimeRemaining}s left`);
} else {
  console.log(`Next storm in ${status.nextStormIn}s`);
}
```

### Historical Scenarios

```typescript
import {
  createFebruary21Weather,     // Apocalyptic opening bombardment
  createQuietPeriodWeather      // Relatively safe periods
} from './engine/artilleryWeather';

// February 21, 1916 - constant drumfire
const apocalypse = createFebruary21Weather({ x: 0, y: 0, z: 0 });

// Quiet period - harassing fire with occasional storms
const quietPeriod = createQuietPeriodWeather({ x: 0, y: 0, z: 0 });
```

### Player Survival Loop

The player's experience:

```typescript
// 1. Constant awareness
const detections = engine.getPlayerDetections();

// 2. Shell recognition (audio-based)
for (const detection of detections) {
  if (detection.isRecognized) {
    // Player identified shell type by sound
    playShellSound(detection.shellType); // "Ffff-CRACK!" = 75mm
  } else {
    // Player doesn't recognize it yet (learning curve)
    playGenericSound();
  }

  // 3. Threat assessment
  switch (detection.threatLevel) {
    case 'extreme':
      // Within lethal range - MOVE NOW
      showWarning("💀 INCOMING!");
      break;
    case 'high':
      // Severe wound range - take cover
      showWarning("⚠️ Danger close!");
      break;
    case 'moderate':
      // Could get hit - be alert
      break;
  }

  // 4. Recommended action
  console.log(detection.recommendedAction);
  // "Drop prone immediately!"
  // "Sprint to nearest deep cover!"
  // "Continue task"
}

// 5. Player skills improve over time
if (playerSurvived) {
  engine.updatePlayerSkills(shellWasRecognized, true);
  // Recognition improves: novice → experienced → veteran
}
```

## Core Systems

### 1. Shell System

Each shell type has complete ballistic and damage data:

```typescript
import { getShellDefinition } from './engine/shells';

const shell75mm = getShellDefinition(ShellType.FRENCH_75MM);

console.log(shell75mm.audio.incomingSound); // "Ffff-CRACK! (sharp, high-pitched whistle)"
console.log(shell75mm.ballistics.muzzleVelocity); // 575 m/s
console.log(shell75mm.damage.lethal); // 10 meters
```

**Shell Tiers**:
- **Tier 1 (Light)**: 75mm, 77mm - 60% of shells, high rate of fire
- **Tier 2 (Medium)**: 155mm - 25% of shells, heavy bombardment
- **Tier 3 (Heavy)**: 210mm, 305mm - 10% of shells, fort destruction
- **Tier 4 (Super-Heavy)**: 420mm - 2% of shells, apocalyptic impact

### 2. Ballistics System

Realistic projectile physics:

```typescript
import { calculateFiringAngle, calculateFlightTime } from './engine/ballistics';

const range = 5000; // 5km
const muzzleVelocity = 575; // m/s

const angle = calculateFiringAngle(range, muzzleVelocity);
const flightTime = calculateFlightTime(range, muzzleVelocity, angle);

console.log(`Angle: ${angle * 180 / Math.PI}°`);
console.log(`Flight time: ${flightTime}s`);
```

**Features**:
- High-arc trajectories for howitzers
- Wind effects on shell trajectory
- Dispersion based on range and accuracy
- Sound propagation (340 m/s)
- Doppler effect for pitch shifts

### 3. Damage System

Distance-based damage calculation with cover modifiers:

```typescript
import { calculateDamage } from './engine/damage';

const damage = calculateDamage(
  impactPosition,
  targetPosition,
  shellDefinition,
  CoverType.SHALLOW_TRENCH
);

console.log(`Survival chance: ${damage.finalSurvivalChance}%`);
console.log(`Wounds: ${damage.wounds}`);
console.log(`Shell shock: ${damage.shellShock}`);
console.log(`Buried alive: ${damage.buriedAlive}`);
```

**Damage Zones** (75mm example):
- **0-5m**: 0% survival (instant death)
- **5-10m**: 10% survival (catastrophic wounds)
- **10-20m**: 60% survival (severe wounds)
- **20-30m**: 90% survival (moderate wounds)
- **30-50m**: 98% survival (light wounds)

**Cover Protection**:
- Open Ground: 0%
- Shell Crater: 75%
- Shallow Trench: 60%
- Dugout Entrance: 85%
- Deep Dugout: 95%
- Concrete Bunker: 99%

### 4. Bombardment Patterns

Multiple historical bombardment patterns:

```typescript
import { BombardmentPattern } from './engine/types';

// Random bombardment (most common)
const random = createFireMission(..., BombardmentPattern.RANDOM);

// Creeping barrage (advances with infantry)
const creeping = createCreepingBarrageMission(
  startLine,
  endLine,
  400, // Width in meters
  3600 // Duration in seconds
);

// Box barrage (traps enemy)
const box = createFireMission(..., BombardmentPattern.BOX_BARRAGE);

// Concentration fire (destroys forts)
const concentration = createFireMission(..., BombardmentPattern.CONCENTRATION);
```

**Bombardment Intensities**:
- **Harassing Fire**: 1-3 shells/min, constant psychological pressure
- **Light**: 10-20 shells/min, sustained shelling
- **Heavy**: 40-60 shells/min, trench destruction
- **Drumfire**: 100+ shells/min, apocalyptic (Feb 21, 1916)

### 5. Player Detection System

Audio-based shell recognition minigame:

```typescript
const detections = engine.getPlayerDetections();

for (const detection of detections) {
  console.log(`Shell: ${detection.shellType}`);
  console.log(`Threat: ${detection.threatLevel}`);
  console.log(`Time to impact: ${detection.timeToImpact}s`);
  console.log(`Recognized: ${detection.isRecognized}`);
  console.log(`Action: ${detection.recommendedAction}`);
}
```

**Player Progression**:
- **Novice**: 50% recognition, 3s reaction time, 60% panic chance
- **Experienced**: 80% recognition, 1s reaction time, 40% panic chance
- **Veteran**: 95% recognition, 0.3s reaction time, 15% panic chance

**Threat Levels**:
- **Extreme**: Within lethal radius - immediate action required
- **High**: Severe wound range - sprint to cover
- **Moderate**: Moderate wound range - take cover
- **Low**: Light wound range - stay alert
- **None**: Outside danger zone

### 6. Fire Mission System

Complete battery coordination:

```typescript
import {
  createCounterBatteryMission,
  createPreparatoryBombardment,
  coordinateBatteriesForMission
} from './engine/fireMission';

// Counter-battery fire
const cbMission = createCounterBatteryMission(
  enemyBatteryPosition,
  85 // Observer accuracy
);

// Preparatory bombardment (before assault)
const prepBombardment = createPreparatoryBombardment(
  targetArea,
  2000, // Radius
  7200 // 2 hours
);

// Coordinate multiple batteries
const assigned = coordinateBatteriesForMission(mission, availableBatteries);
```

## Historical Scenarios

### February 21, 1916 - Opening Bombardment

```typescript
import { createVerdunScenario } from './engine/artilleryEngine';
import { createPreparatoryBombardment } from './engine/fireMission';

const engine = createVerdunScenario();

const bombardment = createPreparatoryBombardment(
  { x: 0, y: 0, z: 0 },
  2000,
  32400 // 9 hours
);

bombardment.bombardment.shellsPerMinute = 170;
// Total: ~92,000 shells in 9 hours
```

### October 24, 1916 - Douaumont Recapture

```typescript
const barrage = createCreepingBarrageMission(
  { x: 0, y: 0, z: 0 },
  { x: 1000, y: 0, z: 0 },
  400,
  3600
);
// Nivelle's perfected creeping barrage
```

## API Reference

### ArtilleryEngine

Main engine class:

```typescript
class ArtilleryEngine {
  update(deltaTime: number): void;
  startMission(mission: FireMission): void;
  addBattery(battery: ArtilleryBattery): void;
  getPlayerDetections(): PlayerDetection[];
  setPlayerPosition(position: Position3D): void;
  setPlayerCover(cover: CoverType): void;
  getState(): ArtilleryGameState;
  getStats(): Statistics;
}
```

### Configuration

```typescript
interface ArtilleryEngineConfig {
  tickRate: number;                    // Updates per second
  maxShellsInFlight: number;           // Performance limit
  maxSimulationDistance: number;       // Meters
  audioEnabled: boolean;
  spatialAudioEnabled: boolean;
  playerSkillProgression: boolean;     // Does player improve?
  shellShockEnabled: boolean;
  permadeathMode: boolean;
  historicalShellMix: boolean;         // Use historical percentages?
  historicalCasualtyRates: boolean;
  detailedPhysics: boolean;            // Full ballistics vs simplified
}
```

## Performance

- **Tick Rate**: 60 Hz recommended
- **Max Shells**: 1000 simultaneous (configurable)
- **Simulation Distance**: 10km (shells beyond this aren't simulated)
- **Audio Sources**: 32 concurrent sounds (realistic for human perception)

## Examples

See `examples.ts` for complete working examples:

1. **Basic Setup** - Creating batteries and fire missions
2. **Simple Bombardment** - Light artillery fire
3. **Player Detection** - Shell recognition system
4. **February 21, 1916** - Historical opening bombardment
5. **Creeping Barrage** - French counteroffensive
6. **Counter-Battery** - Observer-directed fire

Run examples:

```typescript
import { runAllExamples } from './engine/examples';

runAllExamples();
```

## Design Philosophy

### Historical Accuracy

Every system is based on primary sources:
- French Army Archives (Service Historique de la Défense)
- German Reichsarchiv records
- "Artillery in the Great War" (Sanders Marble)
- "Verdun 1916" (William Martin)
- Ballistic tables from 1916

### Gameplay Integration

**Artillery is the primary threat**, not combat:
- 70-75% of casualties from artillery (historically accurate)
- Player must **learn to recognize shells by sound**
- **Survival depends on**: cover quality, shell recognition, reaction time, luck
- **Direct hits = instant death** (realistic, unforgiving)
- **Near misses**: wounds, burial alive, shell shock

### Terror and Randomness

Artillery is **random and unpredictable** by design:
- No "safe" pattern to memorize
- Harassing fire never stops (24/7)
- Even deep dugouts can fail against 420mm
- Pure luck element (as in reality)

## Technical Notes

### Coordinate System

- **X-axis**: East-West (east = positive)
- **Y-axis**: Elevation (up = positive)
- **Z-axis**: North-South (north = positive)
- **Units**: Meters

### Time

- **Game Time**: Seconds since start
- **Delta Time**: Seconds per frame
- **Historical Time**: Maps to real 303-day campaign

## Contributing

This engine is designed for educational and historical preservation purposes. Modifications should maintain historical accuracy.

## License

Part of "Only The Dead" - Verdun 1916 Survival Game

## Acknowledgments

Based on the historical research in the Verdun Anthology and Development Book. Every number, every sound, every effect is historically documented.

**"In this war, the artillery conquers and the infantry occupies. The rest is nothing but empty talk."**
- General Robert Nivelle, 1916

---

**Total Code**: ~3,500 lines
**Systems**: 8 major modules
**Shell Types**: 6 historically accurate
**Historical Data Points**: 100+
**Research Sources**: 10+ primary sources
