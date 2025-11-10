// Only The Dead - Player Character Implementation

#include "Character/OTDCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/OTDCameraManager.h"
#include "Survival/OTDSurvivalComponent.h"
#include "Cover/OTDCoverDetectionComponent.h"
#include "Kismet/GameplayStatics.h"

AOTDCharacter::AOTDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 350.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create spring arm for camera positioning
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = BaseArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = CameraLagSpeed;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = CameraRotationLagSpeed;
	CameraBoom->bDoCollisionTest = true; // Important for tight trenches

	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera follows spring arm rotation

	// Create survival component
	SurvivalComponent = CreateDefaultSubobject<UOTDSurvivalComponent>(TEXT("SurvivalComponent"));

	// Create cover detection component
	CoverDetection = CreateDefaultSubobject<UOTDCoverDetectionComponent>(TEXT("CoverDetection"));
}

void AOTDCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize posture
	CurrentPosture = EOTDPosture::Standing;
	TargetPosture = EOTDPosture::Standing;
	PostureBlendAlpha = 1.0f;

	// Set initial camera shoulder
	UpdateCameraOffsets(0.0f);
}

void AOTDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update systems
	UpdatePosture(DeltaTime);
	UpdateCameraOffsets(DeltaTime);
	UpdateMovementSpeed();
	ApplyLeanToSkeleton(DeltaTime);

	// Check for auto-crouch from low stamina
	if (SurvivalComponent)
	{
		const float CurrentStamina = SurvivalComponent->GetStamina();

		if (CurrentStamina < CrouchOnlyStamina && CurrentPosture != EOTDPosture::Prone)
		{
			// Force crouch when stamina is critically low
			SetPosture(EOTDPosture::Crouching);
		}
		else if (CurrentStamina < AutoCrouchStamina && bIsSprinting)
		{
			// Stop sprinting when stamina is low
			StopSprint();
		}
	}
}

void AOTDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input binding will be done with Enhanced Input system
	// This is a placeholder for the base implementation
}

// ========== Posture System ==========

void AOTDCharacter::SetPosture(EOTDPosture NewPosture)
{
	if (!CanTransitionToPosture(NewPosture))
	{
		return;
	}

	if (NewPosture != TargetPosture)
	{
		TargetPosture = NewPosture;
		PostureBlendAlpha = 0.0f;

		// Determine transition time based on posture change
		if ((CurrentPosture == EOTDPosture::Standing && NewPosture == EOTDPosture::Prone) ||
			(CurrentPosture == EOTDPosture::Prone && NewPosture == EOTDPosture::Standing))
		{
			PostureTransitionTime = 1.0f; // Slower for standing <-> prone
		}
		else
		{
			PostureTransitionTime = 0.5f; // Faster for other transitions
		}

		UE_LOG(LogTemp, Log, TEXT("OTDCharacter: Transitioning from %d to %d posture"),
			static_cast<int32>(CurrentPosture), static_cast<int32>(NewPosture));
	}
}

void AOTDCharacter::ForcePosture(EOTDPosture NewPosture, float TransitionTime)
{
	TargetPosture = NewPosture;
	PostureBlendAlpha = 0.0f;
	PostureTransitionTime = TransitionTime;

	UE_LOG(LogTemp, Log, TEXT("OTDCharacter: Force posture to %d (transition %.2fs)"),
		static_cast<int32>(NewPosture), TransitionTime);
}

bool AOTDCharacter::CanTransitionToPosture(EOTDPosture TargetPosture) const
{
	// Can't stand if stamina is too low
	if (TargetPosture == EOTDPosture::Standing && SurvivalComponent)
	{
		if (SurvivalComponent->GetStamina() < AutoCrouchStamina)
		{
			return false;
		}
	}

	// Check for overhead clearance when trying to stand
	if (TargetPosture == EOTDPosture::Standing && CurrentPosture != EOTDPosture::Standing)
	{
		// Trace upward to check for overhead obstacles
		FVector Start = GetActorLocation();
		FVector End = Start + FVector(0.0f, 0.0f, StandingHeight);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
		{
			return false; // Something overhead, can't stand
		}
	}

	return true;
}

// ========== Camera Control ==========

void AOTDCharacter::SwapShoulder()
{
	CurrentShoulder = (CurrentShoulder == EOTDShoulderSide::Right) ?
		EOTDShoulderSide::Left : EOTDShoulderSide::Right;

	// Notify camera manager
	if (AOTDCameraManager* CameraManager = GetCameraManager())
	{
		CameraManager->SetShoulderSide(CurrentShoulder);
	}

	UE_LOG(LogTemp, Verbose, TEXT("OTDCharacter: Swapped to %s shoulder"),
		(CurrentShoulder == EOTDShoulderSide::Right) ? TEXT("Right") : TEXT("Left"));
}

void AOTDCharacter::SetAiming(bool bNewAiming)
{
	if (bIsAiming == bNewAiming)
	{
		return;
	}

	bIsAiming = bNewAiming;

	// Switch camera mode
	if (AOTDCameraManager* CameraManager = GetCameraManager())
	{
		CameraManager->SetCameraMode(
			bIsAiming ? EOTDCameraMode::PrecisionADS : EOTDCameraMode::SurvivalOTS,
			0.2f // Fast blend for responsive aiming
		);
	}

	// Adjust spring arm length for ADS
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = bIsAiming ? ADSArmLength : BaseArmLength;
	}

	UE_LOG(LogTemp, Verbose, TEXT("OTDCharacter: Aiming %s"),
		bIsAiming ? TEXT("enabled") : TEXT("disabled"));
}

// ========== Leaning ==========

void AOTDCharacter::SetLeaning(float LeanAmount)
{
	CurrentLean = FMath::Clamp(LeanAmount, -1.0f, 1.0f);

	// Drain stamina while leaning
	if (FMath::Abs(CurrentLean) > 0.1f && SurvivalComponent)
	{
		SurvivalComponent->DrainStamina(LeanStaminaDrain * GetWorld()->GetDeltaSeconds());
	}
}

// ========== Movement ==========

float AOTDCharacter::GetMovementSpeedModifier() const
{
	if (!SurvivalComponent)
	{
		return 1.0f;
	}

	float Modifier = 1.0f;

	// Hunger modifier (from design doc)
	const float Hunger = SurvivalComponent->GetHunger();
	if (Hunger < 40.0f)
	{
		Modifier *= FMath::Lerp(0.7f, 1.0f, Hunger / 40.0f); // -5% to -30%
	}

	// Stamina modifier
	const float Stamina = SurvivalComponent->GetStamina();
	if (Stamina < 30.0f)
	{
		Modifier *= 0.8f; // -20% when stamina < 30
	}

	// Warmth/cold modifier (from design doc)
	const float Warmth = SurvivalComponent->GetWarmth();
	if (Warmth < 50.0f)
	{
		Modifier *= FMath::Lerp(0.7f, 1.0f, Warmth / 50.0f); // -10% to -30%
	}

	// Posture modifier
	switch (CurrentPosture)
	{
	case EOTDPosture::Standing:
		Modifier *= StandingSpeedMultiplier;
		break;
	case EOTDPosture::Crouching:
		Modifier *= CrouchingSpeedMultiplier;
		break;
	case EOTDPosture::Prone:
		Modifier *= ProneSpeedMultiplier;
		break;
	}

	return Modifier;
}

bool AOTDCharacter::CanSprint() const
{
	if (!SurvivalComponent)
	{
		return false;
	}

	// Can't sprint if stamina too low or not standing
	return SurvivalComponent->GetStamina() >= MinSprintStamina &&
		CurrentPosture == EOTDPosture::Standing;
}

void AOTDCharacter::StartSprint()
{
	if (CanSprint())
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f; // Sprint speed
	}
}

void AOTDCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = 350.0f; // Normal speed
}

// ========== Internal Methods ==========

void AOTDCharacter::UpdatePosture(float DeltaTime)
{
	if (PostureBlendAlpha < 1.0f)
	{
		// Blend toward target posture
		PostureBlendAlpha += DeltaTime / PostureTransitionTime;
		PostureBlendAlpha = FMath::Clamp(PostureBlendAlpha, 0.0f, 1.0f);

		// Interpolate capsule height
		const float CurrentHeight = GetCapsuleHeightForPosture(CurrentPosture);
		const float TargetHeight = GetCapsuleHeightForPosture(TargetPosture);
		const float BlendedHeight = FMath::Lerp(CurrentHeight, TargetHeight, PostureBlendAlpha);

		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			Capsule->SetCapsuleHalfHeight(BlendedHeight);
		}

		// Complete transition
		if (PostureBlendAlpha >= 1.0f)
		{
			CurrentPosture = TargetPosture;
			UE_LOG(LogTemp, Log, TEXT("OTDCharacter: Completed posture transition to %d"),
				static_cast<int32>(CurrentPosture));
		}
	}

	// Drain stamina if sprinting
	if (bIsSprinting && SurvivalComponent)
	{
		SurvivalComponent->DrainStamina(SprintStaminaDrain * DeltaTime);
	}
}

void AOTDCharacter::UpdateCameraOffsets(float DeltaTime)
{
	if (!CameraBoom)
	{
		return;
	}

	// Calculate shoulder socket offset
	const FVector TargetOffset = GetShoulderSocketOffset();

	// Smooth blend to target offset
	const FVector CurrentOffset = CameraBoom->SocketOffset;
	CameraBoom->SocketOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, 10.0f);

	// Adjust camera lag based on movement state
	if (bIsAiming)
	{
		CameraBoom->CameraLagSpeed = 5.0f; // Slower lag for stable aiming
	}
	else if (bIsSprinting)
	{
		CameraBoom->CameraLagSpeed = 15.0f; // Faster lag for dynamic movement
	}
	else
	{
		CameraBoom->CameraLagSpeed = CameraLagSpeed; // Default
	}
}

void AOTDCharacter::UpdateMovementSpeed()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	const float Modifier = GetMovementSpeedModifier();
	const float BaseSpeed = bIsSprinting ? 600.0f : 350.0f;
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * Modifier;
}

void AOTDCharacter::ApplyLeanToSkeleton(float DeltaTime)
{
	// Smooth lean transition
	// In full implementation, this would modify upper spine bone rotation via animation blueprint
	// For now, we apply a subtle camera offset to simulate lean

	if (CameraBoom && FMath::Abs(CurrentLean) > 0.01f)
	{
		// Lean adds a horizontal offset to camera
		const float LeanDistance = 30.0f; // 30cm lean distance
		FVector LeanOffset = FVector(0.0f, CurrentLean * LeanDistance, 0.0f);

		// This would be better handled in animation, but for quick implementation:
		// Apply to socket offset in addition to shoulder offset
		// (Full implementation would use animation blueprint and bone modification)
	}
}

float AOTDCharacter::GetCapsuleHeightForPosture(EOTDPosture Posture) const
{
	switch (Posture)
	{
	case EOTDPosture::Standing:
		return StandingHeight;
	case EOTDPosture::Crouching:
		return CrouchingHeight;
	case EOTDPosture::Prone:
		return ProneHeight;
	default:
		return StandingHeight;
	}
}

FVector AOTDCharacter::GetShoulderSocketOffset() const
{
	const float Sign = (CurrentShoulder == EOTDShoulderSide::Right) ? 1.0f : -1.0f;

	// Base shoulder offset
	FVector Offset(0.0f, Sign * ShoulderOffset, 60.0f); // 60cm up from capsule base

	// Adjust height based on posture
	switch (CurrentPosture)
	{
	case EOTDPosture::Standing:
		Offset.Z = 60.0f;
		break;
	case EOTDPosture::Crouching:
		Offset.Z = 40.0f;
		break;
	case EOTDPosture::Prone:
		Offset.Z = 20.0f;
		break;
	}

	return Offset;
}

AOTDCameraManager* AOTDCharacter::GetCameraManager() const
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		return Cast<AOTDCameraManager>(PC->PlayerCameraManager);
	}
	return nullptr;
}
