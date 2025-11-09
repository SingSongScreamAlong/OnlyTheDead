// Verdun World Builder Plugin
// Runtime terrain deformation system - destructible landscape

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Landscape.h"
#include "VerdunTerrainDeformation.generated.h"

/**
 * Deformation shape types
 */
UENUM(BlueprintType)
enum class EVerdunDeformationShape : uint8
{
	Crater UMETA(DisplayName = "Crater (circular depression)"),
	Trench UMETA(DisplayName = "Trench (linear depression)"),
	Mound UMETA(DisplayName = "Mound (raised area)"),
	Custom UMETA(DisplayName = "Custom Shape")
};

/**
 * Deformation falloff types
 */
UENUM(BlueprintType)
enum class EVerdunDeformationFalloff : uint8
{
	Linear UMETA(DisplayName = "Linear"),
	Smooth UMETA(DisplayName = "Smooth (cosine)"),
	Sharp UMETA(DisplayName = "Sharp (exponential)")
};

/**
 * Single deformation operation
 */
USTRUCT(BlueprintType)
struct FVerdunDeformation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
	float Depth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
	EVerdunDeformationShape Shape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
	EVerdunDeformationFalloff Falloff;

	/** For trenches: direction vector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
	FVector Direction;

	/** For trenches: length */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
	float Length;

	FVerdunDeformation()
		: Location(FVector::ZeroVector)
		, Radius(600.0f) // 6m default
		, Depth(150.0f) // 1.5m default
		, Shape(EVerdunDeformationShape::Crater)
		, Falloff(EVerdunDeformationFalloff::Smooth)
		, Direction(FVector::ForwardVector)
		, Length(1000.0f)
	{}
};

/**
 * Terrain deformation component
 * Attach to landscape to make it deformable at runtime
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VERDUNWORLDBUILDER_API UVerdunTerrainDeformation : public UActorComponent
{
	GENERATED_BODY()

public:
	UVerdunTerrainDeformation();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// SETTINGS
	// ========================================

	/** Target landscape to deform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
	ALandscape* TargetLandscape;

	/** Auto-find landscape if not set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
	bool bAutoFindLandscape;

	/** Enable real-time deformation updates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
	bool bEnableRuntimeDeformation;

	/** Batch multiple deformations for performance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
	bool bBatchDeformations;

	/** Maximum deformations per frame when batching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxDeformationsPerFrame;

	/** Smooth deformation edges for realistic look */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
	bool bSmoothEdges;

	/** Update collision after deformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
	bool bUpdateCollision;

	// ========================================
	// RUNTIME DATA
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	TArray<FVerdunDeformation> PendingDeformations;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	int32 TotalDeformationsApplied;

	// ========================================
	// PUBLIC FUNCTIONS
	// ========================================

	/**
	 * Deform terrain with crater
	 * @param Location World position of crater center
	 * @param Radius Crater radius in cm
	 * @param Depth Crater depth in cm (positive = depression, negative = mound)
	 * @param Falloff Falloff type for smooth edges
	 * @param bImmediate Apply immediately or batch for next frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Deformation")
	void DeformCrater(FVector Location, float Radius, float Depth, EVerdunDeformationFalloff Falloff = EVerdunDeformationFalloff::Smooth, bool bImmediate = false);

	/**
	 * Deform terrain with trench
	 * @param StartLocation Trench start position
	 * @param EndLocation Trench end position
	 * @param Width Trench width in cm
	 * @param Depth Trench depth in cm
	 * @param bZigzag Create zigzag pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Deformation")
	void DeformTrench(FVector StartLocation, FVector EndLocation, float Width, float Depth, bool bZigzag = false);

	/**
	 * Apply custom deformation
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Deformation")
	void ApplyDeformation(const FVerdunDeformation& Deformation, bool bImmediate = false);

	/**
	 * Batch apply multiple deformations efficiently
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Deformation")
	void BatchApplyDeformations(const TArray<FVerdunDeformation>& Deformations);

	/**
	 * Process pending deformations (called automatically if batching)
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Deformation")
	void ProcessPendingDeformations();

	/**
	 * Clear all pending deformations
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Deformation")
	void ClearPendingDeformations();

	/**
	 * Reset terrain to original state
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Deformation")
	void ResetTerrain();

	/**
	 * Get terrain height at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Deformation")
	bool GetTerrainHeightAtLocation(FVector Location, float& OutHeight);

	// ========================================
	// EVENTS
	// ========================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnDeformationApplied(const FVerdunDeformation& Deformation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnBatchDeformationsComplete(int32 NumApplied);

private:
	void ApplyDeformationInternal(const FVerdunDeformation& Deformation);
	void ApplyCraterDeformation(const FVerdunDeformation& Deformation);
	void ApplyTrenchDeformation(const FVerdunDeformation& Deformation);
	float CalculateFalloff(float Distance, float Radius, EVerdunDeformationFalloff FalloffType);

	// Cache for landscape edit interface
	TArray<uint16> OriginalHeightData;
	bool bHasOriginalData;
};
