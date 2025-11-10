// Only The Dead - Mission System Implementation
// Copyright 2025. All Rights Reserved.

#include "MissionSystem.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"

UMissionSystem::UMissionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second

    // Initialize to start of battle: February 21, 1916, 7:00 AM
    CurrentGameDate.Year = 1916;
    CurrentGameDate.Month = 2;
    CurrentGameDate.Day = 21;
    CurrentGameDate.Hour = 7;
    CurrentGameDate.Minute = 0;

    // Historical fact: Battle began 7:15 AM with German bombardment
    TimeCompressionFactor = 1.0f; // 1 real second = 1 game hour by default
    bUseHistoricalTimeline = true;

    CurrentRotation.CycleNumber = 1;
    CurrentRotation.CurrentPhase = ERotationPhase::FrontLine;
    CurrentRotation.DaysInCurrentPhase = 0;
    CurrentRotation.TotalDaysInBattle = 0;
    CurrentRotation.CurrentSector = TEXT("Bois des Caures");
}

void UMissionSystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeMissionDatabase();

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Initialized. Battle begins: Feb 21, 1916"));
    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Victory condition = Survive 303 days until Dec 18, 1916"));
}

void UMissionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Advance game time based on compression factor
    if (TimeCompressionFactor > 0.0f)
    {
        float GameHoursElapsed = DeltaTime / TimeCompressionFactor;
        AdvanceGameTime(GameHoursElapsed);
    }

    UpdateRotationSystem(DeltaTime);
    CheckMissionObjectives();
    CheckForHistoricalEvents();

    // PERSISTENT WORLD: Check if player is near any mission trigger zones
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector PlayerLocation = Owner->GetActorLocation();
        CheckProximityTriggers(PlayerLocation);
    }
}

// ========================================================================
// MISSION DATABASE
// ========================================================================

void UMissionSystem::InitializeMissionDatabase()
{
    LoadMissionDatabase();

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Loaded %d missions"), MissionDatabase.Num());
}

void UMissionSystem::LoadMissionDatabase()
{
    // Load missions from data table
    // In production, this would load from Content/Data/DT_Missions
    // For now, create some core missions programmatically
    //
    // PERSISTENT WORLD NOTE:
    // GPS coordinates from verdun_anthology/geographic_data/locations_database.csv
    // Converted to UE5 world space using ConvertGPSToWorldSpace()

    // Mission 01: The Guns Begin
    FMissionData M01;
    M01.MissionID = TEXT("M01_TheGunsBegin");
    M01.MissionTitle = FText::FromString(TEXT("The Guns Begin"));
    M01.MissionDescription = FText::FromString(TEXT("February 21, 1916. The German bombardment begins. Survive the opening salvos."));
    M01.MissionType = EMissionType::Tutorial;
    M01.MissionDate = FHistoricalDate{1916, 2, 21, 7, 15};
    M01.LocationSector = TEXT("Bois des Caures");
    M01.DurationDays = 1;
    M01.bIsHistoricalEvent = true;
    M01.HistoricalContext = FText::FromString(TEXT("At 7:15 AM, 1,400 German artillery pieces opened fire on French positions. The bombardment lasted 9 hours."));
    // GPS from locations_database.csv: Bois des Caures (49.2500, 5.4167)
    M01.GPS_Coordinates = FVector2D(49.2500, 5.4167);
    M01.MissionLocationWorld = ConvertGPSToWorldSpace(M01.GPS_Coordinates);
    M01.MissionTriggerRadiusMeters = 800.0f; // Large trigger zone for opening mission
    M01.bRequiresProximityTrigger = true;
    MissionDatabase.Add(M01.MissionID, M01);

    // Mission 02: The First Night
    FMissionData M02;
    M02.MissionID = TEXT("M02_TheFirstNight");
    M02.MissionTitle = FText::FromString(TEXT("The First Night"));
    M02.MissionDescription = FText::FromString(TEXT("Survive your first night at Verdun. Learn the sounds of shells."));
    M02.MissionType = EMissionType::FrontLineRotation;
    M02.MissionDate = FHistoricalDate{1916, 2, 21, 19, 0};
    M02.LocationSector = TEXT("Bois des Caures");
    M02.DurationDays = 1;
    M02.bIsHistoricalEvent = false;
    M02.GPS_Coordinates = FVector2D(49.2500, 5.4167); // Same location as M01
    M02.MissionLocationWorld = ConvertGPSToWorldSpace(M02.GPS_Coordinates);
    M02.MissionTriggerRadiusMeters = 500.0f;
    M02.bRequiresProximityTrigger = false; // Auto-triggers after M01 completion
    MissionDatabase.Add(M02.MissionID, M02);

    // Mission 12: The Recapture (Fort Douaumont)
    FMissionData M12;
    M12.MissionID = TEXT("M12_DouaumontRecapture");
    M12.MissionTitle = FText::FromString(TEXT("The Recapture"));
    M12.MissionDescription = FText::FromString(TEXT("October 24, 1916. Recapture Fort Douaumont. This is a climax mission."));
    M12.MissionType = EMissionType::ClimaxMission;
    M12.MissionDate = FHistoricalDate{1916, 10, 24, 11, 40};
    M12.LocationSector = TEXT("Fort Douaumont");
    M12.DurationDays = 1;
    M12.bIsHistoricalEvent = true;
    M12.HistoricalContext = FText::FromString(TEXT("French forces recaptured Fort Douaumont using creeping barrage tactics. Victory was complete by nightfall."));
    // GPS from locations_database.csv: Fort Douaumont (49.2267, 5.5167)
    M12.GPS_Coordinates = FVector2D(49.2267, 5.5167);
    M12.MissionLocationWorld = ConvertGPSToWorldSpace(M12.GPS_Coordinates);
    M12.MissionTriggerRadiusMeters = 1000.0f; // Large fort complex
    M12.bRequiresProximityTrigger = true;
    MissionDatabase.Add(M12.MissionID, M12);

    // Mission 35: The Last Day
    FMissionData M35;
    M35.MissionID = TEXT("M35_TheLastDay");
    M35.MissionTitle = FText::FromString(TEXT("The Last Day"));
    M35.MissionDescription = FText::FromString(TEXT("December 18, 1916. The battle ends. You have survived."));
    M35.MissionType = EMissionType::SpecialEvent;
    M35.MissionDate = FHistoricalDate{1916, 12, 18, 12, 0};
    M35.LocationSector = TEXT("Verdun");
    M35.DurationDays = 1;
    M35.bIsHistoricalEvent = true;
    M35.HistoricalContext = FText::FromString(TEXT("The Battle of Verdun officially ended. 700,000 casualties. The longest battle of WWI."));
    // GPS from locations_database.csv: Verdun city center (49.1600, 5.3833)
    M35.GPS_Coordinates = FVector2D(49.1600, 5.3833);
    M35.MissionLocationWorld = ConvertGPSToWorldSpace(M35.GPS_Coordinates);
    M35.MissionTriggerRadiusMeters = 2000.0f; // Large area for final mission
    M35.bRequiresProximityTrigger = false; // Auto-triggers on Dec 18, 1916
    MissionDatabase.Add(M35.MissionID, M35);

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Mission database initialized with %d missions (Persistent World Mode)"), MissionDatabase.Num());
    UE_LOG(LogTemp, Log, TEXT("MissionSystem: All missions use trigger zones in 1:1 scale Verdun map"));
}

FMissionData UMissionSystem::GetMissionData(const FString& MissionID) const
{
    if (MissionDatabase.Contains(MissionID))
    {
        return MissionDatabase[MissionID];
    }

    UE_LOG(LogTemp, Warning, TEXT("MissionSystem: Mission '%s' not found in database"), *MissionID);
    return FMissionData();
}

TArray<FMissionData> UMissionSystem::GetMissionsForDateRange(FHistoricalDate StartDate, FHistoricalDate EndDate) const
{
    TArray<FMissionData> MatchingMissions;

    for (const auto& MissionPair : MissionDatabase)
    {
        const FMissionData& Mission = MissionPair.Value;

        // Compare dates (simplified - assumes Year/Month/Day comparison)
        int32 MissionDateInt = Mission.MissionDate.Year * 10000 + Mission.MissionDate.Month * 100 + Mission.MissionDate.Day;
        int32 StartDateInt = StartDate.Year * 10000 + StartDate.Month * 100 + StartDate.Day;
        int32 EndDateInt = EndDate.Year * 10000 + EndDate.Month * 100 + EndDate.Day;

        if (MissionDateInt >= StartDateInt && MissionDateInt <= EndDateInt)
        {
            MatchingMissions.Add(Mission);
        }
    }

    return MatchingMissions;
}

// ========================================================================
// CAMPAIGN PROGRESSION
// ========================================================================

bool UMissionSystem::StartMission(const FString& MissionID)
{
    if (!MissionDatabase.Contains(MissionID))
    {
        UE_LOG(LogTemp, Error, TEXT("MissionSystem: Cannot start mission '%s' - not found"), *MissionID);
        return false;
    }

    CurrentMission = MissionDatabase[MissionID];
    CurrentMission.Status = EMissionStatus::Active;

    // Update game date to mission date if using historical timeline
    if (bUseHistoricalTimeline)
    {
        CurrentGameDate = CurrentMission.MissionDate;
    }

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Started mission '%s' - %s"),
           *MissionID,
           *CurrentMission.MissionTitle.ToString());

    // Trigger mission-specific setup
    if (CurrentMission.bIsHistoricalEvent)
    {
        UE_LOG(LogTemp, Log, TEXT("MissionSystem: HISTORICAL EVENT - %s"),
               *CurrentMission.HistoricalContext.ToString());
    }

    return true;
}

void UMissionSystem::CompleteMission(bool bSuccess)
{
    if (CurrentMission.Status != EMissionStatus::Active)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSystem: Cannot complete mission - no active mission"));
        return;
    }

    CurrentMission.Status = bSuccess ? EMissionStatus::Completed : EMissionStatus::Failed;

    if (bSuccess)
    {
        MissionsCompleted++;
        DaysSurvived += CurrentMission.DurationDays;

        UE_LOG(LogTemp, Log, TEXT("MissionSystem: Mission '%s' COMPLETED. Days survived: %d/303"),
               *CurrentMission.MissionID,
               DaysSurvived);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSystem: Mission '%s' FAILED"), *CurrentMission.MissionID);
    }

    UpdateCampaignStatistics();

    // Check victory condition
    if (HasSurvivedEntireBattle())
    {
        UE_LOG(LogTemp, Log, TEXT("MissionSystem: VICTORY! Survived all 303 days of Verdun!"));
        // Trigger victory sequence
    }
}

void UMissionSystem::FailMission(const FString& Reason)
{
    CurrentMission.Status = EMissionStatus::Failed;

    UE_LOG(LogTemp, Warning, TEXT("MissionSystem: Mission '%s' FAILED - Reason: %s"),
           *CurrentMission.MissionID,
           *Reason);

    // In permadeath mode, this could trigger game over
}

void UMissionSystem::AdvanceToNextMission()
{
    // Find next mission in chronological order
    // This is simplified - production would have more sophisticated progression

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Advancing to next mission..."));

    // For now, just log - full implementation would select next mission based on:
    // - Current date
    // - Rotation phase
    // - Historical timeline
    // - Player choices
}

// ========================================================================
// ROTATION SYSTEM (NORIA)
// ========================================================================

void UMissionSystem::StartRotation(ERotationPhase Phase, const FString& SectorName, int32 DurationDays)
{
    CurrentRotation.CurrentPhase = Phase;
    CurrentRotation.CurrentSector = SectorName;
    CurrentRotation.DaysInCurrentPhase = 0;

    FString PhaseName;
    switch (Phase)
    {
        case ERotationPhase::FrontLine: PhaseName = TEXT("FRONT LINE"); break;
        case ERotationPhase::Support: PhaseName = TEXT("SUPPORT"); break;
        case ERotationPhase::Rest: PhaseName = TEXT("REST"); break;
        default: PhaseName = TEXT("UNKNOWN"); break;
    }

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Rotation started - %s in %s (Duration: %d days)"),
           *PhaseName,
           *SectorName,
           DurationDays);
}

void UMissionSystem::AdvanceRotationPhase()
{
    ERotationPhase NextPhase = GetNextRotationPhase();

    // Complete current phase
    RotationCyclesCompleted++;

    // Historical pattern: Front Line (4-8 days) → Support (4-8 days) → Rest (4-7 days)
    int32 NextPhaseDuration = 0;
    FString NextSector;

    switch (NextPhase)
    {
        case ERotationPhase::FrontLine:
            NextPhaseDuration = FMath::RandRange(4, 8);
            NextSector = TEXT("Front Line Sector"); // Would be determined by historical data
            CurrentRotation.CycleNumber++;
            break;

        case ERotationPhase::Support:
            NextPhaseDuration = FMath::RandRange(4, 8);
            NextSector = TEXT("Support Sector");
            break;

        case ERotationPhase::Rest:
            NextPhaseDuration = FMath::RandRange(4, 7);
            NextSector = TEXT("Rest Area");
            break;
    }

    StartRotation(NextPhase, NextSector, NextPhaseDuration);

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Advanced to next rotation phase. Cycle: %d"),
           CurrentRotation.CycleNumber);
}

ERotationPhase UMissionSystem::GetNextRotationPhase() const
{
    // Historical Noria pattern: Front → Support → Rest → Front
    switch (CurrentRotation.CurrentPhase)
    {
        case ERotationPhase::FrontLine:
            return ERotationPhase::Support;
        case ERotationPhase::Support:
            return ERotationPhase::Rest;
        case ERotationPhase::Rest:
            return ERotationPhase::FrontLine;
        default:
            return ERotationPhase::FrontLine;
    }
}

int32 UMissionSystem::GetDaysRemainingInRotation() const
{
    // This would be calculated based on rotation duration - simplified here
    return FMath::Max(0, 8 - CurrentRotation.DaysInCurrentPhase);
}

bool UMissionSystem::HasSurvivedEntireBattle() const
{
    // Victory condition: Survive from Feb 21, 1916 to Dec 18, 1916 (303 days)
    return DaysSurvived >= 303;
}

// ========================================================================
// OBJECTIVES
// ========================================================================

void UMissionSystem::UpdateObjective(const FString& ObjectiveID, bool bCompleted, bool bFailed)
{
    for (FMissionObjective& Objective : CurrentMission.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            if (bCompleted)
            {
                Objective.Status = EObjectiveStatus::Completed;
                UE_LOG(LogTemp, Log, TEXT("MissionSystem: Objective '%s' COMPLETED"), *ObjectiveID);
            }
            else if (bFailed)
            {
                Objective.Status = EObjectiveStatus::Failed;
                UE_LOG(LogTemp, Warning, TEXT("MissionSystem: Objective '%s' FAILED"), *ObjectiveID);
            }
            break;
        }
    }

    CheckMissionObjectives();
}

TArray<FMissionObjective> UMissionSystem::GetActiveObjectives() const
{
    TArray<FMissionObjective> ActiveObjectives;

    for (const FMissionObjective& Objective : CurrentMission.Objectives)
    {
        if (Objective.Status == EObjectiveStatus::Active ||
            Objective.Status == EObjectiveStatus::NotStarted)
        {
            ActiveObjectives.Add(Objective);
        }
    }

    return ActiveObjectives;
}

bool UMissionSystem::AreAllPrimaryObjectivesComplete() const
{
    for (const FMissionObjective& Objective : CurrentMission.Objectives)
    {
        if (Objective.bIsPrimary && Objective.Status != EObjectiveStatus::Completed)
        {
            return false;
        }
    }

    return true;
}

float UMissionSystem::GetObjectiveCompletionPercentage() const
{
    if (CurrentMission.Objectives.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedCount = 0;
    for (const FMissionObjective& Objective : CurrentMission.Objectives)
    {
        if (Objective.Status == EObjectiveStatus::Completed)
        {
            CompletedCount++;
        }
    }

    return (float)CompletedCount / (float)CurrentMission.Objectives.Num() * 100.0f;
}

// ========================================================================
// TIME PROGRESSION
// ========================================================================

void UMissionSystem::AdvanceGameTime(float Hours)
{
    // Add hours to current time
    float TotalMinutes = CurrentGameDate.Hour * 60 + CurrentGameDate.Minute + (Hours * 60);

    while (TotalMinutes >= 24 * 60)
    {
        // Advance to next day
        TotalMinutes -= 24 * 60;
        CurrentGameDate.Day++;
        CurrentRotation.TotalDaysInBattle++;
        CurrentRotation.DaysInCurrentPhase++;

        // Handle month rollover (simplified)
        int32 DaysInMonth = 30; // Simplified
        if (CurrentGameDate.Day > DaysInMonth)
        {
            CurrentGameDate.Day = 1;
            CurrentGameDate.Month++;

            if (CurrentGameDate.Month > 12)
            {
                CurrentGameDate.Month = 1;
                CurrentGameDate.Year++;
            }
        }
    }

    CurrentGameDate.Hour = FMath::FloorToInt(TotalMinutes / 60);
    CurrentGameDate.Minute = FMath::FloorToInt(TotalMinutes) % 60;
}

void UMissionSystem::SkipToNextDay()
{
    float HoursUntilMidnight = 24.0f - GetTimeOfDay();
    AdvanceGameTime(HoursUntilMidnight);

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Skipped to next day. Date: %d/%d/%d"),
           CurrentGameDate.Month,
           CurrentGameDate.Day,
           CurrentGameDate.Year);
}

float UMissionSystem::GetTimeOfDay() const
{
    return CurrentGameDate.Hour + (CurrentGameDate.Minute / 60.0f);
}

bool UMissionSystem::IsNightTime() const
{
    // Historical: Night at Verdun (Nov) was ~5:30 PM to 7:00 AM
    // Simplified: 18:00 (6 PM) to 6:00 AM
    float TimeOfDay = GetTimeOfDay();
    return TimeOfDay >= 18.0f || TimeOfDay < 6.0f;
}

// ========================================================================
// HISTORICAL EVENTS
// ========================================================================

void UMissionSystem::CheckForHistoricalEvents()
{
    // Check if current date matches any historical events
    // This would be much more sophisticated in production

    // Example: Fort Douaumont captured by Germans (Feb 25, 1916)
    if (CurrentGameDate.Year == 1916 &&
        CurrentGameDate.Month == 2 &&
        CurrentGameDate.Day == 25 &&
        !HistoricalEventsTriggered.Contains(TEXT("FortDouaumontCaptured")))
    {
        TriggerHistoricalEvent(TEXT("FortDouaumontCaptured"));
    }

    // Example: Fort Douaumont recaptured by French (Oct 24, 1916)
    if (CurrentGameDate.Year == 1916 &&
        CurrentGameDate.Month == 10 &&
        CurrentGameDate.Day == 24 &&
        !HistoricalEventsTriggered.Contains(TEXT("FortDouaumontRecaptured")))
    {
        TriggerHistoricalEvent(TEXT("FortDouaumontRecaptured"));
    }
}

void UMissionSystem::TriggerHistoricalEvent(const FString& EventID)
{
    HistoricalEventsTriggered.Add(EventID);

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: HISTORICAL EVENT TRIGGERED - %s"), *EventID);

    // Broadcast event to game systems
    // In production, this would trigger cinematics, dialogue, mission changes
}

FText UMissionSystem::GetHistoricalContextForDate(FHistoricalDate Date) const
{
    // Return historical context for specific date
    // This would pull from extensive historical database

    if (Date.Year == 1916 && Date.Month == 2 && Date.Day == 21)
    {
        return FText::FromString(TEXT("The Battle of Verdun begins. German Operation Gericht (Judgment) launches with 1,400 artillery pieces."));
    }

    if (Date.Year == 1916 && Date.Month == 2 && Date.Day == 25)
    {
        return FText::FromString(TEXT("Fort Douaumont falls to German forces with minimal resistance. A strategic disaster for France."));
    }

    if (Date.Year == 1916 && Date.Month == 10 && Date.Day == 24)
    {
        return FText::FromString(TEXT("French forces recapture Fort Douaumont. A symbolic victory after 8 months of fighting."));
    }

    if (Date.Year == 1916 && Date.Month == 12 && Date.Day == 18)
    {
        return FText::FromString(TEXT("The Battle of Verdun officially ends. 700,000 casualties. 'They shall not pass.'"));
    }

    return FText::FromString(TEXT("The battle continues..."));
}

// ========================================================================
// CAMPAIGN STATISTICS
// ========================================================================

float UMissionSystem::GetCampaignCompletionPercentage() const
{
    return (float)DaysSurvived / 303.0f * 100.0f;
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void UMissionSystem::UpdateRotationSystem(float DeltaTime)
{
    // Check if rotation phase should advance
    // This would be more sophisticated in production

    int32 DaysRemaining = GetDaysRemainingInRotation();
    if (DaysRemaining <= 0 && CurrentMission.Status != EMissionStatus::Active)
    {
        // Automatically advance to next rotation phase
        AdvanceRotationPhase();
    }
}

void UMissionSystem::CheckMissionObjectives()
{
    if (CurrentMission.Status != EMissionStatus::Active)
    {
        return;
    }

    // Check if all primary objectives are complete
    if (AreAllPrimaryObjectivesComplete())
    {
        UE_LOG(LogTemp, Log, TEXT("MissionSystem: All primary objectives complete. Mission can be completed."));
        // In production, this would trigger mission completion UI/events
    }
}

void UMissionSystem::UpdateCampaignStatistics()
{
    // Update various campaign statistics
    // This is called after mission completion

    UE_LOG(LogTemp, Log, TEXT("MissionSystem: Campaign Statistics - Missions: %d, Days: %d/303 (%.1f%%)"),
           MissionsCompleted,
           DaysSurvived,
           GetCampaignCompletionPercentage());
}

// ========================================================================
// PERSISTENT WORLD - PROXIMITY TRIGGERS
// ========================================================================

void UMissionSystem::CheckProximityTriggers(FVector PlayerLocation)
{
    // Check all missions with proximity triggers enabled
    for (const auto& MissionPair : MissionDatabase)
    {
        const FMissionData& Mission = MissionPair.Value;

        // Skip if mission doesn't use proximity triggers
        if (!Mission.bRequiresProximityTrigger)
        {
            continue;
        }

        // Skip if mission is already active or completed
        if (Mission.Status != EMissionStatus::NotStarted)
        {
            continue;
        }

        // Check if player is within trigger radius
        if (IsPlayerInMissionTriggerZone(Mission.MissionID, PlayerLocation))
        {
            UE_LOG(LogTemp, Log, TEXT("MissionSystem: Player entered mission trigger zone - %s"), *Mission.MissionID);

            // Auto-start mission
            StartMission(Mission.MissionID);
            break; // Only trigger one mission at a time
        }
    }
}

float UMissionSystem::GetDistanceToMission(const FString& MissionID, FVector PlayerLocation) const
{
    if (!MissionDatabase.Contains(MissionID))
    {
        return -1.0f;
    }

    const FMissionData& Mission = MissionDatabase[MissionID];
    float Distance = FVector::Dist(PlayerLocation, Mission.MissionLocationWorld);

    return Distance / 100.0f; // Convert from cm to meters
}

bool UMissionSystem::IsPlayerInMissionTriggerZone(const FString& MissionID, FVector PlayerLocation) const
{
    float DistanceMeters = GetDistanceToMission(MissionID, PlayerLocation);

    if (DistanceMeters < 0.0f)
    {
        return false; // Mission not found
    }

    const FMissionData& Mission = MissionDatabase[MissionID];
    return DistanceMeters <= Mission.MissionTriggerRadiusMeters;
}

FVector UMissionSystem::ConvertGPSToWorldSpace(FVector2D GPS_Coordinates)
{
    // Convert GPS (Latitude, Longitude) to UE5 world coordinates
    //
    // Verdun battlefield extent (from geographic_data):
    // Lat: 49.0833 to 49.3833 (30 km range)
    // Lon: 5.2833 to 5.6833 (40 km range)
    //
    // UE5 World Space:
    // Origin at Verdun city center: (49.1600, 5.3833)
    // 1 degree latitude ≈ 111 km
    // 1 degree longitude ≈ 74 km (at 49° N)
    //
    // Conversion:
    // X (East-West) = (Longitude - Origin_Lon) * 74000 * 100 (cm)
    // Y (North-South) = (Latitude - Origin_Lat) * 111000 * 100 (cm)
    // Z = 0 (terrain height handled by landscape)

    const FVector2D OriginGPS(49.1600, 5.3833); // Verdun city center
    const float MetersPerDegreeLat = 111000.0f;
    const float MetersPerDegreeLon = 74000.0f;
    const float UE5_CM_PER_METER = 100.0f;

    float DeltaLat = GPS_Coordinates.X - OriginGPS.X;
    float DeltaLon = GPS_Coordinates.Y - OriginGPS.Y;

    float WorldX = DeltaLon * MetersPerDegreeLon * UE5_CM_PER_METER;
    float WorldY = DeltaLat * MetersPerDegreeLat * UE5_CM_PER_METER;
    float WorldZ = 0.0f; // Terrain height handled by landscape

    return FVector(WorldX, WorldY, WorldZ);
}
