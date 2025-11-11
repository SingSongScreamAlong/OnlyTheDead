/**
 * ENVIRONMENTAL TIME SIGNAL SYSTEM
 *
 * Communicates time through environment, not UI.
 * NO clocks. NO time widgets. NO "Day 47" counters.
 *
 * Time is indicated through:
 * - Light quality and sun position
 * - Weather and atmospheric conditions
 * - Sound (artillery tempo, birds, human activity)
 * - NPC behavior and dialogue
 * - Player character physical state
 *
 * Philosophy: Time is felt through the world, not read from a HUD.
 */

import { TimePhase } from './TimeStateManager';

// ============================================================================
// TYPES & INTERFACES
// ============================================================================

export interface LightingState {
    // Sun
    sunAngle: number;           // 0-360 degrees (0 = midnight, 90 = dawn, 180 = noon, 270 = dusk)
    sunIntensity: number;       // 0.0-1.0
    sunColor: RGB;

    // Sky
    skyColor: RGB;
    horizonColor: RGB;
    cloudCover: number;         // 0.0-1.0

    // Ambient
    ambientLightIntensity: number;
    ambientLightColor: RGB;

    // Shadows
    shadowLength: number;       // 0.0-10.0 (relative to object height)
    shadowSharpness: number;    // 0.0-1.0

    // Fog/Atmosphere
    fogDensity: number;         // 0.0-1.0
    fogColor: RGB;
    atmosphericScattering: number;  // 0.0-1.0 (haze)

    // Visibility
    visibilityRange: number;    // Meters
}

export interface WeatherState {
    // Temperature
    temperature: number;        // Celsius

    // Wind
    windSpeed: number;          // m/s
    windDirection: number;      // Degrees

    // Precipitation
    precipitation: PrecipitationType;
    precipitationIntensity: number;  // 0.0-1.0

    // Atmospheric
    cloudCover: number;         // 0.0-1.0
    fogDensity: number;         // 0.0-1.0
    humidity: number;           // 0.0-1.0

    // Ground conditions
    mudConsistency: number;     // 0.0 = frozen/dry, 1.0 = liquid mud
}

export enum PrecipitationType {
    NONE = "NONE",
    RAIN = "RAIN",
    SNOW = "SNOW",
    SLEET = "SLEET"
}

export interface AudioState {
    // Artillery (shells per minute)
    artilleryTempo: number;

    // Ambient sounds
    windVolume: number;         // 0.0-1.0
    birdCallsActive: boolean;
    distantGunfire: number;     // 0.0-1.0

    // Human activity
    humanActivityLevel: ActivityLevel;
    conversationVolume: number; // 0.0-1.0 (whispers to normal speech)

    // Environmental
    mudFootsteps: boolean;
    waterDrips: boolean;
}

export enum ActivityLevel {
    NONE = "NONE",           // Night, silence
    LOW = "LOW",             // Minimal movement, whispers
    MODERATE = "MODERATE",   // Normal trench work
    HIGH = "HIGH"            // Busy resupply, many people
}

export interface NPCBehaviorState {
    alertnessLevel: AlertLevel;
    dialoguePool: DialogueSet;
    activityType: NPCActivity;
    fatigueLevel: number;    // 0.0-1.0 (affects animations)
}

export enum AlertLevel {
    STAND_TO = "STAND_TO",       // Dawn/dusk, high alert
    ALERT = "ALERT",             // Daytime, weapons ready
    ROUTINE = "ROUTINE",         // Work mode
    RESTING = "RESTING"          // Minimal activity
}

export enum NPCActivity {
    STAND_TO_WATCH = "STAND_TO_WATCH",
    TRENCH_REPAIR = "TRENCH_REPAIR",
    SENTRY_DUTY = "SENTRY_DUTY",
    EATING = "EATING",
    RESTING = "RESTING",
    RESUPPLY = "RESUPPLY",
    BURIAL_DETAIL = "BURIAL_DETAIL"
}

export interface DialogueSet {
    greetings: string[];
    observations: string[];
    timeReferences: string[];
}

export interface RGB {
    r: number;  // 0-255
    g: number;
    b: number;
}

export interface Season {
    name: string;
    startMonth: number;  // 1-12
    temperatureRange: { min: number; max: number };
    precipitationChance: number;
    mudLevel: number;
}

// ============================================================================
// CONSTANTS & PRESETS
// ============================================================================

export const SEASONS: Season[] = [
    {
        name: "Winter",
        startMonth: 12,
        temperatureRange: { min: -5, max: 5 },
        precipitationChance: 0.3,
        mudLevel: 0.3  // Frozen
    },
    {
        name: "Spring",
        startMonth: 3,
        temperatureRange: { min: 5, max: 15 },
        precipitationChance: 0.5,
        mudLevel: 0.9  // MUD SEASON
    },
    {
        name: "Summer",
        startMonth: 6,
        temperatureRange: { min: 15, max: 30 },
        precipitationChance: 0.2,
        mudLevel: 0.4
    },
    {
        name: "Autumn",
        startMonth: 9,
        temperatureRange: { min: 5, max: 15 },
        precipitationChance: 0.4,
        mudLevel: 0.7
    }
];

// Artillery tempo patterns (historically accurate for Verdun)
export const ARTILLERY_PATTERNS = {
    DAWN: { min: 20, max: 50 },        // "Morning hate"
    DAYLIGHT: { min: 5, max: 30 },     // Variable
    DUSK: { min: 20, max: 50 },        // "Evening hate"
    NIGHT: { min: 5, max: 15 }         // Harassment fire
};

// ============================================================================
// DIALOGUE SETS
// ============================================================================

export const DIALOGUE_SETS: Record<TimePhase, DialogueSet> = {
    [TimePhase.DAWN]: {
        greetings: ["Morning...", "Still alive.", "Another day."],
        observations: [
            "Watch the wire.",
            "Quiet... too quiet.",
            "Stand-to! Everyone up!",
            "Listen... do you hear that?"
        ],
        timeReferences: [
            "Sun's coming up.",
            "Almost dawn.",
            "Stand-to time.",
            "Here we go again."
        ]
    },

    [TimePhase.DAYLIGHT]: {
        greetings: ["Keep your head down.", "How are you holding up?", "What a day."],
        observations: [
            "Haul those sandbags!",
            "My back is killing me...",
            "How long have we been at this?",
            "When's relief supposed to come?",
            "At least it's not raining."
        ],
        timeReferences: [
            "Still hours until dusk.",
            "Long day ahead.",
            "Sun's high.",
            "Feels like we've been here forever."
        ]
    },

    [TimePhase.DUSK]: {
        greetings: ["Careful now.", "Night's coming.", "Stay sharp."],
        observations: [
            "Stand-to! Man your positions!",
            "Check the flare pistol.",
            "Going to be a long night...",
            "Sun's getting low...",
            "Better eat now, won't get another chance."
        ],
        timeReferences: [
            "Almost dark.",
            "Dusk stand-to.",
            "Night comes fast.",
            "Here comes the dark."
        ]
    },

    [TimePhase.NIGHT]: {
        greetings: ["Shh.", "Quiet.", "..."],
        observations: [
            "Patrol's going out in five.",
            "Stay awake! Sentry duty!",
            "Did you hear that?",
            "I can't see a damn thing...",
            "Keep your eyes on the wire.",
            "Flare!"
        ],
        timeReferences: [
            "Long night ahead.",
            "Hours until dawn.",
            "Can't see the stars tonight.",
            "When will morning come?"
        ]
    }
};

// ============================================================================
// ENVIRONMENTAL TIME SIGNAL SYSTEM CLASS
// ============================================================================

export class EnvironmentalTimeSignalSystem {
    private currentPhase: TimePhase;
    private phaseProgress: number;  // 0.0-1.0

    private lightingState: LightingState;
    private weatherState: WeatherState;
    private audioState: AudioState;
    private npcBehaviorState: NPCBehaviorState;

    private currentSeason: Season;
    private historicalDate: Date;

    constructor() {
        this.currentPhase = TimePhase.DAWN;
        this.phaseProgress = 0.0;

        this.lightingState = this.initializeLighting();
        this.weatherState = this.initializeWeather();
        this.audioState = this.initializeAudio();
        this.npcBehaviorState = this.initializeNPCBehavior();

        this.currentSeason = SEASONS[0];  // Winter (Feb 1916)
        this.historicalDate = new Date(1916, 1, 21);
    }

    // ========================================================================
    // INITIALIZATION
    // ========================================================================

    private initializeLighting(): LightingState {
        return {
            sunAngle: 90,  // Dawn
            sunIntensity: 0.1,
            sunColor: { r: 255, g: 200, b: 150 },
            skyColor: { r: 50, g: 60, b: 80 },
            horizonColor: { r: 100, g: 80, b: 60 },
            cloudCover: 0.6,
            ambientLightIntensity: 0.2,
            ambientLightColor: { r: 100, g: 110, b: 120 },
            shadowLength: 8.0,
            shadowSharpness: 0.3,
            fogDensity: 0.7,
            fogColor: { r: 180, g: 180, b: 180 },
            atmosphericScattering: 0.5,
            visibilityRange: 50
        };
    }

    private initializeWeather(): WeatherState {
        return {
            temperature: 2,
            windSpeed: 3.0,
            windDirection: 270,
            precipitation: PrecipitationType.NONE,
            precipitationIntensity: 0.0,
            cloudCover: 0.6,
            fogDensity: 0.7,
            humidity: 0.8,
            mudConsistency: 0.4
        };
    }

    private initializeAudio(): AudioState {
        return {
            artilleryTempo: 10,
            windVolume: 0.3,
            birdCallsActive: false,
            distantGunfire: 0.2,
            humanActivityLevel: ActivityLevel.LOW,
            conversationVolume: 0.1,
            mudFootsteps: true,
            waterDrips: true
        };
    }

    private initializeNPCBehavior(): NPCBehaviorState {
        return {
            alertnessLevel: AlertLevel.STAND_TO,
            dialoguePool: DIALOGUE_SETS[TimePhase.DAWN],
            activityType: NPCActivity.STAND_TO_WATCH,
            fatigueLevel: 0.6
        };
    }

    // ========================================================================
    // MAIN UPDATE
    // ========================================================================

    /**
     * Update all environmental signals based on time phase and progress
     */
    public update(phase: TimePhase, progress: number, date: Date): void {
        this.currentPhase = phase;
        this.phaseProgress = progress;
        this.historicalDate = date;

        // Update season
        this.updateSeason(date);

        // Update all subsystems
        this.updateLighting(phase, progress);
        this.updateWeather(phase, progress);
        this.updateAudio(phase, progress);
        this.updateNPCBehavior(phase, progress);
    }

    // ========================================================================
    // LIGHTING SYSTEM
    // ========================================================================

    private updateLighting(phase: TimePhase, progress: number): void {
        switch (phase) {
            case TimePhase.DAWN:
                this.updateDawnLighting(progress);
                break;
            case TimePhase.DAYLIGHT:
                this.updateDaylightLighting(progress);
                break;
            case TimePhase.DUSK:
                this.updateDuskLighting(progress);
                break;
            case TimePhase.NIGHT:
                this.updateNightLighting(progress);
                break;
        }
    }

    private updateDawnLighting(progress: number): void {
        // Sun rises from angle 0 (horizon) to 90 (morning sky)
        this.lightingState.sunAngle = this.lerp(0, 45, progress);
        this.lightingState.sunIntensity = this.lerp(0.0, 0.6, progress);

        // Sky transitions from dark blue to light blue/grey
        this.lightingState.skyColor = this.lerpColor(
            { r: 20, g: 30, b: 50 },   // Dark pre-dawn
            { r: 150, g: 170, b: 190 }, // Light morning grey
            progress
        );

        // Horizon glows orange/pink
        this.lightingState.horizonColor = this.lerpColor(
            { r: 50, g: 40, b: 60 },    // Dark
            { r: 255, g: 180, b: 120 }, // Orange glow
            this.smoothstep(0.3, 0.7, progress)
        );

        // Fog lifts gradually
        this.lightingState.fogDensity = this.lerp(0.9, 0.5, progress);

        // Shadows become sharper and shorter
        this.lightingState.shadowLength = this.lerp(10.0, 4.0, progress);
        this.lightingState.shadowSharpness = this.lerp(0.1, 0.6, progress);

        // Visibility increases
        this.lightingState.visibilityRange = this.lerp(10, 100, progress);
    }

    private updateDaylightLighting(progress: number): void {
        // Sun moves from morning (45°) through noon (180°) to afternoon (270°)
        this.lightingState.sunAngle = this.lerp(45, 270, progress);
        this.lightingState.sunIntensity = this.lerp(0.6, 0.8, Math.sin(progress * Math.PI));  // Peak at noon

        // Sky: Morning grey → midday blue → afternoon grey
        const noonBlue = { r: 150, g: 180, b: 210 };
        const grey = { r: 140, g: 150, b: 160 };

        if (progress < 0.5) {
            this.lightingState.skyColor = this.lerpColor(grey, noonBlue, progress * 2);
        } else {
            this.lightingState.skyColor = this.lerpColor(noonBlue, grey, (progress - 0.5) * 2);
        }

        // Shadows: Long morning → short noon → long afternoon
        this.lightingState.shadowLength = this.lerp(4.0, 1.5, Math.sin(progress * Math.PI));
        this.lightingState.shadowSharpness = 0.8;

        // Fog mostly cleared
        this.lightingState.fogDensity = 0.2;

        // Maximum visibility
        this.lightingState.visibilityRange = 300;
    }

    private updateDuskLighting(progress: number): void {
        // Sun sets from afternoon (270°) toward horizon (360°)
        this.lightingState.sunAngle = this.lerp(270, 360, progress);
        this.lightingState.sunIntensity = this.lerp(0.6, 0.0, progress);

        // Sky transitions from light to dark with golden/red sunset
        this.lightingState.skyColor = this.lerpColor(
            { r: 150, g: 170, b: 190 }, // Afternoon grey
            { r: 30, g: 35, b: 50 },    // Dark blue night
            progress
        );

        // Horizon glows golden/red
        this.lightingState.horizonColor = this.lerpColor(
            { r: 255, g: 200, b: 100 }, // Golden
            { r: 150, g: 50, b: 30 },   // Deep red
            this.smoothstep(0.2, 0.8, progress)
        );

        // Fog returns
        this.lightingState.fogDensity = this.lerp(0.2, 0.7, progress);

        // Shadows lengthen and soften
        this.lightingState.shadowLength = this.lerp(1.5, 8.0, progress);
        this.lightingState.shadowSharpness = this.lerp(0.8, 0.2, progress);

        // Visibility decreases
        this.lightingState.visibilityRange = this.lerp(300, 20, progress);
    }

    private updateNightLighting(progress: number): void {
        // Night is dark
        this.lightingState.sunAngle = 0;  // Below horizon
        this.lightingState.sunIntensity = 0.0;

        // Dark sky (varies by cloud cover and moon)
        const moonPhase = this.calculateMoonPhase(this.historicalDate);
        const moonlight = moonPhase * (1.0 - this.lightingState.cloudCover) * 0.2;

        this.lightingState.skyColor = {
            r: Math.floor(20 + moonlight * 30),
            g: Math.floor(25 + moonlight * 35),
            b: Math.floor(40 + moonlight * 40)
        };

        // No shadows (or very faint from moon)
        this.lightingState.shadowLength = 0.0;
        this.lightingState.shadowSharpness = 0.0;

        // Thick fog at night
        this.lightingState.fogDensity = 0.8;

        // Very limited visibility
        this.lightingState.visibilityRange = this.lerp(5, 15, moonlight);
    }

    // ========================================================================
    // WEATHER SYSTEM
    // ========================================================================

    private updateWeather(phase: TimePhase, progress: number): void {
        // Temperature varies by phase and season
        const seasonTemp = this.currentSeason.temperatureRange;

        switch (phase) {
            case TimePhase.DAWN:
                this.weatherState.temperature = seasonTemp.min;  // Coldest
                break;
            case TimePhase.DAYLIGHT:
                this.weatherState.temperature = this.lerp(seasonTemp.min, seasonTemp.max, progress);
                break;
            case TimePhase.DUSK:
                this.weatherState.temperature = this.lerp(seasonTemp.max, seasonTemp.min, progress);
                break;
            case TimePhase.NIGHT:
                this.weatherState.temperature = seasonTemp.min - 2;  // Coldest
                break;
        }

        // Wind tends to increase in afternoon
        if (phase === TimePhase.DAYLIGHT) {
            this.weatherState.windSpeed = this.lerp(2.0, 5.0, progress);
        } else {
            this.weatherState.windSpeed = 2.5;
        }

        // Mud consistency (affected by temperature, precipitation, season)
        this.updateMudConsistency();
    }

    private updateMudConsistency(): void {
        let mud = this.currentSeason.mudLevel;

        // Frozen if below 0°C
        if (this.weatherState.temperature < 0) {
            mud = Math.min(mud, 0.2);
        }

        // Rain increases mud
        if (this.weatherState.precipitation === PrecipitationType.RAIN) {
            mud = Math.min(1.0, mud + (this.weatherState.precipitationIntensity * 0.3));
        }

        this.weatherState.mudConsistency = mud;
    }

    // ========================================================================
    // AUDIO SYSTEM
    // ========================================================================

    private updateAudio(phase: TimePhase, progress: number): void {
        // Artillery tempo follows historical patterns
        const pattern = ARTILLERY_PATTERNS[phase];
        this.audioState.artilleryTempo = this.lerp(pattern.min, pattern.max, Math.random());

        // Bird calls only at dawn/dusk (when they actually sing)
        this.audioState.birdCallsActive = (
            (phase === TimePhase.DAWN && progress > 0.3) ||
            (phase === TimePhase.DUSK && progress < 0.7)
        );

        // Human activity varies by phase
        switch (phase) {
            case TimePhase.DAWN:
            case TimePhase.DUSK:
                this.audioState.humanActivityLevel = ActivityLevel.LOW;
                this.audioState.conversationVolume = 0.1;  // Whispers
                break;
            case TimePhase.DAYLIGHT:
                this.audioState.humanActivityLevel = ActivityLevel.MODERATE;
                this.audioState.conversationVolume = 0.6;  // Normal speech
                break;
            case TimePhase.NIGHT:
                this.audioState.humanActivityLevel = ActivityLevel.LOW;
                this.audioState.conversationVolume = 0.05; // Near silence
                break;
        }

        // Wind volume
        this.audioState.windVolume = this.weatherState.windSpeed / 10.0;
    }

    // ========================================================================
    // NPC BEHAVIOR SYSTEM
    // ========================================================================

    private updateNPCBehavior(phase: TimePhase, progress: number): void {
        // Update dialogue pool
        this.npcBehaviorState.dialoguePool = DIALOGUE_SETS[phase];

        // Update alertness and activity
        switch (phase) {
            case TimePhase.DAWN:
            case TimePhase.DUSK:
                this.npcBehaviorState.alertnessLevel = AlertLevel.STAND_TO;
                this.npcBehaviorState.activityType = NPCActivity.STAND_TO_WATCH;
                break;

            case TimePhase.DAYLIGHT:
                this.npcBehaviorState.alertnessLevel = AlertLevel.ROUTINE;
                // Vary activity during day
                if (progress < 0.3) {
                    this.npcBehaviorState.activityType = NPCActivity.EATING;
                } else if (progress < 0.7) {
                    this.npcBehaviorState.activityType = NPCActivity.TRENCH_REPAIR;
                } else {
                    this.npcBehaviorState.activityType = NPCActivity.SENTRY_DUTY;
                }
                break;

            case TimePhase.NIGHT:
                this.npcBehaviorState.alertnessLevel = AlertLevel.ALERT;
                // Night activities rotate
                if (progress < 0.5) {
                    this.npcBehaviorState.activityType = NPCActivity.SENTRY_DUTY;
                } else {
                    this.npcBehaviorState.activityType = NPCActivity.RESTING;
                }
                break;
        }
    }

    // ========================================================================
    // SEASON MANAGEMENT
    // ========================================================================

    private updateSeason(date: Date): void {
        const month = date.getMonth() + 1;  // 1-12

        for (const season of SEASONS) {
            if (month >= season.startMonth && month < season.startMonth + 3) {
                this.currentSeason = season;
                return;
            }
        }

        // Wrap around for winter (Dec-Feb)
        if (month >= 12 || month <= 2) {
            this.currentSeason = SEASONS[0];  // Winter
        }
    }

    // ========================================================================
    // UTILITY
    // ========================================================================

    private lerp(a: number, b: number, t: number): number {
        return a + (b - a) * Math.max(0, Math.min(1, t));
    }

    private smoothstep(edge0: number, edge1: number, x: number): number {
        const t = Math.max(0, Math.min(1, (x - edge0) / (edge1 - edge0)));
        return t * t * (3 - 2 * t);
    }

    private lerpColor(a: RGB, b: RGB, t: number): RGB {
        return {
            r: Math.floor(this.lerp(a.r, b.r, t)),
            g: Math.floor(this.lerp(a.g, b.g, t)),
            b: Math.floor(this.lerp(a.b, b.b, t))
        };
    }

    private calculateMoonPhase(date: Date): number {
        // Simplified moon phase calculation (0.0 = new moon, 1.0 = full moon)
        const days = Math.floor((date.getTime() - new Date(1916, 0, 1).getTime()) / (1000 * 60 * 60 * 24));
        const phase = (days % 29.53) / 29.53;  // Lunar cycle is ~29.53 days

        // Convert to brightness (full moon = 1.0, new moon = 0.0)
        return Math.abs(Math.cos(phase * Math.PI * 2)) ;
    }

    // ========================================================================
    // GETTERS
    // ========================================================================

    public getLightingState(): Readonly<LightingState> {
        return { ...this.lightingState };
    }

    public getWeatherState(): Readonly<WeatherState> {
        return { ...this.weatherState };
    }

    public getAudioState(): Readonly<AudioState> {
        return { ...this.audioState };
    }

    public getNPCBehaviorState(): Readonly<NPCBehaviorState> {
        return { ...this.npcBehaviorState };
    }

    public getCurrentSeason(): Season {
        return this.currentSeason;
    }

    /**
     * Get random NPC dialogue appropriate for current time
     */
    public getRandomDialogue(type: 'greeting' | 'observation' | 'timeReference'): string {
        const pool = this.npcBehaviorState.dialoguePool;
        let options: string[] = [];

        switch (type) {
            case 'greeting':
                options = pool.greetings;
                break;
            case 'observation':
                options = pool.observations;
                break;
            case 'timeReference':
                options = pool.timeReferences;
                break;
        }

        return options[Math.floor(Math.random() * options.length)];
    }

    // ========================================================================
    // DEBUG
    // ========================================================================

    public getDebugInfo(): string {
        const light = this.lightingState;
        const weather = this.weatherState;
        const audio = this.audioState;
        const npc = this.npcBehaviorState;

        return `
=== ENVIRONMENTAL TIME SIGNAL DEBUG ===
Phase: ${this.currentPhase} (${(this.phaseProgress * 100).toFixed(0)}%)
Season: ${this.currentSeason.name}
Date: ${this.historicalDate.toDateString()}

LIGHTING:
  Sun Angle: ${light.sunAngle.toFixed(0)}°
  Sun Intensity: ${(light.sunIntensity * 100).toFixed(0)}%
  Sky: RGB(${light.skyColor.r}, ${light.skyColor.g}, ${light.skyColor.b})
  Fog: ${(light.fogDensity * 100).toFixed(0)}%
  Visibility: ${light.visibilityRange.toFixed(0)}m

WEATHER:
  Temperature: ${weather.temperature.toFixed(1)}°C
  Wind: ${weather.windSpeed.toFixed(1)} m/s
  Precipitation: ${weather.precipitation}
  Mud: ${(weather.mudConsistency * 100).toFixed(0)}%

AUDIO:
  Artillery: ${audio.artilleryTempo.toFixed(0)} shells/min
  Birds: ${audio.birdCallsActive ? 'Yes' : 'No'}
  Activity: ${audio.humanActivityLevel}
  Conversation: ${(audio.conversationVolume * 100).toFixed(0)}%

NPC BEHAVIOR:
  Alertness: ${npc.alertnessLevel}
  Activity: ${npc.activityType}
  Fatigue: ${(npc.fatigueLevel * 100).toFixed(0)}%
=======================================
        `.trim();
    }
}

// ============================================================================
// SINGLETON INSTANCE (Optional)
// ============================================================================

let _envSignalInstance: EnvironmentalTimeSignalSystem | null = null;

export function getEnvironmentalSignalSystem(): EnvironmentalTimeSignalSystem {
    if (!_envSignalInstance) {
        _envSignalInstance = new EnvironmentalTimeSignalSystem();
    }
    return _envSignalInstance;
}

export function resetEnvironmentalSignalSystem(): void {
    _envSignalInstance = null;
}
