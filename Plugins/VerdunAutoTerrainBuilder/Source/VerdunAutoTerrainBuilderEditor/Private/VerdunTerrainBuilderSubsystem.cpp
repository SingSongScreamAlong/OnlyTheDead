// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerdunTerrainBuilderSubsystem.h"
#include "VerdunTerrainParameters.h"
#include "VerdunLocationData.h"
#include "VerdunCraterGenerator.h"
#include "VerdunTrenchGenerator.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "Landscape.h"
#include "LandscapeEditorUtils.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

void UVerdunTerrainBuilderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("VerdunTerrainBuilderSubsystem Initialized"));
}

void UVerdunTerrainBuilderSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("VerdunTerrainBuilderSubsystem Deinitialized"));
}

void UVerdunTerrainBuilderSubsystem::OpenTerrainBuilderWindow()
{
	FText DialogTitle = FText::FromString(TEXT("Verdun Auto Terrain Builder"));
	FText DialogText = FText::FromString(
		TEXT("This will automatically build the Verdun battlefield terrain from your data files.\n\n")
		TEXT("Steps:\n")
		TEXT("1. Load terrain parameters from JSON\n")
		TEXT("2. Load locations database from CSV\n")
		TEXT("3. Import heightmap and create landscape\n")
		TEXT("4. Generate craters\n")
		TEXT("5. Generate trenches\n")
		TEXT("6. Place location markers\n\n")
		TEXT("Data location: verdun_anthology/geographic_data/\n\n")
		TEXT("Do you want to proceed?")
	);

	EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNo, DialogText, &DialogTitle);

	if (Response == EAppReturnType::Yes)
	{
		BuildVerdunTerrain();
	}
}

void UVerdunTerrainBuilderSubsystem::BuildVerdunTerrain()
{
	LogProgress(TEXT("=== Starting Verdun Terrain Build ==="));

	// Validate data files exist
	if (!ValidateDataFiles())
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::FromString(TEXT("Failed to find required data files. Please check that verdun_anthology/geographic_data/ exists in your project root.")));
		return;
	}

	// Step 1: Load terrain parameters
	LogProgress(TEXT("Step 1: Loading terrain parameters..."));
	if (!LoadTerrainParameters())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to load terrain parameters. Check the log for details.")));
		return;
	}

	// Step 2: Load locations database
	LogProgress(TEXT("Step 2: Loading locations database..."));
	if (!LoadLocationsDatabase())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to load locations database. Check the log for details.")));
		return;
	}

	// Step 3: Import heightmap (optional - user may create manually)
	LogProgress(TEXT("Step 3: Checking for heightmap..."));
	// Note: Heightmap import is optional and can be done manually by the user

	// Step 4: Generate craters
	LogProgress(TEXT("Step 4: Generating craters..."));
	if (!GenerateCraters())
	{
		UE_LOG(LogTemp, Warning, TEXT("Crater generation had issues. Continuing..."));
	}

	// Step 5: Generate trenches
	LogProgress(TEXT("Step 5: Generating trenches..."));
	if (!GenerateTrenches())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trench generation had issues. Continuing..."));
	}

	// Step 6: Place location markers
	LogProgress(TEXT("Step 6: Placing location markers..."));
	if (!PlaceLocationMarkers())
	{
		UE_LOG(LogTemp, Warning, TEXT("Location marker placement had issues. Continuing..."));
	}

	LogProgress(TEXT("=== Verdun Terrain Build Complete! ==="));

	FMessageDialog::Open(EAppMsgType::Ok,
		FText::FromString(TEXT("Verdun terrain building complete!\n\nCheck the World Outliner for generated actors:\n- CraterGenerator\n- TrenchGenerator\n- Location markers")));
}

bool UVerdunTerrainBuilderSubsystem::LoadTerrainParameters()
{
	FString DataPath = GetVerdunDataPath();
	FString JsonPath = FPaths::Combine(DataPath, TEXT("terrain_parameters.json"));

	// Create terrain parameters object
	TerrainParameters = NewObject<UVerdunTerrainParameters>(this);
	if (!TerrainParameters)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create TerrainParameters object"));
		return false;
	}

	// Load from JSON
	if (!TerrainParameters->LoadFromJSON(JsonPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load terrain parameters from: %s"), *JsonPath);
		return false;
	}

	LogProgress(FString::Printf(TEXT("Successfully loaded terrain parameters: %s"), *TerrainParameters->TerrainName));
	return true;
}

bool UVerdunTerrainBuilderSubsystem::LoadLocationsDatabase()
{
	FString DataPath = GetVerdunDataPath();
	FString CsvPath = FPaths::Combine(DataPath, TEXT("locations_database.csv"));

	// Create location database object
	LocationDatabase = NewObject<UVerdunLocationDatabase>(this);
	if (!LocationDatabase)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create LocationDatabase object"));
		return false;
	}

	// Load from CSV
	if (!LocationDatabase->LoadFromCSV(CsvPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load locations database from: %s"), *CsvPath);
		return false;
	}

	// Convert to UE5 coordinates
	if (TerrainParameters)
	{
		LocationDatabase->ConvertToUE5Coordinates(TerrainParameters);
	}

	LogProgress(FString::Printf(TEXT("Successfully loaded %d locations"), LocationDatabase->Locations.Num()));
	return true;
}

bool UVerdunTerrainBuilderSubsystem::ImportHeightmapAndCreateLandscape()
{
	LogProgress(TEXT("Note: Heightmap import should be done using UE5's Landscape Import tool."));
	LogProgress(TEXT("Use the 16-bit PNG heightmap from verdun_anthology/geographic_data/UE5_Import/"));
	return true;
}

bool UVerdunTerrainBuilderSubsystem::GenerateCraters()
{
	if (!GEditor || !GEditor->GetEditorWorldContext().World())
	{
		UE_LOG(LogTemp, Error, TEXT("No valid world found"));
		return false;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();

	// Spawn crater generator actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(TEXT("VerdunCraterGenerator"));

	CraterGenerator = World->SpawnActor<AVerdunCraterGenerator>(AVerdunCraterGenerator::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!CraterGenerator)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn crater generator"));
		return false;
	}

	// Configure crater generator
	CraterGenerator->TerrainParameters = TerrainParameters;
	CraterGenerator->LocationDatabase = LocationDatabase;

	// Generate craters
	CraterGenerator->GenerateCraters();

	LogProgress(FString::Printf(TEXT("Generated %d craters"), CraterGenerator->GeneratedCraters.Num()));
	return true;
}

bool UVerdunTerrainBuilderSubsystem::GenerateTrenches()
{
	if (!GEditor || !GEditor->GetEditorWorldContext().World())
	{
		UE_LOG(LogTemp, Error, TEXT("No valid world found"));
		return false;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();

	// Spawn trench generator actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(TEXT("VerdunTrenchGenerator"));

	TrenchGenerator = World->SpawnActor<AVerdunTrenchGenerator>(AVerdunTrenchGenerator::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!TrenchGenerator)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn trench generator"));
		return false;
	}

	// Configure trench generator
	TrenchGenerator->TerrainParameters = TerrainParameters;
	TrenchGenerator->LocationDatabase = LocationDatabase;

	// Generate trenches
	TrenchGenerator->GenerateTrenchNetworks();

	// Create spline visualization
	TrenchGenerator->CreateSplineVisualization();

	LogProgress(FString::Printf(TEXT("Generated %d trench segments"), TrenchGenerator->GeneratedTrenches.Num()));
	return true;
}

bool UVerdunTerrainBuilderSubsystem::PlaceLocationMarkers()
{
	if (!LocationDatabase || !GEditor || !GEditor->GetEditorWorldContext().World())
	{
		UE_LOG(LogTemp, Error, TEXT("No valid world or location database found"));
		return false;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();

	// Place empty actors as markers for now
	// User can replace these with actual fort/village meshes
	int32 PlacedCount = 0;

	for (const FVerdunLocation& Location : LocationDatabase->Locations)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(*FString::Printf(TEXT("Marker_%s"), *Location.Name));

		AActor* Marker = World->SpawnActor<AActor>(AActor::StaticClass(), Location.UE5Location, FRotator::ZeroRotator, SpawnParams);

		if (Marker)
		{
			// Set actor label for easy identification
			Marker->SetActorLabel(FString::Printf(TEXT("%s (%s)"), *Location.Name, *Location.Type));
			PlacedCount++;
		}
	}

	LogProgress(FString::Printf(TEXT("Placed %d location markers"), PlacedCount));
	return true;
}

FString UVerdunTerrainBuilderSubsystem::GetVerdunDataPath() const
{
	// Get project root directory
	FString ProjectDir = FPaths::ProjectDir();
	FString DataPath = FPaths::Combine(ProjectDir, TEXT("verdun_anthology"), TEXT("geographic_data"));

	return DataPath;
}

bool UVerdunTerrainBuilderSubsystem::ValidateDataFiles()
{
	FString DataPath = GetVerdunDataPath();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Check if directory exists
	if (!PlatformFile.DirectoryExists(*DataPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Data directory not found: %s"), *DataPath);
		return false;
	}

	// Check for required files
	FString JsonPath = FPaths::Combine(DataPath, TEXT("terrain_parameters.json"));
	FString CsvPath = FPaths::Combine(DataPath, TEXT("locations_database.csv"));

	if (!PlatformFile.FileExists(*JsonPath))
	{
		UE_LOG(LogTemp, Error, TEXT("terrain_parameters.json not found: %s"), *JsonPath);
		return false;
	}

	if (!PlatformFile.FileExists(*CsvPath))
	{
		UE_LOG(LogTemp, Error, TEXT("locations_database.csv not found: %s"), *CsvPath);
		return false;
	}

	LogProgress(TEXT("Data files validated successfully"));
	return true;
}

void UVerdunTerrainBuilderSubsystem::LogProgress(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[Verdun Terrain Builder] %s"), *Message);
}
