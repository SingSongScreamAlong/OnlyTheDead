#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Artillery/ArtilleryTypes.h"
#include "BarrageDirector.generated.h"

class AArtilleryShell;

// ---------------------------------------------------------------------------
// EBarrageType — how shells are distributed in space and time
//
// Historical context:
//  Harassing   — sporadic fire to deny rest, prevent supply movement, erode morale
//  Creeping    — a moving wall of fire advancing with the infantry assault, keeping
//                defenders pinned until the last moment (developed mid-1915)
//  Box         — surrounds a section of trench to prevent relief or retreat;
//                effectively a death trap for anyone inside
//  Saturation  — maximum density over an area, no safe ground, overwhelm by volume
//  Registration— ranging shots before a major barrage, alerted defenders but was
//                necessary to calibrate guns
//  Drumfire    — "Trommelfeuer": fire so intense individual impacts are
//                indistinguishable; a continuous roar. Used at Verdun, Feb 21 1916.
//                1,400 German guns opening a barrage heard in London.
// ---------------------------------------------------------------------------
UENUM(BlueprintType)
enum class EBarrageType : uint8
{
    Harassing       UMETA(DisplayName = "Harassing Fire"),
    Creeping        UMETA(DisplayName = "Creeping Barrage"),
    Box             UMETA(DisplayName = "Box Barrage"),
    Saturation      UMETA(DisplayName = "Saturation Barrage"),
    Registration    UMETA(DisplayName = "Registration Fire (Ranging)"),
    Drumfire        UMETA(DisplayName = "Drumfire / Trommelfeuer")
};

// ---------------------------------------------------------------------------
// FBarrageWave — configuration for a single barrage episode
// Multiple waves can run simultaneously (different calibres, different zones)
// ---------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FBarrageWave
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage")
    EBarrageType Type = EBarrageType::Harassing;

    // World-space centre of target zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage")
    FVector TargetCenter = FVector::ZeroVector;

    // Scatter radius around TargetCenter (cm). Historical CEP:
    //   75mm  at 5km:  ~2500cm (25m)
    //   155mm at 8km:  ~4000cm (40m)
    //   210mm at 10km: ~6000cm (60m)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage")
    float AccuracyCEP = 3000.0f;

    // Total shells per minute across all batteries in this wave
    // Reference rates: Harassing=5, Creeping=20, Saturation=60, Drumfire=120
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage",
              meta = (ClampMin = "1", ClampMax = "300"))
    float ShellsPerMinute = 10.0f;

    // Duration in seconds. -1 = runs until StopBarrage() is called.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage")
    float DurationSeconds = 60.0f;

    // Shell class for this wave (different waves can use different calibres)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage")
    TSubclassOf<AArtilleryShell> ShellClass;

    // Primary shell type for this wave (drives FX scaling and sound pitch)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage")
    EShellType ShellType = EShellType::Shell75mm;

    // Fraction of heavy (155mm+) shells mixed in with light shells (0.0-1.0)
    // At Verdun, barrages mixed calibres to break down different types of cover
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HeavyShellFraction = 0.0f;

    // Heavy shell class to mix in (if HeavyShellFraction > 0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage")
    TSubclassOf<AArtilleryShell> HeavyShellClass;

    // ---- Creeping barrage ----
    // Direction the barrage line advances (world space, should be normalised)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Creeping")
    FVector CreepDirection = FVector::ForwardVector;

    // How fast the barrage line advances (cm/s).
    // Historical: infantry walked ~80cm/s, barrage led by ~50m = stays close
    // Standard creep: ~833cm/s (50m/min). Fast: ~1667cm/s (100m/min).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Creeping",
              meta = (ClampMin = "100", ClampMax = "5000"))
    float CreepSpeedCmPerSec = 833.0f;

    // Width of the creeping barrage line (cm) — shells scatter across this width
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Creeping")
    float CreepWidth = 5000.0f;  // 50m wide

    // ---- Box barrage ----
    // Half-extent of the box (area inside is "sealed off")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Box")
    FVector BoxHalfExtent = FVector(2000.0f, 2000.0f, 0.0f);
};

// ---------------------------------------------------------------------------
// Internal state for a running barrage wave
// ---------------------------------------------------------------------------
struct FActiveWave
{
    FBarrageWave    Config;
    int32           WaveID      = -1;
    FTimerHandle    FireTimer;
    FVector         CurrentTargetCenter;    // Advances for creeping barrage
    float           StartTime   = 0.0f;
};

// ---------------------------------------------------------------------------
// ABarrageDirector
//
// Orchestrates all incoming artillery fire. Drop one in the level, configure
// the launch origin (off-map gun positions), and call StartBarrage() or
// one of the preset sequences.
//
// The key design insight: at Verdun the volume of fire was itself the primary
// threat. This actor drives that volume — multiple simultaneous waves of
// different calibres, different patterns, building in intensity.
//
// Historical preset: StartVerdunOpeningSequence()
//   Escalates over ~5 minutes from registration rounds through drumfire —
//   the same sequence experienced by Verdun defenders on Feb 21, 1916.
// ---------------------------------------------------------------------------
UCLASS()
class ONLYTHEDEAD_API ABarrageDirector : public AActor
{
    GENERATED_BODY()

public:
    ABarrageDirector();

    // ---- Launch origin ----
    // Where shells appear to come from (off-map gun positions).
    // Multiple origins give different whistle angles and flight paths.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Setup")
    TArray<FVector> GunOrigins;

    // Default if GunOrigins is empty
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Setup")
    FVector DefaultGunOrigin = FVector(50000.0f, 5000.0f, 8000.0f);

    // ---- Default shell classes (set in Blueprint child) ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Setup")
    TSubclassOf<AArtilleryShell> DefaultShellClass;        // 75mm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Setup")
    TSubclassOf<AArtilleryShell> HeavyShellClass;          // 155mm or 210mm

    // ---- Preset target: the trench the director targets ----
    // Barrage preset functions aim at this location

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrage|Setup")
    FVector TrenchCenterLocation = FVector::ZeroVector;

    // ---- Runtime controls ----

    // Start a barrage wave. Returns a WaveID for stopping it.
    UFUNCTION(BlueprintCallable, Category = "Barrage")
    int32 StartBarrage(const FBarrageWave& Wave);

    // Stop a specific wave by ID (from StartBarrage return value)
    UFUNCTION(BlueprintCallable, Category = "Barrage")
    void StopBarrage(int32 WaveID);

    // Stop all active barrages
    UFUNCTION(BlueprintCallable, Category = "Barrage")
    void StopAllBarrages();

    // ---- Historical presets ----

    /**
     * StartVerdunOpeningSequence
     *
     * Recreates the escalating barrage structure of February 21, 1916:
     *   Phase 1 — Registration fire (5 shells, 30s gap, unsettling silence)
     *   Phase 2 — Harassing fire (5 shells/min, random scatter, 60s)
     *   Phase 3 — Sustained barrage (20 shells/min, mixed 75+155mm, 90s)
     *   Phase 4 — Saturation (60 shells/min, all calibres, 90s)
     *   Phase 5 — Drumfire / Trommelfeuer (120 shells/min, continuous)
     *
     * Each phase is queued with a timer so the escalation happens automatically.
     * Total duration to Drumfire: ~4.5 minutes.
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Barrage|Presets")
    void StartVerdunOpeningSequence();

    // Creeping barrage advancing toward TrenchCenterLocation
    UFUNCTION(BlueprintCallable, Category = "Barrage|Presets")
    void StartCreepingBarrage(FVector StartLine, FVector TargetLine, float DurationSeconds = 120.0f);

    // Box barrage sealing off a trench section
    UFUNCTION(BlueprintCallable, Category = "Barrage|Presets")
    void StartBoxBarrage(FVector Center, FVector HalfExtent, float DurationSeconds = 90.0f);

    // Query
    UFUNCTION(BlueprintPure, Category = "Barrage")
    int32 GetActiveWaveCount() const { return ActiveWaves.Num(); }

    UFUNCTION(BlueprintPure, Category = "Barrage")
    float GetCurrentShellsPerMinute() const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    TArray<FActiveWave> ActiveWaves;
    int32 NextWaveID = 0;

    // Fire one shell for a given active wave
    void FireShellForWave(int32 WaveID);

    // Calculate the next impact point for a wave (accounts for wave type)
    FVector CalculateImpactPoint(const FActiveWave& Wave) const;

    // Pick a launch origin (cycles through GunOrigins, or uses Default)
    FVector GetLaunchOrigin(const FActiveWave& Wave) const;

    // Apply CEP scatter to an ideal impact point
    FVector ApplyScatter(FVector IdealPoint, float CEP) const;

    int32 LastGunOriginIndex = 0;
};
