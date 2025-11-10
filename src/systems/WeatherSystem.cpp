// Only The Dead - Weather System Implementation
// Copyright 2025. All Rights Reserved.

#include "WeatherSystem.h"
#include "SurvivalSystem.h"
#include "Kismet/GameplayStatics.h"

UWeatherSystem::UWeatherSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Initialize with typical Feb 1916 Verdun weather
    CurrentWeather.Condition = EWeatherCondition::Cloudy;
    CurrentWeather.TemperatureCelsius = 2.0f; // Cold winter
    CurrentWeather.WindSpeedKPH = 15.0f;
    CurrentWeather.PrecipitationIntensity = 0.0f;
    CurrentWeather.VisibilityMeters = 500.0f; // Reduced by winter conditions
    CurrentWeather.CloudCoverage = 0.8f;
    CurrentWeather.bIsNight = false;

    CurrentTimeOfDay = 7.0f; // Battle started at 7:15 AM
    TimeProgressionRate = 1.0f;
    MudLevel = 0.5f; // Verdun was notoriously muddy
    bUseHistoricalWeather = true;
}

void UWeatherSystem::BeginPlay()
{
    Super::BeginPlay();

    if (bUseHistoricalWeather)
    {
        ApplyHistoricalWeather();
    }

    UE_LOG(LogTemp, Log, TEXT("WeatherSystem: Initialized. Temp: %.1f°C, Mud: %.0f%%"),
           CurrentWeather.TemperatureCelsius,
           MudLevel * 100.0f);
}

void UWeatherSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateTimeOfDay(DeltaTime);
    UpdateWeatherTransition(DeltaTime);
    UpdateMudLevel(DeltaTime);
    UpdatePlayerWetness(DeltaTime);
    ApplyWeatherEffects();
}

// ========================================================================
// WEATHER STATE
// ========================================================================

void UWeatherSystem::SetWeatherCondition(EWeatherCondition NewCondition)
{
    CurrentWeather.Condition = NewCondition;

    // Update related parameters
    switch (NewCondition)
    {
        case EWeatherCondition::Clear:
            CurrentWeather.PrecipitationIntensity = 0.0f;
            CurrentWeather.CloudCoverage = 0.2f;
            CurrentWeather.VisibilityMeters = 1000.0f;
            break;

        case EWeatherCondition::Cloudy:
            CurrentWeather.PrecipitationIntensity = 0.0f;
            CurrentWeather.CloudCoverage = 0.8f;
            CurrentWeather.VisibilityMeters = 800.0f;
            break;

        case EWeatherCondition::Rainy:
            CurrentWeather.PrecipitationIntensity = 0.6f;
            CurrentWeather.CloudCoverage = 1.0f;
            CurrentWeather.VisibilityMeters = 400.0f;
            break;

        case EWeatherCondition::Stormy:
            CurrentWeather.PrecipitationIntensity = 1.0f;
            CurrentWeather.CloudCoverage = 1.0f;
            CurrentWeather.VisibilityMeters = 200.0f;
            CurrentWeather.WindSpeedKPH = 40.0f;
            break;

        case EWeatherCondition::Snowy:
            CurrentWeather.PrecipitationIntensity = 0.4f;
            CurrentWeather.CloudCoverage = 1.0f;
            CurrentWeather.VisibilityMeters = 300.0f;
            CurrentWeather.TemperatureCelsius = -2.0f;
            break;

        case EWeatherCondition::Foggy:
            CurrentWeather.PrecipitationIntensity = 0.1f;
            CurrentWeather.CloudCoverage = 1.0f;
            CurrentWeather.VisibilityMeters = 100.0f; // Very limited visibility
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("WeatherSystem: Weather changed. Visibility: %.0fm"), CurrentWeather.VisibilityMeters);
}

void UWeatherSystem::TransitionToWeather(FWeatherState TargetWeather, float TransitionTimeSeconds)
{
    this->TargetWeather = TargetWeather;
    WeatherTransitionDuration = TransitionTimeSeconds;
    WeatherTransitionProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("WeatherSystem: Starting weather transition (%.0f sec)"), TransitionTimeSeconds);
}

// ========================================================================
// HISTORICAL WEATHER
// ========================================================================

FWeatherState UWeatherSystem::GetHistoricalWeather(FHistoricalDate Date) const
{
    // Historical weather data for Verdun 1916

    FWeatherState Weather;

    // Feb 21, 1916 - Opening day
    if (Date.Month == 2 && Date.Day == 21)
    {
        Weather.Condition = EWeatherCondition::Cloudy;
        Weather.TemperatureCelsius = 2.0f;
        Weather.VisibilityMeters = 500.0f;
        Weather.CloudCoverage = 0.9f;
        return Weather;
    }

    // General seasonal patterns
    return GetDefaultWeatherForMonth(Date.Month);
}

void UWeatherSystem::ApplyHistoricalWeather()
{
    // This would get current game date from MissionSystem
    // For now, use default February weather
    FHistoricalDate CurrentDate;
    CurrentDate.Year = 1916;
    CurrentDate.Month = 2;
    CurrentDate.Day = 21;

    CurrentWeather = GetHistoricalWeather(CurrentDate);

    UE_LOG(LogTemp, Log, TEXT("WeatherSystem: Applied historical weather for %d/%d/%d"),
           CurrentDate.Month,
           CurrentDate.Day,
           CurrentDate.Year);
}

// ========================================================================
// TIME OF DAY
// ========================================================================

void UWeatherSystem::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDay = FMath::Fmod(Hours, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }

    CurrentWeather.bIsNight = IsNighttime();
}

void UWeatherSystem::AdvanceTime(float GameHours)
{
    SetTimeOfDay(CurrentTimeOfDay + GameHours);
}

bool UWeatherSystem::IsDaytime() const
{
    // Historical: Winter daylight ~8 AM to 5 PM
    return CurrentTimeOfDay >= 8.0f && CurrentTimeOfDay < 17.0f;
}

bool UWeatherSystem::IsNighttime() const
{
    return !IsDaytime();
}

FRotator UWeatherSystem::GetSunRotation() const
{
    // Calculate sun position based on time of day
    // Simplified solar angle calculation

    float SunAngle = (CurrentTimeOfDay - 12.0f) * 15.0f; // 15 degrees per hour
    return FRotator(-60.0f + SunAngle, 0.0f, 0.0f);
}

float UWeatherSystem::GetVisibilityDistance() const
{
    float Visibility = CurrentWeather.VisibilityMeters;

    // Night reduces visibility
    if (IsNighttime())
    {
        Visibility *= 0.1f; // 90% reduction at night
    }

    // Fog/rain reduces visibility
    Visibility *= (1.0f - CurrentWeather.PrecipitationIntensity * 0.5f);

    return Visibility;
}

// ========================================================================
// WEATHER EFFECTS ON GAMEPLAY
// ========================================================================

float UWeatherSystem::GetMovementSpeedModifier() const
{
    float Modifier = 1.0f;

    // Mud slows movement
    Modifier -= MudLevel * 0.5f; // Up to 50% slower

    // Rain/snow slows movement
    Modifier -= CurrentWeather.PrecipitationIntensity * 0.2f;

    return FMath::Clamp(Modifier, 0.3f, 1.0f);
}

float UWeatherSystem::GetVisibilityModifier() const
{
    float BaseVisibility = GetVisibilityDistance() / 1000.0f; // Normalized
    return FMath::Clamp(BaseVisibility, 0.1f, 1.0f);
}

float UWeatherSystem::GetWarmthDepletionModifier() const
{
    float Modifier = 1.0f;

    // Cold increases warmth depletion
    if (CurrentWeather.TemperatureCelsius < 10.0f)
    {
        float ColdIntensity = (10.0f - CurrentWeather.TemperatureCelsius) / 20.0f;
        Modifier += ColdIntensity * 2.0f; // Up to 3x depletion in extreme cold
    }

    // Wind increases depletion (wind chill)
    float WindFactor = CurrentWeather.WindSpeedKPH / 50.0f;
    Modifier += WindFactor * 0.5f;

    // Wet increases depletion
    if (bPlayerIsWet)
    {
        Modifier *= 1.5f; // 50% more depletion when wet
    }

    return Modifier;
}

bool UWeatherSystem::IsExposedToElements() const
{
    // Check if player is in shelter/dugout
    // For now, assume player is always exposed in trenches
    return true;
}

bool UWeatherSystem::IsPlayerWet() const
{
    return bPlayerIsWet;
}

// ========================================================================
// MUD SYSTEM
// ========================================================================

void UWeatherSystem::UpdateMudLevel(float DeltaTime)
{
    // Historical: Verdun became a sea of mud due to constant rain + shell impacts

    // Rain increases mud
    if (CurrentWeather.Condition == EWeatherCondition::Rainy ||
        CurrentWeather.Condition == EWeatherCondition::Stormy)
    {
        MudLevel += 0.01f * DeltaTime; // Gradual increase
    }

    // Sun dries mud (rare at Verdun winter 1916)
    if (CurrentWeather.Condition == EWeatherCondition::Clear && IsDaytime())
    {
        MudLevel -= 0.005f * DeltaTime; // Slow drying
    }

    MudLevel = FMath::Clamp(MudLevel, 0.0f, 1.0f);
}

float UWeatherSystem::GetMudMovementPenalty() const
{
    // Historical: Soldiers often sank knee-deep in mud at Verdun

    if (MudLevel > 0.8f)
    {
        return 0.3f; // 70% slower
    }
    else if (MudLevel > 0.5f)
    {
        return 0.5f; // 50% slower
    }
    else if (MudLevel > 0.3f)
    {
        return 0.8f; // 20% slower
    }

    return 1.0f; // No penalty
}

bool UWeatherSystem::IsMuddy(FVector Location) const
{
    // Check if specific location is muddy
    // For now, assume all ground is muddy if mud level > 0.3

    return MudLevel > 0.3f;
}

// ========================================================================
// SEASONAL CHANGES
// ========================================================================

FString UWeatherSystem::GetSeasonForDate(FHistoricalDate Date) const
{
    if (Date.Month >= 3 && Date.Month <= 5)
        return TEXT("Spring");
    else if (Date.Month >= 6 && Date.Month <= 8)
        return TEXT("Summer");
    else if (Date.Month >= 9 && Date.Month <= 11)
        return TEXT("Autumn");
    else
        return TEXT("Winter");
}

void UWeatherSystem::GetSeasonalTemperatureRange(FHistoricalDate Date, float& OutMinTemp, float& OutMaxTemp) const
{
    // Historical temperature ranges for Verdun region

    int32 Month = Date.Month;

    if (Month == 1 || Month == 2 || Month == 12) // Winter
    {
        OutMinTemp = -5.0f;
        OutMaxTemp = 5.0f;
    }
    else if (Month >= 3 && Month <= 5) // Spring
    {
        OutMinTemp = 5.0f;
        OutMaxTemp = 15.0f;
    }
    else if (Month >= 6 && Month <= 8) // Summer
    {
        OutMinTemp = 15.0f;
        OutMaxTemp = 25.0f;
    }
    else // Autumn
    {
        OutMinTemp = 5.0f;
        OutMaxTemp = 15.0f;
    }
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void UWeatherSystem::UpdateWeatherTransition(float DeltaTime)
{
    if (WeatherTransitionProgress >= 1.0f)
    {
        return; // No transition in progress
    }

    WeatherTransitionProgress += DeltaTime / WeatherTransitionDuration;
    WeatherTransitionProgress = FMath::Clamp(WeatherTransitionProgress, 0.0f, 1.0f);

    // Lerp weather parameters
    float Alpha = WeatherTransitionProgress;
    CurrentWeather.TemperatureCelsius = FMath::Lerp(CurrentWeather.TemperatureCelsius, TargetWeather.TemperatureCelsius, Alpha);
    CurrentWeather.WindSpeedKPH = FMath::Lerp(CurrentWeather.WindSpeedKPH, TargetWeather.WindSpeedKPH, Alpha);
    CurrentWeather.VisibilityMeters = FMath::Lerp(CurrentWeather.VisibilityMeters, TargetWeather.VisibilityMeters, Alpha);

    if (WeatherTransitionProgress >= 1.0f)
    {
        CurrentWeather = TargetWeather;
        UE_LOG(LogTemp, Log, TEXT("WeatherSystem: Weather transition complete"));
    }
}

void UWeatherSystem::UpdateTimeOfDay(float DeltaTime)
{
    // Time progression (1 real hour = TimeProgressionRate game hours)
    float GameHoursElapsed = (DeltaTime / 3600.0f) * TimeProgressionRate;
    AdvanceTime(GameHoursElapsed);
}

void UWeatherSystem::UpdatePlayerWetness(float DeltaTime)
{
    // Player gets wet in rain
    if (CurrentWeather.Condition == EWeatherCondition::Rainy ||
        CurrentWeather.Condition == EWeatherCondition::Stormy)
    {
        WetnessLevel += 0.1f * DeltaTime * CurrentWeather.PrecipitationIntensity;
    }

    // Player dries off in shelter or clear weather
    if (CurrentWeather.Condition == EWeatherCondition::Clear && !IsExposedToElements())
    {
        WetnessLevel -= 0.05f * DeltaTime;
    }

    WetnessLevel = FMath::Clamp(WetnessLevel, 0.0f, 1.0f);

    bPlayerIsWet = (WetnessLevel > 0.3f);
}

void UWeatherSystem::ApplyWeatherEffects()
{
    // Apply weather effects to survival meters

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    USurvivalSystem* SurvivalSystem = Owner->FindComponentByClass<USurvivalSystem>();
    if (!SurvivalSystem)
    {
        return;
    }

    // Cold weather depletes warmth faster
    float WarmthModifier = GetWarmthDepletionModifier();
    // This would be applied in SurvivalSystem's tick

    // Rain/cold reduces morale slightly
    if (CurrentWeather.TemperatureCelsius < 5.0f || bPlayerIsWet)
    {
        // Small continuous morale drain from miserable conditions
        // Applied over time, not immediately
    }
}

FWeatherState UWeatherSystem::GetDefaultWeatherForMonth(int32 Month) const
{
    // Historical: Verdun 1916 weather patterns

    FWeatherState Weather;

    if (Month == 2) // February - Cold, often cloudy
    {
        Weather.Condition = EWeatherCondition::Cloudy;
        Weather.TemperatureCelsius = 2.0f;
        Weather.WindSpeedKPH = 15.0f;
        Weather.VisibilityMeters = 500.0f;
        Weather.CloudCoverage = 0.8f;
    }
    else if (Month >= 3 && Month <= 5) // Spring - Rainy
    {
        Weather.Condition = EWeatherCondition::Rainy;
        Weather.TemperatureCelsius = 10.0f;
        Weather.WindSpeedKPH = 20.0f;
        Weather.VisibilityMeters = 400.0f;
        Weather.CloudCoverage = 0.9f;
        Weather.PrecipitationIntensity = 0.6f;
    }
    else if (Month >= 6 && Month <= 8) // Summer - Variable
    {
        Weather.Condition = EWeatherCondition::Cloudy;
        Weather.TemperatureCelsius = 20.0f;
        Weather.WindSpeedKPH = 10.0f;
        Weather.VisibilityMeters = 800.0f;
        Weather.CloudCoverage = 0.5f;
    }
    else if (Month >= 9 && Month <= 11) // Autumn - Rainy, cold
    {
        Weather.Condition = EWeatherCondition::Rainy;
        Weather.TemperatureCelsius = 8.0f;
        Weather.WindSpeedKPH = 25.0f;
        Weather.VisibilityMeters = 350.0f;
        Weather.CloudCoverage = 0.9f;
        Weather.PrecipitationIntensity = 0.7f;
    }
    else // December - Cold, snowy
    {
        Weather.Condition = EWeatherCondition::Snowy;
        Weather.TemperatureCelsius = -1.0f;
        Weather.WindSpeedKPH = 20.0f;
        Weather.VisibilityMeters = 300.0f;
        Weather.CloudCoverage = 1.0f;
        Weather.PrecipitationIntensity = 0.4f;
    }

    return Weather;
}
