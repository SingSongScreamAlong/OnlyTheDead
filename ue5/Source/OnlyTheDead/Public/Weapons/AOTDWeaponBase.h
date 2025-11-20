//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "AOTDWeaponBase.generated.h"

class USkeletalMeshComponent;
class UNiagaraSystem;
class UDamageType;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOTDWeaponSimpleSignature);

USTRUCT(BlueprintType)
struct FOTDShotRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Shot")
    FVector MuzzleLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Shot")
    FVector ShootDirection = FVector::ForwardVector;

    UPROPERTY(BlueprintReadWrite, Category = "Shot")
    float SpreadRadians = 0.0f;
};

USTRUCT(BlueprintType)
struct FOTDShotResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Shot")
    bool bDidHit = false;

    UPROPERTY(BlueprintReadOnly, Category = "Shot")
    FHitResult Hit;

    UPROPERTY(BlueprintReadOnly, Category = "Shot")
    FVector TraceStart = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Shot")
    FVector TraceEnd = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Shot")
    FVector ShotDirection = FVector::ForwardVector;
};

USTRUCT(BlueprintType)
struct FOTDImpactEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
    TEnumAsByte<EPhysicalSurface> SurfaceType = SurfaceType_Default;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
    TObjectPtr<UNiagaraSystem> Particle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
    TObjectPtr<USoundBase> ImpactSound;
};

UCLASS(Abstract)
class AOTDWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AOTDWeaponBase();

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual bool Reload();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual bool CanFire() const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool IsReloading() const { return bIsReloading; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    float GetTimeUntilNextShot() const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
    void OnShotFX(const FOTDShotResult& Result);

    UPROPERTY(BlueprintAssignable, Category = "Weapon")
    FOTDWeaponSimpleSignature OnShotFired;

protected:
    virtual void BeginPlay() override;

    virtual void HandleShot(const FOTDShotRequest& Request, FOTDShotResult& OutResult);
    virtual FOTDShotRequest BuildShotRequest() const;
    virtual void ConsumeAmmo();
    virtual void OnShotCompleted(const FOTDShotResult& ShotResult);
    virtual float ComputeDamageForDistance(float Distance) const;
    virtual void OnReloadComplete();
    virtual void HandleImpactEffects(const FOTDShotResult& ShotResult);
    virtual void HandlePassBySuppression(const FOTDShotRequest& Request, const FOTDShotResult& ShotResult);

    void ScheduleNextShot();
    void HandleReloadComplete();
    void PerformShot();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName MuzzleSocketName = TEXT("Muzzle");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    float FireRate = 60.f; // rounds per minute

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    int32 MagazineSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    bool bAutomatic = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Reload")
    float ReloadDuration = 2.6f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Spread")
    float BaseSpreadDegrees = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    float TraceDistance = 20000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    float DamageFalloffStart = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    float DamageFalloffEnd = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    float BaseDamage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
    TObjectPtr<UNiagaraSystem> MuzzleFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
    TArray<FOTDImpactEffect> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
    TObjectPtr<USoundBase> PassBySound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Suppression")
    float SuppressionIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Suppression")
    float PassByCheckRadius = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Suppression")
    float PassByMaxDistance = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    bool bDebugWeaponTraces = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    TSubclassOf<UDamageType> DamageTypeClass;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
    int32 CurrentAmmo = 0;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
    bool bWantsToFire = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
    bool bIsReloading = false;

    float TimeBetweenShots = 0.1f;
    float NextAllowedFireTime = 0.f;

    FTimerHandle ShotTimerHandle;
    FTimerHandle ReloadTimerHandle;
};
//// END NEW CODE
