// Only The Dead - Environment Degradation System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "EnvironmentDegradationSystem.generated.h"

/**
 * Environment degradation system - CORE FEATURE
 * Transforms Verdun from picturesque countryside to apocalyptic wasteland over 303 days
 * This is a CHARACTER in the story - the dying world
 *
 * PERSISTENT WORLD DESIGN:
 * - ONE 1:1 scale map (30-50 km²)
 * - Player experiences transformation in real-time
 * - Environmental state persists and evolves
 * - Trees die, buildings collapse, craters accumulate
 */

UENUM(BlueprintType)
enum class EEnvironmentState : uint8
{
    Pristine            UMETA(DisplayName = "Pristine (Pre-war, tutorial only)"),
    EarlyWar            UMETA(DisplayName = "Early War (Days 1-30, damage beginning)"),
    Deteriorating       UMETA(DisplayName = "Deteriorating (Days 31-100, severe damage)"),
    Devastated          UMETA(DisplayName = "Devastated (Days 101-200, moonscape forming)"),
    Apocalyptic         UMETA(DisplayName = "Apocalyptic (Days 201-303, Zone Rouge)"),
    ZoneRouge           UMETA(DisplayName = "Zone Rouge (Post-war, uninhabitable)")
};

UENUM(BlueprintType)
enum class EDegradationType : uint8
{
    TreeDestruction     UMETA(DisplayName = "Trees (Shattered, Burned, Gone)"),
    BuildingDamage      UMETA(DisplayName = "Buildings (Cracked → Ruins → Rubble)"),
    CraterFormation     UMETA(DisplayName = "Craters (Accumulate, Overlap, Merge)"),
    TerrainDeformation  UMETA(DisplayName = "Terrain (Churned, Muddy, Toxic)"),
    VegetationDeath     UMETA(DisplayName = "Vegetation (Dead, Poisoned)"),
    WaterContamination  UMETA(DisplayName = "Water (Blood, Chemicals, Corpses)"),
    AtmosphericChange   UMETA(DisplayName = "Atmosphere (Smoke, Gas, Ash)")
};

USTRUCT(BlueprintType)
struct FEnvironmentRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FString RegionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FText RegionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FVector RegionCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    float RadiusMeters = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    EEnvironmentState CurrentState = EEnvironmentState::EarlyWar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    float DegradationLevel = 0.0f; // 0.0 = pristine, 1.0 = zone rouge

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    int32 ShellImpactsReceived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    int32 TreesRemaining = 1000; // Decreases over time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    int32 BuildingsRemaining = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    int32 CratersCreated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    bool bIsActiveWarZone = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FString HistoricalNotes;
};

USTRUCT(BlueprintType)
struct FDegradationEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FHistoricalDate EventDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString RegionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    EDegradationType DegradationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float Intensity = 0.5f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    bool bIsHistoricalEvent = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UEnvironmentDegradationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentDegradationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // WORLD STATE
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
    EEnvironmentState GlobalEnvironmentState = EEnvironmentState::EarlyWar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
    float GlobalDegradationLevel = 0.0f; // 0.0 (Feb 21) → 1.0 (Dec 18)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
    TMap<FString, FEnvironmentRegion> Regions;

    /** Get current global environment state */
    UFUNCTION(BlueprintPure, Category = "Environment")
    EEnvironmentState GetGlobalState() const { return GlobalEnvironmentState; }

    /** Get degradation level for specific location */
    UFUNCTION(BlueprintPure, Category = "Environment")
    float GetDegradationAtLocation(FVector Location) const;

    /** Get region data by ID */
    UFUNCTION(BlueprintPure, Category = "Environment")
    FEnvironmentRegion GetRegion(const FString& RegionID) const;

    // ========================================================================
    // DEGRADATION PROGRESSION
    // ========================================================================

    /** Update degradation based on current date */
    UFUNCTION(BlueprintCallable, Category = "Environment|Degradation")
    void UpdateDegradation(FHistoricalDate CurrentDate);

    /** Apply shell impact to region (increases degradation) */
    UFUNCTION(BlueprintCallable, Category = "Environment|Degradation")
    void ApplyShellImpact(FVector ImpactLocation, EShellType ShellType);

    /** Apply bombardment to region (mass degradation) */
    UFUNCTION(BlueprintCallable, Category = "Environment|Degradation")
    void ApplyBombardment(FString RegionID, EBombardmentIntensity Intensity, float DurationHours);

    /** Destroy tree at location */
    UFUNCTION(BlueprintCallable, Category = "Environment|Degradation")
    void DestroyTree(FVector TreeLocation);

    /** Damage building at location */
    UFUNCTION(BlueprintCallable, Category = "Environment|Degradation")
    void DamageBuilding(FVector BuildingLocation, float DamageAmount);

    /** Create crater at location */
    UFUNCTION(BlueprintCallable, Category = "Environment|Degradation")
    void CreateCrater(FVector Location, float DiameterMeters, float DepthMeters);

    // ========================================================================
    // ENVIRONMENTAL TRANSFORMATION
    // ========================================================================

    /** Transform region to new state */
    UFUNCTION(BlueprintCallable, Category = "Environment|Transformation")
    void TransformRegion(FString RegionID, EEnvironmentState NewState);

    /** Get transformation progress (0-1 for current day) */
    UFUNCTION(BlueprintPure, Category = "Environment|Transformation")
    float GetTransformationProgress(int32 CurrentDay) const;

    /** Check if region has reached Zone Rouge status */
    UFUNCTION(BlueprintPure, Category = "Environment|Transformation")
    bool IsRegionZoneRouge(FString RegionID) const;

    /** Get visible transformation description for player */
    UFUNCTION(BlueprintPure, Category = "Environment|Transformation")
    FText GetTransformationDescription(int32 CurrentDay) const;

    // ========================================================================
    // REGIONAL MANAGEMENT
    // ========================================================================

    /** Initialize all battlefield regions */
    UFUNCTION(BlueprintCallable, Category = "Environment|Regions")
    void InitializeRegions();

    /** Get region containing location */
    UFUNCTION(BlueprintPure, Category = "Environment|Regions")
    FString GetRegionAtLocation(FVector Location) const;

    /** Get all regions in degradation state */
    UFUNCTION(BlueprintPure, Category = "Environment|Regions")
    TArray<FEnvironmentRegion> GetRegionsByState(EEnvironmentState State) const;

    /** Mark region as active war zone (accelerated degradation) */
    UFUNCTION(BlueprintCallable, Category = "Environment|Regions")
    void SetRegionActiveWarZone(FString RegionID, bool bIsActive);

    // ========================================================================
    // HISTORICAL DEGRADATION EVENTS
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Historical")
    TArray<FDegradationEvent> HistoricalDegradationEvents;

    /** Load historical degradation timeline */
    UFUNCTION(BlueprintCallable, Category = "Environment|Historical")
    void LoadHistoricalDegradationEvents();

    /** Apply historical degradation for specific date */
    UFUNCTION(BlueprintCallable, Category = "Environment|Historical")
    void ApplyHistoricalDegradation(FHistoricalDate Date);

    /** Get historical event for date */
    UFUNCTION(BlueprintPure, Category = "Environment|Historical")
    TArray<FDegradationEvent> GetEventsForDate(FHistoricalDate Date) const;

    // ========================================================================
    // VISUAL EFFECTS
    // ========================================================================

    /** Spawn debris field at location */
    UFUNCTION(BlueprintCallable, Category = "Environment|VFX")
    void SpawnDebrisField(FVector Location, float RadiusMeters);

    /** Update foliage density in region */
    UFUNCTION(BlueprintCallable, Category = "Environment|VFX")
    void UpdateFoliageDensity(FString RegionID, float NewDensity);

    /** Apply terrain deformation */
    UFUNCTION(BlueprintCallable, Category = "Environment|VFX")
    void ApplyTerrainDeformation(FVector Location, float IntensityMultiplier);

    /** Spawn dead/burning trees */
    UFUNCTION(BlueprintCallable, Category = "Environment|VFX")
    void SpawnDeadTrees(FString RegionID, int32 Count);

    // ========================================================================
    // ATMOSPHERE & AMBIENCE
    // ========================================================================

    /** Get atmospheric condition for region */
    UFUNCTION(BlueprintPure, Category = "Environment|Atmosphere")
    FString GetAtmosphericCondition(FString RegionID) const;

    /** Should show smoke/gas clouds */
    UFUNCTION(BlueprintPure, Category = "Environment|Atmosphere")
    bool ShouldShowSmokeLayer(FVector Location) const;

    /** Get ash particle density */
    UFUNCTION(BlueprintPure, Category = "Environment|Atmosphere")
    float GetAshDensity(FVector Location) const;

    // ========================================================================
    // PLAYER FEEDBACK
    // ========================================================================

    /** Get narrative text for current transformation */
    UFUNCTION(BlueprintPure, Category = "Environment|Narrative")
    FText GetEnvironmentNarrative(int32 CurrentDay) const;

    /** Trigger environmental milestone (e.g., "All trees in Bois des Caures destroyed") */
    UFUNCTION(BlueprintCallable, Category = "Environment|Narrative")
    void TriggerEnvironmentMilestone(FString MilestoneID);

protected:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    float TimeSinceLastUpdate = 0.0f;
    int32 TotalShellImpacts = 0;
    int32 TotalTreesDestroyed = 0;
    int32 TotalBuildingsDestroyed = 0;
    int32 TotalCratersCreated = 0;

    TArray<FString> TriggeredMilestones;

    void UpdateRegionalDegradation(float DeltaTime);
    void CheckForEnvironmentMilestones();
    void ApplyDegradationEffects(FString RegionID);
    float CalculateDegradationRate(FEnvironmentRegion& Region) const;

    // Historical degradation rates (calibrated to 303 days)
    float GetHistoricalDegradationRate(int32 DayOfBattle) const;
};
