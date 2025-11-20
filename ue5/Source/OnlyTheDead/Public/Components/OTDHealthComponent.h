//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OTDHealthComponent.generated.h"

class UDamageType;
class UOTDDamageTypeBase;
class UOTDMentalStateComponent;

UENUM(BlueprintType)
enum class EOTDWoundSeverity : uint8
{
    None = 0,
    Light,
    Moderate,
    Severe,
    Critical,
    Fatal
};

USTRUCT(BlueprintType)
struct FOTDWoundInstigator
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Wound")
    TWeakObjectPtr<AActor> InstigatorActor;

    UPROPERTY(BlueprintReadOnly, Category = "Wound")
    TSubclassOf<UDamageType> DamageType;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOTDHealthChangedSignature, float, PreviousValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOTDWoundSeverityChangedSignature, EOTDWoundSeverity, PreviousTier, EOTDWoundSeverity, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOTDBleedingStateChangedSignature, bool, bIsBleeding);

UCLASS(ClassGroup = (OnlyTheDead), meta = (BlueprintSpawnableComponent))
class UOTDHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOTDHealthComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamage(float DamageAmount, TSubclassOf<UDamageType> DamageTypeClass, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsBleeding() const { return bIsBleeding; }

    UFUNCTION(BlueprintPure, Category = "Health")
    EOTDWoundSeverity GetWoundSeverity() const { return CurrentWoundSeverity; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    void StopBleeding();

    UPROPERTY(BlueprintAssignable)
    FOTDHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable)
    FOTDWoundSeverityChangedSignature OnWoundSeverityChanged;

    UPROPERTY(BlueprintAssignable)
    FOTDBleedingStateChangedSignature OnBleedingStateChanged;

protected:
    void ApplyBleeding(float DeltaTime);
    void UpdateWoundSeverity(float DamageApplied, const UOTDDamageTypeBase* DamageTypeData, AActor* InstigatorActor);
    void SetBleeding(bool bShouldBleed, float BleedRateOverride = -1.f);
    void ProcessDamageTypeEffects(const UOTDDamageTypeBase* DamageTypeData, AActor* DamageInstigator);
    void CacheMentalStateComponent();

    float ComputeBleedRateForSeverity(EOTDWoundSeverity Severity) const;
    float ClampHealth(float Value) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
    float CurrentHealth = MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float BaseBleedRate = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float ModerateBleedMultiplier = 1.75f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float SevereBleedMultiplier = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float CriticalBleedMultiplier = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float FatalHealthThreshold = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float LightWoundThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float ModerateWoundThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float SevereWoundThreshold = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Bleeding")
    float CriticalWoundThreshold = 65.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
    bool bIsBleeding = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
    float ActiveBleedRate = 0.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
    EOTDWoundSeverity CurrentWoundSeverity = EOTDWoundSeverity::None;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
    FOTDWoundInstigator LastWoundInstigator;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
    float BleedMultiplierFromDamage = 1.0f;

    TWeakObjectPtr<UOTDMentalStateComponent> MentalStateComponent;
};
//// END NEW CODE
