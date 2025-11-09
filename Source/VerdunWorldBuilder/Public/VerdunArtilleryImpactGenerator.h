// Verdun World Builder Plugin
// Artillery impact generation system - USER IMPLEMENTS THIS to generate craters/trenches

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VerdunArtilleryImpactGenerator.generated.h"

/**
 * Shell types for artillery impacts
 */
UENUM(BlueprintType)
enum class EVerdunShellType : uint8
{
	Field75mm UMETA(DisplayName = "75mm Field Gun (French)"),
	Howitzer155mm UMETA(DisplayName = "155mm Howitzer"),
	Heavy305mm UMETA(DisplayName = "305mm Heavy Howitzer"),
	SuperHeavy420mm UMETA(DisplayName = "420mm Super-Heavy (Big Bertha)")
};

/**
 * Shell impact data
 */
USTRUCT(BlueprintType)
struct FVerdunShellImpact
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Impact")
	EVerdunShellType ShellType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Impact")
	FVector ImpactLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Impact")
	float CraterDiameterMeters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Impact")
	float CraterDepthMeters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Impact")
	float TimeStamp;

	FVerdunShellImpact()
		: ShellType(EVerdunShellType::Field75mm)
		, ImpactLocation(FVector::ZeroVector)
		, CraterDiameterMeters(2.0f)
		, CraterDepthMeters(0.5f)
		, TimeStamp(0.0f)
	{}
};

/**
 * Crater density zone
 */
USTRUCT(BlueprintType)
struct FVerdunCraterDensityZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater Density")
	FVector ZoneCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater Density")
	float ZoneRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater Density")
	float CratersPerHectare;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater Density")
	FString IntensityLevel; // "high", "medium", "low"

	FVerdunCraterDensityZone()
		: ZoneCenter(FVector::ZeroVector)
		, ZoneRadius(1000.0f)
		, CratersPerHectare(50.0f)
		, IntensityLevel(TEXT("medium"))
	{}
};

/**
 * Interface for artillery impact generation
 * IMPLEMENT THIS IN YOUR OWN CLASS to generate craters, trenches, and terrain modifications
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UVerdunArtilleryImpactGenerator : public UInterface
{
	GENERATED_BODY()
};

class VERDUNWORLDBUILDER_API IVerdunArtilleryImpactGenerator
{
	GENERATED_BODY()

public:
	/**
	 * Generate shell impacts for a given area
	 * @param ZoneCenter Center of the impact zone
	 * @param ZoneRadius Radius of the zone in cm
	 * @param TargetDensity Target craters per hectare
	 * @param OutImpacts Generated shell impacts
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Verdun|Artillery")
	void GenerateShellImpacts(FVector ZoneCenter, float ZoneRadius, float TargetDensity, TArray<FVerdunShellImpact>& OutImpacts);

	/**
	 * Apply a single crater to the landscape
	 * @param Impact Shell impact data
	 * @param Landscape Target landscape actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Verdun|Artillery")
	void ApplyCraterToLandscape(const FVerdunShellImpact& Impact, ALandscape* Landscape);

	/**
	 * Generate trench network
	 * @param StartPoint Trench start position
	 * @param EndPoint Trench end position
	 * @param TrenchWidth Width in cm (default 150cm)
	 * @param TrenchDepth Depth in cm (default 200cm)
	 * @param ZigzagFrequency Zigzag pattern frequency
	 * @param Landscape Target landscape actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Verdun|Artillery")
	void GenerateTrench(FVector StartPoint, FVector EndPoint, float TrenchWidth, float TrenchDepth, float ZigzagFrequency, ALandscape* Landscape);

	/**
	 * Get recommended crater dimensions for shell type
	 * @param ShellType Type of artillery shell
	 * @param OutDiameterMeters Crater diameter in meters
	 * @param OutDepthMeters Crater depth in meters
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Verdun|Artillery")
	void GetCraterDimensionsForShellType(EVerdunShellType ShellType, float& OutDiameterMeters, float& OutDepthMeters);
};

/**
 * Default implementation of artillery impact generator
 * Provides basic crater/trench generation - extend this for custom behavior
 */
UCLASS(Blueprintable, BlueprintType)
class VERDUNWORLDBUILDER_API UVerdunDefaultArtilleryGenerator : public UObject, public IVerdunArtilleryImpactGenerator
{
	GENERATED_BODY()

public:
	UVerdunDefaultArtilleryGenerator();

	// IVerdunArtilleryImpactGenerator interface
	virtual void GenerateShellImpacts_Implementation(FVector ZoneCenter, float ZoneRadius, float TargetDensity, TArray<FVerdunShellImpact>& OutImpacts) override;
	virtual void ApplyCraterToLandscape_Implementation(const FVerdunShellImpact& Impact, ALandscape* Landscape) override;
	virtual void GenerateTrench_Implementation(FVector StartPoint, FVector EndPoint, float TrenchWidth, float TrenchDepth, float ZigzagFrequency, ALandscape* Landscape) override;
	virtual void GetCraterDimensionsForShellType_Implementation(EVerdunShellType ShellType, float& OutDiameterMeters, float& OutDepthMeters) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Settings")
	int32 RandomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Settings")
	float SmallCraterPercentage; // 75mm - 60%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Settings")
	float MediumCraterPercentage; // 155mm - 25%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Settings")
	float LargeCraterPercentage; // 305mm - 10%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artillery Settings")
	float HugeCraterPercentage; // 420mm - 5%
};
