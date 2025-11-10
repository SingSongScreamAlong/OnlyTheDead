// Only The Dead - Survival Component
// Tracks all 7 survival meters: Hunger, Thirst, Stamina, Warmth, Hygiene, Morale, Alertness
// Based on exact formulas from the design document

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OTDSurvivalComponent.generated.h"

/**
 * Delegates for survival meter changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivalMeterChanged, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalDeath, FString, Cause);

/**
 * Disease/status effects
 */
UENUM(BlueprintType)
enum class EOTDDisease : uint8
{
	None,
	TrenchFever,
	Dysentery,
	Lice,
	Frostbite,
	ShellShock,
	GasExposure
};

/**
 * Survival Component managing all 7 meters
 * All values range from 0.0 (death/minimum) to 100.0 (full/maximum)
 *
 * Based on specifications from ONLY_THE_DEAD_DEVELOPMENT_BOOK.md:
 * - Hunger: 3-8/hr depletion, death in 7 days
 * - Thirst: 5-12/hr depletion, death in 1-3 days (MORE critical)
 * - Stamina: 2.0/sec base regen, various depletion costs
 * - Warmth: 0.5-3.0/min depletion, hypothermia in 4 hours
 * - Hygiene: 2.0/hr depletion, disease risk
 * - Morale: 0.5/hr base depletion, shell shock at < 30
 * - Alertness: 4.0/hr depletion, collapse after 7 days awake
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UOTDSurvivalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOTDSurvivalComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== Meter Getters ==========

	UFUNCTION(BlueprintPure, Category = "Survival|Meters")
	float GetHunger() const { return Hunger; }

	UFUNCTION(BlueprintPure, Category = "Survival|Meters")
	float GetThirst() const { return Thirst; }

	UFUNCTION(BlueprintPure, Category = "Survival|Meters")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, Category = "Survival|Meters")
	float GetWarmth() const { return Warmth; }

	UFUNCTION(BlueprintPure, Category = "Survival|Meters")
	float GetHygiene() const { return Hygiene; }

	UFUNCTION(BlueprintPure, Category = "Survival|Meters")
	float GetMorale() const { return Morale; }

	UFUNCTION(BlueprintPure, Category = "Survival|Meters")
	float GetAlertness() const { return Alertness; }

	// ========== Meter Modification ==========

	UFUNCTION(BlueprintCallable, Category = "Survival|Meters")
	void RestoreHunger(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Survival|Meters")
	void RestoreThirst(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Survival|Meters")
	void DrainStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Survival|Meters")
	void RestoreStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Survival|Meters")
	void RestoreWarmth(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Survival|Meters")
	void ModifyMorale(float Amount); // Can be positive or negative

	UFUNCTION(BlueprintCallable, Category = "Survival|Meters")
	void RestoreAlertness(float Amount); // Sleep

	// ========== Activity Costs (from design doc) ==========

	/**
	 * Drain stamina for sprinting (15/sec)
	 */
	UFUNCTION(BlueprintCallable, Category = "Survival|Activities")
	void ApplySprintCost(float DeltaTime);

	/**
	 * Drain stamina for melee combat (10/sec)
	 */
	UFUNCTION(BlueprintCallable, Category = "Survival|Activities")
	void ApplyMeleeCost(float DeltaTime);

	/**
	 * Drain stamina for climbing (12/sec)
	 */
	UFUNCTION(BlueprintCallable, Category = "Survival|Activities")
	void ApplyClimbCost(float DeltaTime);

	/**
	 * Drain stamina for carrying wounded (20/sec)
	 */
	UFUNCTION(BlueprintCallable, Category = "Survival|Activities")
	void ApplyCarryWoundedCost(float DeltaTime);

	// ========== Status Effects ==========

	/**
	 * Check if player has a disease
	 */
	UFUNCTION(BlueprintPure, Category = "Survival|Status")
	bool HasDisease(EOTDDisease Disease) const;

	/**
	 * Apply disease/status effect
	 */
	UFUNCTION(BlueprintCallable, Category = "Survival|Status")
	void ApplyDisease(EOTDDisease Disease);

	/**
	 * Remove disease (usually requires medical treatment)
	 */
	UFUNCTION(BlueprintCallable, Category = "Survival|Status")
	void RemoveDisease(EOTDDisease Disease);

	/**
	 * Get movement speed modifier based on all survival meters
	 */
	UFUNCTION(BlueprintPure, Category = "Survival|Modifiers")
	float GetMovementSpeedModifier() const;

	/**
	 * Get aim accuracy modifier (0.0 to 1.0)
	 * Affected by: hunger, stamina, alertness, morale
	 */
	UFUNCTION(BlueprintPure, Category = "Survival|Modifiers")
	float GetAimAccuracyModifier() const;

	// ========== Events ==========

	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalMeterChanged OnHungerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalMeterChanged OnThirstChanged;

	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalMeterChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalMeterChanged OnWarmthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalMeterChanged OnHygieneChanged;

	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalMeterChanged OnMoraleChanged;

	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalMeterChanged OnAlertнessChanged;

	UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
	FOnSurvivalDeath OnDeath;

protected:
	// ========== Survival Meters (0.0 to 100.0) ==========

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Meters")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Meters")
	float Thirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Meters")
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Meters")
	float Warmth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Meters")
	float Hygiene = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Meters")
	float Morale = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Meters")
	float Alertness = 100.0f;

	// ========== Depletion Rates (from design doc) ==========

	/**
	 * Hunger depletion: 3-8/hr depending on activity
	 * Base: 3/hr idle, 8/hr heavy activity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float HungerDepletionRate = 5.0f; // Per hour (average)

	/**
	 * Thirst depletion: 5-12/hr depending on activity
	 * MORE critical than hunger
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float ThirstDepletionRate = 8.0f; // Per hour (average)

	/**
	 * Stamina regeneration: 2.0/sec base
	 * Modified by hunger, thirst, warmth
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float StaminaRegenRate = 2.0f; // Per second

	/**
	 * Warmth depletion: 0.5-3.0/min depending on weather/wet
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float WarmthDepletionRate = 1.5f; // Per minute (average)

	/**
	 * Hygiene depletion: 2.0/hr constant
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float HygieneDepletionRate = 2.0f; // Per hour

	/**
	 * Morale depletion: 0.5/hr base
	 * Heavily modified by events
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float MoraleDepletionRate = 0.5f; // Per hour

	/**
	 * Alertness depletion: 4.0/hr awake, 2x faster at night
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float AlertnessDepletionRate = 4.0f; // Per hour

	// ========== Status Tracking ==========

	UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
	TSet<EOTDDisease> ActiveDiseases;

	UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
	bool bIsAlive = true;

	UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
	float TimeSinceLastSleep = 0.0f; // In hours

	UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
	bool bIsNightTime = false;

	// ========== Internal Methods ==========

	/**
	 * Update all meters based on time passage
	 */
	void UpdateMeters(float DeltaTime);

	/**
	 * Check for death conditions
	 */
	void CheckDeathConditions();

	/**
	 * Check for disease onset from low hygiene
	 */
	void CheckDiseaseProgression();

	/**
	 * Apply cascading effects (low meters affecting others)
	 */
	void ApplyCascadingEffects(float DeltaTime);

	/**
	 * Clamp meter to valid range and broadcast change
	 */
	void ClampAndBroadcast(float& Meter, FOnSurvivalMeterChanged& Delegate);
};
