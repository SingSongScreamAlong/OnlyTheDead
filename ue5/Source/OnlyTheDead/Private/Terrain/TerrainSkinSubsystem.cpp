//// BEGIN NEW CODE
#include "Terrain/TerrainSkinSubsystem.h"

#include "Curves/CurveFloat.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

namespace
{
    constexpr float EPSILON = 0.01f;
}

UTerrainSkinSubsystem::UTerrainSkinSubsystem()
{
    MaterialNoiseProfiles.Add(EOTDTerrainMaterial::Mud, {1.f, 0.4f, 3, 0.5f});
    MaterialNoiseProfiles.Add(EOTDTerrainMaterial::Chalk, {0.6f, 0.3f, 2, 0.4f});
    MaterialNoiseProfiles.Add(EOTDTerrainMaterial::Forest, {1.2f, 0.5f, 4, 0.6f});
    MaterialNoiseProfiles.Add(EOTDTerrainMaterial::Urban, {0.4f, 0.2f, 1, 0.3f});

    MaterialResponseParams.Add(EOTDTerrainMaterial::Mud, {1.0f, 1.0f, 1.15f, 1.0f, 1.0f});
    MaterialResponseParams.Add(EOTDTerrainMaterial::Chalk, {0.85f, 0.9f, 0.8f, 0.9f, 0.9f});
    MaterialResponseParams.Add(EOTDTerrainMaterial::Forest, {1.1f, 0.95f, 1.2f, 1.1f, 1.05f});
    MaterialResponseParams.Add(EOTDTerrainMaterial::Urban, {0.6f, 0.7f, 0.5f, 0.8f, 0.5f});
}

void UTerrainSkinSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UTerrainSkinSubsystem::Deinitialize()
{
    ActiveCraters.Empty();
    Super::Deinitialize();
}

void UTerrainSkinSubsystem::Tick(float DeltaTime)
{
    PruneExpiredCraters();
    ProcessQueuedShells(DeltaTime);

    if (bEnableDebugDraw)
    {
        for (const FOTDActiveCrater& Crater : ActiveCraters)
        {
            DrawDebugSphere(GetWorld(), Crater.Descriptor.Location, Crater.Descriptor.Radius, 12, FColor::Red, false, -1.f, 0, 2.f);
        }
    }
}

FGuid UTerrainSkinSubsystem::RegisterCrater(const FOTDCraterDescriptor& Descriptor)
{
    if (ActiveCraters.Num() >= MaxActiveCraters)
    {
        ActiveCraters.RemoveAt(0);
    }

    FOTDActiveCrater NewCrater;
    NewCrater.Descriptor = Descriptor;
    NewCrater.Descriptor.CraterId = FGuid::NewGuid();
    NewCrater.Descriptor.SpawnTimeSeconds = GetWorld()->GetTimeSeconds();

    ActiveCraters.Add(NewCrater);
    OnCraterAdded.Broadcast(NewCrater.Descriptor.CraterId, NewCrater.Descriptor);
    return NewCrater.Descriptor.CraterId;
}

FGuid UTerrainSkinSubsystem::RegisterShellImpactCrater(const FVector& ImpactPoint, const FVector& ImpactDirection, float Yield, EOTDTerrainMaterial Material)
{
    const FOTDCraterDescriptor Descriptor = BuildCraterDescriptor(ImpactPoint, ImpactDirection, Yield, Material);
    return RegisterCrater(Descriptor);
}

bool UTerrainSkinSubsystem::RemoveCrater(const FGuid& CraterId)
{
    const int32 Index = ActiveCraters.IndexOfByPredicate([&CraterId](const FOTDActiveCrater& Entry)
    {
        return Entry.Descriptor.CraterId == CraterId;
    });

    if (Index != INDEX_NONE)
    {
        const FOTDCraterDescriptor Descriptor = ActiveCraters[Index].Descriptor;
        ActiveCraters.RemoveAt(Index);
        OnCraterRemoved.Broadcast(Descriptor.CraterId, Descriptor);
        return true;
    }

    return false;
}

void UTerrainSkinSubsystem::ClearAllCraters()
{
    for (const FOTDActiveCrater& Entry : ActiveCraters)
    {
        OnCraterRemoved.Broadcast(Entry.Descriptor.CraterId, Entry.Descriptor);
    }
    ActiveCraters.Empty();
}

const FOTDCraterDescriptor* UTerrainSkinSubsystem::GetCrater(const FGuid& CraterId) const
{
    const FOTDActiveCrater* Entry = ActiveCraters.FindByPredicate([&CraterId](const FOTDActiveCrater& Test)
    {
        return Test.Descriptor.CraterId == CraterId;
    });

    return Entry ? &Entry->Descriptor : nullptr;
}

float UTerrainSkinSubsystem::SampleDeformation(const FVector& WorldLocation) const
{
    float Accumulator = 0.f;
    for (const FOTDActiveCrater& Entry : ActiveCraters)
    {
        const float DistanceSq = FVector::DistSquared2D(Entry.Descriptor.Location, WorldLocation);
        const float InfluenceRadius = Entry.Descriptor.Radius + Entry.Descriptor.RimWidth + MaxTrackedRadius;
        if (DistanceSq > FMath::Square(InfluenceRadius))
        {
            continue;
        }

        Accumulator += ComputeCraterContribution(Entry.Descriptor, WorldLocation);
    }
    return Accumulator;
}

void UTerrainSkinSubsystem::SetMaterialNoiseProfile(EOTDTerrainMaterial Material, const FOTDNoiseProfile& Profile)
{
    MaterialNoiseProfiles.FindOrAdd(Material) = Profile;
}

void UTerrainSkinSubsystem::PruneExpiredCraters()
{
    const double Now = GetWorld()->GetTimeSeconds();

    for (int32 Index = ActiveCraters.Num() - 1; Index >= 0; --Index)
    {
        const FOTDActiveCrater& Crater = ActiveCraters[Index];
        if (Crater.Descriptor.LifetimeSeconds > EPSILON && Now - Crater.Descriptor.SpawnTimeSeconds >= Crater.Descriptor.LifetimeSeconds)
        {
            OnCraterRemoved.Broadcast(Crater.Descriptor.CraterId, Crater.Descriptor);
            ActiveCraters.RemoveAt(Index);
        }
    }
}

float UTerrainSkinSubsystem::ComputeCraterContribution(const FOTDCraterDescriptor& Descriptor, const FVector& WorldLocation) const
{
    const float Distance = FVector::Dist2D(Descriptor.Location, WorldLocation);
    if (IsContributionNegligible(Distance, Descriptor.Radius + Descriptor.RimWidth))
    {
        return 0.f;
    }

    const float AngleMultiplier = CalculateAngleMultiplier(Descriptor.EntryAngleDegrees);
    const float Radius = Descriptor.Radius * AngleMultiplier;
    const float Normalized = Distance / FMath::Max(Radius, EPSILON);
    float Contribution = 0.f;

    if (Distance <= Radius)
    {
        Contribution = -Descriptor.Depth * AngleMultiplier * (1.f - Normalized);
    }
    else
    {
        const float RimT = (Distance - Radius) / FMath::Max(Descriptor.RimWidth, EPSILON);
        Contribution = Descriptor.RimHeight * AngleMultiplier * (1.f - RimT);
    }

    const FOTDNoiseProfile& Noise = Descriptor.NoiseProfile;
    const float NoiseValue = GenerateNoiseValue(Noise, FVector2D(WorldLocation.X, WorldLocation.Y));
    Contribution += NoiseValue * Noise.Amplitude;

    return Contribution;
}

const FOTDNoiseProfile& UTerrainSkinSubsystem::GetNoiseProfileForMaterial(EOTDTerrainMaterial Material) const
{
    if (const FOTDNoiseProfile* Profile = MaterialNoiseProfiles.Find(Material))
    {
        return *Profile;
    }

    return MaterialNoiseProfiles.FindChecked(EOTDTerrainMaterial::Mud);
}

const FOTDMaterialResponseParams& UTerrainSkinSubsystem::GetMaterialResponse(EOTDTerrainMaterial Material) const
{
    if (const FOTDMaterialResponseParams* Params = MaterialResponseParams.Find(Material))
    {
        return *Params;
    }

    return MaterialResponseParams.FindChecked(EOTDTerrainMaterial::Mud);
}

void UTerrainSkinSubsystem::SetMaxActiveCraters(int32 NewMax)
{
    MaxActiveCraters = FMath::Max(0, NewMax);
    if (ActiveCraters.Num() > MaxActiveCraters)
    {
        const int32 Excess = ActiveCraters.Num() - MaxActiveCraters;
        ActiveCraters.RemoveAt(0, Excess);
    }
}

FOTDCraterDescriptor UTerrainSkinSubsystem::BuildCraterDescriptor(const FVector& ImpactPoint, const FVector& ImpactDirection, float Yield, EOTDTerrainMaterial Material) const
{
    FOTDCraterDescriptor Descriptor;
    Descriptor.Location = ImpactPoint;
    Descriptor.ImpactDirection = ImpactDirection.GetSafeNormal();
    Descriptor.Material = Material;
    Descriptor.Yield = FMath::Max(0.1f, Yield);

    const float BaseRadius = 200.f * Descriptor.Yield;
    const float BaseDepth = 60.f * Descriptor.Yield;
    const float BaseRimHeight = 25.f * Descriptor.Yield;
    const float BaseRimWidth = BaseRadius * 0.45f;

    const float EntryAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(-Descriptor.ImpactDirection, FVector::UpVector)));
    Descriptor.EntryAngleDegrees = EntryAngle;

    const FOTDMaterialResponseParams& Response = GetMaterialResponse(Material);

    Descriptor.Radius = BaseRadius * Response.RadiusMultiplier;
    Descriptor.Depth = BaseDepth * Response.DepthMultiplier;
    Descriptor.RimHeight = BaseRimHeight * Response.RimHeightMultiplier;
    Descriptor.RimWidth = BaseRimWidth * Response.RimWidthMultiplier;
    Descriptor.LifetimeSeconds *= Response.LifetimeScale;
    Descriptor.NoiseProfile = GetNoiseProfileForMaterial(Material);

    return Descriptor;
}

float UTerrainSkinSubsystem::CalculateAngleMultiplier(float EntryAngleDegrees) const
{
    const float Radians = FMath::DegreesToRadians(FMath::Clamp(EntryAngleDegrees, 0.f, 90.f));
    return FMath::Clamp(FMath::Cos(Radians) + 0.35f, 0.25f, 1.35f);
}

float UTerrainSkinSubsystem::GenerateNoiseValue(const FOTDNoiseProfile& Profile, const FVector2D& Location) const
{
    float Amplitude = Profile.Amplitude;
    float Frequency = Profile.Frequency;
    float NoiseSum = 0.f;
    float AccumulatedAmplitude = 0.f;

    for (int32 Octave = 0; Octave < Profile.Octaves; ++Octave)
    {
        NoiseSum += FMath::PerlinNoise2D(Location * Frequency) * Amplitude;
        AccumulatedAmplitude += Amplitude;
        Amplitude *= Profile.Persistence;
        Frequency *= 2.f;
    }

    return AccumulatedAmplitude > EPSILON ? NoiseSum / AccumulatedAmplitude : 0.f;
}

bool UTerrainSkinSubsystem::IsContributionNegligible(float Distance, float Radius) const
{
    return Distance > Radius + EPSILON;
}

void UTerrainSkinSubsystem::ProcessQueuedShells(float DeltaTime)
{
    int32 SolvedCount = 0;

    for (int32 Index = QueuedShellImpacts.Num() - 1; Index >= 0 && SolvedCount < MaxShellSolvesPerTick; --Index)
    {
        FVector ImpactPoint;
        FVector ImpactNormal;
        if (SolveShellImpact(QueuedShellImpacts[Index], ImpactPoint, ImpactNormal))
        {
            if (!ActiveCraters.ContainsByPredicate([&ImpactPoint, this](const FOTDActiveCrater& Entry)
            {
                return FVector::Dist2D(Entry.Descriptor.Location, ImpactPoint) <= MinCraterSpacing;
            }))
            {
                RegisterShellImpactCrater(ImpactPoint, ImpactNormal, QueuedShellImpacts[Index].Yield, QueuedShellImpacts[Index].Material);
            }
        }

        QueuedShellImpacts.RemoveAt(Index);
        ++SolvedCount;
    }
}

bool UTerrainSkinSubsystem::SolveShellImpact(const FOTDQueuedShellImpact& Impact, FVector& OutImpactPoint, FVector& OutImpactNormal) const
{
    if (!GetWorld())
    {
        return false;
    }

    FVector CurrentLocation = Impact.Start;
    FVector Velocity = Impact.Velocity;
    const FVector Gravity = FVector(0.f, 0.f, GetWorld()->GetGravityZ() * ShellGravityScale);

    const float StepTime = ShellTraceStep / FMath::Max(Velocity.Size(), 1.f);
    float AccumulatedTime = 0.f;

    for (int32 Iteration = 0; Iteration < Impact.MaxIterations && AccumulatedTime <= Impact.MaxSimulationTime; ++Iteration)
    {
        const FVector NextVelocity = Velocity + Gravity * StepTime;
        const FVector NextLocation = CurrentLocation + Velocity * StepTime + 0.5f * Gravity * FMath::Square(StepTime);

        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, CurrentLocation, NextLocation, ECC_Visibility))
        {
            OutImpactPoint = Hit.ImpactPoint;
            OutImpactNormal = Hit.ImpactNormal;
            return true;
        }

        CurrentLocation = NextLocation;
        Velocity = NextVelocity;
        AccumulatedTime += StepTime;
    }

    return false;
}
//// END NEW CODE
