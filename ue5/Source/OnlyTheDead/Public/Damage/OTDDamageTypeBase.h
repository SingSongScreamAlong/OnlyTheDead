//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "OTDDamageTypeBase.generated.h"

UCLASS(Abstract, Blueprintable)
class UOTDDamageTypeBase : public UDamageType
{
    GENERATED_BODY()

public:
    UOTDDamageTypeBase();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    float SeverityBonus;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    float BleedMultiplier;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Psychological")
    float SuppressionImpulse;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Psychological")
    float FearImpulse;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Psychological")
    float ShockImpulse;
};

UCLASS()
class UOTDDamageType_Bullet : public UOTDDamageTypeBase
{
    GENERATED_BODY()

public:
    UOTDDamageType_Bullet();
};

UCLASS()
class UOTDDamageType_Shell : public UOTDDamageTypeBase
{
    GENERATED_BODY()

public:
    UOTDDamageType_Shell();
};

UCLASS()
class UOTDDamageType_Gas : public UOTDDamageTypeBase
{
    GENERATED_BODY()

public:
    UOTDDamageType_Gas();
};

UCLASS()
class UOTDDamageType_Collapse : public UOTDDamageTypeBase
{
    GENERATED_BODY()

public:
    UOTDDamageType_Collapse();
};
//// END NEW CODE
