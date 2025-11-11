// Copyright Only The Dead Development Team. All Rights Reserved.

#include "LevelInspector.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Light.h"
#include "Landscape.h"
#include "Components/LightComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Selection.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

ULevelInspector::ULevelInspector()
{
}

UWorld* ULevelInspector::GetEditorWorld()
{
	if (GEditor)
	{
		return GEditor->GetEditorWorldContext().World();
	}
	return nullptr;
}

FString ULevelInspector::GetLevelSummaryText(UWorld* World)
{
	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return TEXT("No level loaded");
	}

	FLevelSummary Summary = GetLevelSummary(World);

	FString Result = FString::Printf(
		TEXT("Level Summary:\n")
		TEXT("- Total Actors: %d\n")
		TEXT("- Static Meshes: %d\n")
		TEXT("- Lights: %d\n")
		TEXT("- Landscapes: %d\n")
		TEXT("- Selected: %d\n")
		TEXT("- Bounds: %s to %s\n\n")
		TEXT("Actors by Type:\n"),
		Summary.TotalActors,
		Summary.StaticMeshActors,
		Summary.Lights,
		Summary.Landscapes,
		Summary.SelectedActorCount,
		*FormatLocation(Summary.LevelBoundsMin),
		*FormatLocation(Summary.LevelBoundsMax)
	);

	for (const auto& Pair : Summary.ActorCountByType)
	{
		Result += FString::Printf(TEXT("- %s: %d\n"), *Pair.Key, Pair.Value);
	}

	return Result;
}

FLevelSummary ULevelInspector::GetLevelSummary(UWorld* World)
{
	FLevelSummary Summary;

	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return Summary;
	}

	// Count actors by type
	FVector MinBounds = FVector(MAX_flt);
	FVector MaxBounds = FVector(-MAX_flt);

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsA<AWorldSettings>())
		{
			continue;
		}

		Summary.TotalActors++;

		// Update bounds
		FVector Location = Actor->GetActorLocation();
		MinBounds = MinBounds.ComponentMin(Location);
		MaxBounds = MaxBounds.ComponentMax(Location);

		// Count by type
		FString TypeName = GetActorTypeString(Actor);
		if (!Summary.ActorCountByType.Contains(TypeName))
		{
			Summary.ActorCountByType.Add(TypeName, 0);
		}
		Summary.ActorCountByType[TypeName]++;

		// Specific types
		if (Actor->IsA<AStaticMeshActor>())
		{
			Summary.StaticMeshActors++;
		}
		else if (Actor->IsA<ALight>())
		{
			Summary.Lights++;
		}
		else if (Actor->IsA<ALandscape>())
		{
			Summary.Landscapes++;
		}
	}

	Summary.LevelBoundsMin = MinBounds;
	Summary.LevelBoundsMax = MaxBounds;
	Summary.SelectedActorCount = GetSelectedActors().Num();

	return Summary;
}

TArray<AActor*> ULevelInspector::GetAllActors(UWorld* World)
{
	TArray<AActor*> Actors;

	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return Actors;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (*It && !(*It)->IsA<AWorldSettings>())
		{
			Actors.Add(*It);
		}
	}

	return Actors;
}

TArray<AActor*> ULevelInspector::FindActorsByName(UWorld* World, const FString& NamePattern)
{
	TArray<AActor*> MatchingActors;

	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return MatchingActors;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsA<AWorldSettings>())
		{
			continue;
		}

		FString ActorName = Actor->GetActorLabel();
		if (MatchesWildcard(ActorName, NamePattern))
		{
			MatchingActors.Add(Actor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Found %d actors matching pattern '%s'"), MatchingActors.Num(), *NamePattern);

	return MatchingActors;
}

TArray<AActor*> ULevelInspector::FindActorsByClass(UWorld* World, UClass* ActorClass)
{
	TArray<AActor*> MatchingActors;

	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World || !ActorClass)
	{
		return MatchingActors;
	}

	for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
	{
		MatchingActors.Add(*It);
	}

	return MatchingActors;
}

TArray<AActor*> ULevelInspector::FindActorsByTag(UWorld* World, const FName& Tag)
{
	TArray<AActor*> MatchingActors;

	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return MatchingActors;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor->Tags.Contains(Tag))
		{
			MatchingActors.Add(Actor);
		}
	}

	return MatchingActors;
}

TArray<AActor*> ULevelInspector::GetSelectedActors()
{
	TArray<AActor*> SelectedActors;

	if (GEditor)
	{
		USelection* Selection = GEditor->GetSelectedActors();
		if (Selection)
		{
			Selection->GetSelectedObjects<AActor>(SelectedActors);
		}
	}

	return SelectedActors;
}

TArray<AActor*> ULevelInspector::GetActorsInRadius(UWorld* World, const FVector& Center, float Radius)
{
	TArray<AActor*> ActorsInRadius;

	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return ActorsInRadius;
	}

	float RadiusSquared = Radius * Radius;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsA<AWorldSettings>())
		{
			continue;
		}

		float DistanceSquared = FVector::DistSquared(Actor->GetActorLocation(), Center);
		if (DistanceSquared <= RadiusSquared)
		{
			ActorsInRadius.Add(Actor);
		}
	}

	return ActorsInRadius;
}

TArray<AActor*> ULevelInspector::GetActorsInBox(UWorld* World, const FVector& BoxMin, const FVector& BoxMax)
{
	TArray<AActor*> ActorsInBox;

	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return ActorsInBox;
	}

	FBox Box(BoxMin, BoxMax);

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsA<AWorldSettings>())
		{
			continue;
		}

		if (Box.IsInside(Actor->GetActorLocation()))
		{
			ActorsInBox.Add(Actor);
		}
	}

	return ActorsInBox;
}

FActorInspectionData ULevelInspector::InspectActor(AActor* Actor)
{
	FActorInspectionData Data;

	if (!Actor)
	{
		return Data;
	}

	Data.Name = Actor->GetActorLabel();
	Data.Class = Actor->GetClass()->GetName();
	Data.Location = Actor->GetActorLocation();
	Data.Rotation = Actor->GetActorRotation();
	Data.Scale = Actor->GetActorScale3D();

	// Tags
	for (const FName& Tag : Actor->Tags)
	{
		Data.Tags.Add(Tag.ToString());
	}

	// Components
	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component)
		{
			Data.Components.Add(Component->GetName());
		}
	}

	return Data;
}

FString ULevelInspector::GetActorDescription(AActor* Actor)
{
	if (!Actor)
	{
		return TEXT("null");
	}

	FActorInspectionData Data = InspectActor(Actor);

	FString Description = FString::Printf(
		TEXT("%s '%s' at %s"),
		*Data.Class,
		*Data.Name,
		*FormatLocation(Data.Location)
	);

	if (Data.Components.Num() > 0)
	{
		Description += FString::Printf(TEXT(" with %d components"), Data.Components.Num());
	}

	if (Data.Tags.Num() > 0)
	{
		Description += TEXT(" [Tags: ");
		for (int32 i = 0; i < Data.Tags.Num(); i++)
		{
			Description += Data.Tags[i];
			if (i < Data.Tags.Num() - 1)
			{
				Description += TEXT(", ");
			}
		}
		Description += TEXT("]");
	}

	return Description;
}

bool ULevelInspector::ActorMatchesDescription(AActor* Actor, const FString& Description)
{
	if (!Actor)
	{
		return false;
	}

	FString ActorDesc = GetActorDescription(Actor).ToLower();
	FString SearchDesc = Description.ToLower();

	// Simple substring match - could be improved with NLP
	return ActorDesc.Contains(SearchDesc);
}

TArray<FString> ULevelInspector::GetAvailableAssets(const FString& AssetType, const FString& PathFilter)
{
	TArray<FString> Assets;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Build filter
	FARFilter Filter;
	if (!AssetType.IsEmpty())
	{
		Filter.ClassPaths.Add(FTopLevelAssetPath(AssetType));
	}
	if (!PathFilter.IsEmpty())
	{
		Filter.PackagePaths.Add(FName(*PathFilter));
		Filter.bRecursivePaths = true;
	}

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		Assets.Add(AssetData.GetSoftObjectPath().ToString());
	}

	return Assets;
}

FString ULevelInspector::FindAssetPath(const FString& AssetName, const FString& AssetType)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	if (!AssetType.IsEmpty())
	{
		Filter.ClassPaths.Add(FTopLevelAssetPath(AssetType));
	}

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (AssetData.AssetName.ToString().Contains(AssetName))
		{
			return AssetData.GetSoftObjectPath().ToString();
		}
	}

	return FString();
}

FString ULevelInspector::GetLandscapeInfo(UWorld* World)
{
	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return TEXT("No world");
	}

	FString Info = TEXT("Landscapes:\n");
	int32 Count = 0;

	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		ALandscape* Landscape = *It;
		Count++;

		FVector Location = Landscape->GetActorLocation();
		FIntRect Bounds = Landscape->GetBoundingRect();

		Info += FString::Printf(
			TEXT("- Landscape %d at %s, Size: %dx%d\n"),
			Count,
			*FormatLocation(Location),
			Bounds.Width(),
			Bounds.Height()
		);
	}

	if (Count == 0)
	{
		Info += TEXT("- None\n");
	}

	return Info;
}

TArray<FString> ULevelInspector::GetMaterialsInLevel(UWorld* World)
{
	TArray<FString> Materials;
	TSet<FString> UniqueMaterials;

	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return Materials;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		TArray<UMeshComponent*> MeshComponents;
		Actor->GetComponents<UMeshComponent>(MeshComponents);

		for (UMeshComponent* MeshComp : MeshComponents)
		{
			if (!MeshComp)
			{
				continue;
			}

			for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
			{
				UMaterialInterface* Material = MeshComp->GetMaterial(i);
				if (Material)
				{
					UniqueM.Add(Material->GetPathName());
				}
			}
		}
	}

	Materials = UniqueM.Array();
	return Materials;
}

FString ULevelInspector::GetLightingInfo(UWorld* World)
{
	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return TEXT("No world");
	}

	FString Info = TEXT("Lighting:\n");
	int32 DirectionalLights = 0;
	int32 PointLights = 0;
	int32 SpotLights = 0;

	for (TActorIterator<ALight> It(World); It; ++It)
	{
		ALight* Light = *It;
		if (!Light)
		{
			continue;
		}

		FString LightType = Light->GetClass()->GetName();
		if (LightType.Contains(TEXT("Directional")))
		{
			DirectionalLights++;
		}
		else if (LightType.Contains(TEXT("Point")))
		{
			PointLights++;
		}
		else if (LightType.Contains(TEXT("Spot")))
		{
			SpotLights++;
		}
	}

	Info += FString::Printf(
		TEXT("- Directional Lights: %d\n")
		TEXT("- Point Lights: %d\n")
		TEXT("- Spot Lights: %d\n"),
		DirectionalLights,
		PointLights,
		SpotLights
	);

	return Info;
}

FString ULevelInspector::GenerateAIContextString(UWorld* World)
{
	if (!World)
	{
		World = GetEditorWorld();
	}

	if (!World)
	{
		return TEXT("ERROR: No world available");
	}

	FString Context = TEXT("=== CURRENT LEVEL STATE ===\n\n");

	// Summary
	Context += GetLevelSummaryText(World);
	Context += TEXT("\n");

	// Landscape
	Context += GetLandscapeInfo(World);
	Context += TEXT("\n");

	// Lighting
	Context += GetLightingInfo(World);
	Context += TEXT("\n");

	// Selected actors
	TArray<AActor*> Selected = GetSelectedActors();
	if (Selected.Num() > 0)
	{
		Context += TEXT("Selected Actors:\n");
		for (AActor* Actor : Selected)
		{
			Context += TEXT("- ") + GetActorDescription(Actor) + TEXT("\n");
		}
		Context += TEXT("\n");
	}

	// Recent actors (for context)
	TArray<AActor*> AllActors = GetAllActors(World);
	if (AllActors.Num() > 0)
	{
		Context += TEXT("Recent Actors (last 10):\n");
		int32 Count = FMath::Min(10, AllActors.Num());
		for (int32 i = AllActors.Num() - Count; i < AllActors.Num(); i++)
		{
			Context += TEXT("- ") + GetActorDescription(AllActors[i]) + TEXT("\n");
		}
	}

	Context += TEXT("\n=== END LEVEL STATE ===");

	return Context;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

bool ULevelInspector::MatchesWildcard(const FString& String, const FString& Pattern)
{
	if (Pattern.Contains(TEXT("*")))
	{
		TArray<FString> Parts;
		Pattern.ParseIntoArray(Parts, TEXT("*"));

		int32 SearchPos = 0;
		for (const FString& Part : Parts)
		{
			if (Part.IsEmpty())
			{
				continue;
			}

			int32 FoundPos = String.Find(Part, ESearchCase::IgnoreCase, ESearchDir::FromStart, SearchPos);
			if (FoundPos == INDEX_NONE)
			{
				return false;
			}
			SearchPos = FoundPos + Part.Len();
		}

		return true;
	}
	else
	{
		return String.Equals(Pattern, ESearchCase::IgnoreCase);
	}
}

FString ULevelInspector::GetActorTypeString(AActor* Actor)
{
	if (!Actor)
	{
		return TEXT("Unknown");
	}

	FString ClassName = Actor->GetClass()->GetName();

	// Simplify common types
	if (ClassName.Contains(TEXT("StaticMesh")))
	{
		return TEXT("StaticMesh");
	}
	else if (ClassName.Contains(TEXT("Light")))
	{
		return TEXT("Light");
	}
	else if (ClassName.Contains(TEXT("Landscape")))
	{
		return TEXT("Landscape");
	}
	else if (ClassName.Contains(TEXT("Camera")))
	{
		return TEXT("Camera");
	}
	else if (ClassName.Contains(TEXT("Player")))
	{
		return TEXT("PlayerStart");
	}

	return ClassName;
}

FString ULevelInspector::FormatLocation(const FVector& Location)
{
	return FString::Printf(TEXT("(%.0f, %.0f, %.0f)"), Location.X, Location.Y, Location.Z);
}
