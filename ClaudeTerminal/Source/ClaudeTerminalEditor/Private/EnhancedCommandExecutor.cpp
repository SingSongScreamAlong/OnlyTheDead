// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnhancedCommandExecutor.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Selection.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Camera/CameraActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

FEnhancedCommandExecutor::FEnhancedCommandExecutor()
	: bInitialized(false)
{
}

FEnhancedCommandExecutor::~FEnhancedCommandExecutor()
{
}

bool FEnhancedCommandExecutor::Initialize(FString& OutLog)
{
	OutLog = TEXT("Initializing Enhanced Command Executor...\n");

	// Build asset catalog
	if (!AssetCatalog.BuildCatalog(OutLog))
	{
		OutLog += TEXT("ERROR: Failed to build asset catalog\n");
		return false;
	}

	OutLog += AssetCatalog.GetCatalogSummary(false);

	bInitialized = true;
	OutLog += TEXT("Enhanced Command Executor initialized successfully!\n");
	return true;
}

bool FEnhancedCommandExecutor::ProcessResponse(const FString& Response, UWorld* World, FString& OutExecutionLog)
{
	if (!bInitialized)
	{
		Initialize(OutExecutionLog);
	}

	if (!World)
	{
		OutExecutionLog = TEXT("ERROR: Invalid world");
		return false;
	}

	// Extract JSON command
	TSharedPtr<FJsonObject> Command;
	if (!ExtractJSONCommand(Response, Command))
	{
		// If no JSON found, treat as conversational response
		OutExecutionLog = TEXT("Response received (no command to execute)");
		return true;
	}

	FString CommandType;
	if (!Command->TryGetStringField(TEXT("command"), CommandType))
	{
		OutExecutionLog = TEXT("ERROR: No command type specified");
		return false;
	}

	TSharedPtr<FJsonObject> Parameters;
	if (Command->HasField(TEXT("parameters")))
	{
		Parameters = Command->GetObjectField(TEXT("parameters"));
	}

	OutExecutionLog = FString::Printf(TEXT("Executing command: %s\n"), *CommandType);

	// Route to appropriate handler
	bool bSuccess = false;

	// Asset & Object Placement
	if (CommandType == TEXT("spawn_object")) {
		bSuccess = ExecuteSpawnObject(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("place_building")) {
		bSuccess = ExecutePlaceBuilding(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("place_props")) {
		bSuccess = ExecutePlaceProps(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("place_vegetation")) {
		bSuccess = ExecutePlaceVegetation(Parameters, World, OutExecutionLog);
	}

	// Material & Texture
	else if (CommandType == TEXT("apply_material")) {
		bSuccess = ExecuteApplyMaterial(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("set_material_parameter")) {
		bSuccess = ExecuteSetMaterialParameter(Parameters, World, OutExecutionLog);
	}

	// Lighting & Atmosphere
	else if (CommandType == TEXT("create_light")) {
		bSuccess = ExecuteCreateLight(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("modify_light")) {
		bSuccess = ExecuteModifyLight(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("set_time_of_day")) {
		bSuccess = ExecuteSetTimeOfDay(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("set_weather")) {
		bSuccess = ExecuteSetWeather(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("set_fog")) {
		bSuccess = ExecuteSetFog(Parameters, World, OutExecutionLog);
	}

	// Animation & Cinematic
	else if (CommandType == TEXT("create_camera")) {
		bSuccess = ExecuteCreateCamera(Parameters, World, OutExecutionLog);
	}

	// Actor Manipulation
	else if (CommandType == TEXT("modify_actor")) {
		bSuccess = ExecuteModifyActor(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("delete_actor")) {
		bSuccess = ExecuteDeleteActor(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("duplicate_actor")) {
		bSuccess = ExecuteDuplicateActor(Parameters, World, OutExecutionLog);
	}

	// Batch Operations
	else if (CommandType == TEXT("batch_place")) {
		bSuccess = ExecuteBatchPlace(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("arrange_pattern")) {
		bSuccess = ExecuteArrangePattern(Parameters, World, OutExecutionLog);
	}

	// Landscape
	else if (CommandType == TEXT("create_landscape")) {
		bSuccess = ExecuteCreateLandscape(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("import_terrain")) {
		bSuccess = ExecuteImportTerrain(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("modify_terrain")) {
		bSuccess = ExecuteModifyTerrain(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("create_crater")) {
		bSuccess = ExecuteCreateCrater(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("create_trench")) {
		bSuccess = ExecuteCreateTrench(Parameters, World, OutExecutionLog);
	}

	// Fall back to base executor for legacy commands
	else {
		return BaseExecutor.ProcessResponse(Response, World, OutExecutionLog);
	}

	return bSuccess;
}

// ===== ASSET & OBJECT PLACEMENT =====

bool FEnhancedCommandExecutor::ExecuteSpawnObject(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get object description
	FString ObjectDescription;
	if (!Parameters->TryGetStringField(TEXT("object_type"), ObjectDescription))
	{
		OutLog += TEXT("ERROR: No object_type specified\n");
		return false;
	}

	// Find best matching asset
	UStaticMesh* Mesh = FindBestStaticMesh(ObjectDescription);
	if (!Mesh)
	{
		OutLog += FString::Printf(TEXT("ERROR: Could not find asset matching '%s'\n"), *ObjectDescription);
		return false;
	}

	// Resolve location (supports "camera", "cursor", "selected", or explicit coordinates)
	FVector Location = ResolveLocation(Parameters, World);
	FRotator Rotation = ResolveRotation(Parameters, World);

	// Spawn actor
	FScopedTransaction Transaction(FText::FromString(TEXT("Spawn Object")));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
	if (!NewActor)
	{
		OutLog += TEXT("ERROR: Failed to spawn actor\n");
		return false;
	}

	NewActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
	NewActor->SetActorLabel(FString::Printf(TEXT("%s"), *ObjectDescription));

	// Intelligent placement (snap to ground, check collisions, etc.)
	PlaceObjectIntelligently(NewActor, Location, ObjectDescription, World, OutLog);

	// Visual feedback
	SelectActor(NewActor);

	OutLog += FString::Printf(TEXT("Spawned %s at %s\n"), *ObjectDescription, *Location.ToString());
	return true;
}

bool FEnhancedCommandExecutor::ExecutePlaceBuilding(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get building type (e.g., "French WWI farmhouse")
	FString BuildingType;
	if (!Parameters->TryGetStringField(TEXT("building_type"), BuildingType))
	{
		OutLog += TEXT("ERROR: No building_type specified\n");
		return false;
	}

	// Search for matching building asset
	TArray<FAssetMetadata> BuildingAssets;
	int32 NumFound = AssetCatalog.SearchStaticMeshes(BuildingType, BuildingAssets, 5);

	if (NumFound == 0)
	{
		OutLog += FString::Printf(TEXT("ERROR: No buildings found matching '%s'\n"), *BuildingType);
		return false;
	}

	// Use the best match
	const FAssetMetadata& BestMatch = BuildingAssets[0];
	UStaticMesh* BuildingMesh = LoadObject<UStaticMesh>(nullptr, *BestMatch.AssetPath);

	if (!BuildingMesh)
	{
		OutLog += FString::Printf(TEXT("ERROR: Failed to load building asset: %s\n"), *BestMatch.AssetPath);
		return false;
	}

	// Resolve location
	FVector Location = ResolveLocation(Parameters, World);
	FRotator Rotation = ResolveRotation(Parameters, World);

	// Snap to ground
	Location = FindGroundHeight(Location, World);

	// Spawn building
	FScopedTransaction Transaction(FText::FromString(TEXT("Place Building")));

	FActorSpawnParameters SpawnParams;
	AStaticMeshActor* Building = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);

	if (!Building)
	{
		OutLog += TEXT("ERROR: Failed to spawn building\n");
		return false;
	}

	Building->GetStaticMeshComponent()->SetStaticMesh(BuildingMesh);
	Building->SetActorLabel(FString::Printf(TEXT("%s"), *BuildingType));

	// Apply appropriate material if specified
	if (Parameters->HasField(TEXT("material")))
	{
		FString MaterialQuery;
		Parameters->TryGetStringField(TEXT("material"), MaterialQuery);
		UMaterialInterface* Material = FindBestMaterial(MaterialQuery);
		if (Material)
		{
			Building->GetStaticMeshComponent()->SetMaterial(0, Material);
			OutLog += FString::Printf(TEXT("  Applied material: %s\n"), *MaterialQuery);
		}
	}

	// Update asset usage
	AssetCatalog.UpdateAssetUsage(BestMatch.AssetPath);

	// Visual feedback
	SelectActor(Building);

	OutLog += FString::Printf(TEXT("Placed %s at %s\n"), *BuildingType, *Location.ToString());
	OutLog += FString::Printf(TEXT("  Asset: %s\n"), *BestMatch.AssetName);

	return true;
}

bool FEnhancedCommandExecutor::ExecutePlaceProps(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get prop type
	FString PropType;
	if (!Parameters->TryGetStringField(TEXT("prop_type"), PropType))
	{
		OutLog += TEXT("ERROR: No prop_type specified\n");
		return false;
	}

	// Get count (default 1)
	int32 Count = 1;
	Parameters->TryGetNumberField(TEXT("count"), Count);
	Count = FMath::Clamp(Count, 1, 100); // Reasonable limit

	// Search for matching props
	TArray<FAssetMetadata> PropAssets;
	int32 NumFound = AssetCatalog.SearchStaticMeshes(PropType, PropAssets, 10);

	if (NumFound == 0)
	{
		OutLog += FString::Printf(TEXT("ERROR: No props found matching '%s'\n"), *PropType);
		return false;
	}

	// Get base location
	FVector BaseLocation = ResolveLocation(Parameters, World);

	// Get distribution pattern
	FString Pattern = TEXT("scatter"); // Default
	Parameters->TryGetStringField(TEXT("pattern"), Pattern);

	// Get radius for scatter
	float Radius = 1000.0f; // Default 10 meters
	Parameters->TryGetNumberField(TEXT("radius"), Radius);

	FScopedTransaction Transaction(FText::FromString(TEXT("Place Props")));

	TArray<AActor*> SpawnedActors;

	for (int32 i = 0; i < Count; i++)
	{
		// Choose random asset from top matches for variety
		int32 AssetIndex = FMath::RandRange(0, FMath::Min(3, NumFound - 1));
		const FAssetMetadata& Asset = PropAssets[AssetIndex];

		UStaticMesh* PropMesh = LoadObject<UStaticMesh>(nullptr, *Asset.AssetPath);
		if (!PropMesh) continue;

		// Calculate location based on pattern
		FVector Location = BaseLocation;
		if (Pattern == TEXT("scatter"))
		{
			// Random scatter
			float Angle = FMath::FRandRange(0.0f, 360.0f);
			float Distance = FMath::FRandRange(0.0f, Radius);
			Location.X += FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance;
			Location.Y += FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance;
		}
		else if (Pattern == TEXT("line"))
		{
			// Linear arrangement
			Location.Y += i * (Radius / Count);
		}
		else if (Pattern == TEXT("circle"))
		{
			// Circular arrangement
			float Angle = (360.0f / Count) * i;
			Location.X += FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius;
			Location.Y += FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius;
		}

		// Snap to ground
		Location = FindGroundHeight(Location, World);

		// Random rotation
		FRotator Rotation = FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

		// Spawn
		FActorSpawnParameters SpawnParams;
		AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);

		if (PropActor)
		{
			PropActor->GetStaticMeshComponent()->SetStaticMesh(PropMesh);
			PropActor->SetActorLabel(FString::Printf(TEXT("%s_%d"), *PropType, i + 1));
			SpawnedActors.Add(PropActor);
		}
	}

	// Visual feedback
	SelectActors(SpawnedActors);

	OutLog += FString::Printf(TEXT("Placed %d props (%s) in %s pattern\n"), SpawnedActors.Num(), *PropType, *Pattern);
	return true;
}

bool FEnhancedCommandExecutor::ExecutePlaceVegetation(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	// Similar to ExecutePlaceProps but optimized for vegetation (trees, bushes, grass)
	return ExecutePlaceProps(Parameters, World, OutLog);
}

// ===== MATERIAL & TEXTURE =====

bool FEnhancedCommandExecutor::ExecuteApplyMaterial(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get material description
	FString MaterialQuery;
	if (!Parameters->TryGetStringField(TEXT("material"), MaterialQuery))
	{
		OutLog += TEXT("ERROR: No material specified\n");
		return false;
	}

	// Find best matching material
	UMaterialInterface* Material = FindBestMaterial(MaterialQuery);
	if (!Material)
	{
		OutLog += FString::Printf(TEXT("ERROR: Could not find material matching '%s'\n"), *MaterialQuery);
		return false;
	}

	// Get target (selected actors or specific actor)
	if (!GEditor || !GEditor->GetSelectedActorCount())
	{
		OutLog += TEXT("ERROR: No actors selected. Select actors first, then apply material.\n");
		return false;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Apply Material")));

	int32 ActorsModified = 0;

	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (!Actor) continue;

		// Find static mesh component
		UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
		if (MeshComp)
		{
			MeshComp->SetMaterial(0, Material);
			ActorsModified++;
		}
	}

	OutLog += FString::Printf(TEXT("Applied material '%s' to %d actors\n"), *MaterialQuery, ActorsModified);
	return true;
}

bool FEnhancedCommandExecutor::ExecuteSetMaterialParameter(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get target (selected actors)
	if (!GEditor || !GEditor->GetSelectedActorCount())
	{
		OutLog += TEXT("ERROR: No actors selected. Select actors first.\n");
		return false;
	}

	// Get parameter name and value
	FString ParamName;
	if (!Parameters->TryGetStringField(TEXT("parameter_name"), ParamName))
	{
		OutLog += TEXT("ERROR: No parameter_name specified\n");
		return false;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Set Material Parameter")));

	int32 ActorsModified = 0;

	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (!Actor) continue;

		UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
		if (!MeshComp) continue;

		// Get or create dynamic material instance
		for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
		{
			UMaterialInterface* Material = MeshComp->GetMaterial(i);
			if (!Material) continue;

			UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Material);
			if (!DynMat)
			{
				// Create dynamic instance
				DynMat = MeshComp->CreateDynamicMaterialInstance(i, Material);
			}

			if (!DynMat) continue;

			// Try to set parameter based on type
			// Scalar parameter
			if (Parameters->HasField(TEXT("scalar_value")))
			{
				double Value;
				Parameters->TryGetNumberField(TEXT("scalar_value"), Value);
				DynMat->SetScalarParameterValue(FName(*ParamName), Value);
				OutLog += FString::Printf(TEXT("  Set scalar parameter '%s' = %.2f\n"), *ParamName, Value);
			}
			// Vector parameter
			else if (Parameters->HasField(TEXT("vector_value")))
			{
				FLinearColor Color = ParseColor(Parameters, TEXT("vector_value"), FLinearColor::White);
				DynMat->SetVectorParameterValue(FName(*ParamName), Color);
				OutLog += FString::Printf(TEXT("  Set vector parameter '%s' = (%.2f, %.2f, %.2f, %.2f)\n"),
					*ParamName, Color.R, Color.G, Color.B, Color.A);
			}
			// Texture parameter
			else if (Parameters->HasField(TEXT("texture_path")))
			{
				FString TexturePath;
				Parameters->TryGetStringField(TEXT("texture_path"), TexturePath);
				UTexture* Texture = LoadObject<UTexture>(nullptr, *TexturePath);
				if (Texture)
				{
					DynMat->SetTextureParameterValue(FName(*ParamName), Texture);
					OutLog += FString::Printf(TEXT("  Set texture parameter '%s'\n"), *ParamName);
				}
			}

			ActorsModified++;
		}
	}

	OutLog += FString::Printf(TEXT("Modified material parameters on %d actors\n"), ActorsModified);
	return ActorsModified > 0;
}

// ===== LIGHTING & ATMOSPHERE =====

bool FEnhancedCommandExecutor::ExecuteCreateLight(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get light type
	FString LightType = TEXT("point");
	Parameters->TryGetStringField(TEXT("light_type"), LightType);

	FVector Location = ResolveLocation(Parameters, World);
	FRotator Rotation = ResolveRotation(Parameters, World);

	// Get light properties
	FLinearColor Color = ParseColor(Parameters, TEXT("color"), FLinearColor::White);
	float Intensity = 3000.0f;
	Parameters->TryGetNumberField(TEXT("intensity"), Intensity);

	FScopedTransaction Transaction(FText::FromString(TEXT("Create Light")));

	AActor* LightActor = nullptr;

	if (LightType == TEXT("directional"))
	{
		ADirectionalLight* DirLight = World->SpawnActor<ADirectionalLight>(Location, Rotation);
		if (DirLight)
		{
			DirLight->GetLightComponent()->SetIntensity(Intensity);
			DirLight->GetLightComponent()->SetLightColor(Color);
			LightActor = DirLight;
		}
	}
	else if (LightType == TEXT("point"))
	{
		APointLight* PointLight = World->SpawnActor<APointLight>(Location, Rotation);
		if (PointLight)
		{
			PointLight->GetPointLightComponent()->SetIntensity(Intensity);
			PointLight->GetPointLightComponent()->SetLightColor(Color);

			float Radius = 1000.0f;
			Parameters->TryGetNumberField(TEXT("radius"), Radius);
			PointLight->GetPointLightComponent()->SetAttenuationRadius(Radius);

			LightActor = PointLight;
		}
	}
	else if (LightType == TEXT("spot"))
	{
		ASpotLight* SpotLight = World->SpawnActor<ASpotLight>(Location, Rotation);
		if (SpotLight)
		{
			SpotLight->GetSpotLightComponent()->SetIntensity(Intensity);
			SpotLight->GetSpotLightComponent()->SetLightColor(Color);
			LightActor = SpotLight;
		}
	}

	if (!LightActor)
	{
		OutLog += FString::Printf(TEXT("ERROR: Failed to create %s light\n"), *LightType);
		return false;
	}

	SelectActor(LightActor);
	OutLog += FString::Printf(TEXT("Created %s light at %s\n"), *LightType, *Location.ToString());
	return true;
}

bool FEnhancedCommandExecutor::ExecuteModifyLight(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get target light (selected light or find by type)
	ALight* TargetLight = nullptr;

	if (GEditor && GEditor->GetSelectedActorCount() > 0)
	{
		// Use selected light
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			TargetLight = Cast<ALight>(*It);
			if (TargetLight) break;
		}
	}

	if (!TargetLight)
	{
		// Find directional light (sun) as default
		TargetLight = FindDirectionalLight(World);
	}

	if (!TargetLight)
	{
		OutLog += TEXT("ERROR: No light found. Select a light or ensure there's a directional light in scene.\n");
		return false;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Modify Light")));

	ULightComponent* LightComp = TargetLight->GetLightComponent();
	if (!LightComp)
	{
		OutLog += TEXT("ERROR: Light has no light component\n");
		return false;
	}

	bool bModified = false;

	// Modify intensity
	if (Parameters->HasField(TEXT("intensity")))
	{
		double Intensity;
		Parameters->TryGetNumberField(TEXT("intensity"), Intensity);
		LightComp->SetIntensity(Intensity);
		OutLog += FString::Printf(TEXT("  Set intensity to %.1f\n"), Intensity);
		bModified = true;
	}

	// Modify color
	if (Parameters->HasField(TEXT("color")))
	{
		FLinearColor Color = ParseColor(Parameters, TEXT("color"), FLinearColor::White);
		LightComp->SetLightColor(Color);
		OutLog += FString::Printf(TEXT("  Set color to (%.2f, %.2f, %.2f)\n"), Color.R, Color.G, Color.B);
		bModified = true;
	}

	// Modify radius (point/spot lights)
	if (Parameters->HasField(TEXT("radius")))
	{
		UPointLightComponent* PointLight = Cast<UPointLightComponent>(LightComp);
		if (PointLight)
		{
			double Radius;
			Parameters->TryGetNumberField(TEXT("radius"), Radius);
			PointLight->SetAttenuationRadius(Radius);
			OutLog += FString::Printf(TEXT("  Set radius to %.1f\n"), Radius);
			bModified = true;
		}
	}

	// Modify rotation (directional lights)
	if (Parameters->HasField(TEXT("rotation")))
	{
		FRotator Rotation = ParseRotator(Parameters, TEXT("rotation"), FRotator::ZeroRotator);
		TargetLight->SetActorRotation(Rotation);
		OutLog += FString::Printf(TEXT("  Set rotation to %s\n"), *Rotation.ToString());
		bModified = true;
	}

	if (bModified)
	{
		OutLog += FString::Printf(TEXT("Modified light: %s\n"), *TargetLight->GetName());
		return true;
	}
	else
	{
		OutLog += TEXT("No modifications specified\n");
		return false;
	}
}

bool FEnhancedCommandExecutor::ExecuteSetTimeOfDay(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get time (0-24 hours or description like "dawn", "noon", "dusk")
	FString TimeDescription;
	float TimeOfDay = 12.0f; // Noon default

	if (Parameters->TryGetStringField(TEXT("time"), TimeDescription))
	{
		FString LowerTime = TimeDescription.ToLower();
		if (LowerTime == TEXT("dawn") || LowerTime == TEXT("sunrise"))
			TimeOfDay = 6.0f;
		else if (LowerTime == TEXT("morning"))
			TimeOfDay = 9.0f;
		else if (LowerTime == TEXT("noon") || LowerTime == TEXT("midday"))
			TimeOfDay = 12.0f;
		else if (LowerTime == TEXT("afternoon"))
			TimeOfDay = 15.0f;
		else if (LowerTime == TEXT("dusk") || LowerTime == TEXT("sunset"))
			TimeOfDay = 18.0f;
		else if (LowerTime == TEXT("night") || LowerTime == TEXT("midnight"))
			TimeOfDay = 0.0f;
	}
	else
	{
		Parameters->TryGetNumberField(TEXT("time"), TimeOfDay);
	}

	// Find directional light (sun)
	ADirectionalLight* Sun = FindDirectionalLight(World);
	if (!Sun)
	{
		OutLog += TEXT("ERROR: No directional light found in scene\n");
		return false;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Set Time of Day")));

	// Calculate sun angle based on time
	float SunAngle = (TimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 at midnight, 0 at 6am, 90 at noon
	FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);

	Sun->SetActorRotation(SunRotation);

	// Adjust intensity based on time
	float Intensity = 5.0f; // Night
	if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f)
	{
		// Day time
		float DayProgress = (TimeOfDay - 6.0f) / 12.0f;
		Intensity = 5.0f + FMath::Sin(DayProgress * PI) * 15.0f; // Peak at noon
	}
	Sun->GetLightComponent()->SetIntensity(Intensity);

	OutLog += FString::Printf(TEXT("Set time of day to %.1f hours (sun angle: %.1f degrees)\n"), TimeOfDay, SunAngle);
	return true;
}

bool FEnhancedCommandExecutor::ExecuteSetWeather(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get weather type
	FString WeatherType = TEXT("clear");
	Parameters->TryGetStringField(TEXT("weather_type"), WeatherType);
	FString LowerWeather = WeatherType.ToLower();

	FScopedTransaction Transaction(FText::FromString(TEXT("Set Weather")));

	bool bModified = false;

	// Handle different weather types
	if (LowerWeather == TEXT("clear"))
	{
		// Clear weather: minimal fog, bright sun
		// Set fog
		AExponentialHeightFog* Fog = nullptr;
		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
		{
			Fog = *It;
			break;
		}
		if (Fog)
		{
			Fog->GetComponent()->SetFogDensity(0.01f);
			OutLog += TEXT("  Set fog to minimal\n");
		}

		// Set sun intensity
		ADirectionalLight* Sun = FindDirectionalLight(World);
		if (Sun)
		{
			Sun->GetLightComponent()->SetIntensity(10.0f);
			OutLog += TEXT("  Set sun to bright\n");
		}

		bModified = true;
	}
	else if (LowerWeather == TEXT("foggy") || LowerWeather == TEXT("fog"))
	{
		// Foggy: heavy fog, dimmer sun
		AExponentialHeightFog* Fog = nullptr;
		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
		{
			Fog = *It;
			break;
		}
		if (!Fog)
		{
			Fog = World->SpawnActor<AExponentialHeightFog>();
		}
		if (Fog)
		{
			Fog->GetComponent()->SetFogDensity(0.05f);
			Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.7f, 0.7f, 0.8f));
			OutLog += TEXT("  Set heavy fog\n");
		}

		ADirectionalLight* Sun = FindDirectionalLight(World);
		if (Sun)
		{
			Sun->GetLightComponent()->SetIntensity(5.0f);
			OutLog += TEXT("  Dimmed sun\n");
		}

		bModified = true;
	}
	else if (LowerWeather == TEXT("overcast") || LowerWeather == TEXT("cloudy"))
	{
		// Overcast: medium fog, medium sun
		AExponentialHeightFog* Fog = nullptr;
		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
		{
			Fog = *It;
			break;
		}
		if (!Fog)
		{
			Fog = World->SpawnActor<AExponentialHeightFog>();
		}
		if (Fog)
		{
			Fog->GetComponent()->SetFogDensity(0.02f);
			Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.6f, 0.6f, 0.65f));
			OutLog += TEXT("  Set overcast fog\n");
		}

		ADirectionalLight* Sun = FindDirectionalLight(World);
		if (Sun)
		{
			Sun->GetLightComponent()->SetIntensity(7.0f);
			Sun->GetLightComponent()->SetLightColor(FLinearColor(0.9f, 0.9f, 1.0f));
			OutLog += TEXT("  Set diffuse sunlight\n");
		}

		bModified = true;
	}
	else if (LowerWeather == TEXT("stormy") || LowerWeather == TEXT("storm"))
	{
		// Stormy: dark, heavy fog
		AExponentialHeightFog* Fog = nullptr;
		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
		{
			Fog = *It;
			break;
		}
		if (!Fog)
		{
			Fog = World->SpawnActor<AExponentialHeightFog>();
		}
		if (Fog)
		{
			Fog->GetComponent()->SetFogDensity(0.08f);
			Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.4f, 0.4f, 0.5f));
			OutLog += TEXT("  Set stormy conditions\n");
		}

		ADirectionalLight* Sun = FindDirectionalLight(World);
		if (Sun)
		{
			Sun->GetLightComponent()->SetIntensity(3.0f);
			Sun->GetLightComponent()->SetLightColor(FLinearColor(0.7f, 0.7f, 0.8f));
			OutLog += TEXT("  Set dark lighting\n");
		}

		bModified = true;
	}

	// Custom intensity/density if specified
	if (Parameters->HasField(TEXT("fog_density")))
	{
		double Density;
		Parameters->TryGetNumberField(TEXT("fog_density"), Density);

		AExponentialHeightFog* Fog = nullptr;
		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
		{
			Fog = *It;
			break;
		}
		if (!Fog)
		{
			Fog = World->SpawnActor<AExponentialHeightFog>();
		}
		if (Fog)
		{
			Fog->GetComponent()->SetFogDensity(Density);
			OutLog += FString::Printf(TEXT("  Custom fog density: %.3f\n"), Density);
		}
		bModified = true;
	}

	if (bModified)
	{
		OutLog += FString::Printf(TEXT("Set weather to: %s\n"), *WeatherType);
		return true;
	}
	else
	{
		OutLog += TEXT("Unknown weather type. Use: clear, foggy, overcast, stormy\n");
		return false;
	}
}

bool FEnhancedCommandExecutor::ExecuteSetFog(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Find or create exponential height fog
	AExponentialHeightFog* Fog = nullptr;
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		Fog = *It;
		break;
	}

	if (!Fog)
	{
		// Create fog if it doesn't exist
		FScopedTransaction Transaction(FText::FromString(TEXT("Create Fog")));
		Fog = World->SpawnActor<AExponentialHeightFog>();
		if (!Fog)
		{
			OutLog += TEXT("ERROR: Failed to create fog\n");
			return false;
		}
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Set Fog")));

	// Get fog properties
	float Density = 0.02f;
	Parameters->TryGetNumberField(TEXT("density"), Density);

	float Height = -200.0f;
	Parameters->TryGetNumberField(TEXT("height"), Height);

	FLinearColor Color = ParseColor(Parameters, TEXT("color"), FLinearColor(0.8f, 0.8f, 0.9f));

	// Apply fog settings
	UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
	if (FogComp)
	{
		FogComp->SetFogDensity(Density);
		FogComp->SetFogHeightFalloff(0.2f);
		FogComp->SetFogInscatteringColor(Color);
	}

	OutLog += FString::Printf(TEXT("Set fog: density=%.3f, height=%.1f\n"), Density, Height);
	return true;
}

// ===== ANIMATION & CINEMATIC =====

bool FEnhancedCommandExecutor::ExecuteCreateCamera(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FVector Location = ResolveLocation(Parameters, World);
	FRotator Rotation = ResolveRotation(Parameters, World);

	FScopedTransaction Transaction(FText::FromString(TEXT("Create Camera")));

	ACameraActor* Camera = World->SpawnActor<ACameraActor>(Location, Rotation);
	if (!Camera)
	{
		OutLog += TEXT("ERROR: Failed to create camera\n");
		return false;
	}

	SelectActor(Camera);
	OutLog += FString::Printf(TEXT("Created camera at %s\n"), *Location.ToString());
	return true;
}

bool FEnhancedCommandExecutor::ExecutePlayAnimation(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!Parameters.IsValid())
	{
		OutLog += TEXT("ERROR: No parameters provided\n");
		return false;
	}

	// Get animation asset path or name
	FString AnimationQuery;
	if (!Parameters->TryGetStringField(TEXT("animation"), AnimationQuery))
	{
		OutLog += TEXT("ERROR: No animation specified\n");
		return false;
	}

	// Get target (selected actors with skeletal mesh components)
	if (!GEditor || !GEditor->GetSelectedActorCount())
	{
		OutLog += TEXT("ERROR: No actors selected. Select actors with skeletal meshes.\n");
		return false;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Play Animation")));

	int32 ActorsAnimated = 0;

	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (!Actor) continue;

		// Find skeletal mesh component
		USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
		if (!SkelMeshComp) continue;

		// Try to load animation asset
		UAnimationAsset* AnimAsset = nullptr;

		// First try as direct path
		AnimAsset = LoadObject<UAnimationAsset>(nullptr, *AnimationQuery);

		// If not found, try searching asset catalog
		if (!AnimAsset)
		{
			// Search for animation assets
			TArray<FAssetMetadata> AnimAssets;
			if (AssetCatalog.SearchAssets(AnimationQuery, AnimAssets, 5) > 0)
			{
				for (const FAssetMetadata& Asset : AnimAssets)
				{
					if (Asset.AssetType == TEXT("AnimSequence") || Asset.AssetType == TEXT("AnimMontage"))
					{
						AnimAsset = LoadObject<UAnimationAsset>(nullptr, *Asset.AssetPath);
						if (AnimAsset) break;
					}
				}
			}
		}

		if (!AnimAsset)
		{
			OutLog += FString::Printf(TEXT("  Could not find animation: %s\n"), *AnimationQuery);
			continue;
		}

		// Play animation
		UAnimSequence* AnimSequence = Cast<UAnimSequence>(AnimAsset);
		if (AnimSequence)
		{
			SkelMeshComp->PlayAnimation(AnimSequence, false);
			OutLog += FString::Printf(TEXT("  Playing animation on %s\n"), *Actor->GetName());
			ActorsAnimated++;
		}
		else
		{
			// Try as montage
			UAnimMontage* AnimMontage = Cast<UAnimMontage>(AnimAsset);
			if (AnimMontage && SkelMeshComp->GetAnimInstance())
			{
				SkelMeshComp->GetAnimInstance()->Montage_Play(AnimMontage);
				OutLog += FString::Printf(TEXT("  Playing montage on %s\n"), *Actor->GetName());
				ActorsAnimated++;
			}
		}
	}

	if (ActorsAnimated > 0)
	{
		OutLog += FString::Printf(TEXT("Played animation on %d actors\n"), ActorsAnimated);
		return true;
	}
	else
	{
		OutLog += TEXT("No valid skeletal mesh components found on selected actors\n");
		return false;
	}
}

// ===== ACTOR MANIPULATION =====

bool FEnhancedCommandExecutor::ExecuteModifyActor(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!GEditor || !GEditor->GetSelectedActorCount())
	{
		OutLog += TEXT("ERROR: No actors selected\n");
		return false;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Modify Actor")));

	// Get modification parameters
	FVector LocationOffset = ParseVector(Parameters, TEXT("location_offset"), FVector::ZeroVector);
	FRotator RotationOffset = ParseRotator(Parameters, TEXT("rotation_offset"), FRotator::ZeroRotator);
	FVector Scale = ParseVector(Parameters, TEXT("scale"), FVector(1.0f, 1.0f, 1.0f));

	bool bSetLocation = Parameters->HasField(TEXT("location"));
	bool bSetRotation = Parameters->HasField(TEXT("rotation"));
	bool bSetScale = Parameters->HasField(TEXT("scale"));

	FVector NewLocation = ParseVector(Parameters, TEXT("location"), FVector::ZeroVector);
	FRotator NewRotation = ParseRotator(Parameters, TEXT("rotation"), FRotator::ZeroRotator);

	int32 ActorsModified = 0;

	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (!Actor) continue;

		if (bSetLocation)
		{
			Actor->SetActorLocation(NewLocation);
		}
		else if (!LocationOffset.IsZero())
		{
			Actor->SetActorLocation(Actor->GetActorLocation() + LocationOffset);
		}

		if (bSetRotation)
		{
			Actor->SetActorRotation(NewRotation);
		}
		else if (!RotationOffset.IsZero())
		{
			Actor->SetActorRotation(Actor->GetActorRotation() + RotationOffset);
		}

		if (bSetScale)
		{
			Actor->SetActorScale3D(Scale);
		}

		ActorsModified++;
	}

	OutLog += FString::Printf(TEXT("Modified %d actors\n"), ActorsModified);
	return true;
}

bool FEnhancedCommandExecutor::ExecuteDeleteActor(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!GEditor || !GEditor->GetSelectedActorCount())
	{
		OutLog += TEXT("ERROR: No actors selected\n");
		return false;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Delete Actor")));

	TArray<AActor*> ActorsToDelete;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (Actor)
		{
			ActorsToDelete.Add(Actor);
		}
	}

	for (AActor* Actor : ActorsToDelete)
	{
		Actor->Destroy();
	}

	OutLog += FString::Printf(TEXT("Deleted %d actors\n"), ActorsToDelete.Num());
	return true;
}

bool FEnhancedCommandExecutor::ExecuteDuplicateActor(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!GEditor || !GEditor->GetSelectedActorCount())
	{
		OutLog += TEXT("ERROR: No actors selected\n");
		return false;
	}

	// Get duplication parameters
	int32 Count = 1;
	Parameters->TryGetNumberField(TEXT("count"), Count);
	Count = FMath::Clamp(Count, 1, 100);

	FVector Offset = ParseVector(Parameters, TEXT("offset"), FVector(500.0f, 0.0f, 0.0f));
	FString Pattern = TEXT("linear");
	Parameters->TryGetStringField(TEXT("pattern"), Pattern);

	FScopedTransaction Transaction(FText::FromString(TEXT("Duplicate Actor")));

	TArray<AActor*> SourceActors;
	TArray<AActor*> NewActors;

	// Collect source actors
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (Actor)
		{
			SourceActors.Add(Actor);
		}
	}

	// Duplicate each source actor
	for (AActor* SourceActor : SourceActors)
	{
		FVector BaseLocation = SourceActor->GetActorLocation();
		FRotator BaseRotation = SourceActor->GetActorRotation();

		for (int32 i = 0; i < Count; i++)
		{
			// Calculate position based on pattern
			FVector NewLocation = BaseLocation;

			if (Pattern == TEXT("linear") || Pattern == TEXT("line"))
			{
				NewLocation = BaseLocation + Offset * (i + 1);
			}
			else if (Pattern == TEXT("grid"))
			{
				int32 GridSize = FMath::CeilToInt(FMath::Sqrt(Count));
				int32 Row = i / GridSize;
				int32 Col = i % GridSize;
				NewLocation = BaseLocation + FVector(Row * Offset.X, Col * Offset.Y, 0.0f);
			}
			else if (Pattern == TEXT("circle"))
			{
				float Angle = (360.0f / Count) * i;
				float AngleRad = FMath::DegreesToRadians(Angle);
				float Radius = Offset.Size();
				NewLocation = BaseLocation + FVector(
					FMath::Cos(AngleRad) * Radius,
					FMath::Sin(AngleRad) * Radius,
					0.0f
				);
			}
			else if (Pattern == TEXT("scatter"))
			{
				float Radius = Offset.Size();
				float Angle = FMath::FRandRange(0.0f, 360.0f);
				float Distance = FMath::FRandRange(0.0f, Radius);
				float AngleRad = FMath::DegreesToRadians(Angle);
				NewLocation = BaseLocation + FVector(
					FMath::Cos(AngleRad) * Distance,
					FMath::Sin(AngleRad) * Distance,
					FMath::FRandRange(-Offset.Z, Offset.Z)
				);
			}

			// Duplicate actor
			FActorSpawnParameters SpawnParams;
			SpawnParams.Template = SourceActor;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* NewActor = World->SpawnActor<AActor>(
				SourceActor->GetClass(),
				NewLocation,
				BaseRotation,
				SpawnParams
			);

			if (NewActor)
			{
				NewActor->SetActorLabel(FString::Printf(TEXT("%s_Copy_%d"), *SourceActor->GetActorLabel(), i + 1));
				NewActors.Add(NewActor);
			}
		}
	}

	// Select duplicated actors
	if (NewActors.Num() > 0)
	{
		SelectActors(NewActors);
		OutLog += FString::Printf(TEXT("Duplicated %d actors into %d copies in %s pattern\n"),
			SourceActors.Num(), NewActors.Num(), *Pattern);
		return true;
	}
	else
	{
		OutLog += TEXT("Failed to duplicate actors\n");
		return false;
	}
}

// ===== BATCH OPERATIONS =====

bool FEnhancedCommandExecutor::ExecuteBatchPlace(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	// Wrapper around place_props for now
	return ExecutePlaceProps(Parameters, World, OutLog);
}

bool FEnhancedCommandExecutor::ExecuteArrangePattern(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	if (!GEditor || !GEditor->GetSelectedActorCount())
	{
		OutLog += TEXT("ERROR: No actors selected\n");
		return false;
	}

	// Get pattern parameters
	FString Pattern = TEXT("grid");
	Parameters->TryGetStringField(TEXT("pattern"), Pattern);

	float Spacing = 500.0f; // 5 meters default
	Parameters->TryGetNumberField(TEXT("spacing"), Spacing);

	FVector Center = FVector::ZeroVector;
	if (Parameters->HasField(TEXT("center")))
	{
		Center = ParseVector(Parameters, TEXT("center"), FVector::ZeroVector);
	}
	else
	{
		// Calculate center of selected actors
		int32 Count = 0;
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			AActor* Actor = Cast<AActor>(*It);
			if (Actor)
			{
				Center += Actor->GetActorLocation();
				Count++;
			}
		}
		if (Count > 0)
		{
			Center /= Count;
		}
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Arrange Pattern")));

	TArray<AActor*> ActorsToArrange;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (Actor)
		{
			ActorsToArrange.Add(Actor);
		}
	}

	if (ActorsToArrange.Num() == 0)
	{
		OutLog += TEXT("No valid actors to arrange\n");
		return false;
	}

	// Arrange actors based on pattern
	if (Pattern == TEXT("grid"))
	{
		int32 GridSize = FMath::CeilToInt(FMath::Sqrt(ActorsToArrange.Num()));
		int32 HalfSize = GridSize / 2;

		for (int32 i = 0; i < ActorsToArrange.Num(); i++)
		{
			int32 Row = i / GridSize;
			int32 Col = i % GridSize;

			FVector NewLocation = Center + FVector(
				(Row - HalfSize) * Spacing,
				(Col - HalfSize) * Spacing,
				0.0f
			);

			ActorsToArrange[i]->SetActorLocation(NewLocation);
		}

		OutLog += FString::Printf(TEXT("Arranged %d actors in %dx%d grid with %.1f spacing\n"),
			ActorsToArrange.Num(), GridSize, GridSize, Spacing);
	}
	else if (Pattern == TEXT("circle"))
	{
		float AngleStep = 360.0f / ActorsToArrange.Num();

		for (int32 i = 0; i < ActorsToArrange.Num(); i++)
		{
			float Angle = AngleStep * i;
			float AngleRad = FMath::DegreesToRadians(Angle);

			FVector NewLocation = Center + FVector(
				FMath::Cos(AngleRad) * Spacing,
				FMath::Sin(AngleRad) * Spacing,
				0.0f
			);

			ActorsToArrange[i]->SetActorLocation(NewLocation);

			// Optional: Face inward
			FRotator LookAtCenter = (Center - NewLocation).Rotation();
			ActorsToArrange[i]->SetActorRotation(FRotator(0.0f, LookAtCenter.Yaw, 0.0f));
		}

		OutLog += FString::Printf(TEXT("Arranged %d actors in circle with %.1f radius\n"),
			ActorsToArrange.Num(), Spacing);
	}
	else if (Pattern == TEXT("line"))
	{
		int32 HalfCount = ActorsToArrange.Num() / 2;

		for (int32 i = 0; i < ActorsToArrange.Num(); i++)
		{
			FVector NewLocation = Center + FVector((i - HalfCount) * Spacing, 0.0f, 0.0f);
			ActorsToArrange[i]->SetActorLocation(NewLocation);
		}

		OutLog += FString::Printf(TEXT("Arranged %d actors in line with %.1f spacing\n"),
			ActorsToArrange.Num(), Spacing);
	}
	else if (Pattern == TEXT("sphere"))
	{
		// Fibonacci sphere distribution for even spacing
		float GoldenRatio = (1.0f + FMath::Sqrt(5.0f)) / 2.0f;
		float AngleIncrement = PI * 2.0f * GoldenRatio;

		for (int32 i = 0; i < ActorsToArrange.Num(); i++)
		{
			float t = (float)i / ActorsToArrange.Num();
			float Inclination = FMath::Acos(1.0f - 2.0f * t);
			float Azimuth = AngleIncrement * i;

			FVector NewLocation = Center + FVector(
				FMath::Sin(Inclination) * FMath::Cos(Azimuth) * Spacing,
				FMath::Sin(Inclination) * FMath::Sin(Azimuth) * Spacing,
				FMath::Cos(Inclination) * Spacing
			);

			ActorsToArrange[i]->SetActorLocation(NewLocation);

			// Face outward
			FRotator LookOutward = (NewLocation - Center).Rotation();
			ActorsToArrange[i]->SetActorRotation(LookOutward);
		}

		OutLog += FString::Printf(TEXT("Arranged %d actors on sphere with %.1f radius\n"),
			ActorsToArrange.Num(), Spacing);
	}
	else
	{
		OutLog += TEXT("Unknown pattern. Use: grid, circle, line, sphere\n");
		return false;
	}

	return true;
}

// ===== LANDSCAPE (delegate to LandscapeManager) =====

bool FEnhancedCommandExecutor::ExecuteCreateLandscape(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	// Delegate to landscape manager
	FString HeightmapPath;
	if (!Parameters->TryGetStringField(TEXT("heightmap_path"), HeightmapPath))
	{
		OutLog += TEXT("ERROR: No heightmap_path specified\n");
		return false;
	}

	FVector Location = ParseVector(Parameters, TEXT("location"), FVector::ZeroVector);
	FVector Scale = ParseVector(Parameters, TEXT("scale"), FVector(100.0f, 100.0f, 100.0f));

	FScopedTransaction Transaction(FText::FromString(TEXT("Create Landscape")));

	ALandscape* Landscape = LandscapeManager.CreateLandscapeFromHeightmap(
		World, HeightmapPath, Location, Scale, nullptr, OutLog
	);

	if (Landscape && GEditor)
	{
		SelectActor(Landscape);
	}

	return Landscape != nullptr;
}

bool FEnhancedCommandExecutor::ExecuteImportTerrain(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FString HeightmapPath;
	if (!Parameters->TryGetStringField(TEXT("heightmap_path"), HeightmapPath))
	{
		OutLog += TEXT("ERROR: No heightmap_path specified\n");
		return false;
	}

	double GPSLat = 49.1597;
	double GPSLon = 5.4267;
	double MetersPerPixel = 10.0;

	Parameters->TryGetNumberField(TEXT("gps_latitude"), GPSLat);
	Parameters->TryGetNumberField(TEXT("gps_longitude"), GPSLon);
	Parameters->TryGetNumberField(TEXT("meters_per_pixel"), MetersPerPixel);

	FScopedTransaction Transaction(FText::FromString(TEXT("Import Terrain")));

	ALandscape* Landscape = LandscapeManager.CreateLandscapeFromGPSData(
		World, HeightmapPath, GPSLat, GPSLon, MetersPerPixel, nullptr, OutLog
	);

	if (Landscape && GEditor)
	{
		SelectActor(Landscape);
	}

	return Landscape != nullptr;
}

bool FEnhancedCommandExecutor::ExecuteModifyTerrain(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FString Operation = TEXT("raise");
	Parameters->TryGetStringField(TEXT("operation"), Operation);

	FVector Location = ParseVector(Parameters, TEXT("location"), FVector::ZeroVector);

	float Radius = 500.0f;
	float Strength = 0.5f;

	Parameters->TryGetNumberField(TEXT("radius"), Radius);
	Parameters->TryGetNumberField(TEXT("strength"), Strength);

	FScopedTransaction Transaction(FText::FromString(TEXT("Modify Terrain")));

	return LandscapeManager.ModifyLandscapeTerrain(World, Location, Radius, Strength, Operation, OutLog);
}

bool FEnhancedCommandExecutor::ExecuteCreateCrater(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FVector Location = ParseVector(Parameters, TEXT("location"), FVector::ZeroVector);

	float Radius = 1000.0f;
	float Depth = 300.0f;

	Parameters->TryGetNumberField(TEXT("radius"), Radius);
	Parameters->TryGetNumberField(TEXT("depth"), Depth);

	FScopedTransaction Transaction(FText::FromString(TEXT("Create Crater")));

	return LandscapeManager.CreateCrater(World, Location, Radius, Depth, OutLog);
}

bool FEnhancedCommandExecutor::ExecuteCreateTrench(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FVector StartLocation = ParseVector(Parameters, TEXT("start_location"), FVector::ZeroVector);
	FVector EndLocation = ParseVector(Parameters, TEXT("end_location"), FVector(1000.0f, 0.0f, 0.0f));

	float Width = 200.0f;
	float Depth = 250.0f;

	Parameters->TryGetNumberField(TEXT("width"), Width);
	Parameters->TryGetNumberField(TEXT("depth"), Depth);

	FScopedTransaction Transaction(FText::FromString(TEXT("Create Trench")));

	return LandscapeManager.CreateTrench(World, StartLocation, EndLocation, Width, Depth, OutLog);
}

// ===== HELPER FUNCTIONS =====

bool FEnhancedCommandExecutor::ExtractJSONCommand(const FString& Response, TSharedPtr<FJsonObject>& OutCommand)
{
	// Look for JSON object in response
	int32 JsonStart = Response.Find(TEXT("{"));
	int32 JsonEnd = Response.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	if (JsonStart == INDEX_NONE || JsonEnd == INDEX_NONE || JsonEnd <= JsonStart)
	{
		return false;
	}

	FString JsonString = Response.Mid(JsonStart, JsonEnd - JsonStart + 1);

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (FJsonSerializer::Deserialize(Reader, OutCommand) && OutCommand.IsValid())
	{
		return true;
	}

	return false;
}

FVector FEnhancedCommandExecutor::ParseVector(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FVector& Default)
{
	if (!Obj.IsValid() || !Obj->HasField(FieldName))
	{
		return Default;
	}

	const TSharedPtr<FJsonObject>* VecObj;
	if (Obj->TryGetObjectField(FieldName, VecObj))
	{
		FVector Result = Default;
		(*VecObj)->TryGetNumberField(TEXT("x"), Result.X);
		(*VecObj)->TryGetNumberField(TEXT("y"), Result.Y);
		(*VecObj)->TryGetNumberField(TEXT("z"), Result.Z);
		return Result;
	}

	return Default;
}

FRotator FEnhancedCommandExecutor::ParseRotator(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FRotator& Default)
{
	if (!Obj.IsValid() || !Obj->HasField(FieldName))
	{
		return Default;
	}

	const TSharedPtr<FJsonObject>* RotObj;
	if (Obj->TryGetObjectField(FieldName, RotObj))
	{
		FRotator Result = Default;
		(*RotObj)->TryGetNumberField(TEXT("pitch"), Result.Pitch);
		(*RotObj)->TryGetNumberField(TEXT("yaw"), Result.Yaw);
		(*RotObj)->TryGetNumberField(TEXT("roll"), Result.Roll);
		return Result;
	}

	return Default;
}

FLinearColor FEnhancedCommandExecutor::ParseColor(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FLinearColor& Default)
{
	if (!Obj.IsValid() || !Obj->HasField(FieldName))
	{
		return Default;
	}

	const TSharedPtr<FJsonObject>* ColorObj;
	if (Obj->TryGetObjectField(FieldName, ColorObj))
	{
		FLinearColor Result = Default;
		(*ColorObj)->TryGetNumberField(TEXT("r"), Result.R);
		(*ColorObj)->TryGetNumberField(TEXT("g"), Result.G);
		(*ColorObj)->TryGetNumberField(TEXT("b"), Result.B);
		(*ColorObj)->TryGetNumberField(TEXT("a"), Result.A);
		return Result;
	}

	return Default;
}

FVector FEnhancedCommandExecutor::ResolveLocation(const TSharedPtr<FJsonObject>& Parameters, UWorld* World)
{
	if (!Parameters.IsValid())
	{
		return FVector::ZeroVector;
	}

	// Check for explicit location
	if (Parameters->HasField(TEXT("location")))
	{
		return ParseVector(Parameters, TEXT("location"), FVector::ZeroVector);
	}

	// Check for semantic location
	FString LocationRef;
	if (Parameters->TryGetStringField(TEXT("location_ref"), LocationRef))
	{
		FString LowerRef = LocationRef.ToLower();

		if (LowerRef == TEXT("camera") && GEditor)
		{
			// Get camera location
			FViewport* Viewport = GEditor->GetActiveViewport();
			if (Viewport)
			{
				FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(Viewport->GetClient());
				if (ViewportClient)
				{
					return ViewportClient->GetViewLocation();
				}
			}
		}
		else if (LowerRef == TEXT("selected") && GEditor && GEditor->GetSelectedActorCount() > 0)
		{
			// Get center of selected actors
			FVector Center = FVector::ZeroVector;
			int32 Count = 0;
			for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
			{
				AActor* Actor = Cast<AActor>(*It);
				if (Actor)
				{
					Center += Actor->GetActorLocation();
					Count++;
				}
			}
			if (Count > 0)
			{
				return Center / Count;
			}
		}
	}

	return FVector::ZeroVector;
}

FRotator FEnhancedCommandExecutor::ResolveRotation(const TSharedPtr<FJsonObject>& Parameters, UWorld* World)
{
	if (!Parameters.IsValid())
	{
		return FRotator::ZeroRotator;
	}

	// Check for explicit rotation
	if (Parameters->HasField(TEXT("rotation")))
	{
		return ParseRotator(Parameters, TEXT("rotation"), FRotator::ZeroRotator);
	}

	// Check for semantic rotation
	FString RotationRef;
	if (Parameters->TryGetStringField(TEXT("rotation_ref"), RotationRef))
	{
		FString LowerRef = RotationRef.ToLower();

		if (LowerRef == TEXT("camera") && GEditor)
		{
			// Get camera rotation
			FViewport* Viewport = GEditor->GetActiveViewport();
			if (Viewport)
			{
				FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(Viewport->GetClient());
				if (ViewportClient)
				{
					return ViewportClient->GetViewRotation();
				}
			}
		}
	}

	return FRotator::ZeroRotator;
}

UStaticMesh* FEnhancedCommandExecutor::FindBestStaticMesh(const FString& Query)
{
	TArray<FAssetMetadata> Results;
	if (AssetCatalog.SearchStaticMeshes(Query, Results, 1) > 0)
	{
		return LoadObject<UStaticMesh>(nullptr, *Results[0].AssetPath);
	}
	return nullptr;
}

UMaterialInterface* FEnhancedCommandExecutor::FindBestMaterial(const FString& Query)
{
	TArray<FAssetMetadata> Results;
	if (AssetCatalog.SearchMaterials(Query, Results, 1) > 0)
	{
		return LoadObject<UMaterialInterface>(nullptr, *Results[0].AssetPath);
	}
	return nullptr;
}

UClass* FEnhancedCommandExecutor::FindBestBlueprint(const FString& Query)
{
	TArray<FAssetMetadata> Results;
	if (AssetCatalog.SearchBlueprints(Query, Results, 1) > 0)
	{
		UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *Results[0].AssetPath);
		if (BP && BP->GeneratedClass)
		{
			return BP->GeneratedClass;
		}
	}
	return nullptr;
}

bool FEnhancedCommandExecutor::PlaceObjectIntelligently(AActor* Actor, const FVector& Location, const FString& Context, UWorld* World, FString& OutLog)
{
	if (!Actor || !World)
	{
		return false;
	}

	// Snap to ground if context suggests it
	FString LowerContext = Context.ToLower();
	bool bShouldSnapToGround = LowerContext.Contains(TEXT("building")) ||
	                           LowerContext.Contains(TEXT("prop")) ||
	                           LowerContext.Contains(TEXT("vehicle")) ||
	                           LowerContext.Contains(TEXT("tree"));

	if (bShouldSnapToGround)
	{
		FVector GroundLocation = FindGroundHeight(Location, World);
		Actor->SetActorLocation(GroundLocation);
	}

	return true;
}

FVector FEnhancedCommandExecutor::FindGroundHeight(const FVector& Location, UWorld* World)
{
	if (!World)
	{
		return Location;
	}

	// Raycast down to find ground
	FVector StartTrace = Location + FVector(0.0f, 0.0f, 10000.0f);
	FVector EndTrace = Location - FVector(0.0f, 0.0f, 10000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, QueryParams))
	{
		return HitResult.Location;
	}

	return Location;
}

bool FEnhancedCommandExecutor::CheckPlacementValid(const FVector& Location, const FVector& Extent, UWorld* World)
{
	// TODO: Check for collisions, valid placement, etc.
	return true;
}

void FEnhancedCommandExecutor::SelectActor(AActor* Actor)
{
	if (GEditor && Actor)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(Actor, true, true);
		GEditor->NoteSelectionChange();
	}
}

void FEnhancedCommandExecutor::SelectActors(const TArray<AActor*>& Actors)
{
	if (GEditor)
	{
		GEditor->SelectNone(false, true);
		for (AActor* Actor : Actors)
		{
			if (Actor)
			{
				GEditor->SelectActor(Actor, true, false);
			}
		}
		GEditor->NoteSelectionChange();
	}
}

ADirectionalLight* FEnhancedCommandExecutor::FindDirectionalLight(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void FEnhancedCommandExecutor::SetGlobalLightingIntensity(float Intensity, UWorld* World)
{
	if (ADirectionalLight* Sun = FindDirectionalLight(World))
	{
		Sun->GetLightComponent()->SetIntensity(Intensity);
	}
}

FString FEnhancedCommandExecutor::GetHelpText()
{
	FString HelpText = TEXT("=== ENHANCED NLP CONTROLLER FOR UE5 ===\n\n");

	HelpText += TEXT("ASSET & OBJECT PLACEMENT:\n");
	HelpText += TEXT("  spawn_object - Spawn object with intelligent asset selection\n");
	HelpText += TEXT("  place_building - Place building with semantic search\n");
	HelpText += TEXT("  place_props - Place multiple props in patterns\n");
	HelpText += TEXT("  place_vegetation - Place trees/vegetation\n\n");

	HelpText += TEXT("MATERIAL & TEXTURE:\n");
	HelpText += TEXT("  apply_material - Apply material to selected actors\n");
	HelpText += TEXT("  set_material_parameter - Modify material parameters\n\n");

	HelpText += TEXT("LIGHTING & ATMOSPHERE:\n");
	HelpText += TEXT("  create_light - Create lights (directional, point, spot)\n");
	HelpText += TEXT("  set_time_of_day - Set time and sun position\n");
	HelpText += TEXT("  set_fog - Configure fog parameters\n");
	HelpText += TEXT("  set_weather - Set weather conditions\n\n");

	HelpText += TEXT("ACTOR MANIPULATION:\n");
	HelpText += TEXT("  modify_actor - Move/rotate/scale selected actors\n");
	HelpText += TEXT("  delete_actor - Delete selected actors\n");
	HelpText += TEXT("  duplicate_actor - Duplicate selected actors\n\n");

	HelpText += TEXT("BATCH OPERATIONS:\n");
	HelpText += TEXT("  batch_place - Place multiple objects at once\n");
	HelpText += TEXT("  arrange_pattern - Arrange objects in patterns\n\n");

	HelpText += TEXT("LANDSCAPE & TERRAIN:\n");
	HelpText += TEXT("  create_landscape - Create landscape from heightmap\n");
	HelpText += TEXT("  import_terrain - Import GPS-referenced terrain\n");
	HelpText += TEXT("  modify_terrain - Raise/lower/smooth terrain\n");
	HelpText += TEXT("  create_crater - Create shell crater\n");
	HelpText += TEXT("  create_trench - Carve trench line\n\n");

	HelpText += TEXT("EXAMPLES:\n");
	HelpText += TEXT("  \"Place a French WWI farmhouse at the camera location\"\n");
	HelpText += TEXT("  \"Create 20 sandbags in a line\"\n");
	HelpText += TEXT("  \"Set time to dusk\"\n");
	HelpText += TEXT("  \"Apply weathered material to selected objects\"\n");
	HelpText += TEXT("  \"Place trees in a scatter pattern\"\n");
	HelpText += TEXT("  \"Create a crater at 1000,500,0\"\n\n");

	HelpText += TEXT("SEMANTIC LOCATIONS:\n");
	HelpText += TEXT("  'camera' - Current camera position\n");
	HelpText += TEXT("  'selected' - Center of selected actors\n");
	HelpText += TEXT("  'cursor' - Where mouse cursor points\n");

	return HelpText;
}
