// Only The Dead - Environment Degradation System Implementation
// Copyright 2025. All Rights Reserved.

#include "EnvironmentDegradationSystem.h"
#include "Kismet/GameplayStatics.h"

UEnvironmentDegradationSystem::UEnvironmentDegradationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds

    GlobalEnvironmentState = EEnvironmentState::EarlyWar;
    GlobalDegradationLevel = 0.0f;
    TimeSinceLastUpdate = 0.0f;

    TotalShellImpacts = 0;
    TotalTreesDestroyed = 0;
    TotalBuildingsDestroyed = 0;
    TotalCratersCreated = 0;
}

void UEnvironmentDegradationSystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeRegions();
    LoadHistoricalDegradationEvents();

    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Initialized. PERSISTENT WORLD"));
    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Player will witness transformation over 303 days"));
    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Verdun → Zone Rouge"));
}

void UEnvironmentDegradationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastUpdate += DeltaTime;

    UpdateRegionalDegradation(DeltaTime);
    CheckForEnvironmentMilestones();
}

// ========================================================================
// WORLD STATE
// ========================================================================

float UEnvironmentDegradationSystem::GetDegradationAtLocation(FVector Location) const
{
    // Find region containing this location
    FString RegionID = GetRegionAtLocation(Location);

    if (Regions.Contains(RegionID))
    {
        return Regions[RegionID].DegradationLevel;
    }

    // Fall back to global degradation
    return GlobalDegradationLevel;
}

FEnvironmentRegion UEnvironmentDegradationSystem::GetRegion(const FString& RegionID) const
{
    if (Regions.Contains(RegionID))
    {
        return Regions[RegionID];
    }

    return FEnvironmentRegion();
}

// ========================================================================
// DEGRADATION PROGRESSION
// ========================================================================

void UEnvironmentDegradationSystem::UpdateDegradation(FHistoricalDate CurrentDate)
{
    // Calculate global degradation based on current day (0-303)
    int32 DayOfBattle = 0; // Would calculate from Feb 21, 1916

    // Simplified: Each day = 1/303 progression
    GlobalDegradationLevel = (float)DayOfBattle / 303.0f;

    // Update global state based on day
    if (DayOfBattle < 30)
    {
        GlobalEnvironmentState = EEnvironmentState::EarlyWar;
    }
    else if (DayOfBattle < 100)
    {
        GlobalEnvironmentState = EEnvironmentState::Deteriorating;
    }
    else if (DayOfBattle < 200)
    {
        GlobalEnvironmentState = EEnvironmentState::Devastated;
    }
    else
    {
        GlobalEnvironmentState = EEnvironmentState::Apocalyptic;
    }

    // Apply historical degradation events for this date
    ApplyHistoricalDegradation(CurrentDate);

    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Day %d/303, State: %d, Degradation: %.1f%%"),
           DayOfBattle,
           (int32)GlobalEnvironmentState,
           GlobalDegradationLevel * 100.0f);
}

void UEnvironmentDegradationSystem::ApplyShellImpact(FVector ImpactLocation, EShellType ShellType)
{
    TotalShellImpacts++;

    // Find region
    FString RegionID = GetRegionAtLocation(ImpactLocation);

    if (Regions.Contains(RegionID))
    {
        FEnvironmentRegion& Region = Regions[RegionID];
        Region.ShellImpactsReceived++;

        // Increase degradation based on shell size
        float DegradationIncrease = 0.0001f; // Base increment

        switch (ShellType)
        {
            case EShellType::Shell_75mm_French:
                DegradationIncrease *= 1.0f;
                break;
            case EShellType::Shell_155mm_Heavy:
                DegradationIncrease *= 2.5f;
                break;
            case EShellType::Shell_210mm_Mortar:
                DegradationIncrease *= 4.0f;
                break;
            case EShellType::Shell_305mm_Railway:
                DegradationIncrease *= 8.0f;
                break;
            case EShellType::Shell_420mm_BigBertha:
                DegradationIncrease *= 15.0f; // Massive impact
                break;
            default:
                break;
        }

        Region.DegradationLevel += DegradationIncrease;
        Region.DegradationLevel = FMath::Clamp(Region.DegradationLevel, 0.0f, 1.0f);

        // Chance to destroy trees/buildings in blast radius
        float DestructionChance = DegradationIncrease * 10.0f;
        if (FMath::FRand() < DestructionChance && Region.TreesRemaining > 0)
        {
            DestroyTree(ImpactLocation);
            Region.TreesRemaining--;
        }

        // Create crater
        float CraterSize = 2.0f; // Base size in meters
        switch (ShellType)
        {
            case EShellType::Shell_155mm_Heavy: CraterSize = 5.0f; break;
            case EShellType::Shell_210mm_Mortar: CraterSize = 6.0f; break;
            case EShellType::Shell_305mm_Railway: CraterSize = 10.0f; break;
            case EShellType::Shell_420mm_BigBertha: CraterSize = 15.0f; break;
            default: break;
        }

        CreateCrater(ImpactLocation, CraterSize, CraterSize * 0.6f);
    }
}

void UEnvironmentDegradationSystem::ApplyBombardment(FString RegionID, EBombardmentIntensity Intensity, float DurationHours)
{
    if (!Regions.Contains(RegionID))
    {
        return;
    }

    FEnvironmentRegion& Region = Regions[RegionID];

    // Calculate shells fired based on intensity and duration
    int32 ShellsFired = 0;

    switch (Intensity)
    {
        case EBombardmentIntensity::Harassing:
            ShellsFired = FMath::RoundToInt(DurationHours * 1.5f); // 1-2 per minute
            break;
        case EBombardmentIntensity::Light:
            ShellsFired = FMath::RoundToInt(DurationHours * 7.5f * 60.0f); // 5-10 per minute
            break;
        case EBombardmentIntensity::Heavy:
            ShellsFired = FMath::RoundToInt(DurationHours * 25.0f * 60.0f); // 20-30 per minute
            break;
        case EBombardmentIntensity::Trommelfeuer:
            ShellsFired = FMath::RoundToInt(DurationHours * 100.0f * 60.0f); // 100+ per minute
            break;
        default:
            break;
    }

    Region.ShellImpactsReceived += ShellsFired;

    // Mass degradation
    float DegradationIncrease = ShellsFired * 0.00001f;
    Region.DegradationLevel += DegradationIncrease;
    Region.DegradationLevel = FMath::Clamp(Region.DegradationLevel, 0.0f, 1.0f);

    // Destroy trees proportionally
    int32 TreesDestroyed = FMath::RoundToInt(Region.TreesRemaining * DegradationIncrease * 10.0f);
    Region.TreesRemaining = FMath::Max(0, Region.TreesRemaining - TreesDestroyed);
    TotalTreesDestroyed += TreesDestroyed;

    // Damage buildings
    int32 BuildingsDamaged = FMath::RoundToInt(Region.BuildingsRemaining * DegradationIncrease * 5.0f);
    Region.BuildingsRemaining = FMath::Max(0, Region.BuildingsRemaining - BuildingsDamaged);
    TotalBuildingsDestroyed += BuildingsDamaged;

    // Create many craters
    int32 CratersCreated = ShellsFired / 10; // Not every shell creates persistent crater
    Region.CratersCreated += CratersCreated;
    TotalCratersCreated += CratersCreated;

    UE_LOG(LogTemp, Warning, TEXT("EnvironmentDegradationSystem: Bombardment of %s - %d shells, %d trees destroyed, %d craters"),
           *RegionID,
           ShellsFired,
           TreesDestroyed,
           CratersCreated);
}

void UEnvironmentDegradationSystem::DestroyTree(FVector TreeLocation)
{
    TotalTreesDestroyed++;

    // In production:
    // 1. Find tree actor/foliage instance at location
    // 2. Replace with "shattered tree" mesh
    // 3. Add debris
    // 4. Update foliage maps

    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Tree destroyed at (%.0f, %.0f)"),
           TreeLocation.X,
           TreeLocation.Y);
}

void UEnvironmentDegradationSystem::DamageBuilding(FVector BuildingLocation, float DamageAmount)
{
    // In production:
    // 1. Find building actor
    // 2. Swap to damaged mesh variant
    // 3. If damage > threshold, swap to rubble
    // 4. Spawn debris

    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Building damaged (%.0f damage)"), DamageAmount);
}

void UEnvironmentDegradationSystem::CreateCrater(FVector Location, float DiameterMeters, float DepthMeters)
{
    TotalCratersCreated++;

    // In production:
    // 1. Spawn crater mesh OR deform landscape
    // 2. Add crater to persistent crater map
    // 3. Create water puddle if rainy
    // 4. Update navigation mesh

    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Crater created - %.1fm diameter, %.1fm deep"),
           DiameterMeters,
           DepthMeters);
}

// ========================================================================
// ENVIRONMENTAL TRANSFORMATION
// ========================================================================

void UEnvironmentDegradationSystem::TransformRegion(FString RegionID, EEnvironmentState NewState)
{
    if (!Regions.Contains(RegionID))
    {
        return;
    }

    FEnvironmentRegion& Region = Regions[RegionID];
    EEnvironmentState OldState = Region.CurrentState;
    Region.CurrentState = NewState;

    UE_LOG(LogTemp, Warning, TEXT("EnvironmentDegradationSystem: %s transformed %d → %d"),
           *Region.RegionName.ToString(),
           (int32)OldState,
           (int32)NewState);

    // Apply visual transformation
    ApplyDegradationEffects(RegionID);
}

float UEnvironmentDegradationSystem::GetTransformationProgress(int32 CurrentDay) const
{
    return (float)CurrentDay / 303.0f;
}

bool UEnvironmentDegradationSystem::IsRegionZoneRouge(FString RegionID) const
{
    if (Regions.Contains(RegionID))
    {
        return Regions[RegionID].DegradationLevel >= 0.95f; // 95% = Zone Rouge
    }
    return false;
}

FText UEnvironmentDegradationSystem::GetTransformationDescription(int32 CurrentDay) const
{
    // Narrative descriptions of transformation

    if (CurrentDay < 7)
    {
        return FText::FromString(TEXT("The woods are still green, though shells have begun to bite. Buildings stand, if scarred."));
    }
    else if (CurrentDay < 30)
    {
        return FText::FromString(TEXT("Trees are shattered. The first craters have appeared. Smoke drifts across ruined villages."));
    }
    else if (CurrentDay < 100)
    {
        return FText::FromString(TEXT("The forest is gone. Only splintered stumps remain. Buildings are rubble. Mud is everywhere."));
    }
    else if (CurrentDay < 200)
    {
        return FText::FromString(TEXT("A moonscape. Craters overlap. Nothing green remains. The earth itself is dying."));
    }
    else
    {
        return FText::FromString(TEXT("Zone Rouge. A poisoned wasteland. Nothing can live here. The world has ended."));
    }
}

// ========================================================================
// REGIONAL MANAGEMENT
// ========================================================================

void UEnvironmentDegradationSystem::InitializeRegions()
{
    // Initialize all major battlefield regions
    // Historical sectors of Verdun battlefield

    // BOIS DES CAURES (Opening battle)
    FEnvironmentRegion BoisDesCaures;
    BoisDesCaures.RegionID = TEXT("BoisDesCaures");
    BoisDesCaures.RegionName = FText::FromString(TEXT("Bois des Caures"));
    BoisDesCaures.RegionCenter = FVector(0, 0, 0); // Would use real GPS coordinates converted to UE5
    BoisDesCaures.RadiusMeters = 2000.0f;
    BoisDesCaures.TreesRemaining = 5000; // Dense forest initially
    BoisDesCaures.BuildingsRemaining = 3; // Few buildings
    BoisDesCaures.bIsActiveWarZone = true; // Front line Feb 21-25
    BoisDesCaures.HistoricalNotes = TEXT("Site of first German attack. Entire forest destroyed by Feb 25.");
    Regions.Add(BoisDesCaures.RegionID, BoisDesCaures);

    // FORT DOUAUMONT
    FEnvironmentRegion Douaumont;
    Douaumont.RegionID = TEXT("FortDouaumont");
    Douaumont.RegionName = FText::FromString(TEXT("Fort Douaumont"));
    Douaumont.RegionCenter = FVector(500000, 300000, 38800); // Real coordinates
    Douaumont.RadiusMeters = 1000.0f;
    Douaumont.TreesRemaining = 200;
    Douaumont.BuildingsRemaining = 1; // The fort itself
    Douaumont.HistoricalNotes = TEXT("Largest fort. Captured Feb 25, recaptured Oct 24.");
    Regions.Add(Douaumont.RegionID, Douaumont);

    // MORT-HOMME (Dead Man's Hill)
    FEnvironmentRegion MortHomme;
    MortHomme.RegionID = TEXT("MortHomme");
    MortHomme.RegionName = FText::FromString(TEXT("Mort-Homme (Dead Man)"));
    MortHomme.RegionCenter = FVector(-200000, 100000, 29500);
    MortHomme.RadiusMeters = 1500.0f;
    MortHomme.TreesRemaining = 3000;
    MortHomme.BuildingsRemaining = 0;
    MortHomme.bIsActiveWarZone = true;
    MortHomme.HistoricalNotes = TEXT("Named 'Dead Man' before the battle. Prophetic.");
    Regions.Add(MortHomme.RegionID, MortHomme);

    // FLEURY-DEVANT-DOUAUMONT (Village)
    FEnvironmentRegion Fleury;
    Fleury.RegionID = TEXT("Fleury");
    Fleury.RegionName = FText::FromString(TEXT("Fleury-devant-Douaumont"));
    Fleury.RegionCenter = FVector(480000, 280000, 35000);
    Fleury.RadiusMeters = 800.0f;
    Fleury.TreesRemaining = 100;
    Fleury.BuildingsRemaining = 85; // Village
    Fleury.HistoricalNotes = TEXT("Changed hands 16 times. Completely destroyed. Never rebuilt.");
    Regions.Add(Fleury.RegionID, Fleury);

    // Add more regions as needed (total ~20-30 for full battlefield coverage)

    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Initialized %d regions"), Regions.Num());
}

FString UEnvironmentDegradationSystem::GetRegionAtLocation(FVector Location) const
{
    // Find closest region
    float ClosestDistance = TNumericLimits<float>::Max();
    FString ClosestRegion;

    for (const auto& RegionPair : Regions)
    {
        const FEnvironmentRegion& Region = RegionPair.Value;
        float Distance = FVector::Dist(Location, Region.RegionCenter);

        if (Distance < Region.RadiusMeters * 100.0f && Distance < ClosestDistance) // *100 for cm conversion
        {
            ClosestDistance = Distance;
            ClosestRegion = Region.RegionID;
        }
    }

    return ClosestRegion;
}

TArray<FEnvironmentRegion> UEnvironmentDegradationSystem::GetRegionsByState(EEnvironmentState State) const
{
    TArray<FEnvironmentRegion> MatchingRegions;

    for (const auto& RegionPair : Regions)
    {
        if (RegionPair.Value.CurrentState == State)
        {
            MatchingRegions.Add(RegionPair.Value);
        }
    }

    return MatchingRegions;
}

void UEnvironmentDegradationSystem::SetRegionActiveWarZone(FString RegionID, bool bIsActive)
{
    if (Regions.Contains(RegionID))
    {
        Regions[RegionID].bIsActiveWarZone = bIsActive;

        UE_LOG(LogTemp, Warning, TEXT("EnvironmentDegradationSystem: %s is %s war zone"),
               *RegionID,
               bIsActive ? TEXT("ACTIVE") : TEXT("no longer"));
    }
}

// ========================================================================
// HISTORICAL DEGRADATION EVENTS
// ========================================================================

void UEnvironmentDegradationSystem::LoadHistoricalDegradationEvents()
{
    // Load timeline of major degradation events
    // These are triggered on specific dates to match historical transformation

    // Feb 21, 1916 - Opening bombardment destroys Bois des Caures
    FDegradationEvent Event_Feb21;
    Event_Feb21.EventDate = FHistoricalDate{1916, 2, 21, 7, 15};
    Event_Feb21.RegionID = TEXT("BoisDesCaures");
    Event_Feb21.DegradationType = EDegradationType::TreeDestruction;
    Event_Feb21.Intensity = 1.0f;
    Event_Feb21.Description = FText::FromString(TEXT("9-hour bombardment. Forest obliterated."));
    Event_Feb21.bIsHistoricalEvent = true;
    HistoricalDegradationEvents.Add(Event_Feb21);

    // Feb 25, 1916 - Fort Douaumont captured, surroundings devastated
    FDegradationEvent Event_Feb25;
    Event_Feb25.EventDate = FHistoricalDate{1916, 2, 25, 16, 0};
    Event_Feb25.RegionID = TEXT("FortDouaumont");
    Event_Feb25.DegradationType = EDegradationType::BuildingDamage;
    Event_Feb25.Intensity = 0.8f;
    Event_Feb25.Description = FText::FromString(TEXT("Fort captured. Surrounding area cratered."));
    Event_Feb25.bIsHistoricalEvent = true;
    HistoricalDegradationEvents.Add(Event_Feb25);

    // June 23, 1916 - Peak of German advance, maximum devastation
    FDegradationEvent Event_Jun23;
    Event_Jun23.EventDate = FHistoricalDate{1916, 6, 23, 12, 0};
    Event_Jun23.RegionID = TEXT("Fleury");
    Event_Jun23.DegradationType = EDegradationType::BuildingDamage;
    Event_Jun23.Intensity = 1.0f;
    Event_Jun23.Description = FText::FromString(TEXT("Fleury destroyed. No building left intact."));
    Event_Jun23.bIsHistoricalEvent = true;
    HistoricalDegradationEvents.Add(Event_Jun23);

    UE_LOG(LogTemp, Log, TEXT("EnvironmentDegradationSystem: Loaded %d historical degradation events"),
           HistoricalDegradationEvents.Num());
}

void UEnvironmentDegradationSystem::ApplyHistoricalDegradation(FHistoricalDate Date)
{
    TArray<FDegradationEvent> EventsToday = GetEventsForDate(Date);

    for (const FDegradationEvent& Event : EventsToday)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentDegradationSystem: HISTORICAL EVENT - %s"),
               *Event.Description.ToString());

        // Apply degradation effect
        if (Regions.Contains(Event.RegionID))
        {
            FEnvironmentRegion& Region = Regions[Event.RegionID];
            Region.DegradationLevel += Event.Intensity * 0.2f;
            Region.DegradationLevel = FMath::Clamp(Region.DegradationLevel, 0.0f, 1.0f);
        }
    }
}

TArray<FDegradationEvent> UEnvironmentDegradationSystem::GetEventsForDate(FHistoricalDate Date) const
{
    TArray<FDegradationEvent> MatchingEvents;

    for (const FDegradationEvent& Event : HistoricalDegradationEvents)
    {
        if (Event.EventDate.Year == Date.Year &&
            Event.EventDate.Month == Date.Month &&
            Event.EventDate.Day == Date.Day)
        {
            MatchingEvents.Add(Event);
        }
    }

    return MatchingEvents;
}

// ========================================================================
// VISUAL EFFECTS
// ========================================================================

void UEnvironmentDegradationSystem::SpawnDebrisField(FVector Location, float RadiusMeters)
{
    // Spawn debris particles, broken equipment, corpses, etc.
}

void UEnvironmentDegradationSystem::UpdateFoliageDensity(FString RegionID, float NewDensity)
{
    // Update UE5 foliage system density
}

void UEnvironmentDegradationSystem::ApplyTerrainDeformation(FVector Location, float IntensityMultiplier)
{
    // Deform landscape using runtime landscape editing
}

void UEnvironmentDegradationSystem::SpawnDeadTrees(FString RegionID, int32 Count)
{
    // Replace living trees with dead/shattered variants
}

// ========================================================================
// ATMOSPHERE & AMBIENCE
// ========================================================================

FString UEnvironmentDegradationSystem::GetAtmosphericCondition(FString RegionID) const
{
    if (!Regions.Contains(RegionID))
    {
        return TEXT("Normal");
    }

    const FEnvironmentRegion& Region = Regions[RegionID];

    if (Region.DegradationLevel > 0.8f)
    {
        return TEXT("Heavy smoke, toxic atmosphere");
    }
    else if (Region.DegradationLevel > 0.5f)
    {
        return TEXT("Smoke-filled, ash falling");
    }
    else if (Region.DegradationLevel > 0.2f)
    {
        return TEXT("Light smoke, dust");
    }

    return TEXT("Clear");
}

bool UEnvironmentDegradationSystem::ShouldShowSmokeLayer(FVector Location) const
{
    float Degradation = GetDegradationAtLocation(Location);
    return Degradation > 0.3f;
}

float UEnvironmentDegradationSystem::GetAshDensity(FVector Location) const
{
    float Degradation = GetDegradationAtLocation(Location);
    return FMath::Clamp(Degradation * 2.0f, 0.0f, 1.0f);
}

// ========================================================================
// PLAYER FEEDBACK
// ========================================================================

FText UEnvironmentDegradationSystem::GetEnvironmentNarrative(int32 CurrentDay) const
{
    return GetTransformationDescription(CurrentDay);
}

void UEnvironmentDegradationSystem::TriggerEnvironmentMilestone(FString MilestoneID)
{
    if (TriggeredMilestones.Contains(MilestoneID))
    {
        return; // Already triggered
    }

    TriggeredMilestones.Add(MilestoneID);

    UE_LOG(LogTemp, Warning, TEXT("EnvironmentDegradationSystem: MILESTONE - %s"), *MilestoneID);

    // Example milestones:
    // - "AllTreesDestroyedBoisDesCaures"
    // - "FleuryCompletelyDestroyed"
    // - "FirstZoneRougeRegion"
    // - "50PercentWorldDestroyed"
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void UEnvironmentDegradationSystem::UpdateRegionalDegradation(float DeltaTime)
{
    // Update all regions
    for (auto& RegionPair : Regions)
    {
        FEnvironmentRegion& Region = RegionPair.Value;

        // Active war zones degrade faster
        if (Region.bIsActiveWarZone)
        {
            float DegradationRate = CalculateDegradationRate(Region);
            Region.DegradationLevel += DegradationRate * DeltaTime;
            Region.DegradationLevel = FMath::Clamp(Region.DegradationLevel, 0.0f, 1.0f);

            // Update region state based on degradation level
            if (Region.DegradationLevel >= 0.95f && Region.CurrentState != EEnvironmentState::ZoneRouge)
            {
                TransformRegion(RegionPair.Key, EEnvironmentState::ZoneRouge);
            }
            else if (Region.DegradationLevel >= 0.7f && Region.CurrentState == EEnvironmentState::Deteriorating)
            {
                TransformRegion(RegionPair.Key, EEnvironmentState::Devastated);
            }
        }
    }
}

void UEnvironmentDegradationSystem::CheckForEnvironmentMilestones()
{
    // Check for environmental milestones

    // All trees destroyed in a region
    for (const auto& RegionPair : Regions)
    {
        if (RegionPair.Value.TreesRemaining == 0)
        {
            FString MilestoneID = FString::Printf(TEXT("AllTreesDestroyed_%s"), *RegionPair.Key);
            TriggerEnvironmentMilestone(MilestoneID);
        }
    }

    // Global milestones
    if (GlobalDegradationLevel >= 0.5f)
    {
        TriggerEnvironmentMilestone(TEXT("HalfwayToZoneRouge"));
    }

    if (TotalCratersCreated >= 10000)
    {
        TriggerEnvironmentMilestone(TEXT("TenThousandCraters"));
    }
}

void UEnvironmentDegradationSystem::ApplyDegradationEffects(FString RegionID)
{
    // Apply visual/audio effects for region transformation
    // Called when region changes state
}

float UEnvironmentDegradationSystem::CalculateDegradationRate(FEnvironmentRegion& Region) const
{
    // Base degradation rate (0-1 over 303 days)
    float BaseRate = 1.0f / (303.0f * 24.0f * 3600.0f); // Per second

    // Accelerate in active war zones
    if (Region.bIsActiveWarZone)
    {
        BaseRate *= 10.0f;
    }

    return BaseRate;
}

float UEnvironmentDegradationSystem::GetHistoricalDegradationRate(int32 DayOfBattle) const
{
    // Historical degradation was NOT linear
    // Intense periods: Feb 21-Mar 10, April, June, Oct-Nov
    // Calmer periods: March middle, May, July-Sept

    if (DayOfBattle < 20) // Opening assault
    {
        return 3.0f; // 3x normal rate
    }
    else if (DayOfBattle > 200 && DayOfBattle < 250) // Oct-Nov offensives
    {
        return 2.5f;
    }
    else if (DayOfBattle > 120 && DayOfBattle < 160) // June fighting
    {
        return 2.0f;
    }

    return 1.0f; // Normal rate
}
