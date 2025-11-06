// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClaudeAPIClient.h"
#include "ClaudeTerminalSettings.h"
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
		"  \"command\": \"command_name\",\n"
		"  \"parameters\": {\n"
		"    // Command-specific parameters\n"
		"  }\n"
		"}\n"
		"```\n\n"

		"Available commands:\n"
		"- spawn_actor: Spawn an actor\n"
		"  Parameters: class_name, location {x,y,z}, rotation {pitch,yaw,roll}, scale {x,y,z}\n"
		"- create_road: Create spline road\n"
		"  Parameters: start_location {x,y,z}, end_location {x,y,z}, width, material_path\n"
		"- place_model: Place static mesh\n"
		"  Parameters: mesh_path, location {x,y,z}, rotation {pitch,yaw,roll}, scale {x,y,z}\n"
		"- create_material: Create material instance\n"
		"  Parameters: base_material_path, instance_name, parameters\n\n"

		"LANDSCAPE & TERRAIN COMMANDS:\n"
		"- create_landscape: Create landscape from heightmap file\n"
		"  Parameters: heightmap_path, location {x,y,z}, scale {x,y,z}, material_path (optional)\n"
		"- import_terrain: Import GPS-referenced terrain (SRTM data)\n"
		"  Parameters: heightmap_path, gps_latitude, gps_longitude, meters_per_pixel, material_path (optional)\n"
		"- modify_terrain: Modify existing landscape\n"
		"  Parameters: operation (raise|lower|smooth|flatten), location {x,y,z}, radius, strength\n"
		"- create_crater: Create shell crater\n"
		"  Parameters: location {x,y,z}, radius, depth\n"
		"- create_trench: Carve trench line\n"
		"  Parameters: start_location {x,y,z}, end_location {x,y,z}, width, depth\n\n"

		"SPECIAL NOTE - This is for a WWI Verdun game:\n"
		"- User has real SRTM elevation data for Verdun battlefield\n"
		"- GPS coordinates can be converted: Fort Douaumont is at 49.1597°N, 5.4267°E\n"
		"- When user mentions 'Verdun terrain', 'SRTM data', or GPS coordinates, use import_terrain\n"
		"- Historical trenches should be ~2m deep, 2m wide\n"
		"- Shell craters typically 5-15m radius, 2-5m deep\n\n"

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
	// Get settings
	const UClaudeTerminalSettings* Settings = GetDefault<UClaudeTerminalSettings>();
	if (!Settings)
	{
		OnResponseReceived.ExecuteIfBound(false, TEXT("Error: Could not load Claude Terminal settings."));
		return;
	}

	// Use settings values or fallback to instance values
	FString CurrentAPIKey = Settings->APIKey.IsEmpty() ? APIKey : Settings->APIKey;
	FString CurrentEndpoint = Settings->APIEndpoint.IsEmpty() ? APIEndpoint : Settings->APIEndpoint;
	FString CurrentModel = Settings->ModelName.IsEmpty() ? ModelName : Settings->ModelName;

	if (CurrentAPIKey.IsEmpty())
	{
		OnResponseReceived.ExecuteIfBound(false, TEXT("Error: API Key not set. Please configure your Claude API key in Project Settings > Plugins > Claude Terminal."));
		return;
	}

	if (!HttpModule)
	{
		OnResponseReceived.ExecuteIfBound(false, TEXT("Error: HTTP module not available."));
		return;
	}

	// Update instance variables
	APIKey = CurrentAPIKey;
	APIEndpoint = CurrentEndpoint;
	ModelName = CurrentModel;

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
