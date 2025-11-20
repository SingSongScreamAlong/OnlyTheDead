//// BEGIN NEW CODE
#include "Weapons/ARifle_1916.h"

#include "DrawDebugHelpers.h"

ARifle_1916::ARifle_1916()
{
    FireRate = 12.f;
    MagazineSize = 5;
    BaseDamage = 85.f;
    DamageFalloffStart = 1500.f;
    DamageFalloffEnd = 8500.f;
    BaseSpreadDegrees = 0.25f;
    ReloadDuration = 3.6f;
    TraceDistance = 25000.f;
}

void ARifle_1916::BeginPlay()
{
    Super::BeginPlay();
}

bool ARifle_1916::CanFire() const
{
    return !bBoltCycling && Super::CanFire();
}

void ARifle_1916::HandleShot(const FOTDShotRequest& Request, FOTDShotResult& OutResult)
{
    const FVector Forward = Request.ShootDirection.GetSafeNormal();
    const FVector SpreadDirection = FMath::VRandCone(Forward, Request.SpreadRadians * 0.5f);

    const float FlightTime = TraceDistance / MuzzleVelocity;
    const FVector Gravity = FVector(0.f, 0.f, GetWorld()->GetGravityZ() * GravityScale);
    const FVector EndPoint = Request.MuzzleLocation + SpreadDirection * TraceDistance + 0.5f * Gravity * FMath::Square(FlightTime);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(RifleTrace), true, GetOwner());
    Params.AddIgnoredActor(this);

    OutResult.bDidHit = GetWorld()->LineTraceSingleByChannel(OutResult.Hit, Request.MuzzleLocation, EndPoint, ECC_Visibility, Params);
    if (OutResult.bDidHit)
    {
        const float Distance = (OutResult.Hit.ImpactPoint - Request.MuzzleLocation).Size();
        const float DamageValue = ComputeDamageForDistance(Distance);
        UGameplayStatics::ApplyPointDamage(
            OutResult.Hit.GetActor(),
            DamageValue,
            SpreadDirection,
            OutResult.Hit,
            GetInstigatorController(),
            this,
            DamageTypeClass ? *DamageTypeClass : UDamageType::StaticClass()
        );
    }
}

void ARifle_1916::OnShotCompleted(const FOTDShotResult& ShotResult)
{
    StartBoltCycle();
}

void ARifle_1916::OnReloadComplete()
{
    bBoltCycling = false;
}

void ARifle_1916::StartBoltCycle()
{
    bBoltCycling = true;
    GetWorldTimerManager().SetTimer(BoltCycleTimerHandle, this, &ARifle_1916::CompleteBoltCycle, BoltCycleDuration, false);
}

void ARifle_1916::CompleteBoltCycle()
{
    bBoltCycling = false;
}
//// END NEW CODE
