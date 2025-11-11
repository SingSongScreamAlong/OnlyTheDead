// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LLMIntegration.h"
#include "AIAgentCore.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAgentTaskStarted, const FString& /* TaskDescription */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAgentTaskCompleted, const FString& /* TaskDescription */, bool /* bSuccess */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAgentThinking, const FString& /* ThoughtProcess */, float /* Progress */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAgentLog, const FString& /* LogMessage */);

/**
 * Task execution status
 */
UENUM(BlueprintType)
enum class EAgentTaskStatus : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Planning UMETA(DisplayName = "Planning"),
	Executing UMETA(DisplayName = "Executing"),
	Verifying UMETA(DisplayName = "Verifying"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * Individual task step in the agent's plan
 */
USTRUCT(BlueprintType)
struct FAgentTaskStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString CommandType; // e.g., "CreateActor", "ModifyTerrain", "SetMaterial"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	TMap<FString, FString> Parameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString Result;
};

/**
 * Agent task containing multiple steps
 */
USTRUCT(BlueprintType)
struct FAgentTask
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString UserRequest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	TArray<FAgentTaskStep> Steps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	EAgentTaskStatus Status = EAgentTaskStatus::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FDateTime EndTime;
};

/**
 * Core AI Agent System - Plans and executes tasks autonomously
 */
UCLASS(BlueprintType)
class VERDUNAIAGENTRUNTIME_API UAIAgentCore : public UObject
{
	GENERATED_BODY()

public:
	UAIAgentCore();

	/**
	 * Initialize the agent with LLM configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent")
	void Initialize(const FLLMRequestConfig& InConfig);

	/**
	 * Execute a natural language command
	 * @param Command - Natural language command (e.g., "Create a trench system from A to B")
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent")
	void ExecuteCommand(const FString& Command);

	/**
	 * Stop the current task execution
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent")
	void StopExecution();

	/**
	 * Get current task status
	 */
	UFUNCTION(BlueprintPure, Category = "AI Agent")
	EAgentTaskStatus GetCurrentStatus() const { return CurrentTask.Status; }

	/**
	 * Get current task
	 */
	UFUNCTION(BlueprintPure, Category = "AI Agent")
	FAgentTask GetCurrentTask() const { return CurrentTask; }

	/**
	 * Set project context (helps agent understand project-specific terms)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent")
	void SetProjectContext(const FString& ContextDescription);

	/**
	 * Add knowledge to the agent (e.g., "Verdun trenches are 2m deep")
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent")
	void AddKnowledge(const FString& Knowledge);

	// Delegates
	FOnAgentTaskStarted OnTaskStarted;
	FOnAgentTaskCompleted OnTaskCompleted;
	FOnAgentThinking OnThinking;
	FOnAgentLog OnLog;

private:
	// Planning phase
	void StartPlanning(const FString& UserCommand);
	void OnPlanningComplete(const FString& Response);
	void OnPlanningFailed(const FString& Error);
	void ParsePlanFromLLM(const FString& LLMResponse);

	// Execution phase
	void StartExecution();
	void ExecuteNextStep();
	void OnStepComplete(bool bSuccess, const FString& Result);

	// Verification phase
	void VerifyCompletion();
	void OnVerificationComplete(const FString& Response);

	// Helper functions
	FString BuildPlanningPrompt(const FString& UserCommand);
	FString BuildExecutionPrompt(const FAgentTaskStep& Step);
	FString BuildVerificationPrompt();
	void LogMessage(const FString& Message);

	// Member variables
	UPROPERTY()
	ULLMIntegration* LLMIntegration;

	FLLMRequestConfig Config;
	FAgentTask CurrentTask;
	int32 CurrentStepIndex;

	FString ProjectContext;
	TArray<FString> KnowledgeBase;

	// System prompt for the agent
	static const FString AgentSystemPrompt;
};
