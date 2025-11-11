# Phase 4: Native Tool Use & Autonomous Agents

## Overview

Phase 4 transforms the Verdun AI Agent from a JSON-based planning system to a **native tool calling system** with **autonomous error recovery**. This enables the LLM to directly invoke UE5 functions with type-safe parameters and self-correct when operations fail.

---

## Architecture

### Core Components

#### 1. **ToolRegistry** (`ToolRegistry.h/cpp`)
Central registry for all available tools with dynamic registration.

**Key Features:**
- Tool definition schema (name, description, parameters)
- Type-safe parameter definitions
- Tool executor interface
- JSON schema generation (Anthropic & OpenAI formats)
- Centralized tool execution with error handling

**Built-in Tool Executors:**
- `UTerrainToolExecutor` - Terrain modification operations
- `UContentGeneratorToolExecutor` - Procedural content generation
- `ULevelInspectorToolExecutor` - Scene inspection and queries
- `UVisionToolExecutor` - Screenshot capture and analysis
- `UActorManipulatorToolExecutor` - Actor spawn/manipulation

**Example Tool Definition:**
```cpp
FToolDefinition TerrainTool;
TerrainTool.Name = TEXT("modify_terrain");
TerrainTool.Description = TEXT("Modify terrain landscape by raising, lowering, creating craters, or excavating trenches");
TerrainTool.Category = TEXT("terrain");

FToolParameterDefinition OpParam;
OpParam.Name = TEXT("operation");
OpParam.Type = TEXT("string");
OpParam.Description = TEXT("Type of terrain operation to perform");
OpParam.bRequired = true;
OpParam.EnumValues = {TEXT("raise"), TEXT("lower"), TEXT("crater"), TEXT("trench")};
TerrainTool.Parameters.Add(OpParam);
```

#### 2. **LLMIntegration Tool Calling** (`LLMIntegration.h/cpp`)
Extended to support native function calling for Anthropic and OpenAI.

**New Methods:**
```cpp
// Send request with tool definitions
void SendRequestWithTools(
    const FString& UserMessage,
    const FLLMRequestConfig& Config,
    const FString& ToolDefinitionsJSON,
    FOnLLMResponseReceived OnSuccess,
    FOnLLMRequestFailed OnFailure
);

// Continue conversation after tool execution
void SendToolResult(
    const FString& ToolName,
    const FString& ToolResult,
    const FLLMRequestConfig& Config,
    FOnLLMResponseReceived OnSuccess,
    FOnLLMRequestFailed OnFailure
);

// Check if response is a tool call
bool IsToolCallResponse(const FString& Response, FString& OutToolName, FString& OutToolInput) const;
```

**Features:**
- Multi-turn conversation tracking
- Automatic message history management
- Tool result formatting for both Anthropic and OpenAI
- Tool call ID tracking for correct response matching

#### 3. **AIAgentCore Autonomous Execution** (`AIAgentCore.h/cpp`)
Complete rewrite of execution system to use tool calling.

**New Execution Flow:**
```
User Command → StartToolCallingExecution()
    ↓
SendRequestWithTools() [with all tool definitions]
    ↓
OnLLMResponseReceived()
    ↓
   [Tool call?] YES → ProcessToolCall()
    |                      ↓
    |                 ExecuteTool()
    |                      ↓
    |          OnToolExecutionComplete()
    |                      ↓
    |            [Success?] YES → SendToolResult() [back to LLM]
    |                      |                ↓
    |                      |        (Loop continues...)
    |                      |
    |                     NO → [Autonomous mode?]
    |                              YES → Retry with error feedback
    |                              NO → Fail task
    ↓
   NO → Task Complete
```

**Autonomous Features:**
- **Automatic retry** on tool failures (up to 3 attempts)
- **Error feedback loop** - LLM receives error details and tries different approach
- **Self-verification** - Agent can check its own work
- **Iterative improvement** - Can make adjustments based on results

**Configuration:**
```cpp
// Initialize with world context and tool registry
AgentCore->Initialize(Config, World);

// Enable autonomous mode for self-correction
AgentCore->SetAutonomousMode(true);

// Execute command (uses tool calling automatically)
AgentCore->ExecuteCommand(TEXT("Create a trench from 0,0,0 to 1000,0,0"));
```

---

## Tool Execution Flow

### 1. **Tool Registration** (Initialization)
```cpp
// ToolRegistry initialization
UToolRegistry* Registry = NewObject<UToolRegistry>();
Registry->InitializeDefaultTools(World);

// Generates JSON schema automatically:
{
  "tools": [
    {
      "name": "modify_terrain",
      "description": "Modify terrain landscape...",
      "input_schema": {
        "type": "object",
        "properties": {
          "operation": {
            "type": "string",
            "description": "Type of terrain operation",
            "enum": ["raise", "lower", "crater", "trench"]
          },
          "location": {
            "type": "string",
            "description": "Location in world space (format: 'x,y,z')"
          },
          ...
        },
        "required": ["operation", "location"]
      }
    }
  ]
}
```

### 2. **LLM Tool Call** (Agent Decision)
LLM receives tools and decides to use one:
```json
{
  "content": [
    {
      "type": "tool_use",
      "id": "toolu_01A2B3C4D5",
      "name": "modify_terrain",
      "input": {
        "operation": "crater",
        "location": "500,500,100",
        "diameter": 800,
        "depth": 150
      }
    }
  ]
}
```

### 3. **Tool Execution** (UE5 API Call)
```cpp
// Parse parameters from JSON
TMap<FString, FString> Parameters;
Parameters.Add(TEXT("operation"), TEXT("crater"));
Parameters.Add(TEXT("location"), TEXT("500,500,100"));
Parameters.Add(TEXT("diameter"), TEXT("800"));
Parameters.Add(TEXT("depth"), TEXT("150"));

// Execute tool
FToolExecutionResult Result = ToolRegistry->ExecuteTool(TEXT("modify_terrain"), Parameters);

// Result:
Result.bSuccess = true;
Result.Result = "Created crater at (500, 500, 100) with diameter 800cm and depth 150cm";
```

### 4. **Result Feedback** (Back to LLM)
```json
{
  "role": "user",
  "content": [
    {
      "type": "tool_result",
      "tool_use_id": "toolu_01A2B3C4D5",
      "content": "Created crater at (500, 500, 100) with diameter 800cm and depth 150cm"
    }
  ]
}
```

### 5. **Autonomous Retry** (On Failure)
```cpp
// If tool fails:
Result.bSuccess = false;
Result.ErrorMessage = "Landscape not found in scene";

// Agent receives error and tries different approach:
{
  "role": "user",
  "content": [
    {
      "type": "tool_result",
      "tool_use_id": "toolu_01A2B3C4D5",
      "content": "Error: Landscape not found in scene. Please try a different approach."
    }
  ]
}

// LLM might respond with:
{
  "type": "tool_use",
  "name": "inspect_level",  // Different tool!
  "input": {
    "query_type": "summary"  // Check what's in the scene first
  }
}
```

---

## Key Improvements Over Phase 1-3

| Feature | Phase 1-3 (JSON) | Phase 4 (Tool Calling) |
|---------|------------------|------------------------|
| **Planning** | LLM generates full plan upfront | Iterative - decides next step based on results |
| **Error Handling** | Fails entire task | Autonomous retry with alternative approaches |
| **Type Safety** | String parameters only | Typed parameters (string, number, enum, object) |
| **Conversation** | One-shot | Multi-turn with context |
| **Verification** | Manual | Automatic with self-correction |
| **Flexibility** | Rigid plan execution | Adaptive based on real-time feedback |
| **Tool Discovery** | Hardcoded commands | Dynamic tool schema |

---

## Usage Examples

### Example 1: Simple Terrain Modification
```cpp
// User command
"Create a crater at location 1000, 2000, 50"

// Tool call sequence:
1. modify_terrain(operation="crater", location="1000,2000,50")
   → Success: "Created crater..."

// Agent: "I've created a crater at the specified location."
```

### Example 2: Complex Multi-Tool Task
```cpp
// User command
"Build a defensive trench line running north-south through the middle of the map"

// Tool call sequence:
1. inspect_level(query_type="summary")
   → Success: "Map dimensions: 10000x10000..."

2. generate_content(content_type="trench_system",
                    path="5000,0,0;5000,5000,0;5000,10000,0",
                    width=150, depth=200)
   → Success: "Generated trench system with 2 segments"

3. modify_terrain(operation="trench",
                  path="5000,0,0;5000,10000,0",
                  width=150, depth=200)
   → Success: "Excavated trench path..."

// Agent: "I've built a defensive trench line running north-south..."
```

### Example 3: Autonomous Error Recovery
```cpp
// User command
"Create barbed wire obstacles around coordinates 3000, 3000"

// Tool call sequence:
1. generate_content(content_type="barbed_wire",
                    path="3000,3000,0;4000,3000,0",  // Bad path
                    rows=4)
   → FAIL: "Path must have at least 3 points"

   [Autonomous Retry - LLM tries different approach]

2. generate_content(content_type="barbed_wire",
                    path="2800,2800,0;3000,3200,0;3200,2800,0;3000,2800,0",
                    rows=4)
   → Success: "Generated barbed wire obstacle with 4 rows"

// Agent: "I've created barbed wire obstacles in a defensive perimeter..."
```

---

## Configuration Options

### Enable/Disable Tool Calling
```cpp
// In AIAgentCore initialization
bUseToolCalling = true;  // Use Phase 4 native tool calling
bUseToolCalling = false; // Fall back to Phase 1-3 JSON planning
```

### Autonomous Mode Settings
```cpp
// Enable self-correction
AgentCore->SetAutonomousMode(true);

// Configure retry limits
MaxRetries = 3; // Default: 3 attempts
```

### LLM Configuration
```cpp
FLLMRequestConfig Config;
Config.Provider = ELLMProvider::Anthropic;
Config.ModelName = TEXT("claude-3-5-sonnet-20241022");
Config.bEnableToolUse = true; // Must be true for tool calling
Config.Temperature = 0.7f;
Config.MaxTokens = 4000;
```

---

## Tool Executor Interface

### Creating Custom Tools

```cpp
// 1. Define tool executor
UCLASS()
class UMyCustomToolExecutor : public UToolExecutor
{
    GENERATED_BODY()

public:
    virtual FToolExecutionResult Execute_Implementation(const TMap<FString, FString>& Parameters) override
    {
        FToolExecutionResult Result;

        // Your custom logic here
        const FString* Param1 = Parameters.Find(TEXT("my_param"));
        if (!Param1)
        {
            Result.bSuccess = false;
            Result.ErrorMessage = TEXT("Missing required parameter: my_param");
            return Result;
        }

        // Do work...
        Result.bSuccess = true;
        Result.Result = TEXT("Custom tool executed successfully");
        return Result;
    }

    virtual FToolDefinition GetDefinition_Implementation() const override
    {
        FToolDefinition Def;
        Def.Name = TEXT("my_custom_tool");
        Def.Description = TEXT("Does something custom");
        Def.Category = TEXT("custom");

        FToolParameterDefinition Param;
        Param.Name = TEXT("my_param");
        Param.Type = TEXT("string");
        Param.Description = TEXT("A parameter for my tool");
        Param.bRequired = true;
        Def.Parameters.Add(Param);

        return Def;
    }
};

// 2. Register tool
UMyCustomToolExecutor* CustomExecutor = NewObject<UMyCustomToolExecutor>();
ToolRegistry->RegisterTool(TEXT("my_custom_tool"), CustomExecutor);
```

---

## Performance Characteristics

### Tool Registry
- **Registration**: O(1) - Hash map lookup
- **Schema Generation**: O(n) - Linear in number of tools
- **Tool Execution**: O(1) - Direct function call

### LLM Communication
- **Initial Request**: 1 API call with tool definitions
- **Tool Calls**: 1 API call per tool execution
- **Multi-turn**: Maintains conversation history automatically
- **Average Task**: 2-5 API calls (inspect → execute → verify)

### Error Recovery
- **First Attempt**: Immediate execution
- **Retry**: 2-second delay between attempts
- **Max Retries**: 3 (configurable)
- **Fallback**: Graceful failure after max retries

---

## Testing & Validation

### Unit Tests (Recommended)
```cpp
// Test tool registration
void TestToolRegistration()
{
    UToolRegistry* Registry = NewObject<UToolRegistry>();
    Registry->InitializeDefaultTools(World);

    check(Registry->HasTool(TEXT("modify_terrain")));
    check(Registry->HasTool(TEXT("generate_content")));
    check(Registry->HasTool(TEXT("inspect_level")));
}

// Test tool execution
void TestToolExecution()
{
    TMap<FString, FString> Params;
    Params.Add(TEXT("operation"), TEXT("raise"));
    Params.Add(TEXT("location"), TEXT("100,100,0"));
    Params.Add(TEXT("radius"), TEXT("500"));
    Params.Add(TEXT("strength"), TEXT("1.0"));

    FToolExecutionResult Result = Registry->ExecuteTool(TEXT("modify_terrain"), Params);
    check(Result.bSuccess);
}

// Test autonomous retry
void TestAutonomousRetry()
{
    AgentCore->SetAutonomousMode(true);
    AgentCore->ExecuteCommand(TEXT("Invalid command that will fail"));

    // Should retry up to 3 times before giving up
    check(RetryCount <= MaxRetries);
}
```

---

## Debugging

### Enable Verbose Logging
```cpp
// In LLMIntegration.cpp
UE_LOG(LogTemp, Log, TEXT("Sending tool call: %s"), *ToolName);
UE_LOG(LogTemp, Log, TEXT("Tool parameters: %s"), *ParametersJSON);

// In ToolRegistry.cpp
UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Executing tool '%s' with %d parameters"), *ToolName, Parameters.Num());

// In AIAgentCore.cpp
UE_LOG(LogTemp, Log, TEXT("[AIAgent] Tool '%s' executed successfully: %s"), *ToolName, *Result.Result);
```

### Common Issues

**Issue**: "Tool not found in registry"
- **Cause**: ToolRegistry not initialized
- **Fix**: Call `ToolRegistry->InitializeDefaultTools(World)` before use

**Issue**: "Tool call parsing failed"
- **Cause**: LLM response format mismatch
- **Fix**: Verify LLM provider (Anthropic vs OpenAI) and API version

**Issue**: "Autonomous retry not working"
- **Cause**: Autonomous mode disabled
- **Fix**: Call `AgentCore->SetAutonomousMode(true)`

---

## Future Enhancements (Phase 5 Candidates)

1. **Parallel Tool Execution**
   - Execute independent tools concurrently
   - Dependency graph resolution

2. **Tool Composition**
   - Combine multiple tools into macros
   - Higher-level composite operations

3. **Learning & Adaptation**
   - Track tool success rates
   - Prefer tools that work well for specific tasks

4. **Visual Feedback Loop**
   - Automatic screenshot capture after operations
   - Vision model verification of results

5. **Blueprint Tool Generator**
   - Generate Blueprint nodes from tool definitions
   - Visual tool composition

---

## API Reference

### ToolRegistry

```cpp
// Initialize default tools
void InitializeDefaultTools(UWorld* World);

// Register custom tool
void RegisterTool(const FString& ToolName, UToolExecutor* Executor);

// Execute tool
FToolExecutionResult ExecuteTool(const FString& ToolName, const TMap<FString, FString>& Parameters);

// Get tool schema
FString GenerateToolSchemaJSON() const; // Anthropic format
FString GenerateOpenAIFunctionsJSON() const; // OpenAI format

// Query tools
bool HasTool(const FString& ToolName) const;
TArray<FToolDefinition> GetAllToolDefinitions() const;
TArray<FToolDefinition> GetToolsByCategory(const FString& Category) const;
```

### LLMIntegration (Tool Calling)

```cpp
// Send request with tools
void SendRequestWithTools(
    const FString& UserMessage,
    const FLLMRequestConfig& Config,
    const FString& ToolDefinitionsJSON,
    FOnLLMResponseReceived OnSuccess,
    FOnLLMRequestFailed OnFailure
);

// Send tool result
void SendToolResult(
    const FString& ToolName,
    const FString& ToolResult,
    const FLLMRequestConfig& Config,
    FOnLLMResponseReceived OnSuccess,
    FOnLLMRequestFailed OnFailure
);

// Check for tool call
bool IsToolCallResponse(const FString& Response, FString& OutToolName, FString& OutToolInput) const;
```

### AIAgentCore (Autonomous)

```cpp
// Initialize with world and tool support
void Initialize(const FLLMRequestConfig& InConfig, UWorld* World);

// Execute command (uses tool calling)
void ExecuteCommand(const FString& Command);

// Enable autonomous mode
void SetAutonomousMode(bool bEnabled);

// Stop execution
void StopExecution();
```

---

## Summary

Phase 4 delivers a **production-ready autonomous agent system** with:

✅ Native tool calling (Anthropic & OpenAI)
✅ Type-safe parameter definitions
✅ Autonomous error recovery (retry logic)
✅ Multi-turn conversation loops
✅ Dynamic tool registration
✅ Comprehensive error handling
✅ Backward compatibility (legacy JSON mode)

**Impact**: Transforms the AI agent from a "plan-and-execute" system to a **truly autonomous system** that can adapt, self-correct, and iterate until tasks are complete.

**Status**: Production-ready for UE5 Editor integration. Ready for Phase 5 (Blueprint generation, Material Editor API, Python execution).
