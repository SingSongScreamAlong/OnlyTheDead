// Only The Dead - Save System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "../core/GameTypes.h"
#include "SaveSystem.generated.h"

/**
 * Save system with support for manual saves, autosaves, and permadeath
 * Handles campaign progress, player state, and difficulty modes
 */

USTRUCT(BlueprintType)
struct FPlayerSaveData
{
    GENERATED_BODY()

    // Survival Meters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Survival")
    TMap<ESurvivalMeterType, float> SurvivalMeterValues;

    // Health
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Health")
    TArray<FInjury> ActiveInjuries;

    // Inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Inventory")
    TArray<FInventoryItem> InventoryItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Inventory")
    EWeaponType EquippedPrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Inventory")
    int32 PrimaryAmmoCount = 0;

    // Position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Position")
    FVector PlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Position")
    FRotator PlayerRotation;
};

USTRUCT(BlueprintType)
struct FCampaignSaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign")
    FHistoricalDate CurrentDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign")
    FString CurrentMissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign")
    EMissionStatus CurrentMissionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign")
    ERotationPhase CurrentRotationPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign")
    int32 DaysSurvived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign")
    int32 MissionsCompleted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign")
    TArray<FString> CompletedMissionIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign")
    TArray<FString> FailedMissionIDs;
};

UCLASS()
class ONLYTHEDEAD_API UOnlyTheDeadSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UOnlyTheDeadSaveGame();

    // ========================================================================
    // SAVE METADATA
    // ========================================================================

    UPROPERTY(VisibleAnywhere, Category = "Save")
    FString SaveSlotName;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    int32 SaveIndex = 0;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    FDateTime SaveTimestamp;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    float TotalPlayTimeHours = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    EDifficultyMode Difficulty;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    bool bIsPermadeathMode = false;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    bool bIsIronmanSave = false;

    // ========================================================================
    // SAVE DATA
    // ========================================================================

    UPROPERTY(VisibleAnywhere, Category = "Save")
    FPlayerSaveData PlayerData;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    FCampaignSaveData CampaignData;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    TMap<FString, FPlayerSkill> PlayerSkills;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    TArray<FString> UnlockedAchievements;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    TMap<FString, int32> IntegerStats;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    TMap<FString, float> FloatStats;
};

UCLASS()
class ONLYTHEDEAD_API USaveSystem : public UObject
{
    GENERATED_BODY()

public:
    // ========================================================================
    // SAVE OPERATIONS
    // ========================================================================

    /** Save game to slot */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static bool SaveGameToSlot(const FString& SlotName, int32 UserIndex = 0);

    /** Load game from slot */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static UOnlyTheDeadSaveGame* LoadGameFromSlot(const FString& SlotName, int32 UserIndex = 0);

    /** Delete save slot */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static bool DeleteSaveSlot(const FString& SlotName, int32 UserIndex = 0);

    /** Check if save slot exists */
    UFUNCTION(BlueprintPure, Category = "Save")
    static bool DoesSaveSlotExist(const FString& SlotName, int32 UserIndex = 0);

    // ========================================================================
    // AUTOSAVE
    // ========================================================================

    /** Create autosave */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static bool CreateAutosave();

    /** Load most recent autosave */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static UOnlyTheDeadSaveGame* LoadMostRecentAutosave();

    /** Get autosave slot name */
    UFUNCTION(BlueprintPure, Category = "Save")
    static FString GetAutosaveSlotName();

    // ========================================================================
    // CHECKPOINT SYSTEM
    // ========================================================================

    /** Create checkpoint (start of mission, start of day, etc.) */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static bool CreateCheckpoint(const FString& CheckpointID);

    /** Load checkpoint */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static UOnlyTheDeadSaveGame* LoadCheckpoint(const FString& CheckpointID);

    // ========================================================================
    // PERMADEATH & IRONMAN
    // ========================================================================

    /** Start new ironman campaign (single save, permadeath) */
    UFUNCTION(BlueprintCallable, Category = "Save|Permadeath")
    static bool StartIronmanCampaign(EDifficultyMode Difficulty);

    /** Delete ironman save on death */
    UFUNCTION(BlueprintCallable, Category = "Save|Permadeath")
    static void OnPlayerDeathPermadeath();

    /** Check if current save is ironman */
    UFUNCTION(BlueprintPure, Category = "Save|Permadeath")
    static bool IsIronmanMode();

    // ========================================================================
    // SAVE MANAGEMENT
    // ========================================================================

    /** Get all save slots */
    UFUNCTION(BlueprintPure, Category = "Save")
    static TArray<FString> GetAllSaveSlots();

    /** Get save metadata without loading full save */
    UFUNCTION(BlueprintPure, Category = "Save")
    static bool GetSaveMetadata(const FString& SlotName, FDateTime& OutTimestamp, float& OutPlayTime, int32& OutDaysSurvived);

    /** Copy save to new slot */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static bool CopySaveToSlot(const FString& SourceSlot, const FString& DestinationSlot);

protected:
    static UOnlyTheDeadSaveGame* CreateNewSaveGame(EDifficultyMode Difficulty, bool bIsIronman);
    static void PopulateSaveData(UOnlyTheDeadSaveGame* SaveGame);
    static void ApplySaveData(const UOnlyTheDeadSaveGame* SaveGame);
};
