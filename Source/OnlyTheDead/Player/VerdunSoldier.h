#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player/SurvivalComponent.h"
#include "InputActionValue.h"
#include "VerdunSoldier.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UPostProcessComponent;

/**
 * AVerdunSoldier
 *
 * The player character. A French soldier at Verdun, 1916.
 *
 * Controls (Enhanced Input):
 *   WASD / Left stick  — move
 *   Mouse / Right stick — look
 *   Ctrl / B           — crouch (crouch in trench for cover)
 *   Space              — jump (rarely useful in a trench, but present)
 *
 * Shell shock integration:
 *   When SurvivalComponent.IsShellShocked():
 *     - Movement speed reduced by 40%
 *     - Camera shakes continuously
 *     - Post-process: desaturation + vignette intensifies
 *     - Audio: muffled (Low-pass filter applied via Sound Mix)
 *
 * When Dissociated:
 *     - Movement reduced 60%
 *     - Heavy vignette, slow-motion feel
 *     - Hands visible but trembling (animation state)
 *
 * Input mapping context and actions are assigned in a Blueprint child class
 * (BP_VerdunSoldier) to keep assets editor-bound.
 */
UCLASS()
class ONLYTHEDEAD_API AVerdunSoldier : public ACharacter
{
    GENERATED_BODY()

public:
    AVerdunSoldier();

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                              AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintPure, Category = "Soldier")
    USurvivalComponent* GetSurvivalComponent() const { return SurvivalComp; }

    // ---- Camera ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* CameraArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* Camera;

    // Post-process for shell shock vignette, desaturation, blur
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPostProcessComponent* ShellShockPP;

    // ---- Input assets (assign in Blueprint) ----

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    // ---- Camera shake ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Shock")
    TSubclassOf<UCameraShakeBase> NearMissCameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Shock")
    TSubclassOf<UCameraShakeBase> ShellShockCameraShake;

    // ---- Sound ----

    // Applied as a Sound Mix when shell shocked (muffles audio)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Shock")
    USoundMix* ShellShockSoundMix;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell Shock")
    USoundBase* HeartbeatSound;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USurvivalComponent* SurvivalComp;

private:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartCrouch(const FInputActionValue& Value);
    void EndCrouch(const FInputActionValue& Value);

    UFUNCTION()
    void OnShellShockStateChanged(EShellShockState NewState);

    void ApplyShellShockPostProcess(EShellShockState State);
    void UpdateMovementSpeed();

    EShellShockState LastShockState = EShellShockState::Normal;

    // Post-process blend weights — interpolated each tick
    float TargetVignetteIntensity  = 0.0f;
    float CurrentVignetteIntensity = 0.0f;

    float TargetDesaturation  = 0.0f;
    float CurrentDesaturation = 0.0f;

    // Normal walk speed (stored so we can restore it after shell shock)
    float DefaultWalkSpeed = 0.0f;
};
