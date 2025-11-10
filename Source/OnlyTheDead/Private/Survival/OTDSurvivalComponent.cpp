// Only The Dead - Survival Component Implementation

#include "Survival/OTDSurvivalComponent.h"

UOTDSurvivalComponent::UOTDSurvivalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize all meters to full
	Hunger = 100.0f;
	Thirst = 100.0f;
	Stamina = 100.0f;
	Warmth = 100.0f;
	Hygiene = 100.0f;
	Morale = 100.0f;
	Alertness = 100.0f;

	bIsAlive = true;
	TimeSinceLastSleep = 0.0f;
}

void UOTDSurvivalComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOTDSurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsAlive)
	{
		return;
	}

	UpdateMeters(DeltaTime);
	ApplyCascadingEffects(DeltaTime);
	CheckDiseaseProgression();
	CheckDeathConditions();
}

// ========== Meter Modification ==========

void UOTDSurvivalComponent::RestoreHunger(float Amount)
{
	Hunger = FMath::Clamp(Hunger + Amount, 0.0f, 100.0f);
	OnHungerChanged.Broadcast(Hunger, 100.0f);
}

void UOTDSurvivalComponent::RestoreThirst(float Amount)
{
	Thirst = FMath::Clamp(Thirst + Amount, 0.0f, 100.0f);
	OnThirstChanged.Broadcast(Thirst, 100.0f);
}

void UOTDSurvivalComponent::DrainStamina(float Amount)
{
	Stamina = FMath::Clamp(Stamina - Amount, 0.0f, 100.0f);
	OnStaminaChanged.Broadcast(Stamina, 100.0f);
}

void UOTDSurvivalComponent::RestoreStamina(float Amount)
{
	Stamina = FMath::Clamp(Stamina + Amount, 0.0f, 100.0f);
	OnStaminaChanged.Broadcast(Stamina, 100.0f);
}

void UOTDSurvivalComponent::RestoreWarmth(float Amount)
{
	Warmth = FMath::Clamp(Warmth + Amount, 0.0f, 100.0f);
	OnWarmthChanged.Broadcast(Warmth, 100.0f);
}

void UOTDSurvivalComponent::ModifyMorale(float Amount)
{
	Morale = FMath::Clamp(Morale + Amount, 0.0f, 100.0f);
	OnMoraleChanged.Broadcast(Morale, 100.0f);

	// Check for shell shock at < 30 morale (from design doc)
	if (Morale < 30.0f && !HasDisease(EOTDDisease::ShellShock))
	{
		ApplyDisease(EOTDDisease::ShellShock);
	}
}

void UOTDSurvivalComponent::RestoreAlertness(float Amount)
{
	Alertness = FMath::Clamp(Alertness + Amount, 0.0f, 100.0f);
	OnAlertнessChanged.Broadcast(Alertness, 100.0f);

	// Reset sleep timer if significant rest
	if (Amount > 50.0f)
	{
		TimeSinceLastSleep = 0.0f;
	}
}

// ========== Activity Costs ==========

void UOTDSurvivalComponent::ApplySprintCost(float DeltaTime)
{
	// 15/sec from design doc
	DrainStamina(15.0f * DeltaTime);
}

void UOTDSurvivalComponent::ApplyMeleeCost(float DeltaTime)
{
	// 10/sec from design doc
	DrainStamina(10.0f * DeltaTime);
}

void UOTDSurvivalComponent::ApplyClimbCost(float DeltaTime)
{
	// 12/sec from design doc
	DrainStamina(12.0f * DeltaTime);
}

void UOTDSurvivalComponent::ApplyCarryWoundedCost(float DeltaTime)
{
	// 20/sec from design doc
	DrainStamina(20.0f * DeltaTime);
}

// ========== Status Effects ==========

bool UOTDSurvivalComponent::HasDisease(EOTDDisease Disease) const
{
	return ActiveDiseases.Contains(Disease);
}

void UOTDSurvivalComponent::ApplyDisease(EOTDDisease Disease)
{
	if (!ActiveDiseases.Contains(Disease))
	{
		ActiveDiseases.Add(Disease);
		UE_LOG(LogTemp, Warning, TEXT("OTDSurvival: Applied disease %d"), static_cast<int32>(Disease));
	}
}

void UOTDSurvivalComponent::RemoveDisease(EOTDDisease Disease)
{
	ActiveDiseases.Remove(Disease);
	UE_LOG(LogTemp, Log, TEXT("OTDSurvival: Removed disease %d"), static_cast<int32>(Disease));
}

// ========== Modifiers ==========

float UOTDSurvivalComponent::GetMovementSpeedModifier() const
{
	float Modifier = 1.0f;

	// Hunger modifier (from design doc)
	if (Hunger < 40.0f)
	{
		Modifier *= FMath::Lerp(0.7f, 1.0f, Hunger / 40.0f); // -5% to -30%
	}

	// Stamina modifier (from design doc)
	if (Stamina < 30.0f)
	{
		Modifier *= 0.8f; // -20%
	}

	// Warmth/cold modifier (from design doc)
	if (Warmth < 50.0f)
	{
		Modifier *= FMath::Lerp(0.7f, 1.0f, Warmth / 50.0f); // -10% to -30%
	}

	// Disease modifiers
	if (HasDisease(EOTDDisease::TrenchFever) || HasDisease(EOTDDisease::Dysentery))
	{
		Modifier *= 0.7f; // -30% for illness
	}

	if (HasDisease(EOTDDisease::Frostbite))
	{
		Modifier *= 0.5f; // -50% for frostbite
	}

	return Modifier;
}

float UOTDSurvivalComponent::GetAimAccuracyModifier() const
{
	float Modifier = 1.0f;

	// Hunger affects aim (from design doc)
	if (Hunger < 50.0f)
	{
		Modifier *= FMath::Lerp(0.6f, 1.0f, Hunger / 50.0f);
	}

	// Low stamina = shaky aim
	if (Stamina < 30.0f)
	{
		Modifier *= FMath::Lerp(0.5f, 1.0f, Stamina / 30.0f);
	}

	// Alertness affects reaction time and precision
	if (Alertness < 40.0f)
	{
		Modifier *= FMath::Lerp(0.4f, 1.0f, Alertness / 40.0f);
	}

	// Morale affects steadiness (shell shock)
	if (Morale < 30.0f)
	{
		Modifier *= 0.6f; // Shell shock penalty
	}

	// Shell shock disease
	if (HasDisease(EOTDDisease::ShellShock))
	{
		Modifier *= 0.5f;
	}

	return FMath::Clamp(Modifier, 0.1f, 1.0f);
}

// ========== Internal Methods ==========

void UOTDSurvivalComponent::UpdateMeters(float DeltaTime)
{
	const float DeltaHours = DeltaTime / 3600.0f;
	const float DeltaMinutes = DeltaTime / 60.0f;

	// Hunger depletion (3-8/hr from design doc)
	Hunger -= HungerDepletionRate * DeltaHours;
	ClampAndBroadcast(Hunger, OnHungerChanged);

	// Thirst depletion (5-12/hr from design doc, MORE critical)
	Thirst -= ThirstDepletionRate * DeltaHours;
	ClampAndBroadcast(Thirst, OnThirstChanged);

	// Stamina regeneration (2.0/sec base from design doc)
	// Modified by hunger and thirst
	float StaminaRegenMod = 1.0f;
	if (Thirst < 50.0f)
	{
		StaminaRegenMod *= FMath::Lerp(0.3f, 1.0f, Thirst / 50.0f);
	}
	if (Hunger < 50.0f)
	{
		StaminaRegenMod *= FMath::Lerp(0.5f, 1.0f, Hunger / 50.0f);
	}

	Stamina += StaminaRegenRate * StaminaRegenMod * DeltaTime;
	ClampAndBroadcast(Stamina, OnStaminaChanged);

	// Warmth depletion (0.5-3.0/min from design doc)
	Warmth -= WarmthDepletionRate * DeltaMinutes;
	ClampAndBroadcast(Warmth, OnWarmthChanged);

	// Hygiene depletion (2.0/hr constant from design doc)
	Hygiene -= HygieneDepletionRate * DeltaHours;
	ClampAndBroadcast(Hygiene, OnHygieneChanged);

	// Morale depletion (0.5/hr base from design doc)
	Morale -= MoraleDepletionRate * DeltaHours;
	ClampAndBroadcast(Morale, OnMoraleChanged);

	// Alertness depletion (4.0/hr awake, 2x at night from design doc)
	float AlertnessRate = AlertnessDepletionRate;
	if (bIsNightTime)
	{
		AlertnessRate *= 2.0f;
	}
	Alertness -= AlertnessRate * DeltaHours;
	ClampAndBroadcast(Alertness, OnAlertнessChanged);

	// Track time awake
	TimeSinceLastSleep += DeltaHours;
}

void UOTDSurvivalComponent::CheckDeathConditions()
{
	FString DeathCause;

	// Thirst death in 1-3 days (from design doc)
	if (Thirst <= 0.0f)
	{
		DeathCause = TEXT("Dehydration");
	}
	// Hunger death in 7 days (from design doc)
	else if (Hunger <= 0.0f)
	{
		DeathCause = TEXT("Starvation");
	}
	// Hypothermia in 4 hours (from design doc)
	else if (Warmth <= 0.0f)
	{
		DeathCause = TEXT("Hypothermia");
	}
	// Collapse from exhaustion after 7 days awake (from design doc)
	else if (TimeSinceLastSleep >= 168.0f) // 7 days * 24 hours
	{
		DeathCause = TEXT("Exhaustion");
	}

	if (!DeathCause.IsEmpty())
	{
		bIsAlive = false;
		OnDeath.Broadcast(DeathCause);
		UE_LOG(LogTemp, Error, TEXT("OTDSurvival: Player died from %s"), *DeathCause);
	}
}

void UOTDSurvivalComponent::CheckDiseaseProgression()
{
	// Low hygiene leads to disease
	if (Hygiene < 20.0f && FMath::RandRange(0.0f, 100.0f) < 1.0f)
	{
		if (!HasDisease(EOTDDisease::TrenchFever))
		{
			ApplyDisease(EOTDDisease::TrenchFever);
		}
	}

	if (Hygiene < 10.0f && FMath::RandRange(0.0f, 100.0f) < 2.0f)
	{
		if (!HasDisease(EOTDDisease::Lice))
		{
			ApplyDisease(EOTDDisease::Lice);
		}
	}

	// Cold exposure leads to frostbite
	if (Warmth < 10.0f && FMath::RandRange(0.0f, 100.0f) < 3.0f)
	{
		if (!HasDisease(EOTDDisease::Frostbite))
		{
			ApplyDisease(EOTDDisease::Frostbite);
		}
	}
}

void UOTDSurvivalComponent::ApplyCascadingEffects(float DeltaTime)
{
	// Low thirst reduces stamina regeneration (already in UpdateMeters)

	// Low morale increases hunger/thirst depletion (stress eating/drinking)
	if (Morale < 30.0f)
	{
		const float StressMultiplier = 1.5f;
		HungerDepletionRate = FMath::Lerp(5.0f, 5.0f * StressMultiplier, (30.0f - Morale) / 30.0f);
		ThirstDepletionRate = FMath::Lerp(8.0f, 8.0f * StressMultiplier, (30.0f - Morale) / 30.0f);
	}
	else
	{
		// Reset to base rates
		HungerDepletionRate = 5.0f;
		ThirstDepletionRate = 8.0f;
	}

	// Diseases affect other meters
	if (HasDisease(EOTDDisease::Dysentery))
	{
		// Dysentery increases thirst and decreases hygiene
		Thirst -= 2.0f * (DeltaTime / 3600.0f);
		Hygiene -= 1.0f * (DeltaTime / 3600.0f);
	}

	if (HasDisease(EOTDDisease::TrenchFever))
	{
		// Fever drains stamina
		Stamina -= 0.5f * DeltaTime;
	}
}

void UOTDSurvivalComponent::ClampAndBroadcast(float& Meter, FOnSurvivalMeterChanged& Delegate)
{
	Meter = FMath::Clamp(Meter, 0.0f, 100.0f);
	Delegate.Broadcast(Meter, 100.0f);
}
