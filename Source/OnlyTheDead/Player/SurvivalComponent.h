#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

class ATrenchSegment;

// POC implements the Morale meter in full; remaining 6 meters are stubbed
// so the architecture is ready for Phase 2 implementation.
UENUM(BlueprintType)
enum class EShellShockState : uint8
{
    Normal          UMETA(DisplayName = "Normal"),
    ShellShocked    UMETA(DisplayName = "Shell Shocked"),   // Morale < 30
    Dissociated     UMETA(DisplayName = "Dissociated")      // Morale < 10
};

// Fired whenever a meter value changes — drives HUD
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeterChanged, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShellShockChanged, EShellShockState, NewState);

/**
 * USurvivalComponent
 *
 * Implements the 7-meter survival system from the game design document.
 * POC scope: Morale meter is fully functional.
 * All other meters (Hunger, Thirst, Stamina, Warmth, Hygiene, Alertness)
 * are present with their decay rates and can be enabled via bPOCMoraleOnly.
 *
 * Shell shock triggers at Morale < 30 per design doc.
 * Dissociation (progressive numbness) at Morale < 10.
 *
 * Cover mechanics: SurvivalComponent queries nearby ATrenchSegments for
 * their cover value and reduces incoming morale damage accordingly.
 */
UCLASS(ClassGroup = (OnlyTheDead), meta = (BlueprintSpawnableComponent))
class ONLYTHEDEAD_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Morale (fully implemented for POC) ----

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Morale")
    float Morale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Morale")
    float MaxMorale = 100.0f;

    // Base recovery rate per second when not under fire (design doc: 0.5/hour → ~0.0001/s)
    // Scaled up for POC playability: 2.0/min
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Morale")
    float MoraleRecoveryRate = 0.033f;  // per second

    // Morale < this value → shell shock state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Morale")
    float ShellShockThreshold = 30.0f;

    // Morale < this → dissociation (design doc: progressive numbness mechanic)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Morale")
    float DissociationThreshold = 10.0f;

    // Apply morale damage from an explosion; bNearMiss applies a trauma bonus
    UFUNCTION(BlueprintCallable, Category = "Survival|Morale")
    void ApplyMoraleDamage(float Amount, bool bNearMiss = false);

    UFUNCTION(BlueprintPure, Category = "Survival|Morale")
    EShellShockState GetShellShockState() const { return CurrentShockState; }

    UFUNCTION(BlueprintPure, Category = "Survival|Morale")
    bool IsShellShocked() const { return CurrentShockState != EShellShockState::Normal; }

    // ---- Stub meters (architecture complete, not yet simulated) ----
    // Values exposed so HUD can still display them even if static in POC

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Meters")
    float Hunger = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Meters")
    float Thirst = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Meters")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Meters")
    float Warmth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Meters")
    float Hygiene = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Meters")
    float Alertness = 100.0f;

    // Set false to enable all 7 meters (Phase 2)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Debug")
    bool bPOCMoraleOnly = true;

    // ---- Events ----

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnMeterChanged OnMoraleChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnShellShockChanged OnShellShockStateChanged;

    // ---- Cover ----

    // Scan radius for finding the nearest intact TrenchSegment (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Cover")
    float CoverScanRadius = 300.0f;

    UFUNCTION(BlueprintPure, Category = "Survival|Cover")
    float GetCurrentCoverValue() const;

protected:
    virtual void BeginPlay() override;

private:
    void SimulateMoraleDecay(float DeltaTime);
    void UpdateShellShockState();
    float FindBestNearbyTrenchCover() const;

    EShellShockState CurrentShockState = EShellShockState::Normal;

    // Cooldown before morale starts recovering after last hit
    float MoraleRecoveryCooldown = 0.0f;
    const float RecoveryCooldownDuration = 15.0f; // seconds after last shell
};
