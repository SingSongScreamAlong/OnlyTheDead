// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VerdunTerrainParameters.generated.h"

USTRUCT(BlueprintType)
struct FVerdunMaterialComposition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Mud = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Chalk = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float DeadGrass = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Rock = 5.0f;
};

USTRUCT(BlueprintType)
struct FVerdunCraterParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	FString ShellType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	float DiameterMeters = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	float DepthMeters = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	float Percentage = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	FString Note;
};

USTRUCT(BlueprintType)
struct FVerdunCraterDensity
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	float HighIntensity = 75.0f; // craters per hectare

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	float MediumIntensity = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	float LowIntensity = 12.5f;
};

USTRUCT(BlueprintType)
struct FVerdunTrenchParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trenches")
	float TotalLengthKm = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trenches")
	float WidthMeters = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trenches")
	float DepthMeters = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trenches")
	FString Pattern = TEXT("Zigzag");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trenches")
	float TraverseSpacingMeters = 12.5f;
};

USTRUCT(BlueprintType)
struct FVerdunGeographicBounds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	double North = 49.25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	double South = 49.14;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	double East = 5.47;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	double West = 5.31;
};

USTRUCT(BlueprintType)
struct FVerdunGeographicCenter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	double Latitude = 49.2050;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	double Longitude = 5.4300;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	float ElevationMeters = 365.0f;
};

/**
 * Main terrain parameters data object
 */
UCLASS(BlueprintType)
class VERDUNAUTOTERRAINBUILDER_API UVerdunTerrainParameters : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString TerrainName = TEXT("Verdun_Battlefield_1916");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	FVerdunGeographicCenter GeographicCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	FVerdunGeographicBounds GeographicBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geography")
	float AreaKm2 = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation")
	float ElevationMinMeters = 165.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation")
	float ElevationMaxMeters = 390.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation")
	float ElevationMeanMeters = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	FVerdunMaterialComposition SurfaceMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	int32 TotalCraters = 500000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	FVerdunCraterDensity CraterDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craters")
	TArray<FVerdunCraterParameters> CraterTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trenches")
	FVerdunTrenchParameters TrenchParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float TreeSurvivalRate = 1.0f; // 1%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float GroundVegetationSurvivalRate = 5.0f; // 5%

	// Coordinate conversion constants (from WGS84 to UE5 local coords in cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
	float MetersPerDegreeLongitude = 71370.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
	float MetersPerDegreeLatitude = 111320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
	float CentimetersPerMeter = 100.0f;

	/**
	 * Convert WGS84 lat/lon to UE5 local coordinates (cm)
	 * Origin is at the battlefield center
	 */
	UFUNCTION(BlueprintCallable, Category = "Coordinates")
	FVector ConvertLatLonToUE5Coords(double Latitude, double Longitude, float ElevationMeters) const;

	/**
	 * Load terrain parameters from JSON file
	 */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool LoadFromJSON(const FString& FilePath);
};
