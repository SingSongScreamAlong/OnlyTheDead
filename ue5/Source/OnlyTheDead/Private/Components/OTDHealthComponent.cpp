//// BEGIN NEW CODE
#include "Components/OTDHealthComponent.h"

#include "OTDLogging.h"
#include "Damage/OTDDamageTypeBase.h"
#include "Components/OTDMentalStateComponent.h"
#include "GameFramework/Actor.h"

UOTDHealthComponent::UOTDHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f;
}

void UOTDHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = ClampHealth(MaxHealth);
    CacheMentalStateComponent();
}

void UOTDHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsBleeding)
    {
        ApplyBleeding(DeltaTime);
    }
}

void UOTDHealthComponent::ApplyDamage(float DamageAmount, TSubclassOf<UDamageType> DamageTypeClass, AActor* DamageInstigator)
{
    if (DamageAmount <= 0.f || CurrentHealth <= 0.f)
    {
        return;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = ClampHealth(CurrentHealth - DamageAmount);

    LastWoundInstigator.InstigatorActor = DamageInstigator;
    LastWoundInstigator.DamageType = DamageTypeClass;

    const UOTDDamageTypeBase* DamageData = DamageTypeClass ? Cast<UOTDDamageTypeBase>(DamageTypeClass->GetDefaultObject()) : nullptr;
    UpdateWoundSeverity(DamageAmount, DamageData, DamageInstigator);
    ProcessDamageTypeEffects(DamageData, DamageInstigator);

    OnHealthChanged.Broadcast(PreviousHealth, CurrentHealth);

    if (CurrentHealth <= 0.f)
    {
        SetBleeding(false);
        CurrentWoundSeverity = EOTDWoundSeverity::Fatal;
        OnWoundSeverityChanged.Broadcast(EOTDWoundSeverity::Critical, EOTDWoundSeverity::Fatal);
    }
}

void UOTDHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.f || CurrentHealth <= 0.f)
    {
        return;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = ClampHealth(CurrentHealth + HealAmount);

    if (CurrentHealth > FatalHealthThreshold)
    {
        SetBleeding(false);
    }

    OnHealthChanged.Broadcast(PreviousHealth, CurrentHealth);
}

void UOTDHealthComponent::StopBleeding()
{
    SetBleeding(false);
}

void UOTDHealthComponent::ApplyBleeding(float DeltaTime)
{
    const float Damage = ActiveBleedRate * DeltaTime;
    if (Damage <= 0.f)
    {
        return;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = ClampHealth(CurrentHealth - Damage);
    OnHealthChanged.Broadcast(PreviousHealth, CurrentHealth);

    if (CurrentHealth <= 0.f)
    {
        SetBleeding(false);
        CurrentWoundSeverity = EOTDWoundSeverity::Fatal;
        OnWoundSeverityChanged.Broadcast(EOTDWoundSeverity::Critical, EOTDWoundSeverity::Fatal);
    }
}

void UOTDHealthComponent::UpdateWoundSeverity(float DamageApplied, const UOTDDamageTypeBase* DamageTypeData, AActor* InstigatorActor)
{
    EOTDWoundSeverity NewSeverity = CurrentWoundSeverity;

    const float AdjustedDamage = DamageTypeData ? DamageApplied * DamageTypeData->SeverityBonus : DamageApplied;

    if (AdjustedDamage >= CriticalWoundThreshold || CurrentHealth <= FatalHealthThreshold)
    {
        NewSeverity = EOTDWoundSeverity::Critical;
    }
    else if (AdjustedDamage >= SevereWoundThreshold)
    {
        NewSeverity = EOTDWoundSeverity::Severe;
    }
    else if (AdjustedDamage >= ModerateWoundThreshold)
    {
        NewSeverity = EOTDWoundSeverity::Moderate;
    }
    else if (AdjustedDamage >= LightWoundThreshold)
    {
        NewSeverity = EOTDWoundSeverity::Light;
    }

    if (NewSeverity != CurrentWoundSeverity)
    {
        const EOTDWoundSeverity PreviousSeverity = CurrentWoundSeverity;
        CurrentWoundSeverity = NewSeverity;
        OnWoundSeverityChanged.Broadcast(PreviousSeverity, CurrentWoundSeverity);

        if (CurrentWoundSeverity >= EOTDWoundSeverity::Moderate)
        {
            BleedMultiplierFromDamage = DamageTypeData ? DamageTypeData->BleedMultiplier : 1.0f;
            SetBleeding(true);
        }
    }

    if (CurrentWoundSeverity >= EOTDWoundSeverity::Critical)
    {
        SetBleeding(true, ComputeBleedRateForSeverity(EOTDWoundSeverity::Critical));
    }
}

void UOTDHealthComponent::SetBleeding(bool bShouldBleed, float BleedRateOverride)
{
    if (bIsBleeding == bShouldBleed && BleedRateOverride < 0.f)
    {
        return;
    }

    bIsBleeding = bShouldBleed;
    const float BaseRate = (BleedRateOverride >= 0.f ? BleedRateOverride : ComputeBleedRateForSeverity(CurrentWoundSeverity));
    ActiveBleedRate = bIsBleeding ? BaseRate * BleedMultiplierFromDamage : 0.0f;

    OnBleedingStateChanged.Broadcast(bIsBleeding);
}

float UOTDHealthComponent::ComputeBleedRateForSeverity(EOTDWoundSeverity Severity) const
{
    switch (Severity)
    {
    case EOTDWoundSeverity::Moderate:
        return BaseBleedRate * ModerateBleedMultiplier;
    case EOTDWoundSeverity::Severe:
        return BaseBleedRate * SevereBleedMultiplier;
    case EOTDWoundSeverity::Critical:
    case EOTDWoundSeverity::Fatal:
        return BaseBleedRate * CriticalBleedMultiplier;
    case EOTDWoundSeverity::Light:
    default:
        return BaseBleedRate;
    }
}

float UOTDHealthComponent::ClampHealth(float Value) const
{
    return FMath::Clamp(Value, 0.f, MaxHealth);
}

void UOTDHealthComponent::ProcessDamageTypeEffects(const UOTDDamageTypeBase* DamageTypeData, AActor* DamageInstigator)
{
    if (!DamageTypeData)
    {
        return;
    }

    CacheMentalStateComponent();
    if (UOTDMentalStateComponent* Mental = MentalStateComponent.Get())
    {
        if (DamageTypeData->SuppressionImpulse > 0.f)
        {
            Mental->ApplySuppression(DamageTypeData->SuppressionImpulse, DamageInstigator);
        }

        if (DamageTypeData->FearImpulse > 0.f)
        {
            Mental->ApplyFear(DamageTypeData->FearImpulse, DamageInstigator);
        }

        if (DamageTypeData->ShockImpulse > 0.f)
        {
            Mental->ApplyShock(DamageTypeData->ShockImpulse);
        }
    }
}

void UOTDHealthComponent::CacheMentalStateComponent()
{
    if (MentalStateComponent.IsValid())
    {
        return;
    }

    if (AActor* Owner = GetOwner())
    {
        MentalStateComponent = Owner->FindComponentByClass<UOTDMentalStateComponent>();
    }
}
//// END NEW CODE
