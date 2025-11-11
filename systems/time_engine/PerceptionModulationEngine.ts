/**
 * PERCEPTION MODULATION ENGINE
 *
 * Makes time FEEL different without changing actual time.
 * The historical clock ticks at constant rate, but the player's
 * subjective experience of time stretches and compresses based on:
 * - Stress/danger
 * - Fear/trauma
 * - Exhaustion
 * - Routine
 *
 * Philosophy: Time doesn't slow down - YOU do.
 * Implementation: Modulate animation speed, audio, visuals, controls.
 * Not actual time dilation, but experiential time distortion.
 */

// ============================================================================
// TYPES & ENUMS
// ============================================================================

export enum StressLevel {
    NONE = 0,       // 0.0-0.2
    LOW = 1,        // 0.2-0.4
    MODERATE = 2,   // 0.4-0.6
    HIGH = 3,       // 0.6-0.8
    EXTREME = 4     // 0.8-1.0
}

export interface StressSource {
    id: string;
    name: string;
    intensity: number;      // 0.0-1.0
    decayRate: number;      // Per second
    duration: number;       // Total duration (seconds), -1 = continuous
    timeRemaining: number;  // Seconds remaining
}

export interface PerceptionModifiers {
    // Animation pacing (1.0 = normal, <1.0 = slower/heavier)
    movementSpeed: number;       // Walk/run speed
    turnSpeed: number;           // Look/turn speed
    interactionSpeed: number;    // Reload, item use, etc.
    animationPlayback: number;   // Animation playback rate

    // Audio compression (0.0 = normal, 1.0 = maximum effect)
    audioCompression: number;    // High-frequency muffling
    heartbeatVolume: number;     // Heartbeat prominence
    tinnitus: number;            // Ringing in ears
    underwaterEffect: number;    // Distant/muffled quality

    // Visual effects (0.0 = normal, 1.0 = maximum effect)
    colorDesaturation: number;   // 0.0 = full color, 1.0 = monochrome
    fieldOfView: number;         // 40-100 degrees
    vignette: number;            // Edge darkening (tunnel vision)
    blur: number;                // Motion/focus blur
    shake: number;               // Camera shake/tremor

    // Control responsiveness (1.0 = normal, <1.0 = delayed/unresponsive)
    inputResponse: number;       // Input delay
    aimSensitivity: number;      // Aim control sensitivity
    reloadSpeed: number;         // Reload action speed
    interactionReliability: number;  // Can fail at low values (shell shock paralysis)
}

export interface PerceptionState {
    currentStressLevel: StressLevel;
    currentStressValue: number;  // 0.0-1.0
    activeSources: Map<string, StressSource>;
    modifiers: PerceptionModifiers;
}

// ============================================================================
// STRESS SOURCE PRESETS
// ============================================================================

export const STRESS_PRESETS = {
    // Artillery
    ARTILLERY_NEAR_MISS: {
        intensity: 0.8,
        decayRate: 0.2,      // Decays over 4-5 seconds
        duration: 5
    },
    ARTILLERY_DIRECT_HIT_NEARBY: {
        intensity: 0.95,
        decayRate: 0.1,      // Decays over 9-10 seconds
        duration: 10
    },
    BOMBARDMENT_CONTINUOUS: {
        intensity: 0.7,
        decayRate: 0.0,      // No decay (continuous)
        duration: -1
    },

    // Combat
    UNDER_FIRE: {
        intensity: 0.6,
        decayRate: 0.0,
        duration: -1
    },
    NEAR_MISS_RIFLE: {
        intensity: 0.5,
        decayRate: 0.3,
        duration: 3
    },
    FRIENDLY_DEATH: {
        intensity: 0.9,
        decayRate: 0.06,     // Decays over 15 seconds
        duration: 15
    },

    // Environmental
    NO_MANS_LAND: {
        intensity: 0.7,
        decayRate: 0.0,
        duration: -1
    },
    TRAPPED_BOMBARDMENT_SHELTER: {
        intensity: 0.75,
        decayRate: 0.0,
        duration: -1
    },
    LOST_IN_TRENCHES: {
        intensity: 0.55,
        decayRate: 0.0,
        duration: -1
    },
    GAS_ATTACK: {
        intensity: 0.85,
        decayRate: 0.0,
        duration: -1
    },

    // Injury
    BLEEDING: {
        intensity: 0.4,
        decayRate: 0.0,
        duration: -1
    },
    WOUNDED: {
        intensity: 0.6,
        decayRate: 0.0,
        duration: -1
    },
    DYING: {
        intensity: 0.95,
        decayRate: 0.0,
        duration: -1
    },

    // Trauma
    SHELL_SHOCK_EVENT: {
        intensity: 1.0,
        decayRate: 0.016,    // Decays over 60 seconds
        duration: 60
    },
    PANIC_ATTACK: {
        intensity: 0.85,
        decayRate: 0.05,     // Decays over 17 seconds
        duration: 20
    }
};

// ============================================================================
// PERCEPTION MODULATION ENGINE CLASS
// ============================================================================

export class PerceptionModulationEngine {
    private state: PerceptionState;

    // Baseline values (normal state)
    private readonly BASELINE_MODIFIERS: PerceptionModifiers = {
        movementSpeed: 1.0,
        turnSpeed: 1.0,
        interactionSpeed: 1.0,
        animationPlayback: 1.0,

        audioCompression: 0.0,
        heartbeatVolume: 0.0,
        tinnitus: 0.0,
        underwaterEffect: 0.0,

        colorDesaturation: 0.0,
        fieldOfView: 90.0,
        vignette: 0.0,
        blur: 0.0,
        shake: 0.0,

        inputResponse: 1.0,
        aimSensitivity: 1.0,
        reloadSpeed: 1.0,
        interactionReliability: 1.0
    };

    // Event callbacks
    private onStressLevelChangeCallbacks: Array<(newLevel: StressLevel, oldLevel: StressLevel) => void> = [];
    private onPerceptionChangeCallbacks: Array<(modifiers: PerceptionModifiers) => void> = [];

    constructor() {
        this.state = {
            currentStressLevel: StressLevel.NONE,
            currentStressValue: 0.0,
            activeSources: new Map(),
            modifiers: { ...this.BASELINE_MODIFIERS }
        };
    }

    // ========================================================================
    // MAIN UPDATE LOOP
    // ========================================================================

    /**
     * Update perception state - call every frame
     */
    public update(deltaTime: number): void {
        // Update all active stress sources
        this.updateStressSources(deltaTime);

        // Calculate total stress value
        const oldStressValue = this.state.currentStressValue;
        this.state.currentStressValue = this.calculateTotalStress();

        // Update stress level
        const oldLevel = this.state.currentStressLevel;
        this.state.currentStressLevel = this.calculateStressLevel(this.state.currentStressValue);

        // Recalculate modifiers based on stress
        this.state.modifiers = this.calculateModifiers(this.state.currentStressValue);

        // Trigger callbacks if stress level changed
        if (oldLevel !== this.state.currentStressLevel) {
            this.onStressLevelChangeCallbacks.forEach(callback => {
                callback(this.state.currentStressLevel, oldLevel);
            });
        }

        // Trigger perception change callbacks if significant change
        if (Math.abs(this.state.currentStressValue - oldStressValue) > 0.05) {
            this.onPerceptionChangeCallbacks.forEach(callback => {
                callback(this.state.modifiers);
            });
        }
    }

    // ========================================================================
    // STRESS SOURCE MANAGEMENT
    // ========================================================================

    /**
     * Add a stress source (e.g., artillery near-miss)
     */
    public addStressSource(
        id: string,
        name: string,
        intensity: number,
        decayRate: number,
        duration: number = -1
    ): void {
        const source: StressSource = {
            id,
            name,
            intensity: Math.max(0.0, Math.min(1.0, intensity)),
            decayRate,
            duration,
            timeRemaining: duration
        };

        this.state.activeSources.set(id, source);

        console.log(`[Perception] Stress source added: ${name} (intensity: ${intensity.toFixed(2)})`);
    }

    /**
     * Add a stress source using a preset
     */
    public addStressPreset(id: string, presetName: keyof typeof STRESS_PRESETS): void {
        const preset = STRESS_PRESETS[presetName];
        if (preset) {
            this.addStressSource(id, presetName, preset.intensity, preset.decayRate, preset.duration);
        } else {
            console.warn(`[Perception] Unknown stress preset: ${presetName}`);
        }
    }

    /**
     * Remove a stress source by ID
     */
    public removeStressSource(id: string): void {
        if (this.state.activeSources.delete(id)) {
            console.log(`[Perception] Stress source removed: ${id}`);
        }
    }

    /**
     * Remove all stress sources
     */
    public clearAllStressSources(): void {
        this.state.activeSources.clear();
        console.log(`[Perception] All stress sources cleared`);
    }

    /**
     * Check if a specific stress source is active
     */
    public hasStressSource(id: string): boolean {
        return this.state.activeSources.has(id);
    }

    // ========================================================================
    // STRESS CALCULATION
    // ========================================================================

    private updateStressSources(deltaTime: number): void {
        const toRemove: string[] = [];

        this.state.activeSources.forEach((source, id) => {
            // Decay intensity
            if (source.decayRate > 0) {
                source.intensity = Math.max(0, source.intensity - (source.decayRate * deltaTime));
            }

            // Update duration
            if (source.duration > 0) {
                source.timeRemaining -= deltaTime;
                if (source.timeRemaining <= 0) {
                    toRemove.push(id);
                }
            }

            // Remove if intensity reached zero
            if (source.intensity <= 0) {
                toRemove.push(id);
            }
        });

        // Remove expired sources
        toRemove.forEach(id => this.state.activeSources.delete(id));
    }

    private calculateTotalStress(): number {
        if (this.state.activeSources.size === 0) {
            return 0.0;
        }

        // Use maximum stress value (not additive - most intense source dominates)
        // But with a slight boost from multiple sources
        let maxStress = 0.0;
        let stressCount = 0;

        this.state.activeSources.forEach(source => {
            maxStress = Math.max(maxStress, source.intensity);
            stressCount++;
        });

        // Multiple sources add up to 20% more stress
        const multiSourceBonus = Math.min(0.2, (stressCount - 1) * 0.05);

        return Math.min(1.0, maxStress + multiSourceBonus);
    }

    private calculateStressLevel(stress: number): StressLevel {
        if (stress >= 0.8) return StressLevel.EXTREME;
        if (stress >= 0.6) return StressLevel.HIGH;
        if (stress >= 0.4) return StressLevel.MODERATE;
        if (stress >= 0.2) return StressLevel.LOW;
        return StressLevel.NONE;
    }

    // ========================================================================
    // MODIFIER CALCULATION
    // ========================================================================

    private calculateModifiers(stress: number): PerceptionModifiers {
        // Interpolate between baseline and extreme values based on stress
        const modifiers: PerceptionModifiers = {
            // Animation pacing (higher stress = slower, heavier movement)
            movementSpeed: this.lerp(1.0, 0.7, stress),
            turnSpeed: this.lerp(1.0, 0.8, stress),
            interactionSpeed: this.lerp(1.0, 0.6, stress),
            animationPlayback: this.lerp(1.0, 0.85, stress),

            // Audio effects (higher stress = more compression)
            audioCompression: this.lerp(0.0, 1.0, stress),
            heartbeatVolume: this.lerp(0.0, 0.8, stress),
            tinnitus: this.lerp(0.0, 0.6, this.smoothstep(0.6, 1.0, stress)),  // Only at high stress
            underwaterEffect: this.lerp(0.0, 0.7, this.smoothstep(0.7, 1.0, stress)),  // Only at extreme stress

            // Visual effects (higher stress = more distortion)
            colorDesaturation: this.lerp(0.0, 0.7, stress),
            fieldOfView: this.lerp(90.0, 50.0, stress),  // Tunnel vision
            vignette: this.lerp(0.0, 0.8, stress),
            blur: this.lerp(0.0, 0.4, this.smoothstep(0.5, 1.0, stress)),
            shake: this.lerp(0.0, 0.6, stress),

            // Control responsiveness (higher stress = less control)
            inputResponse: this.lerp(1.0, 0.4, stress),  // 40% responsiveness at max stress
            aimSensitivity: this.lerp(1.0, 0.5, stress),
            reloadSpeed: this.lerp(1.0, 0.6, stress),
            interactionReliability: this.lerp(1.0, 0.3, this.smoothstep(0.8, 1.0, stress))  // Can fail at extreme stress
        };

        return modifiers;
    }

    // ========================================================================
    // GETTERS
    // ========================================================================

    public getState(): Readonly<PerceptionState> {
        return {
            ...this.state,
            activeSources: new Map(this.state.activeSources),
            modifiers: { ...this.state.modifiers }
        };
    }

    public getStressValue(): number {
        return this.state.currentStressValue;
    }

    public getStressLevel(): StressLevel {
        return this.state.currentStressLevel;
    }

    public getModifiers(): Readonly<PerceptionModifiers> {
        return { ...this.state.modifiers };
    }

    public getActiveSourceCount(): number {
        return this.state.activeSources.size;
    }

    // ========================================================================
    // EVENT REGISTRATION
    // ========================================================================

    public onStressLevelChange(callback: (newLevel: StressLevel, oldLevel: StressLevel) => void): void {
        this.onStressLevelChangeCallbacks.push(callback);
    }

    public onPerceptionChange(callback: (modifiers: PerceptionModifiers) => void): void {
        this.onPerceptionChangeCallbacks.push(callback);
    }

    // ========================================================================
    // UTILITY
    // ========================================================================

    private lerp(a: number, b: number, t: number): number {
        return a + (b - a) * t;
    }

    private smoothstep(edge0: number, edge1: number, x: number): number {
        // Smooth interpolation between edge0 and edge1
        const t = Math.max(0, Math.min(1, (x - edge0) / (edge1 - edge0)));
        return t * t * (3 - 2 * t);
    }

    // ========================================================================
    // CONVENIENCE METHODS
    // ========================================================================

    /**
     * Quick stress events (using presets)
     */
    public artilleryNearMiss(): void {
        this.addStressPreset('artillery_' + Date.now(), 'ARTILLERY_NEAR_MISS');
    }

    public friendlyDeath(): void {
        this.addStressPreset('death_' + Date.now(), 'FRIENDLY_DEATH');
    }

    public shellShockEvent(): void {
        this.addStressPreset('shellshock_' + Date.now(), 'SHELL_SHOCK_EVENT');
    }

    /**
     * Continuous stress states (must be manually removed)
     */
    public enterBombardment(): void {
        this.addStressPreset('bombardment', 'BOMBARDMENT_CONTINUOUS');
    }

    public exitBombardment(): void {
        this.removeStressSource('bombardment');
    }

    public enterNoMansLand(): void {
        this.addStressPreset('nomansland', 'NO_MANS_LAND');
    }

    public exitNoMansLand(): void {
        this.removeStressSource('nomansland');
    }

    public enterCombat(): void {
        this.addStressPreset('combat', 'UNDER_FIRE');
    }

    public exitCombat(): void {
        this.removeStressSource('combat');
    }

    // ========================================================================
    // DEBUG
    // ========================================================================

    public getDebugInfo(): string {
        const sources: string[] = [];
        this.state.activeSources.forEach(source => {
            const duration = source.duration > 0 ? `${source.timeRemaining.toFixed(1)}s` : 'continuous';
            sources.push(`  - ${source.name}: ${(source.intensity * 100).toFixed(0)}% (${duration})`);
        });

        const mods = this.state.modifiers;

        return `
=== PERCEPTION MODULATION ENGINE DEBUG ===
Stress Value: ${(this.state.currentStressValue * 100).toFixed(1)}%
Stress Level: ${StressLevel[this.state.currentStressLevel]}
Active Sources: ${this.state.activeSources.size}
${sources.join('\n')}

MODIFIERS:
Animation:
  Movement Speed: ${(mods.movementSpeed * 100).toFixed(0)}%
  Turn Speed: ${(mods.turnSpeed * 100).toFixed(0)}%
  Interaction Speed: ${(mods.interactionSpeed * 100).toFixed(0)}%

Audio:
  Compression: ${(mods.audioCompression * 100).toFixed(0)}%
  Heartbeat: ${(mods.heartbeatVolume * 100).toFixed(0)}%
  Tinnitus: ${(mods.tinnitus * 100).toFixed(0)}%

Visual:
  Desaturation: ${(mods.colorDesaturation * 100).toFixed(0)}%
  FOV: ${mods.fieldOfView.toFixed(0)}°
  Vignette: ${(mods.vignette * 100).toFixed(0)}%
  Shake: ${(mods.shake * 100).toFixed(0)}%

Controls:
  Input Response: ${(mods.inputResponse * 100).toFixed(0)}%
  Aim Sensitivity: ${(mods.aimSensitivity * 100).toFixed(0)}%
  Reload Speed: ${(mods.reloadSpeed * 100).toFixed(0)}%
  Reliability: ${(mods.interactionReliability * 100).toFixed(0)}%
==========================================
        `.trim();
    }
}

// ============================================================================
// SINGLETON INSTANCE (Optional)
// ============================================================================

let _perceptionEngineInstance: PerceptionModulationEngine | null = null;

export function getPerceptionEngine(): PerceptionModulationEngine {
    if (!_perceptionEngineInstance) {
        _perceptionEngineInstance = new PerceptionModulationEngine();
    }
    return _perceptionEngineInstance;
}

export function resetPerceptionEngine(): void {
    _perceptionEngineInstance = null;
}
