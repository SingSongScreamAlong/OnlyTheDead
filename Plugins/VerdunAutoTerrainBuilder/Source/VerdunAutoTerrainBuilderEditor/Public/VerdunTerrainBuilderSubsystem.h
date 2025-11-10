// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "VerdunTerrainBuilderSubsystem.generated.h"

/**
 * Editor subsystem for managing Verdun terrain building operations
 */
UCLASS()
class VERDUNAUTOTERRAINBUILDEREDITOR_API UVerdunTerrainBuilderSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Open the terrain builder window/UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	void OpenTerrainBuilderWindow();

	/**
	 * Build the entire Verdun battlefield terrain automatically
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	void BuildVerdunTerrain();

	/**
	 * Import heightmap and create landscape
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	bool ImportHeightmapAndCreateLandscape();

	/**
	 * Load terrain parameters from JSON
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	bool LoadTerrainParameters();

	/**
	 * Load locations database from CSV
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	bool LoadLocationsDatabase();

	/**
	 * Generate and place all craters
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	bool GenerateCraters();

	/**
	 * Generate all trench systems
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	bool GenerateTrenches();

	/**
	 * Place location markers for historical sites
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	bool PlaceLocationMarkers();

	/**
	 * Get the data path for the verdun_anthology folder
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun Terrain Builder")
	FString GetVerdunDataPath() const;

	// Data objects
	UPROPERTY()
	class UVerdunTerrainParameters* TerrainParameters = nullptr;

	UPROPERTY()
	class UVerdunLocationDatabase* LocationDatabase = nullptr;

	UPROPERTY()
	class AVerdunCraterGenerator* CraterGenerator = nullptr;

	UPROPERTY()
	class AVerdunTrenchGenerator* TrenchGenerator = nullptr;

private:
	// Internal helpers
	bool ValidateDataFiles();
	void LogProgress(const FString& Message);
};
