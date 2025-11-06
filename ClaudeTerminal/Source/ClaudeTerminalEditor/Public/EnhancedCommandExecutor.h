// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommandExecutor.h"
#include "LandscapeManager.h"
#include "AssetCatalogSystem.h"

class UWorld;
class AActor;
class UStaticMesh;
class UMaterialInterface;
class ALight;
class ADirectionalLight;
class APointLight;
class ASpotLight;
class ACameraActor;

/**
 * Comprehensive NLP command executor for UE5
 * Handles natural language commands for all aspects of level editing:
 * - Asset spawning (intelligent selection)
 * - Building/structure placement
 * - Material application
 * - Lighting control
 * - Props and decoration
 * - Animation and cinematics
 * - Batch operations
 */
class CLAUDETERMINALEDITOR_API FEnhancedCommandExecutor
{
public:
	FEnhancedCommandExecutor();
	~FEnhancedCommandExecutor();

	/**
	 * Initialize the executor (build asset catalog, etc.)
	 */
	bool Initialize(FString& OutLog);

	/**
	 * Process natural language response from Claude
	 */
	bool ProcessResponse(const FString& Response, UWorld* World, FString& OutExecutionLog);

	/**
	 * Get comprehensive help text
	 */
	static FString GetHelpText();

	/**
	 * Get asset catalog system
	 */
	FAssetCatalogSystem* GetAssetCatalog() { return &AssetCatalog; }

private:
	// JSON command extraction
	bool ExtractJSONCommand(const FString& Response, TSharedPtr<FJsonObject>& OutCommand);

	// ===== ASSET & OBJECT PLACEMENT =====
	bool ExecuteSpawnObject(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecutePlaceBuilding(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecutePlaceProps(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecutePlaceVegetation(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// ===== MATERIAL & TEXTURE =====
	bool ExecuteApplyMaterial(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteSetMaterialParameter(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// ===== LIGHTING & ATMOSPHERE =====
	bool ExecuteCreateLight(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteModifyLight(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteSetTimeOfDay(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteSetWeather(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteSetFog(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// ===== ANIMATION & CINEMATIC =====
	bool ExecuteCreateCamera(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecutePlayAnimation(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// ===== ACTOR MANIPULATION =====
	bool ExecuteModifyActor(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteDeleteActor(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteDuplicateActor(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// ===== BATCH OPERATIONS =====
	bool ExecuteBatchPlace(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteArrangePattern(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// ===== SCENE COMPOSITION =====
	bool ExecuteComposeScene(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// ===== LANDSCAPE (from existing system) =====
	bool ExecuteCreateLandscape(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteImportTerrain(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteModifyTerrain(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteCreateCrater(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteCreateTrench(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// ===== HELPER FUNCTIONS =====

	// Parse JSON types
	FVector ParseVector(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FVector& Default = FVector::ZeroVector);
	FRotator ParseRotator(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FRotator& Default = FRotator::ZeroRotator);
	FLinearColor ParseColor(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FLinearColor& Default = FLinearColor::White);

	// Context-aware placement
	FVector ResolveLocation(const TSharedPtr<FJsonObject>& Parameters, UWorld* World);
	FRotator ResolveRotation(const TSharedPtr<FJsonObject>& Parameters, UWorld* World);

	// Asset resolution
	UStaticMesh* FindBestStaticMesh(const FString& Query);
	UMaterialInterface* FindBestMaterial(const FString& Query);
	UClass* FindBestBlueprint(const FString& Query);

	// Intelligent placement
	bool PlaceObjectIntelligently(AActor* Actor, const FVector& Location, const FString& Context, UWorld* World, FString& OutLog);
	FVector FindGroundHeight(const FVector& Location, UWorld* World);
	bool CheckPlacementValid(const FVector& Location, const FVector& Extent, UWorld* World);

	// Visual feedback
	void SelectActor(AActor* Actor);
	void SelectActors(const TArray<AActor*>& Actors);

	// Lighting helpers
	ADirectionalLight* FindDirectionalLight(UWorld* World);
	void SetGlobalLightingIntensity(float Intensity, UWorld* World);

	// Components
	FCommandExecutor BaseExecutor; // For basic commands
	FLandscapeManager LandscapeManager; // For terrain
	FAssetCatalogSystem AssetCatalog; // For intelligent asset selection

	// State
	bool bInitialized;
};
