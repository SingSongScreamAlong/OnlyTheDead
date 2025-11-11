// Copyright Only The Dead Development Team. All Rights Reserved.

#include "AIAgentCore.h"
#include "Json.h"
#include "JsonUtilities.h"

const FString UAIAgentCore::AgentSystemPrompt = TEXT(
	"You are an autonomous AI agent integrated into Unreal Engine 5. Your role is to help developers build game content through natural language commands.\n\n"
	"CAPABILITIES:\n"
	"- Create and manipulate actors, blueprints, materials\n"
	"- Sculpt and modify terrain (landscapes)\n"
	"- Place and organize assets\n"
	"- Generate procedural content\n"
	"- Execute Python scripts in UE5\n"
	"- Access the full UE5 API\n\n"
	"RESPONSE FORMAT:\n"
	"When given a task, respond with a JSON plan:\n"
	"{\n"
	"  \"description\": \"Brief description of what you'll do\",\n"
	"  \"steps\": [\n"
	"    {\n"
	"      \"description\": \"Step description\",\n"
	"      \"commandType\": \"CreateActor|ModifyTerrain|SetMaterial|ExecutePython|etc\",\n"
	"      \"parameters\": {\"key\": \"value\"}\n"
	"    }\n"
	"  ]\n"
	"}\n\n"
	"IMPORTANT:\n"
	"- Break complex tasks into simple, atomic steps\n"
	"- Each step should be independently executable\n"
	"- Use project context to understand domain-specific terms\n"
	"- Verify your work after completion\n"
	"- If uncertain, ask for clarification\n\n"
);

UAIAgentCore::UAIAgentCore()
	: CurrentStepIndex(0)
	, bUseToolCalling(true)
	, bAutonomousMode(false)
	, RetryCount(0)
	, MaxRetries(3)
{
}

void UAIAgentCore::Initialize(const FLLMRequestConfig& InConfig, UWorld* World)
{
	Config = InConfig;
	Config.bEnableToolUse = true; // Enable native tool calling
	WorldContext = World;

	// Create LLM integration
	if (!LLMIntegration)
	{
		LLMIntegration = NewObject<ULLMIntegration>(this);
	}

	// Create and initialize ToolRegistry
	if (!ToolRegistry)
	{
		ToolRegistry = NewObject<UToolRegistry>(this);
		ToolRegistry->InitializeDefaultTools(World);
	}

	// Build system prompt
	if (bUseToolCalling)
	{
		Config.SystemPrompt = BuildToolUseSystemPrompt();
	}
	else
	{
		Config.SystemPrompt = AgentSystemPrompt + TEXT("\nPROJECT CONTEXT:\n") + ProjectContext;
	}

	LogMessage(TEXT("AI Agent initialized with ") + UEnum::GetValueAsString(Config.Provider));
	LogMessage(FString::Printf(TEXT("Tool Calling: %s | Autonomous Mode: %s"),
		bUseToolCalling ? TEXT("Enabled") : TEXT("Disabled"),
		bAutonomousMode ? TEXT("Enabled") : TEXT("Disabled")));
}

void UAIAgentCore::ExecuteCommand(const FString& Command)
{
	if (CurrentTask.Status == EAgentTaskStatus::Planning ||
		CurrentTask.Status == EAgentTaskStatus::Executing)
	{
		LogMessage(TEXT("Agent is already working on a task. Please wait or call StopExecution()."));
		return;
	}

	LogMessage(FString::Printf(TEXT("Received command: %s"), *Command));

	CurrentTask = FAgentTask();
	CurrentTask.UserRequest = Command;
	CurrentTask.StartTime = FDateTime::Now();
	CurrentStepIndex = 0;
	RetryCount = 0;

	// Use tool calling if enabled, otherwise fall back to legacy JSON planning
	if (bUseToolCalling)
	{
		StartToolCallingExecution(Command);
	}
	else
	{
		StartPlanning(Command);
	}
}

void UAIAgentCore::StopExecution()
{
	if (LLMIntegration)
	{
		LLMIntegration->CancelRequest();
	}

	CurrentTask.Status = EAgentTaskStatus::Failed;
	CurrentTask.EndTime = FDateTime::Now();

	LogMessage(TEXT("Task execution stopped by user"));
	OnTaskCompleted.Broadcast(CurrentTask.UserRequest, false);
}

void UAIAgentCore::SetProjectContext(const FString& ContextDescription)
{
	ProjectContext = ContextDescription;
	Config.SystemPrompt = AgentSystemPrompt + TEXT("\nPROJECT CONTEXT:\n") + ProjectContext;

	if (!KnowledgeBase.IsEmpty())
	{
		Config.SystemPrompt += TEXT("\n\nKNOWLEDGE BASE:\n");
		for (const FString& Knowledge : KnowledgeBase)
		{
			Config.SystemPrompt += TEXT("- ") + Knowledge + TEXT("\n");
		}
	}

	LogMessage(TEXT("Project context updated"));
}

void UAIAgentCore::AddKnowledge(const FString& Knowledge)
{
	KnowledgeBase.Add(Knowledge);
	SetProjectContext(ProjectContext); // Rebuild system prompt
	LogMessage(FString::Printf(TEXT("Knowledge added: %s"), *Knowledge));
}

// ============================================================================
// PLANNING PHASE
// ============================================================================

void UAIAgentCore::StartPlanning(const FString& UserCommand)
{
	CurrentTask.Status = EAgentTaskStatus::Planning;
	OnTaskStarted.Broadcast(UserCommand);
	OnThinking.Broadcast(TEXT("Analyzing request and creating execution plan..."), 0.1f);

	FString PlanningPrompt = BuildPlanningPrompt(UserCommand);

	FOnLLMResponseReceived OnSuccess;
	OnSuccess.BindUObject(this, &UAIAgentCore::OnPlanningComplete);

	FOnLLMRequestFailed OnFailure;
	OnFailure.BindUObject(this, &UAIAgentCore::OnPlanningFailed);

	LLMIntegration->SendRequest(PlanningPrompt, Config, OnSuccess, OnFailure);
}

FString UAIAgentCore::BuildPlanningPrompt(const FString& UserCommand)
{
	FString Prompt = FString::Printf(
		TEXT("The user wants you to: \"%s\"\n\n"
		     "Create a detailed execution plan as JSON. Break this into atomic steps that can be executed in Unreal Engine 5.\n\n"
		     "Available command types:\n"
		     "- CreateActor: Spawn an actor in the level\n"
		     "- ModifyTerrain: Sculpt or modify landscape\n"
		     "- CreateBlueprint: Create a new blueprint class\n"
		     "- SetMaterial: Apply a material to an actor\n"
		     "- ExecutePython: Run a Python script in UE5\n"
		     "- PlaceAsset: Place an asset from content browser\n"
		     "- CreateSpline: Create a spline for paths/roads/trenches\n"
		     "- ModifyGeometry: Modify mesh geometry\n\n"
		     "Respond ONLY with valid JSON, no other text."),
		*UserCommand
	);

	return Prompt;
}

void UAIAgentCore::OnPlanningComplete(const FString& Response)
{
	LogMessage(TEXT("Planning complete, parsing response..."));
	OnThinking.Broadcast(TEXT("Plan created, preparing to execute..."), 0.3f);

	ParsePlanFromLLM(Response);

	if (CurrentTask.Steps.Num() > 0)
	{
		StartExecution();
	}
	else
	{
		OnPlanningFailed(TEXT("No valid steps in plan"));
	}
}

void UAIAgentCore::OnPlanningFailed(const FString& Error)
{
	LogMessage(FString::Printf(TEXT("Planning failed: %s"), *Error));
	CurrentTask.Status = EAgentTaskStatus::Failed;
	CurrentTask.EndTime = FDateTime::Now();
	OnTaskCompleted.Broadcast(CurrentTask.UserRequest, false);
}

void UAIAgentCore::ParsePlanFromLLM(const FString& LLMResponse)
{
	// Extract JSON from response (LLM might include markdown code blocks)
	FString JsonString = LLMResponse;

	// Remove markdown code blocks if present
	if (JsonString.Contains(TEXT("```json")))
	{
		int32 StartIdx = JsonString.Find(TEXT("```json")) + 7;
		int32 EndIdx = JsonString.Find(TEXT("```"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIdx);
		if (EndIdx > StartIdx)
		{
			JsonString = JsonString.Mid(StartIdx, EndIdx - StartIdx).TrimStartAndEnd();
		}
	}
	else if (JsonString.Contains(TEXT("```")))
	{
		int32 StartIdx = JsonString.Find(TEXT("```")) + 3;
		int32 EndIdx = JsonString.Find(TEXT("```"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIdx);
		if (EndIdx > StartIdx)
		{
			JsonString = JsonString.Mid(StartIdx, EndIdx - StartIdx).TrimStartAndEnd();
		}
	}

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		LogMessage(TEXT("Failed to parse JSON plan"));
		return;
	}

	// Extract description
	FString Description;
	if (JsonObject->TryGetStringField(TEXT("description"), Description))
	{
		CurrentTask.Description = Description;
	}

	// Extract steps
	const TArray<TSharedPtr<FJsonValue>>* StepsArray;
	if (JsonObject->TryGetArrayField(TEXT("steps"), StepsArray))
	{
		for (const TSharedPtr<FJsonValue>& StepValue : *StepsArray)
		{
			TSharedPtr<FJsonObject> StepObj = StepValue->AsObject();
			if (!StepObj.IsValid()) continue;

			FAgentTaskStep Step;
			StepObj->TryGetStringField(TEXT("description"), Step.Description);
			StepObj->TryGetStringField(TEXT("commandType"), Step.CommandType);

			// Parse parameters
			TSharedPtr<FJsonObject> ParamsObj = StepObj->GetObjectField(TEXT("parameters"));
			if (ParamsObj.IsValid())
			{
				for (const auto& Param : ParamsObj->Values)
				{
					FString Value;
					if (Param.Value->TryGetString(Value))
					{
						Step.Parameters.Add(Param.Key, Value);
					}
				}
			}

			CurrentTask.Steps.Add(Step);
		}
	}

	LogMessage(FString::Printf(TEXT("Parsed %d steps from plan"), CurrentTask.Steps.Num()));
}

// ============================================================================
// EXECUTION PHASE
// ============================================================================

void UAIAgentCore::StartExecution()
{
	CurrentTask.Status = EAgentTaskStatus::Executing;
	CurrentStepIndex = 0;

	LogMessage(FString::Printf(TEXT("Starting execution: %s"), *CurrentTask.Description));
	LogMessage(FString::Printf(TEXT("Total steps: %d"), CurrentTask.Steps.Num()));

	ExecuteNextStep();
}

void UAIAgentCore::ExecuteNextStep()
{
	if (CurrentStepIndex >= CurrentTask.Steps.Num())
	{
		// All steps completed
		VerifyCompletion();
		return;
	}

	FAgentTaskStep& CurrentStep = CurrentTask.Steps[CurrentStepIndex];

	float Progress = (float)CurrentStepIndex / (float)CurrentTask.Steps.Num();
	OnThinking.Broadcast(
		FString::Printf(TEXT("Step %d/%d: %s"), CurrentStepIndex + 1, CurrentTask.Steps.Num(), *CurrentStep.Description),
		Progress
	);

	LogMessage(FString::Printf(TEXT("Executing step %d: %s [%s]"),
		CurrentStepIndex + 1,
		*CurrentStep.Description,
		*CurrentStep.CommandType));

	// NOTE: This is where we would call the UE5 command executor
	// For now, we'll simulate execution
	// TODO: Integrate with UE5CommandExecutor

	// Simulate step completion
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		OnStepComplete(true, TEXT("Step completed successfully (simulated)"));
	}, 1.0f, false);
}

void UAIAgentCore::OnStepComplete(bool bSuccess, const FString& Result)
{
	if (CurrentStepIndex >= CurrentTask.Steps.Num())
	{
		return;
	}

	FAgentTaskStep& CurrentStep = CurrentTask.Steps[CurrentStepIndex];
	CurrentStep.bCompleted = bSuccess;
	CurrentStep.Result = Result;

	if (bSuccess)
	{
		LogMessage(FString::Printf(TEXT("Step %d completed: %s"), CurrentStepIndex + 1, *Result));
		CurrentStepIndex++;
		ExecuteNextStep();
	}
	else
	{
		LogMessage(FString::Printf(TEXT("Step %d failed: %s"), CurrentStepIndex + 1, *Result));
		CurrentTask.Status = EAgentTaskStatus::Failed;
		CurrentTask.EndTime = FDateTime::Now();
		OnTaskCompleted.Broadcast(CurrentTask.UserRequest, false);
	}
}

// ============================================================================
// VERIFICATION PHASE
// ============================================================================

void UAIAgentCore::VerifyCompletion()
{
	CurrentTask.Status = EAgentTaskStatus::Verifying;
	OnThinking.Broadcast(TEXT("Verifying task completion..."), 0.95f);

	LogMessage(TEXT("All steps completed, verifying..."));

	// For now, we'll assume success if all steps completed
	// TODO: Add LLM-based verification
	CurrentTask.Status = EAgentTaskStatus::Completed;
	CurrentTask.EndTime = FDateTime::Now();

	LogMessage(TEXT("Task completed successfully!"));
	OnTaskCompleted.Broadcast(CurrentTask.UserRequest, true);
}

void UAIAgentCore::OnVerificationComplete(const FString& Response)
{
	// TODO: Parse verification response
	CurrentTask.Status = EAgentTaskStatus::Completed;
	CurrentTask.EndTime = FDateTime::Now();
	OnTaskCompleted.Broadcast(CurrentTask.UserRequest, true);
}

FString UAIAgentCore::BuildVerificationPrompt()
{
	FString Prompt = TEXT("Review the following completed steps and verify if the original request was fulfilled:\n\n");
	Prompt += FString::Printf(TEXT("Original Request: %s\n\n"), *CurrentTask.UserRequest);
	Prompt += TEXT("Completed Steps:\n");

	for (int32 i = 0; i < CurrentTask.Steps.Num(); i++)
	{
		const FAgentTaskStep& Step = CurrentTask.Steps[i];
		Prompt += FString::Printf(TEXT("%d. %s - %s\n"), i + 1, *Step.Description, *Step.Result);
	}

	Prompt += TEXT("\nDid we successfully complete the user's request? Respond with YES or NO and explain why.");

	return Prompt;
}

void UAIAgentCore::LogMessage(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[AIAgent] %s"), *Message);
	OnLog.Broadcast(Message);
}

// ============================================================================
// TOOL CALLING EXECUTION (Phase 4)
// ============================================================================

void UAIAgentCore::StartToolCallingExecution(const FString& UserCommand)
{
	CurrentTask.Status = EAgentTaskStatus::Executing;
	OnTaskStarted.Broadcast(UserCommand);

	LogMessage(TEXT("Starting tool calling execution..."));
	OnThinking.Broadcast(TEXT("Understanding your request and determining which tools to use..."), 0.1f);

	if (!ToolRegistry)
	{
		LogMessage(TEXT("ERROR: ToolRegistry not initialized!"));
		OnToolCallResponseFailed(TEXT("ToolRegistry not initialized"));
		return;
	}

	// Get tool definitions as JSON
	FString ToolDefinitionsJSON = ToolRegistry->GenerateToolSchemaJSON();

	// Send request with tools
	LLMIntegration->SendRequestWithTools(
		UserCommand,
		Config,
		ToolDefinitionsJSON,
		FOnLLMResponseReceived::CreateUObject(this, &UAIAgentCore::OnLLMResponseReceived),
		FOnLLMRequestFailed::CreateUObject(this, &UAIAgentCore::OnToolCallResponseFailed)
	);
}

void UAIAgentCore::OnLLMResponseReceived(const FString& Response)
{
	LogMessage(TEXT("Received LLM response"));

	// Check if response contains a tool call
	FString ToolName;
	FString ToolInput;

	if (LLMIntegration->IsToolCallResponse(Response, ToolName, ToolInput))
	{
		LogMessage(FString::Printf(TEXT("LLM requested tool: %s"), *ToolName));
		OnThinking.Broadcast(FString::Printf(TEXT("Executing tool: %s"), *ToolName), 0.5f);

		ProcessToolCall(ToolName, ToolInput);
	}
	else
	{
		// No tool call - this is the final response
		LogMessage(TEXT("Task completed - no more tools needed"));
		OnThinking.Broadcast(TEXT("Task completed successfully!"), 1.0f);

		CurrentTask.Status = EAgentTaskStatus::Completed;
		CurrentTask.EndTime = FDateTime::Now();
		OnTaskCompleted.Broadcast(CurrentTask.UserRequest, true);
	}
}

void UAIAgentCore::OnToolCallResponseFailed(const FString& Error)
{
	LogMessage(FString::Printf(TEXT("Tool call failed: %s"), *Error));

	CurrentTask.Status = EAgentTaskStatus::Failed;
	CurrentTask.EndTime = FDateTime::Now();
	OnTaskCompleted.Broadcast(CurrentTask.UserRequest, false);
}

void UAIAgentCore::ProcessToolCall(const FString& ToolName, const FString& ToolInput)
{
	if (!ToolRegistry)
	{
		LogMessage(TEXT("ERROR: ToolRegistry not initialized!"));
		OnToolExecutionComplete(ToolName, FToolExecutionResult());
		return;
	}

	// Parse tool input JSON to parameters map
	TMap<FString, FString> Parameters;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ToolInput);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		for (const auto& Pair : JsonObject->Values)
		{
			FString Value;
			if (Pair.Value->TryGetString(Value))
			{
				Parameters.Add(Pair.Key, Value);
			}
			else if (Pair.Value->Type == EJson::Number)
			{
				double NumValue = Pair.Value->AsNumber();
				Parameters.Add(Pair.Key, FString::SanitizeFloat(NumValue));
			}
			else if (Pair.Value->Type == EJson::Boolean)
			{
				bool BoolValue = Pair.Value->AsBool();
				Parameters.Add(Pair.Key, BoolValue ? TEXT("true") : TEXT("false"));
			}
		}
	}

	LogMessage(FString::Printf(TEXT("Executing tool '%s' with %d parameters"), *ToolName, Parameters.Num()));

	// Execute the tool
	FToolExecutionResult Result = ToolRegistry->ExecuteTool(ToolName, Parameters);

	// Report back to agent
	OnToolExecutionComplete(ToolName, Result);
}

void UAIAgentCore::OnToolExecutionComplete(const FString& ToolName, const FToolExecutionResult& Result)
{
	if (Result.bSuccess)
	{
		LogMessage(FString::Printf(TEXT("Tool '%s' executed successfully: %s"), *ToolName, *Result.Result));
		OnThinking.Broadcast(FString::Printf(TEXT("Tool '%s' completed successfully"), *ToolName), 0.7f);

		// Send result back to LLM to continue conversation
		LLMIntegration->SendToolResult(
			ToolName,
			Result.Result,
			Config,
			FOnLLMResponseReceived::CreateUObject(this, &UAIAgentCore::OnLLMResponseReceived),
			FOnLLMRequestFailed::CreateUObject(this, &UAIAgentCore::OnToolCallResponseFailed)
		);
	}
	else
	{
		LogMessage(FString::Printf(TEXT("Tool '%s' failed: %s"), *ToolName, *Result.ErrorMessage));

		if (bAutonomousMode && RetryCount < MaxRetries)
		{
			// Autonomous retry
			RetryCount++;
			LogMessage(FString::Printf(TEXT("Autonomous retry %d/%d..."), RetryCount, MaxRetries));
			OnThinking.Broadcast(FString::Printf(TEXT("Retry attempt %d/%d..."), RetryCount, MaxRetries), 0.6f);

			// Send error back to LLM so it can try a different approach
			FString ErrorResult = FString::Printf(TEXT("Error: %s. Please try a different approach."), *Result.ErrorMessage);
			LLMIntegration->SendToolResult(
				ToolName,
				ErrorResult,
				Config,
				FOnLLMResponseReceived::CreateUObject(this, &UAIAgentCore::OnLLMResponseReceived),
				FOnLLMRequestFailed::CreateUObject(this, &UAIAgentCore::OnToolCallResponseFailed)
			);
		}
		else
		{
			// Give up after max retries
			LogMessage(TEXT("Max retries reached or autonomous mode disabled. Task failed."));
			CurrentTask.Status = EAgentTaskStatus::Failed;
			CurrentTask.EndTime = FDateTime::Now();
			OnTaskCompleted.Broadcast(CurrentTask.UserRequest, false);
		}
	}
}

// ============================================================================
// AUTONOMOUS ITERATION
// ============================================================================

void UAIAgentCore::StartAutonomousIteration()
{
	if (!bAutonomousMode)
	{
		return;
	}

	LogMessage(TEXT("Starting autonomous iteration..."));
	OnThinking.Broadcast(TEXT("Reviewing work and checking if improvements are needed..."), 0.9f);

	FString IterationPrompt = TEXT("Review what you just accomplished. ");
	IterationPrompt += TEXT("Does it meet the user's requirements? ");
	IterationPrompt += TEXT("If not, what tools should you use to improve it? ");
	IterationPrompt += TEXT("If yes, respond without calling any more tools.");

	LLMIntegration->SendRequest(
		IterationPrompt,
		Config,
		FOnLLMResponseReceived::CreateUObject(this, &UAIAgentCore::OnIterationThinkingComplete),
		FOnLLMRequestFailed::CreateUObject(this, &UAIAgentCore::OnToolCallResponseFailed)
	);
}

void UAIAgentCore::OnIterationThinkingComplete(const FString& Response)
{
	FString ToolName;
	FString ToolInput;

	if (LLMIntegration->IsToolCallResponse(Response, ToolName, ToolInput))
	{
		// Agent wants to make improvements
		LogMessage(TEXT("Agent is making improvements..."));
		ProcessToolCall(ToolName, ToolInput);
	}
	else
	{
		// Agent is satisfied with the result
		LogMessage(TEXT("Agent verified work is complete"));
		CurrentTask.Status = EAgentTaskStatus::Completed;
		CurrentTask.EndTime = FDateTime::Now();
		OnTaskCompleted.Broadcast(CurrentTask.UserRequest, true);
	}
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FString UAIAgentCore::BuildToolUseSystemPrompt()
{
	FString Prompt = TEXT("You are an autonomous AI agent integrated into Unreal Engine 5.\n\n");

	Prompt += TEXT("You have access to powerful tools that allow you to:\n");
	Prompt += TEXT("- Modify terrain (create craters, excavate trenches, raise/lower ground)\n");
	Prompt += TEXT("- Generate WWI battlefield content (trenches, barbed wire, fortifications)\n");
	Prompt += TEXT("- Inspect the level (find actors, query scene state, get context)\n");
	Prompt += TEXT("- Capture and analyze screenshots (visual verification)\n");
	Prompt += TEXT("- Spawn and manipulate actors in the scene\n\n");

	Prompt += TEXT("HOW TO USE TOOLS:\n");
	Prompt += TEXT("When you receive a user request, use the appropriate tools to accomplish it.\n");
	Prompt += TEXT("You can call multiple tools in sequence - each tool result will be sent back to you.\n");
	Prompt += TEXT("When the task is complete, respond without calling any more tools.\n\n");

	Prompt += TEXT("IMPORTANT GUIDELINES:\n");
	Prompt += TEXT("- Always inspect the level first before modifying it\n");
	Prompt += TEXT("- Use appropriate UE5 units (100 units = 1 meter)\n");
	Prompt += TEXT("- Break complex tasks into multiple tool calls\n");
	Prompt += TEXT("- Verify your work if possible (use vision tools)\n\n");

	if (!ProjectContext.IsEmpty())
	{
		Prompt += TEXT("PROJECT CONTEXT:\n");
		Prompt += ProjectContext + TEXT("\n\n");
	}

	if (!KnowledgeBase.IsEmpty())
	{
		Prompt += TEXT("KNOWLEDGE BASE:\n");
		for (const FString& Knowledge : KnowledgeBase)
		{
			Prompt += TEXT("- ") + Knowledge + TEXT("\n");
		}
		Prompt += TEXT("\n");
	}

	return Prompt;
}
