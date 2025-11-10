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
 *
 * PERSISTENT WORLD SUPPORT:
 * - Saves all 20+ region degradation states (trees, buildings, craters)
 * - Saves thousands of shell crater positions
 * - Saves environmental transformation over 303 days
 * - Saves mission trigger states (which missions activated in world)
 * - Player position in world coordinates (not mission-based)
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

    // ========================================================================
    // PERSISTENT WORLD: Mission Trigger States
    // ========================================================================

    /** Missions that have been activated (player entered trigger zone) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign|PersistentWorld")
    TArray<FString> ActivatedMissionIDs;

    /** Mission statuses in persistent world (by MissionID) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|Campaign|PersistentWorld")
    TMap<FString, EMissionStatus> MissionTriggerStates;
};

// ========================================================================
// PERSISTENT WORLD SAVE DATA
// ========================================================================

/** Saved crater data for persistent world */
USTRUCT(BlueprintType)
struct FSavedCrater
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    float DiameterMeters = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    float DepthMeters = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    EShellType ShellTypeThatCreated;
};

/** Saved region degradation state */
USTRUCT(BlueprintType)
struct FSavedRegionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    FString RegionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    float DegradationLevel = 0.0f; // 0.0 = pristine, 1.0 = Zone Rouge

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    int32 TreesRemaining = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    int32 BuildingsRemaining = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    int32 CratersCreated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    int32 ShellImpactsReceived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    uint8 CurrentEnvironmentState; // Cast to EEnvironmentState
};

/** Complete persistent world save data */
USTRUCT(BlueprintType)
struct FPersistentWorldSaveData
{
    GENERATED_BODY()

    // ========================================================================
    // ENVIRONMENTAL DEGRADATION
    // ========================================================================

    /** All region degradation states (20+ regions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    TArray<FSavedRegionState> RegionStates;

    /** Total shell impacts across entire battlefield */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    int32 TotalShellImpacts = 0;

    /** Historical degradation events that have been triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    TArray<FString> TriggeredDegradationEvents;

    // ========================================================================
    // CRATER DATA (Optimized Storage)
    // ========================================================================

    /**
     * Saved crater positions (limited to most recent/important)
     * Full crater data would be HUGE (50,000+ by Day 303)
     * We save:
     * - All craters from last 7 days (for visible degradation)
     * - Sample of older craters (for persistent landscape)
     * Total: ~5,000 craters saved (manageable size)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    TArray<FSavedCrater> SavedCraters;

    /** Crater count per region (for statistics/degradation calculation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    TMap<FString, int32> CraterCountPerRegion;

    // ========================================================================
    // DESTROYED OBJECTS
    // ========================================================================

    /** Trees destroyed (positions, for visual removal on load) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    TArray<FVector> DestroyedTreePositions;

    /** Buildings destroyed (by ID/name) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    TArray<FString> DestroyedBuildingIDs;

    // ========================================================================
    // WEATHER & TIME
    // ========================================================================

    /** Current weather state per region */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    TMap<FString, uint8> RegionWeatherStates; // Cast to EWeatherCondition

    /** Mud level per region (0.0 = dry, 1.0 = impassable) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save|PersistentWorld")
    TMap<FString, float> RegionMudLevels;
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

    // ========================================================================
    // PERSISTENT WORLD DATA
    // ========================================================================

    /** Complete persistent world state (environmental degradation, craters, etc.) */
    UPROPERTY(VisibleAnywhere, Category = "Save|PersistentWorld")
    FPersistentWorldSaveData PersistentWorldData;
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
