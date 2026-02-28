#include "Artillery/ArtilleryShell.h"
#include "Artillery/ArtilleryTypes.h"
#include "Player/SurvivalComponent.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"

AArtilleryShell::AArtilleryShell()
{
    PrimaryActorTick.bCanEverTick = true;

    // Collision root — small sphere matching shell tip
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
    ProjectileMovement->bAutoActivate = false;  // Activated by LaunchAtTarget()

    WhistleAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("WhistleAudio"));
    WhistleAudioComp->SetupAttachment(RootComponent);
    WhistleAudioComp->bAutoActivate = false;

    // Reasonable default — Blueprint child overrides with actual asset data
    ShellData = ShellPresets::Make75mm();

    // Kill after 30s so leaked shells don't persist
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
        // Orient shell body along flight direction
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

    const FVector Origin = GetActorLocation();
    const FVector Delta  = TargetLocation - Origin;

    // Total flight time: IncomingWhistleSeconds covers only the incoming arc —
    // we use it as the full flight time for POC simplicity so the
    // whistle plays from the moment of launch.
    const float FlightTime = FMath::Max(ShellData.IncomingWhistleSeconds, 1.0f);

    const float GravityZ = GetWorld()->GetGravityZ(); // typically -980.0f cm/s²

    FVector LaunchVelocity;
    LaunchVelocity.X = Delta.X / FlightTime;
    LaunchVelocity.Y = Delta.Y / FlightTime;
    // Solve vertical: TargetZ = OriginZ + vz*t + 0.5*g*t²
    //   → vz = (DeltaZ - 0.5*g*t²) / t
    LaunchVelocity.Z = (Delta.Z - 0.5f * GravityZ * FlightTime * FlightTime) / FlightTime;

    ProjectileMovement->Velocity = LaunchVelocity;
    ProjectileMovement->Activate();

    // Start the incoming whistle
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

    // Estimate remaining time using height above target
    const float HeightAbove = GetActorLocation().Z - LaunchTarget.Z;
    const float VelocityZ   = ProjectileMovement->Velocity.Z;
    const float GravityZ    = GetWorld()->GetGravityZ();

    // Quadratic: HeightAbove = VelocityZ*t + 0.5*GravityZ*t²
    const float A = 0.5f * GravityZ;
    const float B = VelocityZ;
    const float C = -HeightAbove;

    const float Discriminant = B * B - 4.0f * A * C;
    if (Discriminant < 0.0f) return 0.0f;

    const float Sqrt = FMath::Sqrt(Discriminant);
    const float T1   = (-B + Sqrt) / (2.0f * A);
    const float T2   = (-B - Sqrt) / (2.0f * A);

    // Return smallest positive root
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
    // Freeze the shell immediately
    ProjectileMovement->StopMovementImmediately();
    SetActorHiddenInGame(true);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WhistleAudioComp->Stop();

    SpawnExplosionFX(ImpactPoint, ImpactNormal);
    SpawnCrater(ImpactPoint, ImpactNormal);
    ApplyExplosionDamage(ImpactPoint);
    ApplyMoraleEffect(ImpactPoint);

    // Play the crack/boom at impact point — attenuation set in Sound Cue
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactPoint);
    }

    OnShellDetonated.Broadcast(this, ImpactPoint);

    // Cleanup after VFX settle
    SetLifeSpan(5.0f);
}

// ---------------------------------------------------------------------------
// Explosion effects
// ---------------------------------------------------------------------------

void AArtilleryShell::SpawnExplosionFX(const FVector& Location, const FVector& Normal)
{
    if (ExplosionNiagara)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ExplosionNiagara,
            Location,
            Normal.Rotation(),
            // Scale by crater radius so big shells make a bigger blast cloud
            FVector(ShellData.CraterRadius / 200.0f)
        );
    }

#if WITH_EDITOR
    // Debug radii visible in PIE — remove for shipping
    DrawDebugSphere(GetWorld(), Location, ShellData.LethalRadius, 16, FColor::Red,    false, 6.0f, 0, 3.0f);
    DrawDebugSphere(GetWorld(), Location, ShellData.WoundRadius,  16, FColor::Orange, false, 6.0f, 0, 2.0f);
    DrawDebugSphere(GetWorld(), Location, ShellData.ShockRadius,  16, FColor::Yellow, false, 6.0f, 0, 1.0f);
#endif
}

void AArtilleryShell::SpawnCrater(const FVector& Location, const FVector& Normal)
{
    if (!CraterActorClass) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.Owner = this;

    AActor* Crater = GetWorld()->SpawnActor<AActor>(
        CraterActorClass,
        Location,
        Normal.Rotation(),
        Params
    );

    // Scale crater mesh to match shell caliber
    if (Crater)
    {
        const float Scale = ShellData.CraterRadius / 200.0f;
        Crater->SetActorScale3D(FVector(Scale));
    }
}

void AArtilleryShell::ApplyExplosionDamage(const FVector& Location)
{
    // UE5 radial damage — falls off linearly from MaxDamage to 0 at WoundRadius
    UGameplayStatics::ApplyRadialDamageWithFalloff(
        GetWorld(),
        ShellData.MaxDamage,        // Base damage
        ShellData.MaxDamage * 0.1f, // Min damage at edge
        Location,
        ShellData.LethalRadius,     // Inner (full damage) radius
        ShellData.WoundRadius,      // Outer (falloff) radius
        2.0f,                       // Damage falloff exponent
        nullptr,                    // Damage type class
        TArray<AActor*>(),
        this,
        GetInstigatorController(),
        ECC_Visibility
    );
}

void AArtilleryShell::ApplyMoraleEffect(const FVector& Location)
{
    // Notify all pawns within shock radius
    // SurvivalComponent responds to this with morale damage + shell shock state
    TArray<AActor*> NearbyPawns;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyPawns);

    for (AActor* Pawn : NearbyPawns)
    {
        const float Dist = FVector::Dist(Pawn->GetActorLocation(), Location);
        if (Dist > ShellData.ShockRadius) continue;

        USurvivalComponent* Survival = Pawn->FindComponentByClass<USurvivalComponent>();
        if (!Survival) continue;

        // Linear interpolation: full morale damage near center, minimum at shock edge
        const float Alpha = FMath::Clamp(Dist / ShellData.ShockRadius, 0.0f, 1.0f);
        const float MoraleDmg = FMath::Lerp(
            ShellData.MoraleDamageLethal,
            ShellData.MoraleDamageShock,
            Alpha
        );

        const bool bNearMiss = Dist <= ShellData.LethalRadius * 2.0f;
        Survival->ApplyMoraleDamage(MoraleDmg, bNearMiss);
    }
}

// ---------------------------------------------------------------------------
// Whistle pitch modulation
// ---------------------------------------------------------------------------

void AArtilleryShell::UpdateWhistlePitch()
{
    if (!WhistleAudioComp || !WhistleAudioComp->IsPlaying()) return;

    // As shell descends from apex, pitch rises (Doppler / air compression)
    // Map: high altitude above target = low pitch, near ground = high pitch
    const float HeightAbove = FMath::Max(0.0f, GetActorLocation().Z - LaunchTarget.Z);
    const float MaxHeight   = 10000.0f; // approximate arc height in cm
    const float T           = 1.0f - FMath::Clamp(HeightAbove / MaxHeight, 0.0f, 1.0f);
    const float Pitch       = FMath::Lerp(0.7f, 1.4f, T);

    WhistleAudioComp->SetPitchMultiplier(Pitch);
}
