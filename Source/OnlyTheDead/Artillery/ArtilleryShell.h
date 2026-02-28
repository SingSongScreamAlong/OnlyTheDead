#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Artillery/ArtilleryTypes.h"
#include "ArtilleryShell.generated.h"

class UProjectileMovementComponent;
class UAudioComponent;
class UStaticMeshComponent;
class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;

// Fired when shell detonates — TrenchSegments and HUD subscribe to this
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnShellDetonated,
    AArtilleryShell*, Shell,
    FVector, ImpactLocation
);

/**
 * AArtilleryShell
 *
 * A single artillery round fired by the ArtilleryManager.
 * Key mechanic: the incoming whistle sound is the player's ONLY warning.
 * Time to impact matches IncomingWhistleSeconds in FShellData.
 *
 * Lifecycle:
 *   1. Spawned by ArtilleryManager at LaunchOrigin
 *   2. LaunchAtTarget() sets initial velocity for a ballistic arc
 *   3. Whistle audio plays and pitches up as shell descends
 *   4. On collision: Detonate() -> radial damage + morale effect + crater
 */
UCLASS()
class ONLYTHEDEAD_API AArtilleryShell : public AActor
{
    GENERATED_BODY()

public:
    AArtilleryShell();

    // Shell properties (set by ArtilleryManager before launch)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    FShellData ShellData;

    // Fires the shell toward TargetLocation using ballistic physics
    UFUNCTION(BlueprintCallable, Category = "Artillery")
    void LaunchAtTarget(const FVector& TargetLocation);

    UFUNCTION(BlueprintPure, Category = "Artillery")
    float GetTimeToImpact() const;

    // Broadcast when shell hits terrain/trench
    UPROPERTY(BlueprintAssignable, Category = "Artillery|Events")
    FOnShellDetonated OnShellDetonated;

    // ---- Assignable assets (set in Blueprint child class) ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX")
    UNiagaraSystem* ExplosionNiagara;

    // Crater actor spawned at impact point (static mesh with decal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX")
    TSubclassOf<AActor> CraterActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Audio")
    USoundBase* WhistleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Audio")
    USoundBase* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Audio")
    USoundBase* DistantThumpSound;  // Heard from far away after detonation

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShellMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* WhistleAudioComp;

    // Hit callback — bound in BeginPlay
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
               UPrimitiveComponent* OtherComp, FVector NormalImpulse,
               const FHitResult& Hit);

private:
    void Detonate(const FVector& ImpactPoint, const FVector& ImpactNormal);
    void SpawnExplosionFX(const FVector& Location, const FVector& Normal);
    void ApplyExplosionDamage(const FVector& Location);
    void ApplyMoraleEffect(const FVector& Location);
    void SpawnCrater(const FVector& Location, const FVector& Normal);

    // Displace terrain vertices at impact point — persistent, accumulative
    void DeformTerrain(const FVector& ImpactPoint);

    // Pitch whistle up as shell descends (height-based modulation)
    void UpdateWhistlePitch();

    FVector LaunchTarget;
    bool bDetonated = false;
    float LaunchTime = 0.0f;
};
