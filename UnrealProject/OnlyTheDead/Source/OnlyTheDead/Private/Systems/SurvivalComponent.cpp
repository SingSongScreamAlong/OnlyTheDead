// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

#include "Systems/SurvivalComponent.h"
#include "GameFramework/Character.h"

USurvivalComponent::USurvivalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USurvivalComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeMeters();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update all survival meters
	UpdateMeter(HungerMeter, ESurvivalMeterType::Hunger, DeltaTime);
	UpdateMeter(ThirstMeter, ESurvivalMeterType::Thirst, DeltaTime);
	UpdateMeter(StaminaMeter, ESurvivalMeterType::Stamina, DeltaTime);
	UpdateMeter(WarmthMeter, ESurvivalMeterType::Warmth, DeltaTime);
	UpdateMeter(HygieneMeter, ESurvivalMeterType::Hygiene, DeltaTime);
	UpdateMeter(MoraleMeter, ESurvivalMeterType::Morale, DeltaTime);
	UpdateMeter(AlertnessMeter, ESurvivalMeterType::Alertness, DeltaTime);
}

void USurvivalComponent::InitializeMeters()
{
	// HUNGER METER
	// Depletion: 3.0 per hour resting = 0.000833 per second
	HungerMeter.CurrentValue = 100.0f;
	HungerMeter.MaxValue = 100.0f;
	HungerMeter.MinValue = 0.0f;
	HungerMeter.DepletionRate = 3.0f / 3600.0f; // Convert per-hour to per-second (resting rate)
	HungerMeter.RegenerationRate = 0.0f; // No natural regeneration
	HungerMeter.CriticalThreshold = 40.0f;
	HungerMeter.DeathTimer = 168.0f * 3600.0f; // 168 hours (7 days) in seconds
	HungerMeter.CurrentDeathCountdown = -1.0f;
	HungerMeter.bIsCritical = false;
	HungerMeter.bIsCountingDownToDeath = false;

	// THIRST METER
	// Depletion: 5.0 per hour resting = 0.001389 per second
	// Death after 24 hours (3 days = 72 hours)
	ThirstMeter.CurrentValue = 100.0f;
	ThirstMeter.MaxValue = 100.0f;
	ThirstMeter.MinValue = 0.0f;
	ThirstMeter.DepletionRate = 5.0f / 3600.0f;
	ThirstMeter.RegenerationRate = 0.0f;
	ThirstMeter.CriticalThreshold = 40.0f;
	ThirstMeter.DeathTimer = 72.0f * 3600.0f; // 72 hours (3 days) in seconds
	ThirstMeter.CurrentDeathCountdown = -1.0f;
	ThirstMeter.bIsCritical = false;
	ThirstMeter.bIsCountingDownToDeath = false;

	// STAMINA METER
	// Regenerates at 2.0 per second when resting
	StaminaMeter.CurrentValue = 100.0f;
	StaminaMeter.MaxValue = 100.0f;
	StaminaMeter.MinValue = 0.0f;
	StaminaMeter.DepletionRate = 0.0f; // Depleted by actions, not time
	StaminaMeter.RegenerationRate = 2.0f; // 2.0 per second
	StaminaMeter.CriticalThreshold = 30.0f;
	StaminaMeter.DeathTimer = -1.0f; // No death from stamina depletion
	StaminaMeter.CurrentDeathCountdown = -1.0f;
	StaminaMeter.bIsCritical = false;
	StaminaMeter.bIsCountingDownToDeath = false;

	// WARMTH METER
	// Depletion: 0.5 per minute in winter = 0.00833 per second
	// Death after 4 hours at 0
	WarmthMeter.CurrentValue = 100.0f;
	WarmthMeter.MaxValue = 100.0f;
	WarmthMeter.MinValue = 0.0f;
	WarmthMeter.DepletionRate = 0.5f / 60.0f; // Base winter rate
	WarmthMeter.RegenerationRate = 0.0f; // Restored by specific actions
	WarmthMeter.CriticalThreshold = 40.0f;
	WarmthMeter.DeathTimer = 4.0f * 3600.0f; // 4 hours in seconds
	WarmthMeter.CurrentDeathCountdown = -1.0f;
	WarmthMeter.bIsCritical = false;
	WarmthMeter.bIsCountingDownToDeath = false;

	// HYGIENE METER
	// Depletion: 2.0 per hour = 0.000556 per second
	HygieneMeter.CurrentValue = 100.0f;
	HygieneMeter.MaxValue = 100.0f;
	HygieneMeter.MinValue = 0.0f;
	HygieneMeter.DepletionRate = 2.0f / 3600.0f;
	HygieneMeter.RegenerationRate = 0.0f; // Restored by hygiene actions
	HygieneMeter.CriticalThreshold = 40.0f;
	HygieneMeter.DeathTimer = -1.0f; // No direct death, but illness guaranteed at 0
	HygieneMeter.CurrentDeathCountdown = -1.0f;
	HygieneMeter.bIsCritical = false;
	HygieneMeter.bIsCountingDownToDeath = false;

	// MORALE METER
	// Depletion: 0.5 per hour = 0.000139 per second
	MoraleMeter.CurrentValue = 100.0f;
	MoraleMeter.MaxValue = 100.0f;
	MoraleMeter.MinValue = 0.0f;
	MoraleMeter.DepletionRate = 0.5f / 3600.0f;
	MoraleMeter.RegenerationRate = 0.0f; // Restored by events
	MoraleMeter.CriticalThreshold = 30.0f;
	MoraleMeter.DeathTimer = -1.0f; // No direct death, but breakdown at 0
	MoraleMeter.CurrentDeathCountdown = -1.0f;
	MoraleMeter.bIsCritical = false;
	MoraleMeter.bIsCountingDownToDeath = false;

	// ALERTNESS METER
	// Depletion: 4.0 per hour awake = 0.001111 per second
	AlertnessMeter.CurrentValue = 100.0f;
	AlertnessMeter.MaxValue = 100.0f;
	AlertnessMeter.MinValue = 0.0f;
	AlertnessMeter.DepletionRate = 4.0f / 3600.0f;
	AlertnessMeter.RegenerationRate = 0.0f; // Restored by sleep
	AlertnessMeter.CriticalThreshold = 40.0f;
	AlertnessMeter.DeathTimer = -1.0f; // No direct death, falls asleep at 0
	AlertnessMeter.CurrentDeathCountdown = -1.0f;
	AlertnessMeter.bIsCritical = false;
	AlertnessMeter.bIsCountingDownToDeath = false;
}

void USurvivalComponent::UpdateMeter(FSurvivalMeter& Meter, ESurvivalMeterType MeterType, float DeltaTime)
{
	// Apply depletion
	if (Meter.DepletionRate > 0.0f)
	{
		Meter.CurrentValue -= Meter.DepletionRate * DeltaTime;
	}

	// Apply regeneration
	if (Meter.RegenerationRate > 0.0f)
	{
		Meter.CurrentValue += Meter.RegenerationRate * DeltaTime;
	}

	// Clamp value
	Meter.CurrentValue = FMath::Clamp(Meter.CurrentValue, Meter.MinValue, Meter.MaxValue);

	// Check for critical state
	bool bWasCritical = Meter.bIsCritical;
	Meter.bIsCritical = Meter.CurrentValue <= Meter.CriticalThreshold;

	// Fire events on state changes
	if (Meter.bIsCritical && !bWasCritical)
	{
		OnMeterCritical.Broadcast(MeterType);
	}
	else if (!Meter.bIsCritical && bWasCritical)
	{
		OnMeterRecovered.Broadcast(MeterType);
	}

	// Handle death countdown
	if (Meter.CurrentValue <= 0.0f && Meter.DeathTimer > 0.0f)
	{
		if (!Meter.bIsCountingDownToDeath)
		{
			// Start death countdown
			Meter.bIsCountingDownToDeath = true;
			Meter.CurrentDeathCountdown = Meter.DeathTimer;
			OnDeathCountdownStart.Broadcast(MeterType, Meter.DeathTimer);
		}
		else
		{
			// Continue countdown
			Meter.CurrentDeathCountdown -= DeltaTime;

			if (Meter.CurrentDeathCountdown <= 0.0f)
			{
				// Player has died
				HandleSurvivalDeath(MeterType);
			}
		}
	}
	else if (Meter.CurrentValue > 0.0f && Meter.bIsCountingDownToDeath)
	{
		// Meter recovered above 0, stop death countdown
		Meter.bIsCountingDownToDeath = false;
		Meter.CurrentDeathCountdown = -1.0f;
	}
}

FSurvivalMeter* USurvivalComponent::GetMeterByType(ESurvivalMeterType MeterType)
{
	switch (MeterType)
	{
		case ESurvivalMeterType::Hunger:	return &HungerMeter;
		case ESurvivalMeterType::Thirst:	return &ThirstMeter;
		case ESurvivalMeterType::Stamina:	return &StaminaMeter;
		case ESurvivalMeterType::Warmth:	return &WarmthMeter;
		case ESurvivalMeterType::Hygiene:	return &HygieneMeter;
		case ESurvivalMeterType::Morale:	return &MoraleMeter;
		case ESurvivalMeterType::Alertness:	return &AlertnessMeter;
		default:							return nullptr;
	}
}

const FSurvivalMeter* USurvivalComponent::GetMeterByType(ESurvivalMeterType MeterType) const
{
	switch (MeterType)
	{
		case ESurvivalMeterType::Hunger:	return &HungerMeter;
		case ESurvivalMeterType::Thirst:	return &ThirstMeter;
		case ESurvivalMeterType::Stamina:	return &StaminaMeter;
		case ESurvivalMeterType::Warmth:	return &WarmthMeter;
		case ESurvivalMeterType::Hygiene:	return &HygieneMeter;
		case ESurvivalMeterType::Morale:	return &MoraleMeter;
		case ESurvivalMeterType::Alertness:	return &AlertnessMeter;
		default:							return nullptr;
	}
}

void USurvivalComponent::ModifyMeter(ESurvivalMeterType MeterType, float DeltaValue)
{
	FSurvivalMeter* Meter = GetMeterByType(MeterType);
	if (Meter)
	{
		Meter->CurrentValue = FMath::Clamp(Meter->CurrentValue + DeltaValue, Meter->MinValue, Meter->MaxValue);
	}
}

void USurvivalComponent::SetMeter(ESurvivalMeterType MeterType, float NewValue)
{
	FSurvivalMeter* Meter = GetMeterByType(MeterType);
	if (Meter)
	{
		Meter->CurrentValue = FMath::Clamp(NewValue, Meter->MinValue, Meter->MaxValue);
	}
}

float USurvivalComponent::GetMeterValue(ESurvivalMeterType MeterType) const
{
	const FSurvivalMeter* Meter = GetMeterByType(MeterType);
	return Meter ? Meter->CurrentValue : 0.0f;
}

float USurvivalComponent::GetMeterPercentage(ESurvivalMeterType MeterType) const
{
	const FSurvivalMeter* Meter = GetMeterByType(MeterType);
	if (Meter && Meter->MaxValue > 0.0f)
	{
		return (Meter->CurrentValue / Meter->MaxValue) * 100.0f;
	}
	return 0.0f;
}

bool USurvivalComponent::IsMeterCritical(ESurvivalMeterType MeterType) const
{
	const FSurvivalMeter* Meter = GetMeterByType(MeterType);
	return Meter ? Meter->bIsCritical : false;
}

bool USurvivalComponent::IsMeterCountingToDeath(ESurvivalMeterType MeterType) const
{
	const FSurvivalMeter* Meter = GetMeterByType(MeterType);
	return Meter ? Meter->bIsCountingDownToDeath : false;
}

float USurvivalComponent::GetDeathCountdown(ESurvivalMeterType MeterType) const
{
	const FSurvivalMeter* Meter = GetMeterByType(MeterType);
	return Meter ? Meter->CurrentDeathCountdown : -1.0f;
}

void USurvivalComponent::ConsumeFood(float FoodValue)
{
	ModifyMeter(ESurvivalMeterType::Hunger, FoodValue);

	UE_LOG(LogTemp, Log, TEXT("Consumed food: +%.1f hunger (Now: %.1f)"), FoodValue, GetMeterValue(ESurvivalMeterType::Hunger));
}

void USurvivalComponent::DrinkWater(float WaterValue)
{
	ModifyMeter(ESurvivalMeterType::Thirst, WaterValue);

	UE_LOG(LogTemp, Log, TEXT("Drank water: +%.1f thirst (Now: %.1f)"), WaterValue, GetMeterValue(ESurvivalMeterType::Thirst));
}

void USurvivalComponent::Rest(float RestDuration)
{
	// Resting restores stamina faster
	float StaminaRestored = 2.0f * RestDuration; // Base regeneration rate
	ModifyMeter(ESurvivalMeterType::Stamina, StaminaRestored);

	UE_LOG(LogTemp, Log, TEXT("Rested for %.1f seconds: +%.1f stamina"), RestDuration, StaminaRestored);
}

void USurvivalComponent::Sleep(float SleepDuration)
{
	// Sleep restores alertness: 25 points per hour
	float HoursSle pt = SleepDuration / 3600.0f;
	float AlertnessRestored = 25.0f * HoursSlept;
	ModifyMeter(ESurvivalMeterType::Alertness, AlertnessRestored);

	// Also restore some stamina
	float StaminaRestored = 5.0f * HoursSlept;
	ModifyMeter(ESurvivalMeterType::Stamina, StaminaRestored);

	UE_LOG(LogTemp, Log, TEXT("Slept for %.1f hours: +%.1f alertness, +%.1f stamina"), HoursSlept, AlertnessRestored, StaminaRestored);
}

bool USurvivalComponent::ConsumeStamina(float StaminaCost)
{
	if (StaminaMeter.CurrentValue >= StaminaCost)
	{
		ModifyMeter(ESurvivalMeterType::Stamina, -StaminaCost);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Not enough stamina! Required: %.1f, Current: %.1f"), StaminaCost, StaminaMeter.CurrentValue);
	return false;
}

bool USurvivalComponent::HasStamina(float RequiredStamina) const
{
	return StaminaMeter.CurrentValue >= RequiredStamina;
}

void USurvivalComponent::HandleSurvivalDeath(ESurvivalMeterType MeterType)
{
	// Fire death event
	OnSurvivalDeath.Broadcast(MeterType);

	// Log death cause
	FString MeterName;
	switch (MeterType)
	{
		case ESurvivalMeterType::Hunger:	MeterName = TEXT("Starvation"); break;
		case ESurvivalMeterType::Thirst:	MeterName = TEXT("Dehydration"); break;
		case ESurvivalMeterType::Warmth:	MeterName = TEXT("Hypothermia"); break;
		default:							MeterName = TEXT("Unknown"); break;
	}

	UE_LOG(LogTemp, Error, TEXT("PLAYER DEATH: %s"), *MeterName);

	// Additional death handling should be done by the GameMode
}
