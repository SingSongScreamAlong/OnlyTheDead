# Verdun AI Agent - Autonomous UE5 Development Assistant

**An AI-powered plugin for Unreal Engine 5 that uses NLP and LLMs to build game content through natural language commands.**

---

## 🎯 Overview

Verdun AI Agent is a revolutionary UE5 plugin that allows you to **talk to your game engine** and have it autonomously build content for you. Instead of manually placing actors, sculpting terrain, or creating blueprints, you simply **describe what you want** and the AI agent does it for you.

### Key Features

✅ **Natural Language Interface** - Describe what you want in plain English
✅ **Autonomous Planning** - AI breaks down complex tasks into executable steps
✅ **Full UE5 Integration** - Manipulates actors, terrain, blueprints, materials
✅ **LLM Flexibility** - Supports OpenAI, Anthropic Claude, and local models
✅ **Verdun-Specific Tools** - Specialized generators for WWI content
✅ **Real-time Feedback** - Watch the AI work with live progress updates
✅ **Context-Aware** - Understands your project's specific terminology

---

## 🚀 Quick Start

### 1. Installation

1. Copy the `VerdunAIAgent` folder to your project's `Plugins/` directory
2. Restart Unreal Engine
3. Enable the plugin: **Edit > Plugins > Search "Verdun AI Agent"**
4. Restart the editor

### 2. Configuration

1. Open the AI Agent window: **Window > Verdun AI Agent** (or toolbar button)
2. Enter your API key (OpenAI or Anthropic)
3. Select your model (e.g., `claude-3-5-sonnet-20241022` or `gpt-4`)
4. Optionally customize the project context

### 3. First Command

Try a simple command:

```
Create a static mesh actor at location (0, 0, 0)
```

Then try something more complex:

```
Create a French trench system from coordinates (0, 0, 0) to (10000, 0, 0) with dugouts every 50 meters
```

---

## 📖 Usage Examples

### Basic Commands

**Spawn Actors:**
```
Create 10 trees randomly within a 1000 unit radius around (0, 0, 0)
```

**Modify Terrain:**
```
Raise the terrain at location (5000, 5000, 0) with a radius of 1000 units
```

**Create Blueprints:**
```
Create a blueprint actor called BP_TrenchSegment based on StaticMeshActor
```

### Verdun-Specific Commands

**Trenches:**
```
Generate a French trench system from (0, 0, 0) to (20000, 0, 0) with zigzag pattern and 5 dugouts
```

**Artillery Craters:**
```
Create 50 random artillery craters in No Man's Land between the French and German trenches
```

**Fortifications:**
```
Build Fort Douaumont at coordinates (5000, 5000, 100) with 75% damage level
```

**Complete Battlefield:**
```
Create a complete Verdun battlefield section with French trenches on the south, German trenches 300m north, artillery craters in between, and barbed wire defenses
```

---

## 🏗️ Architecture

### Core Components

1. **LLM Integration (`ULLMIntegration`)**
   - Handles communication with AI APIs
   - Supports OpenAI, Anthropic, and local models
   - Manages API keys and request configuration

2. **Agent Core (`UAIAgentCore`)**
   - Autonomous planning and execution
   - Breaks tasks into atomic steps
   - Verifies completion

3. **UE5 Command Executor (`UUE5CommandExecutor`)**
   - Executes commands in the Unreal Engine
   - Interfaces with UE5 APIs
   - Handles actor spawning, terrain modification, etc.

4. **Verdun Content Generator (`UVerdunContentGenerator`)**
   - Specialized WWI content creation
   - Trenches, fortifications, craters, villages
   - Historically accurate specifications

5. **Editor Widget (`SAIAgentEditorWidget`)**
   - User interface in the editor
   - Real-time progress feedback
   - Activity log

### Data Flow

```
User Command → AI Agent → LLM Planning → Task Steps → UE5 Executor → Content Created
                    ↓                                        ↓
              Project Context                          Verification
```

---

## 🛠️ Configuration

### LLM Providers

#### Anthropic Claude (Recommended)
```cpp
Config.Provider = ELLMProvider::Anthropic;
Config.APIKey = "sk-ant-api...";
Config.ModelName = "claude-3-5-sonnet-20241022";
```

**Pros:** Best reasoning, excellent at complex tasks, safer
**Cons:** Requires API key, costs money

#### OpenAI
```cpp
Config.Provider = ELLMProvider::OpenAI;
Config.APIKey = "sk-...";
Config.ModelName = "gpt-4";
```

**Pros:** Very capable, widely supported
**Cons:** Requires API key, costs money

#### Local Models (Ollama, LM Studio)
```cpp
Config.Provider = ELLMProvider::Local;
Config.BaseURL = "http://localhost:11434/api/generate";
Config.ModelName = "llama2";
```

**Pros:** Free, private, no internet required
**Cons:** Requires local model setup, less capable

### Project Context

The agent uses project context to understand domain-specific terms:

```cpp
AgentCore->SetProjectContext(
    "This is a WWI Verdun game.\n"
    "Trenches are 2m deep, 1.5m wide.\n"
    "Dugouts are underground shelters.\n"
    "Fort Douaumont is a major fortification."
);
```

You can also add knowledge dynamically:

```cpp
AgentCore->AddKnowledge("No Man's Land is typically 200-400 meters wide");
AgentCore->AddKnowledge("German trenches were deeper than French trenches");
```

---

## 📚 API Reference

### UAIAgentCore

```cpp
// Initialize the agent
void Initialize(const FLLMRequestConfig& Config);

// Execute a natural language command
void ExecuteCommand(const FString& Command);

// Stop current execution
void StopExecution();

// Set project context
void SetProjectContext(const FString& Context);

// Add knowledge
void AddKnowledge(const FString& Knowledge);
```

### UVerdunContentGenerator

```cpp
// Generate trench system
static AActor* GenerateTrenchSystem(
    UWorld* World,
    const TArray<FVector>& SplinePath,
    const FTrenchConfiguration& Config
);

// Create artillery crater
static AActor* CreateArtilleryCrater(
    UWorld* World,
    const FVector& Location,
    const FArtilleryCraterConfig& Config
);

// Generate No Man's Land
static void GenerateNoMansLand(
    UWorld* World,
    const TArray<FVector>& FrenchTrenchLine,
    const TArray<FVector>& GermanTrenchLine,
    int32 CraterCount,
    bool bAddBarbedWire
);

// Create fortification
static AActor* CreateFortification(
    UWorld* World,
    const FVector& Location,
    const FRotator& Rotation,
    const FFortificationConfig& Config
);
```

---

## 🎓 Advanced Usage

### Blueprint Integration

The agent is fully Blueprint-accessible:

1. Create a Blueprint
2. Add an AI Agent Core component
3. Call `Execute Command` with your text input
4. Bind to delegates for progress updates

Example Blueprint flow:
```
User Input (String) → Execute Command → OnTaskStarted → OnThinking → OnTaskCompleted
```

### Custom Commands

You can extend the command executor with your own commands:

```cpp
// In UUE5CommandExecutor.cpp
void UUE5CommandExecutor::ExecuteCustomCommand(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
    // Your custom logic here
    OnComplete.ExecuteIfBound(true, "Custom command executed");
}
```

Then register it in `ExecuteStep()`:

```cpp
else if (Step.CommandType == TEXT("CustomCommand"))
{
    ExecuteCustomCommand(Step, OnComplete);
}
```

### Python Integration

The agent can execute Python scripts:

```
Run this Python script in UE5:
import unreal
print("Hello from AI Agent!")
```

This requires the **Python Script Plugin** to be enabled.

---

## 🔒 Security & API Keys

### Storing API Keys

**⚠️ NEVER commit API keys to version control!**

Best practices:
1. Store keys in environment variables
2. Use UE5's config files (`.gitignore`d)
3. Use a secrets management system

### API Costs

- **Claude 3.5 Sonnet**: ~$3 per 1M tokens input, ~$15 per 1M tokens output
- **GPT-4**: ~$30 per 1M tokens input, ~$60 per 1M tokens output
- **Local Models**: Free (but requires powerful GPU)

A typical command uses 1000-5000 tokens, so ~$0.01-0.10 per command.

---

## 🐛 Troubleshooting

### "No editor world found"
**Solution:** Make sure you have a level open in the editor

### "Failed to parse JSON plan"
**Solution:** The LLM returned invalid JSON. Try:
- Simplifying your command
- Updating your model
- Checking your API key

### "Class not found: XYZ"
**Solution:** Make sure the class exists and is loaded:
```cpp
// Load the class first
UClass* MyClass = LoadClass<AActor>(nullptr, TEXT("/Game/MyBlueprint.MyBlueprint_C"));
```

### Agent gets stuck in "Planning" state
**Solution:**
- Check your internet connection
- Verify your API key is valid
- Check the log for error messages

---

## 🚧 Limitations & Future Work

### Current Limitations

- **Terrain Sculpting:** Basic implementation, needs full Landscape API integration
- **Python Execution:** Requires Python plugin, not fully tested
- **Verification:** Currently automatic, needs LLM-based verification
- **Undo/Redo:** Not yet implemented
- **Streaming:** LLM responses are not streamed (coming soon)

### Roadmap

**v1.1:**
- [ ] Full terrain sculpting with Landscape API
- [ ] LLM-based task verification
- [ ] Streaming responses
- [ ] Multi-step undo/redo

**v1.2:**
- [ ] Voice input support
- [ ] Image understanding (show the AI a sketch)
- [ ] Blueprint visual scripting generation
- [ ] Asset generation suggestions

**v2.0:**
- [ ] Multi-agent collaboration
- [ ] Learning from user corrections
- [ ] Integration with version control
- [ ] Cloud collaboration

---

## 💡 Tips & Best Practices

1. **Be Specific:** "Create a trench" vs "Create a 2m deep French trench from A to B with 5 dugouts"
2. **Use Coordinates:** Provide exact coordinates for predictable results
3. **Break Down Complex Tasks:** Instead of "Build the entire battlefield", do it in steps
4. **Use Project Context:** Add domain knowledge so the AI understands your terminology
5. **Verify Results:** Always check what the AI created before building on it
6. **Start Simple:** Test with simple commands before complex ones
7. **Save Often:** AI can make mistakes, save before major operations

---

## 🤝 Contributing

This plugin is part of the **Only The Dead** project. Contributions welcome!

Areas needing help:
- Full Landscape API integration
- More content generators (bunkers, villages, forests)
- Better error handling and recovery
- Performance optimization
- Unit tests

---

## 📄 License

Copyright Only The Dead Development Team. All Rights Reserved.

---

## 🙏 Acknowledgments

- Built for the **Only The Dead** WWI Verdun survival game
- Powered by Anthropic Claude and OpenAI
- Uses Unreal Engine 5's powerful APIs

---

## 📧 Support

- **GitHub Issues:** https://github.com/SingSongScreamAlong/OnlyTheDead/issues
- **Documentation:** See `/Plugins/VerdunAIAgent/Documentation/`

---

**"Let AI build your battlefield while you focus on making history."**

---

*Last Updated: November 2025*
*Plugin Version: 1.0.0*
*Compatible with: UE5.0+*
