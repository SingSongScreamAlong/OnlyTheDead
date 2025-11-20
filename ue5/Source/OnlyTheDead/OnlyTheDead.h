//// BEGIN NEW CODE
#pragma once

#include "CoreMinimal.h"

class FOnlyTheDeadModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
//// END NEW CODE
