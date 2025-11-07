// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameLogicGenerationSystem.h"
#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"

bool FGameLogicGenerationSystem::GenerateGameLogic(
	const FString& Description,
	UWorld* World,
	const TArray<AActor*>& ContextActors,
	FGameLogicResult& OutResult)
{
	if (!World)
	{
		OutResult.bSuccess = false;
		OutResult.ErrorMessage = TEXT("Invalid world context");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GameLogic: Generating game logic from: '%s'"), *Description);

	// Classify the type of game logic requested
	EGameLogicType LogicType = ClassifyGameLogic(Description);

	// Extract configuration from natural language
	TMap<FString, FString> Config;
	ExtractConfiguration(Description, LogicType, Config);

	// Generate based on type
	switch (LogicType)
	{
	case EGameLogicType::CapturePoint:
		{
			FCapturePointConfig CPConfig;
			CPConfig.CaptureRadius = ExtractNumericValue(Description, TEXT("radius"), 500.0f);
			CPConfig.CaptureTime = ExtractNumericValue(Description, TEXT("time"), 10.0f);
			CPConfig.NumTeams = FMath::RoundToInt(ExtractNumericValue(Description, TEXT("teams"), 2.0f));

			FVector Location = ContextActors.Num() > 0 ? ContextActors[0]->GetActorLocation() : FVector::ZeroVector;
			return GenerateCapturePointSystem(Location, CPConfig, World, OutResult);
		}

	case EGameLogicType::DoorInteraction:
		{
			if (ContextActors.Num() > 0)
			{
				bool bAutomatic = Description.Contains(TEXT("automatic")) || Description.Contains(TEXT("proximity"));
				return GenerateDoorInteraction(ContextActors[0], bAutomatic, World, OutResult);
			}
			else
			{
				OutResult.ErrorMessage = TEXT("No door actor specified");
				return false;
			}
		}

	case EGameLogicType::PatrolAI:
		{
			if (ContextActors.Num() < 2)
			{
				OutResult.ErrorMessage = TEXT("Need AI actor and waypoints");
				return false;
			}

			FPatrolConfig PatrolConfig;
			for (int32 i = 1; i < ContextActors.Num(); i++)
			{
				PatrolConfig.WaypointLocations.Add(ContextActors[i]->GetActorLocation());
			}
			PatrolConfig.WalkSpeed = ExtractNumericValue(Description, TEXT("speed"), 200.0f);
			PatrolConfig.bLoopPath = !Description.Contains(TEXT("once")) && !Description.Contains(TEXT("reverse"));

			return GenerateAIPatrolBehavior(ContextActors[0], PatrolConfig, World, OutResult);
		}

	case EGameLogicType::WaveSpawner:
		{
			FVector SpawnLoc = ContextActors.Num() > 0 ? ContextActors[0]->GetActorLocation() : FVector::ZeroVector;
			int32 WaveCount = FMath::RoundToInt(ExtractNumericValue(Description, TEXT("waves"), 5.0f));
			int32 EnemiesPerWave = FMath::RoundToInt(ExtractNumericValue(Description, TEXT("enemies"), 10.0f));
			float TimeBetween = ExtractNumericValue(Description, TEXT("delay"), 30.0f);

			return GenerateWaveSpawner(SpawnLoc, WaveCount, EnemiesPerWave, TimeBetween, World, OutResult);
		}

	case EGameLogicType::InventorySystem:
		{
			int32 MaxSlots = FMath::RoundToInt(ExtractNumericValue(Description, TEXT("slots"), 10.0f));
			return GenerateInventorySystem(MaxSlots, World, OutResult);
		}

	case EGameLogicType::Collectible:
		{
			FString CollectibleType = ExtractStringValue(Description, TEXT("type"), TEXT("generic"));
			return GenerateCollectibleSystem(ContextActors, CollectibleType, World, OutResult);
		}

	default:
		OutResult.bSuccess = false;
		OutResult.ErrorMessage = TEXT("Unsupported game logic type");
		return false;
	}
}

bool FGameLogicGenerationSystem::GenerateCapturePointSystem(
	const FVector& Location,
	const FCapturePointConfig& Config,
	UWorld* World,
	FGameLogicResult& OutResult)
{
	UE_LOG(LogTemp, Log, TEXT("GameLogic: Generating capture point system at %s"), *Location.ToString());

	// Create Blueprint specification
	FGeneratedBlueprint CapturePointBP;
	CapturePointBP.BlueprintName = TEXT("BP_CapturePoint");
	CapturePointBP.ParentClass = AActor::StaticClass();

	// Add components
	FGeneratedComponent TriggerVolume;
	TriggerVolume.ComponentName = TEXT("CaptureVolume");
	TriggerVolume.ComponentClass = USphereComponent::StaticClass();
	TriggerVolume.Properties.Add(TEXT("SphereRadius"), FString::Printf(TEXT("%.1f"), Config.CaptureRadius));
	CapturePointBP.Components.Add(TriggerVolume);

	// Add variables
	CapturePointBP.Variables.Add(TEXT("CurrentTeam:int32:-1"));
	CapturePointBP.Variables.Add(TEXT("CaptureProgress:float:0.0"));
	CapturePointBP.Variables.Add(TEXT("PlayersInZone:int32:0"));
	CapturePointBP.Variables.Add(TEXT("CaptureTime:float:") + FString::Printf(TEXT("%.1f"), Config.CaptureTime));
	CapturePointBP.Variables.Add(TEXT("bIsContested:bool:false"));

	// Add functions
	CapturePointBP.Functions.Add(TEXT("OnPlayerEnterZone:Increment PlayersInZone, Start Capture Timer"));
	CapturePointBP.Functions.Add(TEXT("OnPlayerLeaveZone:Decrement PlayersInZone, Stop if empty"));
	CapturePointBP.Functions.Add(TEXT("UpdateCaptureProgress:Increase progress over time, Check for completion"));
	CapturePointBP.Functions.Add(TEXT("OnCaptureComplete:Set CurrentTeam, Broadcast event, Update UI"));

	// Event graphs
	CapturePointBP.EventGraphs.Add(TEXT("BeginPlay:Initialize capture zone, Bind overlap events"));
	CapturePointBP.EventGraphs.Add(TEXT("Tick:Update capture progress if players in zone"));
	CapturePointBP.EventGraphs.Add(TEXT("OnComponentBeginOverlap:Check if player, Determine team, Call OnPlayerEnterZone"));
	CapturePointBP.EventGraphs.Add(TEXT("OnComponentEndOverlap:Check if player, Call OnPlayerLeaveZone"));

	OutResult.GeneratedBlueprints.Add(CapturePointBP);

	// Create UI Blueprint
	FGeneratedBlueprint UIBP;
	UIBP.BlueprintName = TEXT("WBP_CapturePointUI");
	UIBP.ParentClass = UUserWidget::StaticClass();
	UIBP.Variables.Add(TEXT("CaptureProgress:float:0.0"));
	UIBP.Variables.Add(TEXT("CurrentTeam:int32:-1"));
	UIBP.Variables.Add(TEXT("TeamColors:Array<LinearColor>"));

	OutResult.GeneratedBlueprints.Add(UIBP);

	// Instructions for user
	OutResult.InstructionsForUser.Add(TEXT("1. Open generated BP_CapturePoint Blueprint"));
	OutResult.InstructionsForUser.Add(TEXT("2. Configure team colors in the TeamColors array"));
	OutResult.InstructionsForUser.Add(TEXT("3. Open WBP_CapturePointUI and design the progress bar widget"));
	OutResult.InstructionsForUser.Add(TEXT("4. Connect capture events to your game mode for scoring"));
	OutResult.InstructionsForUser.Add(TEXT("5. Place BP_CapturePoint actors in your level at strategic locations"));

	OutResult.SystemDescription = FString::Printf(
		TEXT("Capture Point System: %d teams competing for control. Capture radius: %.0f units, Capture time: %.1f seconds."),
		Config.NumTeams, Config.CaptureRadius, Config.CaptureTime);

	OutResult.bSuccess = true;

	UE_LOG(LogTemp, Log, TEXT("GameLogic: Capture point system generated successfully"));
	return true;
}

bool FGameLogicGenerationSystem::GenerateDoorInteraction(
	AActor* DoorActor,
	bool bAutomatic,
	UWorld* World,
	FGameLogicResult& OutResult)
{
	if (!DoorActor || !World)
	{
		OutResult.ErrorMessage = TEXT("Invalid door actor or world");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GameLogic: Generating door interaction for %s (automatic=%d)"),
		*DoorActor->GetName(), bAutomatic);

	FGeneratedBlueprint DoorBP;
	DoorBP.BlueprintName = TEXT("BP_InteractiveDoor");
	DoorBP.ParentClass = AActor::StaticClass();

	// Components
	FGeneratedComponent TriggerVolume;
	TriggerVolume.ComponentName = TEXT("TriggerVolume");
	TriggerVolume.ComponentClass = UBoxComponent::StaticClass();
	TriggerVolume.Properties.Add(TEXT("BoxExtent"), TEXT("200,200,100"));
	DoorBP.Components.Add(TriggerVolume);

	// Variables
	DoorBP.Variables.Add(TEXT("bIsOpen:bool:false"));
	DoorBP.Variables.Add(TEXT("OpenRotation:Rotator:(0,90,0)"));
	DoorBP.Variables.Add(TEXT("ClosedRotation:Rotator:(0,0,0)"));
	DoorBP.Variables.Add(TEXT("OpenSpeed:float:2.0"));
	DoorBP.Variables.Add(TEXT("bAutomatic:bool:") + FString(bAutomatic ? TEXT("true") : TEXT("false")));

	// Functions
	DoorBP.Functions.Add(TEXT("OpenDoor:Interpolate to OpenRotation, Set bIsOpen=true, Play sound"));
	DoorBP.Functions.Add(TEXT("CloseDoor:Interpolate to ClosedRotation, Set bIsOpen=false, Play sound"));
	DoorBP.Functions.Add(TEXT("ToggleDoor:If open then close, else open"));

	// Events
	if (bAutomatic)
	{
		DoorBP.EventGraphs.Add(TEXT("OnActorBeginOverlap:Check if player, Call OpenDoor"));
		DoorBP.EventGraphs.Add(TEXT("OnActorEndOverlap:Check if player, Wait 2 seconds, Call CloseDoor"));
	}
	else
	{
		DoorBP.EventGraphs.Add(TEXT("OnActorBeginOverlap:Show 'Press E to open' prompt"));
		DoorBP.EventGraphs.Add(TEXT("OnInteract (E key):Call ToggleDoor"));
	}

	OutResult.GeneratedBlueprints.Add(DoorBP);

	OutResult.InstructionsForUser.Add(TEXT("1. Replace your door's class with BP_InteractiveDoor"));
	OutResult.InstructionsForUser.Add(TEXT("2. Configure OpenRotation and ClosedRotation to match your door's swing"));
	OutResult.InstructionsForUser.Add(TEXT("3. Add door opening/closing sound effects"));
	OutResult.InstructionsForUser.Add(TEXT("4. If using interaction (not automatic), bind E key to Interact event"));

	OutResult.SystemDescription = bAutomatic ?
		TEXT("Automatic proximity door: Opens when player approaches, closes when they leave.") :
		TEXT("Interactive door: Player presses E to open/close.");

	OutResult.bSuccess = true;
	return true;
}

bool FGameLogicGenerationSystem::GenerateAIPatrolBehavior(
	AActor* AIActor,
	const FPatrolConfig& Config,
	UWorld* World,
	FGameLogicResult& OutResult)
{
	if (!AIActor || !World)
	{
		OutResult.ErrorMessage = TEXT("Invalid AI actor or world");
		return false;
	}

	if (Config.WaypointLocations.Num() < 2)
	{
		OutResult.ErrorMessage = TEXT("Need at least 2 waypoints for patrol");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GameLogic: Generating patrol behavior with %d waypoints"), Config.WaypointLocations.Num());

	FGeneratedBlueprint PatrolBP;
	PatrolBP.BlueprintName = TEXT("BP_PatrolAI");
	PatrolBP.ParentClass = ACharacter::StaticClass();

	// Variables
	PatrolBP.Variables.Add(TEXT("WaypointLocations:Array<Vector>"));
	PatrolBP.Variables.Add(TEXT("CurrentWaypointIndex:int32:0"));
	PatrolBP.Variables.Add(TEXT("WalkSpeed:float:") + FString::Printf(TEXT("%.1f"), Config.WalkSpeed));
	PatrolBP.Variables.Add(TEXT("WaitTime:float:") + FString::Printf(TEXT("%.1f"), Config.WaitTimeAtWaypoint));
	PatrolBP.Variables.Add(TEXT("bLoopPath:bool:") + FString(Config.bLoopPath ? TEXT("true") : TEXT("false")));
	PatrolBP.Variables.Add(TEXT("bIsWaiting:bool:false"));

	// Functions
	PatrolBP.Functions.Add(TEXT("StartPatrol:Set movement speed, Move to first waypoint"));
	PatrolBP.Functions.Add(TEXT("MoveToNextWaypoint:Increment index, Move to waypoint, Handle loop/reverse"));
	PatrolBP.Functions.Add(TEXT("OnWaypointReached:Wait at waypoint, Then call MoveToNextWaypoint"));
	PatrolBP.Functions.Add(TEXT("OnSeePlayer:Stop patrol, Chase player (if desired)"));

	// Events
	PatrolBP.EventGraphs.Add(TEXT("BeginPlay:Initialize waypoints array, Call StartPatrol"));
	PatrolBP.EventGraphs.Add(TEXT("OnMoveCompleted:Call OnWaypointReached"));

	OutResult.GeneratedBlueprints.Add(PatrolBP);

	// Create waypoint actors
	for (int32 i = 0; i < Config.WaypointLocations.Num(); i++)
	{
		// In actual implementation, would spawn waypoint markers
		UE_LOG(LogTemp, Log, TEXT("  Waypoint %d: %s"), i, *Config.WaypointLocations[i].ToString());
	}

	OutResult.InstructionsForUser.Add(TEXT("1. Replace AI character's class with BP_PatrolAI"));
	OutResult.InstructionsForUser.Add(TEXT("2. Add waypoint locations to the WaypointLocations array"));
	OutResult.InstructionsForUser.Add(TEXT("3. Configure AI Controller to use Simple AI movement"));
	OutResult.InstructionsForUser.Add(TEXT("4. Optional: Add player detection logic to OnSeePlayer"));
	OutResult.InstructionsForUser.Add(TEXT("5. Add animations for walking and idle states"));

	OutResult.SystemDescription = FString::Printf(
		TEXT("AI Patrol System: %d waypoints, %.1f units/sec, %.1fs wait time, %s"),
		Config.WaypointLocations.Num(),
		Config.WalkSpeed,
		Config.WaitTimeAtWaypoint,
		Config.bLoopPath ? TEXT("looping") : TEXT("reverse"));

	OutResult.bSuccess = true;
	return true;
}

bool FGameLogicGenerationSystem::GenerateQuestSystem(
	const FQuestConfig& Config,
	UWorld* World,
	FGameLogicResult& OutResult)
{
	if (!World)
	{
		OutResult.ErrorMessage = TEXT("Invalid world");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GameLogic: Generating quest system '%s'"), *Config.QuestName);

	// Game Instance component for quest management
	FGeneratedBlueprint QuestManagerBP;
	QuestManagerBP.BlueprintName = TEXT("BP_QuestManager");
	QuestManagerBP.ParentClass = UObject::StaticClass();

	// Variables
	QuestManagerBP.Variables.Add(TEXT("ActiveQuests:Array<Quest>"));
	QuestManagerBP.Variables.Add(TEXT("CompletedQuests:Array<Quest>"));

	// Functions
	QuestManagerBP.Functions.Add(TEXT("StartQuest:Add quest to active list, Show notification, Update UI"));
	QuestManagerBP.Functions.Add(TEXT("UpdateObjective:Mark objective complete, Check if quest complete"));
	QuestManagerBP.Functions.Add(TEXT("CompleteQuest:Move to completed, Give rewards, Show notification"));
	QuestManagerBP.Functions.Add(TEXT("GetActiveQuests:Return active quests for UI display"));

	OutResult.GeneratedBlueprints.Add(QuestManagerBP);

	// Quest data structure
	FGeneratedBlueprint QuestStructBP;
	QuestStructBP.BlueprintName = TEXT("Quest_") + Config.QuestName.Replace(TEXT(" "), TEXT("_"));
	for (int32 i = 0; i < Config.ObjectiveDescriptions.Num(); i++)
	{
		QuestStructBP.Variables.Add(FString::Printf(TEXT("Objective%d:String:%s"), i, *Config.ObjectiveDescriptions[i]));
		QuestStructBP.Variables.Add(FString::Printf(TEXT("Objective%dComplete:bool:false"), i));
	}

	OutResult.GeneratedBlueprints.Add(QuestStructBP);

	// UI Widget
	FGeneratedBlueprint QuestUIBP;
	QuestUIBP.BlueprintName = TEXT("WBP_QuestTracker");
	QuestUIBP.ParentClass = UUserWidget::StaticClass();
	QuestUIBP.Variables.Add(TEXT("QuestList:Array<Quest>"));
	QuestUIBP.Functions.Add(TEXT("RefreshQuestList:Get active quests from manager, Update UI"));

	OutResult.GeneratedBlueprints.Add(QuestUIBP);

	OutResult.InstructionsForUser.Add(TEXT("1. Add BP_QuestManager to your Game Instance"));
	OutResult.InstructionsForUser.Add(TEXT("2. Design WBP_QuestTracker widget with quest list"));
	OutResult.InstructionsForUser.Add(TEXT("3. Call StartQuest when player accepts quest"));
	OutResult.InstructionsForUser.Add(TEXT("4. Call UpdateObjective when objectives are completed"));
	OutResult.InstructionsForUser.Add(TEXT("5. Add quest reward logic to CompleteQuest function"));

	OutResult.SystemDescription = FString::Printf(
		TEXT("Quest System: '%s' with %d objectives"),
		*Config.QuestName,
		Config.ObjectiveDescriptions.Num());

	OutResult.bSuccess = true;
	return true;
}

bool FGameLogicGenerationSystem::GenerateInventorySystem(
	int32 MaxSlots,
	UWorld* World,
	FGameLogicResult& OutResult)
{
	UE_LOG(LogTemp, Log, TEXT("GameLogic: Generating inventory system (%d slots)"), MaxSlots);

	// Inventory component
	FGeneratedBlueprint InventoryBP;
	InventoryBP.BlueprintName = TEXT("BP_InventoryComponent");
	InventoryBP.ParentClass = UActorComponent::StaticClass();

	// Variables
	InventoryBP.Variables.Add(TEXT("Items:Array<Item>"));
	InventoryBP.Variables.Add(FString::Printf(TEXT("MaxSlots:int32:%d"), MaxSlots));
	InventoryBP.Variables.Add(TEXT("Gold:int32:0"));

	// Functions
	InventoryBP.Functions.Add(TEXT("AddItem:Check if space available, Add to Items array, Update UI"));
	InventoryBP.Functions.Add(TEXT("RemoveItem:Remove from Items, Update UI"));
	InventoryBP.Functions.Add(TEXT("UseItem:Call item's Use function, Remove if consumable"));
	InventoryBP.Functions.Add(TEXT("IsFull:Return Items.Num >= MaxSlots"));
	InventoryBP.Functions.Add(TEXT("HasItem:Search for item by name/ID"));

	OutResult.GeneratedBlueprints.Add(InventoryBP);

	// Item base class
	FGeneratedBlueprint ItemBP;
	ItemBP.BlueprintName = TEXT("BP_ItemBase");
	ItemBP.ParentClass = UObject::StaticClass();
	ItemBP.Variables.Add(TEXT("ItemName:String"));
	ItemBP.Variables.Add(TEXT("ItemIcon:Texture2D"));
	ItemBP.Variables.Add(TEXT("Description:String"));
	ItemBP.Variables.Add(TEXT("bIsConsumable:bool:false"));
	ItemBP.Variables.Add(TEXT("Value:int32:0"));
	ItemBP.Functions.Add(TEXT("Use:Override in child classes for item-specific behavior"));

	OutResult.GeneratedBlueprints.Add(ItemBP);

	// UI Widget
	FGeneratedBlueprint InventoryUIBP;
	InventoryUIBP.BlueprintName = TEXT("WBP_Inventory");
	InventoryUIBP.ParentClass = UUserWidget::StaticClass();
	InventoryUIBP.Variables.Add(TEXT("ItemSlots:Array<ItemSlotWidget>"));
	InventoryUIBP.Functions.Add(TEXT("RefreshInventory:Update all slots from inventory component"));
	InventoryUIBP.Functions.Add(TEXT("OnItemClicked:Show item details, Enable use/drop buttons"));

	OutResult.GeneratedBlueprints.Add(InventoryUIBP);

	OutResult.InstructionsForUser.Add(TEXT("1. Add BP_InventoryComponent to your player character"));
	OutResult.InstructionsForUser.Add(TEXT("2. Create child Blueprints of BP_ItemBase for each item type"));
	OutResult.InstructionsForUser.Add(TEXT("3. Design WBP_Inventory with grid of item slots"));
	OutResult.InstructionsForUser.Add(TEXT("4. Bind inventory key (e.g., 'I') to toggle WBP_Inventory visibility"));
	OutResult.InstructionsForUser.Add(TEXT("5. Implement Use function for consumable items (health potions, etc.)"));

	OutResult.SystemDescription = FString::Printf(
		TEXT("Inventory System: %d slots with add/remove/use functionality"),
		MaxSlots);

	OutResult.bSuccess = true;
	return true;
}

bool FGameLogicGenerationSystem::GenerateWaveSpawner(
	const FVector& SpawnLocation,
	int32 WaveCount,
	int32 EnemiesPerWave,
	float TimeBetweenWaves,
	UWorld* World,
	FGameLogicResult& OutResult)
{
	UE_LOG(LogTemp, Log, TEXT("GameLogic: Generating wave spawner (%d waves, %d enemies each)"),
		WaveCount, EnemiesPerWave);

	FGeneratedBlueprint SpawnerBP;
	SpawnerBP.BlueprintName = TEXT("BP_WaveSpawner");
	SpawnerBP.ParentClass = AActor::StaticClass();

	// Variables
	SpawnerBP.Variables.Add(TEXT("SpawnLocation:Vector:") + SpawnLocation.ToString());
	SpawnerBP.Variables.Add(FString::Printf(TEXT("TotalWaves:int32:%d"), WaveCount));
	SpawnerBP.Variables.Add(FString::Printf(TEXT("EnemiesPerWave:int32:%d"), EnemiesPerWave));
	SpawnerBP.Variables.Add(FString::Printf(TEXT("TimeBetweenWaves:float:%.1f"), TimeBetweenWaves));
	SpawnerBP.Variables.Add(TEXT("CurrentWave:int32:0"));
	SpawnerBP.Variables.Add(TEXT("EnemiesAlive:int32:0"));
	SpawnerBP.Variables.Add(TEXT("EnemyClass:Class<Actor>"));

	// Functions
	SpawnerBP.Functions.Add(TEXT("StartWaves:Reset counters, Call SpawnWave"));
	SpawnerBP.Functions.Add(TEXT("SpawnWave:Increment CurrentWave, Spawn EnemiesPerWave enemies, Update UI"));
	SpawnerBP.Functions.Add(TEXT("OnEnemyDied:Decrement EnemiesAlive, Check if wave complete"));
	SpawnerBP.Functions.Add(TEXT("OnWaveComplete:Wait TimeBetweenWaves, Call SpawnWave or OnAllWavesComplete"));
	SpawnerBP.Functions.Add(TEXT("OnAllWavesComplete:Show victory screen, Give rewards"));

	// Events
	SpawnerBP.EventGraphs.Add(TEXT("BeginPlay:Wait for game start signal, Call StartWaves"));

	OutResult.GeneratedBlueprints.Add(SpawnerBP);

	// UI Widget
	FGeneratedBlueprint WaveUIBP;
	WaveUIBP.BlueprintName = TEXT("WBP_WaveCounter");
	WaveUIBP.ParentClass = UUserWidget::StaticClass();
	WaveUIBP.Variables.Add(TEXT("CurrentWave:int32"));
	WaveUIBP.Variables.Add(TEXT("TotalWaves:int32"));
	WaveUIBP.Variables.Add(TEXT("EnemiesRemaining:int32"));

	OutResult.GeneratedBlueprints.Add(WaveUIBP);

	OutResult.InstructionsForUser.Add(TEXT("1. Set EnemyClass to your enemy Blueprint"));
	OutResult.InstructionsForUser.Add(TEXT("2. Place BP_WaveSpawner at spawn location"));
	OutResult.InstructionsForUser.Add(TEXT("3. In enemy Blueprint, call OnEnemyDied when health reaches 0"));
	OutResult.InstructionsForUser.Add(TEXT("4. Design WBP_WaveCounter to show wave progress"));
	OutResult.InstructionsForUser.Add(TEXT("5. Optional: Increase difficulty each wave (more enemies, faster, stronger)"));

	OutResult.SystemDescription = FString::Printf(
		TEXT("Wave Spawner: %d waves of %d enemies, %.1fs between waves"),
		WaveCount, EnemiesPerWave, TimeBetweenWaves);

	OutResult.bSuccess = true;
	return true;
}

bool FGameLogicGenerationSystem::GenerateCollectibleSystem(
	const TArray<AActor*>& CollectibleActors,
	const FString& CollectibleType,
	UWorld* World,
	FGameLogicResult& OutResult)
{
	UE_LOG(LogTemp, Log, TEXT("GameLogic: Generating collectible system (%s, %d items)"),
		*CollectibleType, CollectibleActors.Num());

	FGeneratedBlueprint CollectibleBP;
	CollectibleBP.BlueprintName = TEXT("BP_Collectible_") + CollectibleType;
	CollectibleBP.ParentClass = AActor::StaticClass();

	// Components
	FGeneratedComponent TriggerVolume;
	TriggerVolume.ComponentName = TEXT("CollectionTrigger");
	TriggerVolume.ComponentClass = USphereComponent::StaticClass();
	TriggerVolume.Properties.Add(TEXT("SphereRadius"), TEXT("50.0"));
	CollectibleBP.Components.Add(TriggerVolume);

	// Variables
	CollectibleBP.Variables.Add(TEXT("CollectibleType:String:") + CollectibleType);
	CollectibleBP.Variables.Add(TEXT("Value:int32:1"));
	CollectibleBP.Variables.Add(TEXT("bAutoCollect:bool:true"));

	// Functions
	CollectibleBP.Functions.Add(TEXT("OnCollected:Add to player inventory/score, Play collection effect, Destroy self"));

	// Events
	CollectibleBP.EventGraphs.Add(TEXT("OnActorBeginOverlap:Check if player, Call OnCollected"));

	OutResult.GeneratedBlueprints.Add(CollectibleBP);

	// Collection manager
	FGeneratedBlueprint ManagerBP;
	ManagerBP.BlueprintName = TEXT("BP_CollectionManager");
	ManagerBP.Variables.Add(TEXT("TotalCollectibles:int32:") + FString::FromInt(CollectibleActors.Num()));
	ManagerBP.Variables.Add(TEXT("CollectedCount:int32:0"));
	ManagerBP.Functions.Add(TEXT("OnItemCollected:Increment count, Update UI, Check if all collected"));
	ManagerBP.Functions.Add(TEXT("OnAllCollected:Show completion message, Give reward"));

	OutResult.GeneratedBlueprints.Add(ManagerBP);

	OutResult.InstructionsForUser.Add(TEXT("1. Replace collectible actors with BP_Collectible instances"));
	OutResult.InstructionsForUser.Add(TEXT("2. Add BP_CollectionManager to level"));
	OutResult.InstructionsForUser.Add(TEXT("3. Configure Value for each collectible (coins=1, gems=10, etc.)"));
	OutResult.InstructionsForUser.Add(TEXT("4. Add collection sound and particle effects"));
	OutResult.InstructionsForUser.Add(TEXT("5. Create UI to show collected count (e.g., 'Coins: 5/20')"));

	OutResult.SystemDescription = FString::Printf(
		TEXT("Collectible System: %d %s items with auto-collection"),
		CollectibleActors.Num(), *CollectibleType);

	OutResult.bSuccess = true;
	return true;
}

EGameLogicType FGameLogicGenerationSystem::ClassifyGameLogic(const FString& Description)
{
	FString Lower = Description.ToLower();

	if (Lower.Contains(TEXT("capture")) && (Lower.Contains(TEXT("point")) || Lower.Contains(TEXT("zone"))))
		return EGameLogicType::CapturePoint;

	if (Lower.Contains(TEXT("door")) || Lower.Contains(TEXT("open")) || Lower.Contains(TEXT("close")))
		return EGameLogicType::DoorInteraction;

	if (Lower.Contains(TEXT("patrol")) || Lower.Contains(TEXT("waypoint")) || Lower.Contains(TEXT("ai")))
		return EGameLogicType::PatrolAI;

	if (Lower.Contains(TEXT("quest")) || Lower.Contains(TEXT("objective")) || Lower.Contains(TEXT("mission")))
		return EGameLogicType::QuestSystem;

	if (Lower.Contains(TEXT("inventory")) || Lower.Contains(TEXT("item")) || Lower.Contains(TEXT("pickup")))
		return EGameLogicType::InventorySystem;

	if (Lower.Contains(TEXT("wave")) || Lower.Contains(TEXT("spawn")))
		return EGameLogicType::WaveSpawner;

	if (Lower.Contains(TEXT("collect")) || Lower.Contains(TEXT("coin")) || Lower.Contains(TEXT("pickup")))
		return EGameLogicType::Collectible;

	if (Lower.Contains(TEXT("dialog")) || Lower.Contains(TEXT("conversation")))
		return EGameLogicType::DialogSystem;

	if (Lower.Contains(TEXT("combat")) || Lower.Contains(TEXT("health")) || Lower.Contains(TEXT("damage")))
		return EGameLogicType::CombatSystem;

	return EGameLogicType::CapturePoint; // Default
}

bool FGameLogicGenerationSystem::ExtractConfiguration(
	const FString& Description,
	EGameLogicType LogicType,
	TMap<FString, FString>& OutConfig)
{
	// Parse description for configuration values
	// This would use more sophisticated NLP in production

	FString Lower = Description.ToLower();

	// Extract team count
	if (Lower.Contains(TEXT("2 teams")) || Lower.Contains(TEXT("two teams")))
		OutConfig.Add(TEXT("teams"), TEXT("2"));
	else if (Lower.Contains(TEXT("3 teams")) || Lower.Contains(TEXT("three teams")))
		OutConfig.Add(TEXT("teams"), TEXT("3"));

	// Extract time values
	FRegexPattern TimePattern(TEXT("(\\d+)\\s*(second|sec|minute|min)"));
	FRegexMatcher Matcher(TimePattern, Description);
	if (Matcher.FindNext())
	{
		OutConfig.Add(TEXT("time"), Matcher.GetCaptureGroup(1));
	}

	return true;
}

float FGameLogicGenerationSystem::ExtractNumericValue(
	const FString& Description,
	const FString& Keyword,
	float DefaultValue)
{
	// Simple numeric extraction
	// In production, would use proper NLP parsing

	FString Lower = Description.ToLower();
	int32 KeywordIndex = Lower.Find(Keyword.ToLower());

	if (KeywordIndex != INDEX_NONE)
	{
		// Look for number after keyword
		FString AfterKeyword = Description.Mid(KeywordIndex + Keyword.Len());
		FRegexPattern Pattern(TEXT("\\d+(\\.\\d+)?"));
		FRegexMatcher Matcher(Pattern, AfterKeyword);

		if (Matcher.FindNext())
		{
			return FCString::Atof(*Matcher.GetCaptureGroup(0));
		}
	}

	return DefaultValue;
}

bool FGameLogicGenerationSystem::ExtractBooleanValue(
	const FString& Description,
	const FString& Keyword,
	bool DefaultValue)
{
	FString Lower = Description.ToLower();
	FString LowerKeyword = Keyword.ToLower();

	if (Lower.Contains(LowerKeyword))
		return true;

	// Check for negations
	if (Lower.Contains(TEXT("not ") + LowerKeyword) || Lower.Contains(TEXT("no ") + LowerKeyword))
		return false;

	return DefaultValue;
}

FString FGameLogicGenerationSystem::ExtractStringValue(
	const FString& Description,
	const FString& Keyword,
	const FString& DefaultValue)
{
	FString Lower = Description.ToLower();
	int32 KeywordIndex = Lower.Find(Keyword.ToLower());

	if (KeywordIndex != INDEX_NONE)
	{
		// Extract quoted string after keyword
		FString AfterKeyword = Description.Mid(KeywordIndex + Keyword.Len());
		int32 QuoteStart = AfterKeyword.Find(TEXT("\""));
		if (QuoteStart != INDEX_NONE)
		{
			int32 QuoteEnd = AfterKeyword.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, QuoteStart + 1);
			if (QuoteEnd != INDEX_NONE)
			{
				return AfterKeyword.Mid(QuoteStart + 1, QuoteEnd - QuoteStart - 1);
			}
		}
	}

	return DefaultValue;
}

UClass* FGameLogicGenerationSystem::GenerateBlueprintClass(
	const FGeneratedBlueprint& BlueprintInfo,
	UWorld* World)
{
	// This would use Kismet2 functions to actually create Blueprint class
	// Simplified for now - full implementation would create real Blueprint

	UE_LOG(LogTemp, Log, TEXT("GameLogic: Would generate Blueprint '%s' with %d components, %d variables"),
		*BlueprintInfo.BlueprintName,
		BlueprintInfo.Components.Num(),
		BlueprintInfo.Variables.Num());

	// In full implementation:
	// UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(...);
	// Add components, variables, functions, event graphs

	return nullptr; // Would return generated class
}

bool FGameLogicGenerationSystem::AddComponentToBlueprint(
	UBlueprint* Blueprint,
	const FGeneratedComponent& ComponentInfo)
{
	if (!Blueprint)
		return false;

	// Use FBlueprintEditorUtils to add component
	// Simplified for now

	UE_LOG(LogTemp, Log, TEXT("GameLogic: Would add component '%s' of type %s"),
		*ComponentInfo.ComponentName,
		*ComponentInfo.ComponentClass->GetName());

	return true;
}

bool FGameLogicGenerationSystem::GenerateEventGraph(
	UBlueprint* Blueprint,
	const FString& EventDescription)
{
	if (!Blueprint)
		return false;

	// Use UK2Node classes to generate event graph nodes
	// Simplified for now

	UE_LOG(LogTemp, Log, TEXT("GameLogic: Would generate event graph: %s"), *EventDescription);

	return true;
}
