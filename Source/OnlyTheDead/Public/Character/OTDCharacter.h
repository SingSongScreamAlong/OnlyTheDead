// Only The Dead - Player Character
// Main character with survival meters, posture system, and camera integration

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/OTDCameraManager.h"
#include "OTDCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UOTDSurvivalComponent;
class UOTDCoverDetectionComponent;
struct FInputActionValue;

/**
 * Character posture/stance states
 * Tied to survival meters and cover detection
 */
UENUM(BlueprintType)
enum class EOTDPosture : uint8
{
	Standing UMETA(DisplayName = "Standing"),
	Crouching UMETA(DisplayName = "Crouching"),
	Prone UMETA(DisplayName = "Prone"),

	MAX UMETA(Hidden)
};

/**
 * Main player character for Only The Dead
 * Integrates survival meters, posture system, cover detection, and Hellblade 2-style camera
 */
UCLASS()
class ONLYTHEDEAD_API AOTDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOTDCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// ========== Components ==========

	/**
	 * Spring arm for camera positioning with shoulder offset
	 * Handles camera collision and smooth following
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	/**
	 * Follow camera attached to spring arm
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	/**
	 * Survival meters component (hunger, thirst, stamina, warmth, etc)
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
	UOTDSurvivalComponent* SurvivalComponent;

	/**
	 * Cover detection component for artillery shelter
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	UOTDCoverDetectionComponent* CoverDetection;

	// ========== Posture System ==========

	/**
	 * Get current posture/stance
	 */
	UFUNCTION(BlueprintPure, Category = "Character|Posture")
	EOTDPosture GetPosture() const { return CurrentPosture; }

	/**
	 * Request posture change (may be denied by stamina or context)
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Posture")
	void SetPosture(EOTDPosture NewPosture);

	/**
	 * Force immediate posture change (for artillery drop-prone reactions)
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Posture")
	void ForcePosture(EOTDPosture NewPosture, float TransitionTime = 1.0f);

	/**
	 * Check if character can transition to target posture
	 */
	UFUNCTION(BlueprintPure, Category = "Character|Posture")
	bool CanTransitionToPosture(EOTDPosture TargetPosture) const;

	// ========== Camera Control ==========

	/**
	 * Toggle shoulder for OTS camera
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void SwapShoulder();

	/**
	 * Get current shoulder side
	 */
	UFUNCTION(BlueprintPure, Category = "Character|Camera")
	EOTDShoulderSide GetShoulderSide() const { return CurrentShoulder; }

	/**
	 * Enter/exit aim down sights mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void SetAiming(bool bIsAiming);

	/**
	 * Check if currently aiming
	 */
	UFUNCTION(BlueprintPure, Category = "Character|Camera")
	bool IsAiming() const { return bIsAiming; }

	// ========== Leaning ==========

	/**
	 * Apply lean (tap for quick peek, hold for sustained)
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void SetLeaning(float LeanAmount); // -1.0 (left) to 1.0 (right)

	/**
	 * Get current lean amount
	 */
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	float GetLeanAmount() const { return CurrentLean; }

	// ========== Movement Modifiers ==========

	/**
	 * Get movement speed modifier based on survival meters
	 * Accounts for hunger, stamina, cold, wounds, etc.
	 */
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	float GetMovementSpeedModifier() const;

	/**
	 * Check if character can sprint
	 */
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	bool CanSprint() const;

	/**
	 * Start sprinting (consumes stamina)
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void StartSprint();

	/**
	 * Stop sprinting
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void StopSprint();

protected:
	// ========== State ==========

	UPROPERTY(BlueprintReadOnly, Category = "Character|State")
	EOTDPosture CurrentPosture = EOTDPosture::Standing;

	UPROPERTY(BlueprintReadOnly, Category = "Character|State")
	EOTDPosture TargetPosture = EOTDPosture::Standing;

	UPROPERTY(BlueprintReadOnly, Category = "Character|State")
	float PostureBlendAlpha = 1.0f; // 0.0 = current, 1.0 = target

	UPROPERTY(BlueprintReadOnly, Category = "Character|State")
	float PostureTransitionTime = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Camera")
	EOTDShoulderSide CurrentShoulder = EOTDShoulderSide::Right;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Camera")
	bool bIsAiming = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
	float CurrentLean = 0.0f; // -1.0 to 1.0

	UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
	bool bIsSprinting = false;

	// ========== Camera Settings ==========

	/**
	 * Base spring arm length (modified by posture and aiming)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float BaseArmLength = 150.0f;

	/**
	 * Shoulder offset distance from character center
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float ShoulderOffset = 40.0f;

	/**
	 * Camera lag for smooth following
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float CameraLagSpeed = 10.0f;

	/**
	 * Camera rotation lag for smooth rotation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float CameraRotationLagSpeed = 10.0f;

	/**
	 * ADS camera zoom multiplier
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float ADSArmLength = 80.0f;

	// ========== Posture Settings ==========

	/**
	 * Capsule half-height for each posture
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture|Settings")
	float StandingHeight = 88.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture|Settings")
	float CrouchingHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture|Settings")
	float ProneHeight = 30.0f;

	/**
	 * Movement speed multiplier for each posture
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture|Settings")
	float StandingSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture|Settings")
	float CrouchingSpeedMultiplier = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture|Settings")
	float ProneSpeedMultiplier = 0.3f;

	// ========== Stamina Costs ==========

	/**
	 * Stamina drain per second while sprinting
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Settings")
	float SprintStaminaDrain = 15.0f;

	/**
	 * Stamina drain per second while leaning
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Settings")
	float LeanStaminaDrain = 2.0f;

	/**
	 * Minimum stamina required to sprint
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Settings")
	float MinSprintStamina = 30.0f;

	/**
	 * Stamina below which character auto-crouches
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Settings")
	float AutoCrouchStamina = 30.0f;

	/**
	 * Stamina below which character can only crouch-walk
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Settings")
	float CrouchOnlyStamina = 10.0f;

	// ========== Internal Methods ==========

	/**
	 * Update posture blending and capsule size
	 */
	void UpdatePosture(float DeltaTime);

	/**
	 * Update camera offsets based on posture and shoulder
	 */
	void UpdateCameraOffsets(float DeltaTime);

	/**
	 * Update movement speed based on survival meters and posture
	 */
	void UpdateMovementSpeed();

	/**
	 * Apply leaning to skeleton (upper spine IK)
	 */
	void ApplyLeanToSkeleton(float DeltaTime);

	/**
	 * Calculate target capsule height for posture
	 */
	float GetCapsuleHeightForPosture(EOTDPosture Posture) const;

	/**
	 * Calculate camera socket offset for shoulder
	 */
	FVector GetShoulderSocketOffset() const;

	/**
	 * Get reference to camera manager
	 */
	class AOTDCameraManager* GetCameraManager() const;
};
