// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerdunAutoTerrainBuilder.h"

#define LOCTEXT_NAMESPACE "FVerdunAutoTerrainBuilderModule"

void FVerdunAutoTerrainBuilderModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogTemp, Log, TEXT("VerdunAutoTerrainBuilder Runtime Module Started"));
}

void FVerdunAutoTerrainBuilderModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogTemp, Log, TEXT("VerdunAutoTerrainBuilder Runtime Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVerdunAutoTerrainBuilderModule, VerdunAutoTerrainBuilder)
