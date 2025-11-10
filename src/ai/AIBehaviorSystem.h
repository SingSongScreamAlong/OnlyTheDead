// Only The Dead - AI Behavior System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "../core/GameTypes.h"
#include "AIBehaviorSystem.generated.h"

/**
 * AI behavior system for squad mates, enemies, and NPCs
 * Realistic WWI soldier behavior including:
 * - Shell shock reactions
 * - Suppression from artillery
 * - Squad cohesion
 * - Survival behaviors
 */

UENUM(BlueprintType)
enum class EAIBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle/Standing Guard"),
    Patrol          UMETA(DisplayName = "Patrolling"),
    Combat          UMETA(DisplayName = "In Combat"),
    Suppressed      UMETA(DisplayName = "Suppressed (Taking Cover)"),
    Fleeing         UMETA(DisplayName = "Fleeing/Routing"),
    ShellShock      UMETA(DisplayName = "Shell Shocked"),
    SeekingShelter  UMETA(DisplayName = "Seeking Shelter"),
    Wounded         UMETA(DisplayName = "Wounded"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAIRole : uint8
{
    Rifleman        UMETA(DisplayName = "Rifleman"),
    MachineGunner   UMETA(DisplayName = "Machine Gunner"),
    Grenadier       UMETA(DisplayName = "Grenadier"),
    Officer         UMETA(DisplayName = "Officer"),
    Medic           UMETA(DisplayName = "Medic"),
    Runner          UMETA(DisplayName = "Runner (Messenger)"),
    Sniper          UMETA(DisplayName = "Sniper"),
    Engineer        UMETA(DisplayName = "Engineer/Sapper")
};

USTRUCT(BlueprintType)
struct FAISquadMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FCharacterData CharacterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    EAIRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    EAIBehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float Morale = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bIsWounded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bIsDead = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    int32 ShellsExperienced = 0;
};

UCLASS()
class ONLYTHEDEAD_API AAIBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    AAIBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ========================================================================
    // BEHAVIOR STATE
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
    EAIBehaviorState CurrentBehaviorState = EAIBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
    EAIRole AIRole = EAIRole::Rifleman;

    /** Transition to new behavior state */
    UFUNCTION(BlueprintCallable, Category = "AI|State")
    void TransitionToBehaviorState(EAIBehaviorState NewState);

    /** Get current behavior state */
    UFUNCTION(BlueprintPure, Category = "AI|State")
    EAIBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    // ========================================================================
    // COMBAT BEHAVIOR
    // ========================================================================

    /** Engage enemy target */
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void EngageTarget(AActor* Target);

    /** Take cover from enemy fire */
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void TakeCover();

    /** Suppress enemy position */
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void SuppressPosition(FVector TargetLocation);

    /** Throw grenade at target */
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void ThrowGrenade(FVector TargetLocation);

    // ========================================================================
    // ARTILLERY REACTION
    // ========================================================================

    /** React to incoming artillery */
    UFUNCTION(BlueprintCallable, Category = "AI|Artillery")
    void ReactToIncomingShell(FVector ImpactLocation, EShellType ShellType);

    /** Seek nearest shelter from bombardment */
    UFUNCTION(BlueprintCallable, Category = "AI|Artillery")
    void SeekShelterFromBombardment();

    /** Check if AI should panic from artillery */
    UFUNCTION(BlueprintPure, Category = "AI|Artillery")
    bool ShouldPanicFromArtillery(int32 NearbyShellsInLastMinute) const;

    /** Trigger shell shock behavior */
    UFUNCTION(BlueprintCallable, Category = "AI|Artillery")
    void TriggerShellShockBehavior();

    // ========================================================================
    // SQUAD BEHAVIOR
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Squad")
    AActor* SquadLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Squad")
    TArray<AActor*> SquadMembers;

    /** Follow squad leader */
    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void FollowSquadLeader();

    /** Maintain formation */
    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void MaintainFormation(FVector FormationOffset);

    /** Check on wounded comrade */
    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void CheckOnWoundedComrade(AActor* WoundedActor);

    /** Rally nearby soldiers */
    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void RallyNearby Soldiers(float Radius);

    // ========================================================================
    // SURVIVAL BEHAVIOR
    // ========================================================================

    /** Seek food/water */
    UFUNCTION(BlueprintCallable, Category = "AI|Survival")
    void SeekSupplies();

    /** Rest when safe */
    UFUNCTION(BlueprintCallable, Category = "AI|Survival")
    void RestWhenSafe();

    /** Check morale level */
    UFUNCTION(BlueprintPure, Category = "AI|Survival")
    float GetCurrentMorale() const;

    /** Modify morale */
    UFUNCTION(BlueprintCallable, Category = "AI|Survival")
    void ModifyMorale(float DeltaMorale);

    // ========================================================================
    // COMMUNICATION
    // ========================================================================

    /** Shout warning to nearby soldiers */
    UFUNCTION(BlueprintCallable, Category = "AI|Communication")
    void ShoutWarning(const FString& WarningMessage);

    /** Request orders from officer */
    UFUNCTION(BlueprintCallable, Category = "AI|Communication")
    void RequestOrders();

    /** Report situation */
    UFUNCTION(BlueprintCallable, Category = "AI|Communication")
    void ReportSituation(const FString& Report);

protected:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    float CurrentMorale = 75.0f;
    float TimeInCurrentState = 0.0f;
    int32 ShellsExperiencedInLastMinute = 0;
    TArray<float> ShellExposureTimestamps;

    AActor* CurrentTarget;
    FVector LastKnownEnemyLocation;
    AActor* NearestShelter;

    void UpdateBehaviorLogic(float DeltaTime);
    void UpdateMorale(float DeltaTime);
    void CheckForThreats();
    AActor* FindNearestShelter();
    bool IsInShelter() const;
};
