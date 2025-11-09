// Verdun World Builder Plugin
// Terrain material painter implementation

#include "VerdunTerrainMaterialPainter.h"
#include "LandscapeInfo.h"
#include "LandscapeEdit.h"
#include "LandscapeDataAccess.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"

UVerdunTerrainMaterialPainter::UVerdunTerrainMaterialPainter()
{
	PrimaryComponentTick.bCanEverTick = false;

	TargetLandscape = nullptr;
	bAutoFindLandscape = true;
	bUseHistoricalPercentages = true;

	// Historical Verdun percentages (Tier 1 documented)
	MudPercentage = 70.0f;
	ChalkPercentage = 15.0f;
	GrassPercentage = 10.0f;
	RockPercentage = 5.0f;

	bAutoPaintOnBeginPlay = false;
	bIsPainted = false;
}

void UVerdunTerrainMaterialPainter::BeginPlay()
{
	Super::BeginPlay();

	// Auto-find landscape
	if (bAutoFindLandscape && !TargetLandscape)
	{
		for (TActorIterator<ALandscape> It(GetWorld()); It; ++It)
		{
			TargetLandscape = *It;
			UE_LOG(LogTemp, Log, TEXT("VerdunTerrainMaterialPainter: Auto-found landscape"));
			break;
		}
	}

	if (bAutoPaintOnBeginPlay)
	{
		if (MaterialRules.Num() == 0)
		{
			SetupDefaultVerdunRules();
		}

		PaintTerrainMaterials();
	}
}

bool UVerdunTerrainMaterialPainter::PaintTerrainMaterials()
{
	if (!TargetLandscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot paint materials: No target landscape"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("=== PAINTING TERRAIN MATERIALS ==="));
	UE_LOG(LogTemp, Log, TEXT("Target: %s"), *TargetLandscape->GetName());

	// Setup default rules if none exist
	if (MaterialRules.Num() == 0)
	{
		SetupDefaultVerdunRules();
	}

	// Paint each layer according to rules
	for (const FVerdunMaterialRule& Rule : MaterialRules)
	{
		if (!PaintLayer(Rule.Layer, Rule))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to paint layer: %s"), *UEnum::GetValueAsString(Rule.Layer));
		}
	}

	bIsPainted = true;
	OnMaterialsPainted();

	UE_LOG(LogTemp, Log, TEXT("Material painting complete!"));

	return true;
}

bool UVerdunTerrainMaterialPainter::PaintLayer(EVerdunTerrainLayer Layer, const FVerdunMaterialRule& Rule)
{
	ULandscapeLayerInfoObject* LayerInfo = GetLayerInfo(Layer);

	if (!LayerInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("No layer info for: %s"), *UEnum::GetValueAsString(Layer));
		return false;
	}

	ULandscapeInfo* LandscapeInfo = TargetLandscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot get landscape info"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Painting layer: %s (weight: %.2f)"), *UEnum::GetValueAsString(Layer), Rule.BaseWeight);

	// Get landscape bounds
	int32 MinX, MinY, MaxX, MaxY;
	LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);

	FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);

	FRandomStream Random(12345);

	// Paint each vertex
	for (int32 Y = MinY; Y <= MaxY; Y++)
	{
		for (int32 X = MinX; X <= MaxX; X++)
		{
			// Get vertex world position
			FVector VertexWorldPos = TargetLandscape->GetTransform().TransformPosition(FVector(X * 100.0f, Y * 100.0f, 0.0f));

			// Get height at this position
			uint16 Height = LandscapeEdit.GetHeight(X, Y);
			float HeightCm = Height * TargetLandscape->GetActorScale3D().Z;

			// Calculate slope (simplified)
			float Slope = 0.0f; // TODO: Calculate actual slope from neighboring vertices

			// Evaluate rule
			float Weight = EvaluateRule(Rule, VertexWorldPos, Slope, HeightCm);

			if (Weight > 0.0f)
			{
				// Set weight paint
				// NOTE: This is a simplified placeholder
				// Real implementation would use FLandscapeEditDataInterface::SetWeightData
				// LandscapeEdit.SetWeightData(LayerInfo, X, Y, (uint8)(Weight * 255.0f));
			}
		}
	}

	OnLayerPainted(Layer);

	return true;
}

void UVerdunTerrainMaterialPainter::PaintAreaAtLocation(FVector Location, float Radius, EVerdunTerrainLayer Layer, float Weight)
{
	UE_LOG(LogTemp, Log, TEXT("Painting area at %s (R:%.0f, Layer:%s, W:%.2f)"),
		*Location.ToString(), Radius, *UEnum::GetValueAsString(Layer), Weight);

	// TODO: Implement localized painting for runtime changes (e.g., fresh mud after explosion)
}

void UVerdunTerrainMaterialPainter::SetupDefaultVerdunRules()
{
	MaterialRules.Empty();

	// Rule 1: Mud (base layer - everywhere)
	FVerdunMaterialRule MudRule;
	MudRule.Layer = EVerdunTerrainLayer::Mud;
	MudRule.MinSlope = 0.0f;
	MudRule.MaxSlope = 30.0f;
	MudRule.MinElevation = -100000.0f;
	MudRule.MaxElevation = 100000.0f;
	MudRule.BaseWeight = MudPercentage / 100.0f;
	MudRule.bUseNoise = true;
	MudRule.NoiseScale = 500.0f;
	MudRule.NoiseStrength = 0.2f;
	MaterialRules.Add(MudRule);

	// Rule 2: Chalk (on slopes and crater walls)
	FVerdunMaterialRule ChalkRule;
	ChalkRule.Layer = EVerdunTerrainLayer::Chalk;
	ChalkRule.MinSlope = 25.0f;
	ChalkRule.MaxSlope = 90.0f;
	ChalkRule.MinElevation = 30000.0f; // Above 300m
	ChalkRule.MaxElevation = 100000.0f;
	ChalkRule.BaseWeight = ChalkPercentage / 100.0f;
	ChalkRule.bUseNoise = true;
	ChalkRule.NoiseScale = 200.0f;
	ChalkRule.NoiseStrength = 0.3f;
	MaterialRules.Add(ChalkRule);

	// Rule 3: Dead grass (rare, only on high untouched areas)
	FVerdunMaterialRule GrassRule;
	GrassRule.Layer = EVerdunTerrainLayer::DeadGrass;
	GrassRule.MinSlope = 0.0f;
	GrassRule.MaxSlope = 15.0f;
	GrassRule.MinElevation = 35000.0f; // Above 350m (high ground)
	GrassRule.MaxElevation = 100000.0f;
	GrassRule.BaseWeight = GrassPercentage / 100.0f;
	GrassRule.bUseNoise = true;
	GrassRule.NoiseScale = 100.0f;
	GrassRule.NoiseStrength = 0.4f;
	MaterialRules.Add(GrassRule);

	// Rule 4: Rock (very steep slopes and hilltops)
	FVerdunMaterialRule RockRule;
	RockRule.Layer = EVerdunTerrainLayer::Rock;
	RockRule.MinSlope = 40.0f;
	RockRule.MaxSlope = 90.0f;
	RockRule.MinElevation = -100000.0f;
	RockRule.MaxElevation = 100000.0f;
	RockRule.BaseWeight = RockPercentage / 100.0f;
	RockRule.bUseNoise = true;
	RockRule.NoiseScale = 300.0f;
	RockRule.NoiseStrength = 0.3f;
	MaterialRules.Add(RockRule);

	UE_LOG(LogTemp, Log, TEXT("Setup %d default Verdun material rules"), MaterialRules.Num());
	UE_LOG(LogTemp, Log, TEXT("  Mud: %.0f%%, Chalk: %.0f%%, Grass: %.0f%%, Rock: %.0f%%"),
		MudPercentage, ChalkPercentage, GrassPercentage, RockPercentage);
}

void UVerdunTerrainMaterialPainter::ClearAllLayers()
{
	UE_LOG(LogTemp, Warning, TEXT("ClearAllLayers not yet implemented"));
}

float UVerdunTerrainMaterialPainter::GetLayerWeightAtLocation(FVector Location, EVerdunTerrainLayer Layer)
{
	// TODO: Query landscape layer weight at location
	return 0.0f;
}

ULandscapeLayerInfoObject* UVerdunTerrainMaterialPainter::GetLayerInfo(EVerdunTerrainLayer Layer)
{
	switch (Layer)
	{
		case EVerdunTerrainLayer::Mud:
			return MudLayerInfo;

		case EVerdunTerrainLayer::Chalk:
			return ChalkLayerInfo;

		case EVerdunTerrainLayer::DeadGrass:
			return DeadGrassLayerInfo;

		case EVerdunTerrainLayer::Rock:
			return RockLayerInfo;

		default:
			return nullptr;
	}
}

float UVerdunTerrainMaterialPainter::EvaluateRule(const FVerdunMaterialRule& Rule, FVector Location, float Slope, float Elevation)
{
	// Check slope range
	if (Slope < Rule.MinSlope || Slope > Rule.MaxSlope)
	{
		return 0.0f;
	}

	// Check elevation range
	if (Elevation < Rule.MinElevation || Elevation > Rule.MaxElevation)
	{
		return 0.0f;
	}

	float Weight = Rule.BaseWeight;

	// Add noise if enabled
	if (Rule.bUseNoise)
	{
		// Simple noise (would use proper Perlin noise in production)
		float NoiseValue = FMath::Sin(Location.X / Rule.NoiseScale) * FMath::Cos(Location.Y / Rule.NoiseScale);
		Weight += NoiseValue * Rule.NoiseStrength;
	}

	return FMath::Clamp(Weight, 0.0f, 1.0f);
}

void UVerdunTerrainMaterialPainter::CreateDefaultLayerInfos()
{
	// NOTE: Layer info objects must be created in the editor
	// This is a placeholder for runtime
	UE_LOG(LogTemp, Warning, TEXT("Layer info objects must be created in editor"));
}
