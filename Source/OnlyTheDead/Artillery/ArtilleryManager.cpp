#include "Artillery/ArtilleryManager.h"
#include "Artillery/ArtilleryShell.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

AArtilleryManager::AArtilleryManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default launch origin: 20,000 cm (200 m) off in X — "behind German lines"
    LaunchOrigin = FVector(20000.0f, 0.0f, 500.0f);
}

void AArtilleryManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoStartOnBeginPlay && ShellClass)
    {
        StartBarrage();
    }
}

// ---------------------------------------------------------------------------
// Public controls
// ---------------------------------------------------------------------------

void AArtilleryManager::StartBarrage()
{
    if (CurrentState == EBarrageState::Incoming) return;

    CurrentState = EBarrageState::Incoming;
    WalkOffset   = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("ArtilleryManager: Barrage started. Shells away."));
    FireNext();
}

void AArtilleryManager::StopBarrage()
{
    GetWorldTimerManager().ClearTimer(BarrageTimer);
    CurrentState = EBarrageState::Idle;
    UE_LOG(LogTemp, Log, TEXT("ArtilleryManager: Barrage stopped after %d shells."), ShellsFired);
}

void AArtilleryManager::FireShellAt(EShellType Type, FVector TargetLocation)
{
    if (!ShellClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArtilleryManager: ShellClass not set!"));
        return;
    }

    const FVector SpawnLoc = LaunchOrigin + FVector(0.0f, 0.0f, LaunchHeightOffset);

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.Owner = this;

    AArtilleryShell* Shell = GetWorld()->SpawnActor<AArtilleryShell>(ShellClass, SpawnLoc, FRotator::ZeroRotator, Params);
    if (!Shell) return;

    Shell->ShellData = ShellPresets::GetForType(Type);
    Shell->LaunchAtTarget(TargetLocation);

    ++ShellsFired;

    UE_LOG(LogTemp, Verbose, TEXT("ArtilleryManager: Fired %s at (%.0f, %.0f, %.0f). Total shells: %d"),
        *Shell->ShellData.DisplayName, TargetLocation.X, TargetLocation.Y, TargetLocation.Z, ShellsFired);
}

// ---------------------------------------------------------------------------
// Internal — barrage loop
// ---------------------------------------------------------------------------

void AArtilleryManager::FireNext()
{
    if (CurrentState != EBarrageState::Incoming) return;

    const EShellType Type = SelectShellType();
    const FVector    Target = GetNextTargetInZone();

    FireShellAt(Type, Target);
    ScheduleNext();
}

void AArtilleryManager::ScheduleNext()
{
    const float Delay = FMath::RandRange(MinIntervalSeconds, MaxIntervalSeconds);
    GetWorldTimerManager().SetTimer(BarrageTimer, this, &AArtilleryManager::FireNext, Delay, false);
}

EShellType AArtilleryManager::SelectShellType() const
{
    // Normalise probabilities then roll
    const float Total = Prob75mm + Prob77mm + Prob155mm + Prob210mm + Prob305mm;
    float Roll = FMath::RandRange(0.0f, Total);

    Roll -= Prob75mm;  if (Roll <= 0.0f) return EShellType::Shell75mm;
    Roll -= Prob77mm;  if (Roll <= 0.0f) return EShellType::Shell77mm;
    Roll -= Prob155mm; if (Roll <= 0.0f) return EShellType::Shell155mm;
    Roll -= Prob210mm; if (Roll <= 0.0f) return EShellType::Shell210mm;
    return EShellType::Shell305mm;
}

FVector AArtilleryManager::GetNextTargetInZone()
{
    FVector Target;

    switch (BarragePattern)
    {
    case EBarragePattern::LinearWalking:
        {
            // Shell "creeps" forward along the trench (X axis)
            WalkOffset += WalkingBarrageStepCm;
            // Bounce back when we reach the edge so it walks back and forth
            if (WalkOffset > TargetZoneHalfExtent.X)
            {
                WalkOffset = -TargetZoneHalfExtent.X;
            }
            Target = TargetZoneCenter
                   + FVector(WalkOffset, FMath::RandRange(-TargetZoneHalfExtent.Y, TargetZoneHalfExtent.Y), 0.0f);
        }
        break;

    case EBarragePattern::Concentrated:
        {
            // Gaussian scatter around centre — 80% of shells within 25% of extent
            const float ScatterX = FMath::RandRange(-TargetZoneHalfExtent.X * 0.25f, TargetZoneHalfExtent.X * 0.25f);
            const float ScatterY = FMath::RandRange(-TargetZoneHalfExtent.Y * 0.25f, TargetZoneHalfExtent.Y * 0.25f);
            Target = TargetZoneCenter + FVector(ScatterX, ScatterY, 0.0f);
        }
        break;

    case EBarragePattern::Random:
    default:
        Target = TargetZoneCenter + FVector(
            FMath::RandRange(-TargetZoneHalfExtent.X, TargetZoneHalfExtent.X),
            FMath::RandRange(-TargetZoneHalfExtent.Y, TargetZoneHalfExtent.Y),
            0.0f
        );
        break;
    }

    // Snap to terrain surface with a downward line trace
    FHitResult Hit;
    const FVector TraceStart = Target + FVector(0.0f, 0.0f, 5000.0f);
    const FVector TraceEnd   = Target - FVector(0.0f, 0.0f, 5000.0f);

    if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        Target = Hit.ImpactPoint;
    }

    return Target;
}

// ---------------------------------------------------------------------------
// Editor debug visualisation
// ---------------------------------------------------------------------------

#if WITH_EDITOR
void AArtilleryManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Draw target zone whenever a property changes in the editor
    if (GetWorld())
    {
        DrawDebugBox(GetWorld(), TargetZoneCenter, TargetZoneHalfExtent + FVector(0,0,200),
                     FColor::Red, false, 5.0f, 0, 8.0f);

        DrawDebugSphere(GetWorld(), LaunchOrigin, 200.0f, 12, FColor::Cyan, false, 5.0f);
        DrawDebugLine(GetWorld(), LaunchOrigin, TargetZoneCenter, FColor::Cyan, false, 5.0f, 0, 4.0f);
    }
}
#endif
