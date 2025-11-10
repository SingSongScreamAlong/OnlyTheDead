// Only The Dead - Medical System Implementation
// Copyright 2025. All Rights Reserved.

#include "MedicalSystem.h"
#include "SurvivalSystem.h"
#include "Kismet/GameplayStatics.h"

UMedicalSystem::UMedicalSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second

    CurrentHealth = 100.0f;
    MaxHealth = 100.0f;
    bIsMorphineActive = false;
    MorphineTimeRemaining = 0.0f;
}

void UMedicalSystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Initialized. Health: %.0f/%.0f"), CurrentHealth, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Historical - WWI field medicine limited, infection was major killer"));
}

void UMedicalSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateBleedingDamage(DeltaTime);
    UpdateInfections(DeltaTime);

    // Update morphine effects
    if (bIsMorphineActive)
    {
        MorphineTimeRemaining -= DeltaTime;
        if (MorphineTimeRemaining <= 0.0f)
        {
            bIsMorphineActive = false;
            UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Morphine effects worn off"));
        }
    }

    // Check for death
    if (ShouldDieFromInjuries())
    {
        TriggerDeath(TEXT("Injuries"));
    }
}

// ========================================================================
// HEALTH SYSTEM
// ========================================================================

void UMedicalSystem::ApplyDamage(float DamageAmount, EInjuryType InjuryType, EBodyPart BodyPart)
{
    CurrentHealth -= DamageAmount;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Warning, TEXT("MedicalSystem: Damage taken: %.0f, Health: %.0f/%.0f"),
           DamageAmount,
           CurrentHealth,
           MaxHealth);

    // Create injury based on damage
    FInjury NewInjury;
    NewInjury.InjuryType = InjuryType;
    NewInjury.AffectedBodyPart = BodyPart;
    NewInjury.TimeAcquired = UGameplayStatics::GetRealTimeSeconds(GetWorld());

    // Determine severity based on damage
    if (DamageAmount >= 75.0f)
    {
        NewInjury.Severity = EInjurySeverity::Fatal;
        TriggerDeath(TEXT("Fatal injury"));
        return;
    }
    else if (DamageAmount >= 50.0f)
    {
        NewInjury.Severity = EInjurySeverity::Critical;
        NewInjury.bIsBleeding = true;
        NewInjury.BleedingRate = 10.0f; // High bleeding rate
    }
    else if (DamageAmount >= 25.0f)
    {
        NewInjury.Severity = EInjurySeverity::Severe;
        NewInjury.bIsBleeding = true;
        NewInjury.BleedingRate = 5.0f;
    }
    else if (DamageAmount >= 10.0f)
    {
        NewInjury.Severity = EInjurySeverity::Moderate;
        NewInjury.bIsBleeding = FMath::RandBool();
        NewInjury.BleedingRate = 2.0f;
    }
    else
    {
        NewInjury.Severity = EInjurySeverity::Minor;
        NewInjury.bIsBleeding = false;
    }

    AddInjury(NewInjury);

    // Apply morale impact
    AActor* Owner = GetOwner();
    if (Owner)
    {
        USurvivalSystem* SurvivalSystem = Owner->FindComponentByClass<USurvivalSystem>();
        if (SurvivalSystem)
        {
            float MoraleHit = DamageAmount * 0.3f;
            SurvivalSystem->ModifyMeter(ESurvivalMeterType::Morale, -MoraleHit);
        }
    }

    // Check death
    if (CurrentHealth <= 0.0f)
    {
        TriggerDeath(TEXT("Health depleted"));
    }
}

void UMedicalSystem::Heal(float HealAmount)
{
    CurrentHealth += HealAmount;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Healed %.0f HP (Now: %.0f/%.0f)"),
           HealAmount,
           CurrentHealth,
           MaxHealth);
}

// ========================================================================
// INJURY SYSTEM
// ========================================================================

void UMedicalSystem::AddInjury(FInjury NewInjury)
{
    ActiveInjuries.Add(NewInjury);

    FString SeverityStr;
    switch (NewInjury.Severity)
    {
        case EInjurySeverity::Minor: SeverityStr = TEXT("MINOR"); break;
        case EInjurySeverity::Moderate: SeverityStr = TEXT("MODERATE"); break;
        case EInjurySeverity::Severe: SeverityStr = TEXT("SEVERE"); break;
        case EInjurySeverity::Critical: SeverityStr = TEXT("CRITICAL"); break;
        case EInjurySeverity::Fatal: SeverityStr = TEXT("FATAL"); break;
    }

    UE_LOG(LogTemp, Warning, TEXT("MedicalSystem: Injury added - %s, Bleeding: %s"),
           *SeverityStr,
           NewInjury.bIsBleeding ? TEXT("YES") : TEXT("NO"));
}

bool UMedicalSystem::TreatInjury(int32 InjuryIndex, const FString& MedicalSupplyID)
{
    if (!ActiveInjuries.IsValidIndex(InjuryIndex))
    {
        return false;
    }

    FInjury& Injury = ActiveInjuries[InjuryIndex];

    // Apply treatment
    Injury.bIsTreated = true;
    Injury.bIsBleeding = false; // Stop bleeding
    Injury.BleedingRate = 0.0f;

    // Heal some health
    float HealAmount = 25.0f; // Basic field dressing
    Heal(HealAmount);

    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Injury treated, bleeding stopped, +%.0f HP"), HealAmount);

    return true;
}

bool UMedicalSystem::HasInjuryType(EInjuryType InjuryType) const
{
    for (const FInjury& Injury : ActiveInjuries)
    {
        if (Injury.InjuryType == InjuryType)
        {
            return true;
        }
    }
    return false;
}

FInjury UMedicalSystem::GetMostSevereInjury() const
{
    FInjury MostSevere;
    MostSevere.Severity = EInjurySeverity::Minor;

    for (const FInjury& Injury : ActiveInjuries)
    {
        if (Injury.Severity > MostSevere.Severity)
        {
            MostSevere = Injury;
        }
    }

    return MostSevere;
}

bool UMedicalSystem::IsBleedingOut() const
{
    for (const FInjury& Injury : ActiveInjuries)
    {
        if (Injury.bIsBleeding)
        {
            return true;
        }
    }
    return false;
}

float UMedicalSystem::GetTotalBleedingRate() const
{
    float TotalRate = 0.0f;
    for (const FInjury& Injury : ActiveInjuries)
    {
        if (Injury.bIsBleeding)
        {
            TotalRate += Injury.BleedingRate;
        }
    }
    return TotalRate;
}

// ========================================================================
// MEDICAL TREATMENT
// ========================================================================

bool UMedicalSystem::UseMedicalSupply(const FString& SupplyID, int32 InjuryIndex)
{
    // Find supply
    for (FMedicalSupply& Supply : MedicalSupplies)
    {
        if (Supply.SupplyID == SupplyID && Supply.Quantity > 0)
        {
            // Use supply
            Supply.Quantity--;
            return TreatInjury(InjuryIndex, SupplyID);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("MedicalSystem: Medical supply '%s' not available"), *SupplyID);
    return false;
}

bool UMedicalSystem::ApplyFieldDressing(int32 InjuryIndex)
{
    return UseMedicalSupply(TEXT("field_dressing"), InjuryIndex);
}

void UMedicalSystem::InjectMorphine()
{
    // Historical: Morphine was standard WWI pain management
    // Effects: Pain suppression, morale boost, lasts ~4 hours

    bIsMorphineActive = true;
    MorphineTimeRemaining = 4.0f * 3600.0f; // 4 hours in seconds

    AActor* Owner = GetOwner();
    if (Owner)
    {
        USurvivalSystem* SurvivalSystem = Owner->FindComponentByClass<USurvivalSystem>();
        if (SurvivalSystem)
        {
            SurvivalSystem->ModifyMeter(ESurvivalMeterType::Morale, 30.0f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Morphine injected. Pain suppressed for 4 hours."));
}

bool UMedicalSystem::ApplyTourniquet(EBodyPart Limb)
{
    // Tourniquet stops bleeding on limbs
    // Can only be applied to arms/legs

    if (Limb != EBodyPart::LeftArm && Limb != EBodyPart::RightArm &&
        Limb != EBodyPart::LeftLeg && Limb != EBodyPart::RightLeg)
    {
        UE_LOG(LogTemp, Warning, TEXT("MedicalSystem: Cannot apply tourniquet to this body part"));
        return false;
    }

    // Stop all bleeding on this limb
    for (FInjury& Injury : ActiveInjuries)
    {
        if (Injury.AffectedBodyPart == Limb && Injury.bIsBleeding)
        {
            Injury.bIsBleeding = false;
            Injury.BleedingRate = 0.0f;
            UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Tourniquet applied, bleeding stopped"));
        }
    }

    return true;
}

FText UMedicalSystem::SelfTriage()
{
    // Assess own injuries

    FString Assessment;
    int32 InjuryCount = ActiveInjuries.Num();

    if (InjuryCount == 0)
    {
        Assessment = TEXT("No injuries. All clear.");
    }
    else
    {
        Assessment = FString::Printf(TEXT("Injuries: %d\n"), InjuryCount);

        FInjury MostSevere = GetMostSevereInjury();
        Assessment += FString::Printf(TEXT("Most severe: %s\n"),
            MostSevere.Severity == EInjurySeverity::Critical ? TEXT("CRITICAL") :
            MostSevere.Severity == EInjurySeverity::Severe ? TEXT("SEVERE") :
            MostSevere.Severity == EInjurySeverity::Moderate ? TEXT("MODERATE") :
            TEXT("MINOR"));

        if (IsBleedingOut())
        {
            float BleedRate = GetTotalBleedingRate();
            float TimeToDeathMinutes = CalculateTimeToDeathFromBleeding() / 60.0f;
            Assessment += FString::Printf(TEXT("BLEEDING! %.0f HP/sec. Time to death: %.1f min\n"),
                BleedRate,
                TimeToDeathMinutes);
        }

        Assessment += FString::Printf(TEXT("Health: %.0f/%.0f"), CurrentHealth, MaxHealth);
    }

    return FText::FromString(Assessment);
}

// ========================================================================
// FIELD HOSPITAL
// ========================================================================

void UMedicalSystem::TransferToFieldHospital()
{
    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Transferred to field hospital"));

    // Historical: Field hospitals at Verdun were overwhelmed
    // Triage determined who got treatment

    float SurvivalChance = CalculateSurvivalChance();
    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Survival chance: %.0f%%"), SurvivalChance * 100.0f);

    if (SurvivalChance < 0.3f)
    {
        UE_LOG(LogTemp, Warning, TEXT("MedicalSystem: TRIAGE - Too severe, minimal care given"));
    }
}

void UMedicalSystem::ReceiveHospitalTreatment(float HoursInCare)
{
    // Receive treatment at hospital
    // Heal gradually, remove infections, treat injuries

    float HealPerHour = 10.0f;
    float TotalHeal = HealPerHour * HoursInCare;
    Heal(TotalHeal);

    // Treat all injuries
    for (FInjury& Injury : ActiveInjuries)
    {
        Injury.bIsTreated = true;
        Injury.bIsBleeding = false;
        Injury.bIsInfected = false;
    }

    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Hospital treatment complete. Healed %.0f HP"), TotalHeal);
}

float UMedicalSystem::CalculateSurvivalChance() const
{
    // Calculate survival chance based on injuries

    if (CurrentHealth <= 0.0f)
    {
        return 0.0f;
    }

    float BaseChance = CurrentHealth / MaxHealth;

    // Reduce chance for severe injuries
    for (const FInjury& Injury : ActiveInjuries)
    {
        switch (Injury.Severity)
        {
            case EInjurySeverity::Critical:
                BaseChance *= 0.3f; // 70% reduction
                break;
            case EInjurySeverity::Severe:
                BaseChance *= 0.6f; // 40% reduction
                break;
            case EInjurySeverity::Moderate:
                BaseChance *= 0.85f; // 15% reduction
                break;
            default:
                break;
        }
    }

    // Bleeding reduces chance significantly
    if (IsBleedingOut())
    {
        BaseChance *= 0.5f;
    }

    return FMath::Clamp(BaseChance, 0.0f, 1.0f);
}

void UMedicalSystem::EvacuateToRearHospital()
{
    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Evacuated to rear hospital. Mission ended."));
    // This would end the current mission and trigger hospital recovery sequence
}

// ========================================================================
// INFECTION & DISEASE
// ========================================================================

void UMedicalSystem::CheckForInfection(float DeltaTime)
{
    // Untreated wounds can become infected
    // Infection chance increases over time

    for (FInjury& Injury : ActiveInjuries)
    {
        if (!Injury.bIsTreated && !Injury.bIsInfected)
        {
            float TimeSinceInjury = UGameplayStatics::GetRealTimeSeconds(GetWorld()) - Injury.TimeAcquired;
            float InfectionChance = 0.01f * (TimeSinceInjury / 3600.0f); // 1% per hour

            if (FMath::FRand() < InfectionChance * DeltaTime)
            {
                Injury.bIsInfected = true;
                UE_LOG(LogTemp, Warning, TEXT("MedicalSystem: Wound has become INFECTED!"));
            }
        }
    }
}

void UMedicalSystem::ContractDisease(EInjuryType DiseaseType)
{
    FInjury Disease;
    Disease.InjuryType = DiseaseType;
    Disease.Severity = EInjurySeverity::Moderate;
    Disease.AffectedBodyPart = EBodyPart::Torso;
    Disease.bIsInfected = false;
    Disease.TimeAcquired = UGameplayStatics::GetRealTimeSeconds(GetWorld());

    AddInjury(Disease);

    UE_LOG(LogTemp, Warning, TEXT("MedicalSystem: Contracted disease"));
}

bool UMedicalSystem::TreatDisease(EInjuryType DiseaseType)
{
    // Historical: WWI treatment for diseases was limited
    // Antibiotics didn't exist yet

    for (int32 i = ActiveInjuries.Num() - 1; i >= 0; i--)
    {
        if (ActiveInjuries[i].InjuryType == DiseaseType)
        {
            ActiveInjuries.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Disease treated"));
            return true;
        }
    }

    return false;
}

// ========================================================================
// DEATH SYSTEM
// ========================================================================

bool UMedicalSystem::ShouldDieFromInjuries() const
{
    // Death from:
    // 1. Health depleted
    // 2. Fatal injuries
    // 3. Untreated critical bleeding

    if (CurrentHealth <= 0.0f)
    {
        return true;
    }

    for (const FInjury& Injury : ActiveInjuries)
    {
        if (Injury.Severity == EInjurySeverity::Fatal)
        {
            return true;
        }
    }

    // Bleeding out for too long
    if (IsBleedingOut())
    {
        float TimeToDeathminutes = CalculateTimeToDeathFromBleeding();
        if (TimeToDeathminutes <= 0.0f)
        {
            return true;
        }
    }

    return false;
}

float UMedicalSystem::CalculateTimeToDeathFromBleeding() const
{
    float BleedingRate = GetTotalBleedingRate();

    if (BleedingRate <= 0.0f)
    {
        return TNumericLimits<float>::Max();
    }

    return CurrentHealth / BleedingRate; // Seconds until death
}

void UMedicalSystem::TriggerDeath(const FString& CauseOfDeath)
{
    UE_LOG(LogTemp, Error, TEXT("MedicalSystem: PLAYER DEATH - Cause: %s"), *CauseOfDeath);
    // Trigger death sequence
}

// ========================================================================
// COMRADE TREATMENT
// ========================================================================

bool UMedicalSystem::TreatWoundedComrade(AActor* ComradeActor)
{
    if (!ComradeActor)
    {
        return false;
    }

    // Find medical system on comrade
    UMedicalSystem* ComradeMedicalSystem = ComradeActor->FindComponentByClass<UMedicalSystem>();
    if (ComradeMedicalSystem)
    {
        // Apply field dressing
        if (ComradeMedicalSystem->ActiveInjuries.Num() > 0)
        {
            ComradeMedicalSystem->ApplyFieldDressing(0);
            UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Treated wounded comrade"));
            return true;
        }
    }

    return false;
}

bool UMedicalSystem::CarryWoundedComrade(AActor* ComradeActor)
{
    if (!ComradeActor)
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Carrying wounded comrade to safety"));
    // This would trigger carrying animation and movement penalty
    return true;
}

void UMedicalSystem::CallForMedic()
{
    UE_LOG(LogTemp, Log, TEXT("MedicalSystem: Called for medic!"));
    // Trigger medic AI to approach player
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void UMedicalSystem::UpdateBleedingDamage(float DeltaTime)
{
    if (IsBleedingOut())
    {
        float BleedingDamage = GetTotalBleedingRate() * DeltaTime;
        CurrentHealth -= BleedingDamage;
        CurrentHealth = FMath::Max(0.0f, CurrentHealth);

        if (CurrentHealth <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("MedicalSystem: BLED OUT"));
        }
    }
}

void UMedicalSystem::UpdateInfections(float DeltaTime)
{
    CheckForInfection(DeltaTime);

    // Infected wounds cause slow health drain
    for (const FInjury& Injury : ActiveInjuries)
    {
        if (Injury.bIsInfected)
        {
            float InfectionDamage = 1.0f * DeltaTime; // 1 HP per second
            CurrentHealth -= InfectionDamage;
        }
    }
}

void UMedicalSystem::ApplyPainEffects()
{
    // Pain from injuries affects movement, aim, morale
    // Morphine suppresses pain effects

    if (bIsMorphineActive)
    {
        return; // No pain while morphine active
    }

    // Calculate pain level from injuries
    float PainLevel = 0.0f;
    for (const FInjury& Injury : ActiveInjuries)
    {
        switch (Injury.Severity)
        {
            case EInjurySeverity::Critical: PainLevel += 50.0f; break;
            case EInjurySeverity::Severe: PainLevel += 25.0f; break;
            case EInjurySeverity::Moderate: PainLevel += 10.0f; break;
            case EInjurySeverity::Minor: PainLevel += 5.0f; break;
            default: break;
        }
    }

    // Apply pain effects to morale
    if (PainLevel > 0.0f)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            USurvivalSystem* SurvivalSystem = Owner->FindComponentByClass<USurvivalSystem>();
            if (SurvivalSystem)
            {
                SurvivalSystem->ModifyMeter(ESurvivalMeterType::Morale, -PainLevel * 0.1f);
            }
        }
    }
}

float UMedicalSystem::GetTreatmentSuccessRate(EInjurySeverity Severity, bool bIsFieldTreatment) const
{
    // Historical: WWI field medicine success rates

    float BaseSuccessRate = 0.5f;

    if (bIsFieldTreatment)
    {
        // Field treatment less effective
        switch (Severity)
        {
            case EInjurySeverity::Minor: BaseSuccessRate = 0.95f; break;
            case EInjurySeverity::Moderate: BaseSuccessRate = 0.70f; break;
            case EInjurySeverity::Severe: BaseSuccessRate = 0.40f; break;
            case EInjurySeverity::Critical: BaseSuccessRate = 0.15f; break;
            case EInjurySeverity::Fatal: BaseSuccessRate = 0.01f; break;
        }
    }
    else
    {
        // Hospital treatment more effective
        switch (Severity)
        {
            case EInjurySeverity::Minor: BaseSuccessRate = 0.99f; break;
            case EInjurySeverity::Moderate: BaseSuccessRate = 0.90f; break;
            case EInjurySeverity::Severe: BaseSuccessRate = 0.65f; break;
            case EInjurySeverity::Critical: BaseSuccessRate = 0.35f; break;
            case EInjurySeverity::Fatal: BaseSuccessRate = 0.05f; break;
        }
    }

    return BaseSuccessRate;
}
