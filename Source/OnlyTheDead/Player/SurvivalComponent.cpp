#include "Player/SurvivalComponent.h"
#include "Trench/TrenchSegment.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for meter decay
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    Morale    = MaxMorale;
    Hunger    = 100.0f;
    Thirst    = 100.0f;
    Stamina   = 100.0f;
    Warmth    = 100.0f;
    Hygiene   = 100.0f;
    Alertness = 100.0f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    SimulateMoraleDecay(DeltaTime);

    if (!bPOCMoraleOnly)
    {
        // Phase 2: tick all meters here using design-doc decay rates
        // Hunger:    3-8/hour → 0.001–0.002/s
        // Thirst:    5-12/hour → 0.0014–0.003/s
        // Stamina:   activity-driven
        // Warmth:    0.05-0.5/min → 0.0008–0.008/s
        // Hygiene:   2/hour → 0.0006/s
        // Alertness: 4/hour awake → 0.001/s
    }
}

// ---------------------------------------------------------------------------
// Morale
// ---------------------------------------------------------------------------

void USurvivalComponent::ApplyMoraleDamage(float Amount, bool bNearMiss)
{
    // Cover reduces morale damage — you feel safer in a trench
    const float Cover = FindBestNearbyTrenchCover();
    const float Multiplier = FMath::Lerp(1.0f, 0.3f, Cover);
    float ActualDamage = Amount * Multiplier;

    // Near-miss: extra trauma regardless of cover (the psychological reality)
    if (bNearMiss)
    {
        ActualDamage += 5.0f;
    }

    Morale = FMath::Max(0.0f, Morale - ActualDamage);

    // Reset recovery cooldown — morale won't recover while under fire
    MoraleRecoveryCooldown = RecoveryCooldownDuration;

    OnMoraleChanged.Broadcast(Morale, MaxMorale);
    UpdateShellShockState();

    UE_LOG(LogTemp, Verbose, TEXT("SurvivalComponent: Morale damage %.1f (cover %.0f%%) → Morale %.1f"),
        ActualDamage, Cover * 100.0f, Morale);
}

void USurvivalComponent::SimulateMoraleDecay(float DeltaTime)
{
    // Tick down the recovery cooldown
    if (MoraleRecoveryCooldown > 0.0f)
    {
        MoraleRecoveryCooldown -= DeltaTime;
        return; // No recovery while cooldown is active
    }

    // Slow recovery during lulls in bombardment
    if (Morale < MaxMorale)
    {
        const float OldMorale = Morale;
        Morale = FMath::Min(MaxMorale, Morale + MoraleRecoveryRate * DeltaTime);

        if (!FMath::IsNearlyEqual(OldMorale, Morale, 0.1f))
        {
            OnMoraleChanged.Broadcast(Morale, MaxMorale);
            UpdateShellShockState();
        }
    }
}

void USurvivalComponent::UpdateShellShockState()
{
    EShellShockState NewState;

    if (Morale <= DissociationThreshold)
    {
        NewState = EShellShockState::Dissociated;
    }
    else if (Morale <= ShellShockThreshold)
    {
        NewState = EShellShockState::ShellShocked;
    }
    else
    {
        NewState = EShellShockState::Normal;
    }

    if (NewState != CurrentShockState)
    {
        CurrentShockState = NewState;
        OnShellShockStateChanged.Broadcast(NewState);

        UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Shell shock state → %s (Morale: %.1f)"),
            NewState == EShellShockState::Normal      ? TEXT("Normal") :
            NewState == EShellShockState::ShellShocked? TEXT("Shell Shocked") : TEXT("Dissociated"),
            Morale);
    }
}

float USurvivalComponent::GetCurrentCoverValue() const
{
    return FindBestNearbyTrenchCover();
}

// ---------------------------------------------------------------------------
// Cover query
// ---------------------------------------------------------------------------

float USurvivalComponent::FindBestNearbyTrenchCover() const
{
    const FVector MyLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATrenchSegment::StaticClass(), NearbyActors);

    float BestCover = 0.0f;
    for (AActor* Actor : NearbyActors)
    {
        if (FVector::Dist(Actor->GetActorLocation(), MyLocation) <= CoverScanRadius)
        {
            const ATrenchSegment* Seg = Cast<ATrenchSegment>(Actor);
            if (Seg)
            {
                BestCover = FMath::Max(BestCover, Seg->GetCoverValue());
            }
        }
    }

    return BestCover;
}
