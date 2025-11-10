// Only The Dead - Camera Manager Implementation

#include "Camera/OTDCameraManager.h"
#include "Camera/OTDCameraDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"

AOTDCameraManager::AOTDCameraManager()
{
	// Set default camera parameters
	DefaultFOV = 75.0f;
	ViewPitchMin = -70.0f;
	ViewPitchMax = 70.0f;

	// Initialize camera state
	CurrentMode = EOTDCameraMode::SurvivalOTS;
	TargetMode = EOTDCameraMode::SurvivalOTS;
	BlendAlpha = 1.0f;
}

void AOTDCameraManager::BeginPlay()
{
	Super::BeginPlay();

	// Initialize current camera state
	CurrentCameraState = LoadCameraStateForMode(CurrentMode);
	TargetCameraState = CurrentCameraState;
}

void AOTDCameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);

	// Update camera mode blending
	UpdateCameraBlend(DeltaTime);

	// Apply breath sway if in ADS mode
	if (CurrentMode == EOTDCameraMode::PrecisionADS)
	{
		BreathSwayPhase += DeltaTime;
		const float BreathFrequency = 0.3f; // Breaths per second
		const float SwayAmount = FMath::Lerp(0.5f, 2.0f, 1.0f - CurrentSuppressionLevel);

		BreathSwayOffset.X = FMath::Sin(BreathSwayPhase * BreathFrequency * 2.0f * PI) * SwayAmount;
		BreathSwayOffset.Y = FMath::Cos(BreathSwayPhase * BreathFrequency * 2.0f * PI * 0.7f) * SwayAmount * 0.6f;

		// Apply sway to view rotation
		FRotator ViewRotation = GetCameraRotation();
		ViewRotation.Pitch += BreathSwayOffset.Y * 0.1f;
		ViewRotation.Yaw += BreathSwayOffset.X * 0.1f;
		SetViewRotation(ViewRotation);
	}

	// Decay suppression effect
	if (CurrentSuppressionLevel > 0.0f)
	{
		CurrentSuppressionLevel = FMath::FInterpTo(CurrentSuppressionLevel, 0.0f, DeltaTime, 2.0f);
	}
}

// ========== Mode Management ==========

void AOTDCameraManager::SetCameraMode(EOTDCameraMode NewMode, float BlendTimeOverride)
{
	// Check accessibility settings for set-piece locks
	if (NewMode == EOTDCameraMode::LockFramedSetPiece && !bAllowSetPieceLocks)
	{
		return; // Accessibility setting prevents locked cameras
	}

	if (NewMode == CurrentMode && BlendAlpha >= 1.0f)
	{
		return; // Already in this mode and not blending
	}

	// Set target mode
	TargetMode = NewMode;
	TargetCameraState = LoadCameraStateForMode(NewMode);

	// Determine blend time
	BlendDuration = (BlendTimeOverride > 0.0f) ? BlendTimeOverride : TargetCameraState.BlendTime;
	BlendAlpha = 0.0f;

	UE_LOG(LogCamera, Log, TEXT("OTDCameraManager: Transitioning from %d to %d over %.2f seconds"),
		static_cast<int32>(CurrentMode), static_cast<int32>(TargetMode), BlendDuration);
}

// ========== Shoulder Management ==========

void AOTDCameraManager::SwapShoulder()
{
	CurrentShoulder = (CurrentShoulder == EOTDShoulderSide::Right) ?
		EOTDShoulderSide::Left : EOTDShoulderSide::Right;

	UE_LOG(LogCamera, Verbose, TEXT("OTDCameraManager: Swapped to %s shoulder"),
		(CurrentShoulder == EOTDShoulderSide::Right) ? TEXT("Right") : TEXT("Left"));
}

void AOTDCameraManager::SetShoulderSide(EOTDShoulderSide NewSide)
{
	if (CurrentShoulder != NewSide)
	{
		CurrentShoulder = NewSide;
	}
}

// ========== Camera Modifiers ==========

void AOTDCameraManager::ApplySuppressionEffect(float SuppressionLevel)
{
	CurrentSuppressionLevel = FMath::Clamp(SuppressionLevel, 0.0f, 1.0f);

	// Suppression applies blur and grain post-process effects
	// This would be expanded to modify post-process settings in a full implementation
	UE_LOG(LogCamera, Verbose, TEXT("OTDCameraManager: Suppression level set to %.2f"), CurrentSuppressionLevel);
}

void AOTDCameraManager::ApplyBreathSway(float StaminaPercent)
{
	// Low stamina increases sway significantly
	const float SwayMultiplier = FMath::Lerp(3.0f, 1.0f, StaminaPercent);
	// Sway is applied in UpdateCamera tick
}

void AOTDCameraManager::ApplyArtilleryShake(const FVector& ImpactLocation, float ShellCaliberMM)
{
	if (!PCOwner)
	{
		return;
	}

	const float ShakeIntensity = CalculateShakeIntensity(ImpactLocation, ShellCaliberMM);

	if (ShakeIntensity > 0.01f)
	{
		// Apply camera shake
		// In full implementation, would use custom camera shake classes
		// For now, log the shake request
		UE_LOG(LogCamera, Log, TEXT("OTDCameraManager: Artillery shake intensity %.2f (%.0fmm at distance %.1fm)"),
			ShakeIntensity, ShellCaliberMM,
			FVector::Dist(ImpactLocation, GetCameraLocation()) / 100.0f); // Convert to meters

		// Also apply suppression
		ApplySuppressionEffect(FMath::Min(CurrentSuppressionLevel + ShakeIntensity * 0.3f, 1.0f));
	}
}

// ========== Internal Helpers ==========

void AOTDCameraManager::UpdateCameraBlend(float DeltaTime)
{
	if (BlendAlpha < 1.0f)
	{
		// Smooth blend using ease-in-out curve
		BlendAlpha += DeltaTime / BlendDuration;
		BlendAlpha = FMath::Clamp(BlendAlpha, 0.0f, 1.0f);

		// Use smoothstep for better feeling transitions
		const float SmoothAlpha = BlendAlpha * BlendAlpha * (3.0f - 2.0f * BlendAlpha);

		// Lerp between current and target states
		FOTDCameraState BlendedState;
		BlendedState.FOV = FMath::Lerp(CurrentCameraState.FOV, TargetCameraState.FOV, SmoothAlpha);
		BlendedState.CameraOffset = FMath::Lerp(CurrentCameraState.CameraOffset, TargetCameraState.CameraOffset, SmoothAlpha);
		BlendedState.CameraLag = FMath::Lerp(CurrentCameraState.CameraLag, TargetCameraState.CameraLag, SmoothAlpha);
		BlendedState.CameraRotationLag = FMath::Lerp(CurrentCameraState.CameraRotationLag, TargetCameraState.CameraRotationLag, SmoothAlpha);

		ApplyCameraState(BlendedState, SmoothAlpha);

		// Complete transition when done
		if (BlendAlpha >= 1.0f)
		{
			CurrentMode = TargetMode;
			CurrentCameraState = TargetCameraState;
			UE_LOG(LogCamera, Log, TEXT("OTDCameraManager: Completed transition to mode %d"), static_cast<int32>(CurrentMode));
		}
	}
}

void AOTDCameraManager::ApplyCameraState(const FOTDCameraState& State, float Alpha)
{
	// Apply FOV
	SetFOV(State.FOV);

	// Camera offset and lag would be applied to the SpringArm component
	// This requires communication with the pawn/character
	// In a full implementation, this would be done via an interface or direct component access
}

FOTDCameraState AOTDCameraManager::LoadCameraStateForMode(EOTDCameraMode Mode) const
{
	FOTDCameraState State;
	State.Mode = Mode;

	// Check if we have a data asset for this mode
	if (const UOTDCameraDataAsset* const* DataAssetPtr = CameraModeDataAssets.Find(Mode))
	{
		if (*DataAssetPtr != nullptr)
		{
			return (*DataAssetPtr)->CameraState;
		}
	}

	// Fallback to hardcoded defaults for each mode
	switch (Mode)
	{
	case EOTDCameraMode::GuidedWalk:
		State.FOV = 80.0f;
		State.CameraOffset = FVector(-200.0f, 50.0f, 70.0f);
		State.CameraLag = 3.0f;
		State.CameraRotationLag = 8.0f;
		State.BlendTime = 0.35f;
		break;

	case EOTDCameraMode::SurvivalOTS:
		State.FOV = 75.0f;
		State.CameraOffset = FVector(-150.0f, 60.0f, 60.0f);
		State.CameraLag = 1.5f;
		State.CameraRotationLag = 5.0f;
		State.BlendTime = 0.25f;
		break;

	case EOTDCameraMode::PrecisionADS:
		State.FOV = 60.0f;
		State.CameraOffset = FVector(-100.0f, 40.0f, 55.0f);
		State.CameraLag = 0.5f;
		State.CameraRotationLag = 2.0f;
		State.BlendTime = 0.2f;
		break;

	case EOTDCameraMode::LockFramedSetPiece:
		State.FOV = 85.0f;
		State.CameraOffset = FVector(-300.0f, 100.0f, 80.0f);
		State.CameraLag = 0.0f;
		State.CameraRotationLag = 0.0f;
		State.BlendTime = 0.3f;
		break;

	default:
		// Default to SurvivalOTS settings
		State.FOV = 75.0f;
		State.CameraOffset = FVector(-150.0f, 60.0f, 60.0f);
		State.CameraLag = 1.5f;
		State.CameraRotationLag = 5.0f;
		State.BlendTime = 0.25f;
		break;
	}

	// Apply accessibility multipliers
	State.CameraLag *= CameraLagMultiplier;
	State.CameraRotationLag *= CameraLagMultiplier;

	return State;
}

FVector AOTDCameraManager::GetShoulderOffset() const
{
	// Base shoulder offset for OTS camera
	const float ShoulderDistance = 40.0f; // 40cm from center
	const float Sign = (CurrentShoulder == EOTDShoulderSide::Right) ? 1.0f : -1.0f;

	return FVector(0.0f, ShoulderDistance * Sign, 0.0f);
}

float AOTDCameraManager::CalculateShakeIntensity(const FVector& ImpactLocation, float ShellCaliberMM) const
{
	const float DistanceCM = FVector::Dist(ImpactLocation, GetCameraLocation());
	const float DistanceM = DistanceCM / 100.0f;

	// Shell caliber determines base intensity and falloff
	// Based on historical data from artillery specs in the design doc
	float BaseIntensity = 0.0f;
	float EffectiveRadius = 0.0f;

	if (ShellCaliberMM <= 77.0f) // 75mm/77mm light shells
	{
		BaseIntensity = 1.0f;
		EffectiveRadius = 30.0f; // meters
	}
	else if (ShellCaliberMM <= 155.0f) // 155mm medium shells
	{
		BaseIntensity = 2.5f;
		EffectiveRadius = 60.0f;
	}
	else if (ShellCaliberMM <= 210.0f) // 210mm heavy shells
	{
		BaseIntensity = 4.0f;
		EffectiveRadius = 100.0f;
	}
	else // 305mm+ super-heavy
	{
		BaseIntensity = 6.0f;
		EffectiveRadius = 150.0f;
	}

	// Calculate falloff
	const float NormalizedDistance = FMath::Clamp(DistanceM / EffectiveRadius, 0.0f, 1.0f);
	const float Falloff = 1.0f - (NormalizedDistance * NormalizedDistance); // Quadratic falloff

	return BaseIntensity * Falloff;
}
