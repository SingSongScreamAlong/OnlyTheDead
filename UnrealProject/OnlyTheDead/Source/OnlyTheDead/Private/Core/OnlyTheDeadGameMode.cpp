// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

#include "Core/OnlyTheDeadGameMode.h"
#include "Characters/VerdunSoldierCharacter.h"
#include "Kismet/GameplayStatics.h"

AOnlyTheDeadGameMode::AOnlyTheDeadGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = AVerdunSoldierCharacter::StaticClass();

	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;

	// Initialize timeline - start at Day 1 (Feb 21, 1916)
	CurrentBattleDay = 1;
	CurrentPhase = EVerdunBattlePhase::Phase1_February;
	TimeAcceleration = 10.0f; // 1 real hour = 10 game hours
	CurrentTimeOfDay = 6.0f; // Start at 6 AM

	// Initialize rotation system
	CurrentRotation = ERotationPosition::FrontLine;
	DaysRemainingInRotation = 7; // 7 days front line
	TotalRotationsCompleted = 0;

	// Permadeath settings
	bPermadeathEnabled = true;
	DeathCount = 0;

	// Mission state
	CurrentMissionID = TEXT("");
	bMissionActive = false;

	// Difficulty
	DifficultyLevel = TEXT("Normal");
	ShellFrequencyMultiplier = 1.0f;
	SurvivalDepletionMultiplier = 1.0f;

	// Statistics
	TotalShellsSurvived = 0;
	TotalMissionsCompleted = 0;
	TotalDistanceWalked = 0.0f;

	// Autosave
	AutosaveInterval = 300.0f; // 5 minutes
	TimeSinceLastAutosave = 0.0f;
}

void AOnlyTheDeadGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("Only The Dead - Game Started"));
	UE_LOG(LogTemp, Log, TEXT("Battle of Verdun - Day %d - %s"), CurrentBattleDay, *GetCurrentDateString());
	UE_LOG(LogTemp, Log, TEXT("Current Rotation: Front Line - %d days remaining"), DaysRemainingInRotation);
}

void AOnlyTheDeadGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Advance time
	float TimeAdvancement = (DeltaTime / 3600.0f) * TimeAcceleration; // Convert to hours with acceleration
	CurrentTimeOfDay += TimeAdvancement;

	// Handle day transitions
	if (CurrentTimeOfDay >= 24.0f)
	{
		CurrentTimeOfDay -= 24.0f;
		CurrentBattleDay++;
		DaysRemainingInRotation--;

		UE_LOG(LogTemp, Log, TEXT("New Day: Day %d - %s"), CurrentBattleDay, *GetCurrentDateString());

		// Check for rotation change
		if (DaysRemainingInRotation <= 0)
		{
			AdvanceRotation();
		}

		// Update battle phase
		UpdateBattlePhase();
	}

	// Handle autosave
	HandleAutosave(DeltaTime);
}

FString AOnlyTheDeadGameMode::GetCurrentDateString() const
{
	// Calculate date from Feb 21, 1916 + CurrentBattleDay
	// Simple implementation - actual implementation should use proper date library
	int32 Day = 21 + (CurrentBattleDay - 1);
	int32 Month = 2; // February
	int32 Year = 1916;

	// Handle month transitions (simplified)
	const int32 DaysInMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // 1916 was a leap year

	while (Day > DaysInMonth[Month - 1])
	{
		Day -= DaysInMonth[Month - 1];
		Month++;
		if (Month > 12)
		{
			Month = 1;
			Year++;
		}
	}

	const TCHAR* MonthNames[] = {
		TEXT("January"), TEXT("February"), TEXT("March"), TEXT("April"),
		TEXT("May"), TEXT("June"), TEXT("July"), TEXT("August"),
		TEXT("September"), TEXT("October"), TEXT("November"), TEXT("December")
	};

	return FString::Printf(TEXT("%s %d, %d"), MonthNames[Month - 1], Day, Year);
}

void AOnlyTheDeadGameMode::AdvanceTime(float Hours)
{
	CurrentTimeOfDay += Hours;

	while (CurrentTimeOfDay >= 24.0f)
	{
		CurrentTimeOfDay -= 24.0f;
		CurrentBattleDay++;
		DaysRemainingInRotation--;

		if (DaysRemainingInRotation <= 0)
		{
			AdvanceRotation();
		}
	}

	UpdateBattlePhase();
}

void AOnlyTheDeadGameMode::AdvanceRotation()
{
	TotalRotationsCompleted++;

	// Rotation cycle: Front Line -> Support -> Rest -> Front Line
	switch (CurrentRotation)
	{
		case ERotationPosition::FrontLine:
			CurrentRotation = ERotationPosition::Support;
			DaysRemainingInRotation = 7; // 7 days support
			UE_LOG(LogTemp, Log, TEXT("ROTATION CHANGE: Moving to Support Line (7 days)"));
			break;

		case ERotationPosition::Support:
			CurrentRotation = ERotationPosition::Rest;
			DaysRemainingInRotation = 7; // 7 days rest
			UE_LOG(LogTemp, Log, TEXT("ROTATION CHANGE: Moving to Rest Area (7 days)"));
			break;

		case ERotationPosition::Rest:
			CurrentRotation = ERotationPosition::FrontLine;
			DaysRemainingInRotation = 7; // 7 days front line
			UE_LOG(LogTemp, Log, TEXT("ROTATION CHANGE: Returning to Front Line (7 days)"));
			break;
	}

	OnRotationChange.Broadcast();
}

int32 AOnlyTheDeadGameMode::GetRotationDuration(ERotationPosition Rotation) const
{
	// Standard rotation is 7 days per position
	return 7;
}

void AOnlyTheDeadGameMode::HandlePlayerDeath(const FString& CauseOfDeath)
{
	DeathCount++;

	UE_LOG(LogTemp, Error, TEXT("===== PLAYER DEATH ====="));
	UE_LOG(LogTemp, Error, TEXT("Cause: %s"), *CauseOfDeath);
	UE_LOG(LogTemp, Error, TEXT("Survived: %d days"), CurrentBattleDay - 1);
	UE_LOG(LogTemp, Error, TEXT("Rotations Completed: %d"), TotalRotationsCompleted);
	UE_LOG(LogTemp, Error, TEXT("Missions Completed: %d"), TotalMissionsCompleted);
	UE_LOG(LogTemp, Error, TEXT("========================"));

	OnPlayerDeath.Broadcast(CauseOfDeath);

	if (bPermadeathEnabled)
	{
		// Show death screen, statistics, then offer to restart
		UE_LOG(LogTemp, Warning, TEXT("Permadeath enabled - Playthrough ended"));

		// In a full implementation, this would trigger the death screen UI
		// For now, we'll just log it
	}
	else
	{
		// Respawn or reload checkpoint
		UE_LOG(LogTemp, Warning, TEXT("Permadeath disabled - Reloading checkpoint"));
	}
}

void AOnlyTheDeadGameMode::RestartPlaythrough()
{
	UE_LOG(LogTemp, Log, TEXT("Restarting playthrough from beginning..."));

	// Reset all game state
	CurrentBattleDay = 1;
	CurrentPhase = EVerdunBattlePhase::Phase1_February;
	CurrentTimeOfDay = 6.0f;
	CurrentRotation = ERotationPosition::FrontLine;
	DaysRemainingInRotation = 7;
	TotalRotationsCompleted = 0;
	TotalShellsSurvived = 0;
	TotalMissionsCompleted = 0;
	TotalDistanceWalked = 0.0f;

	// Reload the level
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void AOnlyTheDeadGameMode::StartMission(const FString& MissionID)
{
	CurrentMissionID = MissionID;
	bMissionActive = true;

	UE_LOG(LogTemp, Log, TEXT("Mission Started: %s"), *MissionID);
}

void AOnlyTheDeadGameMode::CompleteMission(bool bSuccess)
{
	if (!bMissionActive)
	{
		return;
	}

	if (bSuccess)
	{
		TotalMissionsCompleted++;
		UE_LOG(LogTemp, Log, TEXT("Mission Completed: %s (Total: %d)"), *CurrentMissionID, TotalMissionsCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission Completed with Failure: %s"), *CurrentMissionID);
	}

	bMissionActive = false;
	CurrentMissionID = TEXT("");
}

void AOnlyTheDeadGameMode::FailMission(const FString& FailureReason)
{
	if (!bMissionActive)
	{
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Mission Failed: %s - Reason: %s"), *CurrentMissionID, *FailureReason);

	bMissionActive = false;
	CurrentMissionID = TEXT("");

	// Mission failure might trigger specific consequences
}

void AOnlyTheDeadGameMode::TriggerAutosave()
{
	UE_LOG(LogTemp, Log, TEXT("Autosave triggered - Day %d, %s"), CurrentBattleDay, *GetCurrentDateString());

	// In full implementation, this would save game state to disk
	// For now, just log it

	TimeSinceLastAutosave = 0.0f;
}

void AOnlyTheDeadGameMode::UpdateBattlePhase()
{
	EVerdunBattlePhase NewPhase = CurrentPhase;

	// Phase transitions based on historical timeline
	if (CurrentBattleDay >= 1 && CurrentBattleDay <= 14)
	{
		NewPhase = EVerdunBattlePhase::Phase1_February;
	}
	else if (CurrentBattleDay >= 15 && CurrentBattleDay <= 48)
	{
		NewPhase = EVerdunBattlePhase::Phase2_March;
	}
	else if (CurrentBattleDay >= 49 && CurrentBattleDay <= 98)
	{
		NewPhase = EVerdunBattlePhase::Phase3_April;
	}
	else if (CurrentBattleDay >= 99 && CurrentBattleDay <= 141)
	{
		NewPhase = EVerdunBattlePhase::Phase4_MayJune;
	}
	else if (CurrentBattleDay >= 142 && CurrentBattleDay <= 196)
	{
		NewPhase = EVerdunBattlePhase::Phase5_Summer;
	}
	else if (CurrentBattleDay >= 197 && CurrentBattleDay <= 303)
	{
		NewPhase = EVerdunBattlePhase::Phase6_Fall;
	}

	if (NewPhase != CurrentPhase)
	{
		CurrentPhase = NewPhase;
		UE_LOG(LogTemp, Warning, TEXT("===== BATTLE PHASE CHANGE ====="));
		UE_LOG(LogTemp, Warning, TEXT("New Phase: %d"), (int32)NewPhase);
		OnPhaseChange.Broadcast(NewPhase);
	}
}

void AOnlyTheDeadGameMode::HandleAutosave(float DeltaTime)
{
	TimeSinceLastAutosave += DeltaTime;

	if (TimeSinceLastAutosave >= AutosaveInterval)
	{
		TriggerAutosave();
	}
}
