// Only The Dead - Cover Detection Component
// Procedural detection of artillery shelter (trenches, craters, dugouts)
// Non-sticky, geometry-aware system built for WWI trench warfare

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OTDCoverDetectionComponent.generated.h"

/**
 * Cover type determining protection level from artillery
 * Based on historical data from design document
 */
UENUM(BlueprintType)
enum class EOTDCoverType : uint8
{
	None UMETA(DisplayName = "Open Ground (0% protection)"),
	ShallowTrench UMETA(DisplayName = "Shallow Trench (60% protection)"),
	ShellCrater UMETA(DisplayName = "Shell Crater (75% protection)"),
	DugoutEntrance UMETA(DisplayName = "Dugout Entrance (85% protection)"),
	DeepDugout UMETA(DisplayName = "Deep Dugout (95% protection)"),
	ConcreteBunker UMETA(DisplayName = "Concrete Bunker (99% protection)")
};

/**
 * Cover quality/height for posture determination
 */
UENUM(BlueprintType)
enum class EOTDCoverHeight : uint8
{
	None,
	Low,      // Knee to waist height
	High      // Waist to head height
};

/**
 * Result of cover detection trace fan
 */
USTRUCT(BlueprintType)
struct FOTDCoverInfo
{
	GENERATED_BODY()

	/** Type of cover detected */
	UPROPERTY(BlueprintReadOnly)
	EOTDCoverType CoverType = EOTDCoverType::None;

	/** Height/quality of cover */
	UPROPERTY(BlueprintReadOnly)
	EOTDCoverHeight CoverHeight = EOTDCoverHeight::None;

	/** Protection percentage (0.0 to 1.0) */
	UPROPERTY(BlueprintReadOnly)
	float ProtectionValue = 0.0f;

	/** Normal vector of cover surface (for lean direction) */
	UPROPERTY(BlueprintReadOnly)
	FVector CoverNormal = FVector::ZeroVector;

	/** World location of cover surface */
	UPROPERTY(BlueprintReadOnly)
	FVector CoverLocation = FVector::ZeroVector;

	/** Whether character can peek over/around this cover */
	UPROPERTY(BlueprintReadOnly)
	bool bCanPeek = false;

	/** Is this cover degraded (damaged by shells)? */
	UPROPERTY(BlueprintReadOnly)
	bool bIsDegraded = false;

	/** Distance to cover in cm */
	UPROPERTY(BlueprintReadOnly)
	float DistanceToCover = 0.0f;
};

/**
 * Delegate for cover state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoverStateChanged, const FOTDCoverInfo&, CoverInfo);

/**
 * Cover Detection Component
 * Continuously traces for cover around character and evaluates protection
 * Non-sticky: just provides information, doesn't snap/lock character
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UOTDCoverDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOTDCoverDetectionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== Cover Query ==========

	/**
	 * Get current cover information
	 */
	UFUNCTION(BlueprintPure, Category = "Cover")
	FOTDCoverInfo GetCurrentCover() const { return CurrentCoverInfo; }

	/**
	 * Check if character is currently near cover
	 */
	UFUNCTION(BlueprintPure, Category = "Cover")
	bool IsNearCover() const { return CurrentCoverInfo.CoverType != EOTDCoverType::None; }

	/**
	 * Get protection value for current cover (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Cover")
	float GetProtectionValue() const { return CurrentCoverInfo.ProtectionValue; }

	/**
	 * Check if can peek over current cover
	 */
	UFUNCTION(BlueprintPure, Category = "Cover")
	bool CanPeekFromCover() const { return CurrentCoverInfo.bCanPeek; }

	/**
	 * Get direction to lean for best cover (normalized)
	 */
	UFUNCTION(BlueprintPure, Category = "Cover")
	FVector GetCoverDirection() const { return CurrentCoverInfo.CoverNormal; }

	// ========== Manual Query ==========

	/**
	 * Perform manual cover detection at specific location
	 */
	UFUNCTION(BlueprintCallable, Category = "Cover")
	FOTDCoverInfo DetectCoverAtLocation(const FVector& Location, const FRotator& Facing);

	/**
	 * Check if location provides overhead protection from artillery
	 */
	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool HasOverheadProtection(const FVector& Location) const;

	// ========== Events ==========

	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverStateChanged OnEnteredCover;

	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverStateChanged OnExitedCover;

	UPROPERTY(BlueprintAssignable, Category = "Cover|Events")
	FOnCoverStateChanged OnCoverChanged;

protected:
	// ========== Current State ==========

	UPROPERTY(BlueprintReadOnly, Category = "Cover")
	FOTDCoverInfo CurrentCoverInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Cover")
	FOTDCoverInfo PreviousCoverInfo;

	// ========== Detection Settings ==========

	/**
	 * Distance to trace for cover detection (cm)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Settings")
	float CoverDetectionRange = 100.0f;

	/**
	 * How often to update cover detection (seconds)
	 * Lower = more responsive, higher = better performance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Settings")
	float DetectionInterval = 0.1f;

	/**
	 * Number of traces in the fan (more = more accurate, fewer = faster)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Settings")
	int32 TraceFanCount = 5;

	/**
	 * Debug visualization
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Settings")
	bool bDebugDrawTraces = false;

	// ========== Protection Values (from design doc) ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Protection")
	float ShallowTrenchProtection = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Protection")
	float ShellCraterProtection = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Protection")
	float DugoutEntranceProtection = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Protection")
	float DeepDugoutProtection = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Protection")
	float ConcreteBunkerProtection = 0.99f;

	// ========== Internal State ==========

	float TimeSinceLastDetection = 0.0f;

	// ========== Internal Methods ==========

	/**
	 * Perform cover detection trace fan
	 */
	void PerformCoverDetection();

	/**
	 * Evaluate surface from hit result to determine cover type
	 */
	FOTDCoverInfo EvaluateCoverSurface(const TArray<FHitResult>& HitResults) const;

	/**
	 * Determine cover type from geometry and tags
	 */
	EOTDCoverType DetermineCoverType(const FHitResult& HitResult) const;

	/**
	 * Calculate protection value for cover type
	 */
	float GetProtectionForCoverType(EOTDCoverType CoverType) const;

	/**
	 * Check if cover surface is suitable (vertical, proper height)
	 */
	bool IsSuitableCoverSurface(const FHitResult& HitResult, float CharacterHeight) const;

	/**
	 * Perform trace fan from character position
	 */
	TArray<FHitResult> PerformTraceFan(const FVector& Start, const FVector& Forward, float Range) const;

	/**
	 * Check for overhead protection (dugout roof, bunker ceiling)
	 */
	bool CheckOverheadProtection(const FVector& Location) const;

	/**
	 * Broadcast cover state change events
	 */
	void BroadcastCoverChange();
};
