// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * Suggestion type
 */
UENUM()
enum class ESuggestionType : uint8
{
	AddProps,              // Add props around building
	AddVegetation,         // Add trees/bushes
	AddLighting,           // Improve lighting
	AddDefenses,           // Add defensive structures
	AddRoads,              // Connect with roads
	FixComposition,        // Improve scene composition
	HistoricalAccuracy,    // Historical suggestions
	ImproveRealism,        // Realism improvements
	AddDetails,            // Add detail objects
	ColorCorrection        // Color/material suggestions
};

/**
 * Design suggestion
 */
struct FDesignSuggestion
{
	ESuggestionType Type;
	FString Title;
	FString Description;
	float Priority = 0.5f;           // 0.0-1.0
	TArray<FString> Steps;           // Implementation steps
	TArray<AActor*> AffectedActors;  // Actors this suggestion relates to
	FVector SuggestedLocation;       // Where to apply suggestion
	bool bAutoApplicable = false;    // Can be auto-applied
};

/**
 * Scene analysis result
 */
struct FSceneAnalysis
{
	TArray<FString> Issues;          // Problems found
	TArray<FString> Strengths;       // Good aspects
	TArray<FDesignSuggestion> Suggestions;
	float OverallScore = 0.5f;       // 0.0-1.0 (scene quality)
	FString Summary;
};

/**
 * AI-Assisted design suggestion system
 * TIER 3.4 Feature - Revolutionary
 */
class CLAUDETERMINALEDITOR_API FDesignSuggestionSystem
{
public:
	/**
	 * Analyze scene and generate suggestions
	 * @param World World to analyze
	 * @param FocusActors Specific actors to analyze (empty = analyze all)
	 * @param OutAnalysis Analysis result with suggestions
	 * @return true if analysis successful
	 */
	static bool AnalyzeScene(
		UWorld* World,
		const TArray<AActor*>& FocusActors,
		FSceneAnalysis& OutAnalysis);

	/**
	 * Get suggestions for specific actor
	 * @param Actor Actor to analyze
	 * @param World World context
	 * @param OutSuggestions Generated suggestions
	 * @return Number of suggestions generated
	 */
	static int32 GetSuggestionsForActor(
		AActor* Actor,
		UWorld* World,
		TArray<FDesignSuggestion>& OutSuggestions);

	/**
	 * Check historical accuracy
	 * @param Actors Actors to check
	 * @param Era Historical era (e.g., "WWI", "Medieval")
	 * @param OutSuggestions Accuracy suggestions
	 * @return Number of issues found
	 */
	static int32 CheckHistoricalAccuracy(
		const TArray<AActor*>& Actors,
		const FString& Era,
		TArray<FDesignSuggestion>& OutSuggestions);

	/**
	 * Analyze lighting quality
	 * @param World World to analyze
	 * @param OutSuggestions Lighting suggestions
	 * @return Lighting quality score (0.0-1.0)
	 */
	static float AnalyzeLighting(
		UWorld* World,
		TArray<FDesignSuggestion>& OutSuggestions);

	/**
	 * Analyze scene composition
	 * @param World World to analyze
	 * @param CameraLocation Viewpoint
	 * @param OutSuggestions Composition suggestions
	 * @return Composition score (0.0-1.0)
	 */
	static float AnalyzeComposition(
		UWorld* World,
		const FVector& CameraLocation,
		TArray<FDesignSuggestion>& OutSuggestions);

	/**
	 * Detect isolated objects
	 * @param World World to analyze
	 * @param OutSuggestions Suggestions for isolated objects
	 * @return Number of isolated objects found
	 */
	static int32 DetectIsolatedObjects(
		UWorld* World,
		TArray<FDesignSuggestion>& OutSuggestions);

	/**
	 * Suggest props for building
	 * @param Building Building actor
	 * @param Style Style (e.g., "military", "residential")
	 * @param OutSuggestions Prop suggestions
	 * @return Number of suggestions
	 */
	static int32 SuggestPropsForBuilding(
		AActor* Building,
		const FString& Style,
		TArray<FDesignSuggestion>& OutSuggestions);

	/**
	 * Apply suggestion automatically
	 * @param Suggestion Suggestion to apply
	 * @param World World context
	 * @return true if applied successfully
	 */
	static bool ApplySuggestion(
		const FDesignSuggestion& Suggestion,
		UWorld* World);

	/**
	 * Get historical rules for era
	 * @param Era Historical era
	 * @return Array of rules/constraints
	 */
	static TArray<FString> GetHistoricalRules(const FString& Era);

private:
	// Analyze actor density
	static float AnalyzeActorDensity(UWorld* World);

	// Check for empty areas
	static TArray<FVector> FindEmptyAreas(UWorld* World, float MinSize);

	// Calculate actor isolation score
	static float CalculateIsolationScore(AActor* Actor, const TArray<AActor*>& NearbyActors);

	// Find nearby actors
	static TArray<AActor*> FindNearbyActors(AActor* CenterActor, float Radius, UWorld* World);

	// Suggest vegetation placement
	static FVector FindVegetationLocation(AActor* NearActor, UWorld* World);

	// Validate suggestion applicability
	static bool ValidateSuggestion(const FDesignSuggestion& Suggestion, UWorld* World);
};
