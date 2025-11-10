// Only The Dead - Save System Implementation
// Copyright 2025. All Rights Reserved.

#include "SaveSystem.h"
#include "Kismet/GameplayStatics.h"
#include "SurvivalSystem.h"
#include "MedicalSystem.h"
#include "InventorySystem.h"
#include "MissionSystem.h"
#include "ProgressionSystem.h"
#include "EnvironmentDegradationSystem.h"
#include "WeatherSystem.h"

UOnlyTheDeadSaveGame::UOnlyTheDeadSaveGame()
{
    SaveSlotName = TEXT("DefaultSave");
    SaveIndex = 0;
    TotalPlayTimeHours = 0.0f;
    Difficulty = EDifficultyMode::Soldier;
    bIsPermadeathMode = false;
    bIsIronmanSave = false;
}

// ========================================================================
// SAVE OPERATIONS
// ========================================================================

bool USaveSystem::SaveGameToSlot(const FString& SlotName, int32 UserIndex)
{
    UOnlyTheDeadSaveGame* SaveGameInstance = Cast<UOnlyTheDeadSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UOnlyTheDeadSaveGame::StaticClass())
    );

    if (!SaveGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to create save game object"));
        return false;
    }

    // Populate save data
    SaveGameInstance->SaveSlotName = SlotName;
    SaveGameInstance->SaveTimestamp = FDateTime::Now();
    PopulateSaveData(SaveGameInstance);

    // Save to disk
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, UserIndex);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SaveSystem: Game saved to slot '%s'"), *SlotName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to save game to slot '%s'"), *SlotName);
    }

    return bSuccess;
}

UOnlyTheDeadSaveGame* USaveSystem::LoadGameFromSlot(const FString& SlotName, int32 UserIndex)
{
    if (!DoesSaveSlotExist(SlotName, UserIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Save slot '%s' does not exist"), *SlotName);
        return nullptr;
    }

    UOnlyTheDeadSaveGame* SaveGame = Cast<UOnlyTheDeadSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex)
    );

    if (SaveGame)
    {
        UE_LOG(LogTemp, Log, TEXT("SaveSystem: Loaded game from slot '%s'"), *SlotName);
        UE_LOG(LogTemp, Log, TEXT("SaveSystem: Days survived: %d, Play time: %.1f hours"),
               SaveGame->CampaignData.DaysSurvived,
               SaveGame->TotalPlayTimeHours);

        // Apply save data to game
        ApplySaveData(SaveGame);

        return SaveGame;
    }

    UE_LOG(LogTemp, Error, TEXT("SaveSystem: Failed to load game from slot '%s'"), *SlotName);
    return nullptr;
}

bool USaveSystem::DeleteSaveSlot(const FString& SlotName, int32 UserIndex)
{
    if (!DoesSaveSlotExist(SlotName, UserIndex))
    {
        return false;
    }

    bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SaveSystem: Deleted save slot '%s'"), *SlotName);
    }

    return bSuccess;
}

bool USaveSystem::DoesSaveSlotExist(const FString& SlotName, int32 UserIndex)
{
    return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

// ========================================================================
// AUTOSAVE
// ========================================================================

bool USaveSystem::CreateAutosave()
{
    FString AutosaveSlot = GetAutosaveSlotName();
    return SaveGameToSlot(AutosaveSlot, 0);
}

UOnlyTheDeadSaveGame* USaveSystem::LoadMostRecentAutosave()
{
    FString AutosaveSlot = GetAutosaveSlotName();
    return LoadGameFromSlot(AutosaveSlot, 0);
}

FString USaveSystem::GetAutosaveSlotName()
{
    return TEXT("Autosave_OnlyTheDead");
}

// ========================================================================
// CHECKPOINT SYSTEM
// ========================================================================

bool USaveSystem::CreateCheckpoint(const FString& CheckpointID)
{
    FString CheckpointSlot = FString::Printf(TEXT("Checkpoint_%s"), *CheckpointID);
    return SaveGameToSlot(CheckpointSlot, 0);
}

UOnlyTheDeadSaveGame* USaveSystem::LoadCheckpoint(const FString& CheckpointID)
{
    FString CheckpointSlot = FString::Printf(TEXT("Checkpoint_%s"), *CheckpointID);
    return LoadGameFromSlot(CheckpointSlot, 0);
}

// ========================================================================
// PERMADEATH & IRONMAN
// ========================================================================

bool USaveSystem::StartIronmanCampaign(EDifficultyMode Difficulty)
{
    // Create new ironman save
    UOnlyTheDeadSaveGame* IronmanSave = CreateNewSaveGame(Difficulty, true);

    if (!IronmanSave)
    {
        return false;
    }

    // Save to ironman slot
    bool bSuccess = UGameplayStatics::SaveGameToSlot(IronmanSave, TEXT("IronmanSave"), 0);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveSystem: IRONMAN campaign started. Permadeath active!"));
    }

    return bSuccess;
}

void USaveSystem::OnPlayerDeathPermadeath()
{
    UE_LOG(LogTemp, Error, TEXT("SaveSystem: Player death in permadeath mode. Deleting save..."));

    // Delete ironman save
    DeleteSaveSlot(TEXT("IronmanSave"), 0);

    UE_LOG(LogTemp, Warning, TEXT("SaveSystem: Permadeath save deleted. Campaign ended."));
}

bool USaveSystem::IsIronmanMode()
{
    if (!DoesSaveSlotExist(TEXT("IronmanSave"), 0))
    {
        return false;
    }

    UOnlyTheDeadSaveGame* SaveGame = Cast<UOnlyTheDeadSaveGame>(
        UGameplayStatics::LoadGameFromSlot(TEXT("IronmanSave"), 0)
    );

    if (SaveGame)
    {
        return SaveGame->bIsIronmanSave;
    }

    return false;
}

// ========================================================================
// SAVE MANAGEMENT
// ========================================================================

TArray<FString> USaveSystem::GetAllSaveSlots()
{
    // This would enumerate all save files on disk
    // For now, return known save slots

    TArray<FString> SaveSlots;

    if (DoesSaveSlotExist(TEXT("ManualSave1"), 0))
        SaveSlots.Add(TEXT("ManualSave1"));

    if (DoesSaveSlotExist(TEXT("ManualSave2"), 0))
        SaveSlots.Add(TEXT("ManualSave2"));

    if (DoesSaveSlotExist(TEXT("ManualSave3"), 0))
        SaveSlots.Add(TEXT("ManualSave3"));

    if (DoesSaveSlotExist(GetAutosaveSlotName(), 0))
        SaveSlots.Add(GetAutosaveSlotName());

    if (DoesSaveSlotExist(TEXT("IronmanSave"), 0))
        SaveSlots.Add(TEXT("IronmanSave"));

    return SaveSlots;
}

bool USaveSystem::GetSaveMetadata(const FString& SlotName, FDateTime& OutTimestamp, float& OutPlayTime, int32& OutDaysSurvived)
{
    if (!DoesSaveSlotExist(SlotName, 0))
    {
        return false;
    }

    UOnlyTheDeadSaveGame* SaveGame = Cast<UOnlyTheDeadSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (SaveGame)
    {
        OutTimestamp = SaveGame->SaveTimestamp;
        OutPlayTime = SaveGame->TotalPlayTimeHours;
        OutDaysSurvived = SaveGame->CampaignData.DaysSurvived;
        return true;
    }

    return false;
}

bool USaveSystem::CopySaveToSlot(const FString& SourceSlot, const FString& DestinationSlot)
{
    UOnlyTheDeadSaveGame* SourceSave = LoadGameFromSlot(SourceSlot, 0);

    if (!SourceSave)
    {
        return false;
    }

    // Save to new slot
    SourceSave->SaveSlotName = DestinationSlot;
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SourceSave, DestinationSlot, 0);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SaveSystem: Copied save from '%s' to '%s'"),
               *SourceSlot,
               *DestinationSlot);
    }

    return bSuccess;
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

UOnlyTheDeadSaveGame* USaveSystem::CreateNewSaveGame(EDifficultyMode Difficulty, bool bIsIronman)
{
    UOnlyTheDeadSaveGame* NewSave = Cast<UOnlyTheDeadSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UOnlyTheDeadSaveGame::StaticClass())
    );

    if (NewSave)
    {
        NewSave->Difficulty = Difficulty;
        NewSave->bIsIronmanSave = bIsIronman;
        NewSave->bIsPermadeathMode = bIsIronman;
        NewSave->SaveTimestamp = FDateTime::Now();
    }

    return NewSave;
}

void USaveSystem::PopulateSaveData(UOnlyTheDeadSaveGame* SaveGame)
{
    if (!SaveGame)
    {
        return;
    }

    // Get player controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GWorld, 0);
    if (!PlayerController)
    {
        return;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    // Save survival meters
    USurvivalSystem* SurvivalSystem = PlayerPawn->FindComponentByClass<USurvivalSystem>();
    if (SurvivalSystem)
    {
        for (const auto& MeterPair : SurvivalSystem->SurvivalMeters)
        {
            SaveGame->PlayerData.SurvivalMeterValues.Add(MeterPair.Key, MeterPair.Value.CurrentValue);
        }
    }

    // Save health and injuries
    UMedicalSystem* MedicalSystem = PlayerPawn->FindComponentByClass<UMedicalSystem>();
    if (MedicalSystem)
    {
        SaveGame->PlayerData.CurrentHealth = MedicalSystem->CurrentHealth;
        SaveGame->PlayerData.ActiveInjuries = MedicalSystem->ActiveInjuries;
    }

    // Save inventory
    UInventorySystem* InventorySystem = PlayerPawn->FindComponentByClass<UInventorySystem>();
    if (InventorySystem)
    {
        SaveGame->PlayerData.InventoryItems = InventorySystem->InventoryItems;
        SaveGame->PlayerData.EquippedPrimaryWeapon = InventorySystem->EquippedPrimaryWeapon;
        SaveGame->PlayerData.PrimaryAmmoCount = InventorySystem->PrimaryAmmoCount;
    }

    // Save position
    SaveGame->PlayerData.PlayerLocation = PlayerPawn->GetActorLocation();
    SaveGame->PlayerData.PlayerRotation = PlayerPawn->GetActorRotation();

    // Save campaign progress
    UMissionSystem* MissionSystem = PlayerPawn->FindComponentByClass<UMissionSystem>();
    if (MissionSystem)
    {
        SaveGame->CampaignData.CurrentDate = MissionSystem->CurrentGameDate;
        SaveGame->CampaignData.CurrentMissionID = MissionSystem->CurrentMission.MissionID;
        SaveGame->CampaignData.CurrentMissionStatus = MissionSystem->CurrentMission.Status;
        SaveGame->CampaignData.CurrentRotationPhase = MissionSystem->CurrentRotation.CurrentPhase;
        SaveGame->CampaignData.DaysSurvived = MissionSystem->DaysSurvived;
        SaveGame->CampaignData.MissionsCompleted = MissionSystem->MissionsCompleted;
    }

    // Save progression
    UProgressionSystem* ProgressionSystem = PlayerPawn->FindComponentByClass<UProgressionSystem>();
    if (ProgressionSystem)
    {
        SaveGame->PlayerSkills = ProgressionSystem->PlayerSkills;
        SaveGame->UnlockedAchievements = ProgressionSystem->UnlockedAchievements;
        SaveGame->IntegerStats = ProgressionSystem->IntegerStats;
        SaveGame->FloatStats = ProgressionSystem->FloatStats;
        SaveGame->TotalPlayTimeHours = ProgressionSystem->CampaignProgress.TotalPlayTimeHours;
        SaveGame->Difficulty = ProgressionSystem->CurrentDifficulty;
    }

    // ========================================================================
    // PERSISTENT WORLD: Save environmental degradation state
    // ========================================================================

    UEnvironmentDegradationSystem* EnvDegradationSystem = PlayerPawn->FindComponentByClass<UEnvironmentDegradationSystem>();
    if (!EnvDegradationSystem)
    {
        // Try finding it on GameState
        AGameStateBase* GameState = GWorld->GetGameState();
        if (GameState)
        {
            EnvDegradationSystem = GameState->FindComponentByClass<UEnvironmentDegradationSystem>();
        }
    }

    if (EnvDegradationSystem)
    {
        // Save all region states
        SaveGame->PersistentWorldData.RegionStates.Empty();
        for (const auto& RegionPair : EnvDegradationSystem->Regions)
        {
            const FEnvironmentRegion& Region = RegionPair.Value;

            FSavedRegionState SavedRegion;
            SavedRegion.RegionID = Region.RegionID;
            SavedRegion.DegradationLevel = Region.DegradationLevel;
            SavedRegion.TreesRemaining = Region.TreesRemaining;
            SavedRegion.BuildingsRemaining = Region.BuildingsRemaining;
            SavedRegion.CratersCreated = Region.CratersCreated;
            SavedRegion.ShellImpactsReceived = Region.ShellImpactsReceived;
            SavedRegion.CurrentEnvironmentState = (uint8)Region.CurrentState;

            SaveGame->PersistentWorldData.RegionStates.Add(SavedRegion);
        }

        // Save total shell impacts
        SaveGame->PersistentWorldData.TotalShellImpacts = EnvDegradationSystem->TotalShellImpacts;

        // Save triggered degradation events
        SaveGame->PersistentWorldData.TriggeredDegradationEvents = EnvDegradationSystem->TriggeredHistoricalEvents;

        // Save craters (limited to most recent ~5,000 for performance)
        SaveGame->PersistentWorldData.SavedCraters.Empty();
        int32 MaxCratersToSave = 5000;
        int32 CratersSaved = 0;

        for (const auto& Crater : EnvDegradationSystem->ActiveCraters)
        {
            if (CratersSaved >= MaxCratersToSave)
            {
                break;
            }

            FSavedCrater SavedCrater;
            SavedCrater.Location = Crater.Location;
            SavedCrater.DiameterMeters = Crater.DiameterMeters;
            SavedCrater.DepthMeters = Crater.DepthMeters;
            SavedCrater.ShellTypeThatCreated = Crater.ShellTypeThatCreated;

            SaveGame->PersistentWorldData.SavedCraters.Add(SavedCrater);
            CratersSaved++;
        }

        // Save crater counts per region
        SaveGame->PersistentWorldData.CraterCountPerRegion.Empty();
        for (const auto& RegionPair : EnvDegradationSystem->Regions)
        {
            SaveGame->PersistentWorldData.CraterCountPerRegion.Add(
                RegionPair.Key,
                RegionPair.Value.CratersCreated
            );
        }

        UE_LOG(LogTemp, Log, TEXT("SaveSystem: Saved %d regions, %d craters, %d total shell impacts"),
               SaveGame->PersistentWorldData.RegionStates.Num(),
               SaveGame->PersistentWorldData.SavedCraters.Num(),
               SaveGame->PersistentWorldData.TotalShellImpacts);
    }

    // Save mission trigger states (persistent world)
    if (MissionSystem)
    {
        SaveGame->CampaignData.ActivatedMissionIDs.Empty();
        SaveGame->CampaignData.MissionTriggerStates.Empty();

        for (const auto& MissionPair : MissionSystem->MissionDatabase)
        {
            const FMissionData& Mission = MissionPair.Value;

            if (Mission.Status != EMissionStatus::NotStarted)
            {
                SaveGame->CampaignData.ActivatedMissionIDs.Add(Mission.MissionID);
                SaveGame->CampaignData.MissionTriggerStates.Add(Mission.MissionID, Mission.Status);
            }
        }
    }

    // Save weather state per region
    UWeatherSystem* WeatherSystem = PlayerPawn->FindComponentByClass<UWeatherSystem>();
    if (WeatherSystem)
    {
        SaveGame->PersistentWorldData.RegionWeatherStates.Empty();
        SaveGame->PersistentWorldData.RegionMudLevels.Empty();

        // Would save regional weather/mud data here
        // For now, save global state
        SaveGame->PersistentWorldData.RegionMudLevels.Add(TEXT("Global"), WeatherSystem->MudLevel);
    }

    UE_LOG(LogTemp, Log, TEXT("SaveSystem: Save data populated (PERSISTENT WORLD MODE)"));
}

void USaveSystem::ApplySaveData(const UOnlyTheDeadSaveGame* SaveGame)
{
    if (!SaveGame)
    {
        return;
    }

    // Get player controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GWorld, 0);
    if (!PlayerController)
    {
        return;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    // Restore survival meters
    USurvivalSystem* SurvivalSystem = PlayerPawn->FindComponentByClass<USurvivalSystem>();
    if (SurvivalSystem)
    {
        for (const auto& MeterPair : SaveGame->PlayerData.SurvivalMeterValues)
        {
            SurvivalSystem->SetMeterValue(MeterPair.Key, MeterPair.Value);
        }
    }

    // Restore health and injuries
    UMedicalSystem* MedicalSystem = PlayerPawn->FindComponentByClass<UMedicalSystem>();
    if (MedicalSystem)
    {
        MedicalSystem->CurrentHealth = SaveGame->PlayerData.CurrentHealth;
        MedicalSystem->ActiveInjuries = SaveGame->PlayerData.ActiveInjuries;
    }

    // Restore inventory
    UInventorySystem* InventorySystem = PlayerPawn->FindComponentByClass<UInventorySystem>();
    if (InventorySystem)
    {
        InventorySystem->InventoryItems = SaveGame->PlayerData.InventoryItems;
        InventorySystem->EquippedPrimaryWeapon = SaveGame->PlayerData.EquippedPrimaryWeapon;
        InventorySystem->PrimaryAmmoCount = SaveGame->PlayerData.PrimaryAmmoCount;
    }

    // Restore position
    PlayerPawn->SetActorLocation(SaveGame->PlayerData.PlayerLocation);
    PlayerPawn->SetActorRotation(SaveGame->PlayerData.PlayerRotation);

    // Restore campaign progress
    UMissionSystem* MissionSystem = PlayerPawn->FindComponentByClass<UMissionSystem>();
    if (MissionSystem)
    {
        MissionSystem->CurrentGameDate = SaveGame->CampaignData.CurrentDate;
        MissionSystem->DaysSurvived = SaveGame->CampaignData.DaysSurvived;
        MissionSystem->MissionsCompleted = SaveGame->CampaignData.MissionsCompleted;
        // Note: Would need to reload mission from database
    }

    // Restore progression
    UProgressionSystem* ProgressionSystem = PlayerPawn->FindComponentByClass<UProgressionSystem>();
    if (ProgressionSystem)
    {
        ProgressionSystem->PlayerSkills = SaveGame->PlayerSkills;
        ProgressionSystem->UnlockedAchievements = SaveGame->UnlockedAchievements;
        ProgressionSystem->IntegerStats = SaveGame->IntegerStats;
        ProgressionSystem->FloatStats = SaveGame->FloatStats;
        ProgressionSystem->CurrentDifficulty = SaveGame->Difficulty;
    }

    // ========================================================================
    // PERSISTENT WORLD: Restore environmental degradation state
    // ========================================================================

    UEnvironmentDegradationSystem* EnvDegradationSystem = PlayerPawn->FindComponentByClass<UEnvironmentDegradationSystem>();
    if (!EnvDegradationSystem)
    {
        // Try finding it on GameState
        AGameStateBase* GameState = GWorld->GetGameState();
        if (GameState)
        {
            EnvDegradationSystem = GameState->FindComponentByClass<UEnvironmentDegradationSystem>();
        }
    }

    if (EnvDegradationSystem)
    {
        // Restore all region states
        EnvDegradationSystem->Regions.Empty();
        for (const FSavedRegionState& SavedRegion : SaveGame->PersistentWorldData.RegionStates)
        {
            FEnvironmentRegion RestoredRegion;
            RestoredRegion.RegionID = SavedRegion.RegionID;
            RestoredRegion.DegradationLevel = SavedRegion.DegradationLevel;
            RestoredRegion.TreesRemaining = SavedRegion.TreesRemaining;
            RestoredRegion.BuildingsRemaining = SavedRegion.BuildingsRemaining;
            RestoredRegion.CratersCreated = SavedRegion.CratersCreated;
            RestoredRegion.ShellImpactsReceived = SavedRegion.ShellImpactsReceived;
            RestoredRegion.CurrentState = (EEnvironmentState)SavedRegion.CurrentEnvironmentState;

            EnvDegradationSystem->Regions.Add(SavedRegion.RegionID, RestoredRegion);
        }

        // Restore total shell impacts
        EnvDegradationSystem->TotalShellImpacts = SaveGame->PersistentWorldData.TotalShellImpacts;

        // Restore triggered degradation events
        EnvDegradationSystem->TriggeredHistoricalEvents = SaveGame->PersistentWorldData.TriggeredDegradationEvents;

        // Restore craters
        EnvDegradationSystem->ActiveCraters.Empty();
        for (const FSavedCrater& SavedCrater : SaveGame->PersistentWorldData.SavedCraters)
        {
            FCraterData RestoredCrater;
            RestoredCrater.Location = SavedCrater.Location;
            RestoredCrater.DiameterMeters = SavedCrater.DiameterMeters;
            RestoredCrater.DepthMeters = SavedCrater.DepthMeters;
            RestoredCrater.ShellTypeThatCreated = SavedCrater.ShellTypeThatCreated;

            EnvDegradationSystem->ActiveCraters.Add(RestoredCrater);
        }

        UE_LOG(LogTemp, Log, TEXT("SaveSystem: Restored %d regions, %d craters, %d total shell impacts"),
               EnvDegradationSystem->Regions.Num(),
               EnvDegradationSystem->ActiveCraters.Num(),
               EnvDegradationSystem->TotalShellImpacts);

        // Trigger visual restoration of degraded environment
        // This would:
        // 1. Remove trees at DestroyedTreePositions
        // 2. Destroy buildings in DestroyedBuildingIDs
        // 3. Spawn crater meshes/deformations at SavedCraters positions
        // 4. Update terrain materials based on DegradationLevel per region
        EnvDegradationSystem->RestoreEnvironmentFromSave();
    }

    // Restore mission trigger states (persistent world)
    if (MissionSystem)
    {
        // Restore mission statuses
        for (const FString& ActivatedMissionID : SaveGame->CampaignData.ActivatedMissionIDs)
        {
            if (SaveGame->CampaignData.MissionTriggerStates.Contains(ActivatedMissionID))
            {
                EMissionStatus Status = SaveGame->CampaignData.MissionTriggerStates[ActivatedMissionID];

                // Update mission status in database
                if (MissionSystem->MissionDatabase.Contains(ActivatedMissionID))
                {
                    MissionSystem->MissionDatabase[ActivatedMissionID].Status = Status;
                }
            }
        }
    }

    // Restore weather state
    UWeatherSystem* WeatherSystem = PlayerPawn->FindComponentByClass<UWeatherSystem>();
    if (WeatherSystem)
    {
        // Restore mud levels
        if (SaveGame->PersistentWorldData.RegionMudLevels.Contains(TEXT("Global")))
        {
            WeatherSystem->MudLevel = SaveGame->PersistentWorldData.RegionMudLevels[TEXT("Global")];
        }
    }

    UE_LOG(LogTemp, Log, TEXT("SaveSystem: Save data applied to game (PERSISTENT WORLD MODE)"));
    UE_LOG(LogTemp, Warning, TEXT("SaveSystem: World state restored to Day %d/303"),
           SaveGame->CampaignData.DaysSurvived);
}
