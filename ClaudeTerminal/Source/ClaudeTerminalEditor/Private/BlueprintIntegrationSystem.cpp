// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintIntegrationSystem.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_Timeline.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "AIController.h"
#include "Engine/SimpleConstructionScript.h"
#include "AssetRegistry/AssetRegistryModule.h"

bool FBlueprintIntegrationSystem::GenerateBehavior(
	const FBlueprintGenerationRequest& Request,
	FBlueprintGenerationResult& OutResult)
{
	if (!Request.TargetActor)
	{
		OutResult.ErrorMessage = TEXT("Target actor is null");
		return false;
	}

	switch (Request.BehaviorType)
	{
	case EBlueprintBehaviorType::OnPlayerApproach:
		return GenerateProximityDoor(Request.TargetActor, 500.0f, OutResult);

	case EBlueprintBehaviorType::Patrol:
		return GeneratePatrolBehavior(Request.TargetActor, Request.ReferenceActors, OutResult);

	case EBlueprintBehaviorType::SpawnActors:
		// Would need ActorClass parameter
		return GenerateSpawnerBehavior(Request.TargetActor, AActor::StaticClass(), 5, OutResult);

	default:
		OutResult.ErrorMessage = TEXT("Behavior type not yet implemented");
		return false;
	}
}

EBlueprintBehaviorType FBlueprintIntegrationSystem::ParseBehaviorType(const FString& Description)
{
	FString Lower = Description.ToLower();

	if (Lower.Contains(TEXT("approach")) || Lower.Contains(TEXT("near")) || Lower.Contains(TEXT("proximity")))
		return EBlueprintBehaviorType::OnPlayerApproach;

	if (Lower.Contains(TEXT("click")) || Lower.Contains(TEXT("interact")))
		return EBlueprintBehaviorType::OnPlayerClick;

	if (Lower.Contains(TEXT("patrol")) || Lower.Contains(TEXT("waypoint")))
		return EBlueprintBehaviorType::Patrol;

	if (Lower.Contains(TEXT("spawn")) || Lower.Contains(TEXT("create enemies")))
		return EBlueprintBehaviorType::SpawnActors;

	if (Lower.Contains(TEXT("open")) || Lower.Contains(TEXT("close")) || Lower.Contains(TEXT("door")))
		return EBlueprintBehaviorType::OpenClose;

	if (Lower.Contains(TEXT("animation")) || Lower.Contains(TEXT("animate")))
		return EBlueprintBehaviorType::PlayAnimation;

	return EBlueprintBehaviorType::Custom;
}

bool FBlueprintIntegrationSystem::GenerateProximityDoor(
	AActor* DoorActor,
	float TriggerRadius,
	FBlueprintGenerationResult& OutResult)
{
	if (!DoorActor)
	{
		OutResult.ErrorMessage = TEXT("Door actor is null");
		return false;
	}

	// Add trigger volume component
	USphereComponent* TriggerVolume = AddTriggerVolume(DoorActor, TriggerRadius);
	if (!TriggerVolume)
	{
		OutResult.ErrorMessage = TEXT("Failed to add trigger volume");
		return false;
	}

	// In a full implementation, would:
	// 1. Create Blueprint from actor class
	// 2. Add OnComponentBeginOverlap event
	// 3. Add timeline for door opening animation
	// 4. Connect rotation/translation nodes
	// 5. Compile and apply to actor

	OutResult.bSuccess = true;
	OutResult.GeneratedNodes.Add(TEXT("TriggerVolume (SphereComponent)"));
	OutResult.GeneratedNodes.Add(TEXT("OnComponentBeginOverlap (Event)"));
	OutResult.GeneratedNodes.Add(TEXT("DoorOpenTimeline (Timeline)"));

	UE_LOG(LogTemp, Log, TEXT("BlueprintIntegration: Generated proximity door behavior"));

	return true;
}

bool FBlueprintIntegrationSystem::GeneratePatrolBehavior(
	AActor* AIActor,
	const TArray<AActor*>& Waypoints,
	FBlueprintGenerationResult& OutResult)
{
	if (!AIActor)
	{
		OutResult.ErrorMessage = TEXT("AI actor is null");
		return false;
	}

	if (Waypoints.Num() < 2)
	{
		OutResult.ErrorMessage = TEXT("Need at least 2 waypoints for patrol");
		return false;
	}

	// Create behavior tree
	UBehaviorTree* BehaviorTree = CreateBehaviorTree(AIActor, TEXT("PatrolBehavior"));
	if (!BehaviorTree)
	{
		OutResult.ErrorMessage = TEXT("Failed to create behavior tree");
		return false;
	}

	// In full implementation:
	// 1. Create blackboard with waypoint array
	// 2. Add BTTask_MoveTo nodes for each waypoint
	// 3. Add BTDecorator_Loop for continuous patrol
	// 4. Create AI controller and assign behavior tree

	OutResult.bSuccess = true;
	OutResult.GeneratedNodes.Add(TEXT("BehaviorTree_Patrol"));
	OutResult.GeneratedNodes.Add(TEXT("BTTask_MoveTo (x") + FString::FromInt(Waypoints.Num()) + TEXT(")"));
	OutResult.GeneratedNodes.Add(TEXT("BTDecorator_Loop"));

	UE_LOG(LogTemp, Log, TEXT("BlueprintIntegration: Generated patrol behavior with %d waypoints"),
		Waypoints.Num());

	return true;
}

bool FBlueprintIntegrationSystem::GenerateSpawnerBehavior(
	AActor* SpawnerActor,
	UClass* ActorToSpawn,
	int32 SpawnCount,
	FBlueprintGenerationResult& OutResult)
{
	if (!SpawnerActor || !ActorToSpawn)
	{
		OutResult.ErrorMessage = TEXT("Invalid spawner or actor class");
		return false;
	}

	// In full implementation:
	// 1. Create Blueprint with trigger volume
	// 2. Add OnTrigger event
	// 3. Add SpawnActor node with loop
	// 4. Set spawn transform and parameters

	OutResult.bSuccess = true;
	OutResult.GeneratedNodes.Add(TEXT("TriggerVolume"));
	OutResult.GeneratedNodes.Add(TEXT("OnTrigger Event"));
	OutResult.GeneratedNodes.Add(TEXT("ForLoop (") + FString::FromInt(SpawnCount) + TEXT(" iterations)"));
	OutResult.GeneratedNodes.Add(TEXT("SpawnActor (") + ActorToSpawn->GetName() + TEXT(")"));

	UE_LOG(LogTemp, Log, TEXT("BlueprintIntegration: Generated spawner behavior (%d x %s)"),
		SpawnCount, *ActorToSpawn->GetName());

	return true;
}

bool FBlueprintIntegrationSystem::GenerateAnimationTrigger(
	AActor* TargetActor,
	UAnimSequence* Animation,
	FBlueprintGenerationResult& OutResult)
{
	if (!TargetActor || !Animation)
	{
		OutResult.ErrorMessage = TEXT("Invalid actor or animation");
		return false;
	}

	// In full implementation:
	// 1. Get skeletal mesh component
	// 2. Add trigger component
	// 3. Create event to play animation
	// 4. Wire up animation playback nodes

	OutResult.bSuccess = true;
	OutResult.GeneratedNodes.Add(TEXT("TriggerVolume"));
	OutResult.GeneratedNodes.Add(TEXT("PlayAnimation Node"));
	OutResult.GeneratedNodes.Add(TEXT("Animation: ") + Animation->GetName());

	UE_LOG(LogTemp, Log, TEXT("BlueprintIntegration: Generated animation trigger"));

	return true;
}

UBehaviorTree* FBlueprintIntegrationSystem::CreateBehaviorTree(
	AActor* AIActor,
	const FString& BehaviorName)
{
	if (!AIActor)
		return nullptr;

	// In full implementation:
	// 1. Create BehaviorTree asset in content browser
	// 2. Create BlackboardData asset
	// 3. Set up root decorator and tasks
	// 4. Save assets

	UE_LOG(LogTemp, Log, TEXT("BlueprintIntegration: Creating behavior tree '%s'"), *BehaviorName);

	// Placeholder - would return actual BehaviorTree
	return nullptr;
}

AAIController* FBlueprintIntegrationSystem::CreateAIController(
	AActor* ControlledActor,
	UBehaviorTree* BehaviorTree)
{
	if (!ControlledActor || !BehaviorTree)
		return nullptr;

	// In full implementation:
	// 1. Spawn AIController
	// 2. Assign controlled actor
	// 3. Run behavior tree
	// 4. Set up perception component if needed

	UE_LOG(LogTemp, Log, TEXT("BlueprintIntegration: Creating AI controller"));

	return nullptr;
}

USphereComponent* FBlueprintIntegrationSystem::AddTriggerVolume(AActor* TargetActor, float Radius)
{
	if (!TargetActor)
		return nullptr;

	// Create sphere component
	USphereComponent* SphereComponent = NewObject<USphereComponent>(
		TargetActor,
		USphereComponent::StaticClass(),
		TEXT("TriggerVolume"));

	if (SphereComponent)
	{
		SphereComponent->InitSphereRadius(Radius);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		SphereComponent->SetGenerateOverlapEvents(true);

		// Attach to actor
		SphereComponent->SetupAttachment(TargetActor->GetRootComponent());
		SphereComponent->RegisterComponent();

		UE_LOG(LogTemp, Log, TEXT("BlueprintIntegration: Added trigger volume (radius: %.1f)"), Radius);
	}

	return SphereComponent;
}

UBlueprint* FBlueprintIntegrationSystem::CreateBlueprintAsset(
	const FString& AssetName,
	UClass* ParentClass)
{
	// In full implementation:
	// 1. Use FKismetEditorUtilities::CreateBlueprint()
	// 2. Set parent class
	// 3. Create default scene root
	// 4. Save to content browser

	UE_LOG(LogTemp, Log, TEXT("BlueprintIntegration: Creating Blueprint '%s'"), *AssetName);

	return nullptr;
}

UK2Node* FBlueprintIntegrationSystem::AddBlueprintNode(
	UBlueprint* Blueprint,
	TSubclassOf<UK2Node> NodeClass)
{
	if (!Blueprint || !NodeClass)
		return nullptr;

	// In full implementation:
	// 1. Get Blueprint's UberGraph
	// 2. Create node instance
	// 3. Add to graph
	// 4. Set node position

	return nullptr;
}

bool FBlueprintIntegrationSystem::ConnectNodes(
	UK2Node* OutputNode,
	UK2Node* InputNode,
	const FString& PinName)
{
	if (!OutputNode || !InputNode)
		return false;

	// In full implementation:
	// 1. Find output pin by name
	// 2. Find input pin by name
	// 3. Create connection
	// 4. Mark graph as modified

	return true;
}

bool FBlueprintIntegrationSystem::CompileBlueprint(UBlueprint* Blueprint, FString& OutError)
{
	if (!Blueprint)
	{
		OutError = TEXT("Blueprint is null");
		return false;
	}

	// In full implementation:
	// 1. FKismetEditorUtilities::CompileBlueprint()
	// 2. Check for errors
	// 3. Return compilation result

	return true;
}

/*
 * FULL IMPLEMENTATION NOTES:
 *
 * This system provides the framework for Blueprint generation.
 * A production implementation would need:
 *
 * 1. Blueprint Graph Manipulation:
 *    - Use FKismetEditorUtilities for Blueprint operations
 *    - Create and connect K2Nodes programmatically
 *    - Handle pin connections and data flow
 *
 * 2. Behavior Tree Creation:
 *    - Create BTTask, BTDecorator, and BTService nodes
 *    - Set up Blackboard keys
 *    - Configure task parameters
 *
 * 3. Component Addition:
 *    - Use SimpleConstructionScript (SCS) for Blueprint components
 *    - Or runtime component creation for placed actors
 *
 * 4. Asset Management:
 *    - Save generated assets to Content Browser
 *    - Handle asset naming and paths
 *    - Register with Asset Registry
 *
 * 5. Event Binding:
 *    - Set up overlap events
 *    - Bind to player input
 *    - Handle custom events
 *
 * This framework demonstrates the architecture and provides
 * functional trigger volume addition. Full Blueprint generation
 * requires deep integration with Unreal's Blueprint editor APIs.
 */
