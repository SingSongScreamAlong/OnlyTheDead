// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "Dom/JsonObject.h"

/**
 * Handles communication with the Claude API
 */
class CLAUDETERMINAL_API FClaudeAPIClient
{
public:
	FClaudeAPIClient();
	~FClaudeAPIClient();

	// Callback for when a response is received
	DECLARE_DELEGATE_TwoParams(FOnResponseReceived, bool /*bSuccess*/, const FString& /*Response*/);

	/**
	 * Send a message to Claude
	 * @param Message The user's message
	 * @param OnResponseReceived Callback when response is ready
	 */
	void SendMessage(const FString& Message, FOnResponseReceived OnResponseReceived);

	/**
	 * Set the API key for authentication
	 * @param InAPIKey The Claude API key
	 */
	void SetAPIKey(const FString& InAPIKey);

	/**
	 * Clear the conversation history
	 */
	void ClearConversation();

	/**
	 * Set the system prompt that instructs Claude about UE5 commands
	 */
	void SetSystemPrompt(const FString& InSystemPrompt);

	/**
	 * Get the current conversation history
	 */
	const TArray<TSharedPtr<FJsonObject>>& GetConversationHistory() const { return ConversationHistory; }

private:
	// Handle HTTP response
	void OnResponseReceived_Internal(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnResponseReceived Callback);

	// Build the request JSON
	TSharedPtr<FJsonObject> BuildRequestJSON(const FString& Message) const;

	// API configuration
	FString APIKey;
	FString APIEndpoint;
	FString ModelName;
	FString SystemPrompt;

	// Conversation history
	TArray<TSharedPtr<FJsonObject>> ConversationHistory;

	// HTTP module
	FHttpModule* HttpModule;
};
