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
class UGeometryCollectionComponent;

// Fired when shell detonates — TrenchSegments, HUD, and ConcussionComponent subscribe to this
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnShellDetonated,
    AArtilleryShell*, Shell,
    FVector, ImpactLocation
);

/**
 * AArtilleryShell
 *
 * A single artillery round fired by ABarrageDirector.
 * The incoming whistle is the player's ONLY warning — timing calibrated
 * to IncomingWhistleSeconds so players have exactly that long to react.
 *
 * On detonation, five systems fire in order:
 *   1. AExplosionLight  — Lumen dynamic GI flash (white-orange burst → ember glow)
 *   2. SpawnExplosionFX — Niagara: soil geyser + persistent smoke column + shrapnel
 *   3. SpawnChaosDebris — Chaos: soil/dirt chunks thrown as rigid bodies
 *   4. DeformTerrain    — ADeformableTerrain vertex displacement (persistent crater)
 *   5. SpawnCrater      — Optional static mesh decal at impact point
 *   6. ApplyExplosionDamage — UE5 radial damage
 *   7. ApplyMoraleEffect   — Morale/SurvivalComponent notification
 *   8. NotifyConcussion    — UConcussionComponent shell shock on nearby soldiers
 */
UCLASS()
class ONLYTHEDEAD_API AArtilleryShell : public AActor
{
    GENERATED_BODY()

public:
    AArtilleryShell();

    // Shell properties (set by ABarrageDirector or Blueprint before launch)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    FShellData ShellData;

    // Fires the shell toward TargetLocation using ballistic physics
    UFUNCTION(BlueprintCallable, Category = "Artillery")
    void LaunchAtTarget(const FVector& TargetLocation);

    UFUNCTION(BlueprintPure, Category = "Artillery")
    float GetTimeToImpact() const;

    // Broadcast when shell hits — subscribe to receive impact notifications
    UPROPERTY(BlueprintAssignable, Category = "Artillery|Events")
    FOnShellDetonated OnShellDetonated;

    // ---- Niagara FX (assign in Blueprint child class) ----

    // Instantaneous dirt geyser — GPU particles thrown upward at impact
    // Niagara user params: ImpactScale (float), GroundNormal (vector)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX|Niagara")
    UNiagaraSystem* SoilGeyserNiagara;

    // Persistent rising smoke column — lingers 30-60s, drifts with wind
    // Niagara user params: SmokeScale (float), WindDirection (vector)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX|Niagara")
    UNiagaraSystem* SmokeColumnNiagara;

    // Metal shrapnel burst — ribbon GPU particles with secondary ground impacts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX|Niagara")
    UNiagaraSystem* ShrapnelNiagara;

    // Legacy single-system slot (used if individual systems above are not assigned)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX|Niagara")
    UNiagaraSystem* ExplosionNiagara;

    // ---- Chaos debris (assign in Blueprint child class) ----

    // Pre-fractured soil/rock geometry collection for earth chunks thrown by blast.
    // If assigned: spawned at impact and given a radial field impulse.
    // If null: falls back to spawning simple physics cubes (POC placeholder).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX|Chaos")
    TSubclassOf<AActor> SoilDebrisActorClass;   // BP_SoilDebris (Geometry Collection BP)

    // Number of fallback physics cubes if SoilDebrisActorClass is null
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX|Chaos",
              meta = (ClampMin = "3", ClampMax = "24"))
    int32 FallbackDebrisCount = 8;

    // ---- Static mesh crater decal ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|FX")
    TSubclassOf<AActor> CraterActorClass;

    // ---- Audio ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Audio")
    USoundBase* WhistleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Audio")
    USoundBase* ImpactSound;

    // Heard by distant observers after the crack — low-frequency boom with delay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery|Audio")
    USoundBase* DistantThumpSound;

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

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
               UPrimitiveComponent* OtherComp, FVector NormalImpulse,
               const FHitResult& Hit);

private:
    void Detonate(const FVector& ImpactPoint, const FVector& ImpactNormal);

    // ---- Detonation systems (called in order) ----

    // 1. Lumen dynamic GI flash (AExplosionLight)
    void SpawnLumenFlash(const FVector& Location);

    // 2. Niagara FX: soil geyser, smoke column, shrapnel
    void SpawnExplosionFX(const FVector& Location, const FVector& Normal);

    // 3. Chaos rigid body soil debris
    void SpawnChaosDebris(const FVector& Location, const FVector& Normal);

    // 4. Persistent terrain vertex deformation
    void DeformTerrain(const FVector& ImpactPoint);

    // 5. Static mesh crater decal
    void SpawnCrater(const FVector& Location, const FVector& Normal);

    // 6. Radial damage
    void ApplyExplosionDamage(const FVector& Location);

    // 7. Morale damage
    void ApplyMoraleEffect(const FVector& Location);

    // 8. Shell shock notification to UConcussionComponent on nearby soldiers
    void NotifyConcussion(const FVector& Location);

    // Pitch whistle up as shell descends (height-based Doppler approximation)
    void UpdateWhistlePitch();

    FVector LaunchTarget;
    bool bDetonated = false;
    float LaunchTime = 0.0f;
};
