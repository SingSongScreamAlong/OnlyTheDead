# THE VERDUN TIME ENGINE

**Version**: 1.0
**Created**: 2025-11-11
**Status**: Complete System Architecture & Implementation

---

## OVERVIEW

The Verdun Time Engine is a comprehensive system for creating **experiential time** in a WWI survival game. Time is not measured—it is **felt**. The engine manages dual timelines (historical calendar vs. lived experience), condition-based rotations, subjective time perception, and environmental time signaling.

### Core Philosophy

> **"Time isn't a number. Time is survival. Time is the space between heartbeats when a shell lands."**

The engine implements a critical design insight:

**The historical 303-day timeline is a fixed rail that always runs the same.**
**The player's experience is a variable camera moving along that rail.**

Every playthrough, the war happens identically. Day 14 always has the same events. But whether the player experiences those events—and how they experience them—changes based on their condition, position, and state.

This creates:
- **Deterministic chaos**: The war is predictable, the player's path is not
- **Replayability**: Different rotations show different facets of the same war
- **Historical authenticity**: The war doesn't care about you

---

## ARCHITECTURE

### System Components

```
VerdunTimeEngine (Master Orchestrator)
├── TimeStateManager (Dual Timeline)
│   ├── Historical Calendar (Feb 21 - Dec 18, 1916)
│   ├── Lived Experience Time (shifts, phases, rotations)
│   └── Campaign Phase Tracking (7 historical phases)
│
├── ReliefConditionEvaluator (Condition-Based Rotations)
│   ├── Physical State Thresholds (fatigue, morale, hunger, etc.)
│   ├── Injury State Evaluation (wounds, shell shock, gas)
│   ├── Unit State Tracking (casualties, replacements)
│   └── Time Served Tracking (minimum/maximum durations)
│
├── PerceptionModulationEngine (Subjective Time Experience)
│   ├── Stress Source Management (artillery, combat, trauma)
│   ├── Perception Modifiers (animation, audio, visuals, controls)
│   └── Stress Level Calculation (none → extreme)
│
├── RestPeriodNarrativeSystem (Narrative Beats)
│   ├── Beat Scheduling (First Night, Letter, Mirror, Training, Order)
│   ├── Time Skips (between beats)
│   ├── Player Choices (moral decisions during rest)
│   └── Meter Restoration (incomplete recovery)
│
└── EnvironmentalTimeSignalSystem (NO UI TIME)
    ├── Lighting (sun angle, intensity, fog, visibility)
    ├── Weather (temperature, wind, precipitation, mud)
    ├── Audio (artillery tempo, birds, human activity)
    └── NPC Behavior (alertness, activity, dialogue)
```

---

## KEY FEATURES

### 1. Dual Timeline System

**Historical Timeline** (The War):
- Fixed 303-day calendar (Feb 21 - Dec 18, 1916)
- 7 campaign phases matching historical events
- Events scheduled on exact dates
- Advances independent of player experience

**Lived Timeline** (The Soldier):
- Measured in shifts, not hours
- One "day" = 45-90 minutes real gameplay
- Four phases: Dawn → Daylight → Dusk → Night
- Time perception varies by stress and danger

### 2. Condition-Based Rotations

**Rotations are EARNED, not scheduled.**

Relief is triggered when:
- **Physical state degraded**: Alertness <20% for 2+ hours, Morale <30% for 4+ hours
- **Injured**: Wounded (medium+), bleeding, concussed, gassed, shell shock
- **Unit state**: Casualties >40%, unit combat-ineffective
- **Time served**: 24+ hours (minimum), 72+ hours (mandatory)

**NOT triggered by**: Calendar schedule, "it's been 5 days", arbitrary timers

### 3. Subjective Time Perception

Time doesn't slow down—**you do**.

Stress modulates:
- **Animation**: Movement 70-100% speed, fumbling interactions
- **Audio**: Muffled frequencies, heartbeat volume, tinnitus, underwater effect
- **Visuals**: Desaturation, tunnel vision (FOV 50-90°), vignette, shake
- **Controls**: Input delay, reduced sensitivity, interaction failure (shell shock)

Stress sources:
- Artillery near-miss (intensity 0.8, 5s duration)
- Friendly death (intensity 0.9, 15s duration)
- Bombardment (intensity 0.7, continuous)
- No-man's land (intensity 0.7, continuous)
- Shell shock event (intensity 1.0, 60s duration)

### 4. Rest Period Narrative

**Rest is not gameplay—it's recovery narrative.**

7-14 calendar days compress into 20-40 minutes of gameplay through **narrative beats**:

1. **The First Night**: Nightmares, trauma doesn't stop (Day 1)
2. **The Letter**: Mail from home, player choice (Day 2)
3. **The Mirror**: See your physical change (Day 4)
4. **The Training**: Dread of return (Day 6)
5. **The Order**: Resignation, you're going back (Day 7)

Between beats: Time skips forward with text ("7 days pass. You sleep, eat, try to forget.")

Meters restore **incompletely**:
- Hunger: 80% (hot meals, but not luxury)
- Morale: 50-70% (rest helps, but dread remains)
- Alertness: 60-70% (nightmares prevent full rest)
- Hygiene: 60-70% (lice always return)

### 5. Environmental Time Signaling

**NO UI ELEMENTS FOR TIME.**

Time communicated through:

**Light Quality**:
- Sun angle 0-360° (dynamic)
- Color temperature shifts (dawn orange → noon blue → dusk red)
- Shadow length/sharpness
- Fog density (thick at dawn/night, clear midday)
- Visibility 5m (night) → 300m (day)

**Weather**:
- Temperature varies by phase (coldest at dawn/night)
- Wind increases in afternoon
- Seasonal changes (Winter: frozen, Spring: MUD, Summer: heat, Autumn: rain)
- Mud consistency (affects movement)

**Audio**:
- Artillery tempo follows historical patterns:
  - Dawn/Dusk: 20-50 shells/min ("morning/evening hate")
  - Day: 5-30 shells/min (variable)
  - Night: 5-15 shells/min (harassment)
- Bird calls only at dawn/dusk
- Human activity: whispers (night/stand-to) → normal speech (day)

**NPC Behavior**:
- Alertness: Stand-to (dawn/dusk) → Routine (day) → Alert (night)
- Activities: Watch → Repair → Eating → Sentry → Resting
- Dialogue changes by phase:
  - Dawn: "Stand-to! Everyone up!"
  - Day: "How long have we been at this?"
  - Dusk: "Going to be a long night..."
  - Night: "Did you hear that?"

---

## USAGE

### Basic Integration

```typescript
import { VerdunTimeEngine, getVerdunTimeEngine } from './VerdunTimeEngine';

// Get singleton instance
const timeEngine = getVerdunTimeEngine();

// Start the engine
timeEngine.start();

// Register event handlers
timeEngine.onPhaseChange((newPhase) => {
    console.log(`Time phase changed: ${newPhase}`);
    // Update game lighting, audio, NPCs
});

timeEngine.onReliefEligible((conditions) => {
    console.log(`Relief eligible: ${conditions.reason}`);
    // Show "relief column approaching" notification
});

// Main game loop
function gameUpdate(deltaTime: number) {
    // 1. Update time engine
    timeEngine.update(deltaTime);

    // 2. Update game systems with environmental state
    const lighting = timeEngine.getLightingState();
    const audio = timeEngine.getAudioState();
    const perception = timeEngine.getPerceptionModifiers();

    renderSystem.applyLighting(lighting);
    audioSystem.setArtilleryTempo(audio.artilleryTempo);
    playerController.setMovementSpeed(perception.movementSpeed);
    cameraController.setFOV(perception.fieldOfView);

    // 3. Update time engine with player state
    timeEngine.updateSurvivalMeters({
        hunger: player.hunger,
        thirst: player.thirst,
        // ... etc
    });
}
```

### Stress Events

```typescript
// Artillery near-miss
timeEngine.artilleryNearMiss();

// Witness friendly death
timeEngine.friendlyDeath();

// Enter bombardment (continuous stress)
timeEngine.enterBombardment();
// ... (player in shelter)
timeEngine.exitBombardment();  // Must manually exit

// Shell shock event
timeEngine.shellShockEvent();
```

### Relief & Rotation

```typescript
// Signal relief column arrived
timeEngine.setReliefColumnArrived(true);

// Initiate relief (player leaving front line)
timeEngine.initiateRelief();

// Begin rest period
const restSchedule = timeEngine.beginRestPeriod(7);  // 7 days

// Play narrative beats
let beat = timeEngine.startNextRestBeat();
while (beat) {
    // Show narrative scene to player
    showNarrativeScene(beat);

    // Player makes choice (if applicable)
    if (beat.choices.length > 0) {
        const choiceId = await getPlayerChoice(beat.choices);
        timeEngine.completeRestBeat(beat, choiceId);
    } else {
        timeEngine.completeRestBeat(beat);
    }

    beat = timeEngine.startNextRestBeat();
}

// Return to front line
timeEngine.returnToFrontLine();
timeEngine.arriveAtFrontLine();
```

### Environmental Queries

```typescript
// Get current time indicators (for player awareness, not UI)
const phase = timeEngine.getCurrentPhase();  // DAWN, DAYLIGHT, DUSK, NIGHT
const date = timeEngine.getHistoricalDate();  // JavaScript Date object
const campaignPhase = timeEngine.getCampaignPhase();  // PHASE_I_CHAOS, etc.

// Get NPC dialogue appropriate for current time
const greeting = timeEngine.getNPCDialogue('greeting');
const observation = timeEngine.getNPCDialogue('observation');
const timeRef = timeEngine.getNPCDialogue('timeReference');

console.log(greeting);  // "Morning..." (if dawn)
console.log(observation);  // "Watch the wire." (if stand-to)
console.log(timeRef);  // "Sun's coming up." (if dawn)
```

---

## DESIGN DECISIONS

### Why Condition-Based Rotations?

**Problem**: Scheduled rotations create "waiting for timer" gameplay.

**Solution**: Relief is earned through survival. Player must reach critical state (physical/mental breakdown) OR serve minimum time with replacement available.

**Result**:
- Rotations feel like **earned survival**, not progression grind
- Victory = "I survived long enough to be relieved"
- Every hour closer to relief increases tension

### Why Perception Modulation Instead of Time Dilation?

**Problem**: Slowing actual game time breaks historical timeline.

**Solution**: Historical clock ticks at constant rate. Player *perception* of time changes through animation, audio, visuals, controls.

**Result**:
- Historical events always happen on schedule
- Player *feels* time stretching during fear/stress
- Multiple playthroughs remain historically consistent

### Why Narrative Beats Instead of Playable Rest?

**Problem**: Playing through 7-14 days of rest is boring and breaks pacing.

**Solution**: Compress rest into 5-6 key narrative moments (20-40 min total) with time skips between.

**Result**:
- Rest feels fleeting and insufficient (authentic to soldier experience)
- Player sees physical/mental toll through narrative
- Maintains pacing while advancing calendar
- Rest period = recovery narrative, not gameplay loop

### Why NO UI Time Elements?

**Problem**: Clocks/timers make time intellectual, not experiential.

**Solution**: All time indicated through environment (light, weather, sound, NPC behavior).

**Result**:
- Player learns to read environment like actual soldiers
- "What time is it?" becomes "Is it almost dawn?" (light quality)
- Time becomes **felt**, not **read**
- No UI immersion break

---

## FILE STRUCTURE

```
systems/time_engine/
├── README.md                              (this file)
├── VerdunTimeEngine.ts                    (master orchestrator)
├── TimeStateManager.ts                    (dual timeline core)
├── ReliefConditionEvaluator.ts            (condition-based rotations)
├── PerceptionModulationEngine.ts          (subjective time)
├── RestPeriodNarrativeSystem.ts           (rest narrative beats)
├── EnvironmentalTimeSignalSystem.ts       (environmental time signals)
└── EXAMPLE_USAGE.ts                       (usage demonstrations)
```

---

## TECHNICAL SPECIFICATIONS

### Performance

- **Update frequency**: 60 FPS (call `update(deltaTime)` every frame)
- **Memory footprint**: ~50KB (all systems combined)
- **CPU cost**: <0.1ms per frame (negligible)

### Dependencies

- **None**: Pure TypeScript, engine-agnostic
- Adaptable to: Unreal Engine, Unity, custom engines
- No external libraries required

### Thread Safety

- All systems are single-threaded
- Safe for game loop integration
- Event callbacks are synchronous

---

## INTEGRATION WITH EXISTING SYSTEMS

### Survival Systems

The time engine **consumes** survival meter data but does **not manage** meters:

```typescript
// Game manages meters
player.hunger -= hungerDepletionRate * deltaTime;

// Game provides meters to time engine
timeEngine.updateSurvivalMeters({
    hunger: player.hunger,
    thirst: player.thirst,
    // ...
});

// Time engine evaluates relief conditions
const reliefConditions = timeEngine.getTimeState();
```

### Historical Timeline

The time engine provides historical date for event scheduling:

```typescript
// Game has event database keyed by date
const currentDate = timeEngine.getHistoricalDate();
const events = eventDatabase.getEventsForDate(currentDate);

// Execute events if player is present
events.forEach(event => {
    if (playerIsAtLocation(event.location)) {
        executeEvent(event);
    }
});
```

### Rendering Systems

The time engine provides environmental state for rendering:

```typescript
const lighting = timeEngine.getLightingState();

// Apply to rendering
scene.sun.angle = lighting.sunAngle;
scene.sun.intensity = lighting.sunIntensity;
scene.sun.color = lighting.sunColor;
scene.sky.color = lighting.skyColor;
scene.fog.density = lighting.fogDensity;
scene.fog.color = lighting.fogColor;
```

---

## EXAMPLES

See `EXAMPLE_USAGE.ts` for complete demonstrations:

1. **Basic Game Loop**: Integration with 60 FPS game loop
2. **Survival Degradation**: 24-hour survival with meter depletion
3. **Stress Modulation**: Artillery, bombardment, perception changes
4. **Rest Period**: Narrative beats and time skips
5. **Environmental Signals**: Light, weather, audio, NPC behavior
6. **Complete Rotation**: Full cycle from arrival → survival → relief → rest → return

Run examples:
```bash
ts-node EXAMPLE_USAGE.ts
```

---

## PHILOSOPHY RECAP

### The Emotional Thesis

> **"Surviving another rotation is the only progress that matters."**

Not leveling. Not loot. Not kills. Not missions.

**Just making it back up the trench when the relief whistle blows.**

### The Player Experience

```
┌─────────────────────────────────────────┐
│         FRONT LINE SHIFT                │
│  Terror → Endurance → Exhaustion        │
│  "Survive this hour"                    │
└─────────────────┬───────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────┐
│         RELIEF & MARCH OUT              │
│  Crushing Relief → Collapse             │
│  "I made it. I'm alive."                │
└─────────────────┬───────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────┐
│         REST PERIOD                     │
│  Exhausted Relief → Dread of Return     │
│  "This isn't long enough"               │
└─────────────────┬───────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────┐
│         RETURN MARCH                    │
│  Resignation → Dread                    │
│  "Here we go again"                     │
└─────────────────┬───────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────┐
│         FRONT LINE (AGAIN)              │
│  "Nothing has changed.                  │
│   Everything is worse."                 │
└─────────────────────────────────────────┘
```

### The Whistle

**The most important moment:**

When the relief whistle blows and the player feels:
- Crushing relief
- Collapse of adrenaline
- Trembling in their fingers
- Disbelief they survived

**You have recreated Verdun.**

---

## CREDITS

**Design Philosophy**: "Time is felt, not measured"
**System Architecture**: Dual timeline, condition-based rotations
**Historical Authenticity**: 303-day Verdun timeline (Feb 21 - Dec 18, 1916)

Built for: **Only The Dead** (WWI Survival Game)

---

## LICENSE

This system is part of the *Only The Dead* game development project.

---

## FINAL THOUGHT

> *"Time isn't a number. Time is survival. Time is the space between heartbeats when a shell lands. Time is the eternity of waiting for relief. Time is knowing you'll go back."*

**That is Verdun.**
**That is Only The Dead.**

---

**END OF DOCUMENTATION**
