//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Vignettes/OTDVignetteEvent.h"
#include "OTDVignetteSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOTDVignetteEventDispatchSignature, const FGuid&, InstanceId, const FOTDVignetteEventDefinition&, Definition);

UCLASS()
class UOTDVignetteSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UOTDVignetteSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    UFUNCTION(BlueprintCallable, Category = "Vignette")
    void PushContext(const FOTDVignetteContext& Context);    

    UFUNCTION(BlueprintCallable, Category = "Vignette")
    void PopContext();

    UFUNCTION(BlueprintCallable, Category = "Vignette")
    void RegisterVignetteDefinition(const FOTDVignetteEventDefinition& Definition);

    UFUNCTION(BlueprintCallable, Category = "Vignette")
    void ClearDefinitions();

    UFUNCTION(BlueprintPure, Category = "Vignette")
    const TArray<FOTDActiveVignette>& GetActiveVignettes() const { return ActiveVignettes; }

    UFUNCTION(BlueprintPure, Category = "Vignette|Analytics")
    const TArray<FOTDVignetteAnalyticsRecord>& GetAnalyticsRecords() const { return AnalyticsRecords; }

    UFUNCTION(BlueprintCallable, Category = "Vignette|Analytics")
    void ResetAnalyticsRecords();

    UPROPERTY(BlueprintAssignable)
    FOTDVignetteEventDispatchSignature OnVignetteStarted;

    UPROPERTY(BlueprintAssignable)
    FOTDVignetteEventDispatchSignature OnVignetteCompleted;

protected:
    void UpdateActiveVignettes(float DeltaTime);
    void EvaluateVignetteSpawns(float DeltaTime);
    bool CanStartVignette(const FOTDVignetteEventDefinition& Definition) const;
    float ComputeVignetteWeight(const FOTDVignetteEventDefinition& Definition) const;
    void StartVignette(const FOTDVignetteEventDefinition& Definition);
    const FOTDVignetteContext* GetActiveContext() const;
    void ApplySuppressionPulse(const FVector& Origin, float Radius, float Intensity) const;
    void RecordAnalytics(const FOTDActiveVignette& Vignette, bool bCompleted);

protected:
    UPROPERTY(EditAnywhere, Category = "Vignette")
    float MinIntervalBetweenStarts = 10.f;

    UPROPERTY(EditAnywhere, Category = "Vignette")
    int32 MaxConcurrentVignettes = 2;

    UPROPERTY(EditAnywhere, Category = "Vignette")
    bool bDebugLogging = false;

private:
    TArray<FOTDVignetteContext> ContextStack;
    TArray<FOTDVignetteEventDefinition> Definitions;
    TArray<FOTDActiveVignette> ActiveVignettes;
    TMap<FGameplayTag, double> CooldownMap;
    TArray<FOTDVignetteAnalyticsRecord> AnalyticsRecords;
    double LastVignetteStartTime = -BIG_NUMBER;
};
//// END NEW CODE
