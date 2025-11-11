// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIAgentCore.h"
#include "UE5CommandExecutor.generated.h"

DECLARE_DELEGATE_TwoParams(FOnCommandExecuted, bool /* bSuccess */, const FString& /* Result */);

/**
 * UE5 Command Executor - Executes AI agent commands in Unreal Engine
 * This class provides the interface between the AI agent and UE5's APIs
 */
UCLASS()
class VERDUNAIAGENT_API UUE5CommandExecutor : public UObject
{
	GENERATED_BODY()

public:
	UUE5CommandExecutor();

	/**
	 * Execute a command step from the AI agent
	 * @param Step - The command step to execute
	 * @param OnComplete - Callback when execution completes
	 */
	void ExecuteStep(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);

	/**
	 * Get the current editor world
	 */
	UWorld* GetEditorWorld() const;

private:
	// Command handlers
	void ExecuteCreateActor(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);
	void ExecuteModifyTerrain(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);
	void ExecuteCreateBlueprint(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);
	void ExecuteSetMaterial(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);
	void ExecutePythonScript(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);
	void ExecutePlaceAsset(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);
	void ExecuteCreateSpline(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);
	void ExecuteModifyGeometry(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete);

	// Helper functions
	FVector ParseVector(const FString& VectorString);
	FRotator ParseRotator(const FString& RotatorString);
	UClass* FindClass(const FString& ClassName);
	UObject* FindAsset(const FString& AssetPath);

	// Current execution context
	FOnCommandExecuted CurrentCallback;
};
