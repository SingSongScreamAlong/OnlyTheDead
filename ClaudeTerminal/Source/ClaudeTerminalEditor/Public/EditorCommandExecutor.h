// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommandExecutor.h"
#include "LandscapeManager.h"

/**
 * Extended command executor with editor-only features (landscape manipulation)
 * Wraps the base CommandExecutor and adds landscape/terrain commands
 */
class CLAUDETERMINALEDITOR_API FEditorCommandExecutor
{
public:
	FEditorCommandExecutor();
	~FEditorCommandExecutor();

	/**
	 * Process response - tries editor commands first, then falls back to base commands
	 */
	bool ProcessResponse(const FString& Response, UWorld* World, FString& OutExecutionLog);

	/**
	 * Get extended help text including landscape commands
	 */
	static FString GetHelpText();

private:
	// Extract JSON command
	bool ExtractJSONCommand(const FString& Response, TSharedPtr<FJsonObject>& OutCommand);

	// Landscape command handlers
	bool ExecuteCreateLandscape(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteImportTerrain(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteModifyTerrain(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteCreateCrater(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteCreateTrench(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// Helper functions
	FVector ParseVector(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FVector& Default = FVector::ZeroVector);
	FRotator ParseRotator(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FRotator& Default = FRotator::ZeroRotator);

	// Components
	FCommandExecutor BaseExecutor;
	FLandscapeManager LandscapeManager;
};
