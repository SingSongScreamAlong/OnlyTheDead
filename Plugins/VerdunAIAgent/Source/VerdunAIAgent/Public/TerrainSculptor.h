// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Landscape.h"
#include "LandscapeEdit.h"
#include "TerrainSculptor.generated.h"

/**
 * Terrain operation types
 */
UENUM(BlueprintType)
enum class ETerrainOperation : uint8
{
	Raise UMETA(DisplayName = "Raise"),
	Lower UMETA(DisplayName = "Lower"),
	Flatten UMETA(DisplayName = "Flatten"),
	Smooth UMETA(DisplayName = "Smooth"),
	Noise UMETA(DisplayName = "Add Noise"),
	Terrace UMETA(DisplayName = "Terrace"),
	Crater UMETA(DisplayName = "Create Crater"),
	TrenchExcavation UMETA(DisplayName = "Excavate Trench")
};

/**
 * Terrain sculpting parameters
 */
USTRUCT(BlueprintType)
struct FTerrainSculptParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	ETerrainOperation Operation = ETerrainOperation::Raise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Radius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Strength = 1.0f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Falloff = 0.5f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float TargetHeight = 0.0f; // For flatten operation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	bool bUseTargetHeight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float NoiseScale = 1.0f; // For noise operation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 TerraceSteps = 5; // For terrace operation
};

/**
 * Crater configuration
 */
USTRUCT(BlueprintType)
struct FCraterParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Diameter = 800.0f; // 8 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Depth = 150.0f; // 1.5 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float EdgeSharpness = 0.7f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float RimHeight = 50.0f; // Height of crater rim

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	bool bRandomizeShape = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 RandomSeed = 0;
};

/**
 * Trench excavation parameters
 */
USTRUCT(BlueprintType)
struct FTrenchExcavationParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	TArray<FVector> Path;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Width = 150.0f; // 1.5 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Depth = 200.0f; // 2 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float WallAngle = 80.0f; // Degrees from horizontal

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float FloorWidth = 100.0f; // 1 meter

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	bool bCreateParapet = true; // Raised edge on friendly side

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float ParapetHeight = 50.0f; // 0.5 meters
};

DECLARE_DELEGATE_TwoParams(FOnTerrainModified, bool /* bSuccess */, const FString& /* Result */);

/**
 * Terrain Sculptor - Full Landscape API integration
 * Provides real terrain manipulation capabilities
 */
UCLASS(BlueprintType)
class VERDUNAIAGENT_API UTerrainSculptor : public UObject
{
	GENERATED_BODY()

public:
	UTerrainSculptor();

	/**
	 * Find landscape in world
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static ALandscape* FindLandscape(UWorld* World);

	/**
	 * Sculpt terrain with parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool SculptTerrain(
		UWorld* World,
		const FTerrainSculptParams& Params,
		FString& OutResult
	);

	/**
	 * Create an artillery crater
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool CreateCrater(
		UWorld* World,
		const FCraterParams& Params,
		FString& OutResult
	);

	/**
	 * Excavate a trench path
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool ExcavateTrench(
		UWorld* World,
		const FTrenchExcavationParams& Params,
		FString& OutResult
	);

	/**
	 * Raise terrain at location
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool RaiseTerrain(
		UWorld* World,
		const FVector& Location,
		float Radius,
		float Strength,
		FString& OutResult
	);

	/**
	 * Lower terrain at location
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool LowerTerrain(
		UWorld* World,
		const FVector& Location,
		float Radius,
		float Strength,
		FString& OutResult
	);

	/**
	 * Flatten terrain to specific height
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool FlattenTerrain(
		UWorld* World,
		const FVector& Location,
		float Radius,
		float TargetHeight,
		FString& OutResult
	);

	/**
	 * Smooth terrain
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool SmoothTerrain(
		UWorld* World,
		const FVector& Location,
		float Radius,
		float Strength,
		FString& OutResult
	);

	/**
	 * Add noise to terrain for realism
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool AddTerrainNoise(
		UWorld* World,
		const FVector& Location,
		float Radius,
		float Scale,
		float Strength,
		FString& OutResult
	);

	/**
	 * Create terraced terrain
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static bool CreateTerraces(
		UWorld* World,
		const FVector& Location,
		float Radius,
		int32 Steps,
		FString& OutResult
	);

	/**
	 * Get terrain height at location
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static float GetTerrainHeight(UWorld* World, const FVector2D& Location);

	/**
	 * Get terrain normal at location
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static FVector GetTerrainNormal(UWorld* World, const FVector2D& Location);

	/**
	 * Sample terrain heightmap in a region
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Terrain")
	static TArray<float> SampleHeightmap(
		UWorld* World,
		const FVector2D& Center,
		float Radius,
		int32 SampleCount
	);

private:
	// Core terrain modification
	static bool ModifyLandscapeHeight(
		ALandscape* Landscape,
		const FVector2D& Center,
		float Radius,
		TFunction<float(const FVector2D&, float, float)> HeightFunction,
		FString& OutResult
	);

	// Falloff curves
	static float CalculateFalloff(float Distance, float Radius, float FalloffAmount);
	static float SmoothFalloff(float T);

	// Crater generation
	static float CalculateCraterHeight(
		const FVector2D& Point,
		const FVector2D& Center,
		const FCraterParams& Params,
		float OriginalHeight
	);

	// Trench excavation
	static bool ExcavateTrenchSegment(
		ALandscape* Landscape,
		const FVector& Start,
		const FVector& End,
		const FTrenchExcavationParams& Params,
		FString& OutResult
	);

	// Utility functions
	static void GetLandscapeExtent(ALandscape* Landscape, FVector2D& Min, FVector2D& Max, float& ZScale);
	static FIntPoint WorldToLandscapeCoordinates(ALandscape* Landscape, const FVector2D& WorldLocation);
	static FVector2D LandscapeToWorldCoordinates(ALandscape* Landscape, const FIntPoint& LandscapeCoords);
};
