// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

#include "Characters/VerdunSoldierCharacter.h"
#include "Systems/SurvivalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AVerdunSoldierCharacter::AVerdunSoldierCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create survival component
	SurvivalComponent = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComponent"));

	// Set default values
	DefaultWalkSpeed = 120.0f; // Slower than typical UE character - realistic WWI soldier movement
	SprintSpeedMultiplier = 1.8f;
	SprintStaminaCostPerSecond = 15.0f;

	// Set character movement defaults
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Initialize state
	bIsInCover = false;
	bIsProne = false;
	bIsSprinting = false;
	MovementSpeedModifier = 1.0f;

	// Initialize stats
	DaysSurvived = 0;
	RotationsCompleted = 0;
	BarragesSurvived = 0;

	// Default identity
	CharacterName = TEXT("Unknown Soldier");
	Rank = TEXT("Soldat");
	Regiment = TEXT("Unknown Regiment");
}

void AVerdunSoldierCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Bind to survival events
	if (SurvivalComponent)
	{
		SurvivalComponent->OnMeterCritical.AddDynamic(this, &AVerdunSoldierCharacter::UpdateMovementSpeed);
		SurvivalComponent->OnMeterRecovered.AddDynamic(this, &AVerdunSoldierCharacter::UpdateMovementSpeed);
	}

	UpdateMovementSpeed();
}

void AVerdunSoldierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle sprinting stamina drain
	if (bIsSprinting)
	{
		HandleSprintingStamina(DeltaTime);
	}
}

void AVerdunSoldierCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input binding will be handled by Enhanced Input in Blueprints or separate input component
}

void AVerdunSoldierCharacter::ToggleProne()
{
	bIsProne = !bIsProne;

	if (bIsProne)
	{
		// Reduce capsule height
		GetCapsuleComponent()->SetCapsuleHalfHeight(40.0f);

		// Reduce movement speed significantly
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.3f;

		// Can't sprint while prone
		if (bIsSprinting)
		{
			SetSprinting(false);
		}

		UE_LOG(LogTemp, Log, TEXT("%s went prone"), *CharacterName);
	}
	else
	{
		// Restore capsule height
		GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);

		// Restore movement speed
		UpdateMovementSpeed();

		UE_LOG(LogTemp, Log, TEXT("%s stood up"), *CharacterName);
	}
}

void AVerdunSoldierCharacter::SetSprinting(bool bNewSprinting)
{
	// Can't sprint while prone
	if (bIsProne && bNewSprinting)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot sprint while prone"));
		return;
	}

	// Can't sprint without stamina
	if (bNewSprinting && !SurvivalComponent->HasStamina(10.0f))
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough stamina to sprint"));
		return;
	}

	bIsSprinting = bNewSprinting;
	UpdateMovementSpeed();
}

void AVerdunSoldierCharacter::EnterCover()
{
	bIsInCover = true;
	UE_LOG(LogTemp, Log, TEXT("%s entered cover"), *CharacterName);

	// Cover provides protection from artillery and small arms fire
	// This will be used by damage calculations
}

void AVerdunSoldierCharacter::LeaveCover()
{
	bIsInCover = false;
	UE_LOG(LogTemp, Log, TEXT("%s left cover"), *CharacterName);
}

float AVerdunSoldierCharacter::GetCurrentMovementSpeed() const
{
	return GetCharacterMovement()->MaxWalkSpeed;
}

void AVerdunSoldierCharacter::UpdateMovementSpeed()
{
	if (bIsProne)
	{
		// Prone movement is very slow
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.3f;
		return;
	}

	// Calculate modifiers from survival meters
	float Modifier = CalculateMovementSpeedModifier();

	// Apply sprint multiplier if sprinting
	float TargetSpeed = DefaultWalkSpeed * Modifier;
	if (bIsSprinting)
	{
		TargetSpeed *= SprintSpeedMultiplier;
	}

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
	MovementSpeedModifier = Modifier;
}

float AVerdunSoldierCharacter::CalculateMovementSpeedModifier() const
{
	if (!SurvivalComponent)
	{
		return 1.0f;
	}

	float Modifier = 1.0f;

	// Hunger effects on movement speed
	float HungerValue = SurvivalComponent->GetMeterValue(ESurvivalMeterType::Hunger);
	if (HungerValue <= 20.0f)
	{
		Modifier *= 0.70f; // -30% movement speed
	}
	else if (HungerValue <= 40.0f)
	{
		Modifier *= 0.85f; // -15% movement speed
	}
	else if (HungerValue <= 70.0f)
	{
		Modifier *= 0.95f; // -5% movement speed
	}

	// Thirst effects on movement speed
	float ThirstValue = SurvivalComponent->GetMeterValue(ESurvivalMeterType::Thirst);
	if (ThirstValue <= 20.0f)
	{
		Modifier *= 0.70f; // -30% movement speed
	}
	else if (ThirstValue <= 40.0f)
	{
		Modifier *= 0.85f; // -15% movement speed
	}

	// Warmth effects on movement speed
	float WarmthValue = SurvivalComponent->GetMeterValue(ESurvivalMeterType::Warmth);
	if (WarmthValue <= 40.0f)
	{
		Modifier *= 0.90f; // -10% movement speed from cold/shivering
	}

	// Stamina effects - can't move fast with low stamina
	float StaminaValue = SurvivalComponent->GetMeterValue(ESurvivalMeterType::Stamina);
	if (StaminaValue <= 10.0f)
	{
		Modifier *= 0.50f; // Can barely move
	}
	else if (StaminaValue <= 30.0f)
	{
		Modifier *= 0.80f; // -20% movement speed
	}

	return FMath::Clamp(Modifier, 0.1f, 1.0f);
}

void AVerdunSoldierCharacter::HandleSprintingStamina(float DeltaTime)
{
	if (!SurvivalComponent)
	{
		return;
	}

	// Drain stamina while sprinting
	float StaminaCost = SprintStaminaCostPerSecond * DeltaTime;

	if (!SurvivalComponent->ConsumeStamina(StaminaCost))
	{
		// Out of stamina - stop sprinting
		SetSprinting(false);
		UE_LOG(LogTemp, Warning, TEXT("%s exhausted - stopped sprinting"), *CharacterName);
	}
}
