#include "Artillery/ArtilleryShell.h"
#include "Artillery/ArtilleryTypes.h"
#include "Player/SurvivalComponent.h"
#include "Player/ConcussionComponent.h"
#include "Terrain/DeformableTerrain.h"
#include "FX/ExplosionLight.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AArtilleryShell::AArtilleryShell()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(12.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
    CollisionSphere->SetNotifyRigidBodyCollision(true);
    RootComponent = CollisionSphere;

    ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellMesh"));
    ShellMesh->SetupAttachment(RootComponent);
    ShellMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionSphere;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bAutoActivate = false;

    WhistleAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("WhistleAudio"));
    WhistleAudioComp->SetupAttachment(RootComponent);
    WhistleAudioComp->bAutoActivate = false;

    ShellData = ShellPresets::Make75mm();
    InitialLifeSpan = 30.0f;
}

void AArtilleryShell::BeginPlay()
{
    Super::BeginPlay();
    CollisionSphere->OnComponentHit.AddDynamic(this, &AArtilleryShell::OnHit);
    LaunchTime = GetWorld()->GetTimeSeconds();
}

void AArtilleryShell::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDetonated && !ProjectileMovement->Velocity.IsNearlyZero())
    {
        SetActorRotation(ProjectileMovement->Velocity.Rotation());
        UpdateWhistlePitch();
    }
}

// ---------------------------------------------------------------------------
// Launch
// ---------------------------------------------------------------------------

void AArtilleryShell::LaunchAtTarget(const FVector& TargetLocation)
{
    LaunchTarget = TargetLocation;

    const FVector Origin   = GetActorLocation();
    const FVector Delta    = TargetLocation - Origin;
    const float FlightTime = FMath::Max(ShellData.IncomingWhistleSeconds, 1.0f);
    const float GravityZ   = GetWorld()->GetGravityZ();

    FVector LaunchVelocity;
    LaunchVelocity.X = Delta.X / FlightTime;
    LaunchVelocity.Y = Delta.Y / FlightTime;
    LaunchVelocity.Z = (Delta.Z - 0.5f * GravityZ * FlightTime * FlightTime) / FlightTime;

    ProjectileMovement->Velocity = LaunchVelocity;
    ProjectileMovement->Activate();

    if (WhistleSound && WhistleAudioComp)
    {
        WhistleAudioComp->SetSound(WhistleSound);
        WhistleAudioComp->Play();
    }

    LaunchTime = GetWorld()->GetTimeSeconds();
}

float AArtilleryShell::GetTimeToImpact() const
{
    if (bDetonated) return 0.0f;

    const float HeightAbove = GetActorLocation().Z - LaunchTarget.Z;
    const float VelocityZ   = ProjectileMovement->Velocity.Z;
    const float GravityZ    = GetWorld()->GetGravityZ();

    const float A = 0.5f * GravityZ;
    const float B = VelocityZ;
    const float C = -HeightAbove;

    const float Discriminant = B * B - 4.0f * A * C;
    if (Discriminant < 0.0f) return 0.0f;

    const float Sqrt = FMath::Sqrt(Discriminant);
    const float T1   = (-B + Sqrt) / (2.0f * A);
    const float T2   = (-B - Sqrt) / (2.0f * A);

    if (T1 > 0.01f && (T2 <= 0.01f || T1 < T2)) return T1;
    if (T2 > 0.01f) return T2;
    return 0.0f;
}

// ---------------------------------------------------------------------------
// Collision → detonation
// ---------------------------------------------------------------------------

void AArtilleryShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                              const FHitResult& Hit)
{
    if (bDetonated) return;
    bDetonated = true;
    Detonate(Hit.ImpactPoint, Hit.ImpactNormal);
}

void AArtilleryShell::Detonate(const FVector& ImpactPoint, const FVector& ImpactNormal)
{
    ProjectileMovement->StopMovementImmediately();
    SetActorHiddenInGame(true);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WhistleAudioComp->Stop();

    // Systems fire in this specific order:
    // Flash first (Lumen picks it up before smoke obscures), then dirt and smoke,
    // then persistent deformation, then damage.
    SpawnLumenFlash(ImpactPoint);
    SpawnExplosionFX(ImpactPoint, ImpactNormal);
    SpawnChaosDebris(ImpactPoint, ImpactNormal);
    DeformTerrain(ImpactPoint);
    SpawnCrater(ImpactPoint, ImpactNormal);
    ApplyExplosionDamage(ImpactPoint);
    ApplyMoraleEffect(ImpactPoint);
    NotifyConcussion(ImpactPoint);

    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactPoint);
    }

    OnShellDetonated.Broadcast(this, ImpactPoint);
    SetLifeSpan(5.0f);
}

// ---------------------------------------------------------------------------
// 1. Lumen dynamic GI flash
// ---------------------------------------------------------------------------

void AArtilleryShell::SpawnLumenFlash(const FVector& Location)
{
    // AExplosionLight is a self-managing actor — spawns, fades, destroys itself.
    // It uses Lumen-visible point lights so the flash illuminates surrounding
    // smoke, trench walls, and churned terrain with correct GI bounces.
    AExplosionLight::SpawnExplosionLight(GetWorld(), Location, ShellData.ShellWeightKg);
}

// ---------------------------------------------------------------------------
// 2. Niagara FX
// ---------------------------------------------------------------------------

void AArtilleryShell::SpawnExplosionFX(const FVector& Location, const FVector& Normal)
{
    // Scale factor: heavier shells make proportionally larger FX
    // 75mm=1.0, 155mm=2.4, 210mm=4.0, 305mm=7.0 — same formula as terrain depth
    const float FXScale = FMath::Clamp(FMath::Pow(ShellData.ShellWeightKg / 7.7f, 0.4f), 1.0f, 7.0f);

    // UE 5.7: ENCPoolMethod::AutoRelease — Niagara components are returned to a
    // pool when complete rather than being destroyed and reallocated. During a
    // Drumfire barrage (120+ shells/min) this eliminates per-impact allocation
    // overhead. Pool size is configured in the NiagaraSystem asset settings.

    // Soil geyser — instantaneous column of earth thrown upward
    // Blueprint sets NS_SoilGeyser to a GPU sim with upward velocity field
    if (SoilGeyserNiagara)
    {
        UNiagaraComponent* Geyser = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), SoilGeyserNiagara,
            Location, Normal.Rotation(),
            FVector(FXScale),
            true,                         // bAutoDestroy
            true,                         // bAutoActivate
            ENCPoolMethod::AutoRelease    // Pool for heavy barrages
        );
        if (Geyser)
        {
            // Expose shell-specific data to Niagara so particle artists can
            // tune emit rate, spread, velocity per calibre without C++ changes
            Geyser->SetFloatParameter(TEXT("ImpactScale"),   FXScale);
            Geyser->SetFloatParameter(TEXT("ShellWeightKg"), ShellData.ShellWeightKg);
            Geyser->SetVectorParameter(TEXT("GroundNormal"),  Normal);
        }
    }

    // Persistent smoke column — rises for 30-60s depending on shell weight.
    // This is the cumulative smoke that historically reduced visibility to
    // near-zero during sustained bombardment.
    // Note: smoke columns are long-lived so pool retention is more valuable here
    // than for short-burst effects.
    if (SmokeColumnNiagara)
    {
        UNiagaraComponent* Smoke = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), SmokeColumnNiagara,
            Location, FRotator::ZeroRotator,
            FVector(FXScale * 0.7f),
            true,                         // bAutoDestroy
            true,                         // bAutoActivate
            ENCPoolMethod::AutoRelease
        );
        if (Smoke)
        {
            Smoke->SetFloatParameter(TEXT("SmokeScale"),    FXScale);
            Smoke->SetFloatParameter(TEXT("LifetimeScale"), FXScale);  // Bigger shells → longer smoke
        }
    }

    // Shrapnel burst — metal fragment ribbons radiating outward
    if (ShrapnelNiagara)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), ShrapnelNiagara,
            Location, Normal.Rotation(),
            FVector(FXScale),
            true, true, ENCPoolMethod::AutoRelease
        );
    }
    else if (ExplosionNiagara)
    {
        // Fallback: single-system slot for simpler setups
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), ExplosionNiagara,
            Location, Normal.Rotation(),
            FVector(FXScale),
            true, true, ENCPoolMethod::AutoRelease
        );
    }

#if WITH_EDITOR
    // Blast radius debug visualisation in PIE
    DrawDebugSphere(GetWorld(), Location, ShellData.LethalRadius, 16, FColor::Red,    false, 6.0f, 0, 3.0f);
    DrawDebugSphere(GetWorld(), Location, ShellData.WoundRadius,  16, FColor::Orange, false, 6.0f, 0, 2.0f);
    DrawDebugSphere(GetWorld(), Location, ShellData.ShockRadius,  16, FColor::Yellow, false, 6.0f, 0, 1.0f);
#endif
}

// ---------------------------------------------------------------------------
// 3. Chaos rigid body soil debris
//
// At Verdun, direct hits created enormous geysers of earth — clods the size
// of a man thrown 50 feet into the air, then falling back into the crater.
// Chaos geometry collections are the correct tool for this.
//
// If SoilDebrisActorClass is assigned in Blueprint:
//   → A Chaos Geometry Collection BP is spawned and given an outward impulse.
//   The collection should be pre-fractured soil/chalk chunks.
//
// If not assigned (POC placeholder):
//   → Simple physics spheres are spawned with random upward velocities.
//   Replace these with proper GC assets when available.
// ---------------------------------------------------------------------------

void AArtilleryShell::SpawnChaosDebris(const FVector& Location, const FVector& Normal)
{
    const float WeightScale  = FMath::Clamp(FMath::Pow(ShellData.ShellWeightKg / 7.7f, 0.4f), 1.0f, 7.0f);
    const float ImpulseForce = ShellData.ShellWeightKg * 80.0f;  // Heavier shells displace more earth

    if (SoilDebrisActorClass)
    {
        // Spawn the Chaos Geometry Collection — it handles its own fracturing and physics
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.Owner = this;

        AActor* Debris = GetWorld()->SpawnActor<AActor>(
            SoilDebrisActorClass, Location, FRotator::ZeroRotator, Params);

        if (Debris)
        {
            // Apply radial force to the geometry collection component
            // (Chaos GC responds to URadialFalloff fields applied via ExternalStrain)
            // In Blueprint: bind to BeginPlay to auto-apply the impulse on spawn
            // by reading ImpulseForce from a custom float parameter
            Debris->SetActorScale3D(FVector(WeightScale * 0.5f));

            // Notify the debris BP of the impulse magnitude via tag or custom event
            // (Chaos GC needs the impulse applied to its physics field, done in BP)
        }
    }
    else
    {
        // Fallback: spawn physics spheres — crude but shows the concept in PIE
        // Replace with proper Chaos GC BP_SoilDebris when art is available
        for (int32 i = 0; i < FallbackDebrisCount; ++i)
        {
            const float Angle    = FMath::FRandRange(0.0f, TWO_PI);
            const float Spread   = FMath::FRandRange(50.0f, ShellData.CraterRadius * 0.8f);
            const FVector SpawnOffset(
                Spread * FMath::Cos(Angle),
                Spread * FMath::Sin(Angle),
                FMath::FRandRange(10.0f, 80.0f)
            );

            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AActor* Chunk = GetWorld()->SpawnActor<AActor>(
                AActor::StaticClass(), Location + SpawnOffset, FRotator::ZeroRotator, Params);

            if (Chunk)
            {
                // Lifetime — dirt clods land and stay
                Chunk->SetLifeSpan(20.0f);

                UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Chunk);
                if (MeshComp)
                {
                    MeshComp->RegisterComponent();
                    MeshComp->AttachToComponent(
                        Chunk->GetRootComponent(),
                        FAttachmentTransformRules::KeepRelativeTransform);
                    MeshComp->SetSimulatePhysics(true);
                    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

                    // Random upward impulse — bigger shells throw chunks higher/farther
                    const float UpForce      = FMath::FRandRange(400.0f, 1200.0f) * WeightScale;
                    const float OutwardForce = FMath::FRandRange(200.0f, 800.0f)  * WeightScale;
                    const FVector Impulse = SpawnOffset.GetSafeNormal() * OutwardForce
                                          + FVector(0, 0, UpForce);
                    MeshComp->AddImpulse(Impulse, NAME_None, true);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// 4. Persistent terrain deformation
// ---------------------------------------------------------------------------

void AArtilleryShell::DeformTerrain(const FVector& ImpactPoint)
{
    static TWeakObjectPtr<ADeformableTerrain> CachedTerrain;

    ADeformableTerrain* Terrain = CachedTerrain.Get();
    if (!Terrain)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeformableTerrain::StaticClass(), Found);
        if (Found.Num() == 0) return;
        Terrain = Cast<ADeformableTerrain>(Found[0]);
        CachedTerrain = Terrain;
    }
    if (!Terrain) return;

    // Crater depth: sub-linear power law calibrated to historical records
    // 75mm (7.7 kg)  → 60cm deep, 2m radius
    // 155mm (43 kg)  → ~115cm deep, 6m radius
    // 210mm (121 kg) → ~200cm deep, 12m radius
    // 305mm (380 kg) → ~350cm deep, 25m radius
    const float ReferenceWeightKg = 7.7f;
    const float BaseDepthCm       = 60.0f;
    const float CraterDepth = BaseDepthCm * FMath::Pow(ShellData.ShellWeightKg / ReferenceWeightKg, 0.4f);

    Terrain->ApplyExplosionDeformation(ImpactPoint, ShellData.CraterRadius, CraterDepth, 0.28f);
}

// ---------------------------------------------------------------------------
// 5. Crater decal mesh
// ---------------------------------------------------------------------------

void AArtilleryShell::SpawnCrater(const FVector& Location, const FVector& Normal)
{
    if (!CraterActorClass) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.Owner = this;

    AActor* Crater = GetWorld()->SpawnActor<AActor>(CraterActorClass, Location, Normal.Rotation(), Params);
    if (Crater)
    {
        const float Scale = ShellData.CraterRadius / 200.0f;
        Crater->SetActorScale3D(FVector(Scale));
    }
}

// ---------------------------------------------------------------------------
// 6. Radial damage
// ---------------------------------------------------------------------------

void AArtilleryShell::ApplyExplosionDamage(const FVector& Location)
{
    UGameplayStatics::ApplyRadialDamageWithFalloff(
        GetWorld(),
        ShellData.MaxDamage,
        ShellData.MaxDamage * 0.1f,
        Location,
        ShellData.LethalRadius,
        ShellData.WoundRadius,
        2.0f,
        nullptr,
        TArray<AActor*>(),
        this,
        GetInstigatorController(),
        ECC_Visibility
    );
}

// ---------------------------------------------------------------------------
// 7. Morale damage (SurvivalComponent)
// ---------------------------------------------------------------------------

void AArtilleryShell::ApplyMoraleEffect(const FVector& Location)
{
    TArray<AActor*> NearbyPawns;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyPawns);

    for (AActor* PawnActor : NearbyPawns)
    {
        const float Dist = FVector::Dist(PawnActor->GetActorLocation(), Location);
        if (Dist > ShellData.ShockRadius) continue;

        USurvivalComponent* Survival = PawnActor->FindComponentByClass<USurvivalComponent>();
        if (!Survival) continue;

        const float Alpha   = FMath::Clamp(Dist / ShellData.ShockRadius, 0.0f, 1.0f);
        const float Damage  = FMath::Lerp(ShellData.MoraleDamageLethal, ShellData.MoraleDamageShock, Alpha);
        const bool bNearMiss = Dist <= ShellData.LethalRadius * 2.0f;
        Survival->ApplyMoraleDamage(Damage, bNearMiss);
    }
}

// ---------------------------------------------------------------------------
// 8. Shell shock (ConcussionComponent)
//
// Notify UConcussionComponent on nearby soldiers/characters.
// Range: full ShockRadius — concussion propagates farther than lethal blast.
// Distant impacts still accumulate shell shock over time (the Verdun reality).
// ---------------------------------------------------------------------------

void AArtilleryShell::NotifyConcussion(const FVector& Location)
{
    TArray<AActor*> NearbyCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), NearbyCharacters);

    for (AActor* CharActor : NearbyCharacters)
    {
        const float Dist = FVector::Dist(CharActor->GetActorLocation(), Location);
        // Extend to 2× ShockRadius — even distant blasts contribute to chronic shock
        if (Dist > ShellData.ShockRadius * 2.0f) continue;

        UConcussionComponent* Concussion = CharActor->FindComponentByClass<UConcussionComponent>();
        if (!Concussion) continue;

        Concussion->NotifyExplosion(Location, ShellData.LethalRadius, ShellData.ShellWeightKg);
    }
}

// ---------------------------------------------------------------------------
// Whistle pitch modulation (Doppler approximation as shell descends)
// ---------------------------------------------------------------------------

void AArtilleryShell::UpdateWhistlePitch()
{
    if (!WhistleAudioComp || !WhistleAudioComp->IsPlaying()) return;

    const float HeightAbove = FMath::Max(0.0f, GetActorLocation().Z - LaunchTarget.Z);
    const float MaxHeight   = 10000.0f;
    const float T           = 1.0f - FMath::Clamp(HeightAbove / MaxHeight, 0.0f, 1.0f);
    const float Pitch       = FMath::Lerp(0.7f, 1.4f, T);

    WhistleAudioComp->SetPitchMultiplier(Pitch);
}
