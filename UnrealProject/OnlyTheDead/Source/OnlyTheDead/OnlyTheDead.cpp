// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

#include "OnlyTheDead.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FOnlyTheDeadModule, OnlyTheDead, "OnlyTheDead");

void FOnlyTheDeadModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("OnlyTheDead Module Started - Battle of Verdun 1916"));
}

void FOnlyTheDeadModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("OnlyTheDead Module Shutdown"));
}
