// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "VerdunContentGenerator.generated.h"

/**
 * Trench configuration
 */
USTRUCT(BlueprintType)
struct FTrenchConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	float Depth = 200.0f; // 2 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	float Width = 150.0f; // 1.5 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	float DugoutSpacing = 5000.0f; // Every 50 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	bool bAddSandbags = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	bool bAddDuckboards = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	bool bAddFiringSteps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	int32 DugoutCount = 0; // 0 = auto-calculate based on spacing

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	bool bZigzag = true; // WWI trenches were zigzagged to prevent enfilade fire
};

/**
 * Artillery crater configuration
 */
USTRUCT(BlueprintType)
struct FArtilleryCraterConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	float Diameter = 800.0f; // 8 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	float Depth = 150.0f; // 1.5 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	bool bFillWithWater = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	float EdgeRoughness = 0.3f; // 0-1, randomness of crater edge
};

/**
 * Fortification configuration
 */
USTRUCT(BlueprintType)
struct FFortificationConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	FString FortType; // "Douaumont", "Vaux", "Small"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	bool bAddBarbedWire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	bool bAddMachineGunNests = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	float DamageLevel = 0.5f; // 0-1, amount of damage/destruction
};

/**
 * Verdun Content Generator - Specialized tools for WWI Verdun content
 */
UCLASS(BlueprintType)
class VERDUNAIAGENT_API UVerdunContentGenerator : public UObject
{
	GENERATED_BODY()

public:
	UVerdunContentGenerator();

	/**
	 * Generate a French trench system along a spline path
	 * @param World - The world to spawn in
	 * @param SplinePath - Array of points defining the trench path
	 * @param Config - Trench configuration
	 * @return The spawned trench actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Content Generation")
	static AActor* GenerateTrenchSystem(
		UWorld* World,
		const TArray<FVector>& SplinePath,
		const FTrenchConfiguration& Config
	);

	/**
	 * Generate a German trench system (slightly different specifications)
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Content Generation")
	static AActor* GenerateGermanTrenchSystem(
		UWorld* World,
		const TArray<FVector>& SplinePath,
		const FTrenchConfiguration& Config
	);

	/**
	 * Create an artillery crater at a location
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Content Generation")
	static AActor* CreateArtilleryCrater(
		UWorld* World,
		const FVector& Location,
		const FArtilleryCraterConfig& Config
	);

	/**
	 * Generate No Man's Land between two trench lines
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Content Generation")
	static void GenerateNoMansLand(
		UWorld* World,
		const TArray<FVector>& FrenchTrenchLine,
		const TArray<FVector>& GermanTrenchLine,
		int32 CraterCount,
		bool bAddBarbedWire
	);

	/**
	 * Create a fortification (Fort Douaumont, Fort Vaux, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Content Generation")
	static AActor* CreateFortification(
		UWorld* World,
		const FVector& Location,
		const FRotator& Rotation,
		const FFortificationConfig& Config
	);

	/**
	 * Generate a destroyed village
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Content Generation")
	static void GenerateDestroyedVillage(
		UWorld* World,
		const FVector& CenterLocation,
		float Radius,
		int32 BuildingCount,
		float DestructionLevel
	);

	/**
	 * Create a communication trench connecting two trenches
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Content Generation")
	static AActor* CreateCommunicationTrench(
		UWorld* World,
		const FVector& StartPoint,
		const FVector& EndPoint,
		const FTrenchConfiguration& Config
	);

	/**
	 * Generate barbed wire obstacles
	 */
	UFUNCTION(BlueprintCallable, Category = "Verdun|Content Generation")
	static void GenerateBarbedWire(
		UWorld* World,
		const TArray<FVector>& Path,
		int32 Rows,
		float Height
	);

private:
	// Helper functions
	static TArray<FVector> GenerateZigzagPath(const TArray<FVector>& BasePath, float ZigzagDistance);
	static void CreateTrenchMesh(UWorld* World, AActor* TrenchActor, const TArray<FVector>& Path, const FTrenchConfiguration& Config);
	static void AddDugout(UWorld* World, AActor* ParentActor, const FVector& Location, const FRotator& Rotation);
	static void AddSandbags(UWorld* World, AActor* ParentActor, const TArray<FVector>& Path);
	static void AddDuckboards(UWorld* World, AActor* ParentActor, const TArray<FVector>& Path);
	static void SculptTerrainForTrench(UWorld* World, const TArray<FVector>& Path, float Width, float Depth);
};

/**
 * Actor that represents a trench system
 */
UCLASS()
class VERDUNAIAGENT_API ATrenchSystemActor : public AActor
{
	GENERATED_BODY()

public:
	ATrenchSystemActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* TrenchPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	FTrenchConfiguration Configuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verdun")
	bool bIsFrenchTrench = true;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
};
