// Only The Dead - Survival System Implementation
// Copyright 2025. All Rights Reserved.

#include "SurvivalSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

USurvivalSystem::USurvivalSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USurvivalSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeSurvivalMeters();
}

void USurvivalSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update all survival meters
    for (auto& MeterPair : SurvivalMeters)
    {
        FSurvivalMeterState& Meter = MeterPair.Value;

        // Calculate depletion for this frame
        float DepletionThisFrame = (Meter.DepletionRatePerHour / 3600.0f) * DeltaTime;
        Meter.CurrentValue = FMath::Clamp(Meter.CurrentValue - DepletionThisFrame, 0.0f, 100.0f);

        // Track time at zero for death calculations
        if (Meter.CurrentValue <= 0.0f)
        {
            Meter.TimeAtZero += DeltaTime;
        }
        else
        {
            Meter.TimeAtZero = 0.0f;
        }

        // Mark as critical if below 25%
        Meter.bIsCritical = (Meter.CurrentValue < 25.0f);
    }

    // Apply cascading effects
    ApplyCascadingEffects(DeltaTime);

    // Check for death conditions
    CheckDeathConditions();
}

void USurvivalSystem::InitializeSurvivalMeters()
{
    // Hunger
    FSurvivalMeterState Hunger;
    Hunger.MeterType = ESurvivalMeterType::Hunger;
    Hunger.CurrentValue = 100.0f;
    Hunger.DepletionRatePerHour = GameConstants::HUNGER_DEPLETION_BASE;
    SurvivalMeters.Add(ESurvivalMeterType::Hunger, Hunger);

    // Thirst (CRITICAL METER)
    FSurvivalMeterState Thirst;
    Thirst.MeterType = ESurvivalMeterType::Thirst;
    Thirst.CurrentValue = 100.0f;
    Thirst.DepletionRatePerHour = GameConstants::THIRST_DEPLETION_BASE;
    SurvivalMeters.Add(ESurvivalMeterType::Thirst, Thirst);

    // Stamina
    FSurvivalMeterState Stamina;
    Stamina.MeterType = ESurvivalMeterType::Stamina;
    Stamina.CurrentValue = 100.0f;
    Stamina.DepletionRatePerHour = 0.0f; // Depletes on action, not time
    SurvivalMeters.Add(ESurvivalMeterType::Stamina, Stamina);

    // Warmth
    FSurvivalMeterState Warmth;
    Warmth.MeterType = ESurvivalMeterType::Warmth;
    Warmth.CurrentValue = 100.0f;
    Warmth.DepletionRatePerHour = GameConstants::WARMTH_DEPLETION_COLD;
    SurvivalMeters.Add(ESurvivalMeterType::Warmth, Warmth);

    // Hygiene
    FSurvivalMeterState Hygiene;
    Hygiene.MeterType = ESurvivalMeterType::Hygiene;
    Hygiene.CurrentValue = 100.0f;
    Hygiene.DepletionRatePerHour = GameConstants::HYGIENE_DEPLETION_BASE;
    SurvivalMeters.Add(ESurvivalMeterType::Hygiene, Hygiene);

    // Morale
    FSurvivalMeterState Morale;
    Morale.MeterType = ESurvivalMeterType::Morale;
    Morale.CurrentValue = 75.0f; // Start at 75%, not 100%
    Morale.DepletionRatePerHour = 0.0f; // Event-based, not time-based
    SurvivalMeters.Add(ESurvivalMeterType::Morale, Morale);

    // Alertness
    FSurvivalMeterState Alertness;
    Alertness.MeterType = ESurvivalMeterType::Alertness;
    Alertness.CurrentValue = 100.0f;
    Alertness.DepletionRatePerHour = GameConstants::ALERTNESS_DEPLETION_BASE;
    SurvivalMeters.Add(ESurvivalMeterType::Alertness, Alertness);
}

float USurvivalSystem::GetMeterValue(ESurvivalMeterType MeterType) const
{
    if (const FSurvivalMeterState* Meter = SurvivalMeters.Find(MeterType))
    {
        return Meter->CurrentValue;
    }
    return 0.0f;
}

void USurvivalSystem::SetMeterValue(ESurvivalMeterType MeterType, float NewValue)
{
    if (FSurvivalMeterState* Meter = SurvivalMeters.Find(MeterType))
    {
        Meter->CurrentValue = ClampMeterValue(NewValue);
    }
}

void USurvivalSystem::ModifyMeter(ESurvivalMeterType MeterType, float DeltaValue)
{
    if (FSurvivalMeterState* Meter = SurvivalMeters.Find(MeterType))
    {
        Meter->CurrentValue = ClampMeterValue(Meter->CurrentValue + DeltaValue);
    }
}

bool USurvivalSystem::IsAnyCritical() const
{
    for (const auto& MeterPair : SurvivalMeters)
    {
        if (MeterPair.Value.bIsCritical)
        {
            return true;
        }
    }
    return false;
}

bool USurvivalSystem::ShouldPlayerDie(FString& OutCauseOfDeath) const
{
    // Check thirst (CRITICAL - death in 24 hours)
    if (const FSurvivalMeterState* Thirst = SurvivalMeters.Find(ESurvivalMeterType::Thirst))
    {
        if (Thirst->CurrentValue <= 0.0f && Thirst->TimeAtZero >= GameConstants::TIME_TO_DEATH_THIRST_HOURS * 3600.0f)
        {
            OutCauseOfDeath = TEXT("Died of dehydration");
            return true;
        }
    }

    // Check hunger (death in 7 days)
    if (const FSurvivalMeterState* Hunger = SurvivalMeters.Find(ESurvivalMeterType::Hunger))
    {
        if (Hunger->CurrentValue <= 0.0f && Hunger->TimeAtZero >= GameConstants::TIME_TO_DEATH_HUNGER_HOURS * 3600.0f)
        {
            OutCauseOfDeath = TEXT("Died of starvation");
            return true;
        }
    }

    // Check warmth (hypothermia death in 4 hours)
    if (const FSurvivalMeterState* Warmth = SurvivalMeters.Find(ESurvivalMeterType::Warmth))
    {
        if (Warmth->CurrentValue <= 0.0f && Warmth->TimeAtZero >= GameConstants::TIME_TO_DEATH_HYPOTHERMIA_HOURS * 3600.0f)
        {
            OutCauseOfDeath = TEXT("Died of hypothermia");
            return true;
        }
    }

    // Check alertness (death from sleep deprivation - 3 days)
    if (const FSurvivalMeterState* Alertness = SurvivalMeters.Find(ESurvivalMeterType::Alertness))
    {
        if (Alertness->CurrentValue <= 0.0f && Alertness->TimeAtZero >= GameConstants::TIME_TO_DEATH_SLEEP_DEPRIVATION_HOURS * 3600.0f)
        {
            OutCauseOfDeath = TEXT("Died of exhaustion and sleep deprivation");
            return true;
        }
    }

    return false;
}

// ============================================================================
// HUNGER SYSTEM
// ============================================================================

void USurvivalSystem::ConsumeFood(FName FoodItemID)
{
    // TODO: Look up food item in data table and get restoration value
    // For now, use placeholder values
    float HungerRestoration = 40.0f; // Example: bread ration

    ModifyMeter(ESurvivalMeterType::Hunger, HungerRestoration);

    UE_LOG(LogTemp, Log, TEXT("Consumed food: %s, restored %f hunger"), *FoodItemID.ToString(), HungerRestoration);
}

FText USurvivalSystem::GetHungerStageText() const
{
    float HungerValue = GetMeterValue(ESurvivalMeterType::Hunger);

    if (HungerValue > 75.0f)
        return FText::FromString(TEXT("Well Fed"));
    else if (HungerValue > 50.0f)
        return FText::FromString(TEXT("Hungry"));
    else if (HungerValue > 25.0f)
        return FText::FromString(TEXT("Very Hungry"));
    else if (HungerValue > 0.0f)
        return FText::FromString(TEXT("Starving"));
    else
        return FText::FromString(TEXT("Critical Starvation"));
}

float USurvivalSystem::CalculateHungerDepletionRate(bool bIsActive, bool bIsCombat) const
{
    float BaseRate = GameConstants::HUNGER_DEPLETION_BASE; // 5.0/hour

    if (bIsCombat)
        return BaseRate * 1.6f; // 8.0/hour in combat
    else if (bIsActive)
        return BaseRate * 1.2f; // 6.0/hour when active
    else
        return BaseRate * 0.6f; // 3.0/hour when resting
}

// ============================================================================
// THIRST SYSTEM (CRITICAL)
// ============================================================================

void USurvivalSystem::DrinkWater(float AmountML, bool bIsContaminated)
{
    // Convert ML to percentage (assume 2L = 100%)
    float ThirstRestoration = (AmountML / 2000.0f) * 100.0f;

    ModifyMeter(ESurvivalMeterType::Thirst, ThirstRestoration);

    if (bIsContaminated)
    {
        // Contaminated water causes hygiene and potential disease
        ModifyMeter(ESurvivalMeterType::Hygiene, -10.0f);

        // Roll for disease (20% chance)
        if (FMath::RandRange(0.0f, 1.0f) < 0.2f)
        {
            RollForDisease(0.0f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Drank %f ML of water (contaminated: %d)"), AmountML, bIsContaminated);
}

FText USurvivalSystem::GetThirstStageText() const
{
    float ThirstValue = GetMeterValue(ESurvivalMeterType::Thirst);

    if (ThirstValue > 75.0f)
        return FText::FromString(TEXT("Hydrated"));
    else if (ThirstValue > 50.0f)
        return FText::FromString(TEXT("Thirsty"));
    else if (ThirstValue > 25.0f)
        return FText::FromString(TEXT("Very Thirsty"));
    else if (ThirstValue > 0.0f)
        return FText::FromString(TEXT("Severely Dehydrated"));
    else
        return FText::FromString(TEXT("CRITICAL - Death Imminent"));
}

bool USurvivalSystem::IsDehydrationCritical() const
{
    if (const FSurvivalMeterState* Thirst = SurvivalMeters.Find(ESurvivalMeterType::Thirst))
    {
        return (Thirst->CurrentValue <= 0.0f && Thirst->TimeAtZero > 3600.0f); // 1 hour at 0
    }
    return false;
}

float USurvivalSystem::CalculateThirstDepletionRate(bool bIsActive, float Temperature) const
{
    float BaseRate = GameConstants::THIRST_DEPLETION_BASE; // 8.0/hour

    // Activity modifier
    float ActivityMultiplier = bIsActive ? 1.5f : 1.0f;

    // Temperature modifier (hot weather increases thirst)
    float TempMultiplier = 1.0f + FMath::Clamp((Temperature - 20.0f) / 30.0f, 0.0f, 0.5f);

    return BaseRate * ActivityMultiplier * TempMultiplier;
}

// ============================================================================
// STAMINA SYSTEM
// ============================================================================

void USurvivalSystem::DepleteStamina(float Amount)
{
    ModifyMeter(ESurvivalMeterType::Stamina, -Amount);
}

void USurvivalSystem::RegenerateStamina(float DeltaTime, bool bIsResting)
{
    float RegenRate = bIsResting ? 30.0f : 10.0f; // per second
    float RegenAmount = RegenRate * DeltaTime;

    // Hunger affects regen
    float HungerValue = GetMeterValue(ESurvivalMeterType::Hunger);
    if (HungerValue < 25.0f)
    {
        RegenAmount *= 0.5f; // Half regen when starving
    }

    ModifyMeter(ESurvivalMeterType::Stamina, RegenAmount);
}

bool USurvivalSystem::HasStaminaForAction(float RequiredStamina) const
{
    return GetMeterValue(ESurvivalMeterType::Stamina) >= RequiredStamina;
}

// ============================================================================
// CASCADING FAILURE SYSTEM
// ============================================================================

void USurvivalSystem::ApplyCascadingEffects(float DeltaTime)
{
    // Low hunger reduces stamina regeneration (handled in RegenerateStamina)

    // Low thirst reduces all meter regeneration
    float ThirstValue = GetMeterValue(ESurvivalMeterType::Thirst);
    if (ThirstValue < 25.0f)
    {
        // Increase depletion of all meters by 20%
        for (auto& MeterPair : SurvivalMeters)
        {
            if (MeterPair.Key != ESurvivalMeterType::Thirst)
            {
                MeterPair.Value.DepletionRatePerHour *= 1.2f;
            }
        }
    }

    // Low morale increases stress-related depletion
    float MoraleValue = GetMeterValue(ESurvivalMeterType::Morale);
    if (MoraleValue < 25.0f)
    {
        // Increase alertness depletion (anxiety prevents sleep)
        if (FSurvivalMeterState* Alertness = SurvivalMeters.Find(ESurvivalMeterType::Alertness))
        {
            Alertness->DepletionRatePerHour *= 1.3f;
        }
    }

    // Low hygiene increases disease risk
    float HygieneValue = GetMeterValue(ESurvivalMeterType::Hygiene);
    if (HygieneValue < 25.0f)
    {
        TimeSinceLastHygieneCheck += DeltaTime;
        if (TimeSinceLastHygieneCheck > 3600.0f) // Check every game hour
        {
            RollForDisease(DeltaTime);
            TimeSinceLastHygieneCheck = 0.0f;
        }
    }
}

void USurvivalSystem::CheckDeathConditions()
{
    FString CauseOfDeath;
    if (ShouldPlayerDie(CauseOfDeath))
    {
        // Trigger player death
        UE_LOG(LogTemp, Error, TEXT("PLAYER DEATH: %s"), *CauseOfDeath);

        // TODO: Call game mode to handle death
        // GetWorld()->GetAuthGameMode()->PlayerDied(CauseOfDeath);
    }
}

// ============================================================================
// DISEASE SYSTEM
// ============================================================================

bool USurvivalSystem::RollForDisease(float DeltaTime)
{
    float HygieneValue = GetMeterValue(ESurvivalMeterType::Hygiene);
    float DiseaseRisk = (100.0f - HygieneValue) / 100.0f; // 0.0 - 1.0

    // Random roll
    float Roll = FMath::RandRange(0.0f, 1.0f);

    if (Roll < DiseaseRisk * 0.05f) // Max 5% chance per check
    {
        // Contract disease
        UE_LOG(LogTemp, Warning, TEXT("Contracted disease! Hygiene: %f"), HygieneValue);

        // Apply disease effects
        bHasDysentery = true;

        // Reduce health and survival meters
        ModifyMeter(ESurvivalMeterType::Hunger, -20.0f);
        ModifyMeter(ESurvivalMeterType::Stamina, -30.0f);

        return true;
    }

    return false;
}

float USurvivalSystem::GetDiseaseRisk() const
{
    float HygieneValue = GetMeterValue(ESurvivalMeterType::Hygiene);
    return (100.0f - HygieneValue) / 100.0f;
}

void USurvivalSystem::UpdateMeterThresholds()
{
    // Update critical flags for UI
    for (auto& MeterPair : SurvivalMeters)
    {
        MeterPair.Value.bIsCritical = (MeterPair.Value.CurrentValue < 25.0f);
    }
}
