#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VerdunGameMode.generated.h"

class AArtilleryManager;

/**
 * AVerdunGameMode
 *
 * Game mode for the POC level.
 * Sets the default pawn to AVerdunSoldier and manages the
 * barrage lifecycle for the proof-of-concept experience.
 *
 * Extended in Blueprint (BP_VerdunGameMode) to expose the
 * artillery manager reference and any level-specific scripting.
 */
UCLASS()
class ONLYTHEDEAD_API AVerdunGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AVerdunGameMode();

    // Reference to the artillery manager in the level
    UPROPERTY(BlueprintReadWrite, Category = "GameMode")
    AArtilleryManager* ArtilleryManager;

    // If set, barrage is paused on begin play and started after this delay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode")
    float BarrageStartDelaySeconds = 5.0f;

    UFUNCTION(BlueprintCallable, Category = "GameMode")
    void StartBarrage();

    UFUNCTION(BlueprintCallable, Category = "GameMode")
    void StopBarrage();

protected:
    virtual void BeginPlay() override;

private:
    void FindArtilleryManager();
    FTimerHandle BarrageStartTimer;
};
