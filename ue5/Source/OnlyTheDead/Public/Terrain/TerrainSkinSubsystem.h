//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TerrainSkinSubsystem.generated.h"

class UCurveFloat;

UENUM(BlueprintType)
enum class EOTDTerrainMaterial : uint8
{
    Mud,
    Chalk,
    Forest,
    Urban
};

USTRUCT()
struct FOTDQueuedShellImpact
{
    GENERATED_BODY()

    FVector Start = FVector::ZeroVector;
    FVector Velocity = FVector::ZeroVector;
    float Yield = 1.0f;
    EOTDTerrainMaterial Material = EOTDTerrainMaterial::Mud;
    float MaxSimulationTime = 10.f;
    int32 MaxIterations = 32;
};

USTRUCT(BlueprintType)
struct FOTDNoiseProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Frequency = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 Octaves = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Persistence = 0.5f;
};

USTRUCT(BlueprintType)
struct FOTDCraterDescriptor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    float Radius = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    float Depth = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    float RimHeight = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    float RimWidth = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    float EntryAngleDegrees = 75.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    FVector ImpactDirection = FVector::DownVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    float Yield = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    EOTDTerrainMaterial Material = EOTDTerrainMaterial::Mud;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    FOTDNoiseProfile NoiseProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
    float LifetimeSeconds = 600.f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Crater")
    FGuid CraterId;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Crater")
    double SpawnTimeSeconds = 0.0;
};

USTRUCT()
struct FOTDActiveCrater
{
    GENERATED_BODY()

    FOTDCraterDescriptor Descriptor;
    TWeakObjectPtr<UCurveFloat> DeformationCurve;
};

USTRUCT(BlueprintType)
struct FOTDMaterialResponseParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float RadiusMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float DepthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float RimHeightMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float RimWidthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float LifetimeScale = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOTDCraterEventSignature, const FGuid&, CraterId, const FOTDCraterDescriptor&, Descriptor);

UCLASS()
class UTerrainSkinSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UTerrainSkinSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    UFUNCTION(BlueprintCallable, Category = "TerrainSkin")
    FGuid RegisterCrater(const FOTDCraterDescriptor& Descriptor);

    UFUNCTION(BlueprintCallable, Category = "TerrainSkin")
    FGuid RegisterShellImpactCrater(const FVector& ImpactPoint, const FVector& ImpactDirection, float Yield, EOTDTerrainMaterial Material);

    UFUNCTION(BlueprintCallable, Category = "TerrainSkin")
    void QueueShellTrajectory(const FVector& StartLocation, const FVector& InitialVelocity, float Yield, EOTDTerrainMaterial Material);

    UFUNCTION(BlueprintCallable, Category = "TerrainSkin")
    bool RemoveCrater(const FGuid& CraterId);

    UFUNCTION(BlueprintCallable, Category = "TerrainSkin")
    void ClearAllCraters();

    UFUNCTION(BlueprintPure, Category = "TerrainSkin")
    const FOTDCraterDescriptor* GetCrater(const FGuid& CraterId) const;

    UFUNCTION(BlueprintPure, Category = "TerrainSkin")
    float SampleDeformation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "TerrainSkin")
    void SetMaterialNoiseProfile(EOTDTerrainMaterial Material, const FOTDNoiseProfile& Profile);

    UPROPERTY(BlueprintAssignable, Category = "TerrainSkin")
    FOTDCraterEventSignature OnCraterAdded;

    UPROPERTY(BlueprintAssignable, Category = "TerrainSkin")
    FOTDCraterEventSignature OnCraterRemoved;

protected:
    void PruneExpiredCraters();
    float ComputeCraterContribution(const FOTDCraterDescriptor& Descriptor, const FVector& WorldLocation) const;
    const FOTDNoiseProfile& GetNoiseProfileForMaterial(EOTDTerrainMaterial Material) const;
    const FOTDMaterialResponseParams& GetMaterialResponse(EOTDTerrainMaterial Material) const;
    FOTDCraterDescriptor BuildCraterDescriptor(const FVector& ImpactPoint, const FVector& ImpactDirection, float Yield, EOTDTerrainMaterial Material) const;
    float CalculateAngleMultiplier(float EntryAngleDegrees) const;
    float GenerateNoiseValue(const FOTDNoiseProfile& Profile, const FVector2D& Location) const;

    bool IsContributionNegligible(float Distance, float Radius) const;
    void ProcessQueuedShells(float DeltaTime);
    bool SolveShellImpact(const FOTDQueuedShellImpact& Impact, FVector& OutImpactPoint, FVector& OutImpactNormal) const;

protected:
    UPROPERTY(EditAnywhere, Category = "TerrainSkin")
    float MaxTrackedRadius = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TerrainSkin", meta = (ClampMin = "0"))
    int32 MaxActiveCraters = 1024;

    UPROPERTY(EditAnywhere, Category = "TerrainSkin")
    bool bEnableDebugDraw = false;

    UPROPERTY(EditAnywhere, Category = "TerrainSkin|Performance")
    int32 MaxShellSolvesPerTick = 4;

    UPROPERTY(EditAnywhere, Category = "TerrainSkin|Performance")
    float ShellTraceStep = 200.f;

    UPROPERTY(EditAnywhere, Category = "TerrainSkin|Performance")
    float ShellGravityScale = 1.0f;

    UPROPERTY(EditAnywhere, Category = "TerrainSkin|Performance")
    float MinCraterSpacing = 150.f;

public:
    UFUNCTION(BlueprintCallable, Category = "TerrainSkin")
    void SetMaxActiveCraters(int32 NewMax);

private:
    TArray<FOTDActiveCrater> ActiveCraters;
    TMap<EOTDTerrainMaterial, FOTDNoiseProfile> MaterialNoiseProfiles;
    TMap<EOTDTerrainMaterial, FOTDMaterialResponseParams> MaterialResponseParams;
    TArray<FOTDQueuedShellImpact> QueuedShellImpacts;
};
//// END NEW CODE
