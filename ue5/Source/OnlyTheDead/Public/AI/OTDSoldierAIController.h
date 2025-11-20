//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "OTDSoldierAIController.generated.h"

UENUM(BlueprintType)
enum class EOTDSoldierState : uint8
{
    Idle,
    Hold,
    Engage,
    Panic,
    Flee,
    Cower
};

UCLASS()
class AOTDSoldierAIController : public AAIController
{
    GENERATED_BODY()

public:
    AOTDSoldierAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTargetActor(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetCoverLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetSuppressionLevel(float NewValue);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetInTrench(bool bInTrench);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetSoldierState(EOTDSoldierState NewState);

protected:
    void InitializeBlackboard(UBlackboardComponent* BlackboardComp, UBlackboardData* BlackboardAsset);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TObjectPtr<UBlackboardData> BlackboardAsset;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
    FName CoverLocationKey = TEXT("CoverLocation");

    UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
    FName SuppressionLevelKey = TEXT("SuppressionLevel");

    UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
    FName InTrenchKey = TEXT("InTrench");

    UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
    FName SoldierStateKey = TEXT("CurrentState");
};
//// END NEW CODE
