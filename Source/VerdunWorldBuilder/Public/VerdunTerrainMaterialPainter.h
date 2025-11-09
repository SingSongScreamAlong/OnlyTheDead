// Verdun World Builder Plugin
// Terrain material painting - mud, chalk, grass, rock blending

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Landscape.h"
#include "VerdunTerrainMaterialPainter.generated.h"

/**
 * Verdun terrain material layers
 */
UENUM(BlueprintType)
enum class EVerdunTerrainLayer : uint8
{
	Mud UMETA(DisplayName = "Mud (70% - churned, wet)"),
	Chalk UMETA(DisplayName = "Chalk (15% - white, exposed on slopes)"),
	DeadGrass UMETA(DisplayName = "Dead Grass (10% - sparse, brown)"),
	Rock UMETA(DisplayName = "Rock (5% - limestone, broken)")
};

/**
 * Material painting rules
 */
USTRUCT(BlueprintType)
struct FVerdunMaterialRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule")
	EVerdunTerrainLayer Layer;

	/** Paint on slopes steeper than this angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule", meta = (ClampMin = "0", ClampMax = "90"))
	float MinSlope;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule", meta = (ClampMin = "0", ClampMax = "90"))
	float MaxSlope;

	/** Paint at elevations within this range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule")
	float MinElevation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule")
	float MaxElevation;

	/** Base weight for this layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseWeight;

	/** Add noise variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule")
	bool bUseNoise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule")
	float NoiseScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Rule")
	float NoiseStrength;

	FVerdunMaterialRule()
		: Layer(EVerdunTerrainLayer::Mud)
		, MinSlope(0.0f)
		, MaxSlope(90.0f)
		, MinElevation(-100000.0f)
		, MaxElevation(100000.0f)
		, BaseWeight(1.0f)
		, bUseNoise(true)
		, NoiseScale(100.0f)
		, NoiseStrength(0.2f)
	{}
};

/**
 * Terrain material painter component
 * Paints landscape layers based on historical terrain composition
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VERDUNWORLDBUILDER_API UVerdunTerrainMaterialPainter : public UActorComponent
{
	GENERATED_BODY()

public:
	UVerdunTerrainMaterialPainter();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// SETTINGS
	// ========================================

	/** Target landscape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	ALandscape* TargetLandscape;

	/** Auto-find landscape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	bool bAutoFindLandscape;

	/** Material layer info assets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	ULandscapeLayerInfoObject* MudLayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	ULandscapeLayerInfoObject* ChalkLayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	ULandscapeLayerInfoObject* DeadGrassLayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	ULandscapeLayerInfoObject* RockLayerInfo;

	/** Painting rules for each layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	TArray<FVerdunMaterialRule> MaterialRules;

	/** Use historical percentages as base */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	bool bUseHistoricalPercentages;

	/** Historical composition (Tier 1 documented) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings", meta = (EditCondition = "bUseHistoricalPercentages"))
	float MudPercentage; // 70%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings", meta = (EditCondition = "bUseHistoricalPercentages"))
	float ChalkPercentage; // 15%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings", meta = (EditCondition = "bUseHistoricalPercentages"))
	float GrassPercentage; // 10%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings", meta = (EditCondition = "bUseHistoricalPercentages"))
	float RockPercentage; // 5%

	/** Auto-paint on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Settings")
	bool bAutoPaintOnBeginPlay;

	// ========================================
	// RUNTIME DATA
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
	bool bIsPainted;

	// ========================================
	// PUBLIC FUNCTIONS
	// ========================================

	/**
	 * Paint all terrain materials based on rules
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Material Painting")
	bool PaintTerrainMaterials();

	/**
	 * Paint specific layer
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Material Painting")
	bool PaintLayer(EVerdunTerrainLayer Layer, const FVerdunMaterialRule& Rule);

	/**
	 * Paint area around location (for dynamic changes)
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Material Painting")
	void PaintAreaAtLocation(FVector Location, float Radius, EVerdunTerrainLayer Layer, float Weight);

	/**
	 * Setup default Verdun material rules
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Material Painting")
	void SetupDefaultVerdunRules();

	/**
	 * Clear all layer painting
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Material Painting")
	void ClearAllLayers();

	/**
	 * Get layer weight at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Material Painting")
	float GetLayerWeightAtLocation(FVector Location, EVerdunTerrainLayer Layer);

	// ========================================
	// EVENTS
	// ========================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnMaterialsPainted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Verdun|Events")
	void OnLayerPainted(EVerdunTerrainLayer Layer);

private:
	ULandscapeLayerInfoObject* GetLayerInfo(EVerdunTerrainLayer Layer);
	float EvaluateRule(const FVerdunMaterialRule& Rule, FVector Location, float Slope, float Elevation);
	void CreateDefaultLayerInfos();
};
