#pragma once

#include "CoreMinimal.h"
#include "ArtilleryTypes.generated.h"

// Shell types matching the game design document
// Historical accuracy: 70-75% of Verdun casualties came from artillery
UENUM(BlueprintType)
enum class EShellType : uint8
{
    Shell75mm    UMETA(DisplayName = "75mm Field Gun (French Soixante-Quinze)"),
    Shell77mm    UMETA(DisplayName = "77mm Feldkanone (German)"),
    Shell155mm   UMETA(DisplayName = "155mm C/1915 Howitzer"),
    Shell210mm   UMETA(DisplayName = "210mm Morser (Heavy German)"),
    Shell305mm   UMETA(DisplayName = "305mm Siege Mortar"),
    GasShell     UMETA(DisplayName = "Gas Shell (Phosgene/Chlorine)")
};

UENUM(BlueprintType)
enum class EBarragePattern : uint8
{
    Random          UMETA(DisplayName = "Random Scatter"),
    LinearWalking   UMETA(DisplayName = "Walking/Creeping Barrage"),
    Concentrated    UMETA(DisplayName = "Concentrated Zone")
};

// All distances in centimeters (UE units = 1cm)
// Radii derived from game design document historical data
USTRUCT(BlueprintType)
struct FShellData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell")
    EShellType Type = EShellType::Shell75mm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell")
    FString DisplayName = TEXT("75mm Field Gun");

    // ---- Damage radii (in cm = UE units) ----
    // Source: game design doc, derived from WWI ordnance records

    // Instant kill within this radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Damage")
    float LethalRadius = 500.0f;        // 5m

    // Wound/injury probability zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Damage")
    float WoundRadius = 1500.0f;        // 15m

    // Morale/shell-shock effect zone (no physical injury, psychological)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Damage")
    float ShockRadius = 3000.0f;        // 30m

    // Terrain deformation / crater radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Damage")
    float CraterRadius = 200.0f;        // 2m

    // Raw damage at center of explosion
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Damage")
    float MaxDamage = 300.0f;

    // ---- Morale damage ----
    // Morale is 0-100; shell shock triggers at < 30

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Morale")
    float MoraleDamageLethal = 35.0f;   // If you survive a near-lethal hit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Morale")
    float MoraleDamageShock = 10.0f;    // Distant explosion, heard/felt

    // ---- Flight characteristics ----

    // cm/s — higher = flatter arc / less hang time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Flight")
    float LaunchSpeed = 6000.0f;

    // Seconds player hears whistle BEFORE impact (the game's core audio mechanic)
    // Ranges 1-30s per design doc. Field guns ~3s, heavy mortars ~12s
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Flight")
    float IncomingWhistleSeconds = 3.0f;

    // Historical shell weight (affects perceived power, screen shake magnitude)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Flight")
    float ShellWeightKg = 7.7f;
};

// -----------------------------------------------------------------------
// Preset shell data matching the game design document specifications
// -----------------------------------------------------------------------
namespace ShellPresets
{
    inline FShellData Make75mm()
    {
        FShellData D;
        D.Type = EShellType::Shell75mm;
        D.DisplayName = TEXT("75mm Soixante-Quinze (French Field Gun)");
        D.LethalRadius       = 500.0f;
        D.WoundRadius        = 1500.0f;
        D.ShockRadius        = 3000.0f;
        D.CraterRadius       = 200.0f;
        D.MaxDamage          = 300.0f;
        D.MoraleDamageLethal = 35.0f;
        D.MoraleDamageShock  = 8.0f;
        D.LaunchSpeed        = 6000.0f;
        D.IncomingWhistleSeconds = 3.0f;
        D.ShellWeightKg      = 7.7f;
        return D;
    }

    inline FShellData Make77mm()
    {
        FShellData D;
        D.Type = EShellType::Shell77mm;
        D.DisplayName = TEXT("77mm Feldkanone (German Field Gun)");
        D.LethalRadius       = 600.0f;
        D.WoundRadius        = 1800.0f;
        D.ShockRadius        = 3500.0f;
        D.CraterRadius       = 250.0f;
        D.MaxDamage          = 350.0f;
        D.MoraleDamageLethal = 35.0f;
        D.MoraleDamageShock  = 8.0f;
        D.LaunchSpeed        = 5800.0f;
        D.IncomingWhistleSeconds = 3.5f;
        D.ShellWeightKg      = 6.85f;
        return D;
    }

    inline FShellData Make155mm()
    {
        FShellData D;
        D.Type = EShellType::Shell155mm;
        D.DisplayName = TEXT("155mm C/1915 Howitzer");
        D.LethalRadius       = 1500.0f;
        D.WoundRadius        = 4000.0f;
        D.ShockRadius        = 8000.0f;
        D.CraterRadius       = 600.0f;
        D.MaxDamage          = 750.0f;
        D.MoraleDamageLethal = 55.0f;
        D.MoraleDamageShock  = 20.0f;
        D.LaunchSpeed        = 4500.0f;
        D.IncomingWhistleSeconds = 8.0f;
        D.ShellWeightKg      = 43.0f;
        return D;
    }

    inline FShellData Make210mm()
    {
        FShellData D;
        D.Type = EShellType::Shell210mm;
        D.DisplayName = TEXT("210mm Morser (German Heavy Howitzer)");
        D.LethalRadius       = 3000.0f;
        D.WoundRadius        = 8000.0f;
        D.ShockRadius        = 15000.0f;
        D.CraterRadius       = 1200.0f;
        D.MaxDamage          = 1500.0f;
        D.MoraleDamageLethal = 70.0f;
        D.MoraleDamageShock  = 35.0f;
        D.LaunchSpeed        = 4000.0f;
        D.IncomingWhistleSeconds = 12.0f;
        D.ShellWeightKg      = 121.0f;
        return D;
    }

    inline FShellData Make305mm()
    {
        FShellData D;
        D.Type = EShellType::Shell305mm;
        D.DisplayName = TEXT("305mm Siege Mortar (\"Big Bertha\" class)");
        D.LethalRadius       = 5000.0f;
        D.WoundRadius        = 12000.0f;
        D.ShockRadius        = 25000.0f;
        D.CraterRadius       = 2500.0f;
        D.MaxDamage          = 3000.0f;
        D.MoraleDamageLethal = 90.0f;
        D.MoraleDamageShock  = 50.0f;
        D.LaunchSpeed        = 3500.0f;
        D.IncomingWhistleSeconds = 20.0f;
        D.ShellWeightKg      = 380.0f;
        return D;
    }

    inline FShellData GetForType(EShellType Type)
    {
        switch (Type)
        {
            case EShellType::Shell75mm:  return Make75mm();
            case EShellType::Shell77mm:  return Make77mm();
            case EShellType::Shell155mm: return Make155mm();
            case EShellType::Shell210mm: return Make210mm();
            case EShellType::Shell305mm: return Make305mm();
            default:                     return Make75mm();
        }
    }
}
