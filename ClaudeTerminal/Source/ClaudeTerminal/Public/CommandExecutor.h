// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UWorld;

/**
 * Parses and executes commands from Claude's responses
 */
class CLAUDETERMINAL_API FCommandExecutor
{
public:
	FCommandExecutor();
	~FCommandExecutor();

	/**
	 * Process a response from Claude and execute any commands found
	 * @param Response The text response from Claude
	 * @param World The world to execute commands in
	 * @param OutExecutionLog Output log of what was executed
	 * @return true if commands were found and executed
	 */
	bool ProcessResponse(const FString& Response, UWorld* World, FString& OutExecutionLog);

private:
	// Extract JSON command from markdown code blocks
	bool ExtractJSONCommand(const FString& Response, TSharedPtr<FJsonObject>& OutCommand);

	// Command handlers
	bool ExecuteSpawnActor(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteCreateRoad(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecutePlaceModel(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteModifyLandscape(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteCreateMaterial(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);
	bool ExecuteBlueprint(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog);

	// Helper functions
	FVector ParseVector(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FVector& Default = FVector::ZeroVector);
	FRotator ParseRotator(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FRotator& Default = FRotator::ZeroRotator);

public:
	/**
	 * Get help text describing available commands
	 * @return Formatted help text
	 */
	static FString GetHelpText();
};
