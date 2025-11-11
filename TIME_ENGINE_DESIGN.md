# THE VERDUN TIME ENGINE
## Core System Architecture for Experiential Time

**Version**: 1.0
**Date**: 2025-11-11
**Status**: System Design & Implementation Specification

---

## PHILOSOPHICAL FOUNDATION

### The Core Problem

Most war games simulate time mechanically:
- Clock ticks at constant rate
- Events scheduled by timestamps
- Rotations happen on calendar intervals
- Players "wait" for time to pass

**This is wrong for Verdun.**

### The Core Solution

Verdun time must be **felt, not measured**.

Time is:
- **Compressed during danger** - seconds stretch into eternity
- **Expanded during fear** - hours vanish in terror
- **Dissolved during trauma** - days blur into each other
- **Earned through survival** - you don't "play through time," you survive until relief

**Victory = not dying today.**

Not "I survived 76 days," but "I survived the last 8 hours."

---

## DUAL TIMELINE ARCHITECTURE

### Timeline 1: Historical Calendar (The War)

```
Real Historical Time:
├─ 303 days (Feb 21 - Dec 18, 1916)
├─ 7 campaign phases
├─ Historical events with exact dates
├─ Strategic situation changes
└─ Weather/seasonal progression
```

**Properties:**
- Advances in **calendar days** regardless of player experience
- Drives strategic context (who holds what, where artillery is, which units are where)
- Visible to player only through:
  - Letters mentioning dates
  - News reports
  - Orders from command
  - Officer dialogue
  - Strategic maps in rest areas
- **Never shown as HUD clock**

### Timeline 2: Lived Experience (The Soldier)

```
Subjective Time:
├─ Measured in shifts, not hours
├─ One "day" = 45-90 minutes real gameplay
├─ Time perception varies by danger
├─ Advances through survival, not clock
└─ Indicated by environment, not UI
```

**Properties:**
- Advances through **player experience** and **physical/mental state**
- Drives gameplay pacing
- Affects how time "feels" during play
- No numerical representation
- Communicated through:
  - Light quality (dawn, daylight, dusk, night)
  - Sound (bird calls, silence, artillery tempo)
  - NPC behavior (exhaustion, speech patterns)
  - Weather changes
  - Player character physical state

---

## THE SHIFT SYSTEM

### Core Principle: You Live in Rotations, Not Hours

Players don't experience "time passing" - they experience **shifts**.

```
Front Line Shift Duration:
├─ Minimum: 1 calendar day (45-90 min gameplay)
├─ Maximum: 3 calendar days (135-270 min gameplay)
├─ Determined by: Player condition + Unit availability + Command orders
└─ Ends when: Player earns relief OR dies
```

### Shift Structure

Each shift consists of **time phases**, not clock hours:

#### Phase 1: DAWN (Stand-To)
- **Real duration**: 5-15 minutes
- **Subjective duration**: Feels like 30+ minutes (high tension)
- **Activities**: Alert posture, weapon ready, watch no-man's land
- **Time signals**:
  - Darkness gradually lifts (not instant)
  - Sounds emerge (birds, distant movement)
  - Officers move along trench
  - Whispered warnings
- **Danger**: Medium-high (attack hour)
- **Time perception**: Stretched (anticipation)

#### Phase 2: DAYLIGHT
- **Real duration**: 15-45 minutes
- **Subjective duration**: Variable (compressed during bombardment, expanded during work)
- **Activities**: Repair, resupply, sentry rotation, eating, survival
- **Time signals**:
  - Sun angle changes (dynamic lighting)
  - Temperature shifts
  - Mud consistency changes
  - Exhaustion accumulates
  - NPC dialogue ("Been at this for hours...")
- **Danger**: Variable (bombardments, raids)
- **Time perception**: Fluid

#### Phase 3: DUSK (Stand-To)
- **Real duration**: 5-15 minutes
- **Subjective duration**: Feels like 20-40 minutes (high tension)
- **Activities**: Alert posture, prepare for night
- **Time signals**:
  - Light quality changes (golden → grey → dark)
  - Shadows lengthen
  - Temperature drops
  - Officers order stand-to
  - Flare gun checks
- **Danger**: Medium-high (attack hour)
- **Time perception**: Stretched (dread)

#### Phase 4: NIGHT
- **Real duration**: 20-60 minutes
- **Subjective duration**: Feels longer (fear + darkness + isolation)
- **Activities**: Patrols, wire repair, resupply, fitful sleep, listening posts
- **Time signals**:
  - Darkness (limited visibility)
  - Sound becomes primary sense
  - Flare cycles
  - Star movement (if clear)
  - Distant artillery flashes
  - Body exhaustion increases dramatically
- **Danger**: High (patrols, raids, uncertainty)
- **Time perception**: Stretched and fragmented

### One Complete "Day" = All Four Phases

**Total real-time duration**: 45-90 minutes
**Total subjective duration**: Feels like 8-16 hours
**Total calendar time**: 1 calendar day passes

---

## TIME PERCEPTION MODULATION SYSTEM

### Core Mechanic: Time Doesn't Speed Up or Slow Down - It FEELS Different

**Implementation**: Not time dilation (actual slowdown), but **experiential time** through:

#### 1. Animation Pacing
```
Normal State:
├─ Walk speed: 1.0x
├─ Turn speed: 1.0x
├─ Interaction speed: 1.0x
└─ Look speed: 1.0x

High Stress State (bombardment, near-miss, pinned):
├─ Walk speed: 0.7-0.9x (sluggish, heavy)
├─ Turn speed: 0.8x (labored)
├─ Interaction speed: 0.6-0.8x (fumbling)
└─ Look speed: 0.9x (slight drag)
```

**Effect**: Player feels time stretching because their avatar moves through molasses.

#### 2. Sound Design
```
Normal State:
├─ All frequencies present
├─ Clear spatial audio
├─ Normal ambient volume
└─ Music: none or minimal

High Stress State:
├─ Muffled high frequencies (pressure effect)
├─ Compressed spatial audio (tunnel hearing)
├─ Increased heartbeat volume (drowns ambient)
├─ Tinnitus layer (ringing)
└─ Music: none (silence is louder)

Extreme Stress State (shell shock moment):
├─ All sound except heartbeat muted
├─ Reverb on all remaining sound
├─ Distant "underwater" quality
├─ Time perception: seconds feel like minutes
└─ Music: single sustained low note (dread)
```

#### 3. Visual Effects
```
Normal State:
├─ Full color saturation
├─ Normal field of view (90-100°)
├─ Clear vision
└─ Smooth camera movement

High Stress State:
├─ Desaturated color (grey-brown)
├─ Reduced FOV (70-80° - peripheral constriction)
├─ Slight blur at edges
└─ Camera sway increases

Extreme Stress State:
├─ Near-monochrome
├─ Severe tunnel vision (40-60° FOV)
├─ Heavy vignette
├─ Camera shake/tremor
└─ Micro-freezes (dissociation effect)
```

#### 4. Control Responsiveness
```
Normal State:
├─ Input response: Instant
├─ Aim sensitivity: 1.0x
├─ Reload speed: 1.0x
└─ Interaction: Instant

High Stress State:
├─ Input response: 50-100ms delay
├─ Aim sensitivity: 0.7-0.9x (heavy, resistant)
├─ Reload speed: 0.8x (fumbling)
└─ Interaction: 200-500ms delay (shaking hands)

Extreme Stress State:
├─ Input response: 100-300ms delay
├─ Aim sensitivity: 0.4-0.6x (trembling)
├─ Reload speed: 0.5-0.7x (panic)
└─ Interaction: Can fail entirely (shell shock paralysis)
```

### Stress State Triggers

#### Time Slows (Subjective) When:
- Artillery lands within 10m (3-10 seconds stretched)
- Near-miss from rifle/MG fire (1-2 seconds stretched)
- Witnessing friendly death (5-15 seconds stretched)
- Pinned in no-man's land (continuous stretch)
- Bleeding out (continuous, increasing)
- Lost/disoriented in communication trench (continuous)
- Trapped in bombardment shelter (continuous)
- Gas attack (continuous until safe)

#### Time Compresses (Subjective) When:
- Combat adrenaline (firefight blur - minutes vanish)
- Shell shock dissociation (hours vanish, suddenly night)
- Physical collapse/unconsciousness (time skip)
- Routine work (time passes faster when in rhythm)

---

## CONDITION-BASED ROTATION SYSTEM

### Core Principle: Rotations Are EARNED, Not Scheduled

You don't "wait for rotation" - you **survive until relief**.

### The Relief Trigger System

Player is eligible for relief when **ANY** of these conditions met:

#### Condition Set A: Physical State (Survival Meters)
```
Relief Required If:
├─ Fatigue (Alertness) < 20% for 2+ hours
├─ Morale < 30% for 4+ hours
├─ Hunger < 20% (starvation risk)
├─ Thirst < 15% (dehydration critical)
├─ Warmth < 25% (hypothermia risk)
├─ Hygiene < 20% (disease risk critical)
└─ ANY meter at absolute zero (collapse)
```

#### Condition Set B: Injury State
```
Relief Required If:
├─ Wounded (any wound severity medium+)
├─ Bleeding (cannot stop)
├─ Concussed (shell blast)
├─ Gassed (chlorine/phosgene exposure)
├─ Shell shock symptoms manifesting
└─ Physically unable to perform duty
```

#### Condition Set C: Unit State
```
Relief Possible If:
├─ Unit casualties > 40% (unit combat ineffective)
├─ Scheduled relief column arrives (Noria rotation)
├─ Position captured/abandoned (forced withdrawal)
├─ Strategic redeployment ordered
└─ End of campaign phase
```

#### Condition Set D: Time Served
```
Relief Possible If:
├─ 24+ calendar hours on front line (1+ game days)
│   AND replacement unit available
│   AND player condition > minimal threshold
│
├─ 72+ calendar hours on front line (3 game days)
│   Mandatory relief regardless of condition
│   (Historical: Men collapse beyond this point)
└─ Exception: Major battle (relief impossible)
```

### The Relief Experience

When relief is triggered:

```
1. NOTIFICATION (Organic, Not UI)
   ├─ Officer shouts down trench: "51st, prepare for relief!"
   ├─ Runners spread word
   ├─ NPCs react: Visible relief, some cry, some laugh
   └─ Player hears sounds: Approaching column, equipment

2. TRANSITION (10-20 minutes real-time)
   ├─ Relief unit arrives (see them approach)
   ├─ Position handover (briefing, pointing, warnings)
   ├─ Gather equipment
   ├─ Form up in communication trench
   ├─ March begins (automatic, player walks with unit)
   └─ Environmental changes: Sounds fade, light changes

3. PSYCHOLOGICAL MOMENT
   ├─ Camera stays on trench as you leave
   ├─ Sound: Artillery continues behind you (you're leaving it)
   ├─ NPCs: Stumbling, thousand-yard stare, silence
   ├─ Player character: Visible exhaustion, trembling hands
   └─ No UI, no fanfare, just survival

4. ARRIVAL AT REST AREA
   ├─ Time skip forward (see below)
   └─ Narrative beat (see below)
```

### If Relief Doesn't Come

If player survives beyond expected relief time:

```
Hour 24-48: Growing Desperation
├─ NPC dialogue: "Where's the relief?" "They forgot us?"
├─ Officer: "Hold position. Orders unchanged."
├─ Morale drain accelerates
└─ Fatigue accumulates dangerously

Hour 48-72: Breaking Point
├─ NPCs: Some break down, some go silent
├─ Player condition critical
├─ Survival becomes hour-to-hour
└─ Historical: This is when units ceased to function

Hour 72+: Historical Collapse
├─ If player somehow still alive: Forced relief
├─ Unit pulled out regardless of situation
└─ OR: Unit overrun, player likely dead
```

---

## REST PERIOD & TIME SKIP SYSTEM

### Core Principle: Don't Play Rest, EXPERIENCE Rest

Rest is not gameplay - rest is **recovery narrative**.

### Time Skip Mechanism

When player reaches rest area:

```
IMMEDIATE EFFECTS (First 5 minutes gameplay):
├─ Arrival: Stumble off march, collapse on ground
├─ Delousing station (mandatory, walk-through)
├─ Hot food queue (walk-through)
├─ Bunk assignment (choose bed, collapse)
└─ Sleep (player presses button to sleep)

TIME SKIP INITIATED:
├─ Screen fades to black (slow, 3-5 seconds)
├─ Text fades in: "7 days behind the lines"
├─ Calendar advances: "March 14-21, 1916"
└─ Gameplay resumes at key narrative moments
```

### Narrative Beat System During Rest

Player doesn't "play" rest days - they experience **narrative beats**:

#### Beat 1: The First Night (Immediately after arrival)
- **Duration**: 5-10 minutes gameplay
- **Activities**: Sleep → wake screaming → walk around camp → can't sleep → talk to NPCs → smoke → eventually collapse again
- **Purpose**: Show trauma doesn't stop at the line
- **Time skipped**: 1 night

#### Beat 2: The Letter (Day 2-3)
- **Duration**: 3-5 minutes gameplay
- **Activities**: Mail call → receive letter → read letter (UI overlay) → player choice: Write back? Burn it? Keep it?
- **Purpose**: Connection to home, morale boost/drain
- **Time skipped**: 1-2 days

#### Beat 3: The Mirror (Day 4-5)
- **Duration**: 2-3 minutes gameplay
- **Activities**: Walk past mirror → automatic stop → camera shows player's face → visible change (sunken eyes, weight loss, beard, mud in wrinkles)
- **Purpose**: Show physical toll
- **Time skipped**: 1-2 days

#### Beat 4: The Training (Day 5-6)
- **Duration**: 5-10 minutes gameplay
- **Activities**: Officer calls formation → practice attack drill → player goes through motions → NPCs discuss rumors of next deployment
- **Purpose**: Dread of return
- **Time skipped**: 1 day

#### Beat 5: The Order (Day 7)
- **Duration**: 3-5 minutes gameplay
- **Activities**: Assembly → officer reads orders → return to front line → NPCs react → march begins
- **Purpose**: Resignation, acceptance
- **Time skipped**: 0 (transition to next rotation)

**Total Rest Period**:
- **Calendar time**: 7-14 days
- **Gameplay time**: 20-40 minutes
- **Subjective experience**: Fleeting, never enough

### Survival Meter Changes During Rest

```
During Time Skip (Automatic):
├─ Hunger: Restored to 80% (hot meals, but not luxury)
├─ Thirst: Restored to 90% (clean water available)
├─ Stamina: Restored to 100%
├─ Warmth: Restored to 70-90% (depends on season/shelter)
├─ Hygiene: Restored to 60-70% (delousing, wash, but lice return)
├─ Morale: Restored to 50-70% (rest helps, but dread of return)
├─ Alertness: Restored to 60-70% (better sleep, but nightmares)

During Narrative Beats (Player Actions):
├─ Can eat/drink for additional restoration
├─ Can write letters (morale boost)
├─ Can talk to NPCs (morale/information)
├─ Can clean equipment (hygiene bonus)
└─ Cannot fully recover (Verdun doesn't allow it)
```

---

## ENVIRONMENTAL TIME SIGNALING (NO CLOCKS)

### Core Principle: Time Is Felt, Not Read

**NO UI ELEMENTS FOR TIME:**
- ❌ No clock
- ❌ No "Day 47" counter
- ❌ No "6 hours until relief"
- ❌ No time-of-day widget

**ONLY ENVIRONMENTAL SIGNALS:**

### Signal Category 1: Light Quality

#### Dawn
```
Visual:
├─ Darkness: Pitch black → deep blue → grey → pale orange
├─ Shadows: Invisible → long & sharp → defined
├─ Visibility: 2m → 10m → 50m → 200m (gradual)
├─ Sky: Stars fade → horizon glow → sun disk appears
└─ Duration: 15-20 minutes real-time (stretched, tense)

Audio:
├─ Silence → distant bird calls → nearby birds
├─ Artillery: Sporadic → increases (morning hate)
└─ Human: Whispers → quiet movement → normal speech
```

#### Daylight
```
Visual:
├─ Sun angle changes (high quality dynamic lighting)
├─ Shadow length/direction shifts
├─ Color temperature: Cool morning → warm midday → cool afternoon
├─ Sky: Clear blue OR overcast grey OR storm clouds
└─ Mud: Dries (cracks) or saturates (slick) based on time + weather

Audio:
├─ Artillery: Variable tempo (quiet → bombardment → quiet)
├─ Activity: Work sounds, voices, equipment
└─ Wind: Changes direction/intensity
```

#### Dusk
```
Visual:
├─ Light: Warm golden → orange → red → purple → deep blue → black
├─ Shadows: Soften → lengthen → merge → vanish
├─ Visibility: 200m → 50m → 10m → 2m (gradual)
├─ Flares: Become more visible against darkening sky
└─ Duration: 15-20 minutes real-time (stretched, tense)

Audio:
├─ Birds: Active → quiet → silent
├─ Artillery: Often increases (evening hate)
└─ Human: Normal speech → whispers → silence
```

#### Night
```
Visual:
├─ Darkness: Near-total (moon phase affects)
├─ Illumination: Only flares, muzzle flash, artillery flash
├─ Stars: Visible if clear (position indicates rough time)
├─ Horizon: Artillery flashes silhouette terrain
└─ Player vision: 2-5m without flare, 20-50m under flare

Audio:
├─ Primary sense (vision unreliable)
├─ Sound localization critical
├─ Footsteps, equipment, breathing amplified
└─ Artillery: Continuous low-level harassment
```

### Signal Category 2: Weather & Atmosphere

```
Morning:
├─ Fog: Common (limited visibility)
├─ Dew: Visible on equipment, cold
├─ Temperature: Coldest point
└─ Ground: Frost (winter) or damp (spring/fall)

Midday:
├─ Sun: High (if visible)
├─ Temperature: Warmest point
├─ Mud: Firmest (if dry) or hottest (summer)
└─ Shadows: Short, directly below objects

Afternoon:
├─ Wind: Often increases
├─ Temperature: Cooling
├─ Light: Angle changes noticeably
└─ Clouds: Tend to build (historically accurate)

Evening:
├─ Temperature: Drops quickly
├─ Wind: Often calm
├─ Mist: Begins to form (low areas)
└─ Ground: Cools, becomes firm

Night:
├─ Temperature: Coldest (winter) or cool (summer)
├─ Frost: Forms (winter)
├─ Fog: Thick (spring/fall)
└─ Mud: Freezes (winter) or remains slick
```

### Signal Category 3: NPC Behavior & Dialogue

```
Dawn:
├─ "Stand-to! Everyone up!"
├─ "Watch the wire!"
├─ "Quiet... listen..."
├─ Nervous tension, weapons ready

Morning:
├─ "Stand down. Get some coffee."
├─ "Check your rifle, it's filthy."
├─ "Breakfast in ten minutes."
├─ Relaxation (relative), work begins

Midday:
├─ "Haul those sandbags!"
├─ "My back is killing me..."
├─ "How long have we been at this?"
├─ Fatigue shows, routine work

Afternoon:
├─ "When's relief supposed to come?"
├─ "Sun's getting low..."
├─ "Better eat now, won't get another chance."
├─ Anticipation of dusk stand-to

Evening:
├─ "Stand-to! Man your positions!"
├─ "Check the flare pistol."
├─ "Going to be a long night..."
├─ Tension returns, dread

Night:
├─ "Patrol's going out in five."
├─ "Stay awake! Sentry duty!"
├─ "Did you hear that?"
├─ "I can't see a damn thing..."
├─ Whispers, fear, isolation
```

### Signal Category 4: Player Character Physical State

```
Time Accumulation (No Numbers, Only Feel):

Hour 0-4:
├─ Movement: Normal
├─ Hands: Steady
├─ Vision: Clear
├─ Breathing: Normal
└─ Stamina: Full

Hour 4-8:
├─ Movement: Slightly heavier
├─ Hands: Occasional tremor (reloading slightly slower)
├─ Vision: Slight blur at edges when tired
├─ Breathing: Heavier after exertion
└─ Stamina: Regen slower

Hour 8-12:
├─ Movement: Noticeably sluggish
├─ Hands: Constant light tremor (aim shake)
├─ Vision: Frequent blinks, dry eyes
├─ Breathing: Labored
└─ Stamina: Depletes fast, regens slow

Hour 12-16:
├─ Movement: Heavy, stumbling
├─ Hands: Severe tremor (can barely reload)
├─ Vision: Tunnel vision starting, micro-sleeps
├─ Breathing: Gasping
└─ Stamina: Near-constant low

Hour 16-20:
├─ Movement: Barely functional
├─ Hands: Uncontrollable shake
├─ Vision: Severe tunnel vision, hallucinations possible
├─ Breathing: Shallow, desperate
└─ Stamina: Cannot sprint

Hour 20+:
├─ Movement: Collapse risk
├─ Hands: Can drop items
├─ Vision: Blackouts, hallucinations
├─ Breathing: Hyperventilation or hypoventilation
└─ Stamina: Cannot maintain alertness
```

### Signal Category 5: Artillery Tempo

```
Historical Artillery Pattern (Verdun):

Early Morning (04:00-06:00):
├─ German harassment: 5-10 shells/hour
├─ Sporadic, unpredictable
└─ Prevents rest, maintains pressure

Morning (06:00-08:00):
├─ "Morning hate": 20-50 shells/hour
├─ Coordinated bombardment
└─ Standard German practice

Daytime (08:00-17:00):
├─ Variable: 5-30 shells/hour
├─ Spikes during attacks or counter-battery
└─ Periods of quiet (minutes to hours)

Evening (17:00-20:00):
├─ "Evening hate": 20-50 shells/hour
├─ Coordinated bombardment
└─ Standard German practice

Night (20:00-04:00):
├─ Harassment: 5-15 shells/hour
├─ Irregular, targeted at communication trenches
└─ Denies sleep, disrupts resupply
```

**Player learns this pattern organically:**
- "It's almost evening... bombardment's coming."
- "Quiet now... probably morning, they'll start shelling soon."
- "Night... at least the big guns rest."

---

## THE 303-DAY CAMPAIGN FRAMEWORK

### Integration of Dual Timelines

```
PLAYER EXPERIENCE (Lived Time):
├─ Front Line Shift (1-3 days, 45-270 min gameplay)
├─ Relief & March (1 day, 20-40 min gameplay)
├─ Rest Period (7-14 days, 20-40 min narrative beats)
└─ Return March (1 day, 20-40 min gameplay)

HISTORICAL CALENDAR (War Time):
├─ 15-28 calendar days elapsed per rotation
├─ Historical events occur on exact dates
├─ Strategic situation evolves
└─ Seasons change
```

### Campaign Rotation Structure

**Total Campaign**: 10-20 rotations over 303 days

**Rotation Duration Variance**:

```
Early Campaign (Feb-Apr):
├─ Rotations: Shorter (15-21 days)
├─ Front line: 4-5 days (chaos, disorganization)
├─ Rest: 7-10 days (plentiful reserves)
└─ Gameplay: Steeper learning curve

Mid Campaign (May-Sep):
├─ Rotations: Standard (21-28 days)
├─ Front line: 5-7 days (organized but brutal)
├─ Rest: 10-14 days (system functioning)
└─ Gameplay: Grinding attrition

Late Campaign (Oct-Dec):
├─ Rotations: Variable (7-28 days)
├─ Front line: 2-7 days (counteroffensive = shorter, intense)
├─ Rest: 5-14 days (depends on strategic phase)
└─ Gameplay: High intensity, hope emerges
```

### Campaign Phase Integration

**Each phase has different time feeling:**

#### Phase I (Feb 21 - Mar 6): CHAOS
- **Historical**: Initial German assault
- **Rotations**: 1-2 (short, emergency)
- **Time feeling**: Compressed, blur of terror
- **Front line duration**: 2-4 days (emergency deployment)
- **Rest**: Minimal or none

#### Phase II (Mar 6 - Apr 9): LEFT BANK
- **Historical**: Mort-Homme, Hill 304 offensive
- **Rotations**: 2-3 (system establishing)
- **Time feeling**: Grinding, repetitive
- **Front line duration**: 4-6 days (standard rotation)
- **Rest**: 7-10 days

#### Phase III (Apr 9 - May 7): ATTRITION
- **Historical**: Renewed fort attacks
- **Rotations**: 2-3 (standard)
- **Time feeling**: Numb, autopilot
- **Front line duration**: 5-7 days (standard)
- **Rest**: 10-14 days

#### Phase IV (May 7 - Jun 23): VAUX
- **Historical**: Fort Vaux siege (CLIMAX 1)
- **Rotations**: 2-4 (includes 7-day Vaux siege)
- **Time feeling**: Stretched to breaking
- **Fort Vaux mission**: 7 days continuous (no relief)
- **Rest**: 10-14 days (recovery after Vaux)

#### Phase V (Jun 23 - Oct 24): STALEMATE
- **Historical**: Lower intensity, Somme draws Germans away
- **Rotations**: 3-5 (longest phase)
- **Time feeling**: Endless, unchanging
- **Front line duration**: 5-7 days (standard)
- **Rest**: 10-14 days

#### Phase VI (Oct 24 - Nov 2): DOUAUMONT
- **Historical**: French counteroffensive (CLIMAX 2)
- **Rotations**: 1-2 (short, intense)
- **Time feeling**: Compressed, urgent
- **Front line duration**: 3-5 days (assault operations)
- **Rest**: 5-7 days (rapid turnaround)

#### Phase VII (Nov 2 - Dec 18): RECAPTURE
- **Historical**: French advance, gains secured
- **Rotations**: 2-3 (victory push)
- **Time feeling**: Cautious hope, fear of dying at the end
- **Front line duration**: 4-6 days (deliberate operations)
- **Rest**: 7-10 days (war winding down)

### Player Never Sees This Structure

Player doesn't know:
- ❌ "I'm in Phase III"
- ❌ "2 more rotations until Vaux"
- ❌ "I have 4 more months to survive"

Player only knows:
- ✅ "I'm at the front. Again."
- ✅ "I survived another shift."
- ✅ "When will this end?"

---

## TECHNICAL IMPLEMENTATION ARCHITECTURE

### Core Systems Required

#### System 1: Time State Manager

```pseudo
TimeStateManager {
    // Dual Timeline Tracking
    historicalDate: Date          // Real Verdun calendar
    campaignPhase: Phase          // Current historical phase
    daysSurvived: int             // Player progression

    // Current Shift State
    currentShift: ShiftType       // FRONT_LINE, SUPPORT, REST, MARCH
    shiftStartTime: GameTime      // When current shift began
    shiftPhase: TimePhase         // DAWN, DAYLIGHT, DUSK, NIGHT

    // Time Progression
    gameTimeScale: float          // Base time multiplier (1.0 default)
    perceptionModifier: float     // Subjective time (0.5-2.0)

    // Methods
    Update(deltaTime)             // Advance time based on modifiers
    TransitionPhase()             // Dawn→Daylight→Dusk→Night
    TriggerRotation()             // Check relief conditions
    SkipTime(days, narrative)     // Rest period time skip
}
```

#### System 2: Shift Phase Controller

```pseudo
ShiftPhaseController {
    // Phase Timing (Real-time durations)
    dawnDuration: Range(5-15 min)
    daylightDuration: Range(15-45 min)
    duskDuration: Range(5-15 min)
    nightDuration: Range(20-60 min)

    // Phase State
    currentPhase: TimePhase
    phaseProgress: float (0.0-1.0)
    phaseStartTime: GameTime

    // Environmental Controllers
    lightingController: LightingSystem
    weatherController: WeatherSystem
    soundController: AudioSystem
    npcController: NPCBehaviorSystem

    // Methods
    AdvancePhase()                        // Transition to next phase
    UpdateEnvironment(phaseProgress)      // Gradual environmental changes
    TriggerStandTo()                      // Dawn/Dusk alert
    UpdateNPCBehavior()                   // Phase-appropriate dialogue/actions
}
```

#### System 3: Perception Modulation Engine

```pseudo
PerceptionModulationEngine {
    // Stress State Tracking
    currentStressLevel: float (0.0-1.0)
    stressSources: List<StressSource>
    stressDecayRate: float

    // Modulation Parameters
    animationScale: float (0.5-1.0)
    audioCompressionLevel: float (0.0-1.0)
    visualEffectIntensity: float (0.0-1.0)
    controlResponsiveness: float (0.5-1.0)
    FOVReduction: float (40-100 degrees)

    // Methods
    AddStressSource(source, intensity, duration)
    RemoveStressSource(source)
    CalculatePerceptionModifier()
    ApplyModulation(playerController, audioSystem, visualSystem)

    // Stress Sources
    enum StressSource {
        ARTILLERY_NEAR_MISS (intensity: 0.8, decay: 5s)
        FRIENDLY_DEATH (intensity: 0.9, decay: 15s)
        UNDER_FIRE (intensity: 0.6, decay: continuous)
        SHELL_SHOCK_EVENT (intensity: 1.0, decay: 60s)
        BLEEDING (intensity: 0.4, decay: continuous)
        NO_MANS_LAND (intensity: 0.7, decay: continuous)
    }
}
```

#### System 4: Relief Condition Evaluator

```pseudo
ReliefConditionEvaluator {
    // Condition Tracking
    survivalMeters: SurvivalMeterSystem
    injuryState: InjurySystem
    unitState: UnitStateSystem
    timeServed: float (hours)

    // Relief Thresholds
    physicalThresholds: {
        alertness: 20%
        morale: 30%
        hunger: 20%
        thirst: 15%
        warmth: 25%
        hygiene: 20%
    }

    timeThresholds: {
        minimum: 24 hours
        maximum: 72 hours
        standard: 48 hours
    }

    // Methods
    EvaluateReliefEligibility() -> bool
    CheckPhysicalConditions() -> bool
    CheckInjuryState() -> bool
    CheckUnitState() -> bool
    CheckTimeServed() -> bool

    TriggerRelief() -> ReliefSequence
    DelayRelief(reason, duration)
}
```

#### System 5: Rest Period Narrative System

```pseudo
RestPeriodNarrativeSystem {
    // Narrative Beat Database
    beats: List<NarrativeBeat> {
        FirstNight,
        TheLetter,
        TheMirror,
        TheTraining,
        TheOrder
    }

    // Beat Structure
    struct NarrativeBeat {
        id: string
        triggerDay: int (rest day 1-14)
        duration: Range(minutes)
        scene: SceneData
        choices: List<PlayerChoice>
        effects: EffectData (morale, stats, etc.)
    }

    // Time Skip Data
    struct TimeSkip {
        daysToSkip: int
        calendarAdvance: int
        meterRestoration: MeterChanges
        characterChanges: PhysicalChanges (face, weight, etc.)
        historicalEvents: List<EventNotification>
    }

    // Methods
    InitiateRestPeriod(daysTotal)
    PlayNarrativeBeat(beatID)
    SkipToNextBeat()
    ApplyRestEffects(timeSkipped)
    TransitionBackToFront()
}
```

#### System 6: Environmental Time Signal System

```pseudo
EnvironmentalTimeSignalSystem {
    // Signal Components
    lightingSystem: DynamicLighting {
        sunAngle: float (0-360 degrees)
        sunIntensity: float (0.0-1.0)
        skyColor: GradientMap (time-based)
        ambientLight: float
        shadowLength: float
        shadowSharpness: float
    }

    weatherSystem: WeatherSimulation {
        temperature: float (varies by time/season)
        windSpeed: float (varies by time)
        windDirection: Vector
        fogDensity: float (high at dawn/dusk)
        precipitation: PrecipType
        cloudCover: float (0.0-1.0)
    }

    audioSystem: TimeAudio {
        ambientSoundscape: AudioMix (time-based)
        artilleryTempo: float (shells/hour)
        birdCalls: bool (dawn/dusk only)
        humanActivity: ActivityLevel
        windAudio: AudioSource
    }

    npcBehaviorSystem: NPCTimeReactions {
        dialoguePool: TimeBasedDialogue
        activitySchedule: NPCActivityMap
        alertnessLevel: AlertLevel (stand-to vs. normal)
        fatigueDisplay: AnimationSet
    }

    // Methods
    UpdateAllSystems(currentPhase, phaseProgress)
    SyncLightingToTime()
    SyncWeatherToTime()
    SyncAudioToTime()
    SyncNPCsToTime()
}
```

---

## EMOTIONAL RHYTHM DESIGN

### The Core Loop

```
┌─────────────────────────────────────────┐
│         FRONT LINE SHIFT                │
│  Terror → Endurance → Exhaustion        │
│  Duration: 45-270 minutes gameplay      │
│  Feeling: "Survive this hour"           │
└─────────────────┬───────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────┐
│         RELIEF & MARCH OUT              │
│  Crushing Relief → Collapse             │
│  Duration: 20-40 minutes gameplay       │
│  Feeling: "I made it. I'm alive."       │
└─────────────────┬───────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────┐
│         REST PERIOD                     │
│  Exhausted Relief → Dread of Return     │
│  Duration: 20-40 minutes (narrative)    │
│  Calendar: 7-14 days skip               │
│  Feeling: "This isn't long enough"      │
└─────────────────┬───────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────┐
│         RETURN MARCH                    │
│  Resignation → Dread                    │
│  Duration: 20-40 minutes gameplay       │
│  Feeling: "Here we go again"            │
└─────────────────┬───────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────┐
│         FRONT LINE SHIFT (AGAIN)        │
│  "Nothing has changed.                  │
│   Everything is worse."                 │
└─────────────────────────────────────────┘
```

### Emotional Beats Per Rotation

**Rotation Start**:
- Emotion: Resignation, dread, acceptance
- "We're going back."

**First 12 Hours**:
- Emotion: Alertness, fear, adaptation
- "Stay sharp. Watch the wire. Listen for shells."

**Hour 12-24**:
- Emotion: Fatigue setting in, routine
- "How long have we been here?"

**Hour 24-36**:
- Emotion: Exhaustion, desperation
- "When's the relief coming?"

**Hour 36-48**:
- Emotion: Breaking point approaching
- "I can't do this much longer."

**Hour 48-72** (if no relief):
- Emotion: Collapse, dissociation
- "I'm going to die here."

**Relief Arrives**:
- Emotion: Disbelief, then overwhelming relief
- "We made it. We're leaving."

**March Out**:
- Emotion: Numb, trembling, hollow
- "I can't believe I'm still alive."

**Rest Begins**:
- Emotion: Collapse, then gradual recovery
- "It's over. For now."

**Rest Middle**:
- Emotion: Peace, but hollow
- "I don't know how to be normal anymore."

**Rest Ends**:
- Emotion: Dread building
- "We have to go back."

**Return March**:
- Emotion: Resignation, hardening
- "Bury it. Shut down. Survive."

### The Whistle

**The most important moment in the entire game:**

```
Relief whistle blows.

Sound design:
├─ Whistle (clear, sharp, cutting through artillery)
├─ Officer: "51st Regiment! Prepare to move out!"
├─ Artillery: Continues (you're leaving, it keeps going)
├─ NPCs: Gasps, sobs, laughter, silence
├─ Player: Breathing (shaky, ragged)
└─ Music: None (silence is louder)

Visual:
├─ Camera stays steady (no cutscene)
├─ Player sees relief column approaching
├─ NPCs: Visible relief (some cry, some laugh, some stare)
├─ Player character: Hands stop shaking for a moment
└─ No UI, no fanfare

Duration: 10-20 seconds of nothing but environment

This is victory.
Not XP, not unlocks, not achievements.
Just survival.
```

---

## DESIGN VALIDATION CHECKLIST

✅ Does time feel lived, not measured?
✅ Can player survive a shift without checking a clock?
✅ Does relief feel EARNED, not scheduled?
✅ Does rest feel fleeting and insufficient?
✅ Does returning to the line feel inevitable?
✅ Do days blur together?
✅ Does danger stretch time?
✅ Does routine compress time?
✅ Does the player think "survive today" not "survive to day 100"?
✅ Does the relief whistle create crushing emotional release?

If yes to all: **You have recreated Verdun.**

---

## FINAL THOUGHT

The goal is not to simulate 303 days.

The goal is to make the player feel what it meant to survive **one more rotation**.

When the player hears that whistle and feels their hands trembling in relief—

When they collapse in the rest area and can't process that they're safe—

When they march back and feel their soul harden because "here we go again"—

**You have succeeded.**

Time isn't a number.
Time is survival.
Time is the space between heartbeats when a shell lands.
Time is the eternity of waiting for relief.
Time is knowing you'll go back.

That is Verdun.
That is Only The Dead.

---

**END OF TIME ENGINE DESIGN v1.0**

*"The only victory is making it back up the trench when the relief column arrives."*
