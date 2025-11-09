// Verdun World Builder Plugin
// Data structures for Tier 1 historical terrain data (documented facts only)

#pragma once

#include "CoreMinimal.h"
#include "VerdunTerrainData.generated.h"

/**
 * Location types in the Verdun battlefield
 */
UENUM(BlueprintType)
enum class EVerdunLocationType : uint8
{
	Fortification UMETA(DisplayName = "Fortification"),
	Hill UMETA(DisplayName = "Hill"),
	DestroyedVillage UMETA(DisplayName = "Destroyed Village"),
	Village UMETA(DisplayName = "Village"),
	Woods UMETA(DisplayName = "Woods"),
	Ravine UMETA(DisplayName = "Ravine"),
	River UMETA(DisplayName = "River"),
	Ridge UMETA(DisplayName = "Ridge"),
	Road UMETA(DisplayName = "Road"),
	Trench UMETA(DisplayName = "Trench"),
	Artillery UMETA(DisplayName = "Artillery Position"),
	Observation UMETA(DisplayName = "Observation Post"),
	Command UMETA(DisplayName = "Command Post"),
	Other UMETA(DisplayName = "Other")
};

/**
 * Single historical location (Tier 1 documented data)
 */
USTRUCT(BlueprintType)
struct FVerdunLocation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	EVerdunLocationType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	double Latitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	double Longitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	float ElevationMeters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	FString Significance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	FString Phase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	FString DateKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun Location")
	FString Notes;

	// Converted world position (set by loader)
	UPROPERTY(BlueprintReadOnly, Category = "Verdun Location")
	FVector WorldPosition;

	FVerdunLocation()
		: Type(EVerdunLocationType::Other)
		, Latitude(0.0)
		, Longitude(0.0)
		, ElevationMeters(0.0f)
		, WorldPosition(FVector::ZeroVector)
	{}
};

/**
 * Geographic bounds of the battlefield
 */
USTRUCT(BlueprintType)
struct FVerdunGeographicBounds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geographic Bounds")
	double North;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geographic Bounds")
	double South;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geographic Bounds")
	double East;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geographic Bounds")
	double West;

	FVerdunGeographicBounds()
		: North(49.25), South(49.14), East(5.47), West(5.31)
	{}
};

/**
 * Battlefield center point
 */
USTRUCT(BlueprintType)
struct FVerdunBattlefieldCenter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battlefield Center")
	double Latitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battlefield Center")
	double Longitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battlefield Center")
	float ElevationMeters;

	FVerdunBattlefieldCenter()
		: Latitude(49.2050), Longitude(5.4300), ElevationMeters(365.0f)
	{}
};

/**
 * Surface material composition (Tier 1 documented from historical photos/testimonies)
 */
USTRUCT(BlueprintType)
struct FVerdunSurfaceMaterials
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Materials", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float MudPercentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Materials", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ChalkPercentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Materials", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DeadGrassPercentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Materials", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RockPercentage;

	FVerdunSurfaceMaterials()
		: MudPercentage(70.0f)
		, ChalkPercentage(15.0f)
		, DeadGrassPercentage(10.0f)
		, RockPercentage(5.0f)
	{}
};

/**
 * Main terrain data container (Tier 1 only - documented facts)
 */
USTRUCT(BlueprintType)
struct FVerdunTerrainData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
	FString TerrainName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
	FVerdunBattlefieldCenter BattlefieldCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
	FVerdunGeographicBounds GeographicBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
	float MinElevationMeters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
	float MaxElevationMeters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
	float MeanElevationMeters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
	FVerdunSurfaceMaterials SurfaceMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
	TArray<FVerdunLocation> Locations;

	FVerdunTerrainData()
		: TerrainName(TEXT("Verdun_Battlefield_1916"))
		, MinElevationMeters(165.0f)
		, MaxElevationMeters(390.0f)
		, MeanElevationMeters(300.0f)
	{}
};
