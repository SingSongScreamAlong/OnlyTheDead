//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "Weapons/AOTDWeaponBase.h"
#include "ARifle_1916.generated.h"

UCLASS()
class ARifle_1916 : public AOTDWeaponBase
{
    GENERATED_BODY()

public:
    ARifle_1916();

    virtual void BeginPlay() override;
    virtual bool CanFire() const override;

protected:
    virtual void HandleShot(const FOTDShotRequest& Request, FOTDShotResult& OutResult) override;
    virtual void OnShotCompleted(const FOTDShotResult& ShotResult) override;
    virtual void OnReloadComplete() override;

    void StartBoltCycle();
    void CompleteBoltCycle();

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ballistics")
    float MuzzleVelocity = 76000.f; // cm/s

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ballistics")
    float GravityScale = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bolt")
    float BoltCycleDuration = 1.35f;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Bolt")
    bool bBoltCycling = false;

    FTimerHandle BoltCycleTimerHandle;
};
//// END NEW CODE
