// Only The Dead - Mission System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "MissionSystem.generated.h"

/**
 * Mission system managing 303-day campaign structure
 * Implements Noria rotation system (Front → Support → Rest)
 * Tracks player progression through Battle of Verdun timeline
 */

UENUM(BlueprintType)
enum class EMissionStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EMissionType : uint8
{
    Tutorial            UMETA(DisplayName = "Tutorial"),
    FrontLineRotation   UMETA(DisplayName = "Front Line Rotation"),
    SupportRotation     UMETA(DisplayName = "Support Line Rotation"),
    RestPeriod          UMETA(DisplayName = "Rest Period"),
    SpecialEvent        UMETA(DisplayName = "Special Event (Historical)"),
    ClimaxMission       UMETA(DisplayName = "Climax Mission")
};

USTRUCT(BlueprintType)
struct FMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FText MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FText MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FHistoricalDate MissionDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString LocationSector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 DurationDays = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FMissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EMissionStatus Status = EMissionStatus::NotStarted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsHistoricalEvent = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FText HistoricalContext;
};

USTRUCT(BlueprintType)
struct FRotationCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    int32 CycleNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    ERotationPhase CurrentPhase = ERotationPhase::FrontLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    int32 DaysInCurrentPhase = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    int32 TotalDaysInBattle = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    FString CurrentSector;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UMissionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UMissionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // MISSION DATABASE
    // ========================================================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missions")
    TMap<FString, FMissionData> MissionDatabase;

    /** Initialize mission database with all 30-35 missions */
    UFUNCTION(BlueprintCallable, Category = "Missions")
    void InitializeMissionDatabase();

    /** Get mission data by ID */
    UFUNCTION(BlueprintPure, Category = "Missions")
    FMissionData GetMissionData(const FString& MissionID) const;

    /** Get all missions for a specific date range */
    UFUNCTION(BlueprintPure, Category = "Missions")
    TArray<FMissionData> GetMissionsForDateRange(FHistoricalDate StartDate, FHistoricalDate EndDate) const;

    // ========================================================================
    // CAMPAIGN PROGRESSION
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campaign")
    FHistoricalDate CurrentGameDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campaign")
    FRotationCycle CurrentRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campaign")
    FMissionData CurrentMission;

    /** Start a new mission */
    UFUNCTION(BlueprintCallable, Category = "Missions")
    bool StartMission(const FString& MissionID);

    /** Complete current mission */
    UFUNCTION(BlueprintCallable, Category = "Missions")
    void CompleteMission(bool bSuccess);

    /** Fail current mission */
    UFUNCTION(BlueprintCallable, Category = "Missions")
    void FailMission(const FString& Reason);

    /** Advance to next mission in campaign */
    UFUNCTION(BlueprintCallable, Category = "Missions")
    void AdvanceToNextMission();

    // ========================================================================
    // ROTATION SYSTEM (NORIA)
    // ========================================================================

    /** Start a rotation cycle */
    UFUNCTION(BlueprintCallable, Category = "Rotation")
    void StartRotation(ERotationPhase Phase, const FString& SectorName, int32 DurationDays);

    /** Complete current rotation phase and advance to next */
    UFUNCTION(BlueprintCallable, Category = "Rotation")
    void AdvanceRotationPhase();

    /** Get next rotation phase in cycle */
    UFUNCTION(BlueprintPure, Category = "Rotation")
    ERotationPhase GetNextRotationPhase() const;

    /** Get days remaining in current rotation */
    UFUNCTION(BlueprintPure, Category = "Rotation")
    int32 GetDaysRemainingInRotation() const;

    /** Check if player has survived to end of battle */
    UFUNCTION(BlueprintPure, Category = "Rotation")
    bool HasSurvivedEntireBattle() const;

    // ========================================================================
    // OBJECTIVES
    // ========================================================================

    /** Update objective status */
    UFUNCTION(BlueprintCallable, Category = "Missions|Objectives")
    void UpdateObjective(const FString& ObjectiveID, bool bCompleted, bool bFailed = false);

    /** Get current objectives */
    UFUNCTION(BlueprintPure, Category = "Missions|Objectives")
    TArray<FMissionObjective> GetActiveObjectives() const;

    /** Check if all primary objectives are complete */
    UFUNCTION(BlueprintPure, Category = "Missions|Objectives")
    bool AreAllPrimaryObjectivesComplete() const;

    /** Get objective completion percentage */
    UFUNCTION(BlueprintPure, Category = "Missions|Objectives")
    float GetObjectiveCompletionPercentage() const;

    // ========================================================================
    // TIME PROGRESSION
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeCompressionFactor = 1.0f; // Real seconds per game hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bUseHistoricalTimeline = true;

    /** Advance game time by specified hours */
    UFUNCTION(BlueprintCallable, Category = "Time")
    void AdvanceGameTime(float Hours);

    /** Skip to next day */
    UFUNCTION(BlueprintCallable, Category = "Time")
    void SkipToNextDay();

    /** Get current time of day (0-24) */
    UFUNCTION(BlueprintPure, Category = "Time")
    float GetTimeOfDay() const;

    /** Check if it's night time (poor visibility) */
    UFUNCTION(BlueprintPure, Category = "Time")
    bool IsNightTime() const;

    // ========================================================================
    // HISTORICAL EVENTS
    // ========================================================================

    /** Check for historical events on current date */
    UFUNCTION(BlueprintCallable, Category = "Historical")
    void CheckForHistoricalEvents();

    /** Trigger historical event */
    UFUNCTION(BlueprintCallable, Category = "Historical")
    void TriggerHistoricalEvent(const FString& EventID);

    /** Get historical context for current date */
    UFUNCTION(BlueprintPure, Category = "Historical")
    FText GetHistoricalContextForDate(FHistoricalDate Date) const;

    // ========================================================================
    // CAMPAIGN STATISTICS
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 MissionsCompleted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 DaysSurvived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 RotationCyclesCompleted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 CloseCallsWithArtillery = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 ComradesSaved = 0;

    /** Get campaign completion percentage (days survived / 303) */
    UFUNCTION(BlueprintPure, Category = "Statistics")
    float GetCampaignCompletionPercentage() const;

protected:
    // ========================================================================
    // INTERNAL FUNCTIONS
    // ========================================================================

    void LoadMissionDatabase();
    void UpdateRotationSystem(float DeltaTime);
    void CheckMissionObjectives();
    void UpdateCampaignStatistics();

    TArray<FString> HistoricalEventsTriggered;
};
