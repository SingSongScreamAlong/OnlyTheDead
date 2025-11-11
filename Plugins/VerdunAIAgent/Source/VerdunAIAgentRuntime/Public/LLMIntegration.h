// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Http.h"
#include "LLMIntegration.generated.h"

// Forward declarations
struct FToolDefinition;
struct FToolExecutionResult;

DECLARE_DELEGATE_OneParam(FOnLLMResponseReceived, const FString& /* Response */);
DECLARE_DELEGATE_OneParam(FOnLLMRequestFailed, const FString& /* ErrorMessage */);
DECLARE_DELEGATE_TwoParams(FOnToolCallRequested, const FString& /* ToolName */, const TMap<FString, FString>& /* Parameters */);

/**
 * Supported LLM Providers
 */
UENUM(BlueprintType)
enum class ELLMProvider : uint8
{
	OpenAI UMETA(DisplayName = "OpenAI (GPT-4, GPT-3.5)"),
	Anthropic UMETA(DisplayName = "Anthropic (Claude)"),
	Local UMETA(DisplayName = "Local Model (Ollama, LM Studio)")
};

/**
 * LLM Request Configuration
 */
USTRUCT(BlueprintType)
struct FLLMRequestConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	ELLMProvider Provider = ELLMProvider::Anthropic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString APIKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString ModelName = TEXT("claude-3-5-sonnet-20241022");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString SystemPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	float Temperature = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	int32 MaxTokens = 4000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	FString BaseURL; // For local models

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Agent")
	bool bEnableToolUse = false; // Enable native function calling
};

/**
 * LLM Integration - Handles communication with various LLM APIs
 */
UCLASS(BlueprintType)
class VERDUNAIAGENTRUNTIME_API ULLMIntegration : public UObject
{
	GENERATED_BODY()

public:
	ULLMIntegration();

	/**
	 * Send a request to the configured LLM
	 * @param UserMessage - The user's message/prompt
	 * @param Config - Configuration for the LLM request
	 * @param OnSuccess - Callback when response is received
	 * @param OnFailure - Callback when request fails
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|LLM")
	void SendRequest(
		const FString& UserMessage,
		const FLLMRequestConfig& Config,
		FOnLLMResponseReceived OnSuccess,
		FOnLLMRequestFailed OnFailure
	);

	/**
	 * Send request with tool definitions (enables native function calling)
	 * @param UserMessage - The user's message/prompt
	 * @param Config - Configuration for the LLM request
	 * @param ToolDefinitionsJSON - JSON array of tool definitions
	 * @param OnSuccess - Callback when response is received
	 * @param OnFailure - Callback when request fails
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|LLM")
	void SendRequestWithTools(
		const FString& UserMessage,
		const FLLMRequestConfig& Config,
		const FString& ToolDefinitionsJSON,
		FOnLLMResponseReceived OnSuccess,
		FOnLLMRequestFailed OnFailure
	);

	/**
	 * Continue conversation after tool execution
	 * @param ToolName - Name of the tool that was executed
	 * @param ToolResult - Result from tool execution
	 * @param Config - Configuration for the LLM request
	 * @param OnSuccess - Callback when response is received
	 * @param OnFailure - Callback when request fails
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|LLM")
	void SendToolResult(
		const FString& ToolName,
		const FString& ToolResult,
		const FLLMRequestConfig& Config,
		FOnLLMResponseReceived OnSuccess,
		FOnLLMRequestFailed OnFailure
	);

	/**
	 * Check if the last response requested a tool call
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|LLM")
	bool IsToolCallResponse(const FString& Response, FString& OutToolName, FString& OutToolInput) const;

	/**
	 * Send a streaming request (for real-time responses)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|LLM")
	void SendStreamingRequest(
		const FString& UserMessage,
		const FLLMRequestConfig& Config,
		FOnLLMResponseReceived OnChunkReceived,
		FOnLLMRequestFailed OnFailure
	);

	/**
	 * Cancel any ongoing requests
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|LLM")
	void CancelRequest();

private:
	// HTTP request handling
	void SendOpenAIRequest(const FString& UserMessage, const FLLMRequestConfig& Config);
	void SendAnthropicRequest(const FString& UserMessage, const FLLMRequestConfig& Config);
	void SendLocalRequest(const FString& UserMessage, const FLLMRequestConfig& Config);

	// Tool use request handling
	void SendAnthropicRequestWithTools(const FLLMRequestConfig& Config);
	void SendOpenAIRequestWithTools(const FLLMRequestConfig& Config);

	// Response handlers
	void HandleOpenAIResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleAnthropicResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleLocalResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// Tool use response handlers
	void HandleAnthropicToolResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleOpenAIToolResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// Helper functions
	FString BuildOpenAIPayload(const FString& UserMessage, const FLLMRequestConfig& Config);
	FString BuildAnthropicPayload(const FString& UserMessage, const FLLMRequestConfig& Config);
	FString BuildLocalPayload(const FString& UserMessage, const FLLMRequestConfig& Config);

	FString ExtractMessageFromOpenAIResponse(const FString& ResponseBody);
	FString ExtractMessageFromAnthropicResponse(const FString& ResponseBody);
	FString ExtractMessageFromLocalResponse(const FString& ResponseBody);

	// Member variables
	TSharedPtr<IHttpRequest> CurrentRequest;
	FOnLLMResponseReceived SuccessCallback;
	FOnLLMRequestFailed FailureCallback;
	FLLMRequestConfig CurrentConfig;

	// Conversation history (for tool use multi-turn)
	TArray<TSharedPtr<FJsonValue>> MessageHistory;

	// Tool definitions
	FString CurrentToolDefinitionsJSON;

	// Last tool call tracking
	FString LastToolCallID;
	FString LastToolName;
};
