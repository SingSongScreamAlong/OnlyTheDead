// Verdun World Builder Plugin
// Landscape generator - creates actual terrain from elevation data

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Landscape.h"
#include "VerdunLandscapeGenerator.generated.h"

/**
 * Generates Verdun landscape from heightmap or procedural data
 * Creates the actual terrain mesh that will be destructible
 */
UCLASS(Blueprintable, BlueprintType)
class VERDUNWORLDBUILDER_API AVerdunLandscapeGenerator : public AActor
{
	GENERATED_BODY()

public:
	AVerdunLandscapeGenerator();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// LANDSCAPE GENERATION SETTINGS
	// ========================================

	/** Import heightmap from file (PNG 16-bit recommended) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	FString HeightmapFilePath;

	/** Use procedural generation instead of heightmap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	bool bUseProcedural;

	/** Landscape size in quads (power of 2 minus 1: 127, 255, 511, 1023, 2047) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation", meta = (ClampMin = "127", ClampMax = "8129"))
	int32 LandscapeQuadsPerSection;

	/** Number of sections (1, 2, 4, 8, 16, 32) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	int32 LandscapeSectionsPerComponent;

	/** Number of components (1x1, 2x2, 4x4, 8x8) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	int32 LandscapeComponentsX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	int32 LandscapeComponentsY;

	/** Scale in X/Y direction (cm) - default 100 means 1m per vertex */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	FVector LandscapeScale;

	/** Minimum elevation in meters (will be Z=0 in UE5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	float MinElevationMeters;

	/** Maximum elevation in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	float MaxElevationMeters;

	/** Auto-generate on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
	bool bAutoGenerateOnBeginPlay;

	// ========================================
	// PROCEDURAL SETTINGS (if not using heightmap)
	// ========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings")
	float ProceduralNoiseScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings")
	float ProceduralNoiseStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings")
	int32 ProceduralSeed;

	// ========================================
	// RUNTIME DATA
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	ALandscape* GeneratedLandscape;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	TArray<uint16> HeightData;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	int32 HeightmapSizeX;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	int32 HeightmapSizeY;

	// ========================================
	// PUBLIC FUNCTIONS
	// ========================================

	/**
	 * Generate the landscape
	 * @param bFromHeightmap If true, loads from file. If false, uses procedural
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Landscape Generation")
	bool GenerateLandscape(bool bFromHeightmap = true);

	/**
	 * Load heightmap from PNG file
	 * @param FilePath Path to 16-bit PNG heightmap
	 * @param OutHeightData Output height data (0-65535)
	 * @param OutSizeX Output width
	 * @param OutSizeY Output height
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Landscape Generation")
	static bool LoadHeightmapFromFile(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutSizeX, int32& OutSizeY);

	/**
	 * Generate procedural heightmap using noise
	 * @param SizeX Width in vertices
	 * @param SizeY Height in vertices
	 * @param OutHeightData Output height data
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Landscape Generation")
	void GenerateProceduralHeightmap(int32 SizeX, int32 SizeY, TArray<uint16>& OutHeightData);

	/**
	 * Create landscape actor from height data
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Landscape Generation")
	ALandscape* CreateLandscapeFromHeightData(const TArray<uint16>& HeightDataArray, int32 SizeX, int32 SizeY);

	/**
	 * Get height value at world position
	 * @param WorldPosition Position to query
	 * @param OutHeight Output height in cm
	 * @return True if valid position
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Landscape Generation")
	bool GetHeightAtPosition(FVector WorldPosition, float& OutHeight);

	// ========================================
	// EVENTS
	// ========================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnLandscapeGenerated(ALandscape* Landscape);

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnLandscapeGenerationFailed(const FString& Reason);

private:
	void SetupLandscapeDefaults();
};
