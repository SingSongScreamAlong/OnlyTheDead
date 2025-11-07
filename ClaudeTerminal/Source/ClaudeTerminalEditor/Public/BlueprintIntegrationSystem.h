// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Blueprint.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"

/**
 * Blueprint behavior types
 */
UENUM()
enum class EBlueprintBehaviorType : uint8
{
	OnPlayerApproach,      // "when player approaches"
	OnPlayerClick,         // "when player clicks"
	OnOverlap,             // "when something touches"
	Patrol,                // "patrol between waypoints"
	FollowPath,            // "follow this path"
	LookAt,                // "always look at player"
	OpenClose,             // "open/close door"
	ActivateDeactivate,    // "turn on/off"
	SpawnActors,           // "spawn enemies"
	PlayAnimation,         // "play animation"
	ChangeM aterial,        // "change appearance"
	Custom                 // Custom behavior
};

/**
 * Blueprint generation request
 */
struct FBlueprintGenerationRequest
{
	EBlueprintBehaviorType BehaviorType = EBlueprintBehaviorType::Custom;
	AActor* TargetActor = nullptr;
	TArray<AActor*> ReferenceActors;  // Waypoints, targets, etc.
	FString CustomLogic;               // Natural language description
	TMap<FString, FString> Parameters; // Behavior parameters
};

/**
 * Blueprint generation result
 */
struct FBlueprintGenerationResult
{
	bool bSuccess = false;
	UBlueprint* GeneratedBlueprint = nullptr;
	FString ErrorMessage;
	TArray<FString> GeneratedNodes;    // List of nodes created
};

/**
 * System for generating Blueprint logic from natural language
 * TIER 3.1 Feature - Revolutionary
 */
class CLAUDETERMINALEDITOR_API FBlueprintIntegrationSystem
{
public:
	/**
	 * Generate Blueprint behavior from natural language
	 * @param Request Behavior generation request
	 * @param OutResult Generation result
	 * @return true if generation successful
	 */
	static bool GenerateBehavior(const FBlueprintGenerationRequest& Request, FBlueprintGenerationResult& OutResult);

	/**
	 * Parse natural language into behavior type
	 * @param Description Natural language description
	 * @return Detected behavior type
	 */
	static EBlueprintBehaviorType ParseBehaviorType(const FString& Description);

	/**
	 * Generate "door opens when player approaches" behavior
	 */
	static bool GenerateProximityDoor(AActor* DoorActor, float TriggerRadius, FBlueprintGenerationResult& OutResult);

	/**
	 * Generate "patrol between waypoints" AI behavior
	 */
	static bool GeneratePatrolBehavior(AActor* AIActor, const TArray<AActor*>& Waypoints, FBlueprintGenerationResult& OutResult);

	/**
	 * Generate "spawn enemies when triggered" behavior
	 */
	static bool GenerateSpawnerBehavior(AActor* SpawnerActor, UClass* ActorToSpawn, int32 SpawnCount, FBlueprintGenerationResult& OutResult);

	/**
	 * Generate "play animation on trigger" behavior
	 */
	static bool GenerateAnimationTrigger(AActor* TargetActor, UAnimSequence* Animation, FBlueprintGenerationResult& OutResult);

	/**
	 * Create behavior tree for AI
	 */
	static UBehaviorTree* CreateBehaviorTree(AActor* AIActor, const FString& BehaviorName);

	/**
	 * Create AI controller with behavior
	 */
	static AAIController* CreateAIController(AActor* ControlledActor, UBehaviorTree* BehaviorTree);

	/**
	 * Add collision trigger to actor
	 */
	static class USphereComponent* AddTriggerVolume(AActor* TargetActor, float Radius);

private:
	// Create Blueprint asset
	static UBlueprint* CreateBlueprintAsset(const FString& AssetName, UClass* ParentClass);

	// Add node to Blueprint graph
	static class UK2Node* AddBlueprintNode(UBlueprint* Blueprint, TSubclassOf<UK2Node> NodeClass);

	// Connect Blueprint nodes
	static bool ConnectNodes(class UK2Node* OutputNode, class UK2Node* InputNode, const FString& PinName);

	// Compile Blueprint
	static bool CompileBlueprint(UBlueprint* Blueprint, FString& OutError);
};
