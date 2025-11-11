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
{
}

void UAIAgentCore::Initialize(const FLLMRequestConfig& InConfig)
{
	Config = InConfig;
	Config.SystemPrompt = AgentSystemPrompt + TEXT("\nPROJECT CONTEXT:\n") + ProjectContext;

	if (!LLMIntegration)
	{
		LLMIntegration = NewObject<ULLMIntegration>(this);
	}

	LogMessage(TEXT("AI Agent initialized with ") + UEnum::GetValueAsString(Config.Provider));
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

	StartPlanning(Command);
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
