// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VerdunCraterGenerator.generated.h"

UENUM(BlueprintType)
enum class EVerdunCraterIntensity : uint8
{
	Low UMETA(DisplayName = "Low Intensity (5-20 craters/hectare)"),
	Medium UMETA(DisplayName = "Medium Intensity (20-50 craters/hectare)"),
	High UMETA(DisplayName = "High Intensity (50-100 craters/hectare)")
};

USTRUCT(BlueprintType)
struct FGeneratedCrater
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	float DiameterCm = 200.0f; // In centimeters (UE5 units)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	float DepthCm = 50.0f; // In centimeters (UE5 units)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	FString ShellType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	bool bWaterFilled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater")
	float Rotation = 0.0f; // Random rotation for variety
};

/**
 * Procedurally generates shell craters across the battlefield
 */
UCLASS(BlueprintType)
class VERDUNAUTOTERRAINBUILDER_API AVerdunCraterGenerator : public AActor
{
	GENERATED_BODY()

public:
	AVerdunCraterGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	class UVerdunTerrainParameters* TerrainParameters = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	class UVerdunLocationDatabase* LocationDatabase = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FVector2D BattlefieldSizeMeters = FVector2D(8000.0f, 8000.0f); // ~64 km²

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 RandomSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bWaterFilledCraters = true; // 60% of craters water-filled

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float WaterFilledPercentage = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	TArray<FGeneratedCrater> GeneratedCraters;

	/**
	 * Generate craters across the battlefield based on terrain parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateCraters();

	/**
	 * Clear all generated craters
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void ClearCraters();

	/**
	 * Get crater density at a specific world location
	 * Returns craters per hectare based on proximity to high-intensity zones
	 */
	UFUNCTION(BlueprintCallable, Category = "Query")
	float GetCraterDensityAtLocation(FVector Location) const;

	/**
	 * Spawn crater mesh actors for all generated craters
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void SpawnCraterMeshes(TSubclassOf<AActor> CraterActorClass);

	/**
	 * Apply craters to landscape using render target
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void ApplyCratersToLandscape(class ALandscape* Landscape);

private:
	// Calculate distance-based intensity for crater distribution
	float CalculateIntensityAtLocation(FVector Location) const;

	// Generate a single crater at a location
	FGeneratedCrater GenerateCraterAtLocation(FVector Location, FRandomStream& RandomStream) const;

	// Check if a location is in a high-intensity zone (near forts)
	bool IsHighIntensityZone(FVector Location) const;

	// Check if a location is in a medium-intensity zone
	bool IsMediumIntensityZone(FVector Location) const;
};
