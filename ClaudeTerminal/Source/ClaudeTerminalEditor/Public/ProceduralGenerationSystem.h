// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * Procedural generation constraint
 */
struct FGenerationConstraint
{
	float MinSpacing = 100.0f;
	float MaxSpacing = 500.0f;
	float MinSlope = 0.0f;
	float MaxSlope = 45.0f;
	bool bAvoidWater = true;
	bool bFollowTerrain = true;
	TArray<FString> RequiredTags;
	TArray<AActor*> AvoidActors;
};

/**
 * Procedural generation result
 */
struct FGenerationResult
{
	bool bSuccess = false;
	TArray<AActor*> GeneratedActors;
	int32 TotalGenerated = 0;
	FString ErrorMessage;
	float GenerationTime = 0.0f;
};

/**
 * Noise parameters for distribution
 */
struct FNoiseParameters
{
	float Frequency = 1.0f;
	float Amplitude = 1.0f;
	int32 Octaves = 4;
	float Persistence = 0.5f;
	float Lacunarity = 2.0f;
	int32 Seed = 0;
};

/**
 * Procedural generation system for creating entire scenes
 * TIER 3.3 Feature - Revolutionary
 */
class CLAUDETERMINALEDITOR_API FProceduralGenerationSystem
{
public:
	/**
	 * Generate trench network procedurally
	 * @param StartLocation Starting point
	 * @param AreaSize Size of area to cover
	 * @param World World context
	 * @param Constraints Generation constraints
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateTrenchNetwork(
		const FVector& StartLocation,
		const FVector& AreaSize,
		UWorld* World,
		const FGenerationConstraint& Constraints,
		FGenerationResult& OutResult);

	/**
	 * Generate destroyed village procedurally
	 * @param CenterLocation Village center
	 * @param Radius Village radius
	 * @param DamageLevel 0.0-1.0 (0=intact, 1=destroyed)
	 * @param World World context
	 * @param Constraints Generation constraints
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateDestroyedVillage(
		const FVector& CenterLocation,
		float Radius,
		float DamageLevel,
		UWorld* World,
		const FGenerationConstraint& Constraints,
		FGenerationResult& OutResult);

	/**
	 * Generate realistic forest
	 * @param CenterLocation Forest center
	 * @param AreaSize Forest size
	 * @param Density Tree density (trees per 1000 sq units)
	 * @param World World context
	 * @param Constraints Generation constraints
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateForest(
		const FVector& CenterLocation,
		const FVector& AreaSize,
		float Density,
		UWorld* World,
		const FGenerationConstraint& Constraints,
		FGenerationResult& OutResult);

	/**
	 * Generate building cluster
	 * @param CenterLocation Cluster center
	 * @param BuildingCount Number of buildings
	 * @param Style "medieval", "wwi", "modern"
	 * @param World World context
	 * @param Constraints Generation constraints
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateBuildingCluster(
		const FVector& CenterLocation,
		int32 BuildingCount,
		const FString& Style,
		UWorld* World,
		const FGenerationConstraint& Constraints,
		FGenerationResult& OutResult);

	/**
	 * Generate road network
	 * @param StartPoint Start location
	 * @param EndPoints Destination locations
	 * @param RoadWidth Width of roads
	 * @param World World context
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateRoadNetwork(
		const FVector& StartPoint,
		const TArray<FVector>& EndPoints,
		float RoadWidth,
		UWorld* World,
		FGenerationResult& OutResult);

	/**
	 * Generate Perlin noise value at location
	 * @param Location 2D location
	 * @param Params Noise parameters
	 * @return Noise value (typically -1.0 to 1.0)
	 */
	static float GeneratePerlinNoise(const FVector2D& Location, const FNoiseParameters& Params);

	/**
	 * Generate Simplex noise value
	 * @param Location 2D location
	 * @param Params Noise parameters
	 * @return Noise value
	 */
	static float GenerateSimplexNoise(const FVector2D& Location, const FNoiseParameters& Params);

	/**
	 * Apply Poisson disc distribution
	 * @param Bounds Distribution area
	 * @param MinDistance Minimum distance between points
	 * @param MaxAttempts Maximum attempts per point
	 * @return Array of distributed points
	 */
	static TArray<FVector> GeneratePoissonDiscDistribution(
		const FBox2D& Bounds,
		float MinDistance,
		int32 MaxAttempts = 30);

	/**
	 * Check if location satisfies constraints
	 * @param Location Location to check
	 * @param World World context
	 * @param Constraints Constraints to check
	 * @return true if valid
	 */
	static bool CheckConstraints(
		const FVector& Location,
		UWorld* World,
		const FGenerationConstraint& Constraints);

	/**
	 * Calculate terrain slope at location
	 * @param Location Location to check
	 * @param World World context
	 * @return Slope in degrees
	 */
	static float CalculateTerrainSlope(const FVector& Location, UWorld* World);

	/**
	 * Find valid placement locations using noise and constraints
	 * @param Bounds Search area
	 * @param Count Number of locations needed
	 * @param World World context
	 * @param Constraints Placement constraints
	 * @return Array of valid locations
	 */
	static TArray<FVector> FindValidPlacements(
		const FBox& Bounds,
		int32 Count,
		UWorld* World,
		const FGenerationConstraint& Constraints);

private:
	// Perlin noise helper
	static float PerlinNoise2D(float x, float y, int32 seed);

	// Simplex noise helper
	static float SimplexNoise2D(float x, float y);

	// Gradient calculation
	static float DotGridGradient(int32 ix, int32 iy, float x, float y, int32 seed);

	// Interpolation
	static float Fade(float t);
	static float Lerp(float a, float b, float t);
};
