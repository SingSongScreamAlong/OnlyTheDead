// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/**
 * Enum for survival meter types
 */
UENUM(BlueprintType)
enum class ESurvivalMeterType : uint8
{
	Hunger		UMETA(DisplayName = "Hunger"),
	Thirst		UMETA(DisplayName = "Thirst"),
	Stamina		UMETA(DisplayName = "Stamina"),
	Warmth		UMETA(DisplayName = "Warmth"),
	Hygiene		UMETA(DisplayName = "Hygiene"),
	Morale		UMETA(DisplayName = "Morale"),
	Alertness	UMETA(DisplayName = "Alertness")
};

/**
 * Structure for individual survival meter data
 */
USTRUCT(BlueprintType)
struct FSurvivalMeter
{
	GENERATED_BODY()

	/** Current value of the meter (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float CurrentValue = 100.0f;

	/** Maximum value of the meter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxValue = 100.0f;

	/** Minimum value of the meter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MinValue = 0.0f;

	/** Rate at which the meter depletes per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float DepletionRate = 0.0f;

	/** Rate at which the meter regenerates per second (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float RegenerationRate = 0.0f;

	/** Critical threshold below which negative effects start (percentage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float CriticalThreshold = 25.0f;

	/** Time remaining until death when meter is at 0 (in seconds, -1 for no death) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float DeathTimer = -1.0f;

	/** Current death countdown if meter is at 0 */
	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float CurrentDeathCountdown = -1.0f;

	/** Whether this meter is currently in critical state */
	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	bool bIsCritical = false;

	/** Whether this meter is currently at zero and counting down to death */
	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	bool bIsCountingDownToDeath = false;
};

/**
 * Component that manages all survival meters for a character
 * Handles Hunger, Thirst, Stamina, Warmth, Hygiene, Morale, and Alertness
 * Based on exact specifications from ONLY_THE_DEAD_DEVELOPMENT_BOOK.md
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API USurvivalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivalComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== SURVIVAL METERS =====

	/** Hunger meter - depletes over time, death after 30 days at 0 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Meters")
	FSurvivalMeter HungerMeter;

	/** Thirst meter - depletes faster than hunger, death after 3 days at 0 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Meters")
	FSurvivalMeter ThirstMeter;

	/** Stamina meter - regenerates when resting, depleted by actions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Meters")
	FSurvivalMeter StaminaMeter;

	/** Warmth meter - affected by weather and clothing, hypothermia when critical */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Meters")
	FSurvivalMeter WarmthMeter;

	/** Hygiene meter - affects disease risk, trench foot, infections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Meters")
	FSurvivalMeter HygieneMeter;

	/** Morale meter - psychological state, shell shock risk when low */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Meters")
	FSurvivalMeter MoraleMeter;

	/** Alertness meter - sleep deprivation, hallucinations when critical */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Meters")
	FSurvivalMeter AlertnessMeter;

	// ===== METER MANIPULATION FUNCTIONS =====

	/** Modify a specific survival meter by a delta value */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void ModifyMeter(ESurvivalMeterType MeterType, float DeltaValue);

	/** Set a specific survival meter to a value */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void SetMeter(ESurvivalMeterType MeterType, float NewValue);

	/** Get the current value of a specific survival meter */
	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetMeterValue(ESurvivalMeterType MeterType) const;

	/** Get the percentage of a specific survival meter (0-100) */
	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetMeterPercentage(ESurvivalMeterType MeterType) const;

	/** Check if a specific meter is in critical state */
	UFUNCTION(BlueprintPure, Category = "Survival")
	bool IsMeterCritical(ESurvivalMeterType MeterType) const;

	/** Check if a specific meter is counting down to death */
	UFUNCTION(BlueprintPure, Category = "Survival")
	bool IsMeterCountingToDeath(ESurvivalMeterType MeterType) const;

	/** Get the death countdown timer for a specific meter */
	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetDeathCountdown(ESurvivalMeterType MeterType) const;

	// ===== SPECIFIC ACTIONS =====

	/** Consume food to restore hunger */
	UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
	void ConsumeFood(float FoodValue);

	/** Drink water to restore thirst */
	UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
	void DrinkWater(float WaterValue);

	/** Rest to restore stamina and alertness */
	UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
	void Rest(float RestDuration);

	/** Sleep to fully restore alertness */
	UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
	void Sleep(float SleepDuration);

	/** Perform an action that costs stamina */
	UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
	bool ConsumeStamina(float StaminaCost);

	/** Check if player has enough stamina for an action */
	UFUNCTION(BlueprintPure, Category = "Survival|Actions")
	bool HasStamina(float RequiredStamina) const;

	// ===== DELEGATES =====

	/** Delegate fired when any meter becomes critical */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeterCritical, ESurvivalMeterType, MeterType);
	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnMeterCritical OnMeterCritical;

	/** Delegate fired when any meter leaves critical state */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeterRecovered, ESurvivalMeterType, MeterType);
	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnMeterRecovered OnMeterRecovered;

	/** Delegate fired when death countdown starts */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathCountdownStart, ESurvivalMeterType, MeterType, float, TimeRemaining);
	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnDeathCountdownStart OnDeathCountdownStart;

	/** Delegate fired when player dies from a survival meter reaching 0 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalDeath, ESurvivalMeterType, MeterType);
	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalDeath OnSurvivalDeath;

private:
	/** Initialize all survival meters with default values based on design doc */
	void InitializeMeters();

	/** Update a specific meter over time */
	void UpdateMeter(FSurvivalMeter& Meter, ESurvivalMeterType MeterType, float DeltaTime);

	/** Get pointer to meter by type */
	FSurvivalMeter* GetMeterByType(ESurvivalMeterType MeterType);
	const FSurvivalMeter* GetMeterByType(ESurvivalMeterType MeterType) const;

	/** Handle death from survival meter depletion */
	void HandleSurvivalDeath(ESurvivalMeterType MeterType);
};
