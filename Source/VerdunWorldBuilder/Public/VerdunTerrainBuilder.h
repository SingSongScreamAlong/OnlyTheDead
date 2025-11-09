// Verdun World Builder Plugin
// Main terrain builder - orchestrates generation, deformation, and materials

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VerdunLandscapeGenerator.h"
#include "VerdunTerrainDeformation.h"
#include "VerdunTerrainMaterialPainter.h"
#include "VerdunTerrainData.h"
#include "VerdunTerrainBuilder.generated.h"

/**
 * Build stages for terrain construction
 */
UENUM(BlueprintType)
enum class EVerdunTerrainBuildStage : uint8
{
	NotStarted UMETA(DisplayName = "Not Started"),
	GeneratingLandscape UMETA(DisplayName = "Generating Landscape"),
	ApplyingMaterials UMETA(DisplayName = "Applying Materials"),
	SettingUpDeformation UMETA(DisplayName = "Setting Up Deformation"),
	Complete UMETA(DisplayName = "Complete"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * Main terrain builder actor
 * ONE-STOP SHOP: Place this in your level, configure, and click "Build Terrain"
 * Creates destructible/deformable Verdun battlefield ready for artillery
 */
UCLASS(Blueprintable, BlueprintType)
class VERDUNWORLDBUILDER_API AVerdunTerrainBuilder : public AActor
{
	GENERATED_BODY()

public:
	AVerdunTerrainBuilder();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// TERRAIN GENERATION SETTINGS
	// ========================================

	/** Path to heightmap file (16-bit PNG) - leave empty for procedural */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	FString HeightmapPath;

	/** Use procedural generation instead of heightmap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	bool bUseProcedural;

	/** Landscape size (127, 255, 511, 1023, 2047) - larger = more detail */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	int32 LandscapeQuads;

	/** Sections per component (1, 2, 4, 8, 16) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	int32 SectionsPerComponent;

	/** Number of components X */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	int32 ComponentCountX;

	/** Number of components Y */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	int32 ComponentCountY;

	/** Scale: 100 = 1m per vertex, 200 = 2m per vertex */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	float TerrainScale;

	/** Min elevation from data (165m for Verdun) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	float MinElevation;

	/** Max elevation from data (390m for Verdun) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Terrain Generation")
	float MaxElevation;

	// ========================================
	// MATERIAL SETTINGS
	// ========================================

	/** Apply terrain materials automatically */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Materials")
	bool bApplyMaterials;

	/** Use historical Verdun percentages (70% mud, 15% chalk, 10% grass, 5% rock) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Materials")
	bool bUseHistoricalPercentages;

	/** Landscape material (needs 4 layers: Mud, Chalk, DeadGrass, Rock) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Materials")
	UMaterialInterface* LandscapeMaterial;

	/** Layer info objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Materials")
	ULandscapeLayerInfoObject* MudLayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Materials")
	ULandscapeLayerInfoObject* ChalkLayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Materials")
	ULandscapeLayerInfoObject* GrassLayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Materials")
	ULandscapeLayerInfoObject* RockLayerInfo;

	// ========================================
	// DEFORMATION SETTINGS
	// ========================================

	/** Enable runtime deformation (destructible terrain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Deformation")
	bool bEnableDeformation;

	/** Batch deformations for performance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Deformation")
	bool bBatchDeformations;

	/** Max deformations per frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Deformation")
	int32 MaxDeformationsPerFrame;

	/** Update collision after deformation (performance impact) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Deformation")
	bool bUpdateCollision;

	// ========================================
	// BUILD SETTINGS
	// ========================================

	/** Auto-build terrain on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Build Settings")
	bool bAutoBuildOnBeginPlay;

	/** Location to spawn landscape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Build Settings")
	FVector LandscapeSpawnLocation;

	// ========================================
	// RUNTIME DATA
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	ALandscape* GeneratedLandscape;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	UVerdunTerrainDeformation* DeformationComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	UVerdunTerrainMaterialPainter* MaterialPainter;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	EVerdunTerrainBuildStage CurrentBuildStage;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	FVerdunTerrainData LoadedTerrainData;

	// ========================================
	// PUBLIC FUNCTIONS
	// ========================================

	/**
	 * BUILD THE TERRAIN!
	 * This is the main function - call this to generate everything
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Building", meta = (DisplayName = "BUILD TERRAIN"))
	void BuildTerrain();

	/**
	 * Step 1: Generate landscape mesh
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Building")
	bool GenerateLandscape();

	/**
	 * Step 2: Apply materials (mud, chalk, grass, rock)
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Building")
	bool ApplyMaterials();

	/**
	 * Step 3: Setup deformation system
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Building")
	bool SetupDeformation();

	/**
	 * Test deformation with a crater
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Building")
	void TestCrater(FVector Location, float Radius = 600.0f, float Depth = 150.0f);

	/**
	 * Test deformation with a trench
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Building")
	void TestTrench(FVector Start, FVector End, float Width = 150.0f, float Depth = 200.0f);

	/**
	 * Get terrain deformation component (for artillery system to use later)
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Building")
	UVerdunTerrainDeformation* GetDeformationComponent() const { return DeformationComponent; }

	/**
	 * Get height at world position
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Terrain Building")
	bool GetTerrainHeight(FVector Location, float& OutHeight);

	// ========================================
	// EVENTS
	// ========================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnTerrainBuildStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnLandscapeGenerated(ALandscape* Landscape);

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnMaterialsApplied();

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnDeformationSetup();

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnTerrainBuildComplete();

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnBuildStageChanged(EVerdunTerrainBuildStage NewStage);

private:
	void SetBuildStage(EVerdunTerrainBuildStage NewStage);
	void SetupDefaultSettings();
};
