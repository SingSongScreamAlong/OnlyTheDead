//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AOTDCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EOTDMovementState : uint8
{
    Idle = 0,
    Walk,
    Run,
    Crouch,
    Prone,
    Stumble,
    Slip,
    Wounded
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOTDMovementStateChangedSignature, EOTDMovementState, PreviousState, EOTDMovementState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOTDStaminaChangedSignature, float, PreviousValue, float, NewValue);

UCLASS(Blueprintable)
class AOTDCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AOTDCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;

    UFUNCTION(BlueprintPure, Category = "Movement|State")
    EOTDMovementState GetMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Movement|State")
    float GetCurrentStateDuration() const;

    UFUNCTION(BlueprintPure, Category = "Stamina")
    float GetStaminaPercent() const { return MaxStamina > 0.f ? CurrentStamina / MaxStamina : 0.f; }

    UFUNCTION(BlueprintPure, Category = "Stamina")
    float GetCurrentStamina() const { return CurrentStamina; }

    UFUNCTION(BlueprintPure, Category = "Stamina")
    bool IsFatigued() const { return bIsFatigued; }

    UFUNCTION(BlueprintCallable, Category = "Movement|State")
    void SetProne(bool bNewProne);

    UFUNCTION(BlueprintCallable, Category = "Movement|State")
    void SetWounded(bool bNewWounded);

    UFUNCTION(BlueprintCallable, Category = "Movement|State")
    void TriggerSlip(float RecoveryTime);

    UFUNCTION(BlueprintCallable, Category = "Movement|State")
    void TriggerStumble(float RecoveryTime);

    UPROPERTY(BlueprintAssignable, Category = "Movement|State")
    FOTDMovementStateChangedSignature OnMovementStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Stamina")
    FOTDStaminaChangedSignature OnStaminaChanged;

protected:
    virtual void BeginPlay() override;

    void EvaluateMovementState(float DeltaSeconds);
    void SetMovementState(EOTDMovementState NewState, bool bForce = false);

    void UpdateTransientStates(float DeltaSeconds);
    void UpdateStamina(float DeltaSeconds);
    void UpdateFatigueState();
    void ApplyStaminaDelta(float Delta);
    void ConsiderSlipAndStumble(float DeltaSeconds);
    void RefreshMovementSpeed();
    void InitializeInputContext();
    void SetSprintRequested(bool bShouldSprint);

    void HandleMoveInput(const FInputActionValue& Value);
    void HandleLookInput(const FInputActionValue& Value);
    void HandleSprintStart(const FInputActionValue& Value);
    void HandleSprintStop(const FInputActionValue& Value);
    void HandleCrouchToggle(const FInputActionValue& Value);
    void HandleProneToggle(const FInputActionValue& Value);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|State", meta = (ClampMin = "50.0"))
    float WalkSpeedThreshold = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|State", meta = (ClampMin = "100.0"))
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|State")
    float SlipRecoveryDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|State")
    float StumbleRecoveryDuration = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
    float MaxStamina = 100.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Stamina")
    float CurrentStamina = MaxStamina;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
    float StaminaRunDrainRate = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
    float StaminaRecoveryIdleRate = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
    float FatigueThresholdPercent = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
    float FatiguedSpeedMultiplier = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slip|Stumble")
    float SlipChanceRunPerSecond = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slip|Stumble")
    float StumbleChanceRunPerSecond = 0.015f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slip|Stumble")
    float FatigueSlipChanceMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slip|Stumble")
    float SlipStaminaPenalty = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slip|Stumble")
    float StumbleStaminaPenalty = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
    float SprintSpeedMultiplier = 1.35f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
    float SprintStaminaDrainMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slip|Stumble")
    float SprintSlipChanceMultiplier = 1.35f;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> RunnerMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    int32 RunnerMappingPriority = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> CrouchAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ProneAction;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement|State")
    EOTDMovementState CurrentMovementState = EOTDMovementState::Idle;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement|State")
    double MovementStateStartTime = 0.0;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement|State")
    bool bIsProne = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement|State")
    bool bIsWounded = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement|State")
    bool bIsSlipping = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement|State")
    bool bIsStumbling = false;

    double SlipStateRemaining = 0.0;
    double StumbleStateRemaining = 0.0;

    UPROPERTY(VisibleInstanceOnly, Category = "Stamina")
    bool bIsFatigued = false;

    float CachedNormalMaxWalkSpeed = 0.0f;
    bool bSprintRequested = false;
};
//// END NEW CODE
