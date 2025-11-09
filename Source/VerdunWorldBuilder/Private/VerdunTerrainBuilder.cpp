// Verdun World Builder Plugin
// Main terrain builder implementation

#include "VerdunTerrainBuilder.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AVerdunTerrainBuilder::AVerdunTerrainBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	SetupDefaultSettings();

	CurrentBuildStage = EVerdunTerrainBuildStage::NotStarted;
}

void AVerdunTerrainBuilder::SetupDefaultSettings()
{
	// Default Verdun settings
	HeightmapPath = TEXT(""); // Will use procedural if empty
	bUseProcedural = true;

	// Landscape settings for ~60 km² Verdun battlefield
	// Each vertex = 30m (SRTM resolution)
	LandscapeQuads = 511; // 512 verts per section
	SectionsPerComponent = 1;
	ComponentCountX = 8;
	ComponentCountY = 8;
	TerrainScale = 3000.0f; // 30m per vertex = 3000cm

	// Verdun elevations (historical data)
	MinElevation = 165.0f;
	MaxElevation = 390.0f;

	// Material settings
	bApplyMaterials = true;
	bUseHistoricalPercentages = true;

	// Deformation settings
	bEnableDeformation = true;
	bBatchDeformations = true;
	MaxDeformationsPerFrame = 10;
	bUpdateCollision = true;

	// Build settings
	bAutoBuildOnBeginPlay = false;
	LandscapeSpawnLocation = FVector::ZeroVector;
}

void AVerdunTerrainBuilder::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoBuildOnBeginPlay)
	{
		BuildTerrain();
	}
}

void AVerdunTerrainBuilder::BuildTerrain()
{
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("  VERDUN TERRAIN BUILDER"));
	UE_LOG(LogTemp, Log, TEXT("  Building Destructible Battlefield"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT(""));

	OnTerrainBuildStarted();

	// Step 1: Generate landscape
	SetBuildStage(EVerdunTerrainBuildStage::GeneratingLandscape);
	if (!GenerateLandscape())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to generate landscape!"));
		SetBuildStage(EVerdunTerrainBuildStage::Failed);
		return;
	}

	// Step 2: Apply materials
	if (bApplyMaterials)
	{
		SetBuildStage(EVerdunTerrainBuildStage::ApplyingMaterials);
		if (!ApplyMaterials())
		{
			UE_LOG(LogTemp, Warning, TEXT("Material application had issues (non-critical)"));
		}
	}

	// Step 3: Setup deformation system
	if (bEnableDeformation)
	{
		SetBuildStage(EVerdunTerrainBuildStage::SettingUpDeformation);
		if (!SetupDeformation())
		{
			UE_LOG(LogTemp, Warning, TEXT("Deformation setup had issues (non-critical)"));
		}
	}

	// Done!
	SetBuildStage(EVerdunTerrainBuildStage::Complete);
	OnTerrainBuildComplete();

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("  TERRAIN BUILD COMPLETE!"));
	UE_LOG(LogTemp, Log, TEXT("  Ready for artillery mechanics"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("NEXT STEPS:"));
	UE_LOG(LogTemp, Log, TEXT("  1. Terrain is now destructible/deformable"));
	UE_LOG(LogTemp, Log, TEXT("  2. Use GetDeformationComponent() for artillery"));
	UE_LOG(LogTemp, Log, TEXT("  3. Test with TestCrater() or TestTrench()"));
	UE_LOG(LogTemp, Log, TEXT(""));
}

bool AVerdunTerrainBuilder::GenerateLandscape()
{
	UE_LOG(LogTemp, Log, TEXT("STEP 1: Generating landscape..."));

	// Spawn landscape generator
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(TEXT("VerdunLandscapeGenerator_Temp"));

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(LandscapeSpawnLocation);

	AVerdunLandscapeGenerator* Generator = GetWorld()->SpawnActor<AVerdunLandscapeGenerator>(
		AVerdunLandscapeGenerator::StaticClass(),
		SpawnTransform,
		SpawnParams
	);

	if (!Generator)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn landscape generator"));
		return false;
	}

	// Configure generator
	Generator->HeightmapFilePath = HeightmapPath;
	Generator->bUseProcedural = bUseProcedural;
	Generator->LandscapeQuadsPerSection = LandscapeQuads;
	Generator->LandscapeSectionsPerComponent = SectionsPerComponent;
	Generator->LandscapeComponentsX = ComponentCountX;
	Generator->LandscapeComponentsY = ComponentCountY;
	Generator->LandscapeScale = FVector(TerrainScale, TerrainScale, 100.0f);
	Generator->MinElevationMeters = MinElevation;
	Generator->MaxElevationMeters = MaxElevation;

	// Generate!
	bool bSuccess = Generator->GenerateLandscape(!bUseProcedural);

	if (bSuccess)
	{
		GeneratedLandscape = Generator->GeneratedLandscape;
		OnLandscapeGenerated(GeneratedLandscape);

		UE_LOG(LogTemp, Log, TEXT("✓ Landscape generated successfully"));
		UE_LOG(LogTemp, Log, TEXT("  Size: %d x %d components"), ComponentCountX, ComponentCountY);
		UE_LOG(LogTemp, Log, TEXT("  Scale: %.0f cm/vertex (%.0fm)"), TerrainScale, TerrainScale / 100.0f);
		UE_LOG(LogTemp, Log, TEXT("  Elevation: %.0f - %.0fm"), MinElevation, MaxElevation);

		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("✗ Landscape generation failed"));
	return false;
}

bool AVerdunTerrainBuilder::ApplyMaterials()
{
	if (!GeneratedLandscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot apply materials: No landscape"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("STEP 2: Applying terrain materials..."));

	// Create material painter component
	MaterialPainter = NewObject<UVerdunTerrainMaterialPainter>(this, UVerdunTerrainMaterialPainter::StaticClass());
	if (!MaterialPainter)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create material painter"));
		return false;
	}

	MaterialPainter->RegisterComponent();

	// Configure painter
	MaterialPainter->TargetLandscape = GeneratedLandscape;
	MaterialPainter->bUseHistoricalPercentages = bUseHistoricalPercentages;
	MaterialPainter->MudLayerInfo = MudLayerInfo;
	MaterialPainter->ChalkLayerInfo = ChalkLayerInfo;
	MaterialPainter->DeadGrassLayerInfo = GrassLayerInfo;
	MaterialPainter->RockLayerInfo = RockLayerInfo;

	// Setup default Verdun rules
	MaterialPainter->SetupDefaultVerdunRules();

	// Paint materials
	bool bSuccess = MaterialPainter->PaintTerrainMaterials();

	if (bSuccess)
	{
		OnMaterialsApplied();
		UE_LOG(LogTemp, Log, TEXT("✓ Materials applied (70%% mud, 15%% chalk, 10%% grass, 5%% rock)"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠ Material application incomplete (may need layer info objects)"));
	}

	return true; // Return true even if incomplete - not critical
}

bool AVerdunTerrainBuilder::SetupDeformation()
{
	if (!GeneratedLandscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot setup deformation: No landscape"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("STEP 3: Setting up deformation system..."));

	// Create deformation component
	DeformationComponent = NewObject<UVerdunTerrainDeformation>(this, UVerdunTerrainDeformation::StaticClass());
	if (!DeformationComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create deformation component"));
		return false;
	}

	DeformationComponent->RegisterComponent();

	// Configure deformation
	DeformationComponent->TargetLandscape = GeneratedLandscape;
	DeformationComponent->bEnableRuntimeDeformation = bEnableDeformation;
	DeformationComponent->bBatchDeformations = bBatchDeformations;
	DeformationComponent->MaxDeformationsPerFrame = MaxDeformationsPerFrame;
	DeformationComponent->bUpdateCollision = bUpdateCollision;

	OnDeformationSetup();

	UE_LOG(LogTemp, Log, TEXT("✓ Deformation system ready"));
	UE_LOG(LogTemp, Log, TEXT("  Runtime deformation: ENABLED"));
	UE_LOG(LogTemp, Log, TEXT("  Batching: %s"), bBatchDeformations ? TEXT("ON") : TEXT("OFF"));
	UE_LOG(LogTemp, Log, TEXT("  Max deformations/frame: %d"), MaxDeformationsPerFrame);

	return true;
}

void AVerdunTerrainBuilder::TestCrater(FVector Location, float Radius, float Depth)
{
	if (!DeformationComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot test crater: Deformation not setup"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("🎯 TEST: Creating crater at %s"), *Location.ToString());
	UE_LOG(LogTemp, Log, TEXT("  Radius: %.0fcm (%.1fm)"), Radius, Radius / 100.0f);
	UE_LOG(LogTemp, Log, TEXT("  Depth: %.0fcm (%.1fm)"), Depth, Depth / 100.0f);

	DeformationComponent->DeformCrater(Location, Radius, Depth, EVerdunDeformationFalloff::Smooth, true);

	UE_LOG(LogTemp, Log, TEXT("✓ Crater created!"));
	UE_LOG(LogTemp, Log, TEXT(""));
}

void AVerdunTerrainBuilder::TestTrench(FVector Start, FVector End, float Width, float Depth)
{
	if (!DeformationComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot test trench: Deformation not setup"));
		return;
	}

	float Length = FVector::Dist(Start, End);

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("🎯 TEST: Creating trench"));
	UE_LOG(LogTemp, Log, TEXT("  Start: %s"), *Start.ToString());
	UE_LOG(LogTemp, Log, TEXT("  End: %s"), *End.ToString());
	UE_LOG(LogTemp, Log, TEXT("  Length: %.0fcm (%.1fm)"), Length, Length / 100.0f);
	UE_LOG(LogTemp, Log, TEXT("  Width: %.0fcm (%.1fm)"), Width, Width / 100.0f);
	UE_LOG(LogTemp, Log, TEXT("  Depth: %.0fcm (%.1fm)"), Depth, Depth / 100.0f);

	DeformationComponent->DeformTrench(Start, End, Width, Depth, false);

	UE_LOG(LogTemp, Log, TEXT("✓ Trench created!"));
	UE_LOG(LogTemp, Log, TEXT(""));
}

bool AVerdunTerrainBuilder::GetTerrainHeight(FVector Location, float& OutHeight)
{
	if (DeformationComponent)
	{
		return DeformationComponent->GetTerrainHeightAtLocation(Location, OutHeight);
	}

	return false;
}

void AVerdunTerrainBuilder::SetBuildStage(EVerdunTerrainBuildStage NewStage)
{
	CurrentBuildStage = NewStage;
	OnBuildStageChanged(NewStage);
}
