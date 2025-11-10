// Only The Dead - AI Behavior System Implementation
// Copyright 2025. All Rights Reserved.

#include "AIBehaviorSystem.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

AAIBehaviorController::AAIBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentBehaviorState = EAIBehaviorState::Idle;
    AIRole = EAIRole::Rifleman;
    CurrentMorale = 75.0f;
    TimeInCurrentState = 0.0f;
    ShellsExperiencedInLastMinute = 0;
    CurrentTarget = nullptr;
    SquadLeader = nullptr;
}

void AAIBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: AI initialized. Role: %d, Morale: %.0f"),
           (int32)AIRole,
           CurrentMorale);
}

void AAIBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeInCurrentState += DeltaTime;

    UpdateBehaviorLogic(DeltaTime);
    UpdateMorale(DeltaTime);
    CheckForThreats();

    // Update shell exposure tracking
    ShellExposureTimestamps.RemoveAll([this](float Timestamp) {
        return (UGameplayStatics::GetRealTimeSeconds(GetWorld()) - Timestamp) > 60.0f;
    });
    ShellsExperiencedInLastMinute = ShellExposureTimestamps.Num();
}

// ========================================================================
// BEHAVIOR STATE
// ========================================================================

void AAIBehaviorController::TransitionToBehaviorState(EAIBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState)
    {
        return;
    }

    EAIBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    TimeInCurrentState = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: State transition %d → %d"),
           (int32)OldState,
           (int32)NewState);

    // State-specific initialization
    switch (NewState)
    {
        case EAIBehaviorState::Combat:
            // Prepare for combat
            break;

        case EAIBehaviorState::Fleeing:
            // Drop heavy equipment, run
            break;

        case EAIBehaviorState::ShellShock:
            TriggerShellShockBehavior();
            break;

        case EAIBehaviorState::SeekingShelter:
            SeekShelterFromBombardment();
            break;

        default:
            break;
    }
}

// ========================================================================
// COMBAT BEHAVIOR
// ========================================================================

void AAIBehaviorController::EngageTarget(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    CurrentTarget = Target;
    LastKnownEnemyLocation = Target->GetActorLocation();
    TransitionToBehaviorState(EAIBehaviorState::Combat);

    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Engaging target"));

    // Move toward firing position
    // Aim at target
    // Fire weapon based on role
}

void AAIBehaviorController::TakeCover()
{
    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Taking cover"));

    TransitionToBehaviorState(EAIBehaviorState::Suppressed);

    // Find nearest cover point
    // Move to cover
    // Stay low
}

void AAIBehaviorController::SuppressPosition(FVector TargetLocation)
{
    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Suppressing position"));

    // Machine gunners and riflemen can suppress
    if (AIRole == EAIRole::MachineGunner || AIRole == EAIRole::Rifleman)
    {
        // Fire at general area to keep enemies pinned
        LastKnownEnemyLocation = TargetLocation;
    }
}

void AAIBehaviorController::ThrowGrenade(FVector TargetLocation)
{
    if (AIRole != EAIRole::Grenadier && AIRole != EAIRole::Rifleman)
    {
        return; // Only certain roles have grenades
    }

    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Throwing grenade at target"));

    // Calculate throw arc
    // Throw grenade
}

// ========================================================================
// ARTILLERY REACTION
// ========================================================================

void AAIBehaviorController::ReactToIncomingShell(FVector ImpactLocation, EShellType ShellType)
{
    ShellExposureTimestamps.Add(UGameplayStatics::GetRealTimeSeconds(GetWorld()));

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    float DistanceToImpact = FVector::Dist(ControlledPawn->GetActorLocation(), ImpactLocation);

    // Morale impact based on distance
    if (DistanceToImpact < 1000.0f) // 10 meters
    {
        ModifyMorale(-20.0f);
        UE_LOG(LogTemp, Warning, TEXT("AIBehaviorController: CLOSE CALL with shell! Morale: %.0f"), CurrentMorale);
    }
    else if (DistanceToImpact < 5000.0f) // 50 meters
    {
        ModifyMorale(-5.0f);
    }

    // Check for panic
    if (ShouldPanicFromArtillery(ShellsExperiencedInLastMinute))
    {
        if (CurrentMorale < 30.0f)
        {
            TransitionToBehaviorState(EAIBehaviorState::Fleeing);
        }
        else
        {
            TransitionToBehaviorState(EAIBehaviorState::SeekingShelter);
        }
    }

    // Experienced soldiers are more resilient
    if (ShellExposureTimestamps.Num() > 50)
    {
        // Veteran - less panic, but still affected
    }
}

void AAIBehaviorController::SeekShelterFromBombardment()
{
    NearestShelter = FindNearestShelter();

    if (NearestShelter)
    {
        UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Seeking shelter from bombardment"));

        // Move to shelter
        MoveToActor(NearestShelter, 100.0f); // Get within 1 meter

        TransitionToBehaviorState(EAIBehaviorState::SeekingShelter);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIBehaviorController: No shelter found! Taking cover in place"));
        TakeCover();
    }
}

bool AAIBehaviorController::ShouldPanicFromArtillery(int32 NearbyShellsInLastMinute) const
{
    // Historical: Artillery caused mass panic and shell shock

    // High bombardment intensity = higher panic chance
    if (NearbyShellsInLastMinute > 20) // Heavy bombardment
    {
        // Panic probability based on morale
        float PanicChance = (100.0f - CurrentMorale) / 100.0f;
        return FMath::FRand() < PanicChance;
    }

    return false;
}

void AAIBehaviorController::TriggerShellShockBehavior()
{
    UE_LOG(LogTemp, Error, TEXT("AIBehaviorController: SHELL SHOCKED! AI incapacitated"));

    // Shell shock behaviors:
    // - Frozen in place (catatonic)
    // - Wandering aimlessly
    // - Unresponsive to orders
    // - Trembling, crying

    TransitionToBehaviorState(EAIBehaviorState::ShellShock);

    // Remain in shell shock for random duration (30 sec - 5 min)
    float ShellShockDuration = FMath::RandRange(30.0f, 300.0f);
}

// ========================================================================
// SQUAD BEHAVIOR
// ========================================================================

void AAIBehaviorController::FollowSquadLeader()
{
    if (!SquadLeader)
    {
        return;
    }

    // Follow leader at safe distance
    float FollowDistance = 500.0f; // 5 meters

    MoveToActor(SquadLeader, FollowDistance);

    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Following squad leader"));
}

void AAIBehaviorController::MaintainFormation(FVector FormationOffset)
{
    if (!SquadLeader)
    {
        return;
    }

    FVector TargetPosition = SquadLeader->GetActorLocation() + FormationOffset;

    MoveToLocation(TargetPosition, 50.0f);
}

void AAIBehaviorController::CheckOnWoundedComrade(AActor* WoundedActor)
{
    if (!WoundedActor)
    {
        return;
    }

    // Medics prioritize helping wounded
    if (AIRole == EAIRole::Medic)
    {
        UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Medic moving to wounded comrade"));
        MoveToActor(WoundedActor, 100.0f);
        // Apply medical treatment when close
    }
    else
    {
        // Non-medics may drag wounded to safety
        UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Checking on wounded comrade"));
    }
}

void AAIBehaviorController::RallyNearbySoldiers(float Radius)
{
    // Officers can rally nearby soldiers
    if (AIRole != EAIRole::Officer)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Officer rallying nearby soldiers"));

    // Find nearby AI
    // Boost their morale
    // Give orders
}

// ========================================================================
// SURVIVAL BEHAVIOR
// ========================================================================

void AAIBehaviorController::SeekSupplies()
{
    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Seeking supplies (food/water)"));

    // Find nearest supply cache
    // Move to it
    // Resupply
}

void AAIBehaviorController::RestWhenSafe()
{
    if (!IsInShelter())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Resting in shelter"));

    TransitionToBehaviorState(EAIBehaviorState::Idle);

    // Regenerate stamina
    // Restore morale slowly
}

float AAIBehaviorController::GetCurrentMorale() const
{
    return CurrentMorale;
}

void AAIBehaviorController::ModifyMorale(float DeltaMorale)
{
    CurrentMorale += DeltaMorale;
    CurrentMorale = FMath::Clamp(CurrentMorale, 0.0f, 100.0f);

    if (CurrentMorale <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("AIBehaviorController: Morale broken! AI fleeing"));
        TransitionToBehaviorState(EAIBehaviorState::Fleeing);
    }
}

// ========================================================================
// COMMUNICATION
// ========================================================================

void AAIBehaviorController::ShoutWarning(const FString& WarningMessage)
{
    UE_LOG(LogTemp, Warning, TEXT("AIBehaviorController: SHOUTING - %s"), *WarningMessage);

    // Play warning voice line
    // Alert nearby soldiers
}

void AAIBehaviorController::RequestOrders()
{
    if (!SquadLeader)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Requesting orders from squad leader"));

    // Send message to squad leader
    // Wait for response
}

void AAIBehaviorController::ReportSituation(const FString& Report)
{
    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Reporting - %s"), *Report);

    // Send situation report up chain of command
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void AAIBehaviorController::UpdateBehaviorLogic(float DeltaTime)
{
    // Update behavior based on current state

    switch (CurrentBehaviorState)
    {
        case EAIBehaviorState::Idle:
            // Look around, maintain watch
            if (SquadLeader)
            {
                FollowSquadLeader();
            }
            break;

        case EAIBehaviorState::Patrol:
            // Continue patrol route
            break;

        case EAIBehaviorState::Combat:
            // Engage target if visible
            if (CurrentTarget && CurrentTarget->IsValidLowLevel())
            {
                // Continue combat
            }
            else
            {
                TransitionToBehaviorState(EAIBehaviorState::Idle);
            }
            break;

        case EAIBehaviorState::Suppressed:
            // Stay in cover, wait for suppression to end
            if (TimeInCurrentState > 10.0f)
            {
                TransitionToBehaviorState(EAIBehaviorState::Combat);
            }
            break;

        case EAIBehaviorState::Fleeing:
            // Run away from danger
            break;

        case EAIBehaviorState::ShellShock:
            // Incapacitated
            if (TimeInCurrentState > 60.0f)
            {
                // Slowly recover
                if (FMath::FRand() < 0.1f) // 10% chance per second after 60s
                {
                    TransitionToBehaviorState(EAIBehaviorState::Idle);
                    UE_LOG(LogTemp, Log, TEXT("AIBehaviorController: Recovering from shell shock"));
                }
            }
            break;

        case EAIBehaviorState::SeekingShelter:
            // Check if reached shelter
            if (IsInShelter())
            {
                RestWhenSafe();
            }
            break;

        case EAIBehaviorState::Wounded:
            // Call for medic
            // Wait for treatment
            break;

        case EAIBehaviorState::Dead:
            // Do nothing
            break;
    }
}

void AAIBehaviorController::UpdateMorale(float DeltaTime)
{
    // Morale naturally regenerates slowly when safe
    if (IsInShelter() && CurrentBehaviorState != EAIBehaviorState::Combat)
    {
        ModifyMorale(1.0f * DeltaTime); // +1 per second when resting
    }

    // Squad leader presence boosts morale
    if (SquadLeader && SquadLeader->IsValidLowLevel())
    {
        float DistanceToLeader = FVector::Dist(GetPawn()->GetActorLocation(), SquadLeader->GetActorLocation());
        if (DistanceToLeader < 1000.0f) // Within 10 meters
        {
            ModifyMorale(0.5f * DeltaTime); // Small boost
        }
    }
}

void AAIBehaviorController::CheckForThreats()
{
    // Scan for enemies
    // Check for incoming shells
    // Watch for dangers
}

AActor* AAIBehaviorController::FindNearestShelter()
{
    // Find nearest dugout, bunker, or crater
    // For now, simplified

    TArray<AActor*> FoundShelters;
    // Would use gameplay tags or actor types to find shelters

    if (FoundShelters.Num() > 0)
    {
        return FoundShelters[0];
    }

    return nullptr;
}

bool AAIBehaviorController::IsInShelter() const
{
    // Check if AI is currently in a shelter
    // Would use overlap volumes or distance checks

    return false; // Simplified
}

// ============================================================================
// PERSISTENT WORLD - NAVIGATION
// ============================================================================

void UAIBehaviorSystem::NavigateToWorldLocation(FVector TargetLocation, float AcceptanceRadius)
{
    // TODO: Full implementation
    // Use UE5 navigation system for long-distance pathfinding
    // Handle navigation across streaming cells
    // Account for craters, destroyed buildings, mud
    UE_LOG(LogTemp, Log, TEXT("AIBehaviorSystem: Navigating to world location %s"), *TargetLocation.ToString());
}

void UAIBehaviorSystem::FollowPlayerAcrossRegions(AActor* PlayerActor)
{
    // TODO: Full implementation
    // Follow player with regional awareness
    // Update CurrentRegionID as player crosses region boundaries
    // Maintain formation distance
    if (PlayerActor)
    {
        NavigateToWorldLocation(PlayerActor->GetActorLocation(), 500.0f);
    }
}

void UAIBehaviorSystem::SetPatrolRouteFromGPS(TArray<FVector2D> GPS_Waypoints)
{
    // TODO: Full implementation
    // Convert GPS coordinates to world space using MissionSystem::ConvertGPSToWorldSpace
    // Set PatrolRoute array
    // Start patrolling waypoints
    PatrolRoute.Empty();
    for (const FVector2D& GPS : GPS_Waypoints)
    {
        FVector WorldPos = FVector(GPS.X * 100000.0f, GPS.Y * 100000.0f, 0.0f); // Placeholder conversion
        PatrolRoute.Add(WorldPos);
    }
    UE_LOG(LogTemp, Log, TEXT("AIBehaviorSystem: Set patrol route with %d waypoints"), PatrolRoute.Num());
}

FVector UAIBehaviorSystem::FindNearestCraterCover(FVector FromLocation, float SearchRadius) const
{
    // TODO: Full implementation
    // Query EnvironmentDegradationSystem for craters near FromLocation
    // Find crater within SearchRadius
    // Return crater center position (provides cover from artillery)
    return FVector::ZeroVector;
}

bool UAIBehaviorSystem::IsLocationNavigable(FVector Location) const
{
    // TODO: Full implementation
    // Check if location is in crater (may be navigable at edges)
    // Check if location is in destroyed building rubble
    // Check navigation mesh validity
    // Query mud level from WeatherSystem (high mud = not navigable)
    return true; // Placeholder: assume navigable
}

float UAIBehaviorSystem::GetTerrainDegradationAtLocation(FVector Location) const
{
    // TODO: Full implementation
    // Query EnvironmentDegradationSystem for region degradation at Location
    // Return degradation level (0.0 = pristine, 1.0 = Zone Rouge)
    // Affects AI movement speed: pristine = 1.0x, devastated = 0.5x
    return 0.0f;
}

void UAIBehaviorSystem::UpdateRegionAwareness()
{
    // TODO: Full implementation
    // Query current region from EnvironmentDegradationSystem
    // Update CurrentRegionID
    // Adapt AI behavior based on region state:
    //   - Pristine regions: normal behavior
    //   - Devastated regions: more cautious, seek crater cover more often
    //   - Zone Rouge: extreme caution, hallucinations possible
    UE_LOG(LogTemp, Log, TEXT("AIBehaviorSystem: Updating region awareness"));
}
