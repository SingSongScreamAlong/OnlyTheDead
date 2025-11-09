// Verdun World Builder Plugin
// World builder manager implementation

#include "VerdunWorldBuilderManager.h"
#include "VerdunTerrainDataLoader.h"
#include "Landscape.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AVerdunWorldBuilderManager::AVerdunWorldBuilderManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default paths (relative to Content folder)
	TerrainParametersPath = FPaths::ProjectContentDir() + TEXT("VerdunData/terrain_parameters.json");
	LocationsDatabasePath = FPaths::ProjectContentDir() + TEXT("VerdunData/locations_database.csv");

	// Default settings
	bAutoLoadOnBeginPlay = false;
	bSpawnFortifications = true;
	bSpawnDestroyedVillages = true;
	bSpawnHills = true;
	bGenerateArtilleryImpacts = true;
	bAutoGenerateHighDensityZonesAroundForts = true;

	// Artillery settings
	FortHighDensityRadius = 200000.0f; // 2km in cm
	FortCraterDensityPerHectare = 75.0f; // High intensity

	// Landscape settings
	bAutoFindLandscape = true;

	CurrentBuildStage = EVerdunBuildStage::NotStarted;
}

void AVerdunWorldBuilderManager::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoLoadOnBeginPlay)
	{
		StartWorldBuilding();
	}
}

void AVerdunWorldBuilderManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVerdunWorldBuilderManager::StartWorldBuilding()
{
	UE_LOG(LogTemp, Log, TEXT("=== VERDUN WORLD BUILDING STARTED ==="));

	SetBuildStage(EVerdunBuildStage::LoadingData);

	if (!LoadTerrainData())
	{
		SetBuildStage(EVerdunBuildStage::Failed);
		return;
	}

	OnDataLoadComplete(true);

	// Auto-find landscape if needed
	if (bAutoFindLandscape && !TargetLandscape)
	{
		for (TActorIterator<ALandscape> It(GetWorld()); It; ++It)
		{
			TargetLandscape = *It;
			UE_LOG(LogTemp, Log, TEXT("Auto-found landscape: %s"), *TargetLandscape->GetName());
			break;
		}
	}

	// Spawn locations
	SetBuildStage(EVerdunBuildStage::SpawningLocations);
	SpawnHistoricalLocations();
	OnLocationsSpawned(SpawnedLocationActors.Num());

	// Generate artillery impacts
	if (bGenerateArtilleryImpacts)
	{
		SetBuildStage(EVerdunBuildStage::GeneratingArtillery);
		GenerateArtilleryImpacts();
		OnArtilleryGenerationComplete(GeneratedImpacts.Num());
	}

	// Apply materials
	SetBuildStage(EVerdunBuildStage::ApplyingMaterials);
	ApplyTerrainMaterials();

	SetBuildStage(EVerdunBuildStage::Complete);
	OnWorldBuildingComplete();

	UE_LOG(LogTemp, Log, TEXT("=== VERDUN WORLD BUILDING COMPLETE ==="));
}

bool AVerdunWorldBuilderManager::LoadTerrainData()
{
	UE_LOG(LogTemp, Log, TEXT("Loading terrain data..."));
	UE_LOG(LogTemp, Log, TEXT("  Terrain params: %s"), *TerrainParametersPath);
	UE_LOG(LogTemp, Log, TEXT("  Locations DB: %s"), *LocationsDatabasePath);

	bool bSuccess = UVerdunTerrainDataLoader::LoadCompleteTerrainData(
		TerrainParametersPath,
		LocationsDatabasePath,
		LoadedTerrainData
	);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Terrain data loaded successfully:"));
		UE_LOG(LogTemp, Log, TEXT("  Name: %s"), *LoadedTerrainData.TerrainName);
		UE_LOG(LogTemp, Log, TEXT("  Locations: %d"), LoadedTerrainData.Locations.Num());
		UE_LOG(LogTemp, Log, TEXT("  Center: %.4f°N, %.4f°E"),
			LoadedTerrainData.BattlefieldCenter.Latitude,
			LoadedTerrainData.BattlefieldCenter.Longitude);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load terrain data!"));
		UE_LOG(LogTemp, Error, TEXT("Make sure files exist at specified paths"));
	}

	return bSuccess;
}

void AVerdunWorldBuilderManager::SpawnHistoricalLocations()
{
	UE_LOG(LogTemp, Log, TEXT("Spawning historical locations..."));

	SpawnedLocationActors.Empty();

	for (const FVerdunLocation& Location : LoadedTerrainData.Locations)
	{
		// Check if we should spawn this type
		bool bShouldSpawn = false;

		switch (Location.Type)
		{
			case EVerdunLocationType::Fortification:
				bShouldSpawn = bSpawnFortifications;
				break;
			case EVerdunLocationType::DestroyedVillage:
				bShouldSpawn = bSpawnDestroyedVillages;
				break;
			case EVerdunLocationType::Hill:
				bShouldSpawn = bSpawnHills;
				break;
			default:
				bShouldSpawn = LocationActorClasses.Contains(Location.Type);
				break;
		}

		if (bShouldSpawn)
		{
			AActor* SpawnedActor = SpawnLocationActor(Location);
			if (SpawnedActor)
			{
				SpawnedLocationActors.Add(SpawnedActor);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d location actors"), SpawnedLocationActors.Num());
}

void AVerdunWorldBuilderManager::GenerateArtilleryImpacts()
{
	UE_LOG(LogTemp, Log, TEXT("Generating artillery impacts..."));

	if (!ArtilleryGeneratorClass)
	{
		// Use default generator
		ArtilleryGeneratorClass = UVerdunDefaultArtilleryGenerator::StaticClass();
	}

	UObject* GeneratorObject = NewObject<UObject>(this, ArtilleryGeneratorClass);
	IVerdunArtilleryImpactGenerator* Generator = Cast<IVerdunArtilleryImpactGenerator>(GeneratorObject);

	if (!Generator)
	{
		UE_LOG(LogTemp, Error, TEXT("Artillery generator class does not implement IVerdunArtilleryImpactGenerator"));
		return;
	}

	// Auto-generate crater density zones around forts if enabled
	if (bAutoGenerateHighDensityZonesAroundForts && CraterDensityZones.Num() == 0)
	{
		AutoGenerateCraterDensityZones();
	}

	GeneratedImpacts.Empty();

	// Generate impacts for each density zone
	for (const FVerdunCraterDensityZone& Zone : CraterDensityZones)
	{
		TArray<FVerdunShellImpact> ZoneImpacts;
		Generator->Execute_GenerateShellImpacts(
			GeneratorObject,
			Zone.ZoneCenter,
			Zone.ZoneRadius,
			Zone.CratersPerHectare,
			ZoneImpacts
		);

		GeneratedImpacts.Append(ZoneImpacts);

		UE_LOG(LogTemp, Log, TEXT("  Zone '%s': %d impacts"), *Zone.IntensityLevel, ZoneImpacts.Num());
	}

	// Apply impacts to landscape (if available)
	if (TargetLandscape)
	{
		for (const FVerdunShellImpact& Impact : GeneratedImpacts)
		{
			Generator->Execute_ApplyCraterToLandscape(GeneratorObject, Impact, TargetLandscape);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated total of %d artillery impacts"), GeneratedImpacts.Num());
}

void AVerdunWorldBuilderManager::ApplyTerrainMaterials()
{
	UE_LOG(LogTemp, Log, TEXT("Applying terrain materials..."));
	UE_LOG(LogTemp, Log, TEXT("  Composition: %.0f%% mud, %.0f%% chalk, %.0f%% grass, %.0f%% rock"),
		LoadedTerrainData.SurfaceMaterials.MudPercentage,
		LoadedTerrainData.SurfaceMaterials.ChalkPercentage,
		LoadedTerrainData.SurfaceMaterials.DeadGrassPercentage,
		LoadedTerrainData.SurfaceMaterials.RockPercentage);

	// TODO: Implement material application to landscape
	// This would typically involve:
	// 1. Creating or updating landscape layer info
	// 2. Painting layers based on terrain rules (mud in low areas, chalk on slopes, etc.)
	// 3. Using the percentage data to control blend weights

	UE_LOG(LogTemp, Warning, TEXT("Material application not yet implemented - set materials manually"));
}

TArray<FVerdunLocation> AVerdunWorldBuilderManager::GetLocationsByType(EVerdunLocationType LocationType)
{
	TArray<FVerdunLocation> Result;

	for (const FVerdunLocation& Location : LoadedTerrainData.Locations)
	{
		if (Location.Type == LocationType)
		{
			Result.Add(Location);
		}
	}

	return Result;
}

bool AVerdunWorldBuilderManager::FindLocationByName(const FString& LocationName, FVerdunLocation& OutLocation)
{
	for (const FVerdunLocation& Location : LoadedTerrainData.Locations)
	{
		if (Location.Name.Equals(LocationName, ESearchCase::IgnoreCase))
		{
			OutLocation = Location;
			return true;
		}
	}

	return false;
}

void AVerdunWorldBuilderManager::SetBuildStage(EVerdunBuildStage NewStage)
{
	CurrentBuildStage = NewStage;
	OnBuildStageChanged(NewStage);

	FString StageName;
	switch (NewStage)
	{
		case EVerdunBuildStage::NotStarted: StageName = TEXT("Not Started"); break;
		case EVerdunBuildStage::LoadingData: StageName = TEXT("Loading Data"); break;
		case EVerdunBuildStage::SpawningLocations: StageName = TEXT("Spawning Locations"); break;
		case EVerdunBuildStage::GeneratingArtillery: StageName = TEXT("Generating Artillery"); break;
		case EVerdunBuildStage::ApplyingMaterials: StageName = TEXT("Applying Materials"); break;
		case EVerdunBuildStage::Complete: StageName = TEXT("Complete"); break;
		case EVerdunBuildStage::Failed: StageName = TEXT("Failed"); break;
	}

	UE_LOG(LogTemp, Log, TEXT("Build stage: %s"), *StageName);
}

void AVerdunWorldBuilderManager::AutoGenerateCraterDensityZones()
{
	UE_LOG(LogTemp, Log, TEXT("Auto-generating crater density zones around forts..."));

	TArray<FVerdunLocation> Forts = GetLocationsByType(EVerdunLocationType::Fortification);

	for (const FVerdunLocation& Fort : Forts)
	{
		FVerdunCraterDensityZone Zone;
		Zone.ZoneCenter = Fort.WorldPosition;
		Zone.ZoneRadius = FortHighDensityRadius;
		Zone.CratersPerHectare = FortCraterDensityPerHectare;
		Zone.IntensityLevel = FString::Printf(TEXT("high_density_fort_%s"), *Fort.Name);

		CraterDensityZones.Add(Zone);

		UE_LOG(LogTemp, Log, TEXT("  Added high-density zone around %s (%.0f craters/hectare)"),
			*Fort.Name, FortCraterDensityPerHectare);
	}
}

AActor* AVerdunWorldBuilderManager::SpawnLocationActor(const FVerdunLocation& Location)
{
	TSubclassOf<AActor> ActorClass = nullptr;

	// Determine which actor class to use
	switch (Location.Type)
	{
		case EVerdunLocationType::Fortification:
			ActorClass = FortificationActorClass;
			break;
		case EVerdunLocationType::DestroyedVillage:
			ActorClass = DestroyedVillageActorClass;
			break;
		case EVerdunLocationType::Hill:
			ActorClass = HillMarkerActorClass;
			break;
		default:
			if (LocationActorClasses.Contains(Location.Type))
			{
				ActorClass = LocationActorClasses[Location.Type];
			}
			break;
	}

	if (!ActorClass)
	{
		// No actor class specified, just log the location
		UE_LOG(LogTemp, Log, TEXT("  Location: %s (%s) at %s"), *Location.Name, *UEnum::GetValueAsString(Location.Type), *Location.WorldPosition.ToString());
		return nullptr;
	}

	// Spawn the actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString::Printf(TEXT("%s_%s"), *UEnum::GetValueAsString(Location.Type), *Location.Name));
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Location.WorldPosition);

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);

	if (SpawnedActor)
	{
		UE_LOG(LogTemp, Log, TEXT("  Spawned %s at %s"), *SpawnedActor->GetName(), *Location.WorldPosition.ToString());
	}

	return SpawnedActor;
}
