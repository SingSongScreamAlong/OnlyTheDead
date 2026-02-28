#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ConcussionComponent.generated.h"

class UCameraShakeBase;
class UAudioComponent;
class USoundBase;

// ---------------------------------------------------------------------------
// EConcussionState — current dissociation level from accumulated shell shock
//
// Historical note: "Shell shock" (now PTSD/TBI) was the defining psychological
// wound of WWI. By the war's end, 80,000 British cases were documented —
// and most went undiagnosed. At Verdun, men were relieved after 8 days maximum
// because the psychological destruction beyond that point was total.
//
// In game terms: this meter fills with near misses and slowly drains.
// Each state changes what the player can do and what they perceive.
// ---------------------------------------------------------------------------
UENUM(BlueprintType)
enum class EConcussionState : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Shaken      UMETA(DisplayName = "Shaken (0.25+)"),     // Mild tremor, edge darkening
    Distressed  UMETA(DisplayName = "Distressed (0.50+)"), // Blurred periphery, ringing
    ShellShocked UMETA(DisplayName = "Shell-Shocked (0.75+)"), // Severe dissociation
    Catatonic   UMETA(DisplayName = "Catatonic (1.0)")     // Cannot move or act
};

// ---------------------------------------------------------------------------
// UConcussionComponent
//
// Attach to AVerdunSoldier. Responds to nearby shell detonations to produce
// the full physical and psychological experience of being under bombardment:
//
//   Physical:   camera shake, stumbling, deafness, dust on screen
//   Acoustic:   ear ringing (distance-dependent pitch/duration), muffling
//   Perceptual: tunnel vision, motion blur, time dilation near misses
//   Chronic:    shell shock meter — fills with near misses, drains slowly.
//               At high levels fundamentally changes what the player can do.
//
// Called externally by ArtilleryShell::Detonate via NotifyExplosion().
// Subscribe to OnShellShockStateChanged to drive Blueprint VFX/UI.
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnShellShockStateChanged,
    EConcussionState, OldState,
    EConcussionState, NewState
);

UCLASS(ClassGroup = (OnlyTheDead), meta = (BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UConcussionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UConcussionComponent();

    // ---- Shell shock meter ----

    // Current accumulated shell shock (0.0 = clear, 1.0 = catatonic)
    // Use this in Blueprint to drive post-process effects (vignette, blur, desaturation)
    UPROPERTY(BlueprintReadOnly, Category = "Concussion")
    float ShellShockLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Concussion")
    EConcussionState CurrentState = EConcussionState::Normal;

    // Broadcast when state tier changes (Normal→Shaken, Shaken→Distressed, etc.)
    UPROPERTY(BlueprintAssignable, Category = "Concussion|Events")
    FOnShellShockStateChanged OnShellShockStateChanged;

    // ---- Tuning ----

    // Thresholds for distance zones (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Thresholds")
    float NearMissRadius = 500.0f;      // Within 5m: severe

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Thresholds")
    float CloseRadius = 3000.0f;        // 5-30m: strong

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Thresholds")
    float MediumRadius = 8000.0f;       // 30-80m: moderate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Thresholds")
    float DistantRadius = 20000.0f;     // 80-200m: distant rumble + ground shake

    // Shell shock added per near miss (NearMiss=0.25, Close=0.10, Medium=0.03)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Tuning")
    float ShockAddNearMiss = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Tuning")
    float ShockAddClose = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Tuning")
    float ShockAddMedium = 0.03f;

    // Drain rate (per second, when no impacts occurring)
    // Historical: men needed days of rest. 0.005/s = ~200s to fully clear.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Tuning",
              meta = (ClampMin = "0.001", ClampMax = "0.1"))
    float ShockDrainPerSecond = 0.005f;

    // ---- Camera shake assets (assign in Blueprint) ----
    // UE5: use TSubclassOf<UCameraShakeBase> with PlayerCameraManager->StartCameraShake

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|CameraShake")
    TSubclassOf<UCameraShakeBase> ShakeNearMiss;    // Violent, sustained

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|CameraShake")
    TSubclassOf<UCameraShakeBase> ShakeClose;        // Strong jolt

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|CameraShake")
    TSubclassOf<UCameraShakeBase> ShakeMedium;       // Rumble

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|CameraShake")
    TSubclassOf<UCameraShakeBase> ShakeDistant;      // Subtle ground shake

    // ---- Audio (assign in Blueprint) ----

    // High-pitched sine-wave ring (variable pitch/duration by distance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Audio")
    USoundBase* EarRingSound;

    // Low-frequency rumble for distant explosions felt through the ground
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concussion|Audio")
    USoundBase* GroundRumbleSound;

    // Muffled/underwater audio filter — applied at Distressed+ state
    // (Set on the audio mix / DSP chain in Blueprint using this bool as trigger)
    UPROPERTY(BlueprintReadOnly, Category = "Concussion")
    bool bAudioMuffled = false;

    // ---- Movement penalties (read by AVerdunSoldier) ----

    // Speed multiplier at current shock level (1.0=normal, 0.0=catatonic)
    UPROPERTY(BlueprintReadOnly, Category = "Concussion")
    float MovementSpeedMultiplier = 1.0f;

    // True when near-miss stumble animation should play
    UPROPERTY(BlueprintReadOnly, Category = "Concussion")
    bool bStumbling = false;

    // ---- Main API ----

    /**
     * NotifyExplosion
     *
     * Called by ArtilleryShell::Detonate for every explosion in the world.
     * This component calculates the distance from its owner and applies
     * the appropriate concussion effects.
     *
     * @param ImpactLocation  World-space detonation point
     * @param BlastRadius     Shell's lethal radius — used to scale effects
     * @param ShellWeightKg   Used to scale camera shake magnitude
     */
    UFUNCTION(BlueprintCallable, Category = "Concussion")
    void NotifyExplosion(FVector ImpactLocation, float BlastRadius, float ShellWeightKg);

    // Force-clear shock (e.g. level restart, medical dressing station rest)
    UFUNCTION(BlueprintCallable, Category = "Concussion")
    void ClearShellShock();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;

private:
    void ApplyNearMiss(float ShellWeightKg);
    void ApplyClose(float ShellWeightKg);
    void ApplyMedium(float ShellWeightKg);
    void ApplyDistant();

    void TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale);
    void TriggerEarRing(float Intensity, float DurationScale);

    void UpdateShellShockState();
    EConcussionState ComputeStateFromLevel() const;

    UPROPERTY()
    UAudioComponent* EarRingComp;

    // Stumble clear timer
    FTimerHandle StumbleTimer;

    // Time since last impact (for drain logic)
    float TimeSinceLastImpact = 999.0f;
};
