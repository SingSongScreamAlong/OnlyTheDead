// Only The Dead - Survival System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "SurvivalSystem.generated.h"

/**
 * Core survival system managing all 7 survival meters
 * Handles hunger, thirst, stamina, warmth, hygiene, morale, and alertness
 * Based on historical survival data from Battle of Verdun
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API USurvivalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // SURVIVAL METERS
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Meters")
    TMap<ESurvivalMeterType, FSurvivalMeterState> SurvivalMeters;

    // ========================================================================
    // PUBLIC API
    // ========================================================================

    /** Initialize all survival meters to starting values */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void InitializeSurvivalMeters();

    /** Get current value of a specific meter */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetMeterValue(ESurvivalMeterType MeterType) const;

    /** Set meter value (clamped 0-100) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetMeterValue(ESurvivalMeterType MeterType, float NewValue);

    /** Modify meter by delta amount */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyMeter(ESurvivalMeterType MeterType, float DeltaValue);

    /** Check if any meter is in critical state (< 25) */
    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsAnyCritical() const;

    /** Check if player should die from any meter */
    UFUNCTION(BlueprintPure, Category = "Survival")
    bool ShouldPlayerDie(FString& OutCauseOfDeath) const;

    // ========================================================================
    // HUNGER SYSTEM
    // ========================================================================

    /** Consume food item */
    UFUNCTION(BlueprintCallable, Category = "Survival|Hunger")
    void ConsumeFood(FName FoodItemID);

    /** Get hunger stage text */
    UFUNCTION(BlueprintPure, Category = "Survival|Hunger")
    FText GetHungerStageText() const;

    /** Calculate hunger depletion rate based on activity */
    UFUNCTION(BlueprintPure, Category = "Survival|Hunger")
    float CalculateHungerDepletionRate(bool bIsActive, bool bIsCombat) const;

    // ========================================================================
    // THIRST SYSTEM (CRITICAL - Most important survival meter)
    // ========================================================================

    /** Drink water */
    UFUNCTION(BlueprintCallable, Category = "Survival|Thirst")
    void DrinkWater(float AmountML, bool bIsContaminated = false);

    /** Get thirst stage text */
    UFUNCTION(BlueprintPure, Category = "Survival|Thirst")
    FText GetThirstStageText() const;

    /** Calculate thirst depletion rate based on conditions */
    UFUNCTION(BlueprintPure, Category = "Survival|Thirst")
    float CalculateThirstDepletionRate(bool bIsActive, float Temperature) const;

    /** Check if player is at risk of death from dehydration */
    UFUNCTION(BlueprintPure, Category = "Survival|Thirst")
    bool IsDehydrationCritical() const;

    // ========================================================================
    // STAMINA SYSTEM
    // ========================================================================

    /** Deplete stamina from action */
    UFUNCTION(BlueprintCallable, Category = "Survival|Stamina")
    void DepleteStamina(float Amount);

    /** Regenerate stamina over time */
    UFUNCTION(BlueprintCallable, Category = "Survival|Stamina")
    void RegenerateStamina(float DeltaTime, bool bIsResting);

    /** Check if player can perform action */
    UFUNCTION(BlueprintPure, Category = "Survival|Stamina")
    bool HasStaminaForAction(float RequiredStamina) const;

    // ========================================================================
    // WARMTH SYSTEM
    // ========================================================================

    /** Update warmth based on environmental conditions */
    UFUNCTION(BlueprintCallable, Category = "Survival|Warmth")
    void UpdateWarmth(float DeltaTime, float AmbientTemperature, bool bIsSheltered, bool bIsWet);

    /** Get frostbite stage */
    UFUNCTION(BlueprintPure, Category = "Survival|Warmth")
    int32 GetFrostbiteStage() const;

    /** Check if player is at risk of hypothermia death */
    UFUNCTION(BlueprintPure, Category = "Survival|Warmth")
    bool IsHypothermiaCritical() const;

    // ========================================================================
    // HYGIENE SYSTEM
    // ========================================================================

    /** Update hygiene (degrades slowly over time) */
    UFUNCTION(BlueprintCallable, Category = "Survival|Hygiene")
    void UpdateHygiene(float DeltaTime, bool bInMud, bool bHasLice);

    /** Perform hygiene activity (washing, delousing) */
    UFUNCTION(BlueprintCallable, Category = "Survival|Hygiene")
    void PerformHygieneActivity(FName ActivityID);

    /** Get disease risk level */
    UFUNCTION(BlueprintPure, Category = "Survival|Hygiene")
    float GetDiseaseRisk() const;

    /** Roll for disease contraction */
    UFUNCTION(BlueprintCallable, Category = "Survival|Hygiene")
    bool RollForDisease(float DeltaTime);

    // ========================================================================
    // MORALE SYSTEM
    // ========================================================================

    /** Modify morale from event */
    UFUNCTION(BlueprintCallable, Category = "Survival|Morale")
    void ModifyMoraleFromEvent(FName EventID, float BaseMoraleChange);

    /** Check for shell shock breakdown */
    UFUNCTION(BlueprintPure, Category = "Survival|Morale")
    bool IsShellShockRisk() const;

    /** Trigger shell shock episode */
    UFUNCTION(BlueprintCallable, Category = "Survival|Morale")
    void TriggerShellShockEpisode();

    /** Get morale stage text */
    UFUNCTION(BlueprintPure, Category = "Survival|Morale")
    FText GetMoraleStageText() const;

    // ========================================================================
    // ALERTNESS SYSTEM
    // ========================================================================

    /** Update alertness based on sleep/wake cycle */
    UFUNCTION(BlueprintCallable, Category = "Survival|Alertness")
    void UpdateAlertness(float DeltaTime, bool bIsSleeping);

    /** Get sleep deprivation stage */
    UFUNCTION(BlueprintPure, Category = "Survival|Alertness")
    int32 GetSleepDeprivationStage() const;

    /** Check for microsleep event */
    UFUNCTION(BlueprintPure, Category = "Survival|Alertness")
    bool ShouldTriggerMicrosleep() const;

    /** Trigger hallucination event */
    UFUNCTION(BlueprintCallable, Category = "Survival|Alertness")
    void TriggerHallucination();

    // ========================================================================
    // CASCADING FAILURE SYSTEM
    // ========================================================================

    /** Check for cascading meter failures (low meters affect others) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyCascadingEffects(float DeltaTime);

    // ========================================================================
    // PERSISTENT WORLD - ENVIRONMENTAL EFFECTS
    // ========================================================================

    /** Update survival meters based on current location's environmental state */
    UFUNCTION(BlueprintCallable, Category = "Survival|PersistentWorld")
    void UpdateEnvironmentalEffects(FVector CurrentLocation, float DeltaTime);

    /** Check if water source at location is contaminated (shell impacts, corpses) */
    UFUNCTION(BlueprintPure, Category = "Survival|PersistentWorld")
    bool IsWaterContaminatedAtLocation(FVector Location) const;

    /** Get disease risk multiplier based on region degradation */
    UFUNCTION(BlueprintPure, Category = "Survival|PersistentWorld")
    float GetDiseaseRiskByLocation(FVector Location) const;

    /** Get shelter quality at location (intact buildings vs craters vs open) */
    UFUNCTION(BlueprintPure, Category = "Survival|PersistentWorld")
    float GetShelterQualityAtLocation(FVector Location) const;

    /** Calculate morale penalty from witnessing degraded environment */
    UFUNCTION(BlueprintPure, Category = "Survival|PersistentWorld")
    float GetMoralePenaltyFromDegradation(FVector Location) const;

    /** Check if location has sufficient cover from environmental hazards */
    UFUNCTION(BlueprintPure, Category = "Survival|PersistentWorld")
    bool HasAdequateCover(FVector Location, float& OutCoverQuality) const;

    /** Get temperature modifier based on shelter and region */
    UFUNCTION(BlueprintPure, Category = "Survival|PersistentWorld")
    float GetTemperatureModifier(FVector Location, bool bIsSheltered) const;

    /** Check if location is in contaminated zone (gas, chemical residue) */
    UFUNCTION(BlueprintPure, Category = "Survival|PersistentWorld")
    bool IsLocationContaminated(FVector Location) const;

    /** Get regional hygiene penalty (mud level affects hygiene degradation) */
    UFUNCTION(BlueprintPure, Category = "Survival|PersistentWorld")
    float GetRegionalHygienePenalty(FVector Location) const;

protected:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    float TimeSinceLastHungerCheck = 0.0f;
    float TimeSinceLastThirstCheck = 0.0f;
    float TimeSinceLastHygieneCheck = 0.0f;
    float TimeSinceLastAlertness Check = 0.0f;

    bool bHasLice = false;
    bool bHasTrenchFoot = false;
    bool bHasDysentery = false;
    bool bHasTrenchFever = false;

    int32 ShellShockEpisodeCount = 0;
    float LastShellShockTime = 0.0f;

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    float ClampMeterValue(float Value) const { return FMath::Clamp(Value, 0.0f, 100.0f); }

    void CheckDeathConditions();
    void UpdateMeterThresholds();
};
