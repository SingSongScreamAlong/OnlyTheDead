// Copyright Epic Games, Inc. All Rights Reserved.

#include "DesignSuggestionSystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h"
#include "Engine/StaticMeshActor.h"

bool FDesignSuggestionSystem::AnalyzeScene(
	UWorld* World,
	const TArray<AActor*>& FocusActors,
	FSceneAnalysis& OutAnalysis)
{
	if (!World)
		return false;

	UE_LOG(LogTemp, Log, TEXT("DesignSuggestion: Analyzing scene..."));

	// Analyze lighting
	float LightingScore = AnalyzeLighting(World, OutAnalysis.Suggestions);

	// Analyze composition
	FVector CameraLocation = FVector::ZeroVector; // Would get from active viewport
	float CompositionScore = AnalyzeComposition(World, CameraLocation, OutAnalysis.Suggestions);

	// Detect isolated objects
	int32 IsolatedCount = DetectIsolatedObjects(World, OutAnalysis.Suggestions);

	// Check historical accuracy if applicable
	if (FocusActors.Num() > 0)
	{
		CheckHistoricalAccuracy(FocusActors, TEXT("WWI"), OutAnalysis.Suggestions);
	}

	// Calculate overall score
	OutAnalysis.OverallScore = (LightingScore + CompositionScore) / 2.0f;

	// Generate summary
	if (IsolatedCount > 0)
	{
		OutAnalysis.Issues.Add(FString::Printf(TEXT("%d isolated objects found"), IsolatedCount));
	}

	if (LightingScore < 0.5f)
	{
		OutAnalysis.Issues.Add(TEXT("Lighting needs improvement"));
	}
	else
	{
		OutAnalysis.Strengths.Add(TEXT("Good lighting setup"));
	}

	OutAnalysis.Summary = FString::Printf(
		TEXT("Scene Analysis: Quality %.1f%%. Found %d suggestions."),
		OutAnalysis.OverallScore * 100.0f,
		OutAnalysis.Suggestions.Num());

	UE_LOG(LogTemp, Log, TEXT("DesignSuggestion: Analysis complete - Score: %.2f, Suggestions: %d"),
		OutAnalysis.OverallScore, OutAnalysis.Suggestions.Num());

	return true;
}

int32 FDesignSuggestionSystem::GetSuggestionsForActor(
	AActor* Actor,
	UWorld* World,
	TArray<FDesignSuggestion>& OutSuggestions)
{
	if (!Actor || !World)
		return 0;

	int32 InitialCount = OutSuggestions.Num();

	// Check if actor is isolated
	TArray<AActor*> NearbyActors = FindNearbyActors(Actor, 1000.0f, World);
	float IsolationScore = CalculateIsolationScore(Actor, NearbyActors);

	if (IsolationScore > 0.7f)
	{
		FDesignSuggestion Suggestion;
		Suggestion.Type = ESuggestionType::AddProps;
		Suggestion.Title = TEXT("Object appears isolated");
		Suggestion.Description = FString::Printf(
			TEXT("The %s looks isolated. Consider adding:"), *Actor->GetName());
		Suggestion.Steps.Add(TEXT("- Props around it (crates, barrels, carts)"));
		Suggestion.Steps.Add(TEXT("- Vegetation (trees, bushes)"));
		Suggestion.Steps.Add(TEXT("- Roads connecting to other objects"));
		Suggestion.Priority = 0.7f;
		Suggestion.AffectedActors.Add(Actor);

		OutSuggestions.Add(Suggestion);
	}

	// Check if building needs props
	if (Actor->GetName().Contains(TEXT("Building")) || Actor->GetName().Contains(TEXT("House")))
	{
		SuggestPropsForBuilding(Actor, TEXT("residential"), OutSuggestions);
	}

	return OutSuggestions.Num() - InitialCount;
}

int32 FDesignSuggestionSystem::CheckHistoricalAccuracy(
	const TArray<AActor*>& Actors,
	const FString& Era,
	TArray<FDesignSuggestion>& OutSuggestions)
{
	int32 InitialCount = OutSuggestions.Num();

	UE_LOG(LogTemp, Log, TEXT("DesignSuggestion: Checking %s historical accuracy"), *Era);

	TArray<FString> Rules = GetHistoricalRules(Era);

	// Check for common historical issues
	bool bHasTrenches = false;
	bool bHasSandbags = false;
	bool bHasBarbedWire = false;

	for (AActor* Actor : Actors)
	{
		if (!Actor)
			continue;

		FString ActorName = Actor->GetName().ToLower();

		if (ActorName.Contains(TEXT("trench")))
			bHasTrenches = true;
		if (ActorName.Contains(TEXT("sandbag")))
			bHasSandbags = true;
		if (ActorName.Contains(TEXT("barbed")) || ActorName.Contains(TEXT("wire")))
			bHasBarbedWire = true;
	}

	// Generate historical suggestions
	if (Era == TEXT("WWI") && bHasTrenches)
	{
		if (!bHasSandbags)
		{
			FDesignSuggestion Suggestion;
			Suggestion.Type = ESuggestionType::HistoricalAccuracy;
			Suggestion.Title = TEXT("Missing sandbags");
			Suggestion.Description = TEXT("WWI trenches typically had sandbags for protection.");
			Suggestion.Steps.Add(TEXT("Add sandbags along trench edges"));
			Suggestion.Steps.Add(TEXT("Place every 50-100 units"));
			Suggestion.Priority = 0.8f;
			OutSuggestions.Add(Suggestion);
		}

		if (!bHasBarbedWire)
		{
			FDesignSuggestion Suggestion;
			Suggestion.Type = ESuggestionType::HistoricalAccuracy;
			Suggestion.Title = TEXT("Missing barbed wire");
			Suggestion.Description = TEXT("WWI trenches used barbed wire defenses 50-100m in front.");
			Suggestion.Steps.Add(TEXT("Place barbed wire obstacles"));
			Suggestion.Steps.Add(TEXT("Position 50-100m from trench line"));
			Suggestion.Priority = 0.8f;
			OutSuggestions.Add(Suggestion);
		}
	}

	return OutSuggestions.Num() - InitialCount;
}

float FDesignSuggestionSystem::AnalyzeLighting(
	UWorld* World,
	TArray<FDesignSuggestion>& OutSuggestions)
{
	if (!World)
		return 0.5f;

	int32 DirectionalLights = 0;
	int32 PointLights = 0;
	float AverageBrightness = 0.0f;
	int32 LightCount = 0;

	// Count lights
	for (TActorIterator<ALight> It(World); It; ++It)
	{
		ALight* Light = *It;
		if (Light && Light->GetLightComponent())
		{
			LightCount++;
			AverageBrightness += Light->GetLightComponent()->Intensity;

			if (Light->GetLightComponent()->IsA<UDirectionalLightComponent>())
			{
				DirectionalLights++;
			}
			else
			{
				PointLights++;
			}
		}
	}

	if (LightCount > 0)
	{
		AverageBrightness /= LightCount;
	}

	// Generate suggestions
	if (DirectionalLights == 0)
	{
		FDesignSuggestion Suggestion;
		Suggestion.Type = ESuggestionType::AddLighting;
		Suggestion.Title = TEXT("No directional light");
		Suggestion.Description = TEXT("Scene has no sun/moon. Add a directional light for global illumination.");
		Suggestion.Steps.Add(TEXT("Add Directional Light actor"));
		Suggestion.Steps.Add(TEXT("Set intensity to 3-5 for daytime"));
		Suggestion.Priority = 0.9f;
		Suggestion.bAutoApplicable = true;
		OutSuggestions.Add(Suggestion);
	}

	if (PointLights < 3)
	{
		FDesignSuggestion Suggestion;
		Suggestion.Type = ESuggestionType::AddLighting;
		Suggestion.Title = TEXT("Limited fill lighting");
		Suggestion.Description = TEXT("Consider adding point lights for better illumination.");
		Suggestion.Steps.Add(TEXT("Add point lights in dark areas"));
		Suggestion.Steps.Add(TEXT("Use warm colors for indoor lights"));
		Suggestion.Priority = 0.5f;
		OutSuggestions.Add(Suggestion);
	}

	// Calculate score
	float Score = 0.5f;
	if (DirectionalLights > 0)
		Score += 0.3f;
	if (PointLights >= 3)
		Score += 0.2f;

	return FMath::Clamp(Score, 0.0f, 1.0f);
}

float FDesignSuggestionSystem::AnalyzeComposition(
	UWorld* World,
	const FVector& CameraLocation,
	TArray<FDesignSuggestion>& OutSuggestions)
{
	if (!World)
		return 0.5f;

	// Analyze actor distribution
	float DensityScore = AnalyzeActorDensity(World);

	// Find empty areas
	TArray<FVector> EmptyAreas = FindEmptyAreas(World, 2000.0f);

	if (EmptyAreas.Num() > 2)
	{
		FDesignSuggestion Suggestion;
		Suggestion.Type = ESuggestionType::AddDetails;
		Suggestion.Title = TEXT("Large empty areas detected");
		Suggestion.Description = FString::Printf(
			TEXT("Found %d large empty areas. Consider filling with:"), EmptyAreas.Num());
		Suggestion.Steps.Add(TEXT("- Scattered props"));
		Suggestion.Steps.Add(TEXT("- Vegetation clusters"));
		Suggestion.Steps.Add(TEXT("- Terrain features"));
		Suggestion.Priority = 0.6f;
		OutSuggestions.Add(Suggestion);
	}

	return DensityScore;
}

int32 FDesignSuggestionSystem::DetectIsolatedObjects(
	UWorld* World,
	TArray<FDesignSuggestion>& OutSuggestions)
{
	if (!World)
		return 0;

	TArray<AActor*> AllActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AllActors.Add(*It);
	}

	int32 IsolatedCount = 0;

	for (AActor* Actor : AllActors)
	{
		if (!Actor || !Actor->IsA<AStaticMeshActor>())
			continue;

		TArray<AActor*> NearbyActors = FindNearbyActors(Actor, 1000.0f, World);
		float IsolationScore = CalculateIsolationScore(Actor, NearbyActors);

		if (IsolationScore > 0.7f)
		{
			IsolatedCount++;
			GetSuggestionsForActor(Actor, World, OutSuggestions);
		}
	}

	return IsolatedCount;
}

int32 FDesignSuggestionSystem::SuggestPropsForBuilding(
	AActor* Building,
	const FString& Style,
	TArray<FDesignSuggestion>& OutSuggestions)
{
	if (!Building)
		return 0;

	FDesignSuggestion Suggestion;
	Suggestion.Type = ESuggestionType::AddProps;
	Suggestion.Title = TEXT("Enhance building with props");
	Suggestion.Description = FString::Printf(TEXT("Add %s props around this building:"), *Style);

	if (Style == TEXT("military"))
	{
		Suggestion.Steps.Add(TEXT("- Sandbags at entrances"));
		Suggestion.Steps.Add(TEXT("- Ammo crates"));
		Suggestion.Steps.Add(TEXT("- Barbed wire fencing"));
		Suggestion.Steps.Add(TEXT("- Watchtower nearby"));
	}
	else if (Style == TEXT("residential"))
	{
		Suggestion.Steps.Add(TEXT("- Crates and barrels"));
		Suggestion.Steps.Add(TEXT("- Carts or wagons"));
		Suggestion.Steps.Add(TEXT("- Garden vegetation"));
		Suggestion.Steps.Add(TEXT("- Fence around property"));
	}
	else
	{
		Suggestion.Steps.Add(TEXT("- Generic props"));
		Suggestion.Steps.Add(TEXT("- Vegetation"));
		Suggestion.Steps.Add(TEXT("- Lighting fixtures"));
	}

	Suggestion.Priority = 0.6f;
	Suggestion.AffectedActors.Add(Building);

	OutSuggestions.Add(Suggestion);
	return 1;
}

bool FDesignSuggestionSystem::ApplySuggestion(
	const FDesignSuggestion& Suggestion,
	UWorld* World)
{
	if (!World || !Suggestion.bAutoApplicable)
		return false;

	UE_LOG(LogTemp, Log, TEXT("DesignSuggestion: Auto-applying suggestion: %s"), *Suggestion.Title);

	// Auto-apply based on type
	switch (Suggestion.Type)
	{
	case ESuggestionType::AddLighting:
		// Would spawn directional light here
		return true;

	case ESuggestionType::AddProps:
		// Would spawn props around affected actors
		return true;

	default:
		return false;
	}
}

TArray<FString> FDesignSuggestionSystem::GetHistoricalRules(const FString& Era)
{
	TArray<FString> Rules;

	if (Era == TEXT("WWI"))
	{
		Rules.Add(TEXT("Trenches should have zigzag pattern to prevent enfilade fire"));
		Rules.Add(TEXT("Sandbags placed every 50m along parapet"));
		Rules.Add(TEXT("Barbed wire 50-100m in front of trench line"));
		Rules.Add(TEXT("Dugouts every 50m for shelter"));
		Rules.Add(TEXT("Fire steps for infantry"));
		Rules.Add(TEXT("Communication trenches to rear areas"));
	}
	else if (Era == TEXT("Medieval"))
	{
		Rules.Add(TEXT("Villages centered around church or manor"));
		Rules.Add(TEXT("Organic street layout (no grid)"));
		Rules.Add(TEXT("Wells or water source central"));
		Rules.Add(TEXT("Defensive walls for fortified settlements"));
		Rules.Add(TEXT("Market square for trading"));
	}

	return Rules;
}

float FDesignSuggestionSystem::AnalyzeActorDensity(UWorld* World)
{
	if (!World)
		return 0.5f;

	int32 ActorCount = 0;
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		ActorCount++;
	}

	// Normalize to 0.0-1.0 (assuming 100+ actors is good density)
	float Score = FMath::Min(ActorCount / 100.0f, 1.0f);
	return Score;
}

TArray<FVector> FDesignSuggestionSystem::FindEmptyAreas(UWorld* World, float MinSize)
{
	TArray<FVector> EmptyAreas;

	// Simplified empty area detection
	// Full implementation would use spatial partitioning

	return EmptyAreas;
}

float FDesignSuggestionSystem::CalculateIsolationScore(AActor* Actor, const TArray<AActor*>& NearbyActors)
{
	if (!Actor)
		return 0.0f;

	// More isolated = higher score
	int32 NearbyCount = NearbyActors.Num();

	if (NearbyCount == 0)
		return 1.0f;
	else if (NearbyCount == 1)
		return 0.8f;
	else if (NearbyCount == 2)
		return 0.6f;
	else if (NearbyCount <= 5)
		return 0.3f;
	else
		return 0.0f;
}

TArray<AActor*> FDesignSuggestionSystem::FindNearbyActors(AActor* CenterActor, float Radius, UWorld* World)
{
	TArray<AActor*> NearbyActors;

	if (!CenterActor || !World)
		return NearbyActors;

	FVector Center = CenterActor->GetActorLocation();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor != CenterActor)
		{
			float Distance = FVector::Dist(Center, Actor->GetActorLocation());
			if (Distance <= Radius)
			{
				NearbyActors.Add(Actor);
			}
		}
	}

	return NearbyActors;
}

FVector FDesignSuggestionSystem::FindVegetationLocation(AActor* NearActor, UWorld* World)
{
	if (!NearActor)
		return FVector::ZeroVector;

	// Find location near actor for vegetation
	FVector BaseLocation = NearActor->GetActorLocation();
	FVector Offset = FMath::RandPointInCircle(500.0f);

	return BaseLocation + FVector(Offset, 0);
}

bool FDesignSuggestionSystem::ValidateSuggestion(const FDesignSuggestion& Suggestion, UWorld* World)
{
	if (!World)
		return false;

	// Check if affected actors still exist
	for (AActor* Actor : Suggestion.AffectedActors)
	{
		if (!Actor || !Actor->IsValidLowLevel())
			return false;
	}

	return true;
}
