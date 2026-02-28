#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Artillery/ArtilleryTypes.h"
#include "ArtilleryManager.generated.h"

class AArtilleryShell;

UENUM(BlueprintType)
enum class EBarrageState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Incoming        UMETA(DisplayName = "Barrage Active"),
    Pause           UMETA(DisplayName = "Lull Between Salvos")
};

/**
 * AArtilleryManager
 *
 * Orchestrates the bombardment for the POC level.
 * Spawns AArtilleryShell actors at randomised intervals within a target zone.
 *
 * Barrage patterns:
 *   Random       — shells land anywhere inside the target box
 *   LinearWalking— shells "walk" along the trench axis (authentic WWI tactic)
 *   Concentrated — shells cluster near a hotspot (simulates registration)
 *
 * Shell type probability matches historical Verdun data:
 *   ~65% 75/77mm field guns, ~25% 155mm howitzers, ~10% heavy mortars
 *
 * Place one of these in the level, set TargetZoneCenter to the trench,
 * LaunchOrigin to a point off-screen "behind German lines", and press Play.
 */
UCLASS()
class ONLYTHEDEAD_API AArtilleryManager : public AActor
{
    GENERATED_BODY()

public:
    AArtilleryManager();

    // ---- Target zone ----

    // Centre of the bombardment area (should overlap with trench)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Targeting")
    FVector TargetZoneCenter;

    // Half-extents of the target box in X/Y (Z is resolved to terrain)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Targeting")
    FVector TargetZoneHalfExtent = FVector(2500.0f, 500.0f, 0.0f);

    // Spawn point for shells — far from the trench, off-screen
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Targeting")
    FVector LaunchOrigin;

    // Height above LaunchOrigin to spawn each shell
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Targeting")
    float LaunchHeightOffset = 500.0f;

    // ---- Timing ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Timing", meta = (ClampMin = "0.5"))
    float MinIntervalSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Timing", meta = (ClampMin = "1.0"))
    float MaxIntervalSeconds = 9.0f;

    // ---- Shell type distribution ----
    // Values should sum to 1.0 (normalised internally if they don't)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Distribution", meta = (ClampMin = "0", ClampMax = "1"))
    float Prob75mm = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Distribution", meta = (ClampMin = "0", ClampMax = "1"))
    float Prob77mm = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Distribution", meta = (ClampMin = "0", ClampMax = "1"))
    float Prob155mm = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Distribution", meta = (ClampMin = "0", ClampMax = "1"))
    float Prob210mm = 0.09f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Distribution", meta = (ClampMin = "0", ClampMax = "1"))
    float Prob305mm = 0.01f;

    // ---- Pattern ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Pattern")
    EBarragePattern BarragePattern = EBarragePattern::Random;

    // For LinearWalking: how far the barrage advances per shell (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Pattern")
    float WalkingBarrageStepCm = 300.0f;

    // ---- Shell class ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Shell")
    TSubclassOf<AArtilleryShell> ShellClass;

    // ---- Controls ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Control")
    bool bAutoStartOnBeginPlay = true;

    UFUNCTION(BlueprintCallable, Category = "Artillery")
    void StartBarrage();

    UFUNCTION(BlueprintCallable, Category = "Artillery")
    void StopBarrage();

    // Fire a specific shell type at a specific location immediately
    UFUNCTION(BlueprintCallable, Category = "Artillery")
    void FireShellAt(EShellType Type, FVector TargetLocation);

    UFUNCTION(BlueprintPure, Category = "Artillery")
    EBarrageState GetBarrageState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Artillery")
    int32 GetShellsFiredCount() const { return ShellsFired; }

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    void FireNext();
    void ScheduleNext();
    EShellType SelectShellType() const;
    FVector GetNextTargetInZone();

    FTimerHandle BarrageTimer;
    EBarrageState CurrentState = EBarrageState::Idle;

    // Walking barrage tracking
    float WalkOffset = 0.0f;

    int32 ShellsFired = 0;
};
