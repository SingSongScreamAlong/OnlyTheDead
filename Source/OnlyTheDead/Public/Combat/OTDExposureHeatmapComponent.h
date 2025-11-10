// Only The Dead - Exposure Heatmap Component
// Tracks player visibility/exposure for artillery and sniper AI targeting
// "The unique twist" - exposed players attract more fire

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OTDExposureHeatmapComponent.generated.h"

/**
 * Exposure level categories
 */
UENUM(BlueprintType)
enum class EOTDExposureLevel : uint8
{
	None UMETA(DisplayName = "Hidden"),
	Low UMETA(DisplayName = "Minimal Exposure"),
	Medium UMETA(DisplayName = "Moderate Exposure"),
	High UMETA(DisplayName = "High Exposure"),
	Critical UMETA(DisplayName = "Critical Exposure")
};

/**
 * Delegate for exposure changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExposureChanged, EOTDExposureLevel, OldLevel, EOTDExposureLevel, NewLevel);

/**
 * Exposure Heatmap Component
 *
 * Tracks how exposed the player is to enemy observation/fire.
 * High exposure attracts:
 * - Artillery fire (ranging shots that walk in)
 * - Sniper attention
 * - Increased suppression
 * - Higher enemy spotting probability
 *
 * Teaches players to use cover, move smart, and avoid lingering in the open.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UOTDExposureHeatmapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOTDExposureHeatmapComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== Exposure Query ==========

	/**
	 * Get current exposure value (0.0 to 100.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Exposure")
	float GetExposureValue() const { return CurrentExposure; }

	/**
	 * Get current exposure level category
	 */
	UFUNCTION(BlueprintPure, Category = "Exposure")
	EOTDExposureLevel GetExposureLevel() const;

	/**
	 * Check if player is critically exposed (imminent artillery danger)
	 */
	UFUNCTION(BlueprintPure, Category = "Exposure")
	bool IsCriticallyExposed() const { return GetExposureLevel() == EOTDExposureLevel::Critical; }

	/**
	 * Get time spent at current exposure level (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Exposure")
	float GetTimeAtCurrentExposure() const { return TimeAtCurrentLevel; }

	/**
	 * Get artillery attraction score (0.0 to 1.0)
	 * Used by EQS for AI targeting
	 */
	UFUNCTION(BlueprintPure, Category = "Exposure|AI")
	float GetArtilleryAttractionScore() const;

	/**
	 * Get sniper attraction score (0.0 to 1.0)
	 * Used by EQS for AI targeting
	 */
	UFUNCTION(BlueprintPure, Category = "Exposure|AI")
	float GetSniperAttractionScore() const;

	// ========== Manual Control ==========

	/**
	 * Add exposure (called when visible to enemy sectors)
	 */
	UFUNCTION(BlueprintCallable, Category = "Exposure")
	void AddExposure(float Amount);

	/**
	 * Force set exposure value
	 */
	UFUNCTION(BlueprintCallable, Category = "Exposure")
	void SetExposure(float Value);

	/**
	 * Reset exposure (when entering deep cover)
	 */
	UFUNCTION(BlueprintCallable, Category = "Exposure")
	void ResetExposure();

	// ========== Events ==========

	UPROPERTY(BlueprintAssignable, Category = "Exposure|Events")
	FOnExposureChanged OnExposureLevelChanged;

protected:
	// ========== Current State ==========

	/**
	 * Current exposure value (0.0 to 100.0)
	 * Built up over time when visible, decays when hidden
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Exposure")
	float CurrentExposure = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Exposure")
	EOTDExposureLevel CurrentLevel = EOTDExposureLevel::None;

	UPROPERTY(BlueprintReadOnly, Category = "Exposure")
	float TimeAtCurrentLevel = 0.0f;

	/**
	 * Total time spent exposed (for statistics)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Exposure|Stats")
	float TotalExposureTime = 0.0f;

	/**
	 * Number of times artillery was attracted
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Exposure|Stats")
	int32 ArtilleryAttractedCount = 0;

	// ========== Settings ==========

	/**
	 * Exposure accumulation rate per second when visible
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Settings")
	float ExposureAccumulationRate = 10.0f; // Per second

	/**
	 * Exposure decay rate per second when in cover
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Settings")
	float ExposureDecayRate = 5.0f; // Per second

	/**
	 * Time at high/critical exposure before artillery is attracted (seconds)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Settings")
	float ArtilleryAttractionThreshold = 3.0f;

	/**
	 * Multiplier for exposure when moving vs stationary
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Settings")
	float MovementExposureMultiplier = 1.5f;

	/**
	 * Multiplier for exposure when firing weapon
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Settings")
	float FiringExposureMultiplier = 2.0f;

	/**
	 * Update interval for visibility checks (seconds)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Settings")
	float VisibilityCheckInterval = 0.5f;

	// ========== Exposure Level Thresholds ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Thresholds")
	float LowExposureThreshold = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Thresholds")
	float MediumExposureThreshold = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Thresholds")
	float HighExposureThreshold = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposure|Thresholds")
	float CriticalExposureThreshold = 80.0f;

	// ========== Internal State ==========

	float TimeSinceLastVisibilityCheck = 0.0f;
	bool bWasVisibleLastFrame = false;
	float CurrentExposureMultiplier = 1.0f;

	// ========== Internal Methods ==========

	/**
	 * Check if player is visible to any enemy sector
	 */
	void UpdateVisibility(float DeltaTime);

	/**
	 * Update exposure value based on visibility and cover
	 */
	void UpdateExposure(float DeltaTime);

	/**
	 * Check for exposure level transitions
	 */
	void UpdateExposureLevel();

	/**
	 * Calculate if player is visible from enemy positions
	 */
	bool IsVisibleToEnemies() const;

	/**
	 * Notify AI systems of high exposure (attracts artillery/snipers)
	 */
	void NotifyAIOfHighExposure();
};
