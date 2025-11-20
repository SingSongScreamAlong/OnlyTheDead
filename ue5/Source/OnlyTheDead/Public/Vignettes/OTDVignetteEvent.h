//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OTDVignetteEvent.generated.h"

UENUM(BlueprintType)
enum class EOTDVignetteState : uint8
{
    Pending,
    Active,
    Completed,
    Failed,
};

USTRUCT(BlueprintType)
struct FOTDVignetteContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    FGameplayTagContainer Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    FVector FocusLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    float Radius = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    float Urgency = 0.5f;
};

USTRUCT(BlueprintType)
struct FOTDVignetteEventDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    FGameplayTag EventTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    float MinCooldown = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    float MaxCooldown = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    float BaseWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    FGameplayTagContainer RequiredContextTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    FGameplayTagContainer ExcludedContextTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
    float DurationSeconds = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette|Presentation")
    TObjectPtr<USoundBase> StartSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette|Presentation")
    TObjectPtr<USoundBase> CompletionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette|Effects")
    float SuppressionPulse = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette|Effects")
    float SuppressionRadius = 800.f;
};

USTRUCT(BlueprintType)
struct FOTDActiveVignette
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    FOTDVignetteEventDefinition Definition;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    FGuid InstanceId;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    EOTDVignetteState State = EOTDVignetteState::Pending;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    float TimeRemaining = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    double StartTimestamp = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    FVector Origin = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    float InfluenceRadius = 0.f;
};

USTRUCT(BlueprintType)
struct FOTDVignetteAnalyticsRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    FGuid InstanceId;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    FGameplayTag EventTag;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    double StartTimestamp = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    double EndTimestamp = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Vignette")
    bool bCompleted = true;
};
//// END NEW CODE
