/**
 * VERDUN TIME ENGINE - MASTER INTEGRATION
 *
 * Orchestrates all time-related systems into a unified, cohesive experience.
 * This is the main interface that the game uses to manage time.
 *
 * Systems integrated:
 * - TimeStateManager: Dual timeline (historical + lived)
 * - ReliefConditionEvaluator: Condition-based rotations
 * - PerceptionModulationEngine: Subjective time experience
 * - RestPeriodNarrativeSystem: Narrative beats during rest
 * - EnvironmentalTimeSignalSystem: Environmental time communication
 *
 * Philosophy: The historical timeline is a fixed rail.
 * The player's experience is a variable camera moving along that rail.
 */

import {
    TimeStateManager,
    ShiftType,
    TimePhase,
    CampaignPhase,
    TimeState
} from './TimeStateManager';

import {
    ReliefConditionEvaluator,
    ReliefConditions,
    SurvivalMeters,
    InjuryState,
    UnitState,
    createMockMeters,
    createMockInjury,
    createMockUnitState
} from './ReliefConditionEvaluator';

import {
    PerceptionModulationEngine,
    PerceptionModifiers,
    StressLevel
} from './PerceptionModulationEngine';

import {
    RestPeriodNarrativeSystem,
    RestPeriodConfig,
    RestPeriodSchedule,
    NarrativeBeat
} from './RestPeriodNarrativeSystem';

import {
    EnvironmentalTimeSignalSystem,
    LightingState,
    WeatherState,
    AudioState,
    NPCBehaviorState
} from './EnvironmentalTimeSignalSystem';

// ============================================================================
// UNIFIED TIME ENGINE
// ============================================================================

export class VerdunTimeEngine {
    // Core systems
    private timeManager: TimeStateManager;
    private reliefEvaluator: ReliefConditionEvaluator;
    private perceptionEngine: PerceptionModulationEngine;
    private restNarrative: RestPeriodNarrativeSystem;
    private environmentSignals: EnvironmentalTimeSignalSystem;

    // Current state
    private isRunning: boolean = false;
    private isPaused: boolean = false;

    // External state (provided by game)
    private survivalMeters: SurvivalMeters;
    private injuryState: InjuryState;
    private unitState: UnitState;

    // Event callbacks
    private onReliefEligibleCallbacks: Array<(conditions: ReliefConditions) => void> = [];
    private onPhaseChangeCallbacks: Array<(newPhase: TimePhase) => void> = [];
    private onCampaignPhaseChangeCallbacks: Array<(newPhase: CampaignPhase) => void> = [];

    constructor() {
        // Initialize all subsystems
        this.timeManager = new TimeStateManager();
        this.reliefEvaluator = new ReliefConditionEvaluator();
        this.perceptionEngine = new PerceptionModulationEngine();
        this.restNarrative = new RestPeriodNarrativeSystem();
        this.environmentSignals = new EnvironmentalTimeSignalSystem();

        // Initialize external state with defaults
        this.survivalMeters = createMockMeters();
        this.injuryState = createMockInjury();
        this.unitState = createMockUnitState();

        // Wire up cross-system events
        this.setupEventHandlers();
    }

    // ========================================================================
    // INITIALIZATION
    // ========================================================================

    private setupEventHandlers(): void {
        // Time manager events
        this.timeManager.onPhaseTransition((newPhase, oldPhase) => {
            console.log(`[TimeEngine] Phase: ${oldPhase} → ${newPhase}`);
            this.onPhaseChangeCallbacks.forEach(cb => cb(newPhase));
        });

        this.timeManager.onCampaignPhaseChange((newPhase) => {
            console.log(`[TimeEngine] Campaign phase: ${newPhase}`);
            this.onCampaignPhaseChangeCallbacks.forEach(cb => cb(newPhase));
        });

        // Relief evaluator events
        this.reliefEvaluator.onReliefEligible((conditions) => {
            console.log(`[TimeEngine] Relief eligible: ${conditions.reason}`);
            this.onReliefEligibleCallbacks.forEach(cb => cb(conditions));
        });

        this.reliefEvaluator.onReliefMandatory((conditions) => {
            console.log(`[TimeEngine] Relief MANDATORY: ${conditions.reason}`);
            // Mandatory relief triggers automatic transition
            this.initiateRelief();
        });

        // Perception engine events
        this.perceptionEngine.onStressLevelChange((newLevel, oldLevel) => {
            console.log(`[TimeEngine] Stress: ${StressLevel[oldLevel]} → ${StressLevel[newLevel]}`);
        });
    }

    // ========================================================================
    // MAIN UPDATE LOOP
    // ========================================================================

    /**
     * Main update function - call every frame from game loop
     */
    public update(deltaTime: number): void {
        if (!this.isRunning || this.isPaused) {
            return;
        }

        // 1. Update time manager (historical + lived time)
        this.timeManager.update(deltaTime);

        // 2. Update perception engine (stress/fear effects)
        this.perceptionEngine.update(deltaTime);

        // 3. Evaluate relief conditions (condition-based rotation)
        const reliefConditions = this.reliefEvaluator.evaluate(
            this.survivalMeters,
            this.injuryState,
            this.unitState,
            this.timeManager.getShiftDuration(),
            this.timeManager.getCurrentShift(),
            deltaTime
        );

        // 4. Update environmental signals (light, weather, audio, NPCs)
        this.environmentSignals.update(
            this.timeManager.getCurrentPhase(),
            this.timeManager.getPhaseProgressNormalized(),
            this.timeManager.getHistoricalDate()
        );

        // 5. Apply perception modifiers to time scale
        // (Stress makes time feel different, but doesn't change historical clock)
        const perceptionMod = this.perceptionEngine.getModifiers();
        // Note: We DON'T modify timeManager's time scale based on perception
        // Perception is purely experiential (animation, audio, visuals)
        // Historical clock always ticks at constant rate
    }

    // ========================================================================
    // GAME STATE UPDATES
    // ========================================================================

    /**
     * Update survival meters (called by game systems)
     */
    public updateSurvivalMeters(meters: Partial<SurvivalMeters>): void {
        this.survivalMeters = { ...this.survivalMeters, ...meters };
    }

    /**
     * Update injury state (called by game systems)
     */
    public updateInjuryState(injury: Partial<InjuryState>): void {
        this.injuryState = { ...this.injuryState, ...injury };
    }

    /**
     * Update unit state (called by game systems)
     */
    public updateUnitState(unit: Partial<UnitState>): void {
        this.unitState = { ...this.unitState, ...unit };
    }

    // ========================================================================
    // STRESS & PERCEPTION
    // ========================================================================

    /**
     * Add stress source (e.g., artillery near-miss)
     */
    public addStress(id: string, name: string, intensity: number, decayRate: number, duration: number = -1): void {
        this.perceptionEngine.addStressSource(id, name, intensity, decayRate, duration);
    }

    /**
     * Convenience methods for common stress events
     */
    public artilleryNearMiss(): void {
        this.perceptionEngine.artilleryNearMiss();
    }

    public friendlyDeath(): void {
        this.perceptionEngine.friendlyDeath();
    }

    public shellShockEvent(): void {
        this.perceptionEngine.shellShockEvent();
    }

    public enterBombardment(): void {
        this.perceptionEngine.enterBombardment();
    }

    public exitBombardment(): void {
        this.perceptionEngine.exitBombardment();
    }

    public enterNoMansLand(): void {
        this.perceptionEngine.enterNoMansLand();
    }

    public exitNoMansLand(): void {
        this.perceptionEngine.exitNoMansLand();
    }

    // ========================================================================
    // ROTATION & RELIEF
    // ========================================================================

    /**
     * Initiate relief (transition from front line to rest)
     */
    public initiateRelief(): void {
        console.log('[TimeEngine] Initiating relief sequence');

        const currentShift = this.timeManager.getCurrentShift();

        if (currentShift === ShiftType.FRONT_LINE || currentShift === ShiftType.SUPPORT_LINE) {
            // Transition to march (relief march out)
            this.timeManager.transitionShift(ShiftType.MARCH);

            // Clear stress (relief!)
            this.perceptionEngine.clearAllStressSources();

            // Complete rotation count
            this.timeManager.completeRotation();

            // After march duration, transition to rest
            // (In real game, this would be triggered by march completion event)
        }
    }

    /**
     * Begin rest period
     */
    public beginRestPeriod(totalDays: number): RestPeriodSchedule {
        console.log(`[TimeEngine] Beginning rest period: ${totalDays} days`);

        // Transition to rest shift
        this.timeManager.transitionShift(ShiftType.REST_AREA);

        // Initialize rest narrative
        const config: RestPeriodConfig = {
            totalDays,
            campaignPhase: this.timeManager.getCampaignPhase(),
            daysSurvived: this.timeManager.getDaysSurvived(),
            isWounded: this.injuryState.isWounded,
            previousStoryFlags: this.restNarrative.getStoryFlags()
        };

        return this.restNarrative.beginRestPeriod(config);
    }

    /**
     * Start next narrative beat during rest
     */
    public startNextRestBeat(): NarrativeBeat | null {
        return this.restNarrative.startNextBeat();
    }

    /**
     * Complete current narrative beat
     */
    public completeRestBeat(beat: NarrativeBeat, choiceId?: string): void {
        this.restNarrative.completeBeat(beat, choiceId);
    }

    /**
     * Return to front line (end rest period)
     */
    public returnToFrontLine(): void {
        console.log('[TimeEngine] Returning to front line');

        // Apply meter restoration
        const restoration = this.restNarrative.calculateMeterRestoration(
            7,  // Days rested (would be calculated from schedule)
            this.injuryState.isWounded
        );

        this.updateSurvivalMeters({
            hunger: Math.min(100, this.survivalMeters.hunger + restoration.hunger),
            thirst: Math.min(100, this.survivalMeters.thirst + restoration.thirst),
            stamina: restoration.stamina,
            warmth: Math.min(100, this.survivalMeters.warmth + restoration.warmth),
            hygiene: Math.min(100, this.survivalMeters.hygiene + restoration.hygiene),
            morale: Math.min(100, this.survivalMeters.morale + restoration.morale),
            alertness: Math.min(100, this.survivalMeters.alertness + restoration.alertness)
        });

        // Transition to march (return march)
        this.timeManager.transitionShift(ShiftType.MARCH);

        // After march, transition back to front line
        // (In real game, triggered by march completion)
    }

    /**
     * Complete return march and arrive at front line
     */
    public arriveAtFrontLine(): void {
        console.log('[TimeEngine] Arrived at front line');

        // Transition to front line
        this.timeManager.transitionShift(ShiftType.FRONT_LINE);

        // Reset to dawn (rotations typically arrive at dawn)
        // This is handled automatically by TimeStateManager.transitionShift()
    }

    // ========================================================================
    // RELIEF AVAILABILITY
    // ========================================================================

    /**
     * Signal that relief column has arrived (scheduled rotation)
     */
    public setReliefColumnArrived(arrived: boolean): void {
        this.reliefEvaluator.setReliefColumnArrived(arrived);
    }

    /**
     * Set replacement unit availability
     */
    public setReplacementAvailable(available: boolean): void {
        this.reliefEvaluator.setReplacementAvailable(available);
    }

    // ========================================================================
    // GETTERS
    // ========================================================================

    public getTimeState(): Readonly<TimeState> {
        return this.timeManager.getState();
    }

    public getHistoricalDate(): Date {
        return this.timeManager.getHistoricalDate();
    }

    public getCurrentPhase(): TimePhase {
        return this.timeManager.getCurrentPhase();
    }

    public getCampaignPhase(): CampaignPhase {
        return this.timeManager.getCampaignPhase();
    }

    public getDaysSurvived(): number {
        return this.timeManager.getDaysSurvived();
    }

    public getPerceptionModifiers(): Readonly<PerceptionModifiers> {
        return this.perceptionEngine.getModifiers();
    }

    public getLightingState(): Readonly<LightingState> {
        return this.environmentSignals.getLightingState();
    }

    public getWeatherState(): Readonly<WeatherState> {
        return this.environmentSignals.getWeatherState();
    }

    public getAudioState(): Readonly<AudioState> {
        return this.environmentSignals.getAudioState();
    }

    public getNPCBehaviorState(): Readonly<NPCBehaviorState> {
        return this.environmentSignals.getNPCBehaviorState();
    }

    /**
     * Get random NPC dialogue for current time
     */
    public getNPCDialogue(type: 'greeting' | 'observation' | 'timeReference'): string {
        return this.environmentSignals.getRandomDialogue(type);
    }

    // ========================================================================
    // CONTROL
    // ========================================================================

    public start(): void {
        this.isRunning = true;
        console.log('[TimeEngine] Started');
    }

    public pause(): void {
        this.isPaused = true;
        console.log('[TimeEngine] Paused');
    }

    public resume(): void {
        this.isPaused = false;
        console.log('[TimeEngine] Resumed');
    }

    public stop(): void {
        this.isRunning = false;
        console.log('[TimeEngine] Stopped');
    }

    // ========================================================================
    // EVENT REGISTRATION
    // ========================================================================

    public onReliefEligible(callback: (conditions: ReliefConditions) => void): void {
        this.onReliefEligibleCallbacks.push(callback);
    }

    public onPhaseChange(callback: (newPhase: TimePhase) => void): void {
        this.onPhaseChangeCallbacks.push(callback);
    }

    public onCampaignPhaseChange(callback: (newPhase: CampaignPhase) => void): void {
        this.onCampaignPhaseChangeCallbacks.push(callback);
    }

    // ========================================================================
    // DEBUG
    // ========================================================================

    public getDebugInfo(): string {
        return `
${'='.repeat(60)}
VERDUN TIME ENGINE - DEBUG OVERVIEW
${'='.repeat(60)}

${this.timeManager.getDebugInfo()}

${this.perceptionEngine.getDebugInfo()}

${this.environmentSignals.getDebugInfo()}

${this.restNarrative.getDebugInfo()}

${'='.repeat(60)}
        `.trim();
    }

    public printDebug(): void {
        console.log(this.getDebugInfo());
    }
}

// ============================================================================
// SINGLETON INSTANCE (Recommended)
// ============================================================================

let _engineInstance: VerdunTimeEngine | null = null;

export function getVerdunTimeEngine(): VerdunTimeEngine {
    if (!_engineInstance) {
        _engineInstance = new VerdunTimeEngine();
    }
    return _engineInstance;
}

export function resetVerdunTimeEngine(): void {
    _engineInstance = null;
}

// ============================================================================
// EXPORTS
// ============================================================================

export {
    // Core engine
    VerdunTimeEngine,

    // Types from subsystems
    ShiftType,
    TimePhase,
    CampaignPhase,
    TimeState,
    SurvivalMeters,
    InjuryState,
    UnitState,
    ReliefConditions,
    PerceptionModifiers,
    StressLevel,
    LightingState,
    WeatherState,
    AudioState,
    NPCBehaviorState,
    RestPeriodSchedule,
    NarrativeBeat
};
