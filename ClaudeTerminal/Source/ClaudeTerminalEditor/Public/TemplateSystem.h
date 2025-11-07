// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * Template element - single component of a template
 */
struct FTemplateElement
{
	FString ElementType;              // "building", "prop", "vegetation", "trench"
	FString AssetPath;                // Path to asset
	FVector RelativeLocation;         // Relative to template origin
	FRotator RelativeRotation;
	FVector Scale = FVector::OneVector;
	TMap<FString, FString> Parameters; // Element-specific parameters
};

/**
 * Template definition - complete scene template
 */
struct FSceneTemplate
{
	FString TemplateName;
	FString Description;
	TArray<FString> Tags;              // "wwi", "french", "defensive", "village"
	TArray<FTemplateElement> Elements;
	FVector BoundingBoxSize;
	FString Author;
	FDateTime CreatedDate;
};

/**
 * Template spawn result
 */
struct FTemplateSpawnResult
{
	bool bSuccess = false;
	TArray<AActor*> SpawnedActors;
	FString ErrorMessage;
	int32 TotalElementsSpawned = 0;
};

/**
 * Template & Preset System for complex scene generation
 * TIER 3.2 Feature - Revolutionary
 */
class CLAUDETERMINALEDITOR_API FTemplateSystem
{
public:
	/**
	 * Load template from JSON file
	 * @param TemplatePath Path to template JSON file
	 * @param OutTemplate Loaded template
	 * @return true if loaded successfully
	 */
	static bool LoadTemplate(const FString& TemplatePath, FSceneTemplate& OutTemplate);

	/**
	 * Save template to JSON file
	 * @param Template Template to save
	 * @param TemplatePath Output path
	 * @return true if saved successfully
	 */
	static bool SaveTemplate(const FSceneTemplate& Template, const FString& TemplatePath);

	/**
	 * Spawn template at location
	 * @param Template Template to spawn
	 * @param SpawnLocation Center location
	 * @param SpawnRotation Base rotation
	 * @param World World to spawn in
	 * @param OutResult Spawn result
	 * @return true if spawned successfully
	 */
	static bool SpawnTemplate(
		const FSceneTemplate& Template,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation,
		UWorld* World,
		FTemplateSpawnResult& OutResult);

	/**
	 * Create template from selected actors
	 * @param SelectedActors Actors to include in template
	 * @param TemplateName Name for new template
	 * @param OutTemplate Created template
	 * @return true if created successfully
	 */
	static bool CreateTemplateFromSelection(
		const TArray<AActor*>& SelectedActors,
		const FString& TemplateName,
		FSceneTemplate& OutTemplate);

	/**
	 * Search for templates by tags
	 * @param SearchTags Tags to search for
	 * @param OutTemplates Matching templates
	 * @return Number of matches found
	 */
	static int32 SearchTemplates(const TArray<FString>& SearchTags, TArray<FSceneTemplate>& OutTemplates);

	/**
	 * Get all available templates
	 * @param OutTemplates All templates
	 * @return Number of templates
	 */
	static int32 GetAllTemplates(TArray<FSceneTemplate>& OutTemplates);

	/**
	 * Generate built-in template: WWI French Defensive Position
	 */
	static FSceneTemplate GenerateWWIDefensivePosition();

	/**
	 * Generate built-in template: Medieval Village
	 */
	static FSceneTemplate GenerateMedievalVillage();

	/**
	 * Generate built-in template: Forest Clearing
	 */
	static FSceneTemplate GenerateForestClearing();

	/**
	 * Generate built-in template: Military Outpost
	 */
	static FSceneTemplate GenerateMilitaryOutpost();

private:
	// Parse template JSON
	static bool ParseTemplateJSON(const FString& JSONString, FSceneTemplate& OutTemplate);

	// Generate template JSON
	static FString GenerateTemplateJSON(const FSceneTemplate& Template);

	// Spawn single template element
	static AActor* SpawnTemplateElement(
		const FTemplateElement& Element,
		const FVector& BaseLocation,
		const FRotator& BaseRotation,
		UWorld* World);

	// Get template directory path
	static FString GetTemplateDirectory();

	// Validate template
	static bool ValidateTemplate(const FSceneTemplate& Template, FString& OutError);
};
