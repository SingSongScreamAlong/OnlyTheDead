// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "VerdunTrenchGenerator.generated.h"

UENUM(BlueprintType)
enum class EVerdunTrenchType : uint8
{
	FrontLine UMETA(DisplayName = "Front Line Fire Trench"),
	Support UMETA(DisplayName = "Support Trench"),
	Reserve UMETA(DisplayName = "Reserve Trench"),
	Communication UMETA(DisplayName = "Communication Trench")
};

USTRUCT(BlueprintType)
struct FTrenchSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench")
	TArray<FVector> SplinePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench")
	EVerdunTrenchType TrenchType = EVerdunTrenchType::FrontLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench")
	float WidthCm = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench")
	float DepthCm = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench")
	bool bCollapsed = false;
};

/**
 * Generates trench systems with historically accurate zigzag patterns
 */
UCLASS(BlueprintType)
class VERDUNAUTOTERRAINBUILDER_API AVerdunTrenchGenerator : public AActor
{
	GENERATED_BODY()

public:
	AVerdunTrenchGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* RootSceneComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	class UVerdunTerrainParameters* TerrainParameters = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	class UVerdunLocationDatabase* LocationDatabase = nullptr;

	// Trench generation parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float TraverseSpacingCm = 1250.0f; // 12.5m zigzag spacing

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float TraverseAngleDegrees = 90.0f; // Angle of zigzag

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float TraverseDepthCm = 150.0f; // How deep each zigzag goes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	int32 RandomSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float TrenchCollapsePercentage = 50.0f; // Percentage of trenches that are collapsed

	// Generated trench data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	TArray<FTrenchSegment> GeneratedTrenches;

	// Spline components for visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TArray<USplineComponent*> TrenchSplines;

	/**
	 * Generate trench networks around fortifications
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateTrenchNetworks();

	/**
	 * Generate a concentric trench system around a fort
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateFortTrenches(FVector FortLocation, float RadiusMeters, int32 NumRings);

	/**
	 * Generate a linear front-line trench system
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateFrontLineTrench(FVector StartPoint, FVector EndPoint, EVerdunTrenchType TrenchType);

	/**
	 * Generate communication trenches connecting front to rear
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateCommunicationTrenches(FVector FrontLineCenter, FVector RearPosition);

	/**
	 * Clear all generated trenches
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void ClearTrenches();

	/**
	 * Create spline components for visualization in editor
	 */
	UFUNCTION(BlueprintCallable, Category = "Visualization")
	void CreateSplineVisualization();

	/**
	 * Apply trenches to landscape (excavate terrain)
	 */
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void ApplyTrenchesToLandscape(class ALandscape* Landscape);

protected:
	virtual void BeginPlay() override;

private:
	// Generate zigzag pattern for trench segment
	TArray<FVector> GenerateZigzagPattern(FVector Start, FVector End, float TraverseSpacing, float TraverseDepth, FRandomStream& RandomStream);

	// Generate curved trench segment
	TArray<FVector> GenerateCurvedTrench(FVector Start, FVector End, float CurvatureAmount, FRandomStream& RandomStream);

	// Add random variation to trench path for realism
	void AddRandomVariation(TArray<FVector>& Points, float VariationAmount, FRandomStream& RandomStream);
};
