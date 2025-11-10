// Only The Dead - Save System Implementation
// Copyright 2025. All Rights Reserved.

#include "SaveSystem.h"
#include "Kismet/GameplayStatics.h"
#include "SurvivalSystem.h"
#include "MedicalSystem.h"
#include "InventorySystem.h"
#include "MissionSystem.h"
#include "ProgressionSystem.h"

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

    UE_LOG(LogTemp, Log, TEXT("SaveSystem: Save data populated"));
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

    UE_LOG(LogTemp, Log, TEXT("SaveSystem: Save data applied to game"));
}
