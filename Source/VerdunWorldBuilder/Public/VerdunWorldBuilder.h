// Verdun World Builder Plugin
// Module header

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FVerdunWorldBuilderModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
