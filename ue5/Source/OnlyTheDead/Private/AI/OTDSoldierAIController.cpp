//// BEGIN NEW CODE
#include "AI/OTDSoldierAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AOTDSoldierAIController::AOTDSoldierAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AOTDSoldierAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardAsset)
    {
        UBlackboardComponent* BlackboardComp = nullptr;
        if (UseBlackboard(BlackboardAsset, BlackboardComp))
        {
            InitializeBlackboard(BlackboardComp, BlackboardAsset);
            if (BehaviorTree)
            {
                RunBehaviorTree(BehaviorTree);
            }
        }
    }
}

void AOTDSoldierAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AOTDSoldierAIController::SetTargetActor(AActor* NewTarget)
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsObject(TargetActorKey, NewTarget);
    }
}

void AOTDSoldierAIController::SetCoverLocation(const FVector& Location)
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsVector(CoverLocationKey, Location);
    }
}

void AOTDSoldierAIController::SetSuppressionLevel(float NewValue)
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsFloat(SuppressionLevelKey, NewValue);
    }
}

void AOTDSoldierAIController::SetInTrench(bool bInTrench)
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsBool(InTrenchKey, bInTrench);
    }
}

void AOTDSoldierAIController::SetSoldierState(EOTDSoldierState NewState)
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsEnum(SoldierStateKey, static_cast<uint8>(NewState));
    }
}

void AOTDSoldierAIController::InitializeBlackboard(UBlackboardComponent* BlackboardComp, UBlackboardData* InBlackboardAsset)
{
    if (!BlackboardComp)
    {
        return;
    }

    BlackboardComp->SetValueAsEnum(SoldierStateKey, static_cast<uint8>(EOTDSoldierState::Idle));
    BlackboardComp->SetValueAsFloat(SuppressionLevelKey, 0.f);
    BlackboardComp->SetValueAsBool(InTrenchKey, false);
}
//// END NEW CODE
