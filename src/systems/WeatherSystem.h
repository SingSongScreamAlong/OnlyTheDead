// Only The Dead - Weather and Time System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "WeatherSystem.generated.h"

/**
 * Weather and time-of-day system
 * Uses historical weather data from Verdun 1916
 * Affects visibility, survival meters, and gameplay
 */

USTRUCT(BlueprintType)
struct FWeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherCondition Condition = EWeatherCondition::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TemperatureCelsius = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeedKPH = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float PrecipitationIntensity = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float VisibilityMeters = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.5f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsNight = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UWeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeatherSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // WEATHER STATE
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bUseHistoricalWeather = true;

    /** Get current weather state */
    UFUNCTION(BlueprintPure, Category = "Weather")
    FWeatherState GetCurrentWeather() const { return CurrentWeather; }

    /** Set weather condition */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherCondition(EWeatherCondition NewCondition);

    /** Transition to new weather over time */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(FWeatherState TargetWeather, float TransitionTimeSeconds);

    // ========================================================================
    // HISTORICAL WEATHER
    // ========================================================================

    /** Get historical weather for specific date */
    UFUNCTION(BlueprintPure, Category = "Weather|Historical")
    FWeatherState GetHistoricalWeather(FHistoricalDate Date) const;

    /** Apply historical weather for current game date */
    UFUNCTION(BlueprintCallable, Category = "Weather|Historical")
    void ApplyHistoricalWeather();

    // ========================================================================
    // TIME OF DAY
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeProgressionRate = 1.0f; // Game hours per real hour

    /** Get current time of day (0-24) */
    UFUNCTION(BlueprintPure, Category = "Time")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    /** Set time of day */
    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(float Hours);

    /** Advance time */
    UFUNCTION(BlueprintCallable, Category = "Time")
    void AdvanceTime(float GameHours);

    /** Check if currently daytime */
    UFUNCTION(BlueprintPure, Category = "Time")
    bool IsDaytime() const;

    /** Check if currently night time */
    UFUNCTION(BlueprintPure, Category = "Time")
    bool IsNighttime() const;

    /** Get sun position (for lighting) */
    UFUNCTION(BlueprintPure, Category = "Time")
    FRotator GetSunRotation() const;

    /** Get current visibility distance */
    UFUNCTION(BlueprintPure, Category = "Time")
    float GetVisibilityDistance() const;

    // ========================================================================
    // WEATHER EFFECTS ON GAMEPLAY
    // ========================================================================

    /** Get movement speed modifier from weather */
    UFUNCTION(BlueprintPure, Category = "Weather|Effects")
    float GetMovementSpeedModifier() const;

    /** Get visibility modifier (fog, rain, night) */
    UFUNCTION(BlueprintPure, Category = "Weather|Effects")
    float GetVisibilityModifier() const;

    /** Get warmth depletion rate modifier */
    UFUNCTION(BlueprintPure, Category = "Weather|Effects")
    float GetWarmthDepletionModifier() const;

    /** Check if player is exposed to elements */
    UFUNCTION(BlueprintPure, Category = "Weather|Effects")
    bool IsExposedToElements() const;

    /** Check if player is wet (affects warmth) */
    UFUNCTION(BlueprintPure, Category = "Weather|Effects")
    bool IsPlayerWet() const;

    // ========================================================================
    // MUD SYSTEM
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mud")
    float MudLevel = 0.0f; // 0-1, increases with rain

    /** Update mud level based on weather */
    UFUNCTION(BlueprintCallable, Category = "Mud")
    void UpdateMudLevel(float DeltaTime);

    /** Get movement penalty from mud */
    UFUNCTION(BlueprintPure, Category = "Mud")
    float GetMudMovementPenalty() const;

    /** Check if location is muddy */
    UFUNCTION(BlueprintPure, Category = "Mud")
    bool IsMuddy(FVector Location) const;

    // ========================================================================
    // SEASONAL CHANGES
    // ========================================================================

    /** Get season for date */
    UFUNCTION(BlueprintPure, Category = "Weather|Season")
    FString GetSeasonForDate(FHistoricalDate Date) const;

    /** Get temperature range for season */
    UFUNCTION(BlueprintPure, Category = "Weather|Season")
    void GetSeasonalTemperatureRange(FHistoricalDate Date, float& OutMinTemp, float& OutMaxTemp) const;

protected:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    FWeatherState TargetWeather;
    float WeatherTransitionProgress = 1.0f;
    float WeatherTransitionDuration = 0.0f;

    float TimeSinceLastWeatherUpdate = 0.0f;
    bool bPlayerIsWet = false;
    float WetnessLevel = 0.0f;

    void UpdateWeatherTransition(float DeltaTime);
    void UpdateTimeOfDay(float DeltaTime);
    void UpdatePlayerWetness(float DeltaTime);
    void ApplyWeatherEffects();

    // Historical weather database (simplified - full version would be in data files)
    FWeatherState GetDefaultWeatherForMonth(int32 Month) const;
};
