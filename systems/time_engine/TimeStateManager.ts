/**
 * TIME STATE MANAGER
 *
 * Core system managing dual timelines (historical calendar vs lived experience)
 * and time progression through the 303-day Verdun campaign.
 *
 * Philosophy: Time is felt, not measured. This system tracks both real historical
 * time and subjective player experience time.
 */

// ============================================================================
// ENUMS & TYPES
// ============================================================================

export enum ShiftType {
    FRONT_LINE = "FRONT_LINE",
    SUPPORT_LINE = "SUPPORT_LINE",
    REST_AREA = "REST_AREA",
    MARCH = "MARCH"
}

export enum TimePhase {
    DAWN = "DAWN",
    DAYLIGHT = "DAYLIGHT",
    DUSK = "DUSK",
    NIGHT = "NIGHT"
}

export enum CampaignPhase {
    PHASE_I_CHAOS = "PHASE_I_CHAOS",                    // Feb 21 - Mar 6
    PHASE_II_LEFT_BANK = "PHASE_II_LEFT_BANK",          // Mar 6 - Apr 9
    PHASE_III_ATTRITION = "PHASE_III_ATTRITION",        // Apr 9 - May 7
    PHASE_IV_VAUX = "PHASE_IV_VAUX",                    // May 7 - Jun 23
    PHASE_V_STALEMATE = "PHASE_V_STALEMATE",            // Jun 23 - Oct 24
    PHASE_VI_DOUAUMONT = "PHASE_VI_DOUAUMONT",          // Oct 24 - Nov 2
    PHASE_VII_RECAPTURE = "PHASE_VII_RECAPTURE"         // Nov 2 - Dec 18
}

export interface CampaignPhaseData {
    phase: CampaignPhase;
    startDate: Date;
    endDate: Date;
    historicalContext: string;
    intensity: number;  // 0.0-1.0
    averageRotationDays: number;
}

export interface TimeState {
    // Historical Timeline
    historicalDate: Date;
    campaignPhase: CampaignPhase;
    daysSurvived: number;
    rotationsCompleted: number;

    // Current Shift State
    currentShift: ShiftType;
    shiftStartTime: number;  // Game time in seconds
    shiftPhase: TimePhase;
    phaseStartTime: number;  // Game time in seconds

    // Time Progression
    gameTime: number;  // Total game time in seconds
    gameTimeScale: number;  // Base time multiplier (1.0 = real-time)
    perceptionModifier: number;  // Subjective time (0.5-2.0)

    // Shift Duration Tracking
    currentShiftDuration: number;  // Seconds in current shift
    targetShiftDuration: number;  // Target duration before relief eligible
}

// ============================================================================
// CAMPAIGN PHASE DEFINITIONS
// ============================================================================

export const CAMPAIGN_PHASES: CampaignPhaseData[] = [
    {
        phase: CampaignPhase.PHASE_I_CHAOS,
        startDate: new Date(1916, 1, 21),  // Feb 21, 1916
        endDate: new Date(1916, 2, 6),     // Mar 6, 1916
        historicalContext: "Initial German assault. Operation Gericht begins. Chaos and desperate defense.",
        intensity: 0.95,
        averageRotationDays: 3
    },
    {
        phase: CampaignPhase.PHASE_II_LEFT_BANK,
        startDate: new Date(1916, 2, 6),   // Mar 6, 1916
        endDate: new Date(1916, 3, 9),     // Apr 9, 1916
        historicalContext: "Offensive on Mort-Homme and Hill 304. Left bank grinding.",
        intensity: 0.85,
        averageRotationDays: 5
    },
    {
        phase: CampaignPhase.PHASE_III_ATTRITION,
        startDate: new Date(1916, 3, 9),   // Apr 9, 1916
        endDate: new Date(1916, 4, 7),     // May 7, 1916
        historicalContext: "Renewed attacks on forts. Attrition warfare solidifies.",
        intensity: 0.75,
        averageRotationDays: 6
    },
    {
        phase: CampaignPhase.PHASE_IV_VAUX,
        startDate: new Date(1916, 4, 7),   // May 7, 1916
        endDate: new Date(1916, 5, 23),    // Jun 23, 1916
        historicalContext: "Fort Vaux siege (Jun 1-7). High-water mark at Souville. CLIMAX.",
        intensity: 1.0,
        averageRotationDays: 5
    },
    {
        phase: CampaignPhase.PHASE_V_STALEMATE,
        startDate: new Date(1916, 5, 23),  // Jun 23, 1916
        endDate: new Date(1916, 9, 24),    // Oct 24, 1916
        historicalContext: "Stalemate. Somme offensive drains German forces. Lower intensity.",
        intensity: 0.5,
        averageRotationDays: 7
    },
    {
        phase: CampaignPhase.PHASE_VI_DOUAUMONT,
        startDate: new Date(1916, 9, 24),  // Oct 24, 1916
        endDate: new Date(1916, 10, 2),    // Nov 2, 1916
        historicalContext: "French counteroffensive. Douaumont recapture. CLIMAX.",
        intensity: 0.95,
        averageRotationDays: 4
    },
    {
        phase: CampaignPhase.PHASE_VII_RECAPTURE,
        startDate: new Date(1916, 10, 2),  // Nov 2, 1916
        endDate: new Date(1916, 11, 18),   // Dec 18, 1916
        historicalContext: "French gains consolidated. Victory push. Cautious hope.",
        intensity: 0.7,
        averageRotationDays: 5
    }
];

// ============================================================================
// TIME CONSTANTS
// ============================================================================

export const TIME_CONSTANTS = {
    // Phase durations in real-time seconds
    DAWN_DURATION: { min: 300, max: 900 },      // 5-15 minutes
    DAYLIGHT_DURATION: { min: 900, max: 2700 }, // 15-45 minutes
    DUSK_DURATION: { min: 300, max: 900 },      // 5-15 minutes
    NIGHT_DURATION: { min: 1200, max: 3600 },   // 20-60 minutes

    // One complete day duration
    MIN_DAY_DURATION: 2700,  // 45 minutes
    MAX_DAY_DURATION: 5400,  // 90 minutes

    // Shift duration targets (front line)
    MIN_SHIFT_DURATION: 2700,   // 1 day (45 min)
    MAX_SHIFT_DURATION: 16200,  // 3 days (270 min)

    // Calendar time
    CAMPAIGN_START: new Date(1916, 1, 21),  // Feb 21, 1916
    CAMPAIGN_END: new Date(1916, 11, 18),   // Dec 18, 1916
    TOTAL_CAMPAIGN_DAYS: 303,

    // Time scale factors
    BASE_TIME_SCALE: 1.0,  // Real-time
    PERCEPTION_MIN: 0.5,   // Time feels slower
    PERCEPTION_MAX: 2.0    // Time feels faster
};

// ============================================================================
// TIME STATE MANAGER CLASS
// ============================================================================

export class TimeStateManager {
    private state: TimeState;
    private phaseData: Map<CampaignPhase, CampaignPhaseData>;

    // Event callbacks (to be registered by game systems)
    private onPhaseTransitionCallbacks: Array<(newPhase: TimePhase, oldPhase: TimePhase) => void> = [];
    private onDayCompleteCallbacks: Array<(dayNumber: number) => void> = [];
    private onCampaignPhaseChangeCallbacks: Array<(newPhase: CampaignPhase) => void> = [];

    constructor() {
        this.state = this.initializeState();
        this.phaseData = this.buildPhaseDataMap();
    }

    // ========================================================================
    // INITIALIZATION
    // ========================================================================

    private initializeState(): TimeState {
        return {
            // Historical timeline starts at Feb 21, 1916
            historicalDate: new Date(TIME_CONSTANTS.CAMPAIGN_START),
            campaignPhase: CampaignPhase.PHASE_I_CHAOS,
            daysSurvived: 0,
            rotationsCompleted: 0,

            // Start at front line, dawn
            currentShift: ShiftType.FRONT_LINE,
            shiftStartTime: 0,
            shiftPhase: TimePhase.DAWN,
            phaseStartTime: 0,

            // Time progression
            gameTime: 0,
            gameTimeScale: TIME_CONSTANTS.BASE_TIME_SCALE,
            perceptionModifier: 1.0,

            // Shift tracking
            currentShiftDuration: 0,
            targetShiftDuration: this.calculateTargetShiftDuration(
                ShiftType.FRONT_LINE,
                CampaignPhase.PHASE_I_CHAOS
            )
        };
    }

    private buildPhaseDataMap(): Map<CampaignPhase, CampaignPhaseData> {
        const map = new Map<CampaignPhase, CampaignPhaseData>();
        CAMPAIGN_PHASES.forEach(phase => {
            map.set(phase.phase, phase);
        });
        return map;
    }

    // ========================================================================
    // MAIN UPDATE LOOP
    // ========================================================================

    /**
     * Main update function - call every frame
     * @param deltaTime Real-world seconds since last frame
     */
    public update(deltaTime: number): void {
        // Apply time scale and perception modifier
        const effectiveTime = deltaTime * this.state.gameTimeScale * this.state.perceptionModifier;

        // Advance game time
        this.state.gameTime += effectiveTime;
        this.state.currentShiftDuration += effectiveTime;

        // Check for phase transitions (dawn -> daylight -> dusk -> night)
        this.checkPhaseTransition();

        // Update historical date if calendar day passed
        this.updateHistoricalDate(effectiveTime);

        // Check for campaign phase changes
        this.checkCampaignPhaseTransition();
    }

    // ========================================================================
    // PHASE MANAGEMENT
    // ========================================================================

    private checkPhaseTransition(): void {
        const phaseDuration = this.getCurrentPhaseDuration();
        const timeInPhase = this.state.gameTime - this.state.phaseStartTime;

        if (timeInPhase >= phaseDuration) {
            const oldPhase = this.state.shiftPhase;
            this.transitionToNextPhase();

            // Notify listeners
            this.onPhaseTransitionCallbacks.forEach(callback => {
                callback(this.state.shiftPhase, oldPhase);
            });
        }
    }

    private transitionToNextPhase(): void {
        const oldPhase = this.state.shiftPhase;

        switch (this.state.shiftPhase) {
            case TimePhase.DAWN:
                this.state.shiftPhase = TimePhase.DAYLIGHT;
                break;
            case TimePhase.DAYLIGHT:
                this.state.shiftPhase = TimePhase.DUSK;
                break;
            case TimePhase.DUSK:
                this.state.shiftPhase = TimePhase.NIGHT;
                break;
            case TimePhase.NIGHT:
                this.state.shiftPhase = TimePhase.DAWN;
                this.onDayComplete();
                break;
        }

        this.state.phaseStartTime = this.state.gameTime;

        console.log(`[TimeEngine] Phase transition: ${oldPhase} → ${this.state.shiftPhase}`);
    }

    private onDayComplete(): void {
        this.state.daysSurvived++;

        console.log(`[TimeEngine] Day ${this.state.daysSurvived} complete`);

        // Notify listeners
        this.onDayCompleteCallbacks.forEach(callback => {
            callback(this.state.daysSurvived);
        });
    }

    private getCurrentPhaseDuration(): number {
        const phase = this.state.shiftPhase;
        const campaignPhase = this.getCurrentCampaignPhaseData();
        const intensity = campaignPhase.intensity;

        // Higher intensity = longer tense phases (dawn/dusk), shorter daylight
        switch (phase) {
            case TimePhase.DAWN:
                return this.lerp(
                    TIME_CONSTANTS.DAWN_DURATION.min,
                    TIME_CONSTANTS.DAWN_DURATION.max,
                    intensity
                );

            case TimePhase.DAYLIGHT:
                return this.lerp(
                    TIME_CONSTANTS.DAYLIGHT_DURATION.max,
                    TIME_CONSTANTS.DAYLIGHT_DURATION.min,
                    intensity  // Inverted: high intensity = shorter day
                );

            case TimePhase.DUSK:
                return this.lerp(
                    TIME_CONSTANTS.DUSK_DURATION.min,
                    TIME_CONSTANTS.DUSK_DURATION.max,
                    intensity
                );

            case TimePhase.NIGHT:
                return this.lerp(
                    TIME_CONSTANTS.NIGHT_DURATION.min,
                    TIME_CONSTANTS.NIGHT_DURATION.max,
                    intensity
                );

            default:
                return 1800;  // 30 min default
        }
    }

    // ========================================================================
    // HISTORICAL DATE TRACKING
    // ========================================================================

    private updateHistoricalDate(deltaTime: number): void {
        // One game day advances one calendar day
        // Track fractional days
        const dayFraction = deltaTime / this.getTotalDayDuration();

        // Advance historical date
        const millisecondsPerDay = 24 * 60 * 60 * 1000;
        this.state.historicalDate = new Date(
            this.state.historicalDate.getTime() + (dayFraction * millisecondsPerDay)
        );
    }

    private getTotalDayDuration(): number {
        // Sum of all phase durations = one complete day
        return (
            this.getCurrentPhaseDuration() // This is dynamic, but approximate
        );
    }

    // ========================================================================
    // CAMPAIGN PHASE MANAGEMENT
    // ========================================================================

    private checkCampaignPhaseTransition(): void {
        const currentPhaseData = this.getCurrentCampaignPhaseData();

        // Check if we've passed the end date of current phase
        if (this.state.historicalDate >= currentPhaseData.endDate) {
            const nextPhase = this.getNextCampaignPhase();
            if (nextPhase) {
                this.state.campaignPhase = nextPhase;

                console.log(`[TimeEngine] Campaign phase transition: ${nextPhase}`);

                // Notify listeners
                this.onCampaignPhaseChangeCallbacks.forEach(callback => {
                    callback(nextPhase);
                });

                // Recalculate target shift duration for new phase
                this.state.targetShiftDuration = this.calculateTargetShiftDuration(
                    this.state.currentShift,
                    nextPhase
                );
            }
        }
    }

    private getNextCampaignPhase(): CampaignPhase | null {
        const phases = Object.values(CampaignPhase);
        const currentIndex = phases.indexOf(this.state.campaignPhase);

        if (currentIndex < phases.length - 1) {
            return phases[currentIndex + 1];
        }

        return null;  // Campaign complete
    }

    private getCurrentCampaignPhaseData(): CampaignPhaseData {
        return this.phaseData.get(this.state.campaignPhase)!;
    }

    // ========================================================================
    // SHIFT MANAGEMENT
    // ========================================================================

    /**
     * Transition to a new shift type (front line, support, rest, march)
     */
    public transitionShift(newShift: ShiftType): void {
        const oldShift = this.state.currentShift;
        this.state.currentShift = newShift;
        this.state.shiftStartTime = this.state.gameTime;
        this.state.currentShiftDuration = 0;

        // Recalculate target shift duration
        this.state.targetShiftDuration = this.calculateTargetShiftDuration(
            newShift,
            this.state.campaignPhase
        );

        console.log(`[TimeEngine] Shift transition: ${oldShift} → ${newShift}`);

        // If transitioning to rest, reset to dawn
        if (newShift === ShiftType.REST_AREA) {
            this.state.shiftPhase = TimePhase.DAYLIGHT;
            this.state.phaseStartTime = this.state.gameTime;
        }

        // If transitioning to march, fix at daylight or night (depending on context)
        if (newShift === ShiftType.MARCH) {
            // Marches usually happen at night (safer)
            this.state.shiftPhase = TimePhase.NIGHT;
            this.state.phaseStartTime = this.state.gameTime;
        }

        // If returning to front line, start at dawn (historically accurate)
        if (newShift === ShiftType.FRONT_LINE && oldShift === ShiftType.MARCH) {
            this.state.shiftPhase = TimePhase.DAWN;
            this.state.phaseStartTime = this.state.gameTime;
        }
    }

    /**
     * Calculate target shift duration based on shift type and campaign phase
     */
    private calculateTargetShiftDuration(shift: ShiftType, phase: CampaignPhase): number {
        const phaseData = this.phaseData.get(phase)!;

        switch (shift) {
            case ShiftType.FRONT_LINE:
                // Front line: 1-3 days (45-270 min)
                // Duration varies by campaign phase intensity
                const baseDuration = phaseData.averageRotationDays * TIME_CONSTANTS.MIN_DAY_DURATION;
                return Math.min(baseDuration, TIME_CONSTANTS.MAX_SHIFT_DURATION);

            case ShiftType.SUPPORT_LINE:
                // Support line: Similar to front line
                return phaseData.averageRotationDays * TIME_CONSTANTS.MIN_DAY_DURATION;

            case ShiftType.REST_AREA:
                // Rest: Handled by narrative system, not time-based
                return Infinity;

            case ShiftType.MARCH:
                // March: 20-40 minutes
                return this.lerp(1200, 2400, phaseData.intensity);

            default:
                return TIME_CONSTANTS.MIN_SHIFT_DURATION;
        }
    }

    /**
     * Check if player has served minimum time for relief eligibility
     */
    public isMinimumTimeServed(): boolean {
        return this.state.currentShiftDuration >= TIME_CONSTANTS.MIN_SHIFT_DURATION;
    }

    /**
     * Check if player has exceeded maximum time (mandatory relief)
     */
    public isMandatoryReliefTime(): boolean {
        return this.state.currentShiftDuration >= TIME_CONSTANTS.MAX_SHIFT_DURATION;
    }

    /**
     * Get progress toward relief eligibility (0.0-1.0)
     */
    public getReliefProgress(): number {
        return Math.min(
            1.0,
            this.state.currentShiftDuration / this.state.targetShiftDuration
        );
    }

    // ========================================================================
    // PERCEPTION MODULATION
    // ========================================================================

    /**
     * Modify time perception (0.5 = time feels slower, 2.0 = time feels faster)
     */
    public setPerceptionModifier(modifier: number): void {
        this.state.perceptionModifier = Math.max(
            TIME_CONSTANTS.PERCEPTION_MIN,
            Math.min(TIME_CONSTANTS.PERCEPTION_MAX, modifier)
        );
    }

    /**
     * Get current time perception modifier
     */
    public getPerceptionModifier(): number {
        return this.state.perceptionModifier;
    }

    /**
     * Reset perception to normal (1.0)
     */
    public resetPerception(): void {
        this.state.perceptionModifier = 1.0;
    }

    // ========================================================================
    // TIME SKIP (REST PERIODS)
    // ========================================================================

    /**
     * Skip forward in calendar time (used during rest periods)
     * @param days Number of calendar days to skip
     */
    public skipTime(days: number): void {
        const millisecondsPerDay = 24 * 60 * 60 * 1000;
        this.state.historicalDate = new Date(
            this.state.historicalDate.getTime() + (days * millisecondsPerDay)
        );

        this.state.daysSurvived += days;

        console.log(`[TimeEngine] Time skip: +${days} days (now ${this.formatDate(this.state.historicalDate)})`);

        // Check for campaign phase transition after skip
        this.checkCampaignPhaseTransition();
    }

    // ========================================================================
    // EVENT REGISTRATION
    // ========================================================================

    public onPhaseTransition(callback: (newPhase: TimePhase, oldPhase: TimePhase) => void): void {
        this.onPhaseTransitionCallbacks.push(callback);
    }

    public onDayComplete(callback: (dayNumber: number) => void): void {
        this.onDayCompleteCallbacks.push(callback);
    }

    public onCampaignPhaseChange(callback: (newPhase: CampaignPhase) => void): void {
        this.onCampaignPhaseChangeCallbacks.push(callback);
    }

    // ========================================================================
    // GETTERS
    // ========================================================================

    public getState(): Readonly<TimeState> {
        return { ...this.state };
    }

    public getHistoricalDate(): Date {
        return new Date(this.state.historicalDate);
    }

    public getCampaignPhase(): CampaignPhase {
        return this.state.campaignPhase;
    }

    public getCurrentShift(): ShiftType {
        return this.state.currentShift;
    }

    public getCurrentPhase(): TimePhase {
        return this.state.shiftPhase;
    }

    public getDaysSurvived(): number {
        return this.state.daysSurvived;
    }

    public getRotationsCompleted(): number {
        return this.state.rotationsCompleted;
    }

    public getShiftDuration(): number {
        return this.state.currentShiftDuration;
    }

    public getPhaseProgress(): number {
        const phaseDuration = this.getCurrentPhaseDuration();
        const timeInPhase = this.state.gameTime - this.state.phaseStartTime;
        return Math.min(1.0, timeInPhase / phaseDuration);
    }

    /**
     * Get phase progress as normalized value (0.0-1.0)
     * Used by environmental systems for gradual transitions
     */
    public getPhaseProgressNormalized(): number {
        return this.getPhaseProgress();
    }

    // ========================================================================
    // ROTATION TRACKING
    // ========================================================================

    public completeRotation(): void {
        this.state.rotationsCompleted++;
        console.log(`[TimeEngine] Rotation #${this.state.rotationsCompleted} complete`);
    }

    // ========================================================================
    // UTILITY
    // ========================================================================

    private lerp(a: number, b: number, t: number): number {
        return a + (b - a) * t;
    }

    private formatDate(date: Date): string {
        const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
        return `${months[date.getMonth()]} ${date.getDate()}, ${date.getFullYear()}`;
    }

    // ========================================================================
    // DEBUG
    // ========================================================================

    public getDebugInfo(): string {
        const state = this.state;
        const phaseData = this.getCurrentCampaignPhaseData();

        return `
=== VERDUN TIME ENGINE DEBUG ===
Historical Date: ${this.formatDate(state.historicalDate)}
Campaign Phase: ${state.campaignPhase}
Phase Context: ${phaseData.historicalContext}
Days Survived: ${state.daysSurvived} / ${TIME_CONSTANTS.TOTAL_CAMPAIGN_DAYS}
Rotations: ${state.rotationsCompleted}

Current Shift: ${state.currentShift}
Shift Duration: ${Math.floor(state.currentShiftDuration / 60)} minutes
Relief Progress: ${(this.getReliefProgress() * 100).toFixed(1)}%

Time Phase: ${state.shiftPhase}
Phase Progress: ${(this.getPhaseProgress() * 100).toFixed(1)}%

Time Scale: ${state.gameTimeScale}x
Perception: ${state.perceptionModifier}x
Effective Time: ${(state.gameTimeScale * state.perceptionModifier).toFixed(2)}x
================================
        `.trim();
    }
}

// ============================================================================
// SINGLETON INSTANCE (Optional)
// ============================================================================

let _timeManagerInstance: TimeStateManager | null = null;

export function getTimeManager(): TimeStateManager {
    if (!_timeManagerInstance) {
        _timeManagerInstance = new TimeStateManager();
    }
    return _timeManagerInstance;
}

export function resetTimeManager(): void {
    _timeManagerInstance = null;
}
