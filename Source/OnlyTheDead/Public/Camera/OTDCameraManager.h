// Only The Dead - Camera Manager
// Hellblade 2-inspired hybrid camera system with multiple contextual modes

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "OTDCameraManager.generated.h"

class UOTDCameraDataAsset;

/**
 * Camera modes for context-driven cinematic framing
 */
UENUM(BlueprintType)
enum class EOTDCameraMode : uint8
{
	// Subtle spline/target framing during story beats, patrols, R&R
	GuidedWalk UMETA(DisplayName = "Guided Walk (Cinematic Explore)"),

	// Over-the-shoulder, shoulder-swapable default play mode
	SurvivalOTS UMETA(DisplayName = "Survival OTS (Default Play)"),

	// Dynamic zoom, hand sway, breath system for aiming
	PrecisionADS UMETA(DisplayName = "Precision ADS (Aim-Down-Sight)"),

	// Brief locked angles for shock moments (artillery hits, etc)
	LockFramedSetPiece UMETA(DisplayName = "Lock-Framed Set Piece"),

	MAX UMETA(Hidden)
};

/**
 * Shoulder side for OTS camera positioning
 */
UENUM(BlueprintType)
enum class EOTDShoulderSide : uint8
{
	Left,
	Right
};

/**
 * Camera state data for smooth blending between modes
 */
USTRUCT(BlueprintType)
struct FOTDCameraState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOTDCameraMode Mode = EOTDCameraMode::SurvivalOTS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOV = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraLag = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraRotationLag = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PostProcessWeight = 0.0f;

	// Blend time to transition to this state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendTime = 0.25f;
};

/**
 * Custom Player Camera Manager with Hellblade 2-style contextual framing
 * Manages smooth transitions between cinematic and gameplay camera modes
 */
UCLASS()
class ONLYTHEDEAD_API AOTDCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AOTDCameraManager();

protected:
	virtual void BeginPlay() override;
	virtual void UpdateCamera(float DeltaTime) override;

public:
	// ========== Mode Management ==========

	/**
	 * Request a camera mode transition with optional blend override
	 * @param NewMode - Target camera mode
	 * @param BlendTimeOverride - Optional custom blend duration (uses mode default if <= 0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(EOTDCameraMode NewMode, float BlendTimeOverride = -1.0f);

	/**
	 * Get current active camera mode
	 */
	UFUNCTION(BlueprintPure, Category = "Camera")
	EOTDCameraMode GetCameraMode() const { return CurrentMode; }

	/**
	 * Check if currently blending between modes
	 */
	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsBlending() const { return BlendAlpha < 1.0f; }

	// ========== Shoulder Management ==========

	/**
	 * Toggle between left and right shoulder for OTS mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SwapShoulder();

	/**
	 * Set specific shoulder side
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetShoulderSide(EOTDShoulderSide NewSide);

	/**
	 * Get current shoulder side
	 */
	UFUNCTION(BlueprintPure, Category = "Camera")
	EOTDShoulderSide GetShoulderSide() const { return CurrentShoulder; }

	// ========== Camera Modifiers ==========

	/**
	 * Apply suppression effects to camera (blur, grain, shake)
	 * @param SuppressionLevel - 0.0 to 1.0, where 1.0 is maximum suppression
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplySuppressionEffect(float SuppressionLevel);

	/**
	 * Apply breath sway to ADS camera based on stamina
	 * @param StaminaPercent - 0.0 to 1.0, where 1.0 is full stamina
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplyBreathSway(float StaminaPercent);

	/**
	 * Apply camera shake for artillery impact
	 * @param ImpactLocation - World location of shell impact
	 * @param ShellCaliberMM - Shell caliber in millimeters (75, 155, 210, etc)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplyArtilleryShake(const FVector& ImpactLocation, float ShellCaliberMM);

	// ========== Accessibility ==========

	/**
	 * Toggle whether set-piece camera locks are allowed (accessibility)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Accessibility")
	bool bAllowSetPieceLocks = true;

	/**
	 * Fixed horizon mode to reduce motion sickness
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Accessibility")
	bool bFixedHorizon = false;

	/**
	 * Reduced camera lag multiplier for motion sensitivity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Accessibility", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CameraLagMultiplier = 1.0f;

protected:
	// ========== Camera State ==========

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	EOTDCameraMode CurrentMode = EOTDCameraMode::SurvivalOTS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	EOTDCameraMode TargetMode = EOTDCameraMode::SurvivalOTS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	EOTDShoulderSide CurrentShoulder = EOTDShoulderSide::Right;

	// Blend progress (0.0 = current state, 1.0 = target state)
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	float BlendAlpha = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	float BlendDuration = 0.25f;

	// ========== Camera Data Assets ==========

	/**
	 * Data assets defining camera parameters for each mode
	 * Assigned in Blueprint or constructor
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Data Assets")
	TMap<EOTDCameraMode, UOTDCameraDataAsset*> CameraModeDataAssets;

	// ========== Camera State Caching ==========

	FOTDCameraState CurrentCameraState;
	FOTDCameraState TargetCameraState;

	// ========== Internal Helpers ==========

	/**
	 * Update blend between camera states
	 */
	void UpdateCameraBlend(float DeltaTime);

	/**
	 * Apply current camera state to viewport
	 */
	void ApplyCameraState(const FOTDCameraState& State, float Alpha);

	/**
	 * Load camera state from data asset for given mode
	 */
	FOTDCameraState LoadCameraStateForMode(EOTDCameraMode Mode) const;

	/**
	 * Calculate shoulder offset based on current side
	 */
	FVector GetShoulderOffset() const;

	/**
	 * Calculate camera shake intensity based on distance to explosion
	 */
	float CalculateShakeIntensity(const FVector& ImpactLocation, float ShellCaliberMM) const;

	// ========== Effect State ==========

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Effects")
	float CurrentSuppressionLevel = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Effects")
	FVector2D BreathSwayOffset = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Effects")
	float BreathSwayPhase = 0.0f;
};
