// Only The Dead - Progression and Meta-Game System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "ProgressionSystem.generated.h"

/**
 * Player progression and meta-game system
 * Tracks experience, skills, unlocks, and 303-day journey
 * Player transforms from Novice → Experienced → Veteran → Survivor
 *
 * PERSISTENT WORLD SUPPORT:
 * - Track regions explored (discovered 15/20 regions)
 * - Region-specific experience (survived 10 days in Fort Douaumont)
 * - Skills unlock from battlefield exposure, not mission completion
 * - Exploration achievements ("Visited all 20 regions", "Survived Zone Rouge")
 * - Continuous day tracking (not mission-based)
 */

UENUM(BlueprintType)
enum class ESkillCategory : uint8
{
    Combat          UMETA(DisplayName = "Combat"),
    Survival        UMETA(DisplayName = "Survival"),
    Artillery       UMETA(DisplayName = "Artillery Awareness"),
    Medical         UMETA(DisplayName = "Medical"),
    Leadership      UMETA(DisplayName = "Leadership")
};

USTRUCT(BlueprintType)
struct FPlayerSkill
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FString SkillID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FText SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FText SkillDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    ESkillCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    int32 CurrentLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    int32 MaxLevel = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float ExperiencePoints = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float ExperienceToNextLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    bool bIsUnlocked = true;
};

USTRUCT(BlueprintType)
struct FCampaignProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DaysSurvived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 MissionsCompleted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 RotationCyclesCompleted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DeathCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float TotalPlayTimeHours = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    EPlayerExperienceLevel CurrentExperienceLevel = EPlayerExperienceLevel::Novice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    bool bHasCompletedCampaign = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UProgressionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UProgressionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // PLAYER PROGRESSION
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    FCampaignProgress CampaignProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    TMap<FString, FPlayerSkill> PlayerSkills;

    /** Get current experience level */
    UFUNCTION(BlueprintPure, Category = "Progression")
    EPlayerExperienceLevel GetExperienceLevel() const;

    /** Get days survived */
    UFUNCTION(BlueprintPure, Category = "Progression")
    int32 GetDaysSurvived() const { return CampaignProgress.DaysSurvived; }

    /** Get campaign completion percentage */
    UFUNCTION(BlueprintPure, Category = "Progression")
    float GetCampaignCompletionPercentage() const;

    // ========================================================================
    // SKILL SYSTEM
    // ========================================================================

    /** Initialize skill tree */
    UFUNCTION(BlueprintCallable, Category = "Progression|Skills")
    void InitializeSkillTree();

    /** Award experience to skill */
    UFUNCTION(BlueprintCallable, Category = "Progression|Skills")
    void AwardSkillExperience(const FString& SkillID, float ExperienceAmount);

    /** Level up skill */
    UFUNCTION(BlueprintCallable, Category = "Progression|Skills")
    bool LevelUpSkill(const FString& SkillID);

    /** Get skill level */
    UFUNCTION(BlueprintPure, Category = "Progression|Skills")
    int32 GetSkillLevel(const FString& SkillID) const;

    /** Get skill modifier (bonus from skill level) */
    UFUNCTION(BlueprintPure, Category = "Progression|Skills")
    float GetSkillModifier(const FString& SkillID) const;

    /** Check if skill is unlocked */
    UFUNCTION(BlueprintPure, Category = "Progression|Skills")
    bool IsSkillUnlocked(const FString& SkillID) const;

    // ========================================================================
    // EXPERIENCE TRANSFORMATION (Novice → Survivor)
    // ========================================================================

    /** Update experience level based on days survived */
    UFUNCTION(BlueprintCallable, Category = "Progression|Experience")
    void UpdateExperienceLevel();

    /** Get experience level description */
    UFUNCTION(BlueprintPure, Category = "Progression|Experience")
    FText GetExperienceLevelDescription() const;

    /** Get stat bonuses for current experience level */
    UFUNCTION(BlueprintPure, Category = "Progression|Experience")
    void GetExperienceLevelBonuses(float& OutSurvivalBonus, float& OutCombatBonus, float& OutMoraleBonus) const;

    // ========================================================================
    // ACHIEVEMENTS & MILESTONES
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievements")
    TArray<FString> UnlockedAchievements;

    /** Unlock achievement */
    UFUNCTION(BlueprintCallable, Category = "Achievements")
    void UnlockAchievement(const FString& AchievementID);

    /** Check if achievement is unlocked */
    UFUNCTION(BlueprintPure, Category = "Achievements")
    bool IsAchievementUnlocked(const FString& AchievementID) const;

    /** Get all achievements */
    UFUNCTION(BlueprintPure, Category = "Achievements")
    TArray<FString> GetAllAchievements() const;

    // ========================================================================
    // STATISTICS TRACKING
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    TMap<FString, int32> IntegerStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    TMap<FString, float> FloatStats;

    /** Increment integer stat */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    void IncrementStat(const FString& StatName, int32 Amount = 1);

    /** Set float stat */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    void SetFloatStat(const FString& StatName, float Value);

    /** Get integer stat */
    UFUNCTION(BlueprintPure, Category = "Statistics")
    int32 GetIntStat(const FString& StatName) const;

    /** Get float stat */
    UFUNCTION(BlueprintPure, Category = "Statistics")
    float GetFloatStat(const FString& StatName) const;

    // ========================================================================
    // DIFFICULTY SCALING
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    EDifficultyMode CurrentDifficulty = EDifficultyMode::Soldier;

    /** Get difficulty modifiers */
    UFUNCTION(BlueprintPure, Category = "Difficulty")
    void GetDifficultyModifiers(float& OutDamageMultiplier, float& OutSurvivalMultiplier, bool& OutPermadeath) const;

    /** Is permadeath active */
    UFUNCTION(BlueprintPure, Category = "Difficulty")
    bool IsPermadeathActive() const;

    // ========================================================================
    // PERSISTENT WORLD - EXPLORATION TRACKING
    // ========================================================================

    /** Regions player has visited (discovered) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    TArray<FString> DiscoveredRegions;

    /** Time spent in each region (in game hours) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    TMap<FString, float> TimeInRegion;

    /** Days survived in each region */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    TMap<FString, int32> DaysSurvivedInRegion;

    /** Mark region as discovered */
    UFUNCTION(BlueprintCallable, Category = "Exploration")
    void DiscoverRegion(FString RegionID);

    /** Check if region has been discovered */
    UFUNCTION(BlueprintPure, Category = "Exploration")
    bool HasDiscoveredRegion(FString RegionID) const;

    /** Get exploration completion percentage */
    UFUNCTION(BlueprintPure, Category = "Exploration")
    float GetExplorationPercentage() const;

    /** Record time spent in region (called by MissionSystem) */
    UFUNCTION(BlueprintCallable, Category = "Exploration")
    void RecordTimeInRegion(FString RegionID, float GameHours);

    /** Get most visited region */
    UFUNCTION(BlueprintPure, Category = "Exploration")
    FString GetMostVistedRegion() const;

    /** Check exploration achievements (visited all regions, etc.) */
    UFUNCTION(BlueprintCallable, Category = "Exploration")
    void CheckExplorationAchievements();

protected:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    float TotalPlayTime = 0.0f;

    void UpdatePlayTime(float DeltaTime);
    void CheckForLevelUps();
    void CheckForAchievements();
};
