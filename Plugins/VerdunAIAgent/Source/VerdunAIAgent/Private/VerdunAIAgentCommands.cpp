// Copyright Only The Dead Development Team. All Rights Reserved.

#include "VerdunAIAgentCommands.h"

#define LOCTEXT_NAMESPACE "FVerdunAIAgentModule"

void FVerdunAIAgentCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Verdun AI Agent", "Open Verdun AI Agent window to interact with the autonomous development assistant", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
