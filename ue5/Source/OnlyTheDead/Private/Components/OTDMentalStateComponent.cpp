//// BEGIN NEW CODE
#include "Components/OTDMentalStateComponent.h"

#include "OTDLogging.h"

UOTDMentalStateComponent::UOTDMentalStateComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
}

void UOTDMentalStateComponent::BeginPlay()
{
    Super::BeginPlay();

    Suppression = 0.0f;
    Fear = 0.0f;
    Panic = 0.0f;
    bHasTriggeredPanic = false;
}

void UOTDMentalStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateSuppression(DeltaTime);
    UpdateFear(DeltaTime);
    UpdatePanic(DeltaTime);
}

void UOTDMentalStateComponent::ApplySuppression(float Amount, AActor* SourceActor)
{
    if (Amount <= 0.f)
    {
        return;
    }

    const float Previous = Suppression;
    Suppression = FMath::Clamp(Suppression + Amount, 0.f, MaxSuppression);
    BroadcastValueChange(Previous, Suppression, OnSuppressionChanged);
}

void UOTDMentalStateComponent::ApplyFear(float Amount, AActor* SourceActor)
{
    if (Amount <= 0.f)
    {
        return;
    }

    const float Previous = Fear;
    Fear = FMath::Clamp(Fear + Amount, 0.f, MaxFear);
    BroadcastValueChange(Previous, Fear, OnFearChanged);
}

void UOTDMentalStateComponent::ApplyShock(float Amount)
{
    if (Amount <= 0.f)
    {
        return;
    }

    const float DeltaPanic = Amount * ShockToPanicMultiplier;
    const float Previous = Panic;
    Panic = FMath::Clamp(Panic + DeltaPanic, 0.f, MaxPanic);
    BroadcastValueChange(Previous, Panic, OnPanicChanged);
}

void UOTDMentalStateComponent::RegisterSuppressionImpulse(float Amount)
{
    if (Amount <= 0.f)
    {
        return;
    }

    const float Previous = Suppression;
    Suppression = FMath::Clamp(Suppression + Amount, 0.f, MaxSuppression);
    BroadcastValueChange(Previous, Suppression, OnSuppressionChanged);
}

void UOTDMentalStateComponent::RegisterNearMiss(float Distance, float Intensity)
{
    const float SuppressionImpulse = ComputeSuppressionFromNearMiss(Distance, Intensity);
    if (SuppressionImpulse <= 0.f)
    {
        return;
    }

    RegisterSuppressionImpulse(SuppressionImpulse);
}

void UOTDMentalStateComponent::UpdateSuppression(float DeltaTime)
{
    if (Suppression <= 0.f)
    {
        return;
    }

    const float Previous = Suppression;
    Suppression = FMath::Clamp(Suppression - SuppressionRecoveryRate * DeltaTime, 0.f, MaxSuppression);
    BroadcastValueChange(Previous, Suppression, OnSuppressionChanged);
}

void UOTDMentalStateComponent::UpdateFear(float DeltaTime)
{
    if (Fear <= 0.f)
    {
        return;
    }

    const float Previous = Fear;
    Fear = FMath::Clamp(Fear - FearRecoveryRate * DeltaTime, 0.f, MaxFear);
    BroadcastValueChange(Previous, Fear, OnFearChanged);
}

void UOTDMentalStateComponent::UpdatePanic(float DeltaTime)
{
    const float PanicDelta = (Suppression * PanicFromSuppressionMultiplier + Fear * PanicFromFearMultiplier) * DeltaTime;
    const float Decay = PanicDecayRate * DeltaTime;

    const float Previous = Panic;
    Panic = FMath::Clamp(Panic + PanicDelta - Decay, 0.f, MaxPanic);
    BroadcastValueChange(Previous, Panic, OnPanicChanged);

    if (!bHasTriggeredPanic && Panic >= PanicThreshold)
    {
        bHasTriggeredPanic = true;
        OnPanicTriggered.Broadcast();
    }
    else if (bHasTriggeredPanic && Panic < PanicThreshold * 0.5f)
    {
        bHasTriggeredPanic = false;
    }
}

void UOTDMentalStateComponent::BroadcastValueChange(float PreviousValue, float NewValue, FOTDMoraleValueChangedSignature& DelegateRef)
{
    if (!FMath::IsNearlyEqual(PreviousValue, NewValue))
    {
        DelegateRef.Broadcast(PreviousValue, NewValue);
    }
}

float UOTDMentalStateComponent::ComputeSuppressionFromNearMiss(float Distance, float Intensity) const
{
    if (Distance > NearMissSuppressionRange)
    {
        return 0.f;
    }

    const float DistanceAlpha = 1.f - FMath::Clamp(Distance / NearMissSuppressionRange, 0.f, 1.f);
    const float Result = NearMissMaxImpulse * DistanceAlpha * FMath::Clamp(Intensity, 0.f, 1.f);
    return Result;
}
//// END NEW CODE
