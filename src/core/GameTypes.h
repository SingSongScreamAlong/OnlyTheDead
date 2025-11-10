// Only The Dead - Core Game Types
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameTypes.generated.h"

/**
 * Core game type definitions for Only The Dead
 * Survival game set during Battle of Verdun (Feb 21 - Dec 18, 1916)
 */

// ============================================================================
// ENUMERATIONS
// ============================================================================

UENUM(BlueprintType)
enum class ESurvivalMeterType : uint8
{
    Hunger      UMETA(DisplayName = "Hunger"),
    Thirst      UMETA(DisplayName = "Thirst"),
    Stamina     UMETA(DisplayName = "Stamina"),
    Warmth      UMETA(DisplayName = "Warmth"),
    Hygiene     UMETA(DisplayName = "Hygiene"),
    Morale      UMETA(DisplayName = "Morale"),
    Alertness   UMETA(DisplayName = "Alertness")
};

UENUM(BlueprintType)
enum class EPlayerExperienceLevel : uint8
{
    Novice      UMETA(DisplayName = "Novice (Feb-Mar)"),
    Experienced UMETA(DisplayName = "Experienced (Apr-Jun)"),
    Veteran     UMETA(DisplayName = "Veteran (Jul-Sep)"),
    Survivor    UMETA(DisplayName = "Survivor (Oct-Dec)")
};

UENUM(BlueprintType)
enum class ERotationPhase : uint8
{
    FrontLine   UMETA(DisplayName = "Front Line (4-7 days)"),
    Support     UMETA(DisplayName = "Support Line (3-5 days)"),
    Rest        UMETA(DisplayName = "Rest Area (2-4 days)"),
    Transit     UMETA(DisplayName = "In Transit")
};

UENUM(BlueprintType)
enum class EShellType : uint8
{
    Shell_75mm      UMETA(DisplayName = "75mm French Field Gun"),
    Shell_77mm      UMETA(DisplayName = "77mm German Field Gun"),
    Shell_105mm     UMETA(DisplayName = "105mm Howitzer"),
    Shell_155mm     UMETA(DisplayName = "155mm Heavy Artillery"),
    Shell_210mm     UMETA(DisplayName = "210mm Mortar"),
    Shell_305mm     UMETA(DisplayName = "305mm Siege Gun"),
    Shell_420mm     UMETA(DisplayName = "420mm Big Bertha"),
    Gas_Phosgene    UMETA(DisplayName = "Phosgene Gas"),
    Gas_Chlorine    UMETA(DisplayName = "Chlorine Gas"),
    Gas_Mustard     UMETA(DisplayName = "Mustard Gas")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Rifle_Lebel         UMETA(DisplayName = "Lebel Model 1886 Rifle"),
    Rifle_Berthier      UMETA(DisplayName = "Berthier Rifle"),
    Rifle_Mauser        UMETA(DisplayName = "Mauser Gewehr 98"),
    MachineGun_Hotchkiss UMETA(DisplayName = "Hotchkiss M1914"),
    MachineGun_MG08     UMETA(DisplayName = "MG 08"),
    Grenade_F1          UMETA(DisplayName = "F1 Grenade"),
    Grenade_Stielhandgranate UMETA(DisplayName = "Stielhandgranate"),
    Melee_Bayonet       UMETA(DisplayName = "Bayonet"),
    Melee_Club          UMETA(DisplayName = "Trench Club"),
    Melee_Knife         UMETA(DisplayName = "Knife")
};

UENUM(BlueprintType)
enum class EInjuryType : uint8
{
    None            UMETA(DisplayName = "No Injury"),
    Shrapnel        UMETA(DisplayName = "Shrapnel Wound"),
    Gunshot         UMETA(DisplayName = "Gunshot Wound"),
    Concussion      UMETA(DisplayName = "Concussion/Shell Shock"),
    Burns           UMETA(DisplayName = "Burns"),
    GasExposure     UMETA(DisplayName = "Gas Exposure"),
    Frostbite       UMETA(DisplayName = "Frostbite"),
    Disease         UMETA(DisplayName = "Disease (Typhoid/Dysentery)"),
    TrenchFoot      UMETA(DisplayName = "Trench Foot"),
    Infection       UMETA(DisplayName = "Infection")
};

UENUM(BlueprintType)
enum class EDifficultyMode : uint8
{
    Recruit     UMETA(DisplayName = "Recruit - Story Mode"),
    Soldier     UMETA(DisplayName = "Soldier - Normal"),
    Veteran     UMETA(DisplayName = "Veteran - Hard"),
    Ironman     UMETA(DisplayName = "Ironman - Permadeath")
};

UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Snow        UMETA(DisplayName = "Snow"),
    Fog         UMETA(DisplayName = "Fog"),
    Storm       UMETA(DisplayName = "Storm")
};

// ============================================================================
// STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct FSurvivalMeterState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    ESurvivalMeterType MeterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float CurrentValue = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float DepletionRatePerHour = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    bool bIsCritical = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float TimeAtZero = 0.0f; // Time spent at 0 (for death calculations)
};

USTRUCT(BlueprintType)
struct FShellData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    EShellType ShellType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float CaliberMM = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float MuzzleVelocityMS = 529.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float BlastRadiusMeters = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float LethalRadiusMeters = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float CraterDepthMeters = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float CraterDiameterMeters = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float FragmentationRange = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    FString AudioCueIncoming = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float FlightTimeLowTrajectory = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery")
    float FlightTimeHighTrajectory = 15.0f;
};

USTRUCT(BlueprintType)
struct FHistoricalDate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    int32 Year = 1916;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    int32 Month = 2; // February

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    int32 Day = 21;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    int32 Hour = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    int32 Minute = 15;

    // Returns day number since start of battle (Feb 21 = Day 0)
    int32 GetDaySinceBattleStart() const
    {
        // Simple calculation (Feb 21, 1916 = Day 0)
        static const int32 DaysInMonth[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // 1916 is leap year

        int32 TotalDays = Day - 21; // Days in February after 21st

        for (int32 M = 3; M <= Month; ++M)
        {
            TotalDays += DaysInMonth[M - 1];
        }

        return TotalDays;
    }
};

USTRUCT(BlueprintType)
struct FMissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FText ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsOptional = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsFailed = false;
};

USTRUCT(BlueprintType)
struct FCharacterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText Rank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText Unit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText Hometown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText BackgroundDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EPlayerExperienceLevel ExperienceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsHistoricalFigure = false;
};

// ============================================================================
// CONSTANTS
// ============================================================================

namespace GameConstants
{
    // Battle Timeline
    constexpr int32 BATTLE_START_YEAR = 1916;
    constexpr int32 BATTLE_START_MONTH = 2;
    constexpr int32 BATTLE_START_DAY = 21;
    constexpr int32 BATTLE_DURATION_DAYS = 303;

    // Survival Meter Defaults
    constexpr float HUNGER_DEPLETION_BASE = 5.0f; // per hour
    constexpr float THIRST_DEPLETION_BASE = 8.0f; // per hour (CRITICAL)
    constexpr float STAMINA_DEPLETION_WALKING = 2.0f;
    constexpr float WARMTH_DEPLETION_COLD = 6.0f;
    constexpr float HYGIENE_DEPLETION_BASE = 0.5f; // per hour (slow)
    constexpr float MORALE_DEPLETION_COMBAT = 10.0f; // per artillery strike
    constexpr float ALERTNESS_DEPLETION_BASE = 4.0f; // per hour awake

    // Death Timings
    constexpr float TIME_TO_DEATH_HUNGER_HOURS = 168.0f; // 7 days
    constexpr float TIME_TO_DEATH_THIRST_HOURS = 24.0f; // 1 day (CRITICAL)
    constexpr float TIME_TO_DEATH_HYPOTHERMIA_HOURS = 4.0f;
    constexpr float TIME_TO_DEATH_SLEEP_DEPRIVATION_HOURS = 72.0f; // 3 days

    // Rotation System
    constexpr int32 FRONTLINE_ROTATION_DAYS_MIN = 4;
    constexpr int32 FRONTLINE_ROTATION_DAYS_MAX = 7;
    constexpr int32 SUPPORT_ROTATION_DAYS_MIN = 3;
    constexpr int32 SUPPORT_ROTATION_DAYS_MAX = 5;
    constexpr int32 REST_ROTATION_DAYS_MIN = 2;
    constexpr int32 REST_ROTATION_DAYS_MAX = 4;

    // Artillery
    constexpr float ARTILLERY_CASUALTY_RATE = 0.70f; // 70% of deaths from artillery
    constexpr int32 SHELLS_FIRED_PER_DAY_AVERAGE = 50000; // Historical average

    // Experience Progression
    constexpr int32 DAYS_TO_EXPERIENCED = 60; // ~March-April
    constexpr int32 DAYS_TO_VETERAN = 150; // ~July-August
    constexpr int32 DAYS_TO_SURVIVOR = 240; // ~October-November
}

