//// BEGIN NEW CODE
#include "Weapons/AOTDWeaponBase.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/OTDMentalStateComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"

AOTDWeaponBase::AOTDWeaponBase()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);

    TimeBetweenShots = FireRate > 0.f ? 60.f / FireRate : 0.1f;
    CurrentAmmo = MagazineSize;
}

void AOTDWeaponBase::HandleImpactEffects(const FOTDShotResult& ShotResult)
{
    if (!ShotResult.bDidHit)
    {
        return;
    }

    EPhysicalSurface SurfaceType = SurfaceType_Default;
    if (ShotResult.Hit.PhysMaterial.IsValid())
    {
        SurfaceType = ShotResult.Hit.PhysMaterial->SurfaceType;
    }

    const FOTDImpactEffect* Effect = ImpactEffects.FindByPredicate([SurfaceType](const FOTDImpactEffect& Entry)
    {
        return Entry.SurfaceType == SurfaceType;
    });

    if (!Effect)
    {
        Effect = ImpactEffects.FindByPredicate([](const FOTDImpactEffect& Entry)
        {
            return Entry.SurfaceType == SurfaceType_Default;
        });
    }

    if (!Effect)
    {
        return;
    }

    if (Effect->Particle)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect->Particle, ShotResult.Hit.ImpactPoint, ShotResult.Hit.ImpactNormal.Rotation());
    }

    if (Effect->ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Effect->ImpactSound, ShotResult.Hit.ImpactPoint);
    }
}

void AOTDWeaponBase::HandlePassBySuppression(const FOTDShotRequest& Request, const FOTDShotResult& ShotResult)
{
    if (SuppressionIntensity <= 0.f)
    {
        return;
    }

    const FVector SegmentStart = Request.MuzzleLocation;
    const FVector SegmentEnd = ShotResult.TraceEnd;
    const FVector SegmentVector = SegmentEnd - SegmentStart;
    const float SegmentLength = SegmentVector.Size();
    if (SegmentLength <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const FVector SegmentDirection = SegmentVector / SegmentLength;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponPassBy), false, GetOwner());
    Params.AddIgnoredActor(this);
    if (AActor* OwnerActor = GetOwner())
    {
        Params.AddIgnoredActor(OwnerActor);
    }

    TArray<FHitResult> Hits;
    const FCollisionShape Sphere = FCollisionShape::MakeSphere(PassByCheckRadius);
    if (!GetWorld()->SweepMultiByChannel(Hits, SegmentStart, SegmentEnd, FQuat::Identity, ECC_Pawn, Sphere, Params))
    {
        return;
    }

    TSet<const AActor*> ProcessedActors;

    for (const FHitResult& Hit : Hits)
    {
        const AActor* HitActor = Hit.GetActor();
        if (!HitActor || ProcessedActors.Contains(HitActor))
        {
            continue;
        }

        ProcessedActors.Add(HitActor);

        UOTDMentalStateComponent* Mental = HitActor->FindComponentByClass<UOTDMentalStateComponent>();
        if (!Mental)
        {
            continue;
        }

        const FVector ActorLocation = HitActor->GetActorLocation();
        const float Projection = FVector::DotProduct(ActorLocation - SegmentStart, SegmentDirection);
        if (Projection < 0.f || Projection > PassByMaxDistance)
        {
            continue;
        }

        const FVector ClosestPoint = SegmentStart + SegmentDirection * FMath::Clamp(Projection, 0.f, SegmentLength);
        const float LateralDistance = FVector::Dist(ActorLocation, ClosestPoint);
        if (LateralDistance > PassByCheckRadius)
        {
            continue;
        }

        const float DistanceAlpha = 1.f - FMath::Clamp(Projection / PassByMaxDistance, 0.f, 1.f);
        const float Intensity = SuppressionIntensity * DistanceAlpha;
        Mental->RegisterNearMiss(LateralDistance, Intensity);

        if (PassBySound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), PassBySound, ClosestPoint);
        }
    }
}

void AOTDWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    TimeBetweenShots = FireRate > 0.f ? 60.f / FireRate : 0.1f;
    CurrentAmmo = FMath::Clamp(CurrentAmmo, 0, MagazineSize);
}

void AOTDWeaponBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AOTDWeaponBase::StartFire()
{
    if (!CanFire())
    {
        if (!bIsReloading)
        {
            Reload();
        }
        return;
    }

    bWantsToFire = true;
    PerformShot();

    if (bAutomatic)
    {
        ScheduleNextShot();
    }
}

void AOTDWeaponBase::StopFire()
{
    bWantsToFire = false;
    GetWorldTimerManager().ClearTimer(ShotTimerHandle);
}

bool AOTDWeaponBase::Reload()
{
    if (bIsReloading || CurrentAmmo == MagazineSize)
    {
        return false;
    }

    bIsReloading = true;
    GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AOTDWeaponBase::HandleReloadComplete, ReloadDuration, false);
    return true;
}

bool AOTDWeaponBase::CanFire() const
{
    if (bIsReloading || CurrentAmmo <= 0)
    {
        return false;
    }

    return GetWorld()->GetTimeSeconds() >= NextAllowedFireTime;
}

float AOTDWeaponBase::GetTimeUntilNextShot() const
{
    const float Remaining = NextAllowedFireTime - GetWorld()->GetTimeSeconds();
    return FMath::Max(Remaining, 0.f);
}

void AOTDWeaponBase::HandleShot(const FOTDShotRequest& Request, FOTDShotResult& OutResult)
{
    const FVector Direction = FMath::VRandCone(Request.ShootDirection, Request.SpreadRadians);
    const FVector End = Request.MuzzleLocation + Direction * TraceDistance;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), true, GetOwner());
    Params.AddIgnoredActor(this);

    OutResult.bDidHit = GetWorld()->LineTraceSingleByChannel(OutResult.Hit, Request.MuzzleLocation, End, ECC_Visibility, Params);
    OutResult.TraceStart = Request.MuzzleLocation;
    OutResult.TraceEnd = OutResult.bDidHit ? OutResult.Hit.ImpactPoint : End;
    OutResult.ShotDirection = Direction;

    if (bDebugWeaponTraces)
    {
        DrawDebugLine(GetWorld(), Request.MuzzleLocation, OutResult.TraceEnd, OutResult.bDidHit ? FColor::Red : FColor::Green, false, 1.5f, 0, 0.5f);
    }

    if (OutResult.bDidHit)
    {
        const float Distance = (OutResult.Hit.ImpactPoint - Request.MuzzleLocation).Size();
        const float DamageValue = ComputeDamageForDistance(Distance);
        UGameplayStatics::ApplyPointDamage(
            OutResult.Hit.GetActor(),
            DamageValue,
            Direction,
            OutResult.Hit,
            GetInstigatorController(),
            this,
            DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass()
        );
    }
}

FOTDShotRequest AOTDWeaponBase::BuildShotRequest() const
{
    FOTDShotRequest Request;
    Request.SpreadRadians = FMath::DegreesToRadians(BaseSpreadDegrees);

    if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
    {
        const FTransform SocketTransform = WeaponMesh->GetSocketTransform(MuzzleSocketName);
        Request.MuzzleLocation = SocketTransform.GetLocation();
        Request.ShootDirection = SocketTransform.GetRotation().GetForwardVector();
    }
    else
    {
        Request.MuzzleLocation = GetActorLocation();
        Request.ShootDirection = GetActorForwardVector();
    }

    return Request;
}

void AOTDWeaponBase::ConsumeAmmo()
{
    CurrentAmmo = FMath::Clamp(CurrentAmmo - 1, 0, MagazineSize);
}

void AOTDWeaponBase::ScheduleNextShot()
{
    if (!bWantsToFire)
    {
        return;
    }

    GetWorldTimerManager().SetTimer(ShotTimerHandle, this, &AOTDWeaponBase::PerformShot, TimeBetweenShots, false);
}

void AOTDWeaponBase::HandleReloadComplete()
{
    bIsReloading = false;
    CurrentAmmo = MagazineSize;
    OnReloadComplete();
}

void AOTDWeaponBase::PerformShot()
{
    if (!CanFire())
    {
        if (!bIsReloading)
        {
            Reload();
        }
        return;
    }

    NextAllowedFireTime = GetWorld()->GetTimeSeconds() + TimeBetweenShots;

    const FOTDShotRequest ShotRequest = BuildShotRequest();

    FOTDShotResult ShotResult;
    HandleShot(ShotRequest, ShotResult);

    ConsumeAmmo();

    if (MuzzleFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFX, ShotRequest.MuzzleLocation, ShotRequest.ShootDirection.Rotation());
    }

    OnShotFired.Broadcast();
    OnShotFX(ShotResult);
    HandleImpactEffects(ShotResult);
    HandlePassBySuppression(ShotRequest, ShotResult);
    OnShotCompleted(ShotResult);

    if (CurrentAmmo <= 0)
    {
        Reload();
    }

    if (bAutomatic && bWantsToFire)
    {
        ScheduleNextShot();
    }
}

void AOTDWeaponBase::OnShotCompleted(const FOTDShotResult& ShotResult)
{
}

float AOTDWeaponBase::ComputeDamageForDistance(float Distance) const
{
    if (Distance <= DamageFalloffStart)
    {
        return BaseDamage;
    }

    const float EffectiveEnd = FMath::Max(DamageFalloffEnd, DamageFalloffStart + 1.f);
    if (Distance >= EffectiveEnd)
    {
        return BaseDamage * 0.3f;
    }

    const float Alpha = (Distance - DamageFalloffStart) / (EffectiveEnd - DamageFalloffStart);
    const float MinDamage = BaseDamage * 0.3f;
    return FMath::Lerp(BaseDamage, MinDamage, Alpha);
}
//// END NEW CODE
