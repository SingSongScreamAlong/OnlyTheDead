// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OnlyTheDeadGameMode.generated.h"

/**
 * Enum for current battle phase at Verdun
 */
UENUM(BlueprintType)
enum class EVerdunBattlePhase : uint8
{
	Phase1_February		UMETA(DisplayName = "Phase 1: Initial German Assault (Feb 21 - Mar 6)"),
	Phase2_March		UMETA(DisplayName = "Phase 2: Fort Douaumont Falls (Mar 7 - Apr 9)"),
	Phase3_April		UMETA(DisplayName = "Phase 3: Mort-Homme Battle (Apr 10 - May 29)"),
	Phase4_MayJune		UMETA(DisplayName = "Phase 4: Fort Vaux Siege (May 30 - Jul 11)"),
	Phase5_Summer		UMETA(DisplayName = "Phase 5: Summer Stalemate (Jul 12 - Sep 3)"),
	Phase6_Fall			UMETA(DisplayName = "Phase 6: French Counter-Offensive (Sep 4 - Dec 18)")
};

/**
 * Enum for rotation position
 */
UENUM(BlueprintType)
enum class ERotationPosition : uint8
{
	FrontLine	UMETA(DisplayName = "Front Line"),
	Support		UMETA(DisplayName = "Support Line"),
	Rest		UMETA(DisplayName = "Rest Area")
};

/**
 * Game Mode for Only The Dead
 * Manages permadeath, mission progression, rotation system, and historical timeline
 */
UCLASS()
class ONLYTHEDEAD_API AOnlyTheDeadGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOnlyTheDeadGameMode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ===== BATTLE TIMELINE =====

	/** Current date at Verdun (days since Feb 21, 1916) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	int32 CurrentBattleDay;

	/** Current battle phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	EVerdunBattlePhase CurrentPhase;

	/** Time acceleration multiplier (1.0 = real time, 10.0 = 1 hour = 6 minutes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	float TimeAcceleration;

	/** Current time of day (0-24 hours) */
	UPROPERTY(BlueprintReadOnly, Category = "Timeline")
	float CurrentTimeOfDay;

	/** Get formatted date string */
	UFUNCTION(BlueprintPure, Category = "Timeline")
	FString GetCurrentDateString() const;

	/** Advance time by specified hours */
	UFUNCTION(BlueprintCallable, Category = "Timeline")
	void AdvanceTime(float Hours);

	// ===== ROTATION SYSTEM =====

	/** Current rotation position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	ERotationPosition CurrentRotation;

	/** Days remaining in current rotation */
	UPROPERTY(BlueprintReadOnly, Category = "Rotation")
	int32 DaysRemainingInRotation;

	/** Total rotations completed */
	UPROPERTY(BlueprintReadOnly, Category = "Rotation")
	int32 TotalRotationsCompleted;

	/** Advance to next rotation */
	UFUNCTION(BlueprintCallable, Category = "Rotation")
	void AdvanceRotation();

	/** Get rotation duration in days */
	UFUNCTION(BlueprintPure, Category = "Rotation")
	int32 GetRotationDuration(ERotationPosition Rotation) const;

	// ===== PERMADEATH SYSTEM =====

	/** Is permadeath enabled? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permadeath")
	bool bPermadeathEnabled;

	/** Number of deaths in current playthrough */
	UPROPERTY(BlueprintReadOnly, Category = "Permadeath")
	int32 DeathCount;

	/** Handle player death */
	UFUNCTION(BlueprintCallable, Category = "Permadeath")
	void HandlePlayerDeath(const FString& CauseOfDeath);

	/** Restart playthrough from beginning */
	UFUNCTION(BlueprintCallable, Category = "Permadeath")
	void RestartPlaythrough();

	// ===== MISSION SYSTEM =====

	/** Current mission ID */
	UPROPERTY(BlueprintReadWrite, Category = "Mission")
	FString CurrentMissionID;

	/** Is mission currently active? */
	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	bool bMissionActive;

	/** Start a mission */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void StartMission(const FString& MissionID);

	/** Complete current mission */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void CompleteMission(bool bSuccess);

	/** Fail current mission */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void FailMission(const FString& FailureReason);

	// ===== DIFFICULTY SETTINGS =====

	/** Difficulty level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	FString DifficultyLevel;

	/** Shell frequency multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float ShellFrequencyMultiplier;

	/** Survival meter depletion multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float SurvivalDepletionMultiplier;

	// ===== STATISTICS =====

	/** Total shells survived */
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalShellsSurvived;

	/** Total missions completed */
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalMissionsCompleted;

	/** Total distance walked (meters) */
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float TotalDistanceWalked;

	// ===== AUTOSAVE =====

	/** Autosave interval in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float AutosaveInterval;

	/** Time since last autosave */
	UPROPERTY()
	float TimeSinceLastAutosave;

	/** Trigger autosave */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void TriggerAutosave();

	// ===== DELEGATES =====

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeath, FString, CauseOfDeath);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDeath OnPlayerDeath;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotationChange);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRotationChange OnRotationChange;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChange, EVerdunBattlePhase, NewPhase);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhaseChange OnPhaseChange;

private:
	/** Update battle phase based on current day */
	void UpdateBattlePhase();

	/** Handle autosave logic */
	void HandleAutosave(float DeltaTime);
};
