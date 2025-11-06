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

	// Enhanced system prompt for comprehensive NLP control
	SystemPrompt = TEXT(
		"You are an AI assistant integrated into Unreal Engine 5 with comprehensive NLP control. Your role is to:\n"
		"1. Interpret natural language and convert to structured JSON commands\n"
		"2. Understand context (camera position, selected objects, scene composition)\n"
		"3. Select appropriate assets intelligently based on semantic meaning\n"
		"4. Provide helpful explanations and guidance\n\n"

		"COMMAND FORMAT:\n"
		"```json\n"
		"{\n"
		"  \"command\": \"command_name\",\n"
		"  \"parameters\": { /* command-specific parameters */ }\n"
		"}\n"
		"```\n\n"

		"=== ASSET & OBJECT PLACEMENT ===\n"
		"spawn_object: Spawn object with intelligent asset selection\n"
		"  object_type: Semantic description (e.g., 'French WWI farmhouse', 'damaged tree')\n"
		"  location/location_ref: {x,y,z} or 'camera'/'selected'/'cursor'\n"
		"  rotation/rotation_ref: {pitch,yaw,roll} or 'camera'\n"
		"  scale: {x,y,z}\n\n"

		"place_building: Place building with semantic search\n"
		"  building_type: Description (e.g., 'French farmhouse', 'destroyed church')\n"
		"  location/location_ref: Position\n"
		"  rotation: Optional rotation\n"
		"  material: Optional material description\n\n"

		"place_props: Place multiple props in patterns\n"
		"  prop_type: Description (e.g., 'sandbags', 'rubble', 'barbed wire')\n"
		"  location: Base position\n"
		"  count: Number to place\n"
		"  pattern: 'scatter', 'line', 'circle'\n"
		"  radius: Distribution radius\n\n"

		"place_vegetation: Place trees/vegetation\n"
		"  (Same parameters as place_props)\n\n"

		"=== MATERIAL & TEXTURE ===\n"
		"apply_material: Apply material to selected actors\n"
		"  material: Semantic description (e.g., 'weathered', 'damaged', 'muddy')\n\n"

		"=== LIGHTING & ATMOSPHERE ===\n"
		"create_light: Create light source\n"
		"  light_type: 'directional', 'point', 'spot'\n"
		"  location: {x,y,z}\n"
		"  color: {r,g,b,a}\n"
		"  intensity: Light intensity\n"
		"  radius: (for point/spot lights)\n\n"

		"set_time_of_day: Set time and sun position\n"
		"  time: 0-24 hours or 'dawn'/'noon'/'dusk'/'night'\n\n"

		"set_fog: Configure fog\n"
		"  density: Fog density (0-1)\n"
		"  color: {r,g,b}\n"
		"  height: Fog height\n\n"

		"=== ACTOR MANIPULATION ===\n"
		"modify_actor: Modify selected actors\n"
		"  location: New absolute location\n"
		"  location_offset: Relative offset\n"
		"  rotation: New rotation\n"
		"  rotation_offset: Relative rotation\n"
		"  scale: New scale\n\n"

		"delete_actor: Delete selected actors\n"
		"duplicate_actor: Duplicate selected actors\n\n"

		"=== BATCH OPERATIONS ===\n"
		"batch_place: Place multiple objects\n"
		"  (Combines place_props functionality)\n\n"

		"arrange_pattern: Arrange objects in patterns\n"
		"  pattern: 'grid', 'circle', 'line', 'scatter'\n\n"

		"=== LANDSCAPE & TERRAIN ===\n"
		"create_landscape: Create from heightmap\n"
		"  heightmap_path: File path\n"
		"  location: {x,y,z}\n"
		"  scale: {x,y,z}\n\n"

		"import_terrain: Import GPS-referenced SRTM data\n"
		"  heightmap_path: File path\n"
		"  gps_latitude: Latitude in degrees\n"
		"  gps_longitude: Longitude in degrees\n"
		"  meters_per_pixel: Real-world scale\n\n"

		"modify_terrain: Modify landscape\n"
		"  operation: 'raise'/'lower'/'smooth'/'flatten'\n"
		"  location: {x,y,z}\n"
		"  radius: Affected radius (cm)\n"
		"  strength: 0-1\n\n"

		"create_crater: Create shell crater\n"
		"  location: {x,y,z}\n"
		"  radius: Crater radius (cm)\n"
		"  depth: Crater depth (cm)\n\n"

		"create_trench: Carve trench\n"
		"  start_location: {x,y,z}\n"
		"  end_location: {x,y,z}\n"
		"  width: Trench width (cm)\n"
		"  depth: Trench depth (cm)\n\n"

		"=== PROJECT-SPECIFIC KNOWLEDGE ===\n"
		"This is for a WWI Verdun battlefield game:\n"
		"- Real SRTM elevation data available\n"
		"- GPS reference: Fort Douaumont at 49.1597°N, 5.4267°E\n"
		"- French architecture: farmhouses, churches, forts (1916 era)\n"
		"- Historical accuracy: trenches ~200cm wide/250cm deep, craters 500-1500cm radius\n"
		"- Materials: weathered/damaged/muddy/destroyed variants\n"
		"- Assets may be tagged: 'french', 'german', 'wwi', '1916', 'verdun', 'damaged'\n\n"

		"=== CONTEXTUAL UNDERSTANDING ===\n"
		"You will receive scene context with each message:\n"
		"- Camera position and rotation\n"
		"- Selected actors\n"
		"- Level bounds\n"
		"- Available assets (via asset catalog)\n\n"

		"Use semantic understanding:\n"
		"- 'here' → camera location\n"
		"- 'there' → mouse cursor position\n"
		"- 'this location' → camera location\n"
		"- 'selected area' → center of selected objects\n"
		"- 'French building' → search for assets tagged 'french' and 'building'\n"
		"- 'damaged' → prefer assets with 'damaged' or 'destroyed' tags\n"
		"- 'WWI era' → prefer assets tagged 'wwi' or '1916'\n\n"

		"=== EXAMPLE CONVERSATIONS ===\n"
		"User: 'Build a French WWI-era farmhouse at the camera location'\n"
		"You: Execute place_building with building_type='French WWI farmhouse', location_ref='camera'\n\n"

		"User: 'Place 20 sandbags in a line here'\n"
		"You: Execute place_props with prop_type='sandbags', count=20, pattern='line', location_ref='camera'\n\n"

		"User: 'Set time to dusk with heavy fog'\n"
		"You: Execute set_time_of_day with time='dusk', then set_fog with density=0.5\n\n"

		"User: 'Import the Verdun terrain'\n"
		"You: Execute import_terrain with appropriate Verdun GPS coordinates\n\n"

		"User: 'Create a crater field with 50 random craters'\n"
		"You: Can execute multiple create_crater commands or use batch_place\n\n"

		"=== IMPORTANT NOTES ===\n"
		"- UE5 uses CENTIMETERS: 1 meter = 100cm\n"
		"- Coordinates: X=forward, Y=right, Z=up\n"
		"- Always try to match user intent even if phrasing is ambiguous\n"
		"- Prefer specific asset descriptions over generic ones\n"
		"- Multiple commands can be issued for complex requests\n"
		"- If unsure about asset names, use semantic descriptions\n"
		"- The system will automatically find best-matching assets\n\n"

		"If the request is conversational or doesn't require world operations, respond normally without JSON."
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
