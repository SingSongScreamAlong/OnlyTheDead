#include "Artillery/BarrageDirector.h"
#include "Artillery/ArtilleryShell.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

ABarrageDirector::ABarrageDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ABarrageDirector::BeginPlay()
{
    Super::BeginPlay();

    if (!bAutoStartOnPlay) return;

    // Optional persistent wide-area harassment — runs for the entire session.
    // Starts immediately (no delay) so the first distant impact is audible
    // while the player is still orienting. Low rate, wide scatter: occasional
    // incoming that could land anywhere. "No safe ground."
    if (bAutoWideAreaHarassment)
    {
        FBarrageWave Wide;
        Wide.Type            = EBarrageType::Harassing;
        Wide.TargetCenter    = TrenchCenterLocation;
        Wide.AccuracyCEP     = WideAreaRadiusCm;
        Wide.ShellsPerMinute = WideAreaShellsPerMinute;
        Wide.DurationSeconds = -1.0f;    // Never stops
        Wide.HeavyShellFraction = 0.15f; // 1 in 7 lands as a heavier calibre
        Wide.HeavyShellClass = HeavyShellClass;
        StartBarrage(Wide);
    }

    // Schedule the Verdun opening sequence after the initial silence.
    // Phase 1 starts with slow registration shots — the player hears them
    // land one by one, then the escalation begins.
    FTimerHandle StartHandle;
    GetWorldTimerManager().SetTimer(StartHandle, [this]()
    {
        StartVerdunOpeningSequence();
    }, AutoStartDelaySec, false);

    // Loop: restart the full sequence when it finishes (~5 min).
    // Drumfire phase (Phase 5) runs until stopped, so in practice the loop
    // only matters if the player somehow outlasts the Drumfire phase.
    if (bLoopSequence)
    {
        // Sequence total duration: ~300s to reach Drumfire + buffer
        const float SequenceDuration = 285.0f + 30.0f;  // 315s = ~5.25min
        FTimerHandle LoopHandle;
        GetWorldTimerManager().SetTimer(LoopHandle, [this]()
        {
            // Stop all barrage waves then replay from the beginning.
            // The wide-area harassment started above is already infinite (Duration=-1)
            // so we stop everything and let BeginPlay logic below restart it.
            StopAllBarrages();

            if (bAutoWideAreaHarassment)
            {
                FBarrageWave Wide;
                Wide.Type            = EBarrageType::Harassing;
                Wide.TargetCenter    = TrenchCenterLocation;
                Wide.AccuracyCEP     = WideAreaRadiusCm;
                Wide.ShellsPerMinute = WideAreaShellsPerMinute;
                Wide.DurationSeconds = -1.0f;
                Wide.HeavyShellFraction = 0.15f;
                Wide.HeavyShellClass = HeavyShellClass;
                StartBarrage(Wide);
            }

            StartVerdunOpeningSequence();
        }, AutoStartDelaySec + SequenceDuration, true);  // Repeating every cycle
    }

    UE_LOG(LogTemp, Log, TEXT("BarrageDirector: Auto-start scheduled in %.0fs. "
        "Wide-area harassment: %s. Loop: %s."),
        AutoStartDelaySec,
        bAutoWideAreaHarassment ? TEXT("yes") : TEXT("no"),
        bLoopSequence ? TEXT("yes") : TEXT("no"));
}

void ABarrageDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAllBarrages();
    Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Core barrage management
// ---------------------------------------------------------------------------

int32 ABarrageDirector::StartBarrage(const FBarrageWave& Wave)
{
    if (!Wave.ShellClass)
    {
        if (!DefaultShellClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("BarrageDirector: No shell class assigned — cannot start barrage."));
            return -1;
        }
    }

    FActiveWave Active;
    Active.Config             = Wave;
    Active.WaveID             = NextWaveID++;
    Active.CurrentTargetCenter = Wave.TargetCenter;
    Active.StartTime          = GetWorld()->GetTimeSeconds();

    // Use defaults if wave doesn't specify a shell class
    if (!Active.Config.ShellClass)    Active.Config.ShellClass    = DefaultShellClass;
    if (!Active.Config.HeavyShellClass) Active.Config.HeavyShellClass = HeavyShellClass;

    const int32 AssignedID = Active.WaveID;

    // Fire interval from rate
    const float IntervalSeconds = 60.0f / FMath::Max(Wave.ShellsPerMinute, 1.0f);

    ActiveWaves.Add(MoveTemp(Active));

    // Start recurring fire timer
    FTimerDelegate Delegate;
    Delegate.BindUObject(this, &ABarrageDirector::FireShellForWave, AssignedID);

    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, Delegate, IntervalSeconds, true, 0.0f);

    // Assign timer handle to the active wave entry
    for (FActiveWave& W : ActiveWaves)
    {
        if (W.WaveID == AssignedID)
        {
            W.FireTimer = Handle;
            break;
        }
    }

    // If duration is finite, schedule stop
    if (Wave.DurationSeconds > 0.0f)
    {
        FTimerHandle StopHandle;
        FTimerDelegate StopDelegate;
        StopDelegate.BindUObject(this, &ABarrageDirector::StopBarrage, AssignedID);
        GetWorldTimerManager().SetTimer(StopHandle, StopDelegate, Wave.DurationSeconds, false);
    }

    UE_LOG(LogTemp, Log,
        TEXT("BarrageDirector: Started wave %d (%s) — %.0f shells/min, duration %.0fs"),
        AssignedID,
        *UEnum::GetValueAsString(Wave.Type),
        Wave.ShellsPerMinute,
        Wave.DurationSeconds);

    return AssignedID;
}

void ABarrageDirector::StopBarrage(int32 WaveID)
{
    for (int32 i = ActiveWaves.Num() - 1; i >= 0; --i)
    {
        if (ActiveWaves[i].WaveID == WaveID)
        {
            GetWorldTimerManager().ClearTimer(ActiveWaves[i].FireTimer);
            UE_LOG(LogTemp, Log, TEXT("BarrageDirector: Stopped wave %d"), WaveID);
            ActiveWaves.RemoveAt(i);
            return;
        }
    }
}

void ABarrageDirector::StopAllBarrages()
{
    for (FActiveWave& Wave : ActiveWaves)
    {
        GetWorldTimerManager().ClearTimer(Wave.FireTimer);
    }
    ActiveWaves.Empty();
    UE_LOG(LogTemp, Log, TEXT("BarrageDirector: All barrages stopped."));
}

float ABarrageDirector::GetCurrentShellsPerMinute() const
{
    float Total = 0.0f;
    for (const FActiveWave& W : ActiveWaves)
    {
        Total += W.Config.ShellsPerMinute;
    }
    return Total;
}

// ---------------------------------------------------------------------------
// Shell firing
// ---------------------------------------------------------------------------

void ABarrageDirector::FireShellForWave(int32 WaveID)
{
    FActiveWave* Wave = nullptr;
    for (FActiveWave& W : ActiveWaves)
    {
        if (W.WaveID == WaveID) { Wave = &W; break; }
    }
    if (!Wave) return;

    // (elapsed time computed inline when needed — not stored on FActiveWave)

    // Advance creeping barrage target
    if (Wave->Config.Type == EBarrageType::Creeping)
    {
        const float FireInterval = 60.0f / FMath::Max(Wave->Config.ShellsPerMinute, 1.0f);
        Wave->CurrentTargetCenter +=
            Wave->Config.CreepDirection * Wave->Config.CreepSpeedCmPerSec * FireInterval;
    }

    // Decide shell class — mix in heavy shells at specified fraction
    TSubclassOf<AArtilleryShell> ChosenClass = Wave->Config.ShellClass;
    if (Wave->Config.HeavyShellClass && Wave->Config.HeavyShellFraction > 0.0f)
    {
        if (FMath::FRand() < Wave->Config.HeavyShellFraction)
        {
            ChosenClass = Wave->Config.HeavyShellClass;
        }
    }

    if (!ChosenClass) return;

    // Calculate target impact point
    FVector ImpactPoint = CalculateImpactPoint(*Wave);
    FVector LaunchOrigin = GetLaunchOrigin(*Wave);

    // Spawn shell
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.Owner = this;

    AArtilleryShell* Shell = GetWorld()->SpawnActor<AArtilleryShell>(
        ChosenClass,
        LaunchOrigin,
        FRotator::ZeroRotator,
        Params
    );

    if (Shell)
    {
        Shell->LaunchAtTarget(ImpactPoint);
    }
}

FVector ABarrageDirector::CalculateImpactPoint(const FActiveWave& Wave) const
{
    FVector BasePoint;

    switch (Wave.Config.Type)
    {
    case EBarrageType::Creeping:
    {
        // Impact along the current creep line — random lateral offset within CreepWidth
        const FVector Perpendicular = FVector::CrossProduct(
            Wave.Config.CreepDirection, FVector::UpVector).GetSafeNormal();
        const float LateralOffset = FMath::FRandRange(
            -Wave.Config.CreepWidth * 0.5f, Wave.Config.CreepWidth * 0.5f);
        BasePoint = Wave.CurrentTargetCenter + Perpendicular * LateralOffset;
        break;
    }

    case EBarrageType::Box:
    {
        // Pick one of the four box sides, then random point along it
        const int32 Side = FMath::RandRange(0, 3);
        const FVector& HE = Wave.Config.BoxHalfExtent;
        switch (Side)
        {
        case 0: BasePoint = Wave.Config.TargetCenter + FVector( HE.X, FMath::FRandRange(-HE.Y, HE.Y), 0); break;
        case 1: BasePoint = Wave.Config.TargetCenter + FVector(-HE.X, FMath::FRandRange(-HE.Y, HE.Y), 0); break;
        case 2: BasePoint = Wave.Config.TargetCenter + FVector(FMath::FRandRange(-HE.X, HE.X),  HE.Y, 0); break;
        case 3: BasePoint = Wave.Config.TargetCenter + FVector(FMath::FRandRange(-HE.X, HE.X), -HE.Y, 0); break;
        }
        break;
    }

    case EBarrageType::Registration:
    {
        // Single shots at or very near the exact target — registration is precise
        BasePoint = Wave.Config.TargetCenter;
        break;
    }

    case EBarrageType::Drumfire:
    case EBarrageType::Saturation:
    {
        // Random within the target zone, weighted slightly toward centre
        // (batteries aren't perfectly random — they saturate the zone)
        const float R = Wave.Config.AccuracyCEP * FMath::Sqrt(FMath::FRand());
        const float Angle = FMath::FRandRange(0.0f, TWO_PI);
        BasePoint = Wave.Config.TargetCenter + FVector(R * FMath::Cos(Angle), R * FMath::Sin(Angle), 0);
        break;
    }

    default: // Harassing
    {
        // Random scatter — unpredictable, anywhere within range
        const float R = FMath::FRandRange(0.0f, Wave.Config.AccuracyCEP);
        const float Angle = FMath::FRandRange(0.0f, TWO_PI);
        BasePoint = Wave.Config.TargetCenter + FVector(R * FMath::Cos(Angle), R * FMath::Sin(Angle), 0);
        break;
    }
    }

    // Apply CEP scatter on top of pattern position
    return ApplyScatter(BasePoint, Wave.Config.AccuracyCEP * 0.3f);
}

FVector ABarrageDirector::GetLaunchOrigin(const FActiveWave& Wave) const
{
    if (GunOrigins.Num() == 0) return DefaultGunOrigin;

    const_cast<ABarrageDirector*>(this)->LastGunOriginIndex =
        (LastGunOriginIndex + 1) % GunOrigins.Num();
    return GunOrigins[LastGunOriginIndex];
}

FVector ABarrageDirector::ApplyScatter(FVector IdealPoint, float CEP) const
{
    // Gaussian scatter approximated by two uniform samples (Box-Muller-lite)
    const float U1 = FMath::FRand();
    const float U2 = FMath::FRand();
    const float R  = CEP * FMath::Sqrt(-2.0f * FMath::Loge(FMath::Max(U1, SMALL_NUMBER)));
    const float A  = TWO_PI * U2;

    return IdealPoint + FVector(R * FMath::Cos(A), R * FMath::Sin(A), 0.0f);
}

// ---------------------------------------------------------------------------
// Historical presets
// ---------------------------------------------------------------------------

void ABarrageDirector::StartVerdunOpeningSequence()
{
    const FVector Target = TrenchCenterLocation;

    UE_LOG(LogTemp, Log, TEXT("BarrageDirector: Starting Verdun Opening Sequence."));
    UE_LOG(LogTemp, Log, TEXT("  February 21, 1916 — 1,400 guns, 9 hours of barrage."));

    // ---- Phase 1: Registration (0s) ----
    // Ranging shots. The defenders hear them — and know what's coming.
    {
        FBarrageWave W;
        W.Type              = EBarrageType::Registration;
        W.TargetCenter      = Target;
        W.AccuracyCEP       = 1500.0f;    // Precise — they've pre-registered their guns
        W.ShellsPerMinute   = 3.0f;       // One every 20 seconds — deliberate
        W.DurationSeconds   = 30.0f;
        StartBarrage(W);
    }

    // ---- Phase 2: Harassing fire (30s) ----
    // Unpredictable, scattershot. You can't rest. You can't think.
    FTimerHandle Phase2Handle;
    GetWorldTimerManager().SetTimer(Phase2Handle, [this, Target]()
    {
        FBarrageWave W;
        W.Type              = EBarrageType::Harassing;
        W.TargetCenter      = Target;
        W.AccuracyCEP       = 4000.0f;
        W.ShellsPerMinute   = 8.0f;
        W.DurationSeconds   = 60.0f;
        StartBarrage(W);
        UE_LOG(LogTemp, Log, TEXT("BarrageDirector: Phase 2 — Harassing fire begins."));
    }, 32.0f, false);

    // ---- Phase 3: Sustained barrage with mixed calibres (90s) ----
    // Field guns and heavy howitzers working together. Different sounds,
    // different impacts, no pattern the brain can lock onto.
    FTimerHandle Phase3Handle;
    GetWorldTimerManager().SetTimer(Phase3Handle, [this, Target]()
    {
        // Light guns — many
        {
            FBarrageWave W;
            W.Type              = EBarrageType::Saturation;
            W.TargetCenter      = Target;
            W.AccuracyCEP       = 3000.0f;
            W.ShellsPerMinute   = 18.0f;
            W.DurationSeconds   = 90.0f;
            StartBarrage(W);
        }
        // Heavy howitzers — fewer but devastating
        {
            FBarrageWave W;
            W.Type              = EBarrageType::Saturation;
            W.TargetCenter      = Target;
            W.AccuracyCEP       = 5000.0f;
            W.ShellsPerMinute   = 4.0f;
            W.DurationSeconds   = 90.0f;
            W.ShellClass        = HeavyShellClass;
            StartBarrage(W);
        }
        UE_LOG(LogTemp, Log, TEXT("BarrageDirector: Phase 3 — Mixed calibre sustained barrage."));
    }, 95.0f, false);

    // ---- Phase 4: Saturation (180s) ----
    // The ground itself is the weapon now.
    FTimerHandle Phase4Handle;
    GetWorldTimerManager().SetTimer(Phase4Handle, [this, Target]()
    {
        FBarrageWave W;
        W.Type              = EBarrageType::Saturation;
        W.TargetCenter      = Target;
        W.AccuracyCEP       = 4000.0f;
        W.ShellsPerMinute   = 60.0f;
        W.DurationSeconds   = 90.0f;
        W.HeavyShellFraction = 0.25f;
        W.HeavyShellClass   = HeavyShellClass;
        StartBarrage(W);
        UE_LOG(LogTemp, Log, TEXT("BarrageDirector: Phase 4 — Saturation barrage. 60 shells/min."));
    }, 190.0f, false);

    // ---- Phase 5: Drumfire / Trommelfeuer (270s+) ----
    // Individual explosions are indistinguishable. A continuous roar.
    // The ground returns to geological time — nothing lives on the surface.
    FTimerHandle Phase5Handle;
    GetWorldTimerManager().SetTimer(Phase5Handle, [this, Target]()
    {
        FBarrageWave W;
        W.Type              = EBarrageType::Drumfire;
        W.TargetCenter      = Target;
        W.AccuracyCEP       = 5000.0f;
        W.ShellsPerMinute   = 120.0f;    // One every 0.5 seconds
        W.DurationSeconds   = -1.0f;     // Continuous until manually stopped
        W.HeavyShellFraction = 0.30f;
        W.HeavyShellClass   = HeavyShellClass;
        StartBarrage(W);
        UE_LOG(LogTemp, Log, TEXT("BarrageDirector: Phase 5 — DRUMFIRE. Trommelfeuer. 120 shells/min."));
        UE_LOG(LogTemp, Log, TEXT("  \"The sound of the cannonade was heard in London.\""));
    }, 285.0f, false);
}

void ABarrageDirector::StartCreepingBarrage(FVector StartLine, FVector TargetLine, float DurationSeconds)
{
    FBarrageWave W;
    W.Type              = EBarrageType::Creeping;
    W.TargetCenter      = StartLine;
    W.AccuracyCEP       = 2500.0f;
    W.ShellsPerMinute   = 20.0f;
    W.DurationSeconds   = DurationSeconds;
    W.CreepDirection    = (TargetLine - StartLine).GetSafeNormal();
    W.CreepSpeedCmPerSec = 833.0f;  // 50m/min — standard creep pace
    W.CreepWidth        = 6000.0f;  // 60m wide
    StartBarrage(W);
}

void ABarrageDirector::StartBoxBarrage(FVector Center, FVector HalfExtent, float DurationSeconds)
{
    FBarrageWave W;
    W.Type              = EBarrageType::Box;
    W.TargetCenter      = Center;
    W.AccuracyCEP       = 2000.0f;
    W.ShellsPerMinute   = 15.0f;
    W.DurationSeconds   = DurationSeconds;
    W.BoxHalfExtent     = HalfExtent;
    W.HeavyShellFraction = 0.2f;
    W.HeavyShellClass   = HeavyShellClass;
    StartBarrage(W);
}
