#include "Player/VerdunSoldier.h"
#include "Player/SurvivalComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

AVerdunSoldier::AVerdunSoldier()
{
    PrimaryActorTick.bCanEverTick = true;

    // First-person capsule — 180 cm tall (average WWI soldier height)
    GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

    // Third-person-style spring arm for POC (swap to first-person in production)
    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(RootComponent);
    CameraArm->TargetArmLength = 300.0f;
    CameraArm->bUsePawnControlRotation = true;
    CameraArm->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    // Post-process for shell shock visual effects
    ShellShockPP = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ShellShockPP"));
    ShellShockPP->SetupAttachment(RootComponent);
    ShellShockPP->bEnabled = true;
    ShellShockPP->BlendWeight = 0.0f;  // Off until shell shock

    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // Character movement tuning
    GetCharacterMovement()->MaxWalkSpeed        = 300.0f;  // 3 m/s — combat walk
    GetCharacterMovement()->MaxWalkSpeedCrouched = 150.0f; // Slow crawl in trench
    GetCharacterMovement()->JumpZVelocity        = 420.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);

    bUseControllerRotationYaw = false;
}

void AVerdunSoldier::BeginPlay()
{
    Super::BeginPlay();

    DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

    // Register input mapping context
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // Listen for shell shock transitions
    SurvivalComp->OnShellShockStateChanged.AddDynamic(this, &AVerdunSoldier::OnShellShockStateChanged);
}

void AVerdunSoldier::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smoothly interpolate post-process values for gradual shell shock onset
    CurrentVignetteIntensity = FMath::FInterpTo(CurrentVignetteIntensity, TargetVignetteIntensity, DeltaTime, 2.0f);
    CurrentDesaturation      = FMath::FInterpTo(CurrentDesaturation,      TargetDesaturation,      DeltaTime, 2.0f);

    // Apply to post-process settings
    FPostProcessSettings& PP = ShellShockPP->Settings;
    PP.bOverride_VignetteIntensity = true;
    PP.VignetteIntensity           = CurrentVignetteIntensity;
    PP.bOverride_ColorSaturation   = true;
    PP.ColorSaturation             = FVector4(1.0f - CurrentDesaturation * 0.8f, 1.0f, 1.0f, 1.0f);

    ShellShockPP->BlendWeight = FMath::Max(CurrentVignetteIntensity, CurrentDesaturation);
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void AVerdunSoldier::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVerdunSoldier::Move);
        }
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVerdunSoldier::Look);
        }
        if (CrouchAction)
        {
            EIC->BindAction(CrouchAction, ETriggerEvent::Started,    this, &AVerdunSoldier::StartCrouch);
            EIC->BindAction(CrouchAction, ETriggerEvent::Completed,  this, &AVerdunSoldier::EndCrouch);
        }
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
    }
}

void AVerdunSoldier::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    if (!Controller) return;

    const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
    const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDir, MovementVector.Y);
    AddMovementInput(RightDir,   MovementVector.X);
}

void AVerdunSoldier::Look(const FInputActionValue& Value)
{
    const FVector2D LookVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);
}

void AVerdunSoldier::StartCrouch(const FInputActionValue& Value)
{
    Crouch();
}

void AVerdunSoldier::EndCrouch(const FInputActionValue& Value)
{
    UnCrouch();
}

// ---------------------------------------------------------------------------
// Damage
// ---------------------------------------------------------------------------

float AVerdunSoldier::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
    const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Route physical damage to morale system as well (trauma from being hit)
    if (SurvivalComp && DamageAmount > 0.0f)
    {
        const bool bIsLethalRange = DamageAmount >= 100.0f;
        SurvivalComp->ApplyMoraleDamage(DamageAmount * 0.5f, bIsLethalRange);
    }

    return Actual;
}

// ---------------------------------------------------------------------------
// Shell shock response
// ---------------------------------------------------------------------------

void AVerdunSoldier::OnShellShockStateChanged(EShellShockState NewState)
{
    LastShockState = NewState;
    ApplyShellShockPostProcess(NewState);
    UpdateMovementSpeed();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        switch (NewState)
        {
        case EShellShockState::ShellShocked:
            if (ShellShockCameraShake)
            {
                PC->ClientStartCameraShake(ShellShockCameraShake, 0.5f);
            }
            if (ShellShockSoundMix)
            {
                UGameplayStatics::PushSoundMixModifier(this, ShellShockSoundMix);
            }
            break;

        case EShellShockState::Normal:
            if (ShellShockSoundMix)
            {
                UGameplayStatics::PopSoundMixModifier(this, ShellShockSoundMix);
            }
            PC->StopAllCameraShakes(false);
            break;

        default:
            break;
        }
    }
}

void AVerdunSoldier::ApplyShellShockPostProcess(EShellShockState State)
{
    switch (State)
    {
    case EShellShockState::Normal:
        TargetVignetteIntensity = 0.0f;
        TargetDesaturation      = 0.0f;
        break;

    case EShellShockState::ShellShocked:
        TargetVignetteIntensity = 0.6f;
        TargetDesaturation      = 0.4f;
        break;

    case EShellShockState::Dissociated:
        TargetVignetteIntensity = 0.9f;
        TargetDesaturation      = 0.85f;
        break;
    }
}

void AVerdunSoldier::UpdateMovementSpeed()
{
    switch (LastShockState)
    {
    case EShellShockState::Normal:
        GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
        break;

    case EShellShockState::ShellShocked:
        // -40% movement per design doc
        GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.60f;
        break;

    case EShellShockState::Dissociated:
        // -60% movement
        GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.40f;
        break;
    }
}
