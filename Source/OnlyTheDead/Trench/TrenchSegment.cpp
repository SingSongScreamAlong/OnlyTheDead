#include "Trench/TrenchSegment.h"

#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

ATrenchSegment::ATrenchSegment()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root is a simple scene component — meshes attach to it
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    IntactMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IntactMesh"));
    IntactMesh->SetupAttachment(RootComponent);
    IntactMesh->SetCollisionProfileName(TEXT("BlockAll"));

    DamagedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DamagedMesh"));
    DamagedMesh->SetupAttachment(RootComponent);
    DamagedMesh->SetCollisionProfileName(TEXT("BlockAll"));
    DamagedMesh->SetVisibility(false);
    DamagedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CollapsedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollapsedMesh"));
    CollapsedMesh->SetupAttachment(RootComponent);
    CollapsedMesh->SetCollisionProfileName(TEXT("BlockAll"));
    CollapsedMesh->SetVisibility(false);
    CollapsedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
    AudioComp->SetupAttachment(RootComponent);
    AudioComp->bAutoActivate = false;
}

void ATrenchSegment::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    TrenchState   = ETrenchState::Intact;
    UpdateMeshVisibility();
}

// ---------------------------------------------------------------------------
// Cover value — queried by SurvivalComponent morale calculation
// ---------------------------------------------------------------------------

float ATrenchSegment::GetCoverValue() const
{
    switch (TrenchState)
    {
    case ETrenchState::Intact:    return 0.95f;
    case ETrenchState::Damaged:   return 0.60f;
    case ETrenchState::Collapsed: return 0.20f;
    case ETrenchState::Buried:    return 0.05f;
    default:                      return 0.0f;
    }
}

// ---------------------------------------------------------------------------
// Damage handling
// ---------------------------------------------------------------------------

float ATrenchSegment::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
    const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (Actual <= 0.0f || TrenchState == ETrenchState::Buried) return 0.0f;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    // Play impact sound on any hit
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }

    // Determine new state from health thresholds
    ETrenchState NewState = TrenchState;
    if (CurrentHealth <= 0.0f)
    {
        // > 150 damage in one hit = buried outright (direct hit from heavy shell)
        NewState = DamageAmount > 150.0f ? ETrenchState::Buried : ETrenchState::Collapsed;
    }
    else if (CurrentHealth <= MaxHealth * 0.5f)
    {
        NewState = ETrenchState::Damaged;
    }

    if (NewState != TrenchState)
    {
        SetTrenchState(NewState);

        // Heavy shells damage adjacent segments (chain collapse effect)
        if (DamageAmount > 100.0f)
        {
            SpreadDamageToAdjacent(0.4f);
        }
    }

    return DamageAmount;
}

void ATrenchSegment::SetTrenchState(ETrenchState NewState)
{
    if (NewState == TrenchState) return;

    const ETrenchState OldState = TrenchState;
    TrenchState = NewState;

    UpdateMeshVisibility();

    if (NewState == ETrenchState::Collapsed || NewState == ETrenchState::Buried)
    {
        TriggerCollapsePhysics();
    }

    OnTrenchStateChanged.Broadcast(this, NewState);

    UE_LOG(LogTemp, Log, TEXT("TrenchSegment[%d]: %s → %s (HP: %.0f/%.0f)"),
        SegmentIndex,
        OldState == ETrenchState::Intact    ? TEXT("Intact")    :
        OldState == ETrenchState::Damaged   ? TEXT("Damaged")   :
        OldState == ETrenchState::Collapsed ? TEXT("Collapsed") : TEXT("Buried"),
        NewState == ETrenchState::Intact    ? TEXT("Intact")    :
        NewState == ETrenchState::Damaged   ? TEXT("Damaged")   :
        NewState == ETrenchState::Collapsed ? TEXT("Collapsed") : TEXT("Buried"),
        CurrentHealth, MaxHealth);
}

// ---------------------------------------------------------------------------
// Mesh management
// ---------------------------------------------------------------------------

void ATrenchSegment::UpdateMeshVisibility()
{
    switch (TrenchState)
    {
    case ETrenchState::Intact:
        IntactMesh->SetVisibility(true);
        IntactMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        DamagedMesh->SetVisibility(false);
        DamagedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CollapsedMesh->SetVisibility(false);
        CollapsedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        break;

    case ETrenchState::Damaged:
        IntactMesh->SetVisibility(false);
        IntactMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DamagedMesh->SetVisibility(true);
        DamagedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CollapsedMesh->SetVisibility(false);
        CollapsedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        break;

    case ETrenchState::Collapsed:
    case ETrenchState::Buried:
        IntactMesh->SetVisibility(false);
        IntactMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DamagedMesh->SetVisibility(false);
        DamagedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CollapsedMesh->SetVisibility(true);
        CollapsedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        break;
    }
}

void ATrenchSegment::TriggerCollapsePhysics()
{
    // Play the collapse/cave-in sound
    if (CollapseSound && AudioComp)
    {
        AudioComp->SetSound(CollapseSound);
        AudioComp->Play();
    }

    // Simulate debris impulse — CollapsedMesh pieces fly if they have physics enabled
    // (Geometry Collection / Chaos setup is done in the editor for the actual meshes;
    //  this just kicks the root actor slightly for cheap visual feedback in POC)
    if (CollapsedMesh && CollapsedMesh->IsSimulatingPhysics())
    {
        const FVector Impulse = FVector(
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(500.0f, 2000.0f)
        );
        CollapsedMesh->AddImpulse(Impulse, NAME_None, true);
    }
}

void ATrenchSegment::SpreadDamageToAdjacent(float Fraction)
{
    for (ATrenchSegment* Adjacent : AdjacentSegments)
    {
        if (!Adjacent || Adjacent->TrenchState == ETrenchState::Buried) continue;

        const float SpreadDmg = MaxHealth * Fraction;
        FDamageEvent DmgEvt;
        Adjacent->TakeDamage(SpreadDmg, DmgEvt, nullptr, this);
    }
}
