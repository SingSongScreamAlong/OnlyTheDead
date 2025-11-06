// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClaudeAPIClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FClaudeAPIClient::FClaudeAPIClient()
	: APIEndpoint(TEXT("https://api.anthropic.com/v1/messages"))
	, ModelName(TEXT("claude-sonnet-4-5-20250929"))
{
	HttpModule = &FHttpModule::Get();

	// Default system prompt for UE5 integration
	SystemPrompt = TEXT(
		"You are an AI assistant integrated into Unreal Engine 5. Your role is to help developers by:\n"
		"1. Interpreting natural language commands and converting them to structured JSON commands\n"
		"2. Providing helpful explanations and guidance\n"
		"3. Understanding UE5 concepts like Actors, Components, Blueprints, Materials, Landscapes, etc.\n\n"

		"When the user asks you to create or modify something in the world, respond with a JSON command in this format:\n"
		"```json\n"
		"{\n"
		"  \"command\": \"spawn_actor|create_road|place_model|modify_landscape|create_material|execute_blueprint\",\n"
		"  \"parameters\": {\n"
		"    // Command-specific parameters\n"
		"  }\n"
		"}\n"
		"```\n\n"

		"Available commands:\n"
		"- spawn_actor: Spawn an actor in the world\n"
		"  Parameters: class_name, location (x,y,z), rotation (pitch,yaw,roll), scale (x,y,z)\n"
		"- create_road: Create a procedural road/path\n"
		"  Parameters: start_location, end_location, width, material_path\n"
		"- place_model: Place a static mesh in the world\n"
		"  Parameters: mesh_path, location, rotation, scale\n"
		"- modify_landscape: Modify terrain\n"
		"  Parameters: operation (raise|lower|smooth|flatten), location, radius, strength\n"
		"- create_material: Create a new material instance\n"
		"  Parameters: base_material_path, instance_name, parameters\n"
		"- execute_blueprint: Call a blueprint function\n"
		"  Parameters: blueprint_path, function_name, arguments\n\n"

		"If the user's request is conversational or doesn't require a world operation, respond normally without JSON commands."
	);
}

FClaudeAPIClient::~FClaudeAPIClient()
{
}

void FClaudeAPIClient::SetAPIKey(const FString& InAPIKey)
{
	APIKey = InAPIKey;
}

void FClaudeAPIClient::SetSystemPrompt(const FString& InSystemPrompt)
{
	SystemPrompt = InSystemPrompt;
}

void FClaudeAPIClient::ClearConversation()
{
	ConversationHistory.Empty();
}

void FClaudeAPIClient::SendMessage(const FString& Message, FOnResponseReceived OnResponseReceived)
{
	if (APIKey.IsEmpty())
	{
		OnResponseReceived.ExecuteIfBound(false, TEXT("Error: API Key not set. Please configure your Claude API key in Project Settings > Claude Terminal."));
		return;
	}

	if (!HttpModule)
	{
		OnResponseReceived.ExecuteIfBound(false, TEXT("Error: HTTP module not available."));
		return;
	}

	// Create HTTP request
	TSharedRef<IHttpRequest> Request = HttpModule->CreateRequest();
	Request->SetVerb(TEXT("POST"));
	Request->SetURL(APIEndpoint);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-api-key"), APIKey);
	Request->SetHeader(TEXT("anthropic-version"), TEXT("2023-06-01"));

	// Build request JSON
	TSharedPtr<FJsonObject> RequestJSON = BuildRequestJSON(Message);
	if (!RequestJSON.IsValid())
	{
		OnResponseReceived.ExecuteIfBound(false, TEXT("Error: Failed to build request JSON."));
		return;
	}

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestJSON.ToSharedRef(), Writer);

	Request->SetContentAsString(RequestBody);

	// Bind response callback
	Request->OnProcessRequestComplete().BindRaw(this, &FClaudeAPIClient::OnResponseReceived_Internal, OnResponseReceived);

	// Send request
	if (!Request->ProcessRequest())
	{
		OnResponseReceived.ExecuteIfBound(false, TEXT("Error: Failed to send HTTP request."));
	}
}

TSharedPtr<FJsonObject> FClaudeAPIClient::BuildRequestJSON(const FString& Message) const
{
	TSharedPtr<FJsonObject> RequestObj = MakeShared<FJsonObject>();

	// Set model
	RequestObj->SetStringField(TEXT("model"), ModelName);

	// Set max tokens
	RequestObj->SetNumberField(TEXT("max_tokens"), 4096);

	// Set system prompt
	RequestObj->SetStringField(TEXT("system"), SystemPrompt);

	// Build messages array (conversation history + new message)
	TArray<TSharedPtr<FJsonValue>> MessagesArray;

	// Add conversation history
	for (const TSharedPtr<FJsonObject>& HistoryMessage : ConversationHistory)
	{
		MessagesArray.Add(MakeShared<FJsonValueObject>(HistoryMessage));
	}

	// Add new user message
	TSharedPtr<FJsonObject> UserMessage = MakeShared<FJsonObject>();
	UserMessage->SetStringField(TEXT("role"), TEXT("user"));

	TArray<TSharedPtr<FJsonValue>> ContentArray;
	TSharedPtr<FJsonObject> ContentObj = MakeShared<FJsonObject>();
	ContentObj->SetStringField(TEXT("type"), TEXT("text"));
	ContentObj->SetStringField(TEXT("text"), Message);
	ContentArray.Add(MakeShared<FJsonValueObject>(ContentObj));

	UserMessage->SetArrayField(TEXT("content"), ContentArray);
	MessagesArray.Add(MakeShared<FJsonValueObject>(UserMessage));

	RequestObj->SetArrayField(TEXT("messages"), MessagesArray);

	return RequestObj;
}

void FClaudeAPIClient::OnResponseReceived_Internal(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnResponseReceived Callback)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		Callback.ExecuteIfBound(false, TEXT("Error: Failed to receive response from Claude API."));
		return;
	}

	int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode != 200)
	{
		FString ErrorMessage = FString::Printf(TEXT("Error: Claude API returned status code %d: %s"),
			ResponseCode, *Response->GetContentAsString());
		Callback.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	// Parse response JSON
	FString ResponseBody = Response->GetContentAsString();
	TSharedPtr<FJsonObject> ResponseJSON;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, ResponseJSON) || !ResponseJSON.IsValid())
	{
		Callback.ExecuteIfBound(false, TEXT("Error: Failed to parse Claude API response."));
		return;
	}

	// Extract assistant's message
	const TArray<TSharedPtr<FJsonValue>>* ContentArray;
	if (!ResponseJSON->TryGetArrayField(TEXT("content"), ContentArray) || ContentArray->Num() == 0)
	{
		Callback.ExecuteIfBound(false, TEXT("Error: Invalid response format from Claude API."));
		return;
	}

	TSharedPtr<FJsonObject> ContentObj = (*ContentArray)[0]->AsObject();
	if (!ContentObj.IsValid())
	{
		Callback.ExecuteIfBound(false, TEXT("Error: Invalid content format from Claude API."));
		return;
	}

	FString AssistantMessage;
	if (!ContentObj->TryGetStringField(TEXT("text"), AssistantMessage))
	{
		Callback.ExecuteIfBound(false, TEXT("Error: Could not extract text from Claude API response."));
		return;
	}

	// Update conversation history
	// Add the user's message
	const TArray<TSharedPtr<FJsonValue>>* RequestMessagesArray;
	if (Request->GetContentAsString().Len() > 0)
	{
		TSharedPtr<FJsonObject> RequestJSON;
		TSharedRef<TJsonReader<>> RequestReader = TJsonReaderFactory<>::Create(Request->GetContentAsString());
		if (FJsonSerializer::Deserialize(RequestReader, RequestJSON) && RequestJSON.IsValid())
		{
			if (RequestJSON->TryGetArrayField(TEXT("messages"), RequestMessagesArray) && RequestMessagesArray->Num() > 0)
			{
				// Add only the last message (the new user message)
				TSharedPtr<FJsonObject> LastMessage = (*RequestMessagesArray)[RequestMessagesArray->Num() - 1]->AsObject();
				if (LastMessage.IsValid())
				{
					ConversationHistory.Add(LastMessage);
				}
			}
		}
	}

	// Add assistant's response
	TSharedPtr<FJsonObject> AssistantMessageObj = MakeShared<FJsonObject>();
	AssistantMessageObj->SetStringField(TEXT("role"), TEXT("assistant"));

	TArray<TSharedPtr<FJsonValue>> AssistantContentArray;
	TSharedPtr<FJsonObject> AssistantContentObj = MakeShared<FJsonObject>();
	AssistantContentObj->SetStringField(TEXT("type"), TEXT("text"));
	AssistantContentObj->SetStringField(TEXT("text"), AssistantMessage);
	AssistantContentArray.Add(MakeShared<FJsonValueObject>(AssistantContentObj));

	AssistantMessageObj->SetArrayField(TEXT("content"), AssistantContentArray);
	ConversationHistory.Add(AssistantMessageObj);

	// Return success with the assistant's message
	Callback.ExecuteIfBound(true, AssistantMessage);
}
