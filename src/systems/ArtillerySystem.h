// Only The Dead - Artillery System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "ArtillerySystem.generated.h"

/**
 * Artillery system managing shell impacts, bombardments, and audio cues
 * Artillery is the primary threat - 70% of casualties from shells
 * Implements shell recognition minigame and realistic ballistics
 */

UENUM(BlueprintType)
enum class EBombardmentIntensity : uint8
{
    None            UMETA(DisplayName = "None"),
    Harassing       UMETA(DisplayName = "Harassing Fire (1-2/min)"),
    Light           UMETA(DisplayName = "Light Barrage (5-10/min)"),
    Heavy           UMETA(DisplayName = "Heavy Barrage (20-30/min)"),
    Trommelfeuer    UMETA(DisplayName = "Drumfire (100+/min)")
};

USTRUCT(BlueprintType)
struct FArtilleryStrike
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    EShellType ShellType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float TimeToImpact = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    bool bIsIncoming = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    FVector OriginLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float FlightTime = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UArtillerySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArtillerySystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // BOMBARDMENT SYSTEM
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bombardment")
    EBombardmentIntensity CurrentIntensity = EBombardmentIntensity::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bombardment")
    TArray<FArtilleryStrike> ActiveStrikes;

    /** Start a bombardment with specified intensity and duration */
    UFUNCTION(BlueprintCallable, Category = "Artillery")
    void StartBombardment(EBombardmentIntensity Intensity, float DurationSeconds);

    /** Stop current bombardment */
    UFUNCTION(BlueprintCallable, Category = "Artillery")
    void StopBombardment();

    /** Fire a single shell at target location */
    UFUNCTION(BlueprintCallable, Category = "Artillery")
    void FireShell(EShellType ShellType, FVector TargetLocation, FVector OriginLocation);

    /** Calculate shell impact damage at given distance */
    UFUNCTION(BlueprintPure, Category = "Artillery")
    float CalculateShellDamage(EShellType ShellType, float DistanceFromImpact) const;

    /** Check if location is in lethal radius of active strikes */
    UFUNCTION(BlueprintPure, Category = "Artillery")
    bool IsLocationInDangerZone(FVector Location, float& OutNearestDistance) const;

    // ========================================================================
    // SHELL RECOGNITION SYSTEM
    // ========================================================================

    /** Play incoming shell audio cue */
    UFUNCTION(BlueprintCallable, Category = "Artillery|Audio")
    void PlayIncomingShellAudio(EShellType ShellType, float Distance);

    /** Player attempts to identify shell type by audio */
    UFUNCTION(BlueprintCallable, Category = "Artillery|Audio")
    bool AttemptShellIdentification(EShellType GuessedType, EShellType ActualType);

    /** Get shell audio cue identifier */
    UFUNCTION(BlueprintPure, Category = "Artillery|Audio")
    FString GetShellAudioCueID(EShellType ShellType) const;

    /** Calculate time player has to react to incoming shell */
    UFUNCTION(BlueprintPure, Category = "Artillery|Audio")
    float GetReactionTimeWindow(EShellType ShellType, float FlightTime) const;

    // ========================================================================
    // SHELL DATA
    // ========================================================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Artillery|Data")
    TMap<EShellType, FShellData> ShellDatabase;

    /** Initialize shell database with historical data */
    UFUNCTION(BlueprintCallable, Category = "Artillery|Data")
    void InitializeShellDatabase();

    /** Get shell data for specific type */
    UFUNCTION(BlueprintPure, Category = "Artillery|Data")
    FShellData GetShellData(EShellType ShellType) const;

    // ========================================================================
    // CRATER SYSTEM
    // ========================================================================

    /** Create crater at impact location */
    UFUNCTION(BlueprintCallable, Category = "Artillery|Craters")
    void CreateCrater(FVector Location, EShellType ShellType);

    /** Get crater size for shell type */
    UFUNCTION(BlueprintPure, Category = "Artillery|Craters")
    void GetCraterDimensions(EShellType ShellType, float& OutDepth, float& OutDiameter) const;

    // ========================================================================
    // MORALE EFFECTS
    // ========================================================================

    /** Calculate morale impact from nearby shell strike */
    UFUNCTION(BlueprintPure, Category = "Artillery|Morale")
    float CalculateMoraleImpact(float DistanceFromPlayer, EShellType ShellType) const;

    /** Check if strike should cause shell shock episode */
    UFUNCTION(BlueprintPure, Category = "Artillery|Morale")
    bool ShouldTriggerShellShock(float DistanceFromPlayer, int32 StrikesInLastMinute) const;

    // ========================================================================
    // HISTORICAL BOMBARDMENT PATTERNS
    // ========================================================================

    /** Start historical bombardment (Feb 21, 07:15 opening barrage) */
    UFUNCTION(BlueprintCallable, Category = "Artillery|Historical")
    void StartHistoricalBombardment_Feb21_Opening();

    /** Start Trommelfeuer (drumfire) pattern */
    UFUNCTION(BlueprintCallable, Category = "Artillery|Historical")
    void StartTrommelfeuer(FVector CenterLocation, float RadiusMeters, float DurationMinutes);

    /** Calculate bombardment intensity from historical data */
    UFUNCTION(BlueprintPure, Category = "Artillery|Historical")
    EBombardmentIntensity GetHistoricalIntensity(FHistoricalDate Date, FString SectorName) const;

protected:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    float BombardmentTimeRemaining = 0.0f;
    float TimeSinceLastShell = 0.0f;
    int32 ShellsInLastMinute = 0;
    TArray<float> ShellTimestamps;

    FTimerHandle BombardmentTimerHandle;

    void UpdateActivesStrikes(float DeltaTime);
    void SpawnShellImpact(const FArtilleryStrike& Strike);
    void ScheduleNextShell();

    // ========================================================================
    // AUDIO SYSTEM INTEGRATION
    // ========================================================================

    UPROPERTY()
    class UAudioComponent* ArtilleryAudioComponent;

    void InitializeAudioSystem();
};
