// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "LevelInspector.generated.h"

/**
 * Actor information for inspection
 */
USTRUCT(BlueprintType)
struct FActorInspectionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	FString Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	FVector Scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	TArray<FString> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	TArray<FString> Components;
};

/**
 * Level summary statistics
 */
USTRUCT(BlueprintType)
struct FLevelSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	int32 TotalActors = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	int32 StaticMeshActors = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	int32 Lights = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	int32 Landscapes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	FVector LevelBoundsMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	FVector LevelBoundsMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	TMap<FString, int32> ActorCountByType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspection")
	int32 SelectedActorCount = 0;
};

/**
 * Level Inspector - Allows AI to query and understand scene state
 * This is critical for the AI to make informed decisions
 */
UCLASS(BlueprintType)
class VERDUNAIAGENT_API ULevelInspector : public UObject
{
	GENERATED_BODY()

public:
	ULevelInspector();

	/**
	 * Get the editor world
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static UWorld* GetEditorWorld();

	/**
	 * Get summary of current level
	 * Returns a human-readable description of what's in the level
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static FString GetLevelSummaryText(UWorld* World);

	/**
	 * Get detailed level statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static FLevelSummary GetLevelSummary(UWorld* World);

	/**
	 * Get all actors in the level
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<AActor*> GetAllActors(UWorld* World);

	/**
	 * Find actors by name (supports wildcards with *)
	 * Example: "Trench*" finds all actors starting with "Trench"
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<AActor*> FindActorsByName(UWorld* World, const FString& NamePattern);

	/**
	 * Find actors by class
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<AActor*> FindActorsByClass(UWorld* World, UClass* ActorClass);

	/**
	 * Find actors by tag
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<AActor*> FindActorsByTag(UWorld* World, const FName& Tag);

	/**
	 * Get currently selected actors in the editor
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<AActor*> GetSelectedActors();

	/**
	 * Get actors within radius of a point
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<AActor*> GetActorsInRadius(UWorld* World, const FVector& Center, float Radius);

	/**
	 * Get actors within a box
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<AActor*> GetActorsInBox(UWorld* World, const FVector& BoxMin, const FVector& BoxMax);

	/**
	 * Get detailed information about a specific actor
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static FActorInspectionData InspectActor(AActor* Actor);

	/**
	 * Get a natural language description of an actor
	 * Example: "StaticMeshActor 'Trench_01' at location (1000, 500, 0) with 3 components"
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static FString GetActorDescription(AActor* Actor);

	/**
	 * Check if an actor matches a description
	 * Used for verification: "Does this look like a trench?"
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static bool ActorMatchesDescription(AActor* Actor, const FString& Description);

	/**
	 * Get available assets in content browser
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<FString> GetAvailableAssets(const FString& AssetType, const FString& PathFilter = TEXT(""));

	/**
	 * Find asset by name or path
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static FString FindAssetPath(const FString& AssetName, const FString& AssetType = TEXT(""));

	/**
	 * Get landscape information
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static FString GetLandscapeInfo(UWorld* World);

	/**
	 * Get materials used in the level
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static TArray<FString> GetMaterialsInLevel(UWorld* World);

	/**
	 * Get lighting information
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static FString GetLightingInfo(UWorld* World);

	/**
	 * Generate a comprehensive context string for the AI
	 * This is what the AI "sees" when looking at the level
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Inspection")
	static FString GenerateAIContextString(UWorld* World);

private:
	// Helper functions
	static bool MatchesWildcard(const FString& String, const FString& Pattern);
	static FString GetActorTypeString(AActor* Actor);
	static FString FormatLocation(const FVector& Location);
};
