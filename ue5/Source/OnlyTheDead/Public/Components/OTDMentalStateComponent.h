//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OTDMentalStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOTDMoraleValueChangedSignature, float, PreviousValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOTDPanicTriggeredSignature);

UCLASS(ClassGroup = (OnlyTheDead), meta = (BlueprintSpawnableComponent))
class UOTDMentalStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOTDMentalStateComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "MentalState")
    void ApplySuppression(float Amount, AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "MentalState")
    void ApplyFear(float Amount, AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "MentalState")
    void ApplyShock(float Amount);

    UFUNCTION(BlueprintCallable, Category = "MentalState")
    void RegisterSuppressionImpulse(float Amount);

    UFUNCTION(BlueprintCallable, Category = "MentalState")
    void RegisterNearMiss(float Distance, float Intensity);

    UFUNCTION(BlueprintPure, Category = "MentalState")
    float GetSuppression() const { return Suppression; }

    UFUNCTION(BlueprintPure, Category = "MentalState")
    float GetFear() const { return Fear; }

    UFUNCTION(BlueprintPure, Category = "MentalState")
    float GetPanic() const { return Panic; }

    UFUNCTION(BlueprintPure, Category = "MentalState")
    bool IsPanicked() const { return Panic >= PanicThreshold; }

    UPROPERTY(BlueprintAssignable)
    FOTDMoraleValueChangedSignature OnSuppressionChanged;

    UPROPERTY(BlueprintAssignable)
    FOTDMoraleValueChangedSignature OnFearChanged;

    UPROPERTY(BlueprintAssignable)
    FOTDMoraleValueChangedSignature OnPanicChanged;

    UPROPERTY(BlueprintAssignable)
    FOTDPanicTriggeredSignature OnPanicTriggered;

protected:
    void UpdateSuppression(float DeltaTime);
    void UpdateFear(float DeltaTime);
    void UpdatePanic(float DeltaTime);
    void BroadcastValueChange(float PreviousValue, float NewValue, FOTDMoraleValueChangedSignature& DelegateRef);
    float ComputeSuppressionFromNearMiss(float Distance, float Intensity) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState", meta = (ClampMin = "0.0"))
    float MaxSuppression = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState", meta = (ClampMin = "0.0"))
    float MaxFear = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState", meta = (ClampMin = "0.0"))
    float MaxPanic = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float PanicThreshold = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float SuppressionRecoveryRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float FearRecoveryRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float PanicDecayRate = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float PanicFromSuppressionMultiplier = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float PanicFromFearMultiplier = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float ShockToPanicMultiplier = 1.5f;

    UPROPERTY(VisibleInstanceOnly, Category = "MentalState")
    float Suppression = 0.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "MentalState")
    float Fear = 0.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "MentalState")
    float Panic = 0.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "MentalState")
    bool bHasTriggeredPanic = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float NearMissSuppressionRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MentalState")
    float NearMissMaxImpulse = 25.0f;
};
//// END NEW CODE
