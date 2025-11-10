// Only The Dead - Progression System Implementation
// Copyright 2025. All Rights Reserved.

#include "ProgressionSystem.h"
#include "Kismet/GameplayStatics.h"

UProgressionSystem::UProgressionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    CurrentDifficulty = EDifficultyMode::Soldier;
    TotalPlayTime = 0.0f;

    // Initialize campaign progress
    CampaignProgress.DaysSurvived = 0;
    CampaignProgress.MissionsCompleted = 0;
    CampaignProgress.RotationCyclesCompleted = 0;
    CampaignProgress.DeathCount = 0;
    CampaignProgress.TotalPlayTimeHours = 0.0f;
    CampaignProgress.CurrentExperienceLevel = EPlayerExperienceLevel::Novice;
    CampaignProgress.bHasCompletedCampaign = false;
}

void UProgressionSystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeSkillTree();

    UE_LOG(LogTemp, Log, TEXT("ProgressionSystem: Initialized. Experience Level: Novice"));
}

void UProgressionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdatePlayTime(DeltaTime);
    CheckForLevelUps();
    CheckForAchievements();
}

// ========================================================================
// PLAYER PROGRESSION
// ========================================================================

EPlayerExperienceLevel UProgressionSystem::GetExperienceLevel() const
{
    return CampaignProgress.CurrentExperienceLevel;
}

float UProgressionSystem::GetCampaignCompletionPercentage() const
{
    return (float)CampaignProgress.DaysSurvived / 303.0f * 100.0f;
}

// ========================================================================
// SKILL SYSTEM
// ========================================================================

void UProgressionSystem::InitializeSkillTree()
{
    // Initialize all skills

    // COMBAT SKILLS
    FPlayerSkill Marksmanship;
    Marksmanship.SkillID = TEXT("skill_marksmanship");
    Marksmanship.SkillName = FText::FromString(TEXT("Marksmanship"));
    Marksmanship.SkillDescription = FText::FromString(TEXT("Improved accuracy with rifles"));
    Marksmanship.Category = ESkillCategory::Combat;
    Marksmanship.CurrentLevel = 0;
    Marksmanship.MaxLevel = 5;
    Marksmanship.ExperienceToNextLevel = 100.0f;
    PlayerSkills.Add(Marksmanship.SkillID, Marksmanship);

    // SURVIVAL SKILLS
    FPlayerSkill Endurance;
    Endurance.SkillID = TEXT("skill_endurance");
    Endurance.SkillName = FText::FromString(TEXT("Endurance"));
    Endurance.SkillDescription = FText::FromString(TEXT("Slower stamina depletion"));
    Endurance.Category = ESkillCategory::Survival;
    Endurance.CurrentLevel = 0;
    Endurance.MaxLevel = 5;
    Endurance.ExperienceToNextLevel = 100.0f;
    PlayerSkills.Add(Endurance.SkillID, Endurance);

    // ARTILLERY AWARENESS
    FPlayerSkill ShellRecognition;
    ShellRecognition.SkillID = TEXT("skill_shell_recognition");
    ShellRecognition.SkillName = FText::FromString(TEXT("Shell Recognition"));
    ShellRecognition.SkillDescription = FText::FromString(TEXT("Better identification of incoming shells"));
    ShellRecognition.Category = ESkillCategory::Artillery;
    ShellRecognition.CurrentLevel = 0;
    ShellRecognition.MaxLevel = 5;
    ShellRecognition.ExperienceToNextLevel = 150.0f;
    PlayerSkills.Add(ShellRecognition.SkillID, ShellRecognition);

    // MEDICAL SKILLS
    FPlayerSkill FieldMedicine;
    FieldMedicine.SkillID = TEXT("skill_field_medicine");
    FieldMedicine.SkillName = FText::FromString(TEXT("Field Medicine"));
    FieldMedicine.SkillDescription = FText::FromString(TEXT("More effective medical treatment"));
    FieldMedicine.Category = ESkillCategory::Medical;
    FieldMedicine.CurrentLevel = 0;
    FieldMedicine.MaxLevel = 5;
    FieldMedicine.ExperienceToNextLevel = 120.0f;
    PlayerSkills.Add(FieldMedicine.SkillID, FieldMedicine);

    // LEADERSHIP SKILLS
    FPlayerSkill Leadership;
    Leadership.SkillID = TEXT("skill_leadership");
    Leadership.SkillName = FText::FromString(TEXT("Leadership"));
    Leadership.SkillDescription = FText::FromString(TEXT("Boost squad morale"));
    Leadership.Category = ESkillCategory::Leadership;
    Leadership.CurrentLevel = 0;
    Leadership.MaxLevel = 5;
    Leadership.ExperienceToNextLevel = 200.0f;
    PlayerSkills.Add(Leadership.SkillID, Leadership);

    UE_LOG(LogTemp, Log, TEXT("ProgressionSystem: Skill tree initialized with %d skills"), PlayerSkills.Num());
}

void UProgressionSystem::AwardSkillExperience(const FString& SkillID, float ExperienceAmount)
{
    if (!PlayerSkills.Contains(SkillID))
    {
        UE_LOG(LogTemp, Warning, TEXT("ProgressionSystem: Skill '%s' not found"), *SkillID);
        return;
    }

    FPlayerSkill& Skill = PlayerSkills[SkillID];
    Skill.ExperiencePoints += ExperienceAmount;

    UE_LOG(LogTemp, Log, TEXT("ProgressionSystem: +%.0f XP to %s (%.0f/%.0f)"),
           ExperienceAmount,
           *Skill.SkillName.ToString(),
           Skill.ExperiencePoints,
           Skill.ExperienceToNextLevel);

    // Check for level up
    if (Skill.ExperiencePoints >= Skill.ExperienceToNextLevel)
    {
        LevelUpSkill(SkillID);
    }
}

bool UProgressionSystem::LevelUpSkill(const FString& SkillID)
{
    if (!PlayerSkills.Contains(SkillID))
    {
        return false;
    }

    FPlayerSkill& Skill = PlayerSkills[SkillID];

    if (Skill.CurrentLevel >= Skill.MaxLevel)
    {
        UE_LOG(LogTemp, Log, TEXT("ProgressionSystem: %s already at max level"), *Skill.SkillName.ToString());
        return false;
    }

    // Level up
    Skill.CurrentLevel++;
    Skill.ExperiencePoints = 0.0f;
    Skill.ExperienceToNextLevel *= 1.5f; // Exponential scaling

    UE_LOG(LogTemp, Warning, TEXT("ProgressionSystem: SKILL LEVEL UP! %s now level %d"),
           *Skill.SkillName.ToString(),
           Skill.CurrentLevel);

    return true;
}

int32 UProgressionSystem::GetSkillLevel(const FString& SkillID) const
{
    if (PlayerSkills.Contains(SkillID))
    {
        return PlayerSkills[SkillID].CurrentLevel;
    }
    return 0;
}

float UProgressionSystem::GetSkillModifier(const FString& SkillID) const
{
    int32 Level = GetSkillLevel(SkillID);

    // Each level provides 10% bonus
    return 1.0f + (Level * 0.1f);
}

bool UProgressionSystem::IsSkillUnlocked(const FString& SkillID) const
{
    if (PlayerSkills.Contains(SkillID))
    {
        return PlayerSkills[SkillID].bIsUnlocked;
    }
    return false;
}

// ========================================================================
// EXPERIENCE TRANSFORMATION (Novice → Survivor)
// ========================================================================

void UProgressionSystem::UpdateExperienceLevel()
{
    // Player transforms based on days survived
    // Novice (0-30 days) → Experienced (31-100) → Veteran (101-200) → Survivor (201-303)

    int32 Days = CampaignProgress.DaysSurvived;

    EPlayerExperienceLevel OldLevel = CampaignProgress.CurrentExperienceLevel;
    EPlayerExperienceLevel NewLevel = OldLevel;

    if (Days >= 201)
    {
        NewLevel = EPlayerExperienceLevel::Survivor;
    }
    else if (Days >= 101)
    {
        NewLevel = EPlayerExperienceLevel::Veteran;
    }
    else if (Days >= 31)
    {
        NewLevel = EPlayerExperienceLevel::Experienced;
    }
    else
    {
        NewLevel = EPlayerExperienceLevel::Novice;
    }

    if (NewLevel != OldLevel)
    {
        CampaignProgress.CurrentExperienceLevel = NewLevel;

        UE_LOG(LogTemp, Warning, TEXT("ProgressionSystem: EXPERIENCE LEVEL UP! Now: %s"),
               *GetExperienceLevelDescription().ToString());
    }
}

FText UProgressionSystem::GetExperienceLevelDescription() const
{
    switch (CampaignProgress.CurrentExperienceLevel)
    {
        case EPlayerExperienceLevel::Novice:
            return FText::FromString(TEXT("Novice - Fresh arrival at Verdun, learning to survive"));

        case EPlayerExperienceLevel::Experienced:
            return FText::FromString(TEXT("Experienced - Survived the first month, knows the dangers"));

        case EPlayerExperienceLevel::Veteran:
            return FText::FromString(TEXT("Veteran - Hardened by months of combat, skilled survivor"));

        case EPlayerExperienceLevel::Survivor:
            return FText::FromString(TEXT("Survivor - Elite status, one of the few who have endured this long"));

        default:
            return FText::FromString(TEXT("Unknown"));
    }
}

void UProgressionSystem::GetExperienceLevelBonuses(float& OutSurvivalBonus, float& OutCombatBonus, float& OutMoraleBonus) const
{
    // Bonuses increase with experience level

    switch (CampaignProgress.CurrentExperienceLevel)
    {
        case EPlayerExperienceLevel::Novice:
            OutSurvivalBonus = 0.0f;
            OutCombatBonus = 0.0f;
            OutMoraleBonus = 0.0f;
            break;

        case EPlayerExperienceLevel::Experienced:
            OutSurvivalBonus = 0.1f; // 10% better survival
            OutCombatBonus = 0.15f; // 15% better combat
            OutMoraleBonus = 0.1f; // 10% morale resistance
            break;

        case EPlayerExperienceLevel::Veteran:
            OutSurvivalBonus = 0.25f; // 25% better survival
            OutCombatBonus = 0.30f; // 30% better combat
            OutMoraleBonus = 0.25f; // 25% morale resistance
            break;

        case EPlayerExperienceLevel::Survivor:
            OutSurvivalBonus = 0.40f; // 40% better survival
            OutCombatBonus = 0.50f; // 50% better combat
            OutMoraleBonus = 0.40f; // 40% morale resistance
            break;
    }
}

// ========================================================================
// ACHIEVEMENTS & MILESTONES
// ========================================================================

void UProgressionSystem::UnlockAchievement(const FString& AchievementID)
{
    if (UnlockedAchievements.Contains(AchievementID))
    {
        return; // Already unlocked
    }

    UnlockedAchievements.Add(AchievementID);

    UE_LOG(LogTemp, Warning, TEXT("ProgressionSystem: ACHIEVEMENT UNLOCKED - %s"), *AchievementID);
}

bool UProgressionSystem::IsAchievementUnlocked(const FString& AchievementID) const
{
    return UnlockedAchievements.Contains(AchievementID);
}

TArray<FString> UProgressionSystem::GetAllAchievements() const
{
    return UnlockedAchievements;
}

// ========================================================================
// STATISTICS TRACKING
// ========================================================================

void UProgressionSystem::IncrementStat(const FString& StatName, int32 Amount)
{
    if (!IntegerStats.Contains(StatName))
    {
        IntegerStats.Add(StatName, 0);
    }

    IntegerStats[StatName] += Amount;
}

void UProgressionSystem::SetFloatStat(const FString& StatName, float Value)
{
    FloatStats.Add(StatName, Value);
}

int32 UProgressionSystem::GetIntStat(const FString& StatName) const
{
    if (IntegerStats.Contains(StatName))
    {
        return IntegerStats[StatName];
    }
    return 0;
}

float UProgressionSystem::GetFloatStat(const FString& StatName) const
{
    if (FloatStats.Contains(StatName))
    {
        return FloatStats[StatName];
    }
    return 0.0f;
}

// ========================================================================
// DIFFICULTY SCALING
// ========================================================================

void UProgressionSystem::GetDifficultyModifiers(float& OutDamageMultiplier, float& OutSurvivalMultiplier, bool& OutPermadeath) const
{
    switch (CurrentDifficulty)
    {
        case EDifficultyMode::Recruit:
            OutDamageMultiplier = 0.5f; // Take 50% damage
            OutSurvivalMultiplier = 0.7f; // 30% slower meter depletion
            OutPermadeath = false;
            break;

        case EDifficultyMode::Soldier:
            OutDamageMultiplier = 1.0f; // Normal damage
            OutSurvivalMultiplier = 1.0f; // Normal depletion
            OutPermadeath = false;
            break;

        case EDifficultyMode::Veteran:
            OutDamageMultiplier = 1.5f; // Take 50% more damage
            OutSurvivalMultiplier = 1.3f; // 30% faster meter depletion
            OutPermadeath = false;
            break;

        case EDifficultyMode::Survivor:
            OutDamageMultiplier = 2.0f; // Double damage
            OutSurvivalMultiplier = 1.5f; // 50% faster meter depletion
            OutPermadeath = true; // Permadeath enforced
            break;
    }
}

bool UProgressionSystem::IsPermadeathActive() const
{
    return CurrentDifficulty == EDifficultyMode::Survivor;
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void UProgressionSystem::UpdatePlayTime(float DeltaTime)
{
    TotalPlayTime += DeltaTime;
    CampaignProgress.TotalPlayTimeHours = TotalPlayTime / 3600.0f;
}

void UProgressionSystem::CheckForLevelUps()
{
    // Check if player should level up experience level
    UpdateExperienceLevel();
}

void UProgressionSystem::CheckForAchievements()
{
    // Check for achievement unlocks based on current stats

    // Survival achievements
    if (CampaignProgress.DaysSurvived >= 7 && !IsAchievementUnlocked(TEXT("ach_first_week")))
    {
        UnlockAchievement(TEXT("ach_first_week"));
    }

    if (CampaignProgress.DaysSurvived >= 30 && !IsAchievementUnlocked(TEXT("ach_first_month")))
    {
        UnlockAchievement(TEXT("ach_first_month"));
    }

    if (CampaignProgress.DaysSurvived >= 100 && !IsAchievementUnlocked(TEXT("ach_hundred_days")))
    {
        UnlockAchievement(TEXT("ach_hundred_days"));
    }

    if (CampaignProgress.DaysSurvived >= 303 && !IsAchievementUnlocked(TEXT("ach_survivor")))
    {
        UnlockAchievement(TEXT("ach_survivor"));
        CampaignProgress.bHasCompletedCampaign = true;
    }

    // Mission achievements
    if (CampaignProgress.MissionsCompleted >= 10 && !IsAchievementUnlocked(TEXT("ach_ten_missions")))
    {
        UnlockAchievement(TEXT("ach_ten_missions"));
    }

    if (CampaignProgress.MissionsCompleted >= 35 && !IsAchievementUnlocked(TEXT("ach_all_missions")))
    {
        UnlockAchievement(TEXT("ach_all_missions"));
    }
}
