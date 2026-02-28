#include "Testing/TestDirector.h"

#include "Artillery/ArtilleryShell.h"
#include "Artillery/BarrageDirector.h"
#include "Player/ConcussionComponent.h"
#include "Terrain/DeformableTerrain.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATestDirector::ATestDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;  // 20 Hz HUD refresh

    // Default test zones — matches the layout in generate_test_level.py
    // Override in Details panel or let the Python script set them.
    //
    //   Zone 0: 75mm  × 1  — single small crater, baseline
    //   Zone 1: 155mm × 1  — medium crater, compare to zone 0
    //   Zone 2: 210mm × 1  — large crater, compare to zones 0+1
    //   Zone 3: 75mm  × 8  — rapid repeat on same spot, shows accumulation
    //   Zone 4: 155mm × 3  — overlapping medium craters
    //   Zone 5: 305mm × 1  — maximum single-shell impact

    FTestZone Z0; Z0.Label="Zone 0 — 75mm single";   Z0.Center=FVector(1000,2000,0); Z0.ShellType=EShellType::Shell75mm;  Z0.ShellCount=1; Z0.Description=TEXT("75mm Soixante-Quinze: 5m lethal, 2m crater");
    FTestZone Z1; Z1.Label="Zone 1 — 155mm single";  Z1.Center=FVector(3500,2000,0); Z1.ShellType=EShellType::Shell155mm; Z1.ShellCount=1; Z1.Description=TEXT("155mm Howitzer: 15m lethal, 6m crater");
    FTestZone Z2; Z2.Label="Zone 2 — 210mm single";  Z2.Center=FVector(7000,2000,0); Z2.ShellType=EShellType::Shell210mm; Z2.ShellCount=1; Z2.Description=TEXT("210mm Morser: 30m lethal, 12m crater");
    FTestZone Z3; Z3.Label="Zone 3 — 75mm barrage";  Z3.Center=FVector(1500,5000,0); Z3.ShellType=EShellType::Shell75mm;  Z3.ShellCount=8; Z3.InterShellDelay=4.0f; Z3.Description=TEXT("8x 75mm — watch ground accumulate");
    FTestZone Z4; Z4.Label="Zone 4 — 155mm barrage"; Z4.Center=FVector(4000,5000,0); Z4.ShellType=EShellType::Shell155mm; Z4.ShellCount=3; Z4.InterShellDelay=8.0f; Z4.Description=TEXT("3x 155mm — overlapping craters merge");
    FTestZone Z5; Z5.Label="Zone 5 — 305mm siege";   Z5.Center=FVector(7500,5000,0); Z5.ShellType=EShellType::Shell305mm; Z5.ShellCount=1; Z5.Description=TEXT("305mm Siege Mortar: 50m lethal, 25m crater");

    TestZones = { Z0, Z1, Z2, Z3, Z4, Z5 };
}

void ATestDirector::BeginPlay()
{
    Super::BeginPlay();

    // Auto-discover level actors if not manually linked
    if (!BarrageDirector)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABarrageDirector::StaticClass(), Found);
        if (Found.Num() > 0) BarrageDirector = Cast<ABarrageDirector>(Found[0]);
    }

    if (!Terrain)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeformableTerrain::StaticClass(), Found);
        if (Found.Num() > 0) Terrain = Cast<ADeformableTerrain>(Found[0]);
    }

    UE_LOG(LogTemp, Log, TEXT("TestDirector: Ready. BarrageDirector=%s, Terrain=%s"),
        BarrageDirector ? TEXT("found") : TEXT("MISSING — assign BP_BarrageDirector"),
        Terrain         ? TEXT("found") : TEXT("MISSING — assign BP_DeformableTerrain"));

    if (bAutoRunComparison)
    {
        FTimerHandle DelayHandle;
        GetWorldTimerManager().SetTimer(DelayHandle, [this]()
        {
            RunCalibreComparison();

            if (bAutoStartVerdunAfterComparison)
            {
                FTimerHandle VerdunHandle;
                GetWorldTimerManager().SetTimer(VerdunHandle, [this]()
                {
                    RunVerdunSequence();
                }, VerdunStartDelay, false);
            }
        }, 3.0f, false);
    }

    PrintLine(TEXT("[TestDirector] Ready — call RunCalibreComparison() or RunVerdunSequence()"), FColor::Cyan, 0);
}

// ---------------------------------------------------------------------------
// Tick — on-screen debug HUD
// ---------------------------------------------------------------------------

void ATestDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TestElapsedTime += DeltaTime;
    DrawHUD();
}

void ATestDirector::DrawHUD() const
{
    // --- Header ---
    PrintLine(TEXT("══════ ONLY THE DEAD — TEST DIRECTOR ══════"), FColor::White, 10);

    // --- Phase ---
    PrintLine(FString::Printf(TEXT("Phase: %s  |  T+%.0fs"),
        *CurrentPhaseLabel, TestElapsedTime), FColor::Yellow, 11);

    // --- Barrage ---
    const int32 ActiveWaves    = BarrageDirector ? BarrageDirector->GetActiveWaveCount() : 0;
    const float ShellsPerMin   = BarrageDirector ? BarrageDirector->GetCurrentShellsPerMinute() : 0.0f;
    const FColor BarrageColor  = ActiveWaves > 0 ? FColor::Orange : FColor::Silver;
    PrintLine(FString::Printf(TEXT("Barrage: %d active waves  |  %.0f shells/min"),
        ActiveWaves, ShellsPerMin), BarrageColor, 12);

    // --- Terrain ---
    if (Terrain)
    {
        PrintLine(FString::Printf(TEXT("Terrain: %d verts  |  deepest crater: %.0f cm"),
            Terrain->GetTotalVertexCount(), DeepestCraterDepthCm), FColor::Green, 13);
    }
    else
    {
        PrintLine(TEXT("Terrain: not found"), FColor::Red, 13);
    }

    // --- Shell stats ---
    PrintLine(FString::Printf(TEXT("Shells fired: %d  |  last: %s at (%.0f, %.0f)"),
        TotalShellsFired,
        *UEnum::GetDisplayValueAsText(LastShellType).ToString(),
        LastImpactLocation.X, LastImpactLocation.Y), FColor::Cyan, 14);

    // --- Player shell shock ---
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (Player)
    {
        UConcussionComponent* Concussion = Player->FindComponentByClass<UConcussionComponent>();
        if (Concussion)
        {
            const float Level = Concussion->ShellShockLevel;
            FColor ShockColor = Level < 0.25f ? FColor::Green :
                                Level < 0.50f ? FColor::Yellow :
                                Level < 0.75f ? FColor::Orange : FColor::Red;
            const FString StateStr = UEnum::GetDisplayValueAsText(Concussion->CurrentState).ToString();
            PrintLine(FString::Printf(TEXT("Shell Shock: %.2f  (%s)  SpeedMult=%.0f%%"),
                Level, *StateStr, Concussion->MovementSpeedMultiplier * 100.0f),
                ShockColor, 15);
        }
    }

    // --- Controls reminder ---
    PrintLine(TEXT("──────────────────────────────────────────"), FColor::Silver, 16);
    PrintLine(TEXT("Blueprint: FireAtZone(N) | RunCalibreComparison | RunVerdunSequence | ResetAll"),
              FColor::Silver, 17);
}

void ATestDirector::PrintLine(const FString& Text, FColor Color, int32 Key) const
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(Key, 0.1f, Color, Text);
    }
}

// ---------------------------------------------------------------------------
// Shell firing
// ---------------------------------------------------------------------------

void ATestDirector::FireAtZone(int32 ZoneIndex)
{
    if (!TestZones.IsValidIndex(ZoneIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("TestDirector: Zone %d does not exist"), ZoneIndex);
        return;
    }

    const FTestZone& Zone = TestZones[ZoneIndex];
    FireShell(Zone.ShellType, Zone.Center);

    PrintLine(FString::Printf(TEXT("[TEST] %s — %s"), *Zone.Label, *Zone.Description),
              FColor::Yellow, 50);
}

void ATestDirector::RunZoneTest(int32 ZoneIndex)
{
    if (!TestZones.IsValidIndex(ZoneIndex)) return;

    const FTestZone& Zone = TestZones[ZoneIndex];
    CurrentPhaseLabel = FString::Printf(TEXT("Zone test: %s"), *Zone.Label);

    PrintLine(FString::Printf(TEXT("[TEST] Starting: %s — firing %d × %s"),
        *Zone.Label, Zone.ShellCount, *UEnum::GetDisplayValueAsText(Zone.ShellType).ToString()),
        FColor::Yellow, 50);

    FireShellSalvo(Zone.ShellType, Zone.Center, Zone.ShellCount, Zone.InterShellDelay);
}

void ATestDirector::RunCalibreComparison()
{
    CurrentPhaseLabel = TEXT("Calibre Comparison");
    PrintLine(TEXT("[TEST] === CALIBRE COMPARISON: 75mm / 155mm / 210mm side by side ==="), FColor::Cyan, 50);

    // Stagger by 1s so flashes don't all overlap
    // Zone 0 = 75mm, Zone 1 = 155mm, Zone 2 = 210mm
    const float Stagger = 1.5f;

    if (TestZones.IsValidIndex(0)) FireShell(TestZones[0].ShellType, TestZones[0].Center);

    if (TestZones.IsValidIndex(1))
    {
        FTimerHandle H1;
        GetWorldTimerManager().SetTimer(H1, [this]()
        {
            FireShell(TestZones[1].ShellType, TestZones[1].Center);
        }, Stagger, false);
        ActiveTimers.Add(H1);
    }

    if (TestZones.IsValidIndex(2))
    {
        FTimerHandle H2;
        GetWorldTimerManager().SetTimer(H2, [this]()
        {
            FireShell(TestZones[2].ShellType, TestZones[2].Center);
        }, Stagger * 2.0f, false);
        ActiveTimers.Add(H2);
    }

    // Then fire accumulation tests after comparison shells have landed
    // 155mm whistle = 8s flight time + Stagger*2 + 5s buffer
    const float AccumulationDelay = ShellPresets::Make155mm().IncomingWhistleSeconds + Stagger * 2.0f + 5.0f;

    // (AccumulationDelay estimated from 155mm whistle time — the longest of the three)
    FTimerHandle H3;
    GetWorldTimerManager().SetTimer(H3, [this]()
    {
        CurrentPhaseLabel = TEXT("Accumulation test — craters overlap");
        PrintLine(TEXT("[TEST] === ACCUMULATION TEST: watch ground churn ==="), FColor::Orange, 50);
        if (TestZones.IsValidIndex(3)) RunZoneTest(3);
        if (TestZones.IsValidIndex(4))
        {
            FTimerHandle H4;
            GetWorldTimerManager().SetTimer(H4, [this]()
            {
                RunZoneTest(4);
            }, 5.0f, false);
            ActiveTimers.Add(H4);
        }
    }, AccumulationDelay, false);
    ActiveTimers.Add(H3);
}

void ATestDirector::RunVerdunSequence()
{
    if (!BarrageDirector)
    {
        UE_LOG(LogTemp, Warning, TEXT("TestDirector: No BarrageDirector — cannot run Verdun sequence"));
        PrintLine(TEXT("[ERROR] BarrageDirector not found!"), FColor::Red, 50);
        return;
    }

    BarrageDirector->TrenchCenterLocation = VerdunSequenceTarget;
    BarrageDirector->StartVerdunOpeningSequence();

    CurrentPhaseLabel = TEXT("Verdun Opening Sequence — Phase 1: Registration");
    PrintLine(TEXT("[TEST] === VERDUN OPENING SEQUENCE STARTED ==="), FColor::Orange, 50);
    PrintLine(TEXT("  Feb 21, 1916 — 1,400 guns — heard in London"), FColor::Orange, 51);
    PrintLine(TEXT("  Watch: Registration → Harassing → Sustained → Saturation → DRUMFIRE"), FColor::Yellow, 52);

    // Update phase label as time progresses
    const TArray<TPair<float, FString>> PhaseLabels = {
        { 32.0f,  TEXT("Phase 2: Harassing fire") },
        { 95.0f,  TEXT("Phase 3: Sustained + mixed calibres") },
        { 190.0f, TEXT("Phase 4: SATURATION — no safe ground") },
        { 285.0f, TEXT("Phase 5: TROMMELFEUER — DRUMFIRE") },
    };

    for (const auto& Phase : PhaseLabels)
    {
        FTimerHandle H;
        FString Label = Phase.Value;
        GetWorldTimerManager().SetTimer(H, [this, Label]()
        {
            CurrentPhaseLabel = Label;
        }, Phase.Key, false);
        ActiveTimers.Add(H);
    }
}

void ATestDirector::RunCreepingBarrage()
{
    if (!BarrageDirector)
    {
        PrintLine(TEXT("[ERROR] BarrageDirector not found!"), FColor::Red, 50);
        return;
    }

    CurrentPhaseLabel = TEXT("Creeping Barrage — advancing at infantry pace");
    PrintLine(TEXT("[TEST] === CREEPING BARRAGE ==="), FColor::Orange, 50);
    PrintLine(TEXT("  Line advancing at 50m/min — stay ahead of it or die"), FColor::Yellow, 51);

    BarrageDirector->StartCreepingBarrage(CreepStart, CreepEnd, 90.0f);
}

void ATestDirector::RunSaturationTest()
{
    if (!BarrageDirector)
    {
        PrintLine(TEXT("[ERROR] BarrageDirector not found!"), FColor::Red, 50);
        return;
    }

    CurrentPhaseLabel = TEXT("Saturation — 60 shells/min for 30s");
    PrintLine(TEXT("[TEST] === SATURATION TEST (30s, 60 shells/min) ==="), FColor::Red, 50);

    FBarrageWave W;
    W.Type              = EBarrageType::Saturation;
    W.TargetCenter      = VerdunSequenceTarget;
    W.AccuracyCEP       = 3500.0f;
    W.ShellsPerMinute   = 60.0f;
    W.DurationSeconds   = 30.0f;
    W.HeavyShellFraction = 0.2f;
    W.HeavyShellClass   = ShellClass_155mm;
    BarrageDirector->StartBarrage(W);
}

void ATestDirector::ResetAll()
{
    // Stop all barrages
    if (BarrageDirector)
    {
        BarrageDirector->StopAllBarrages();
    }

    // Flatten terrain
    if (Terrain)
    {
        Terrain->ResetDeformation();
    }

    // Clear shell shock on all characters
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Characters);
    for (AActor* A : Characters)
    {
        UConcussionComponent* C = A->FindComponentByClass<UConcussionComponent>();
        if (C) C->ClearShellShock();
    }

    // Cancel all pending test timers
    for (FTimerHandle& H : ActiveTimers)
    {
        GetWorldTimerManager().ClearTimer(H);
    }
    ActiveTimers.Empty();

    // Reset stats
    TotalShellsFired     = 0;
    DeepestCraterDepthCm = 0.0f;
    CurrentPhaseLabel    = TEXT("Reset — Idle");
    TestElapsedTime      = 0.0f;

    PrintLine(TEXT("[TEST] RESET COMPLETE — terrain flat, barrages stopped"), FColor::Green, 50);
    UE_LOG(LogTemp, Log, TEXT("TestDirector: Full reset complete."));
}

// ---------------------------------------------------------------------------
// Internal shell firing
// ---------------------------------------------------------------------------

void ATestDirector::FireShell(EShellType Type, FVector Target)
{
    TSubclassOf<AArtilleryShell> ShellClass = GetShellClass(Type);
    if (!ShellClass)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TestDirector: No shell class assigned for %s"),
            *UEnum::GetDisplayValueAsText(Type).ToString());
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.Owner = this;

    AArtilleryShell* Shell = GetWorld()->SpawnActor<AArtilleryShell>(
        ShellClass, GunOrigin, FRotator::ZeroRotator, Params);

    if (!Shell) return;

    Shell->LaunchAtTarget(Target);
    Shell->OnShellDetonated.AddDynamic(this, &ATestDirector::OnShellDetonated);

    LastShellType = Type;
    ++TotalShellsFired;

    UE_LOG(LogTemp, Verbose,
        TEXT("TestDirector: Fired %s → (%.0f, %.0f, %.0f)"),
        *UEnum::GetDisplayValueAsText(Type).ToString(),
        Target.X, Target.Y, Target.Z);
}

void ATestDirector::FireShellSalvo(EShellType Type, FVector Target, int32 Count, float Delay)
{
    for (int32 i = 0; i < Count; ++i)
    {
        // Scatter within 150cm of centre so multiple impacts are distinct
        const float Angle  = FMath::FRandRange(0.0f, TWO_PI);
        const float Spread = (i == 0) ? 0.0f : FMath::FRandRange(50.0f, 150.0f);
        const FVector ImpactTarget = Target + FVector(
            Spread * FMath::Cos(Angle),
            Spread * FMath::Sin(Angle),
            0.0f);

        if (i == 0)
        {
            FireShell(Type, ImpactTarget);
        }
        else
        {
            FTimerHandle H;
            GetWorldTimerManager().SetTimer(H, [this, Type, ImpactTarget]()
            {
                FireShell(Type, ImpactTarget);
            }, Delay * i, false);
            ActiveTimers.Add(H);
        }
    }
}

TSubclassOf<AArtilleryShell> ATestDirector::GetShellClass(EShellType Type) const
{
    switch (Type)
    {
    case EShellType::Shell75mm:  return ShellClass_75mm;
    case EShellType::Shell155mm: return ShellClass_155mm;
    case EShellType::Shell210mm: return ShellClass_210mm;
    case EShellType::Shell305mm: return ShellClass_305mm;
    default:                     return ShellClass_75mm;
    }
}

void ATestDirector::OnShellDetonated(AArtilleryShell* Shell, FVector ImpactLocation)
{
    LastImpactLocation = ImpactLocation;
    LastShellType      = Shell->ShellData.Type;

    // Sample crater depth at impact point
    if (Terrain)
    {
        const float Depth = FMath::Abs(Terrain->GetDisplacementAtLocation(ImpactLocation));
        DeepestCraterDepthCm = FMath::Max(DeepestCraterDepthCm, Depth);
    }
}
