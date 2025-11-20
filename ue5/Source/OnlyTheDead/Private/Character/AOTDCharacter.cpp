//// BEGIN NEW CODE
#include "Character/AOTDCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "OTDLogging.h"

namespace
{
    double GetWorldSecondsSafe(const AActor* Actor)
    {
        return Actor && Actor->GetWorld() ? Actor->GetWorld()->GetTimeSeconds() : 0.0;
    }
}

AOTDCharacter::AOTDCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AOTDCharacter::BeginPlay()
{
    Super::BeginPlay();

    MovementStateStartTime = GetWorldSecondsSafe(this);
    CurrentMovementState = EOTDMovementState::Idle;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        CachedNormalMaxWalkSpeed = MoveComp->MaxWalkSpeed;
    }

    RefreshMovementSpeed();
}

void AOTDCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateTransientStates(DeltaSeconds);
    UpdateStamina(DeltaSeconds);
    EvaluateMovementState(DeltaSeconds);
    ConsiderSlipAndStumble(DeltaSeconds);
}

void AOTDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOTDCharacter::HandleMoveInput);
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AOTDCharacter::HandleMoveInput);
        }

        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOTDCharacter::HandleLookInput);
        }

        if (SprintAction)
        {
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AOTDCharacter::HandleSprintStart);
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AOTDCharacter::HandleSprintStop);
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AOTDCharacter::HandleSprintStop);
        }

        if (CrouchAction)
        {
            EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AOTDCharacter::HandleCrouchToggle);
        }

        if (ProneAction)
        {
            EnhancedInput->BindAction(ProneAction, ETriggerEvent::Triggered, this, &AOTDCharacter::HandleProneToggle);
        }
    }
}

void AOTDCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeInputContext();
}

float AOTDCharacter::GetCurrentStateDuration() const
{
    const double CurrentTime = GetWorldSecondsSafe(this);
    return static_cast<float>(CurrentTime - MovementStateStartTime);
}

void AOTDCharacter::SetProne(bool bNewProne)
{
    if (bIsProne == bNewProne)
    {
        return;
    }

    bIsProne = bNewProne;
    if (bIsProne)
    {
        SetMovementState(EOTDMovementState::Prone, true);
    }
    else
    {
        SetMovementState(EOTDMovementState::Idle, true);
    }

    RefreshMovementSpeed();
}

void AOTDCharacter::SetWounded(bool bNewWounded)
{
    if (bIsWounded == bNewWounded)
    {
        return;
    }

    bIsWounded = bNewWounded;
    if (bIsWounded)
    {
        SetMovementState(EOTDMovementState::Wounded, true);
    }

    RefreshMovementSpeed();
}

void AOTDCharacter::TriggerSlip(float RecoveryTime)
{
    bIsSlipping = true;
    SlipStateRemaining = FMath::Max(RecoveryTime, 0.1f);
    SetMovementState(EOTDMovementState::Slip, true);
    RefreshMovementSpeed();
}

void AOTDCharacter::TriggerStumble(float RecoveryTime)
{
    bIsStumbling = true;
    StumbleStateRemaining = FMath::Max(RecoveryTime, 0.1f);
    SetMovementState(EOTDMovementState::Stumble, true);
    RefreshMovementSpeed();
}

void AOTDCharacter::EvaluateMovementState(float DeltaSeconds)
{
    if (bIsSlipping || bIsStumbling)
    {
        return;
    }

    if (bIsWounded)
    {
        SetMovementState(EOTDMovementState::Wounded);
        return;
    }

    if (bIsProne)
    {
        SetMovementState(EOTDMovementState::Prone);
        return;
    }

    const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    const float Speed = MoveComp ? MoveComp->Velocity.Size2D() : 0.0f;

    if (MoveComp && MoveComp->IsCrouching())
    {
        SetMovementState(EOTDMovementState::Crouch);
    }
    else if (Speed >= RunSpeedThreshold)
    {
        SetMovementState(EOTDMovementState::Run);
    }
    else if (Speed >= WalkSpeedThreshold)
    {
        SetMovementState(EOTDMovementState::Walk);
    }
    else
    {
        SetMovementState(EOTDMovementState::Idle);
    }
}

void AOTDCharacter::SetMovementState(EOTDMovementState NewState, bool bForce /*= false*/)
{
    if (!bForce && CurrentMovementState == NewState)
    {
        return;
    }

    const EOTDMovementState PreviousState = CurrentMovementState;
    CurrentMovementState = NewState;
    MovementStateStartTime = GetWorldSecondsSafe(this);

    OTD_LOG_INFO("Movement state changed %d -> %d", static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    OnMovementStateChanged.Broadcast(PreviousState, NewState);

    RefreshMovementSpeed();
}

void AOTDCharacter::UpdateTransientStates(float DeltaSeconds)
{
    if (bIsSlipping)
    {
        SlipStateRemaining -= DeltaSeconds;
        if (SlipStateRemaining <= 0.0)
        {
            bIsSlipping = false;
            SlipStateRemaining = 0.0;
            SetMovementState(EOTDMovementState::Idle, true);
        }
    }

    if (bIsStumbling)
    {
        StumbleStateRemaining -= DeltaSeconds;
        if (StumbleStateRemaining <= 0.0)
        {
            bIsStumbling = false;
            StumbleStateRemaining = 0.0;
            SetMovementState(EOTDMovementState::Idle, true);
        }
    }
}

void AOTDCharacter::UpdateStamina(float DeltaSeconds)
{
    if (MaxStamina <= 0.f)
    {
        return;
    }

    float Delta = 0.f;
    switch (CurrentMovementState)
    {
    case EOTDMovementState::Run:
        Delta = -StaminaRunDrainRate * (bSprintRequested ? SprintStaminaDrainMultiplier : 1.0f);
        break;
    case EOTDMovementState::Walk:
    case EOTDMovementState::Crouch:
        Delta = StaminaRecoveryIdleRate * 0.4f;
        break;
    case EOTDMovementState::Prone:
    case EOTDMovementState::Idle:
        Delta = StaminaRecoveryIdleRate;
        break;
    default:
        Delta = 0.f;
        break;
    }

    if (bIsSlipping || bIsStumbling || bIsWounded)
    {
        Delta *= 0.25f;
    }

    ApplyStaminaDelta(Delta * DeltaSeconds);
    UpdateFatigueState();
}

void AOTDCharacter::ApplyStaminaDelta(float Delta)
{
    const float Previous = CurrentStamina;
    CurrentStamina = FMath::Clamp(CurrentStamina + Delta, 0.f, MaxStamina);

    if (!FMath::IsNearlyEqual(Previous, CurrentStamina))
    {
        OnStaminaChanged.Broadcast(Previous, CurrentStamina);
    }
}

void AOTDCharacter::UpdateFatigueState()
{
    const float ThresholdValue = MaxStamina * FatigueThresholdPercent;
    const bool bNewFatigued = CurrentStamina <= ThresholdValue;
    if (bNewFatigued == bIsFatigued)
    {
        return;
    }

    bIsFatigued = bNewFatigued;
    RefreshMovementSpeed();
}

void AOTDCharacter::ConsiderSlipAndStumble(float DeltaSeconds)
{
    if (CurrentMovementState != EOTDMovementState::Run || bIsSlipping || bIsStumbling)
    {
        return;
    }

    const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp || !MoveComp->IsMovingOnGround())
    {
        return;
    }

    const float FatigueMultiplier = bIsFatigued ? FatigueSlipChanceMultiplier : 1.0f;
    const float SprintMultiplier = bSprintRequested ? SprintSlipChanceMultiplier : 1.0f;
    const float SlipChance = FMath::Clamp(SlipChanceRunPerSecond * FatigueMultiplier * SprintMultiplier * DeltaSeconds, 0.f, 1.f);
    const float StumbleChance = FMath::Clamp(StumbleChanceRunPerSecond * FatigueMultiplier * SprintMultiplier * DeltaSeconds, 0.f, 1.f);

    if (FMath::FRand() < SlipChance)
    {
        TriggerSlip(SlipRecoveryDuration);
        ApplyStaminaDelta(-SlipStaminaPenalty);
        return;
    }

    if (FMath::FRand() < StumbleChance)
    {
        TriggerStumble(StumbleRecoveryDuration);
        ApplyStaminaDelta(-StumbleStaminaPenalty);
    }
}

void AOTDCharacter::RefreshMovementSpeed()
{
    if (CachedNormalMaxWalkSpeed <= 0.0f)
    {
        if (const UCharacterMovementComponent* MoveCompRead = GetCharacterMovement())
        {
            CachedNormalMaxWalkSpeed = MoveCompRead->MaxWalkSpeed;
        }
    }

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    float TargetSpeed = CachedNormalMaxWalkSpeed;
    if (bSprintRequested && !bIsFatigued && !bIsWounded && !bIsProne)
    {
        TargetSpeed *= SprintSpeedMultiplier;
    }

    if (bIsFatigued)
    {
        TargetSpeed *= FatiguedSpeedMultiplier;
    }

    if (bIsWounded)
    {
        TargetSpeed *= 0.6f;
    }

    if (bIsSlipping || bIsStumbling)
    {
        TargetSpeed *= 0.1f;
    }

    MoveComp->MaxWalkSpeed = FMath::Max(TargetSpeed, 50.0f);
}

void AOTDCharacter::InitializeInputContext()
{
    if (!RunnerMappingContext)
    {
        return;
    }

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                Subsystem->AddMappingContext(RunnerMappingContext, RunnerMappingPriority);
            }
        }
    }
}

void AOTDCharacter::SetSprintRequested(bool bShouldSprint)
{
    if (bSprintRequested == bShouldSprint)
    {
        return;
    }

    bSprintRequested = bShouldSprint;
    RefreshMovementSpeed();
}

void AOTDCharacter::HandleMoveInput(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    if (!Controller || MovementVector.IsNearlyZero())
    {
        return;
    }

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
}

void AOTDCharacter::HandleLookInput(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

void AOTDCharacter::HandleSprintStart(const FInputActionValue& Value)
{
    const bool bPressed = Value.Get<bool>();
    if (bPressed)
    {
        SetSprintRequested(true);
    }
}

void AOTDCharacter::HandleSprintStop(const FInputActionValue& Value)
{
    const bool bPressed = Value.Get<bool>();
    if (!bPressed)
    {
        SetSprintRequested(false);
    }
}

void AOTDCharacter::HandleCrouchToggle(const FInputActionValue& Value)
{
    if (!Value.Get<bool>())
    {
        return;
    }

    if (GetCharacterMovement()->IsCrouching())
    {
        UnCrouch();
    }
    else
    {
        Crouch();
    }
}

void AOTDCharacter::HandleProneToggle(const FInputActionValue& Value)
{
    if (!Value.Get<bool>())
    {
        return;
    }

    SetProne(!bIsProne);
}
//// END NEW CODE
