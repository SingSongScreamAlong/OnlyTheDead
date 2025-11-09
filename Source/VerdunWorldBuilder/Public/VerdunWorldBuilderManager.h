// Verdun World Builder Plugin
// Main manager actor for automated world building

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VerdunTerrainData.h"
#include "VerdunArtilleryImpactGenerator.h"
#include "VerdunWorldBuilderManager.generated.h"

/**
 * World building stages
 */
UENUM(BlueprintType)
enum class EVerdunBuildStage : uint8
{
	NotStarted UMETA(DisplayName = "Not Started"),
	LoadingData UMETA(DisplayName = "Loading Data"),
	SpawningLocations UMETA(DisplayName = "Spawning Locations"),
	GeneratingArtillery UMETA(DisplayName = "Generating Artillery Impacts"),
	ApplyingMaterials UMETA(DisplayName = "Applying Materials"),
	Complete UMETA(DisplayName = "Complete"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * Main world builder manager
 * Place this actor in your level and configure it to automatically build the Verdun battlefield
 */
UCLASS(Blueprintable, BlueprintType)
class VERDUNWORLDBUILDER_API AVerdunWorldBuilderManager : public AActor
{
	GENERATED_BODY()

public:
	AVerdunWorldBuilderManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// DATA LOADING SETTINGS
	// ========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Loading")
	FString TerrainParametersPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Loading")
	FString LocationsDatabasePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Loading")
	bool bAutoLoadOnBeginPlay;

	// ========================================
	// LOCATION SPAWNING SETTINGS
	// ========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location Spawning")
	bool bSpawnFortifications;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location Spawning")
	TSubclassOf<AActor> FortificationActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location Spawning")
	bool bSpawnDestroyedVillages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location Spawning")
	TSubclassOf<AActor> DestroyedVillageActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location Spawning")
	bool bSpawnHills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location Spawning")
	TSubclassOf<AActor> HillMarkerActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location Spawning")
	TMap<EVerdunLocationType, TSubclassOf<AActor>> LocationActorClasses;

	// ========================================
	// ARTILLERY GENERATION SETTINGS
	// ========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Generation")
	bool bGenerateArtilleryImpacts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Generation")
	TSubclassOf<UObject> ArtilleryGeneratorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Generation")
	TArray<FVerdunCraterDensityZone> CraterDensityZones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Generation")
	bool bAutoGenerateHighDensityZonesAroundForts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Generation")
	float FortHighDensityRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Generation")
	float FortCraterDensityPerHectare;

	// ========================================
	// LANDSCAPE SETTINGS
	// ========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
	ALandscape* TargetLandscape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
	bool bAutoFindLandscape;

	// ========================================
	// MATERIAL SETTINGS
	// ========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* MudMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* ChalkMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* DeadGrassMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* RockMaterial;

	// ========================================
	// RUNTIME DATA
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	FVerdunTerrainData LoadedTerrainData;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	EVerdunBuildStage CurrentBuildStage;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	TArray<AActor*> SpawnedLocationActors;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	TArray<FVerdunShellImpact> GeneratedImpacts;

	// ========================================
	// PUBLIC FUNCTIONS
	// ========================================

	/**
	 * Start the world building process
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|World Building")
	void StartWorldBuilding();

	/**
	 * Load terrain data from files
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|World Building")
	bool LoadTerrainData();

	/**
	 * Spawn all historical locations
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|World Building")
	void SpawnHistoricalLocations();

	/**
	 * Generate artillery impacts and apply to landscape
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|World Building")
	void GenerateArtilleryImpacts();

	/**
	 * Apply terrain materials based on surface composition
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|World Building")
	void ApplyTerrainMaterials();

	/**
	 * Get locations of a specific type
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|World Building")
	TArray<FVerdunLocation> GetLocationsByType(EVerdunLocationType LocationType);

	/**
	 * Find location by name
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|World Building")
	bool FindLocationByName(const FString& LocationName, FVerdunLocation& OutLocation);

	// ========================================
	// EVENTS
	// ========================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnDataLoadComplete(bool bSuccess);

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnLocationsSpawned(int32 NumLocations);

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnArtilleryGenerationComplete(int32 NumImpacts);

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnWorldBuildingComplete();

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnBuildStageChanged(EVerdunBuildStage NewStage);

private:
	void SetBuildStage(EVerdunBuildStage NewStage);
	void AutoGenerateCraterDensityZones();
	AActor* SpawnLocationActor(const FVerdunLocation& Location);
};
