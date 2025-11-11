/**
 * RELIEF CONDITION EVALUATOR
 *
 * Implements condition-based rotation system - rotations are EARNED, not scheduled.
 * Player gets relieved when:
 * - Physical/mental state is degraded
 * - Wounded/injured
 * - Unit state requires relief
 * - Minimum time served + replacement available
 *
 * Philosophy: You don't "wait for rotation" - you survive until relief.
 */

import { ShiftType } from './TimeStateManager';

// ============================================================================
// TYPES & INTERFACES
// ============================================================================

export interface SurvivalMeters {
    hunger: number;      // 0-100
    thirst: number;      // 0-100
    stamina: number;     // 0-100
    warmth: number;      // 0-100
    hygiene: number;     // 0-100
    morale: number;      // 0-100
    alertness: number;   // 0-100
}

export interface InjuryState {
    isWounded: boolean;
    woundSeverity: WoundSeverity;
    isBleeding: boolean;
    isConcussed: boolean;
    isGassed: boolean;
    hasShellShock: boolean;
}

export enum WoundSeverity {
    NONE = "NONE",
    LIGHT = "LIGHT",
    MEDIUM = "MEDIUM",
    SEVERE = "SEVERE",
    CRITICAL = "CRITICAL"
}

export interface UnitState {
    casualtyRate: number;       // 0.0-1.0 (percentage of unit casualties)
    combatEffective: boolean;
    replacementAvailable: boolean;
    strategicRedeployment: boolean;
    majorBattleOngoing: boolean;
}

export interface ReliefConditions {
    // Condition categories
    physicalStateRequiresRelief: boolean;
    injuryStateRequiresRelief: boolean;
    unitStateAllowsRelief: boolean;
    timeServedMeetsMinimum: boolean;
    timeServedExceedsMaximum: boolean;

    // Overall eligibility
    reliefEligible: boolean;
    reliefMandatory: boolean;

    // Reason (for debugging/UI feedback)
    reason: string;
}

export interface ReliefThresholds {
    // Physical state thresholds
    alertness: { critical: number; duration: number };
    morale: { critical: number; duration: number };
    hunger: { critical: number };
    thirst: { critical: number };
    warmth: { critical: number };
    hygiene: { critical: number };

    // Time thresholds (in seconds)
    minimumTimeServed: number;
    maximumTimeServed: number;

    // Unit thresholds
    maxCasualtyRate: number;
}

export interface ReliefAvailability {
    reliefColumnArrived: boolean;
    replacementUnitAvailable: boolean;
    estimatedArrivalTime: number;  // Seconds until relief column arrives (if known)
}

// ============================================================================
// DEFAULT THRESHOLDS
// ============================================================================

export const DEFAULT_RELIEF_THRESHOLDS: ReliefThresholds = {
    // Physical state: Low meter for extended duration = relief required
    alertness: { critical: 20, duration: 7200 },    // <20% for 2+ hours
    morale: { critical: 30, duration: 14400 },      // <30% for 4+ hours
    hunger: { critical: 20 },                        // <20% (starvation risk)
    thirst: { critical: 15 },                        // <15% (dehydration critical)
    warmth: { critical: 25 },                        // <25% (hypothermia risk)
    hygiene: { critical: 20 },                       // <20% (disease risk critical)

    // Time thresholds
    minimumTimeServed: 86400,    // 24 hours (1 day)
    maximumTimeServed: 259200,   // 72 hours (3 days) - MANDATORY

    // Unit threshold
    maxCasualtyRate: 0.4         // 40% casualties = unit combat ineffective
};

// ============================================================================
// RELIEF CONDITION EVALUATOR CLASS
// ============================================================================

export class ReliefConditionEvaluator {
    private thresholds: ReliefThresholds;
    private reliefAvailability: ReliefAvailability;

    // Tracking low-meter duration
    private lowMeterTracking: Map<string, number> = new Map();

    // Event callbacks
    private onReliefEligibleCallbacks: Array<(conditions: ReliefConditions) => void> = [];
    private onReliefMandatoryCallbacks: Array<(conditions: ReliefConditions) => void> = [];

    constructor(thresholds: ReliefThresholds = DEFAULT_RELIEF_THRESHOLDS) {
        this.thresholds = thresholds;
        this.reliefAvailability = {
            reliefColumnArrived: false,
            replacementUnitAvailable: false,
            estimatedArrivalTime: -1
        };
    }

    // ========================================================================
    // MAIN EVALUATION FUNCTION
    // ========================================================================

    /**
     * Evaluate all relief conditions
     * Call this every frame or on significant state changes
     */
    public evaluate(
        meters: SurvivalMeters,
        injury: InjuryState,
        unit: UnitState,
        timeServed: number,  // Seconds in current shift
        shiftType: ShiftType,
        deltaTime: number    // For tracking duration
    ): ReliefConditions {
        // Update low-meter duration tracking
        this.updateMeterTracking(meters, deltaTime);

        // Evaluate each condition category
        const physicalState = this.evaluatePhysicalState(meters, timeServed);
        const injuryState = this.evaluateInjuryState(injury);
        const unitState = this.evaluateUnitState(unit);
        const timeMinimum = this.evaluateMinimumTime(timeServed);
        const timeMandatory = this.evaluateMandatoryTime(timeServed);

        // Determine overall eligibility
        const conditions: ReliefConditions = {
            physicalStateRequiresRelief: physicalState.requires,
            injuryStateRequiresRelief: injuryState.requires,
            unitStateAllowsRelief: unitState.allows,
            timeServedMeetsMinimum: timeMinimum,
            timeServedExceedsMaximum: timeMandatory,

            reliefEligible: false,
            reliefMandatory: false,
            reason: ""
        };

        // ===== MANDATORY RELIEF LOGIC =====
        // Mandatory relief overrides everything except major battle

        if (timeMandatory && !unit.majorBattleOngoing) {
            conditions.reliefMandatory = true;
            conditions.reliefEligible = true;
            conditions.reason = "Mandatory relief: 72+ hours on front line. Unit must be relieved.";
            this.triggerMandatoryRelief(conditions);
            return conditions;
        }

        if (injuryState.requires) {
            conditions.reliefMandatory = true;
            conditions.reliefEligible = true;
            conditions.reason = `Mandatory relief: ${injuryState.reason}`;
            this.triggerMandatoryRelief(conditions);
            return conditions;
        }

        if (unit.casualtyRate >= this.thresholds.maxCasualtyRate) {
            conditions.reliefMandatory = true;
            conditions.reliefEligible = true;
            conditions.reason = `Mandatory relief: Unit casualties ${(unit.casualtyRate * 100).toFixed(0)}% (combat ineffective)`;
            this.triggerMandatoryRelief(conditions);
            return conditions;
        }

        // ===== ELIGIBLE RELIEF LOGIC =====
        // Player can be relieved, but not mandatory

        // Physical state degraded + minimum time + replacement available
        if (physicalState.requires && timeMinimum && this.reliefAvailability.replacementUnitAvailable) {
            conditions.reliefEligible = true;
            conditions.reason = `Relief eligible: ${physicalState.reason}. Replacement unit available.`;
            this.triggerEligibleRelief(conditions);
            return conditions;
        }

        // Scheduled rotation (relief column arrived)
        if (this.reliefAvailability.reliefColumnArrived && timeMinimum) {
            conditions.reliefEligible = true;
            conditions.reason = "Relief eligible: Scheduled rotation. Relief column arrived.";
            this.triggerEligibleRelief(conditions);
            return conditions;
        }

        // Strategic redeployment
        if (unit.strategicRedeployment) {
            conditions.reliefEligible = true;
            conditions.reason = "Relief eligible: Strategic redeployment ordered.";
            this.triggerEligibleRelief(conditions);
            return conditions;
        }

        // Not eligible
        conditions.reliefEligible = false;
        conditions.reason = this.getNotEligibleReason(conditions, timeServed);

        return conditions;
    }

    // ========================================================================
    // CONDITION EVALUATORS
    // ========================================================================

    private evaluatePhysicalState(
        meters: SurvivalMeters,
        timeServed: number
    ): { requires: boolean; reason: string } {
        const reasons: string[] = [];

        // Check alertness
        const alertnessDuration = this.lowMeterTracking.get('alertness') || 0;
        if (meters.alertness < this.thresholds.alertness.critical &&
            alertnessDuration >= this.thresholds.alertness.duration) {
            reasons.push(`Severe fatigue (alertness ${meters.alertness.toFixed(0)}% for ${(alertnessDuration / 3600).toFixed(1)}h)`);
        }

        // Check morale
        const moraleDuration = this.lowMeterTracking.get('morale') || 0;
        if (meters.morale < this.thresholds.morale.critical &&
            moraleDuration >= this.thresholds.morale.duration) {
            reasons.push(`Severe morale damage (${meters.morale.toFixed(0)}% for ${(moraleDuration / 3600).toFixed(1)}h)`);
        }

        // Check hunger
        if (meters.hunger < this.thresholds.hunger.critical) {
            reasons.push(`Critical hunger (${meters.hunger.toFixed(0)}% - starvation risk)`);
        }

        // Check thirst
        if (meters.thirst < this.thresholds.thirst.critical) {
            reasons.push(`Critical dehydration (${meters.thirst.toFixed(0)}%)`);
        }

        // Check warmth
        if (meters.warmth < this.thresholds.warmth.critical) {
            reasons.push(`Hypothermia risk (warmth ${meters.warmth.toFixed(0)}%)`);
        }

        // Check hygiene
        if (meters.hygiene < this.thresholds.hygiene.critical) {
            reasons.push(`Disease risk critical (hygiene ${meters.hygiene.toFixed(0)}%)`);
        }

        // Check absolute zero (any meter)
        if (meters.hunger <= 0) reasons.push("Collapsed from starvation");
        if (meters.thirst <= 0) reasons.push("Collapsed from dehydration");
        if (meters.stamina <= 0) reasons.push("Collapsed from exhaustion");
        if (meters.warmth <= 0) reasons.push("Hypothermia - unconscious");
        if (meters.alertness <= 0) reasons.push("Collapsed from fatigue");

        const requires = reasons.length > 0;
        const reason = requires ? reasons.join("; ") : "";

        return { requires, reason };
    }

    private evaluateInjuryState(injury: InjuryState): { requires: boolean; reason: string } {
        const reasons: string[] = [];

        if (injury.isWounded) {
            switch (injury.woundSeverity) {
                case WoundSeverity.MEDIUM:
                    reasons.push("Wounded (medium severity)");
                    break;
                case WoundSeverity.SEVERE:
                    reasons.push("Wounded (severe) - immediate evacuation");
                    break;
                case WoundSeverity.CRITICAL:
                    reasons.push("Critical wound - dying");
                    break;
            }
        }

        if (injury.isBleeding) {
            reasons.push("Bleeding (cannot stop)");
        }

        if (injury.isConcussed) {
            reasons.push("Concussed (shell blast)");
        }

        if (injury.isGassed) {
            reasons.push("Gassed (chlorine/phosgene exposure)");
        }

        if (injury.hasShellShock) {
            reasons.push("Shell shock symptoms manifesting");
        }

        // Only MEDIUM+ wounds require relief
        const requires = reasons.length > 0 && (
            injury.woundSeverity === WoundSeverity.MEDIUM ||
            injury.woundSeverity === WoundSeverity.SEVERE ||
            injury.woundSeverity === WoundSeverity.CRITICAL ||
            injury.isBleeding ||
            injury.isConcussed ||
            injury.isGassed ||
            injury.hasShellShock
        );

        const reason = requires ? reasons.join("; ") : "";

        return { requires, reason };
    }

    private evaluateUnitState(unit: UnitState): { allows: boolean; reason: string } {
        if (unit.majorBattleOngoing) {
            return {
                allows: false,
                reason: "Major battle ongoing - no relief available"
            };
        }

        if (unit.strategicRedeployment) {
            return {
                allows: true,
                reason: "Strategic redeployment ordered"
            };
        }

        if (unit.casualtyRate >= this.thresholds.maxCasualtyRate) {
            return {
                allows: true,
                reason: `Unit casualties ${(unit.casualtyRate * 100).toFixed(0)}% - combat ineffective`
            };
        }

        return { allows: true, reason: "" };
    }

    private evaluateMinimumTime(timeServed: number): boolean {
        return timeServed >= this.thresholds.minimumTimeServed;
    }

    private evaluateMandatoryTime(timeServed: number): boolean {
        return timeServed >= this.thresholds.maximumTimeServed;
    }

    // ========================================================================
    // METER TRACKING (Duration Below Threshold)
    // ========================================================================

    private updateMeterTracking(meters: SurvivalMeters, deltaTime: number): void {
        // Track how long each meter has been below critical threshold
        this.trackMeter('alertness', meters.alertness, this.thresholds.alertness.critical, deltaTime);
        this.trackMeter('morale', meters.morale, this.thresholds.morale.critical, deltaTime);
        // Other meters are instant checks, not duration-based
    }

    private trackMeter(name: string, value: number, threshold: number, deltaTime: number): void {
        if (value < threshold) {
            // Increment duration
            const current = this.lowMeterTracking.get(name) || 0;
            this.lowMeterTracking.set(name, current + deltaTime);
        } else {
            // Reset duration
            this.lowMeterTracking.set(name, 0);
        }
    }

    // ========================================================================
    // RELIEF AVAILABILITY
    // ========================================================================

    /**
     * Signal that relief column has arrived (scheduled rotation)
     */
    public setReliefColumnArrived(arrived: boolean): void {
        this.reliefAvailability.reliefColumnArrived = arrived;
        if (arrived) {
            console.log("[ReliefEvaluator] Relief column arrived");
        }
    }

    /**
     * Set whether replacement unit is available for relief
     */
    public setReplacementAvailable(available: boolean): void {
        this.reliefAvailability.replacementUnitAvailable = available;
    }

    /**
     * Set estimated time until relief column arrives (for player awareness)
     */
    public setEstimatedReliefTime(seconds: number): void {
        this.reliefAvailability.estimatedArrivalTime = seconds;
    }

    /**
     * Get relief availability state
     */
    public getReliefAvailability(): Readonly<ReliefAvailability> {
        return { ...this.reliefAvailability };
    }

    // ========================================================================
    // UTILITY
    // ========================================================================

    private getNotEligibleReason(conditions: ReliefConditions, timeServed: number): string {
        const reasons: string[] = [];

        if (!conditions.timeServedMeetsMinimum) {
            const remaining = (this.thresholds.minimumTimeServed - timeServed) / 3600;
            reasons.push(`Minimum time not met (${remaining.toFixed(1)}h remaining)`);
        }

        if (!this.reliefAvailability.replacementUnitAvailable) {
            reasons.push("No replacement unit available");
        }

        if (!this.reliefAvailability.reliefColumnArrived) {
            if (this.reliefAvailability.estimatedArrivalTime > 0) {
                const eta = this.reliefAvailability.estimatedArrivalTime / 3600;
                reasons.push(`Relief column ETA: ${eta.toFixed(1)}h`);
            } else {
                reasons.push("Relief column not arrived");
            }
        }

        return reasons.join("; ");
    }

    // ========================================================================
    // EVENT CALLBACKS
    // ========================================================================

    private triggerEligibleRelief(conditions: ReliefConditions): void {
        console.log(`[ReliefEvaluator] Relief ELIGIBLE: ${conditions.reason}`);

        this.onReliefEligibleCallbacks.forEach(callback => {
            callback(conditions);
        });
    }

    private triggerMandatoryRelief(conditions: ReliefConditions): void {
        console.log(`[ReliefEvaluator] Relief MANDATORY: ${conditions.reason}`);

        this.onReliefMandatoryCallbacks.forEach(callback => {
            callback(conditions);
        });
    }

    public onReliefEligible(callback: (conditions: ReliefConditions) => void): void {
        this.onReliefEligibleCallbacks.push(callback);
    }

    public onReliefMandatory(callback: (conditions: ReliefConditions) => void): void {
        this.onReliefMandatoryCallbacks.push(callback);
    }

    // ========================================================================
    // DEBUG
    // ========================================================================

    public getDebugInfo(conditions: ReliefConditions, timeServed: number): string {
        const low = this.lowMeterTracking;

        return `
=== RELIEF CONDITION EVALUATOR DEBUG ===
Time Served: ${(timeServed / 3600).toFixed(2)}h / ${(this.thresholds.minimumTimeServed / 3600).toFixed(0)}h min

PHYSICAL STATE:
  Alertness: Low for ${((low.get('alertness') || 0) / 3600).toFixed(2)}h
  Morale: Low for ${((low.get('morale') || 0) / 3600).toFixed(2)}h

RELIEF STATUS:
  Eligible: ${conditions.reliefEligible ? 'YES' : 'NO'}
  Mandatory: ${conditions.reliefMandatory ? 'YES' : 'NO'}
  Reason: ${conditions.reason}

RELIEF AVAILABILITY:
  Column Arrived: ${this.reliefAvailability.reliefColumnArrived}
  Replacement Available: ${this.reliefAvailability.replacementUnitAvailable}
  ETA: ${this.reliefAvailability.estimatedArrivalTime > 0 ? (this.reliefAvailability.estimatedArrivalTime / 3600).toFixed(1) + 'h' : 'Unknown'}
========================================
        `.trim();
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Create mock survival meters for testing
 */
export function createMockMeters(overrides?: Partial<SurvivalMeters>): SurvivalMeters {
    return {
        hunger: 80,
        thirst: 80,
        stamina: 80,
        warmth: 80,
        hygiene: 80,
        morale: 80,
        alertness: 80,
        ...overrides
    };
}

/**
 * Create mock injury state for testing
 */
export function createMockInjury(overrides?: Partial<InjuryState>): InjuryState {
    return {
        isWounded: false,
        woundSeverity: WoundSeverity.NONE,
        isBleeding: false,
        isConcussed: false,
        isGassed: false,
        hasShellShock: false,
        ...overrides
    };
}

/**
 * Create mock unit state for testing
 */
export function createMockUnitState(overrides?: Partial<UnitState>): UnitState {
    return {
        casualtyRate: 0.1,  // 10% casualties (normal)
        combatEffective: true,
        replacementAvailable: true,
        strategicRedeployment: false,
        majorBattleOngoing: false,
        ...overrides
    };
}
