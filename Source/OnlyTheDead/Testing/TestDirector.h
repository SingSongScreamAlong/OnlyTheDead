#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Artillery/ArtilleryTypes.h"
#include "TestDirector.generated.h"

class ABarrageDirector;
class ADeformableTerrain;
class AArtilleryShell;

// ---------------------------------------------------------------------------
// FTestZone — a named target area in the test level
// ---------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FTestZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Label;

    // World-space centre of the zone (where shells should land)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center = FVector::ZeroVector;

    // Shell type to fire into this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EShellType ShellType = EShellType::Shell75mm;

    // How many shells to fire per test run
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShellCount = 1;

    // Delay between shells in a multi-shell test (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InterShellDelay = 3.0f;

    // Description printed to screen when this zone's test runs
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
};

// ---------------------------------------------------------------------------
// ATestDirector
//
// Drop this actor in the test level. It provides:
//
//   On-screen HUD:
//     - Current shell shock state + level (0.0-1.0)
//     - Active barrage waves + shells/min
//     - Terrain: vertices displaced, deepest crater, total area churned
//     - Last shell detonated: type, impact location, crater depth
//     - FPS counter
//
//   Blueprint-callable test API:
//     - FireAtZone(ZoneIndex)       — fire one shell at the named zone
//     - RunZoneTest(ZoneIndex)      — full multi-shell zone test sequence
//     - RunCalibreComparison()      — fire 75mm, 155mm, 210mm side by side
//     - RunVerdunSequence()         — start the BarrageDirector escalation
//     - RunCreepingBarrage()        — creeping barrage across the field
//     - RunSaturationTest()         — 30s of 60 shells/min on the impact field
//     - ResetTerrain()              — flatten all terrain, stop all barrages
//
//   Auto-test mode:
//     - bAutoRunSequence = true → runs RunCalibreComparison() at BeginPlay,
//       then starts the Verdun sequence after a configurable delay
//
// The on-screen display uses AddOnScreenDebugMessage — visible in PIE
// without any UI Blueprint setup needed.
// ---------------------------------------------------------------------------
UCLASS()
class ONLYTHEDEAD_API ATestDirector : public AActor
{
    GENERATED_BODY()

public:
    ATestDirector();

    // ---- Test zones — configure in Details panel ----

    // Pre-defined test zones (filled by generate_test_level.py, editable in editor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Zones")
    TArray<FTestZone> TestZones;

    // Reference zone — the ABarrageDirector fires at this during full tests
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Zones")
    FVector VerdunSequenceTarget = FVector(4000.0f, 5000.0f, 0.0f);

    // Creeping barrage: start and end of the advance line
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Zones")
    FVector CreepStart = FVector(1000.0f, 7000.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Zones")
    FVector CreepEnd   = FVector(9000.0f, 7000.0f, 0.0f);

    // ---- Linked level actors (auto-discovered at BeginPlay if not set) ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Actors")
    ABarrageDirector* BarrageDirector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Actors")
    ADeformableTerrain* Terrain;

    // Shell class for each calibre (assign in Blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Shells")
    TSubclassOf<AArtilleryShell> ShellClass_75mm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Shells")
    TSubclassOf<AArtilleryShell> ShellClass_155mm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Shells")
    TSubclassOf<AArtilleryShell> ShellClass_210mm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Shells")
    TSubclassOf<AArtilleryShell> ShellClass_305mm;

    // Where shells originate (off-map gun position)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Shells")
    FVector GunOrigin = FVector(30000.0f, 4000.0f, 6000.0f);

    // ---- Auto-test ----

    // If true: RunCalibreComparison() fires at BeginPlay automatically
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Auto")
    bool bAutoRunComparison = false;

    // If true: starts Verdun sequence N seconds after comparison
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Auto")
    bool bAutoStartVerdunAfterComparison = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Auto",
              meta = (ClampMin = "5", ClampMax = "120"))
    float VerdunStartDelay = 30.0f;

    // ---- Test API ----

    // Fire one shell of the correct type at a specific zone
    UFUNCTION(BlueprintCallable, Category = "Test")
    void FireAtZone(int32 ZoneIndex);

    // Fire the full zone test sequence (multiple shells with delays)
    UFUNCTION(BlueprintCallable, Category = "Test")
    void RunZoneTest(int32 ZoneIndex);

    // Fire one of each calibre (75mm, 155mm, 210mm) at zones 0, 1, 2 simultaneously
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Test")
    void RunCalibreComparison();

    // Start the BarrageDirector's Verdun opening escalation sequence
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Test")
    void RunVerdunSequence();

    // Creeping barrage across the test field
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Test")
    void RunCreepingBarrage();

    // 30-second saturation test (60 shells/min) at the impact field
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Test")
    void RunSaturationTest();

    // Flatten all terrain, clear all barrages, reset shell shock
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Test")
    void ResetAll();

    // ---- Stats (readable from Blueprint for custom HUD) ----

    UFUNCTION(BlueprintPure, Category = "Test")
    int32 GetTotalShellsFired() const { return TotalShellsFired; }

    UFUNCTION(BlueprintPure, Category = "Test")
    float GetDeepestCraterDepth() const { return DeepestCraterDepthCm; }

    UFUNCTION(BlueprintPure, Category = "Test")
    FString GetCurrentPhaseLabel() const { return CurrentPhaseLabel; }

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    // Fire a single shell of the given type at the target position
    void FireShell(EShellType Type, FVector Target);

    // Fire multiple shells at a target with delays
    void FireShellSalvo(EShellType Type, FVector Target, int32 Count, float Delay);

    // Shell class lookup
    TSubclassOf<AArtilleryShell> GetShellClass(EShellType Type) const;

    // Register for shell detonation events (to track stats)
    UFUNCTION()
    void OnShellDetonated(AArtilleryShell* Shell, FVector ImpactLocation);

    // Screen debug output
    void DrawHUD() const;
    void PrintLine(const FString& Text, FColor Color, int32 Key) const;

    // Stats
    int32 TotalShellsFired       = 0;
    float DeepestCraterDepthCm   = 0.0f;
    FVector LastImpactLocation   = FVector::ZeroVector;
    EShellType LastShellType     = EShellType::Shell75mm;
    FString CurrentPhaseLabel    = TEXT("Idle");
    float TestElapsedTime        = 0.0f;

    // Salvo firing state
    TArray<FTimerHandle> ActiveTimers;
};
