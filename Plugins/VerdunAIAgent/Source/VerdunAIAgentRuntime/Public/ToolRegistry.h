// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ToolRegistry.generated.h"

/**
 * Parameter definition for a tool
 */
USTRUCT(BlueprintType)
struct FToolParameterDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Type; // "string", "number", "boolean", "array", "object"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	bool bRequired = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	TArray<FString> EnumValues; // For enum types

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	TMap<FString, FString> Properties; // For object types (property name -> type)
};

/**
 * Tool definition for function calling
 */
USTRUCT(BlueprintType)
struct FToolDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	TArray<FToolParameterDefinition> Parameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Category; // "terrain", "content", "inspection", "vision", etc.
};

/**
 * Result of a tool execution
 */
USTRUCT(BlueprintType)
struct FToolExecutionResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Result;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	TMap<FString, FString> OutputData; // Structured data return
};

DECLARE_DELEGATE_TwoParams(FOnToolExecuted, const FString& /* ToolName */, const FToolExecutionResult& /* Result */);

/**
 * Tool executor interface
 */
UCLASS(Abstract, Blueprintable)
class VERDUNAIAGENTRUNTIME_API UToolExecutor : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Execute a tool with given parameters
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Tool")
	FToolExecutionResult Execute(const TMap<FString, FString>& Parameters);
	virtual FToolExecutionResult Execute_Implementation(const TMap<FString, FString>& Parameters);

	/**
	 * Get tool definition
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Tool")
	FToolDefinition GetDefinition() const;
	virtual FToolDefinition GetDefinition_Implementation() const;
};

/**
 * Tool Registry - Central system for managing available tools
 * Enables native function calling with Anthropic Claude and OpenAI
 */
UCLASS(BlueprintType)
class VERDUNAIAGENTRUNTIME_API UToolRegistry : public UObject
{
	GENERATED_BODY()

public:
	UToolRegistry();

	/**
	 * Initialize with default UE5 tools
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	void InitializeDefaultTools(UWorld* World);

	/**
	 * Register a tool executor
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	void RegisterTool(const FString& ToolName, UToolExecutor* Executor);

	/**
	 * Unregister a tool
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	void UnregisterTool(const FString& ToolName);

	/**
	 * Get all registered tool definitions (for LLM function calling)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	TArray<FToolDefinition> GetAllToolDefinitions() const;

	/**
	 * Get tools by category
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	TArray<FToolDefinition> GetToolsByCategory(const FString& Category) const;

	/**
	 * Execute a tool by name with parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	FToolExecutionResult ExecuteTool(const FString& ToolName, const TMap<FString, FString>& Parameters);

	/**
	 * Check if tool exists
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	bool HasTool(const FString& ToolName) const;

	/**
	 * Generate JSON schema for all tools (Anthropic format)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	FString GenerateToolSchemaJSON() const;

	/**
	 * Generate OpenAI function calling format
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	FString GenerateOpenAIFunctionsJSON() const;

	/**
	 * Parse tool call from LLM response
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Tools")
	bool ParseToolCall(const FString& Response, FString& OutToolName, TMap<FString, FString>& OutParameters);

private:
	UPROPERTY()
	TMap<FString, UToolExecutor*> RegisteredTools;

	UPROPERTY()
	UWorld* WorldContext;

	// Helper functions
	FString ParameterTypeToJSON(const FString& Type) const;
	FString GenerateParameterSchema(const FToolParameterDefinition& Param) const;
};

/**
 * Built-in tool executors
 */

// Terrain manipulation tools
UCLASS()
class VERDUNAIAGENTRUNTIME_API UTerrainToolExecutor : public UToolExecutor
{
	GENERATED_BODY()

public:
	virtual FToolExecutionResult Execute_Implementation(const TMap<FString, FString>& Parameters) override;
	virtual FToolDefinition GetDefinition_Implementation() const override;

	UPROPERTY()
	UWorld* World;
};

// Content generation tools
UCLASS()
class VERDUNAIAGENTRUNTIME_API UContentGeneratorToolExecutor : public UToolExecutor
{
	GENERATED_BODY()

public:
	virtual FToolExecutionResult Execute_Implementation(const TMap<FString, FString>& Parameters) override;
	virtual FToolDefinition GetDefinition_Implementation() const override;

	UPROPERTY()
	UWorld* World;
};

// Level inspection tools
UCLASS()
class VERDUNAIAGENTRUNTIME_API ULevelInspectorToolExecutor : public UToolExecutor
{
	GENERATED_BODY()

public:
	virtual FToolExecutionResult Execute_Implementation(const TMap<FString, FString>& Parameters) override;
	virtual FToolDefinition GetDefinition_Implementation() const override;

	UPROPERTY()
	UWorld* World;
};

// Vision system tools
UCLASS()
class VERDUNAIAGENTRUNTIME_API UVisionToolExecutor : public UToolExecutor
{
	GENERATED_BODY()

public:
	virtual FToolExecutionResult Execute_Implementation(const TMap<FString, FString>& Parameters) override;
	virtual FToolDefinition GetDefinition_Implementation() const override;

	UPROPERTY()
	UWorld* World;
};

// Actor spawn/manipulation tools
UCLASS()
class VERDUNAIAGENTRUNTIME_API UActorManipulatorToolExecutor : public UToolExecutor
{
	GENERATED_BODY()

public:
	virtual FToolExecutionResult Execute_Implementation(const TMap<FString, FString>& Parameters) override;
	virtual FToolDefinition GetDefinition_Implementation() const override;

	UPROPERTY()
	UWorld* World;
};
