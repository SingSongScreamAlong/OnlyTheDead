// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"

/**
 * Game logic type
 */
UENUM()
enum class EGameLogicType : uint8
{
	CapturePoint,         // Control point capture system
	DoorInteraction,      // Door open/close with triggers
	PatrolAI,             // NPC patrol behavior
	QuestSystem,          // Quest tracking and objectives
	InventorySystem,      // Item collection and management
	DialogSystem,         // NPC conversation trees
	CombatSystem,         // Health, damage, weapons
	WaveSpawner,          // Enemy wave spawning
	Checkpoint,           // Save/load checkpoint
	Collectible,          // Pickup items with effects
	PuzzleMechanic,       // Interactive puzzle
	VehicleSystem         // Drivable vehicles
};

/**
 * Generated component info
 */
struct FGeneratedComponent
{
	FString ComponentName;
	UClass* ComponentClass;
	TMap<FString, FString> Properties; // Property name -> value
};

/**
 * Generated Blueprint info
 */
struct FGeneratedBlueprint
{
	FString BlueprintName;
	UClass* ParentClass;
	TArray<FGeneratedComponent> Components;
	TArray<FString> Variables;        // Variable definitions
	TArray<FString> Functions;        // Function implementations
	TArray<FString> EventGraphs;      // Event graph descriptions
};

/**
 * Game logic generation result
 */
struct FGameLogicResult
{
	bool bSuccess = false;
	FString ErrorMessage;
	TArray<FGeneratedBlueprint> GeneratedBlueprints;
	TArray<AActor*> GeneratedActors;
	TArray<FString> InstructionsForUser;  // Steps user needs to complete manually
	FString SystemDescription;             // What was created
};

/**
 * Capture point configuration
 */
struct FCapturePointConfig
{
	float CaptureRadius = 500.0f;
	float CaptureTime = 10.0f;
	int32 NumTeams = 2;
	TArray<FLinearColor> TeamColors;
	bool bContested = true;               // Can teams contest each other
	bool bShowProgressBar = true;
	bool bShowTeamIndicator = true;
};

/**
 * AI Patrol configuration
 */
struct FPatrolConfig
{
	TArray<FVector> WaypointLocations;
	float WalkSpeed = 200.0f;
	float WaitTimeAtWaypoint = 2.0f;
	bool bLoopPath = true;
	bool bReverseOnEnd = false;
};

/**
 * Quest configuration
 */
struct FQuestConfig
{
	FString QuestName;
	FString Description;
	TArray<FString> ObjectiveDescriptions;
	TArray<AActor*> ObjectiveActors;      // Actors related to objectives
	bool bShowHUD = true;
};

/**
 * Full game logic generation system
 * TIER 4.3 Feature - Revolutionary
 *
 * Generates complete gameplay mechanics from natural language:
 * "Create a capture point system" -> Full Blueprint with UI, logic, scoring
 */
class CLAUDETERMINALEDITOR_API FGameLogicGenerationSystem
{
public:
	/**
	 * Generate game logic from natural language description
	 * @param Description Natural language description (e.g., "Create a capture point system")
	 * @param World World context
	 * @param ContextActors Relevant actors from scene
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateGameLogic(
		const FString& Description,
		UWorld* World,
		const TArray<AActor*>& ContextActors,
		FGameLogicResult& OutResult);

	/**
	 * Generate capture point system
	 * @param Location Capture point location
	 * @param Config Capture point configuration
	 * @param World World context
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateCapturePointSystem(
		const FVector& Location,
		const FCapturePointConfig& Config,
		UWorld* World,
		FGameLogicResult& OutResult);

	/**
	 * Generate door interaction system
	 * @param DoorActor Door actor
	 * @param bAutomatic true = auto-open on proximity, false = player interaction required
	 * @param World World context
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateDoorInteraction(
		AActor* DoorActor,
		bool bAutomatic,
		UWorld* World,
		FGameLogicResult& OutResult);

	/**
	 * Generate AI patrol behavior
	 * @param AIActor Actor to patrol
	 * @param Config Patrol configuration
	 * @param World World context
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateAIPatrolBehavior(
		AActor* AIActor,
		const FPatrolConfig& Config,
		UWorld* World,
		FGameLogicResult& OutResult);

	/**
	 * Generate quest system
	 * @param Config Quest configuration
	 * @param World World context
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateQuestSystem(
		const FQuestConfig& Config,
		UWorld* World,
		FGameLogicResult& OutResult);

	/**
	 * Generate inventory system
	 * @param MaxSlots Number of inventory slots
	 * @param World World context
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateInventorySystem(
		int32 MaxSlots,
		UWorld* World,
		FGameLogicResult& OutResult);

	/**
	 * Generate wave spawner system
	 * @param SpawnLocation Spawn point
	 * @param WaveCount Number of waves
	 * @param EnemiesPerWave Enemies in each wave
	 * @param TimeBetweenWaves Delay between waves (seconds)
	 * @param World World context
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateWaveSpawner(
		const FVector& SpawnLocation,
		int32 WaveCount,
		int32 EnemiesPerWave,
		float TimeBetweenWaves,
		UWorld* World,
		FGameLogicResult& OutResult);

	/**
	 * Generate collectible system
	 * @param CollectibleActors Actors that can be collected
	 * @param CollectibleType Type (e.g., "coin", "health", "ammo")
	 * @param World World context
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateCollectibleSystem(
		const TArray<AActor*>& CollectibleActors,
		const FString& CollectibleType,
		UWorld* World,
		FGameLogicResult& OutResult);

	/**
	 * Classify game logic type from description
	 * @param Description Natural language description
	 * @return Detected logic type
	 */
	static EGameLogicType ClassifyGameLogic(const FString& Description);

	/**
	 * Extract configuration from natural language
	 * @param Description Natural language description
	 * @param LogicType Detected logic type
	 * @param OutConfig Configuration map
	 * @return true if extraction successful
	 */
	static bool ExtractConfiguration(
		const FString& Description,
		EGameLogicType LogicType,
		TMap<FString, FString>& OutConfig);

	/**
	 * Generate Blueprint class programmatically
	 * @param BlueprintInfo Blueprint specification
	 * @param World World context
	 * @return Generated Blueprint class (nullptr if failed)
	 */
	static UClass* GenerateBlueprintClass(
		const FGeneratedBlueprint& BlueprintInfo,
		UWorld* World);

	/**
	 * Add component to Blueprint
	 * @param Blueprint Target Blueprint
	 * @param ComponentInfo Component specification
	 * @return true if added successfully
	 */
	static bool AddComponentToBlueprint(
		UBlueprint* Blueprint,
		const FGeneratedComponent& ComponentInfo);

	/**
	 * Generate Blueprint event graph nodes
	 * @param Blueprint Target Blueprint
	 * @param EventDescription Event graph description
	 * @return true if generation successful
	 */
	static bool GenerateEventGraph(
		UBlueprint* Blueprint,
		const FString& EventDescription);

private:
	// Parse natural language for logic type
	static EGameLogicType ParseLogicType(const FString& Description);

	// Extract numeric parameters
	static float ExtractNumericValue(const FString& Description, const FString& Keyword, float DefaultValue);

	// Extract boolean parameters
	static bool ExtractBooleanValue(const FString& Description, const FString& Keyword, bool DefaultValue);

	// Extract string parameters
	static FString ExtractStringValue(const FString& Description, const FString& Keyword, const FString& DefaultValue);

	// Generate Blueprint skeleton
	static UBlueprint* CreateBlueprintSkeleton(const FString& Name, UClass* ParentClass);

	// Add variable to Blueprint
	static bool AddBlueprintVariable(UBlueprint* Blueprint, const FString& VarName, const FString& VarType);

	// Generate function implementation
	static bool GenerateBlueprintFunction(UBlueprint* Blueprint, const FString& FunctionName, const FString& Implementation);

	// Create UI widget Blueprint
	static UBlueprint* CreateUIWidgetBlueprint(const FString& Name, const FString& UIDescription);
};
