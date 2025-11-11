// Copyright Only The Dead Development Team. All Rights Reserved.

#include "LLMIntegration.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"

ULLMIntegration::ULLMIntegration()
{
}

void ULLMIntegration::SendRequest(
	const FString& UserMessage,
	const FLLMRequestConfig& Config,
	FOnLLMResponseReceived OnSuccess,
	FOnLLMRequestFailed OnFailure)
{
	SuccessCallback = OnSuccess;
	FailureCallback = OnFailure;
	CurrentConfig = Config;

	UE_LOG(LogTemp, Log, TEXT("Sending LLM request to %s"), *UEnum::GetValueAsString(Config.Provider));

	switch (Config.Provider)
	{
	case ELLMProvider::OpenAI:
		SendOpenAIRequest(UserMessage, Config);
		break;
	case ELLMProvider::Anthropic:
		SendAnthropicRequest(UserMessage, Config);
		break;
	case ELLMProvider::Local:
		SendLocalRequest(UserMessage, Config);
		break;
	}
}

void ULLMIntegration::SendStreamingRequest(
	const FString& UserMessage,
	const FLLMRequestConfig& Config,
	FOnLLMResponseReceived OnChunkReceived,
	FOnLLMRequestFailed OnFailure)
{
	// TODO: Implement streaming support
	UE_LOG(LogTemp, Warning, TEXT("Streaming not yet implemented, falling back to regular request"));
	SendRequest(UserMessage, Config, OnChunkReceived, OnFailure);
}

void ULLMIntegration::CancelRequest()
{
	if (CurrentRequest.IsValid())
	{
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
		UE_LOG(LogTemp, Log, TEXT("LLM request cancelled"));
	}
}

// ============================================================================
// OPENAI IMPLEMENTATION
// ============================================================================

void ULLMIntegration::SendOpenAIRequest(const FString& UserMessage, const FLLMRequestConfig& Config)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Config.APIKey));

	Request->SetContentAsString(BuildOpenAIPayload(UserMessage, Config));
	Request->OnProcessRequestComplete().BindUObject(this, &ULLMIntegration::HandleOpenAIResponse);

	CurrentRequest = Request;
	Request->ProcessRequest();
}

FString ULLMIntegration::BuildOpenAIPayload(const FString& UserMessage, const FLLMRequestConfig& Config)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("model"), Config.ModelName.IsEmpty() ? TEXT("gpt-4") : Config.ModelName);
	JsonObject->SetNumberField(TEXT("temperature"), Config.Temperature);
	JsonObject->SetNumberField(TEXT("max_tokens"), Config.MaxTokens);

	TArray<TSharedPtr<FJsonValue>> Messages;

	// System message
	if (!Config.SystemPrompt.IsEmpty())
	{
		TSharedPtr<FJsonObject> SystemMsg = MakeShareable(new FJsonObject);
		SystemMsg->SetStringField(TEXT("role"), TEXT("system"));
		SystemMsg->SetStringField(TEXT("content"), Config.SystemPrompt);
		Messages.Add(MakeShareable(new FJsonValueObject(SystemMsg)));
	}

	// User message
	TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject);
	UserMsg->SetStringField(TEXT("role"), TEXT("user"));
	UserMsg->SetStringField(TEXT("content"), UserMessage);
	Messages.Add(MakeShareable(new FJsonValueObject(UserMsg)));

	JsonObject->SetArrayField(TEXT("messages"), Messages);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return OutputString;
}

void ULLMIntegration::HandleOpenAIResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		FailureCallback.ExecuteIfBound(TEXT("Request failed or invalid response"));
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		FString ErrorMsg = FString::Printf(TEXT("HTTP Error %d: %s"),
			Response->GetResponseCode(),
			*Response->GetContentAsString());
		FailureCallback.ExecuteIfBound(ErrorMsg);
		return;
	}

	FString ResponseBody = Response->GetContentAsString();
	FString ExtractedMessage = ExtractMessageFromOpenAIResponse(ResponseBody);

	if (ExtractedMessage.IsEmpty())
	{
		FailureCallback.ExecuteIfBound(TEXT("Failed to extract message from response"));
		return;
	}

	SuccessCallback.ExecuteIfBound(ExtractedMessage);
}

FString ULLMIntegration::ExtractMessageFromOpenAIResponse(const FString& ResponseBody)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return FString();
	}

	const TArray<TSharedPtr<FJsonValue>>* Choices;
	if (!JsonObject->TryGetArrayField(TEXT("choices"), Choices) || Choices->Num() == 0)
	{
		return FString();
	}

	TSharedPtr<FJsonObject> FirstChoice = (*Choices)[0]->AsObject();
	if (!FirstChoice.IsValid())
	{
		return FString();
	}

	TSharedPtr<FJsonObject> Message = FirstChoice->GetObjectField(TEXT("message"));
	if (!Message.IsValid())
	{
		return FString();
	}

	return Message->GetStringField(TEXT("content"));
}

// ============================================================================
// ANTHROPIC (CLAUDE) IMPLEMENTATION
// ============================================================================

void ULLMIntegration::SendAnthropicRequest(const FString& UserMessage, const FLLMRequestConfig& Config)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(TEXT("https://api.anthropic.com/v1/messages"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-api-key"), Config.APIKey);
	Request->SetHeader(TEXT("anthropic-version"), TEXT("2023-06-01"));

	Request->SetContentAsString(BuildAnthropicPayload(UserMessage, Config));
	Request->OnProcessRequestComplete().BindUObject(this, &ULLMIntegration::HandleAnthropicResponse);

	CurrentRequest = Request;
	Request->ProcessRequest();
}

FString ULLMIntegration::BuildAnthropicPayload(const FString& UserMessage, const FLLMRequestConfig& Config)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("model"), Config.ModelName.IsEmpty() ? TEXT("claude-3-5-sonnet-20241022") : Config.ModelName);
	JsonObject->SetNumberField(TEXT("max_tokens"), Config.MaxTokens);
	JsonObject->SetNumberField(TEXT("temperature"), Config.Temperature);

	// System prompt
	if (!Config.SystemPrompt.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("system"), Config.SystemPrompt);
	}

	// Messages array
	TArray<TSharedPtr<FJsonValue>> Messages;
	TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject);
	UserMsg->SetStringField(TEXT("role"), TEXT("user"));
	UserMsg->SetStringField(TEXT("content"), UserMessage);
	Messages.Add(MakeShareable(new FJsonValueObject(UserMsg)));

	JsonObject->SetArrayField(TEXT("messages"), Messages);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return OutputString;
}

void ULLMIntegration::HandleAnthropicResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		FailureCallback.ExecuteIfBound(TEXT("Request failed or invalid response"));
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		FString ErrorMsg = FString::Printf(TEXT("HTTP Error %d: %s"),
			Response->GetResponseCode(),
			*Response->GetContentAsString());
		FailureCallback.ExecuteIfBound(ErrorMsg);
		return;
	}

	FString ResponseBody = Response->GetContentAsString();
	FString ExtractedMessage = ExtractMessageFromAnthropicResponse(ResponseBody);

	if (ExtractedMessage.IsEmpty())
	{
		FailureCallback.ExecuteIfBound(TEXT("Failed to extract message from response"));
		return;
	}

	SuccessCallback.ExecuteIfBound(ExtractedMessage);
}

FString ULLMIntegration::ExtractMessageFromAnthropicResponse(const FString& ResponseBody)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return FString();
	}

	const TArray<TSharedPtr<FJsonValue>>* Content;
	if (!JsonObject->TryGetArrayField(TEXT("content"), Content) || Content->Num() == 0)
	{
		return FString();
	}

	TSharedPtr<FJsonObject> FirstContent = (*Content)[0]->AsObject();
	if (!FirstContent.IsValid())
	{
		return FString();
	}

	return FirstContent->GetStringField(TEXT("text"));
}

// ============================================================================
// LOCAL MODEL IMPLEMENTATION (Ollama, LM Studio, etc.)
// ============================================================================

void ULLMIntegration::SendLocalRequest(const FString& UserMessage, const FLLMRequestConfig& Config)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	FString URL = Config.BaseURL.IsEmpty() ? TEXT("http://localhost:11434/api/generate") : Config.BaseURL;
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	Request->SetContentAsString(BuildLocalPayload(UserMessage, Config));
	Request->OnProcessRequestComplete().BindUObject(this, &ULLMIntegration::HandleLocalResponse);

	CurrentRequest = Request;
	Request->ProcessRequest();
}

FString ULLMIntegration::BuildLocalPayload(const FString& UserMessage, const FLLMRequestConfig& Config)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("model"), Config.ModelName.IsEmpty() ? TEXT("llama2") : Config.ModelName);

	FString FullPrompt = Config.SystemPrompt.IsEmpty()
		? UserMessage
		: FString::Printf(TEXT("%s\n\nUser: %s"), *Config.SystemPrompt, *UserMessage);

	JsonObject->SetStringField(TEXT("prompt"), FullPrompt);
	JsonObject->SetBoolField(TEXT("stream"), false);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return OutputString;
}

void ULLMIntegration::HandleLocalResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		FailureCallback.ExecuteIfBound(TEXT("Request failed or invalid response"));
		return;
	}

	FString ResponseBody = Response->GetContentAsString();
	FString ExtractedMessage = ExtractMessageFromLocalResponse(ResponseBody);

	if (ExtractedMessage.IsEmpty())
	{
		FailureCallback.ExecuteIfBound(TEXT("Failed to extract message from response"));
		return;
	}

	SuccessCallback.ExecuteIfBound(ExtractedMessage);
}

FString ULLMIntegration::ExtractMessageFromLocalResponse(const FString& ResponseBody)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return FString();
	}

	FString Response;
	if (JsonObject->TryGetStringField(TEXT("response"), Response))
	{
		return Response;
	}

	return FString();
}

// ============================================================================
// TOOL USE / FUNCTION CALLING IMPLEMENTATION
// ============================================================================

void ULLMIntegration::SendRequestWithTools(
	const FString& UserMessage,
	const FLLMRequestConfig& Config,
	const FString& ToolDefinitionsJSON,
	FOnLLMResponseReceived OnSuccess,
	FOnLLMRequestFailed OnFailure)
{
	SuccessCallback = OnSuccess;
	FailureCallback = OnFailure;
	CurrentConfig = Config;
	CurrentToolDefinitionsJSON = ToolDefinitionsJSON;

	UE_LOG(LogTemp, Log, TEXT("Sending LLM request with tools to %s"), *UEnum::GetValueAsString(Config.Provider));

	// Create user message
	TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject);
	UserMsg->SetStringField(TEXT("role"), TEXT("user"));
	UserMsg->SetStringField(TEXT("content"), UserMessage);
	MessageHistory.Add(MakeShareable(new FJsonValueObject(UserMsg)));

	// Send request based on provider
	if (Config.Provider == ELLMProvider::Anthropic)
	{
		SendAnthropicRequestWithTools(Config);
	}
	else if (Config.Provider == ELLMProvider::OpenAI)
	{
		SendOpenAIRequestWithTools(Config);
	}
	else
	{
		FailureCallback.ExecuteIfBound(TEXT("Tool use not supported for local models yet"));
	}
}

void ULLMIntegration::SendAnthropicRequestWithTools(const FLLMRequestConfig& Config)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(TEXT("https://api.anthropic.com/v1/messages"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-api-key"), Config.APIKey);
	Request->SetHeader(TEXT("anthropic-version"), TEXT("2023-06-01"));

	// Build payload with tools
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("model"), Config.ModelName.IsEmpty() ? TEXT("claude-3-5-sonnet-20241022") : Config.ModelName);
	JsonObject->SetNumberField(TEXT("max_tokens"), Config.MaxTokens);
	JsonObject->SetNumberField(TEXT("temperature"), Config.Temperature);

	// System prompt
	if (!Config.SystemPrompt.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("system"), Config.SystemPrompt);
	}

	// Messages
	JsonObject->SetArrayField(TEXT("messages"), MessageHistory);

	// Parse and add tools
	TSharedPtr<FJsonObject> ToolsObject;
	TSharedRef<TJsonReader<>> ToolsReader = TJsonReaderFactory<>::Create(CurrentToolDefinitionsJSON);
	if (FJsonSerializer::Deserialize(ToolsReader, ToolsObject))
	{
		const TArray<TSharedPtr<FJsonValue>>* ToolsArray;
		if (ToolsObject->TryGetArrayField(TEXT("tools"), ToolsArray))
		{
			JsonObject->SetArrayField(TEXT("tools"), *ToolsArray);
		}
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(OutputString);
	Request->OnProcessRequestComplete().BindUObject(this, &ULLMIntegration::HandleAnthropicToolResponse);

	CurrentRequest = Request;
	Request->ProcessRequest();
}

void ULLMIntegration::HandleAnthropicToolResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		FailureCallback.ExecuteIfBound(TEXT("Request failed or invalid response"));
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		FString ErrorMsg = FString::Printf(TEXT("HTTP Error %d: %s"),
			Response->GetResponseCode(),
			*Response->GetContentAsString());
		FailureCallback.ExecuteIfBound(ErrorMsg);
		return;
	}

	FString ResponseBody = Response->GetContentAsString();

	// Parse response to check for tool use
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// Add assistant message to history
		const TArray<TSharedPtr<FJsonValue>>* Content;
		if (JsonObject->TryGetArrayField(TEXT("content"), Content))
		{
			TSharedPtr<FJsonObject> AssistantMsg = MakeShareable(new FJsonObject);
			AssistantMsg->SetStringField(TEXT("role"), TEXT("assistant"));
			AssistantMsg->SetArrayField(TEXT("content"), *Content);
			MessageHistory.Add(MakeShareable(new FJsonValueObject(AssistantMsg)));

			// Check if any content block is a tool use
			for (const TSharedPtr<FJsonValue>& ContentValue : *Content)
			{
				TSharedPtr<FJsonObject> ContentObj = ContentValue->AsObject();
				if (ContentObj.IsValid())
				{
					FString Type;
					if (ContentObj->TryGetStringField(TEXT("type"), Type) && Type == TEXT("tool_use"))
					{
						// Extract tool call info
						ContentObj->TryGetStringField(TEXT("id"), LastToolCallID);
						ContentObj->TryGetStringField(TEXT("name"), LastToolName);
					}
				}
			}
		}
	}

	// Return full response body (caller will check if it's a tool call)
	SuccessCallback.ExecuteIfBound(ResponseBody);
}

void ULLMIntegration::SendOpenAIRequestWithTools(const FLLMRequestConfig& Config)
{
	// OpenAI function calling implementation
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Config.APIKey));

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("model"), Config.ModelName.IsEmpty() ? TEXT("gpt-4") : Config.ModelName);
	JsonObject->SetNumberField(TEXT("temperature"), Config.Temperature);
	JsonObject->SetNumberField(TEXT("max_tokens"), Config.MaxTokens);

	// Messages
	TArray<TSharedPtr<FJsonValue>> Messages;

	// System message
	if (!Config.SystemPrompt.IsEmpty())
	{
		TSharedPtr<FJsonObject> SystemMsg = MakeShareable(new FJsonObject);
		SystemMsg->SetStringField(TEXT("role"), TEXT("system"));
		SystemMsg->SetStringField(TEXT("content"), Config.SystemPrompt);
		Messages.Add(MakeShareable(new FJsonValueObject(SystemMsg)));
	}

	// Add message history
	Messages.Append(MessageHistory);
	JsonObject->SetArrayField(TEXT("messages"), Messages);

	// Parse and add functions
	TSharedPtr<FJsonObject> ToolsObject;
	TSharedRef<TJsonReader<>> ToolsReader = TJsonReaderFactory<>::Create(CurrentToolDefinitionsJSON);
	if (FJsonSerializer::Deserialize(ToolsReader, ToolsObject))
	{
		const TArray<TSharedPtr<FJsonValue>>* FunctionsArray;
		if (ToolsObject->TryGetArrayField(TEXT("functions"), FunctionsArray))
		{
			JsonObject->SetArrayField(TEXT("functions"), *FunctionsArray);
		}
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(OutputString);
	Request->OnProcessRequestComplete().BindUObject(this, &ULLMIntegration::HandleOpenAIToolResponse);

	CurrentRequest = Request;
	Request->ProcessRequest();
}

void ULLMIntegration::HandleOpenAIToolResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		FailureCallback.ExecuteIfBound(TEXT("Request failed or invalid response"));
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		FString ErrorMsg = FString::Printf(TEXT("HTTP Error %d: %s"),
			Response->GetResponseCode(),
			*Response->GetContentAsString());
		FailureCallback.ExecuteIfBound(ErrorMsg);
		return;
	}

	FString ResponseBody = Response->GetContentAsString();
	SuccessCallback.ExecuteIfBound(ResponseBody);
}

void ULLMIntegration::SendToolResult(
	const FString& ToolName,
	const FString& ToolResult,
	const FLLMRequestConfig& Config,
	FOnLLMResponseReceived OnSuccess,
	FOnLLMRequestFailed OnFailure)
{
	SuccessCallback = OnSuccess;
	FailureCallback = OnFailure;
	CurrentConfig = Config;

	UE_LOG(LogTemp, Log, TEXT("Sending tool result for '%s' back to LLM"), *ToolName);

	if (Config.Provider == ELLMProvider::Anthropic)
	{
		// Add tool result message to history
		TSharedPtr<FJsonObject> ToolResultMsg = MakeShareable(new FJsonObject);
		ToolResultMsg->SetStringField(TEXT("role"), TEXT("user"));

		TArray<TSharedPtr<FJsonValue>> Content;
		TSharedPtr<FJsonObject> ToolResultContent = MakeShareable(new FJsonObject);
		ToolResultContent->SetStringField(TEXT("type"), TEXT("tool_result"));
		ToolResultContent->SetStringField(TEXT("tool_use_id"), LastToolCallID);
		ToolResultContent->SetStringField(TEXT("content"), ToolResult);
		Content.Add(MakeShareable(new FJsonValueObject(ToolResultContent)));

		ToolResultMsg->SetArrayField(TEXT("content"), Content);
		MessageHistory.Add(MakeShareable(new FJsonValueObject(ToolResultMsg)));

		// Continue conversation
		SendAnthropicRequestWithTools(Config);
	}
	else if (Config.Provider == ELLMProvider::OpenAI)
	{
		// OpenAI function result
		TSharedPtr<FJsonObject> FunctionMsg = MakeShareable(new FJsonObject);
		FunctionMsg->SetStringField(TEXT("role"), TEXT("function"));
		FunctionMsg->SetStringField(TEXT("name"), ToolName);
		FunctionMsg->SetStringField(TEXT("content"), ToolResult);
		MessageHistory.Add(MakeShareable(new FJsonValueObject(FunctionMsg)));

		SendOpenAIRequestWithTools(Config);
	}
	else
	{
		FailureCallback.ExecuteIfBound(TEXT("Tool use not supported for local models yet"));
	}
}

bool ULLMIntegration::IsToolCallResponse(const FString& Response, FString& OutToolName, FString& OutToolInput) const
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	// Check for Anthropic tool use
	const TArray<TSharedPtr<FJsonValue>>* Content;
	if (JsonObject->TryGetArrayField(TEXT("content"), Content))
	{
		for (const TSharedPtr<FJsonValue>& ContentValue : *Content)
		{
			TSharedPtr<FJsonObject> ContentObj = ContentValue->AsObject();
			if (ContentObj.IsValid())
			{
				FString Type;
				if (ContentObj->TryGetStringField(TEXT("type"), Type) && Type == TEXT("tool_use"))
				{
					ContentObj->TryGetStringField(TEXT("name"), OutToolName);

					const TSharedPtr<FJsonObject>* InputObj;
					if (ContentObj->TryGetObjectField(TEXT("input"), InputObj))
					{
						// Convert input object to JSON string
						FString InputString;
						TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&InputString);
						FJsonSerializer::Serialize((*InputObj).ToSharedRef(), Writer);
						OutToolInput = InputString;
					}

					return true;
				}
			}
		}
	}

	// Check for OpenAI function call
	const TArray<TSharedPtr<FJsonValue>>* Choices;
	if (JsonObject->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
	{
		TSharedPtr<FJsonObject> FirstChoice = (*Choices)[0]->AsObject();
		if (FirstChoice.IsValid())
		{
			const TSharedPtr<FJsonObject>* Message;
			if (FirstChoice->TryGetObjectField(TEXT("message"), Message))
			{
				const TSharedPtr<FJsonObject>* FunctionCall;
				if ((*Message)->TryGetObjectField(TEXT("function_call"), FunctionCall))
				{
					(*FunctionCall)->TryGetStringField(TEXT("name"), OutToolName);
					(*FunctionCall)->TryGetStringField(TEXT("arguments"), OutToolInput);
					return true;
				}
			}
		}
	}

	return false;
}
