// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "VerdunAIAgentStyle.h"

class FVerdunAIAgentCommands : public TCommands<FVerdunAIAgentCommands>
{
public:
	FVerdunAIAgentCommands()
		: TCommands<FVerdunAIAgentCommands>(TEXT("VerdunAIAgent"), NSLOCTEXT("Contexts", "VerdunAIAgent", "Verdun AI Agent Plugin"), NAME_None, FVerdunAIAgentStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
