// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeightmapLoader.h"
#include "GPSCoordinateConverter.h"

class ALandscape;
class UWorld;
class ULandscapeInfo;
class UMaterialInterface;

/**
 * Manages landscape creation and modification for Claude Terminal
 * Integrates heightmap loading, GPS conversion, and terrain manipulation
 */
class CLAUDETERMINALEDITOR_API FLandscapeManager
{
public:
	FLandscapeManager();
	~FLandscapeManager();

	/**
	 * Create a new landscape from heightmap file
	 * @param World World to spawn landscape in
	 * @param HeightmapPath Path to heightmap file
	 * @param Location World location for landscape origin
	 * @param Scale Scale factor (UE5 units per heightmap pixel)
	 * @param Material Optional landscape material
	 * @param OutLog Execution log
	 * @return Spawned landscape actor, or nullptr on failure
	 */
	ALandscape* CreateLandscapeFromHeightmap(
		UWorld* World,
		const FString& HeightmapPath,
		const FVector& Location,
		const FVector& Scale,
		UMaterialInterface* Material,
		FString& OutLog
	);

	/**
	 * Create landscape from GPS-referenced terrain data
	 * @param World World to spawn landscape in
	 * @param HeightmapPath Path to heightmap file
	 * @param GPSOriginLat GPS latitude of heightmap origin
	 * @param GPSOriginLon GPS longitude of heightmap origin
	 * @param MetersPerPixel Real-world scale (meters per heightmap pixel)
	 * @param Material Optional landscape material
	 * @param OutLog Execution log
	 * @return Spawned landscape actor, or nullptr on failure
	 */
	ALandscape* CreateLandscapeFromGPSData(
		UWorld* World,
		const FString& HeightmapPath,
		double GPSOriginLat,
		double GPSOriginLon,
		double MetersPerPixel,
		UMaterialInterface* Material,
		FString& OutLog
	);

	/**
	 * Modify existing landscape height at location
	 * @param World World containing landscape
	 * @param Location World location to modify
	 * @param Radius Affected radius in cm
	 * @param Strength Modification strength (0-1)
	 * @param Operation Type of operation (raise, lower, smooth, flatten)
	 * @param OutLog Execution log
	 * @return true if successful
	 */
	bool ModifyLandscapeTerrain(
		UWorld* World,
		const FVector& Location,
		float Radius,
		float Strength,
		const FString& Operation,
		FString& OutLog
	);

	/**
	 * Create a crater in landscape at location
	 * @param World World containing landscape
	 * @param Location Center of crater
	 * @param Radius Crater radius in cm
	 * @param Depth Crater depth in cm
	 * @param OutLog Execution log
	 * @return true if successful
	 */
	bool CreateCrater(
		UWorld* World,
		const FVector& Location,
		float Radius,
		float Depth,
		FString& OutLog
	);

	/**
	 * Create a trench line in landscape
	 * @param World World containing landscape
	 * @param StartLocation Trench start point
	 * @param EndLocation Trench end point
	 * @param Width Trench width in cm
	 * @param Depth Trench depth in cm
	 * @param OutLog Execution log
	 * @return true if successful
	 */
	bool CreateTrench(
		UWorld* World,
		const FVector& StartLocation,
		const FVector& EndLocation,
		float Width,
		float Depth,
		FString& OutLog
	);

	/**
	 * Get GPS converter instance
	 */
	FGPSCoordinateConverter& GetGPSConverter() { return GPSConverter; }

private:
	// Helper: Find landscape at location
	ALandscape* FindLandscapeAtLocation(UWorld* World, const FVector& Location);

	// Helper: Get landscape info
	ULandscapeInfo* GetLandscapeInfo(ALandscape* Landscape);

	// Helper: Convert heightmap data to landscape format
	bool ConvertHeightmapData(
		const TArray<uint16>& HeightData,
		int32 Width,
		int32 Height,
		TArray<uint16>& OutLandscapeData
	);

	// Helper: Calculate optimal landscape size for heightmap
	void CalculateLandscapeSize(
		int32 HeightmapWidth,
		int32 HeightmapHeight,
		int32& OutQuadsPerComponent,
		int32& OutComponentsX,
		int32& OutComponentsY
	);

	// Helper: Apply terrain modification
	bool ApplyTerrainModification(
		ULandscapeInfo* LandscapeInfo,
		const FVector& Location,
		float Radius,
		TFunction<float(float, float)> ModificationFunc
	);

	// Helper: Create crater deformation
	float CalculateCraterDeformation(float DistanceFromCenter, float Radius, float Depth);

	// Components
	FHeightmapLoader HeightmapLoader;
	FGPSCoordinateConverter GPSConverter;
};
