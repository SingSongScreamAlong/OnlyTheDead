// Copyright Only The Dead Development Team. All Rights Reserved.

#include "VerdunAIAgentRuntime.h"

#define LOCTEXT_NAMESPACE "FVerdunAIAgentRuntimeModule"

void FVerdunAIAgentRuntimeModule::StartupModule()
{
	// Runtime module initialization
	UE_LOG(LogTemp, Log, TEXT("VerdunAIAgent Runtime Module Started"));
}

void FVerdunAIAgentRuntimeModule::ShutdownModule()
{
	// Runtime module shutdown
	UE_LOG(LogTemp, Log, TEXT("VerdunAIAgent Runtime Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVerdunAIAgentRuntimeModule, VerdunAIAgentRuntime)
