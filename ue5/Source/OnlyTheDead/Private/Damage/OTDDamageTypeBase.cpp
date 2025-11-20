//// BEGIN NEW CODE
#include "Damage/OTDDamageTypeBase.h"

UOTDDamageTypeBase::UOTDDamageTypeBase()
{
    SeverityBonus = 1.0f;
    BleedMultiplier = 1.0f;
    SuppressionImpulse = 0.0f;
    FearImpulse = 0.0f;
    ShockImpulse = 0.0f;
}

UOTDDamageType_Bullet::UOTDDamageType_Bullet()
{
    SeverityBonus = 1.1f;
    BleedMultiplier = 1.2f;
    SuppressionImpulse = 25.0f;
    FearImpulse = 15.0f;
    ShockImpulse = 10.0f;
}

UOTDDamageType_Shell::UOTDDamageType_Shell()
{
    SeverityBonus = 1.4f;
    BleedMultiplier = 1.5f;
    SuppressionImpulse = 55.0f;
    FearImpulse = 45.0f;
    ShockImpulse = 60.0f;
}

UOTDDamageType_Gas::UOTDDamageType_Gas()
{
    SeverityBonus = 0.9f;
    BleedMultiplier = 0.6f;
    SuppressionImpulse = 35.0f;
    FearImpulse = 55.0f;
    ShockImpulse = 25.0f;
}

UOTDDamageType_Collapse::UOTDDamageType_Collapse()
{
    SeverityBonus = 1.2f;
    BleedMultiplier = 1.0f;
    SuppressionImpulse = 40.0f;
    FearImpulse = 35.0f;
    ShockImpulse = 45.0f;
}
//// END NEW CODE
