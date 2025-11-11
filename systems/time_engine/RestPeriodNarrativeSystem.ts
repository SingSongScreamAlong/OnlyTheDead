/**
 * REST PERIOD NARRATIVE SYSTEM
 *
 * Handles rest periods through narrative beats, not gameplay.
 * Players don't "play" rest - they experience key moments, then time skips forward.
 *
 * Philosophy: Rest is recovery narrative, not gameplay loop.
 * The historical timeline advances (7-14 days), but player only experiences
 * 5-6 narrative beats that show the psychological and physical toll.
 *
 * Key moments:
 * - The First Night (trauma doesn't stop)
 * - The Letter (connection to home)
 * - The Mirror (physical change)
 * - The Training (dread of return)
 * - The Order (resignation)
 */

import { CampaignPhase } from './TimeStateManager';

// ============================================================================
// TYPES & INTERFACES
// ============================================================================

export enum NarrativeBeatType {
    FIRST_NIGHT = "FIRST_NIGHT",
    THE_LETTER = "THE_LETTER",
    THE_MIRROR = "THE_MIRROR",
    THE_TRAINING = "THE_TRAINING",
    THE_ORDER = "THE_ORDER",
    MEDICAL_TREATMENT = "MEDICAL_TREATMENT",
    CONVERSATION = "CONVERSATION",
    RUMOR_MILL = "RUMOR_MILL"
}

export interface NarrativeBeat {
    id: string;
    type: NarrativeBeatType;
    name: string;
    description: string;

    // Timing
    triggerDay: number;           // Which rest day (1-14)
    duration: number;             // Real-time duration in seconds (180-600s = 3-10min)

    // Scene data (for game to render)
    sceneLocation: string;
    requiredNPCs: string[];
    environmentState: string;     // "night", "day", "evening", etc.

    // Player choices (optional)
    choices: PlayerChoice[];

    // Effects on player state
    effects: NarrativeBeatEffects;

    // Conditions (when does this beat trigger?)
    conditions: BeatConditions;
}

export interface PlayerChoice {
    id: string;
    text: string;
    consequence: ChoiceConsequence;
}

export interface ChoiceConsequence {
    moraleChange: number;
    storyFlag?: string;
    dialogue?: string;
}

export interface NarrativeBeatEffects {
    // Survival meter changes
    moraleChange: number;
    alertnessChange: number;
    hungerChange: number;
    thirstChange: number;

    // Story progression
    storyFlags: string[];

    // Character changes
    physicalChanges?: PhysicalChange[];
}

export interface PhysicalChange {
    type: "weight_loss" | "hollow_eyes" | "beard_growth" | "tremors" | "scars";
    severity: number;  // 0.0-1.0
}

export interface BeatConditions {
    // Required conditions for this beat to trigger
    minDaysSurvived?: number;
    maxDaysSurvived?: number;
    requiredStoryFlags?: string[];
    excludedStoryFlags?: string[];
    campaignPhase?: CampaignPhase[];
    isWounded?: boolean;
}

export interface RestPeriodConfig {
    totalDays: number;            // 7-14 calendar days
    campaignPhase: CampaignPhase;
    daysSurvived: number;
    isWounded: boolean;
    previousStoryFlags: string[];
}

export interface RestPeriodSchedule {
    totalDays: number;
    beats: NarrativeBeat[];
    timeSkips: TimeSkip[];
}

export interface TimeSkip {
    beforeBeatId: string;         // Skip occurs before this beat
    daysToSkip: number;
    description: string;          // "7 days pass. You sleep, eat, try to forget."
}

export interface MeterRestoration {
    hunger: number;
    thirst: number;
    stamina: number;
    warmth: number;
    hygiene: number;
    morale: number;
    alertness: number;
}

// ============================================================================
// NARRATIVE BEAT DEFINITIONS
// ============================================================================

export const NARRATIVE_BEAT_TEMPLATES: Record<NarrativeBeatType, Omit<NarrativeBeat, 'id'>> = {
    [NarrativeBeatType.FIRST_NIGHT]: {
        type: NarrativeBeatType.FIRST_NIGHT,
        name: "The First Night",
        description: "You collapse into sleep. But the artillery never stops. Not in your head.",
        triggerDay: 1,
        duration: 300,  // 5 minutes
        sceneLocation: "rest_barracks",
        requiredNPCs: ["sleeping_soldiers"],
        environmentState: "night",
        choices: [],
        effects: {
            moraleChange: -5,
            alertnessChange: 10,
            hungerChange: 0,
            thirstChange: 0,
            storyFlags: ["experienced_rest_nightmares"],
            physicalChanges: [
                { type: "tremors", severity: 0.3 }
            ]
        },
        conditions: {}
    },

    [NarrativeBeatType.THE_LETTER]: {
        type: NarrativeBeatType.THE_LETTER,
        name: "The Letter",
        description: "Mail call. A letter from home. Do you open it?",
        triggerDay: 2,
        duration: 180,  // 3 minutes
        sceneLocation: "rest_courtyard",
        requiredNPCs: ["mail_clerk", "other_soldiers"],
        environmentState: "day",
        choices: [
            {
                id: "read_letter",
                text: "Read the letter",
                consequence: {
                    moraleChange: 15,
                    storyFlag: "read_letter_home",
                    dialogue: "Your family is well. They miss you. They pray for you. It hurts."
                }
            },
            {
                id: "save_letter",
                text: "Save it for later",
                consequence: {
                    moraleChange: 5,
                    storyFlag: "saved_letter",
                    dialogue: "Not now. You're not ready to think about home."
                }
            },
            {
                id: "burn_letter",
                text: "Burn it without reading",
                consequence: {
                    moraleChange: -10,
                    storyFlag: "burned_letter",
                    dialogue: "The smoke rises. You can't be both here and there. Choose one."
                }
            }
        ],
        effects: {
            moraleChange: 0,  // Determined by choice
            alertnessChange: 0,
            hungerChange: 0,
            thirstChange: 0,
            storyFlags: []  // Set by choice
        },
        conditions: {}
    },

    [NarrativeBeatType.THE_MIRROR]: {
        type: NarrativeBeatType.THE_MIRROR,
        name: "The Mirror",
        description: "You walk past a broken mirror. You stop. You don't recognize the face looking back.",
        triggerDay: 4,
        duration: 120,  // 2 minutes
        sceneLocation: "rest_washroom",
        requiredNPCs: [],
        environmentState: "day",
        choices: [],
        effects: {
            moraleChange: -10,
            alertnessChange: 0,
            hungerChange: 0,
            thirstChange: 0,
            storyFlags: ["saw_reflection"],
            physicalChanges: [
                { type: "weight_loss", severity: 0.4 },
                { type: "hollow_eyes", severity: 0.6 },
                { type: "beard_growth", severity: 0.7 }
            ]
        },
        conditions: {
            minDaysSurvived: 20  // Only after you've been at Verdun a while
        }
    },

    [NarrativeBeatType.THE_TRAINING]: {
        type: NarrativeBeatType.THE_TRAINING,
        name: "The Training",
        description: "Formation drill. Practice attack. You know what this means. You're going back soon.",
        triggerDay: 6,
        duration: 360,  // 6 minutes
        sceneLocation: "rest_training_ground",
        requiredNPCs: ["officer_training", "squad_members"],
        environmentState: "day",
        choices: [],
        effects: {
            moraleChange: -15,
            alertnessChange: 5,
            hungerChange: -10,
            thirstChange: -10,
            storyFlags: ["training_completed"],
            physicalChanges: []
        },
        conditions: {}
    },

    [NarrativeBeatType.THE_ORDER]: {
        type: NarrativeBeatType.THE_ORDER,
        name: "The Order",
        description: "Assembly. The officer reads the orders. You're returning to the line. Tonight.",
        triggerDay: 7,
        duration: 240,  // 4 minutes
        sceneLocation: "rest_assembly_area",
        requiredNPCs: ["commanding_officer", "full_unit"],
        environmentState: "evening",
        choices: [],
        effects: {
            moraleChange: -20,
            alertnessChange: 15,
            hungerChange: 0,
            thirstChange: 0,
            storyFlags: ["received_return_orders"],
            physicalChanges: []
        },
        conditions: {}
    },

    [NarrativeBeatType.MEDICAL_TREATMENT]: {
        type: NarrativeBeatType.MEDICAL_TREATMENT,
        name: "Medical Treatment",
        description: "The aid station. They patch you up. You're not healed, but you're combat-ready. That's enough.",
        triggerDay: 1,
        duration: 300,  // 5 minutes
        sceneLocation: "rest_aid_station",
        requiredNPCs: ["medic", "wounded_soldiers"],
        environmentState: "day",
        choices: [],
        effects: {
            moraleChange: 5,
            alertnessChange: -5,
            hungerChange: 0,
            thirstChange: 0,
            storyFlags: ["received_medical_treatment"],
            physicalChanges: [
                { type: "scars", severity: 0.5 }
            ]
        },
        conditions: {
            isWounded: true
        }
    },

    [NarrativeBeatType.CONVERSATION]: {
        type: NarrativeBeatType.CONVERSATION,
        name: "Conversation",
        description: "You sit with your comrades. Some talk. Some don't. The silence says more.",
        triggerDay: 3,
        duration: 240,  // 4 minutes
        sceneLocation: "rest_canteen",
        requiredNPCs: ["squad_members"],
        environmentState: "evening",
        choices: [
            {
                id: "talk",
                text: "Join the conversation",
                consequence: {
                    moraleChange: 10,
                    dialogue: "You share stories. Some laugh. For a moment, it feels almost normal."
                }
            },
            {
                id: "silent",
                text: "Stay silent",
                consequence: {
                    moraleChange: -5,
                    dialogue: "You have nothing to say. The words won't come."
                }
            }
        ],
        effects: {
            moraleChange: 0,  // Determined by choice
            alertnessChange: 0,
            hungerChange: 0,
            thirstChange: 0,
            storyFlags: []
        },
        conditions: {}
    },

    [NarrativeBeatType.RUMOR_MILL]: {
        type: NarrativeBeatType.RUMOR_MILL,
        name: "Rumors",
        description: "You overhear the rumors. Where you're going. What happened while you were away. Who died.",
        triggerDay: 5,
        duration: 180,  // 3 minutes
        sceneLocation: "rest_canteen",
        requiredNPCs: ["soldiers_gossiping"],
        environmentState: "evening",
        choices: [],
        effects: {
            moraleChange: -10,
            alertnessChange: 5,
            hungerChange: 0,
            thirstChange: 0,
            storyFlags: ["heard_rumors"],
            physicalChanges: []
        },
        conditions: {}
    }
};

// ============================================================================
// REST PERIOD NARRATIVE SYSTEM CLASS
// ============================================================================

export class RestPeriodNarrativeSystem {
    private currentSchedule: RestPeriodSchedule | null = null;
    private currentBeatIndex: number = 0;
    private storyFlags: Set<string> = new Set();

    // Event callbacks
    private onBeatStartCallbacks: Array<(beat: NarrativeBeat) => void> = [];
    private onBeatCompleteCallbacks: Array<(beat: NarrativeBeat) => void> = [];
    private onTimeSkipCallbacks: Array<(skip: TimeSkip) => void> = [];
    private onRestPeriodCompleteCallbacks: Array<() => void> = [];

    constructor() {}

    // ========================================================================
    // REST PERIOD INITIALIZATION
    // ========================================================================

    /**
     * Begin a new rest period
     */
    public beginRestPeriod(config: RestPeriodConfig): RestPeriodSchedule {
        console.log(`[RestNarrative] Beginning rest period: ${config.totalDays} days`);

        // Load previous story flags
        this.storyFlags = new Set(config.previousStoryFlags);

        // Generate schedule
        this.currentSchedule = this.generateSchedule(config);
        this.currentBeatIndex = 0;

        return this.currentSchedule;
    }

    /**
     * Generate a rest period schedule (beats + time skips)
     */
    private generateSchedule(config: RestPeriodConfig): RestPeriodSchedule {
        const beats: NarrativeBeat[] = [];

        // Determine which beats to include based on conditions
        Object.values(NARRATIVE_BEAT_TEMPLATES).forEach(template => {
            if (this.shouldIncludeBeat(template, config)) {
                const beat: NarrativeBeat = {
                    id: `${template.type}_${Date.now()}`,
                    ...template
                };
                beats.push(beat);
            }
        });

        // Sort beats by trigger day
        beats.sort((a, b) => a.triggerDay - b.triggerDay);

        // Generate time skips between beats
        const timeSkips: TimeSkip[] = this.generateTimeSkips(beats, config.totalDays);

        return {
            totalDays: config.totalDays,
            beats,
            timeSkips
        };
    }

    private shouldIncludeBeat(
        template: Omit<NarrativeBeat, 'id'>,
        config: RestPeriodConfig
    ): boolean {
        const conditions = template.conditions;

        // Check days survived
        if (conditions.minDaysSurvived && config.daysSurvived < conditions.minDaysSurvived) {
            return false;
        }
        if (conditions.maxDaysSurvived && config.daysSurvived > conditions.maxDaysSurvived) {
            return false;
        }

        // Check campaign phase
        if (conditions.campaignPhase && !conditions.campaignPhase.includes(config.campaignPhase)) {
            return false;
        }

        // Check wounded status
        if (conditions.isWounded !== undefined && conditions.isWounded !== config.isWounded) {
            return false;
        }

        // Check required story flags
        if (conditions.requiredStoryFlags) {
            for (const flag of conditions.requiredStoryFlags) {
                if (!this.storyFlags.has(flag)) {
                    return false;
                }
            }
        }

        // Check excluded story flags
        if (conditions.excludedStoryFlags) {
            for (const flag of conditions.excludedStoryFlags) {
                if (this.storyFlags.has(flag)) {
                    return false;
                }
            }
        }

        return true;
    }

    private generateTimeSkips(beats: NarrativeBeat[], totalDays: number): TimeSkip[] {
        const skips: TimeSkip[] = [];

        // Calculate days between beats
        let currentDay = 0;

        beats.forEach((beat, index) => {
            const daysSinceLastBeat = beat.triggerDay - currentDay;

            if (daysSinceLastBeat > 0) {
                skips.push({
                    beforeBeatId: beat.id,
                    daysToSkip: daysSinceLastBeat,
                    description: this.generateTimeSkipDescription(daysSinceLastBeat)
                });
            }

            currentDay = beat.triggerDay;
        });

        // Skip remaining days after last beat
        const remainingDays = totalDays - currentDay;
        if (remainingDays > 0) {
            skips.push({
                beforeBeatId: "end",
                daysToSkip: remainingDays,
                description: this.generateTimeSkipDescription(remainingDays)
            });
        }

        return skips;
    }

    private generateTimeSkipDescription(days: number): string {
        const descriptions = [
            `${days} day${days > 1 ? 's' : ''} pass. You sleep, eat, try to forget.`,
            `${days} day${days > 1 ? 's' : ''} blur together. Rest is never enough.`,
            `Time passes. ${days} day${days > 1 ? 's' : ''}. But the trenches are always waiting.`,
            `${days} day${days > 1 ? 's' : ''}. You almost feel human again. Almost.`
        ];

        return descriptions[Math.floor(Math.random() * descriptions.length)];
    }

    // ========================================================================
    // NARRATIVE BEAT PROGRESSION
    // ========================================================================

    /**
     * Start the next narrative beat
     * Returns null if no more beats
     */
    public startNextBeat(): NarrativeBeat | null {
        if (!this.currentSchedule) {
            console.warn("[RestNarrative] No active rest period");
            return null;
        }

        if (this.currentBeatIndex >= this.currentSchedule.beats.length) {
            // Rest period complete
            this.completeRestPeriod();
            return null;
        }

        const beat = this.currentSchedule.beats[this.currentBeatIndex];

        // Check for time skip before this beat
        const timeSkip = this.currentSchedule.timeSkips.find(skip => skip.beforeBeatId === beat.id);
        if (timeSkip) {
            this.triggerTimeSkip(timeSkip);
        }

        console.log(`[RestNarrative] Starting beat: ${beat.name}`);

        // Trigger callbacks
        this.onBeatStartCallbacks.forEach(callback => callback(beat));

        return beat;
    }

    /**
     * Complete the current narrative beat
     */
    public completeBeat(beat: NarrativeBeat, choiceId?: string): void {
        console.log(`[RestNarrative] Completing beat: ${beat.name}`);

        // Apply effects
        this.applyBeatEffects(beat, choiceId);

        // Trigger callbacks
        this.onBeatCompleteCallbacks.forEach(callback => callback(beat));

        // Move to next beat
        this.currentBeatIndex++;
    }

    private applyBeatEffects(beat: NarrativeBeat, choiceId?: string): void {
        // Apply base effects
        const effects = beat.effects;

        // Add story flags
        effects.storyFlags.forEach(flag => this.storyFlags.add(flag));

        // If choice made, apply choice effects
        if (choiceId) {
            const choice = beat.choices.find(c => c.id === choiceId);
            if (choice && choice.consequence.storyFlag) {
                this.storyFlags.add(choice.consequence.storyFlag);
            }
        }

        console.log(`[RestNarrative] Applied effects: Morale ${effects.moraleChange >= 0 ? '+' : ''}${effects.moraleChange}`);
    }

    private triggerTimeSkip(skip: TimeSkip): void {
        console.log(`[RestNarrative] Time skip: ${skip.daysToSkip} days`);

        // Trigger callbacks
        this.onTimeSkipCallbacks.forEach(callback => callback(skip));
    }

    private completeRestPeriod(): void {
        console.log(`[RestNarrative] Rest period complete`);

        // Check for final time skip
        if (this.currentSchedule) {
            const finalSkip = this.currentSchedule.timeSkips.find(skip => skip.beforeBeatId === "end");
            if (finalSkip) {
                this.triggerTimeSkip(finalSkip);
            }
        }

        // Trigger callbacks
        this.onRestPeriodCompleteCallbacks.forEach(callback => callback());

        // Clear schedule
        this.currentSchedule = null;
        this.currentBeatIndex = 0;
    }

    // ========================================================================
    // METER RESTORATION
    // ========================================================================

    /**
     * Calculate meter restoration during rest period
     */
    public calculateMeterRestoration(
        daysSkipped: number,
        isWounded: boolean
    ): MeterRestoration {
        // Rest restores meters, but not completely
        // Wounded soldiers recover more slowly

        const woundedPenalty = isWounded ? 0.7 : 1.0;

        return {
            hunger: Math.min(100, 30 + (daysSkipped * 8)) * woundedPenalty,
            thirst: Math.min(100, 40 + (daysSkipped * 9)) * woundedPenalty,
            stamina: 100,  // Stamina fully restores
            warmth: Math.min(100, 50 + (daysSkipped * 6)),
            hygiene: Math.min(100, 40 + (daysSkipped * 4)),  // Lice always return
            morale: Math.min(100, 20 + (daysSkipped * 5)) * woundedPenalty,  // Never fully recovers
            alertness: Math.min(100, 40 + (daysSkipped * 4)) * woundedPenalty  // Nightmares prevent full rest
        };
    }

    // ========================================================================
    // GETTERS
    // ========================================================================

    public getCurrentSchedule(): RestPeriodSchedule | null {
        return this.currentSchedule;
    }

    public getCurrentBeatIndex(): number {
        return this.currentBeatIndex;
    }

    public getStoryFlags(): string[] {
        return Array.from(this.storyFlags);
    }

    public hasStoryFlag(flag: string): boolean {
        return this.storyFlags.has(flag);
    }

    // ========================================================================
    // EVENT REGISTRATION
    // ========================================================================

    public onBeatStart(callback: (beat: NarrativeBeat) => void): void {
        this.onBeatStartCallbacks.push(callback);
    }

    public onBeatComplete(callback: (beat: NarrativeBeat) => void): void {
        this.onBeatCompleteCallbacks.push(callback);
    }

    public onTimeSkip(callback: (skip: TimeSkip) => void): void {
        this.onTimeSkipCallbacks.push(callback);
    }

    public onRestPeriodComplete(callback: () => void): void {
        this.onRestPeriodCompleteCallbacks.push(callback);
    }

    // ========================================================================
    // DEBUG
    // ========================================================================

    public getDebugInfo(): string {
        if (!this.currentSchedule) {
            return "=== REST PERIOD NARRATIVE DEBUG ===\nNo active rest period\n===================================";
        }

        const beatList = this.currentSchedule.beats.map((beat, index) => {
            const status = index < this.currentBeatIndex ? '✓' : index === this.currentBeatIndex ? '→' : ' ';
            return `  ${status} Day ${beat.triggerDay}: ${beat.name}`;
        }).join('\n');

        return `
=== REST PERIOD NARRATIVE DEBUG ===
Total Days: ${this.currentSchedule.totalDays}
Current Beat: ${this.currentBeatIndex + 1} / ${this.currentSchedule.beats.length}
Story Flags: ${this.storyFlags.size}

BEAT SCHEDULE:
${beatList}

ACTIVE FLAGS:
${Array.from(this.storyFlags).map(f => `  - ${f}`).join('\n')}
====================================
        `.trim();
    }
}

// ============================================================================
// SINGLETON INSTANCE (Optional)
// ============================================================================

let _restNarrativeInstance: RestPeriodNarrativeSystem | null = null;

export function getRestNarrativeSystem(): RestPeriodNarrativeSystem {
    if (!_restNarrativeInstance) {
        _restNarrativeInstance = new RestPeriodNarrativeSystem();
    }
    return _restNarrativeInstance;
}

export function resetRestNarrativeSystem(): void {
    _restNarrativeInstance = null;
}
