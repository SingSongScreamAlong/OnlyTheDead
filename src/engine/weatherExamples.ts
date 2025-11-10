/**
 * KING OF BATTLE - ARTILLERY WEATHER SYSTEM EXAMPLES
 *
 * Artillery as Environmental Hazard
 *
 * These examples show how artillery works as an autonomous "weather system"
 * that the player must survive, not control.
 */

import {
  createVerdunArtilleryWeather,
  createFebruary21Weather,
  createQuietPeriodWeather
} from './artilleryWeather';
import { CoverType } from './types';

// ============================================================================
// EXAMPLE 1: BASIC ARTILLERY WEATHER
// ============================================================================

export function weatherExample1_BasicSurvival(): void {
  console.log("=== Artillery Weather Example 1: Surviving the Storm ===\n");

  // Create the weather system - it runs automatically
  const weather = createVerdunArtilleryWeather(
    { x: 0, y: 0, z: 0 }, // French trench position
    new Date('1916-03-15') // Mid-March 1916
  );

  const engine = (weather as any).engine; // Access underlying engine

  // Player starts in a trench
  engine.setPlayerPosition({ x: 0, y: 0, z: 0 });
  engine.setPlayerCover(CoverType.SHALLOW_TRENCH);

  console.log("You are a French soldier in the trenches...");
  console.log("Artillery never stops. You can only survive.\n");

  // The weather system runs automatically
  // Player doesn't control anything - they just react
  let shellsDetected = 0;
  let extremeThreats = 0;

  for (let minute = 0; minute < 10; minute++) {
    console.log(`\n--- Minute ${minute + 1} ---`);

    // Update weather (60 seconds at 1 second per tick)
    for (let i = 0; i < 60; i++) {
      weather.update(1.0);
      engine.update(1.0);
    }

    // Check what player experiences
    const detections = engine.getPlayerDetections();
    shellsDetected += detections.length;

    for (const detection of detections) {
      if (detection.threatLevel === 'extreme') {
        extremeThreats++;
        console.log(`  💀 EXTREME DANGER: ${detection.recommendedAction}`);
      } else if (detection.threatLevel === 'high') {
        console.log(`  ⚠️  High danger: ${detection.recommendedAction}`);
      }
    }

    // Show weather status
    const status = weather.getStatus();
    console.log(`  Weather: ${status.currentIntensity}`);
    if (status.isStormActive) {
      console.log(`  🌩️  STORM ACTIVE - ${(status.stormTimeRemaining! / 60).toFixed(1)} minutes remaining`);
    } else if (status.nextStormIn) {
      console.log(`  Next storm in ${(status.nextStormIn / 60).toFixed(1)} minutes`);
    }

    const stats = engine.getStats();
    console.log(`  Shells in area: ${stats.shellsInFlight}`);
  }

  console.log("\n=== 10 Minutes Later ===");
  console.log(`Total shells detected near you: ${shellsDetected}`);
  console.log(`Extreme threats survived: ${extremeThreats}`);
  console.log("\nYou survived 10 minutes. 303 days to go...\n");
}

// ============================================================================
// EXAMPLE 2: FEBRUARY 21 APOCALYPSE
// ============================================================================

export function weatherExample2_February21(): void {
  console.log("=== Artillery Weather Example 2: February 21, 1916 ===\n");

  console.log("FEBRUARY 21, 1916 - 07:15 AM");
  console.log("The world is ending.\n");

  // Create February 21 weather - CONSTANT DRUMFIRE
  const weather = createFebruary21Weather({ x: 0, y: 0, z: 0 });
  const engine = (weather as any).engine;

  engine.setPlayerPosition({ x: 0, y: 0, z: 0 });
  engine.setPlayerCover(CoverType.DEEP_DUGOUT); // Even deep dugouts barely help

  console.log("You are in a deep dugout.");
  console.log("The bombardment never stops...\n");

  let totalShells = 0;
  let nearMisses = 0;

  // Simulate just 5 minutes of the 9-hour bombardment
  for (let minute = 0; minute < 5; minute++) {
    for (let i = 0; i < 60; i++) {
      weather.update(1.0);
      engine.update(1.0);
    }

    const detections = engine.getPlayerDetections();
    totalShells += detections.length;

    // Count near misses (within 50m)
    for (const d of detections) {
      if (d.distanceToPlayer <= 50) {
        nearMisses++;
      }
    }

    const stats = engine.getStats();
    console.log(`Minute ${minute + 1}: ${stats.shellsInFlight} shells in flight simultaneously`);
  }

  console.log("\n=== After 5 Minutes ===");
  console.log(`Shells detected: ${totalShells}`);
  console.log(`Near misses (<50m): ${nearMisses}`);
  console.log(`\nThis bombardment will continue for 9 hours.`);
  console.log(`Total expected: ~92,000 shells.`);
  console.log(`\nSurvival is luck.\n`);
}

// ============================================================================
// EXAMPLE 3: QUIET PERIOD (Relatively Safe)
// ============================================================================

export function weatherExample3_QuietPeriod(): void {
  console.log("=== Artillery Weather Example 3: Quiet Period ===\n");

  // Rest area - still dangerous but manageable
  const weather = createQuietPeriodWeather({ x: 0, y: 0, z: 0 });
  const engine = (weather as any).engine;

  engine.setPlayerPosition({ x: 0, y: 0, z: 0 });
  engine.setPlayerCover(CoverType.SHELL_CRATER);

  console.log("July 1916 - Relatively quiet period");
  console.log("Harassing fire continues, but you can breathe...\n");

  let shellsHeard = 0;
  let stormsWeathered = 0;

  for (let minute = 0; minute < 30; minute++) {
    for (let i = 0; i < 60; i++) {
      weather.update(1.0);
      engine.update(1.0);
    }

    const detections = engine.getPlayerDetections();
    shellsHeard += detections.length;

    const status = weather.getStatus();

    // Log only significant events
    if (status.isStormActive && minute % 5 === 0) {
      console.log(`Minute ${minute + 1}: 🌩️  Storm in progress`);
      stormsWeathered++;
    } else if (detections.length > 5) {
      console.log(`Minute ${minute + 1}: Surge of fire - ${detections.length} shells`);
    }
  }

  console.log("\n=== After 30 Minutes ===");
  console.log(`Shells heard: ${shellsHeard}`);
  console.log(`Artillery storms: ${stormsWeathered}`);
  console.log(`\nEven in "quiet" periods, death can find you.\n`);
}

// ============================================================================
// EXAMPLE 4: PLAYER EXPERIENCE - SURVIVING A STORM
// ============================================================================

export function weatherExample4_SurvivingAStorm(): void {
  console.log("=== Artillery Weather Example 4: Surviving a Storm ===\n");

  const weather = createVerdunArtilleryWeather(
    { x: 0, y: 0, z: 0 },
    new Date('1916-04-20')
  );
  const engine = (weather as any).engine;

  // Player narrative
  let playerPosition = { x: 0, y: 0, z: 0 };
  let playerCover = CoverType.SHALLOW_TRENCH;

  engine.setPlayerPosition(playerPosition);
  engine.setPlayerCover(playerCover);

  console.log("You are on patrol in no-man's land...\n");

  let stormStarted = false;
  let playerInDugout = false;

  for (let second = 0; second < 600; second++) { // 10 minutes
    weather.update(1.0);
    engine.update(1.0);

    const status = weather.getStatus();
    const detections = engine.getPlayerDetections();

    // Check if storm is starting
    if (status.isStormActive && !stormStarted) {
      stormStarted = true;
      console.log(`\n[${second}s] 🌩️  STORM INCOMING!`);
      console.log(`Artillery intensity rising dramatically!`);
      console.log(`Multiple batteries opening fire!\n`);
    }

    // Player reactions to threats
    for (const detection of detections) {
      if (detection.threatLevel === 'extreme' && !playerInDugout) {
        console.log(`[${second}s] 💀 ${detection.shellType} incoming!`);
        console.log(`       Distance: ${detection.distanceToPlayer.toFixed(0)}m`);
        console.log(`       Time: ${detection.timeToImpact.toFixed(1)}s`);
        console.log(`       >> SPRINTING TO DUGOUT!`);
        playerCover = CoverType.DEEP_DUGOUT;
        playerInDugout = true;
        engine.setPlayerCover(playerCover);
      }
    }

    // Storm ends
    if (stormStarted && !status.isStormActive) {
      console.log(`\n[${second}s] Storm passing...`);
      console.log(`Artillery returning to baseline.\n`);
      if (playerInDugout) {
        console.log(`[${second}s] Emerging from dugout...`);
        playerCover = CoverType.SHALLOW_TRENCH;
        playerInDugout = false;
        engine.setPlayerCover(playerCover);
      }
      break;
    }
  }

  console.log("\nYou survived the storm.");
  console.log("But there will be another.\n");
}

// ============================================================================
// EXAMPLE 5: DAILY CYCLE
// ============================================================================

export function weatherExample5_DailyCycle(): void {
  console.log("=== Artillery Weather Example 5: A Day in the Life ===\n");

  const weather = createVerdunArtilleryWeather(
    { x: 0, y: 0, z: 0 },
    new Date('1916-05-15')
  );
  const engine = (weather as any).engine;

  engine.setPlayerPosition({ x: 0, y: 0, z: 0 });
  engine.setPlayerCover(CoverType.SHALLOW_TRENCH);

  console.log("May 15, 1916 - A typical day at Verdun\n");

  const hourlyStats: number[] = [];

  // Simulate 24 hours
  for (let hour = 0; hour < 24; hour++) {
    let shellsThisHour = 0;

    // Each hour = 3600 seconds
    for (let second = 0; second < 3600; second++) {
      weather.update(1.0);
      engine.update(1.0);

      shellsThisHour += engine.getPlayerDetections().length;
    }

    hourlyStats.push(shellsThisHour);

    const status = weather.getStatus();
    const stormIndicator = status.isStormActive ? " 🌩️  STORM" : "";
    console.log(`Hour ${hour.toString().padStart(2, '0')}:00 - ${shellsThisHour} shells detected${stormIndicator}`);
  }

  const totalShells = hourlyStats.reduce((a, b) => a + b, 0);
  const avgPerHour = (totalShells / 24).toFixed(0);

  console.log("\n=== 24 Hours Later ===");
  console.log(`Total shells detected: ${totalShells}`);
  console.log(`Average per hour: ${avgPerHour}`);
  console.log(`\nAnother day survived. 302 more to go.\n`);
}

// ============================================================================
// RUN ALL WEATHER EXAMPLES
// ============================================================================

export function runAllWeatherExamples(): void {
  weatherExample1_BasicSurvival();
  console.log("\n" + "=".repeat(70) + "\n");

  weatherExample2_February21();
  console.log("\n" + "=".repeat(70) + "\n");

  weatherExample3_QuietPeriod();
  console.log("\n" + "=".repeat(70) + "\n");

  weatherExample4_SurvivingAStorm();
  console.log("\n" + "=".repeat(70) + "\n");

  weatherExample5_DailyCycle();
}

if (require.main === module) {
  runAllWeatherExamples();
}
