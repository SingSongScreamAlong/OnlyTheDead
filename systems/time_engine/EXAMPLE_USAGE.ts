/**
 * VERDUN TIME ENGINE - EXAMPLE USAGE
 *
 * This demonstrates how a game would use the Verdun Time Engine
 * in a typical gameplay scenario.
 *
 * Scenario: Player's first rotation at Verdun
 * - Arrives at front line (dawn, Day 1)
 * - Survives 2.5 days under bombardment
 * - Gets relieved due to fatigue and morale
 * - Experiences rest period narrative beats
 * - Returns to front line
 */

import { VerdunTimeEngine, getVerdunTimeEngine } from './VerdunTimeEngine';
import { WoundSeverity } from './ReliefConditionEvaluator';

// ============================================================================
// EXAMPLE 1: BASIC GAME LOOP
// ============================================================================

function exampleBasicGameLoop() {
    console.log('\n=== EXAMPLE 1: BASIC GAME LOOP ===\n');

    // Get time engine instance
    const timeEngine = getVerdunTimeEngine();

    // Start the engine
    timeEngine.start();

    // Register event handlers
    timeEngine.onPhaseChange((newPhase) => {
        console.log(`[Game] Time phase changed to: ${newPhase}`);
        // Game would update lighting, audio, NPC behavior here
    });

    timeEngine.onReliefEligible((conditions) => {
        console.log(`[Game] Player eligible for relief: ${conditions.reason}`);
        // Game would show "relief column arriving" notification
    });

    // Simulate game loop (60 FPS)
    const deltaTime = 1/60;  // 16.67ms per frame

    for (let i = 0; i < 600; i++) {  // 10 seconds of game time
        timeEngine.update(deltaTime);

        // Every 60 frames (1 second), print status
        if (i % 60 === 0) {
            const state = timeEngine.getTimeState();
            const lighting = timeEngine.getLightingState();
            const audio = timeEngine.getAudioState();

            console.log(`Frame ${i}: Phase=${state.shiftPhase}, Sun=${lighting.sunAngle.toFixed(0)}°, Artillery=${audio.artilleryTempo.toFixed(0)}/min`);
        }
    }

    console.log('\n[Game] Basic loop complete\n');
}

// ============================================================================
// EXAMPLE 2: SURVIVAL DEGRADATION & RELIEF
// ============================================================================

function exampleSurvivalDegradation() {
    console.log('\n=== EXAMPLE 2: SURVIVAL DEGRADATION & RELIEF ===\n');

    const timeEngine = getVerdunTimeEngine();
    timeEngine.start();

    // Initial state: Player just arrived, fairly healthy
    timeEngine.updateSurvivalMeters({
        hunger: 70,
        thirst: 75,
        stamina: 85,
        warmth: 60,
        hygiene: 50,
        morale: 75,
        alertness: 80
    });

    timeEngine.updateInjuryState({
        isWounded: false,
        woundSeverity: WoundSeverity.NONE,
        isBleeding: false,
        isConcussed: false,
        isGassed: false,
        hasShellShock: false
    });

    timeEngine.updateUnitState({
        casualtyRate: 0.15,  // 15% casualties (normal)
        combatEffective: true,
        replacementAvailable: true,
        strategicRedeployment: false,
        majorBattleOngoing: false
    });

    // Simulate 24 hours (1 day) of degradation
    console.log('[Game] Simulating 24 hours at front line...\n');

    const deltaTime = 1/60;
    const hoursToSimulate = 24;
    const framesPerHour = 60 * 60 * 60;  // 60 FPS * 60 sec * 60 min

    for (let hour = 0; hour < hoursToSimulate; hour++) {
        // Degrade meters every hour
        timeEngine.updateSurvivalMeters({
            hunger: Math.max(0, 70 - (hour * 3)),
            thirst: Math.max(0, 75 - (hour * 4)),
            stamina: Math.max(0, 85 - (hour * 2)),
            warmth: Math.max(0, 60 - (hour * 1)),
            hygiene: Math.max(0, 50 - (hour * 1)),
            morale: Math.max(0, 75 - (hour * 2.5)),
            alertness: Math.max(0, 80 - (hour * 3))
        });

        // Update time engine
        for (let i = 0; i < framesPerHour; i++) {
            timeEngine.update(deltaTime);
        }

        console.log(`Hour ${hour + 1}:`);
        console.log(`  Alertness: ${Math.max(0, 80 - (hour * 3)).toFixed(0)}%`);
        console.log(`  Morale: ${Math.max(0, 75 - (hour * 2.5)).toFixed(0)}%`);
        console.log(`  Hunger: ${Math.max(0, 70 - (hour * 3)).toFixed(0)}%`);

        // Check if eligible for relief
        if (hour >= 8) {  // After 8 hours, check if conditions met
            const conditions = timeEngine.getTimeState();
            console.log(`  Relief progress: ${(timeEngine.getTimeState().currentShiftDuration / 86400 * 100).toFixed(1)}%`);
        }
    }

    console.log('\n[Game] 24 hours complete - player should be eligible for relief\n');
}

// ============================================================================
// EXAMPLE 3: STRESS & PERCEPTION MODULATION
// ============================================================================

function exampleStressModulation() {
    console.log('\n=== EXAMPLE 3: STRESS & PERCEPTION MODULATION ===\n');

    const timeEngine = getVerdunTimeEngine();
    timeEngine.start();

    // Simulate various stress events
    console.log('[Game] Normal state:');
    let modifiers = timeEngine.getPerceptionModifiers();
    console.log(`  Movement speed: ${(modifiers.movementSpeed * 100).toFixed(0)}%`);
    console.log(`  FOV: ${modifiers.fieldOfView.toFixed(0)}°`);
    console.log(`  Audio compression: ${(modifiers.audioCompression * 100).toFixed(0)}%\n`);

    // Artillery near-miss
    console.log('[Game] Artillery near-miss!');
    timeEngine.artilleryNearMiss();

    // Update for 2 seconds
    for (let i = 0; i < 120; i++) {
        timeEngine.update(1/60);
    }

    modifiers = timeEngine.getPerceptionModifiers();
    console.log(`  Movement speed: ${(modifiers.movementSpeed * 100).toFixed(0)}%`);
    console.log(`  FOV: ${modifiers.fieldOfView.toFixed(0)}°`);
    console.log(`  Audio compression: ${(modifiers.audioCompression * 100).toFixed(0)}%\n`);

    // Enter bombardment
    console.log('[Game] Entering bombardment!');
    timeEngine.enterBombardment();

    // Update for 2 seconds
    for (let i = 0; i < 120; i++) {
        timeEngine.update(1/60);
    }

    modifiers = timeEngine.getPerceptionModifiers();
    console.log(`  Movement speed: ${(modifiers.movementSpeed * 100).toFixed(0)}%`);
    console.log(`  FOV: ${modifiers.fieldOfView.toFixed(0)}°`);
    console.log(`  Audio compression: ${(modifiers.audioCompression * 100).toFixed(0)}%`);
    console.log(`  Heartbeat volume: ${(modifiers.heartbeatVolume * 100).toFixed(0)}%\n`);

    // Exit bombardment
    console.log('[Game] Bombardment ended');
    timeEngine.exitBombardment();

    // Update for 5 seconds (stress decays)
    for (let i = 0; i < 300; i++) {
        timeEngine.update(1/60);
    }

    modifiers = timeEngine.getPerceptionModifiers();
    console.log(`  Movement speed: ${(modifiers.movementSpeed * 100).toFixed(0)}%`);
    console.log(`  FOV: ${modifiers.fieldOfView.toFixed(0)}°`);
    console.log(`  Audio compression: ${(modifiers.audioCompression * 100).toFixed(0)}%\n`);
}

// ============================================================================
// EXAMPLE 4: REST PERIOD NARRATIVE
// ============================================================================

function exampleRestPeriod() {
    console.log('\n=== EXAMPLE 4: REST PERIOD NARRATIVE ===\n');

    const timeEngine = getVerdunTimeEngine();
    timeEngine.start();

    // Player is being relieved
    console.log('[Game] Relief column arrived! Beginning relief sequence...\n');
    timeEngine.setReliefColumnArrived(true);
    timeEngine.initiateRelief();

    console.log('[Game] Marching to rest area...\n');
    // (Simulate march duration)

    // Begin rest period
    console.log('[Game] Arrived at rest area. Beginning 7-day rest period.\n');
    const restSchedule = timeEngine.beginRestPeriod(7);

    console.log(`[Game] Rest schedule generated: ${restSchedule.beats.length} narrative beats\n`);

    // Play through narrative beats
    for (let i = 0; i < restSchedule.beats.length; i++) {
        const beat = timeEngine.startNextRestBeat();

        if (!beat) {
            console.log('[Game] Rest period complete\n');
            break;
        }

        console.log(`[Game] Narrative Beat: ${beat.name}`);
        console.log(`  Description: ${beat.description}`);
        console.log(`  Day: ${beat.triggerDay}`);
        console.log(`  Duration: ${(beat.duration / 60).toFixed(1)} minutes\n`);

        if (beat.choices.length > 0) {
            console.log(`  Player choices:`);
            beat.choices.forEach(choice => {
                console.log(`    - ${choice.text}`);
            });

            // Simulate player choosing first option
            const choiceId = beat.choices[0].id;
            console.log(`  [Player chose: ${beat.choices[0].text}]\n`);

            timeEngine.completeRestBeat(beat, choiceId);
        } else {
            timeEngine.completeRestBeat(beat);
        }
    }

    // Return to front line
    console.log('[Game] Orders received: Return to front line tonight\n');
    timeEngine.returnToFrontLine();

    console.log('[Game] Marching back to front line...\n');
    // (Simulate march)

    timeEngine.arriveAtFrontLine();
    console.log('[Game] Arrived at front line. Dawn breaks. Another rotation begins.\n');
}

// ============================================================================
// EXAMPLE 5: ENVIRONMENTAL TIME SIGNALS
// ============================================================================

function exampleEnvironmentalSignals() {
    console.log('\n=== EXAMPLE 5: ENVIRONMENTAL TIME SIGNALS ===\n');

    const timeEngine = getVerdunTimeEngine();
    timeEngine.start();

    // Get NPC dialogue for different times of day
    console.log('[Game] NPC Dialogue Examples:\n');

    // Simulate one full day cycle
    const phases = ['DAWN', 'DAYLIGHT', 'DUSK', 'NIGHT'];
    const deltaTime = 1/60;

    for (let phaseIndex = 0; phaseIndex < phases.length; phaseIndex++) {
        // Run time until phase changes
        let currentPhase = timeEngine.getCurrentPhase();
        const targetPhase = phases[phaseIndex];

        // Fast-forward to target phase
        while (currentPhase !== targetPhase) {
            for (let i = 0; i < 600; i++) {  // 10 seconds at a time
                timeEngine.update(deltaTime);
            }
            currentPhase = timeEngine.getCurrentPhase();
        }

        // Get environmental state
        const lighting = timeEngine.getLightingState();
        const weather = timeEngine.getWeatherState();
        const audio = timeEngine.getAudioState();
        const npc = timeEngine.getNPCBehaviorState();

        console.log(`${currentPhase}:`);
        console.log(`  Time: ${timeEngine.getHistoricalDate().toLocaleTimeString()}`);
        console.log(`  Sun: ${lighting.sunAngle.toFixed(0)}° (intensity: ${(lighting.sunIntensity * 100).toFixed(0)}%)`);
        console.log(`  Visibility: ${lighting.visibilityRange.toFixed(0)}m`);
        console.log(`  Temperature: ${weather.temperature.toFixed(1)}°C`);
        console.log(`  Artillery: ${audio.artilleryTempo.toFixed(0)} shells/min`);
        console.log(`  Birds: ${audio.birdCallsActive ? 'Yes' : 'No'}`);
        console.log(`  NPC Activity: ${npc.activityType}`);
        console.log(`  NPC Says (greeting): "${timeEngine.getNPCDialogue('greeting')}"`);
        console.log(`  NPC Says (observation): "${timeEngine.getNPCDialogue('observation')}"`);
        console.log(`  NPC Says (time ref): "${timeEngine.getNPCDialogue('timeReference')}"\n`);
    }
}

// ============================================================================
// EXAMPLE 6: COMPLETE ROTATION CYCLE
// ============================================================================

function exampleCompleteRotation() {
    console.log('\n=== EXAMPLE 6: COMPLETE ROTATION CYCLE ===\n');
    console.log('This demonstrates a full rotation: Front Line → Relief → Rest → Return\n');

    const timeEngine = getVerdunTimeEngine();
    timeEngine.start();

    // PHASE 1: ARRIVAL AT FRONT LINE
    console.log('--- PHASE 1: ARRIVAL AT FRONT LINE ---');
    console.log(`Date: ${timeEngine.getHistoricalDate().toDateString()}`);
    console.log(`Phase: ${timeEngine.getCurrentPhase()}`);
    console.log(`Campaign: ${timeEngine.getCampaignPhase()}\n`);

    // Set initial meters
    timeEngine.updateSurvivalMeters({
        hunger: 75,
        thirst: 80,
        stamina: 90,
        warmth: 65,
        hygiene: 55,
        morale: 80,
        alertness: 85
    });

    // PHASE 2: SURVIVE 48 HOURS
    console.log('--- PHASE 2: SURVIVING AT FRONT LINE (48 hours) ---\n');

    for (let hour = 0; hour < 48; hour++) {
        // Degrade meters
        timeEngine.updateSurvivalMeters({
            hunger: Math.max(10, 75 - (hour * 1.5)),
            thirst: Math.max(10, 80 - (hour * 1.8)),
            stamina: Math.max(20, 90 - (hour * 1.2)),
            warmth: Math.max(20, 65 - (hour * 0.8)),
            hygiene: Math.max(10, 55 - (hour * 0.9)),
            morale: Math.max(10, 80 - (hour * 1.5)),
            alertness: Math.max(10, 85 - (hour * 1.7))
        });

        // Random stress events
        if (Math.random() < 0.1) {  // 10% chance per hour
            console.log(`Hour ${hour}: Artillery near-miss!`);
            timeEngine.artilleryNearMiss();
        }

        // Simulate hour
        for (let i = 0; i < 3600 * 60; i++) {  // 60 FPS * 3600 seconds
            timeEngine.update(1/60);
        }

        if (hour % 12 === 0) {
            const state = timeEngine.getTimeState();
            console.log(`Hour ${hour}: Phase=${state.shiftPhase}, Alertness=${Math.max(10, 85 - (hour * 1.7)).toFixed(0)}%, Morale=${Math.max(10, 80 - (hour * 1.5)).toFixed(0)}%`);
        }
    }

    // PHASE 3: RELIEF ARRIVES
    console.log('\n--- PHASE 3: RELIEF ARRIVES ---');
    timeEngine.setReliefColumnArrived(true);
    timeEngine.setReplacementAvailable(true);

    console.log('Relief column arrived. Player eligible for relief.');
    console.log('The whistle blows. Time to go.\n');

    timeEngine.initiateRelief();

    // PHASE 4: REST PERIOD
    console.log('--- PHASE 4: REST PERIOD (7 days) ---\n');
    const restSchedule = timeEngine.beginRestPeriod(7);

    // Play beats
    let beat = timeEngine.startNextRestBeat();
    while (beat) {
        console.log(`Beat: ${beat.name} (Day ${beat.triggerDay})`);
        timeEngine.completeRestBeat(beat);
        beat = timeEngine.startNextRestBeat();
    }

    // PHASE 5: RETURN TO LINE
    console.log('\n--- PHASE 5: RETURN TO FRONT LINE ---');
    timeEngine.returnToFrontLine();
    timeEngine.arriveAtFrontLine();

    console.log(`Date: ${timeEngine.getHistoricalDate().toDateString()}`);
    console.log(`Days Survived: ${timeEngine.getDaysSurvived()}`);
    console.log('The cycle begins again.\n');
}

// ============================================================================
// RUN EXAMPLES
// ============================================================================

function runAllExamples() {
    console.log('\n');
    console.log('╔═══════════════════════════════════════════════════════════╗');
    console.log('║         VERDUN TIME ENGINE - EXAMPLE USAGE DEMO           ║');
    console.log('╚═══════════════════════════════════════════════════════════╝');

    // Run each example
    exampleBasicGameLoop();
    exampleSurvivalDegradation();
    exampleStressModulation();
    exampleRestPeriod();
    exampleEnvironmentalSignals();
    exampleCompleteRotation();

    console.log('\n');
    console.log('╔═══════════════════════════════════════════════════════════╗');
    console.log('║                    EXAMPLES COMPLETE                      ║');
    console.log('╚═══════════════════════════════════════════════════════════╝');
    console.log('\n');
}

// ============================================================================
// EXPORT
// ============================================================================

export {
    exampleBasicGameLoop,
    exampleSurvivalDegradation,
    exampleStressModulation,
    exampleRestPeriod,
    exampleEnvironmentalSignals,
    exampleCompleteRotation,
    runAllExamples
};

// Run if executed directly
if (require.main === module) {
    runAllExamples();
}
