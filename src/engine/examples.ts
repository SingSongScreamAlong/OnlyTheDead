/**
 * KING OF BATTLE - ARTILLERY ENGINE
 * Usage Examples
 *
 * Demonstrates how to use the artillery engine
 */

import {
  createArtilleryEngine,
  createVerdunScenario,
  ArtilleryEngine
} from './artilleryEngine';
import {
  createFireMission,
  createArtilleryBattery,
  createPreparatoryBombardment,
  createCreepingBarrageMission,
  createCounterBatteryMission,
  assignBatteryToMission
} from './fireMission';
import {
  ShellType,
  BombardmentIntensity,
  FireMissionType,
  CoverType,
  Position3D
} from './types';

// ============================================================================
// EXAMPLE 1: BASIC SETUP
// ============================================================================

export function example1_BasicSetup(): void {
  console.log("=== Example 1: Basic Artillery Engine Setup ===\n");

  // Create the engine
  const engine = createArtilleryEngine();

  // Set player position
  engine.setPlayerPosition({ x: 0, y: 0, z: 0 });
  engine.setPlayerCover(CoverType.SHALLOW_TRENCH);

  // Create a French 75mm battery
  const frenchBattery = createArtilleryBattery(
    "French 75mm Battery A",
    "french",
    ShellType.FRENCH_75MM,
    6, // 6 guns
    { x: -1000, y: 0, z: 0 }, // 1km behind lines
    11000 // 11km range
  );

  engine.addBattery(frenchBattery);

  console.log("Artillery engine created!");
  console.log(`Battery: ${frenchBattery.name}`);
  console.log(`Guns: ${frenchBattery.guns.length}`);
  console.log(`Range: ${frenchBattery.range}m\n`);
}

// ============================================================================
// EXAMPLE 2: SIMPLE BOMBARDMENT
// ============================================================================

export function example2_SimpleBombardment(): void {
  console.log("=== Example 2: Simple Bombardment ===\n");

  const engine = createArtilleryEngine();

  // Create German battery
  const germanBattery = createArtilleryBattery(
    "German 77mm Battery",
    "german",
    ShellType.GERMAN_77MM,
    4,
    { x: 1000, y: 0, z: 0 },
    7800
  );

  engine.addBattery(germanBattery);

  // Create fire mission targeting French positions
  const targetPosition: Position3D = { x: 100, y: 0, z: 50 };

  const mission = createFireMission(
    FireMissionType.SUPPRESSION,
    targetPosition,
    "French trench line",
    3, // Medium priority
    600, // 10 minutes
    BombardmentIntensity.LIGHT
  );

  // Assign battery to mission
  assignBatteryToMission(germanBattery, mission);

  // Start the mission
  engine.startMission(mission);

  console.log(`Mission started: ${mission.type}`);
  console.log(`Duration: ${mission.bombardment.duration}s`);
  console.log(`Shells per minute: ${mission.bombardment.shellsPerMinute}`);
  console.log(`Total shells: ${(mission.bombardment.duration / 60) * mission.bombardment.shellsPerMinute}\n`);

  // Simulate for 60 seconds
  for (let i = 0; i < 60; i++) {
    engine.update(1.0); // 1 second per update
  }

  const stats = engine.getStats();
  console.log(`Shells fired: ${stats.totalShellsFired}`);
  console.log(`Shells in flight: ${stats.shellsInFlight}\n`);
}

// ============================================================================
// EXAMPLE 3: PLAYER DETECTION
// ============================================================================

export function example3_PlayerDetection(): void {
  console.log("=== Example 3: Player Detection System ===\n");

  const engine = createArtilleryEngine();

  // Player in a trench
  engine.setPlayerPosition({ x: 50, y: 0, z: 50 });
  engine.setPlayerCover(CoverType.SHALLOW_TRENCH);

  // Create attacking battery
  const battery = createArtilleryBattery(
    "German 155mm Battery",
    "german",
    ShellType.HOWITZER_155MM,
    6,
    { x: 2000, y: 0, z: 0 },
    12000
  );

  engine.addBattery(battery);

  // Create bombardment near player
  const mission = createFireMission(
    FireMissionType.HARASSING,
    { x: 100, y: 0, z: 100 }, // Near player
    "Harassing fire",
    2,
    300,
    BombardmentIntensity.HARASSING_FIRE
  );

  assignBatteryToMission(battery, mission);
  engine.startMission(mission);

  // Simulate
  console.log("Starting bombardment near player...\n");

  for (let i = 0; i < 30; i++) {
    engine.update(1.0);

    // Check what player can detect
    const detections = engine.getPlayerDetections();

    if (detections.length > 0) {
      console.log(`--- Second ${i + 1} ---`);
      for (const detection of detections) {
        console.log(`Shell detected!`);
        console.log(`  Type: ${detection.shellType}`);
        console.log(`  Threat: ${detection.threatLevel}`);
        console.log(`  Time to impact: ${detection.timeToImpact.toFixed(1)}s`);
        console.log(`  Distance: ${detection.distanceToPlayer.toFixed(1)}m`);
        console.log(`  Recognized: ${detection.isRecognized}`);
        console.log(`  Action: ${detection.recommendedAction}\n`);
      }
    }
  }
}

// ============================================================================
// EXAMPLE 4: HISTORICAL FEBRUARY 21, 1916 BOMBARDMENT
// ============================================================================

export function example4_February21Bombardment(): void {
  console.log("=== Example 4: Historical February 21, 1916 Opening Bombardment ===\n");

  const engine = createVerdunScenario();

  // Player is a French soldier in the trenches
  engine.setPlayerPosition({ x: 0, y: 0, z: 0 });
  engine.setPlayerCover(CoverType.SHALLOW_TRENCH);

  // Create multiple German batteries
  const batteries = [
    createArtilleryBattery("German 77mm Bty 1", "german", ShellType.GERMAN_77MM, 6, { x: 3000, y: 0, z: -500 }, 7800),
    createArtilleryBattery("German 77mm Bty 2", "german", ShellType.GERMAN_77MM, 6, { x: 3200, y: 0, z: 0 }, 7800),
    createArtilleryBattery("German 155mm Bty 1", "german", ShellType.HOWITZER_155MM, 4, { x: 5000, y: 0, z: -300 }, 12000),
    createArtilleryBattery("German 210mm Bty", "german", ShellType.GERMAN_210MM, 2, { x: 8000, y: 0, z: 0 }, 11000),
    createArtilleryBattery("German 420mm", "german", ShellType.GERMAN_420MM, 1, { x: 10000, y: 0, z: 0 }, 14000)
  ];

  batteries.forEach(b => engine.addBattery(b));

  // Create the apocalyptic opening bombardment
  const bombardment = createPreparatoryBombardment(
    { x: 0, y: 0, z: 0 }, // French lines
    2000, // 2km radius
    32400 // 9 hours
  );

  // This is DRUMFIRE - 170 shells per minute
  bombardment.bombardment.shellsPerMinute = 170;

  // Assign all batteries
  batteries.forEach(b => assignBatteryToMission(b, bombardment));

  engine.startMission(bombardment);

  console.log("FEBRUARY 21, 1916 - 07:15 AM");
  console.log("The apocalypse begins...\n");
  console.log(`Mission: ${bombardment.bombardment.intensity}`);
  console.log(`Duration: ${bombardment.bombardment.duration / 3600} hours`);
  console.log(`Shells per minute: ${bombardment.bombardment.shellsPerMinute}`);
  console.log(`Expected total shells: ${(bombardment.bombardment.duration / 60) * bombardment.bombardment.shellsPerMinute}\n`);

  // Simulate first 5 minutes
  console.log("Simulating first 5 minutes of bombardment...\n");

  let shellsNearPlayer = 0;
  let extremeThreats = 0;

  for (let i = 0; i < 300; i++) { // 5 minutes
    engine.update(1.0);

    const detections = engine.getPlayerDetections();

    for (const d of detections) {
      shellsNearPlayer++;
      if (d.threatLevel === 'extreme') {
        extremeThreats++;
      }
    }

    if (i % 60 === 0) {
      console.log(`Minute ${i / 60 + 1}: ${engine.getStats().shellsInFlight} shells in flight`);
    }
  }

  const stats = engine.getStats();
  console.log("\n=== After 5 Minutes ===");
  console.log(`Total shells fired: ${stats.totalShellsFired}`);
  console.log(`Shells player detected: ${shellsNearPlayer}`);
  console.log(`Extreme threats: ${extremeThreats}`);
  console.log(`Craters created: ${stats.cratersCreated}\n`);

  console.log("This bombardment would continue for 9 hours...");
  console.log("Expected total: ~92,000 shells\n");
}

// ============================================================================
// EXAMPLE 5: CREEPING BARRAGE (OCTOBER 1916)
// ============================================================================

export function example5_CreepingBarrage(): void {
  console.log("=== Example 5: Creeping Barrage (French Counteroffensive) ===\n");

  const engine = createVerdunScenario();

  // French batteries for creeping barrage
  const batteries = [
    createArtilleryBattery("French 75mm Bty 1", "french", ShellType.FRENCH_75MM, 8, { x: -2000, y: 0, z: -500 }, 11000),
    createArtilleryBattery("French 75mm Bty 2", "french", ShellType.FRENCH_75MM, 8, { x: -2000, y: 0, z: 500 }, 11000),
    createArtilleryBattery("French 155mm Bty", "french", ShellType.HOWITZER_155MM, 6, { x: -4000, y: 0, z: 0 }, 12000)
  ];

  batteries.forEach(b => engine.addBattery(b));

  // Create creeping barrage mission
  const startLine: Position3D = { x: 0, y: 0, z: 0 };
  const endLine: Position3D = { x: 1000, y: 0, z: 0 }; // Advance 1km

  const barrage = createCreepingBarrageMission(
    startLine,
    endLine,
    400, // 400m wide
    3600 // 1 hour advance
  );

  batteries.forEach(b => assignBatteryToMission(b, barrage));
  engine.startMission(barrage);

  console.log("OCTOBER 24, 1916 - DOUAUMONT COUNTEROFFENSIVE");
  console.log("Creeping barrage begins...\n");
  console.log(`Start: x=${startLine.x}, z=${startLine.z}`);
  console.log(`End: x=${endLine.x}, z=${endLine.z}`);
  console.log(`Width: ${barrage.bombardment.targetArea.width}m`);
  console.log(`Advance rate: ~50m per 3-4 minutes\n`);

  // Player is infantry advancing behind barrage
  engine.setPlayerPosition({ x: -50, y: 0, z: 0 }); // 50m behind barrage line

  console.log("Infantry must stay 50-100m behind the barrage...\n");

  // Simulate 15 minutes
  for (let i = 0; i < 900; i++) {
    engine.update(1.0);

    // Player advances
    if (i % 180 === 0) { // Every 3 minutes
      const currentPos = engine.getState().playerPosition;
      engine.setPlayerPosition({
        x: currentPos.x + 50,
        y: 0,
        z: 0
      });
      console.log(`Minute ${i / 60}: Infantry advances to x=${currentPos.x + 50}m`);
    }
  }

  console.log("\nBarrage continues advancing ahead of infantry...\n");
}

// ============================================================================
// EXAMPLE 6: COUNTER-BATTERY FIRE
// ============================================================================

export function example6_CounterBattery(): void {
  console.log("=== Example 6: Counter-Battery Fire ===\n");

  const engine = createArtilleryEngine();

  // French observer spots German battery
  const enemyBatteryPosition: Position3D = { x: 3000, y: 0, z: 500 };

  console.log("French observer reports enemy battery!");
  console.log(`Position: x=${enemyBatteryPosition.x}, z=${enemyBatteryPosition.z}\n`);

  // Create French heavy artillery for counter-battery
  const frenchHeavy = createArtilleryBattery(
    "French 155mm Heavy",
    "french",
    ShellType.HOWITZER_155MM,
    6,
    { x: -5000, y: 0, z: 0 },
    16200
  );

  engine.addBattery(frenchHeavy);

  // Create counter-battery mission
  const cbMission = createCounterBatteryMission(
    enemyBatteryPosition,
    85 // 85% observer accuracy
  );

  assignBatteryToMission(frenchHeavy, cbMission);
  engine.startMission(cbMission);

  console.log("Counter-battery fire mission initiated!");
  console.log(`Target area radius: ${cbMission.bombardment.targetArea.radius}m`);
  console.log(`Duration: ${cbMission.bombardment.duration / 60} minutes`);
  console.log(`Concentration fire pattern\n`);

  // Simulate
  for (let i = 0; i < 60; i++) {
    engine.update(1.0);
  }

  const stats = engine.getStats();
  console.log(`Shells fired at enemy battery: ${stats.totalShellsFired}\n`);
}

// ============================================================================
// RUN ALL EXAMPLES
// ============================================================================

export function runAllExamples(): void {
  example1_BasicSetup();
  console.log("\n" + "=".repeat(70) + "\n");

  example2_SimpleBombardment();
  console.log("\n" + "=".repeat(70) + "\n");

  example3_PlayerDetection();
  console.log("\n" + "=".repeat(70) + "\n");

  example4_February21Bombardment();
  console.log("\n" + "=".repeat(70) + "\n");

  example5_CreepingBarrage();
  console.log("\n" + "=".repeat(70) + "\n");

  example6_CounterBattery();
}

// If running this file directly
if (require.main === module) {
  runAllExamples();
}
