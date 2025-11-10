// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VerdunLocationData.generated.h"

/**
 * Represents a single historical location on the Verdun battlefield
 */
USTRUCT(BlueprintType)
struct FVerdunLocation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FString Type; // fortification, hill, destroyed_village, woods, ravine, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	double Latitude = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	double Longitude = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	float ElevationMeters = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FString Significance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FString Phase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FString DateKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FString Notes;

	// UE5 local coordinates (calculated from lat/lon)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FVector UE5Location = FVector::ZeroVector;
};

/**
 * Manages the locations database for the Verdun battlefield
 */
UCLASS(BlueprintType)
class VERDUNAUTOTERRAINBUILDER_API UVerdunLocationDatabase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TArray<FVerdunLocation> Locations;

	/**
	 * Load locations from CSV file
	 */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool LoadFromCSV(const FString& FilePath);

	/**
	 * Load locations from JSON file (UE5 export format)
	 */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool LoadFromJSON(const FString& FilePath);

	/**
	 * Get all locations of a specific type
	 */
	UFUNCTION(BlueprintCallable, Category = "Query")
	TArray<FVerdunLocation> GetLocationsByType(const FString& Type) const;

	/**
	 * Get a location by name
	 */
	UFUNCTION(BlueprintCallable, Category = "Query")
	FVerdunLocation GetLocationByName(const FString& Name, bool& bFound) const;

	/**
	 * Get all fortifications (forts)
	 */
	UFUNCTION(BlueprintCallable, Category = "Query")
	TArray<FVerdunLocation> GetFortifications() const;

	/**
	 * Get all destroyed villages
	 */
	UFUNCTION(BlueprintCallable, Category = "Query")
	TArray<FVerdunLocation> GetDestroyedVillages() const;

	/**
	 * Get all hills
	 */
	UFUNCTION(BlueprintCallable, Category = "Query")
	TArray<FVerdunLocation> GetHills() const;

	/**
	 * Convert all locations to UE5 coordinates using terrain parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Data")
	void ConvertToUE5Coordinates(class UVerdunTerrainParameters* TerrainParams);
};
