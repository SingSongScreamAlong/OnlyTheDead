# Claude Terminal - AI-Powered UE5 Plugin

An Unreal Engine 5 plugin that integrates Claude AI to provide a natural language terminal for manipulating the game world, spawning actors, creating procedural content, and more.

## 🌟 Features

- **Natural Language Commands**: Talk to Claude in plain English to manipulate your UE5 world
- **World Manipulation**: Spawn actors, create roads, place models, modify landscapes
- **Conversational AI**: Ask questions, get help, and have Claude explain UE5 concepts
- **Command History**: Navigate previous commands with up/down arrow keys
- **Persistent Conversations**: Claude remembers the conversation context
- **Editor Integration**: Dockable terminal window with toolbar button access

## 📋 Requirements

- Unreal Engine 5.0 or higher
- Claude API key (get one at https://console.anthropic.com/)
- Internet connection for API access

## 🚀 Installation

### Method 1: Copy to Plugins Directory

1. Copy the `ClaudeTerminal` folder to your project's `Plugins` directory:
   ```
   YourProject/
   └── Plugins/
       └── ClaudeTerminal/
   ```

2. Right-click on your `.uproject` file and select "Generate Visual Studio project files"

3. Open the project in Unreal Engine 5

4. Enable the plugin: Edit > Plugins > Search for "Claude Terminal" > Check "Enabled"

5. Restart Unreal Engine

### Method 2: Engine Plugins Directory

Copy the plugin to your UE5 engine plugins directory:
```
UE_5.X/Engine/Plugins/ClaudeTerminal/
```

## ⚙️ Configuration

### Setting Up Your API Key

1. Get your Claude API key from https://console.anthropic.com/

2. In Unreal Editor, open: **Edit > Project Settings > Plugins > Claude Terminal**

3. Enter your API key in the "API Key" field

4. Save settings

### Alternative: Manual Configuration

Edit `Config/DefaultEditor.ini` in your project:

```ini
[ClaudeTerminal]
APIKey=sk-ant-api03-...
```

## 📖 Usage

### Opening the Terminal

**Method 1**: Click the "Claude Terminal" button in the toolbar

**Method 2**: Go to **Window > Claude AI Terminal**

**Method 3**: Press `Ctrl+Shift+C` (if you configure the shortcut)

### Example Commands

The terminal accepts natural language commands:

#### Spawning Actors
```
"Spawn a cube at 0,0,100"
"Create a point light at my camera location"
"Place a player start at 1000, 500, 0"
```

#### Creating Roads
```
"Create a road from 0,0,0 to 1000,0,0"
"Make a 6-meter wide road from point A to point B"
"Build a dirt road connecting these two locations"
```

#### Placing Models
```
"Place the trench static mesh at these coordinates"
"Add a tree at 500,200,0 with scale 2"
"Put a tank model at the player location facing north"
```

#### Landscape Modification
```
"Raise the terrain at 0,0,0 with radius 500"
"Flatten the ground under the base"
"Smooth the landscape around the spawn point"
```

#### Conversational
```
"How do I create a material?"
"What's the best way to optimize my level?"
"Explain how blueprints work"
```

## 🔧 Available Commands

Claude understands your intent and translates it to structured commands:

### spawn_actor
Spawns any UE5 actor class in the world.

**Parameters**:
- `class_name`: Name of the actor class (e.g., "StaticMeshActor", "PointLight")
- `location`: {x, y, z} coordinates
- `rotation`: {pitch, yaw, roll} in degrees
- `scale`: {x, y, z} scale factors

**Example**:
```
"Spawn a StaticMeshActor at 0,0,100 with rotation 0,45,0"
```

### create_road
Creates a procedural spline-based road between two points.

**Parameters**:
- `start_location`: {x, y, z} start coordinates
- `end_location`: {x, y, z} end coordinates
- `width`: Road width in cm (default: 400)
- `material_path`: Optional material asset path

**Example**:
```
"Create a road from 0,0,0 to 5000,0,0 with width 600"
```

### place_model
Places a static mesh in the world.

**Parameters**:
- `mesh_path`: Asset path to the static mesh
- `location`: {x, y, z} coordinates
- `rotation`: {pitch, yaw, roll} in degrees
- `scale`: {x, y, z} scale factors

**Example**:
```
"Place the trench mesh at 1000,0,0 rotated 90 degrees"
```

### modify_landscape
Modifies terrain height or smoothness.

**Parameters**:
- `operation`: "raise" | "lower" | "smooth" | "flatten"
- `location`: {x, y, z} center point
- `radius`: Affected area radius in cm
- `strength`: Effect strength (0.0 - 1.0)

**Example**:
```
"Raise the terrain at 0,0,0 with radius 1000 and strength 0.5"
```

### create_material
Creates a dynamic material instance.

**Parameters**:
- `base_material_path`: Path to base material
- `instance_name`: Name for the instance
- `parameters`: Object with parameter values

**Example**:
```
"Create a material from M_Basic with red color"
```

## 🎯 Use Cases for "Only The Dead"

This plugin is perfect for your WWI Verdun game development:

### Rapid Prototyping
```
"Create a trench line from Fort Douaumont to the front"
"Place barbed wire obstacles every 50 meters along this path"
"Spawn shell craters in a 500-meter radius around this point"
```

### Environment Setup
```
"Create a muddy battlefield material and apply it here"
"Place destroyed village buildings along this road"
"Add atmospheric fog to simulate gas attacks"
```

### Level Design Iteration
```
"Flatten this area for the French defensive line"
"Raise terrain to create Hill 304"
"Create a network of trenches connecting these points"
```

### Asset Placement
```
"Place soldier spawn points along the trench"
"Add sandbag fortifications every 10 meters"
"Position artillery pieces on this ridge"
```

## 🔒 Security & Privacy

- API key is stored locally in your project's config files
- Never commit `DefaultEditor.ini` with your API key to version control
- Add to `.gitignore`: `Config/DefaultEditor.ini`
- Conversations are sent to Anthropic's API (see their privacy policy)
- No data is stored on external servers beyond the API interaction

## 🐛 Troubleshooting

### "No API Key configured" error
- Make sure you've set your API key in Project Settings
- Verify the key starts with `sk-ant-api03-`
- Check that you have internet connectivity

### Commands not executing
- Ensure you're in the editor (not PIE mode)
- Check the Output Log for detailed error messages
- Verify asset paths are correct (use Content Browser to copy paths)

### Plugin not showing up
- Verify the plugin is in the correct directory
- Regenerate project files
- Check that all `.Build.cs` files are present
- Look for compilation errors in the Output Log

### HTTP request failures
- Check your internet connection
- Verify your API key is valid and has credits
- Check if you're behind a corporate firewall

## 🛠️ Advanced Configuration

### Custom System Prompt

You can modify the system prompt in `ClaudeAPIClient.cpp` to customize Claude's behavior:

```cpp
SystemPrompt = TEXT("Your custom instructions here...");
```

### Adding Custom Commands

1. Add your command to the system prompt in `ClaudeAPIClient.cpp`
2. Implement the handler in `CommandExecutor.cpp`
3. Add routing logic in `ProcessResponse()`

Example:
```cpp
bool FCommandExecutor::ExecuteCustomCommand(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
    // Your implementation
    return true;
}
```

## 📚 Technical Architecture

```
ClaudeTerminal/
├── Source/
│   ├── ClaudeTerminal/              # Runtime module
│   │   ├── ClaudeAPIClient          # HTTP client for Claude API
│   │   ├── CommandExecutor          # Parses and executes commands
│   │   └── ClaudeTerminal           # Module initialization
│   └── ClaudeTerminalEditor/        # Editor module
│       ├── STerminalWidget          # Slate UI widget
│       └── ClaudeTerminalEditor     # Editor integration
├── Config/
│   └── DefaultAITerminal.ini        # Default configuration
├── Content/                          # Plugin content (icons, etc.)
└── ClaudeTerminal.uplugin           # Plugin descriptor
```

### Request Flow

1. User types command in terminal
2. `STerminalWidget` captures input
3. `FClaudeAPIClient` sends to Claude API
4. Claude responds with natural language + optional JSON command
5. `FCommandExecutor` parses JSON and executes UE5 operations
6. Results displayed in terminal output

## 🤝 Contributing

This plugin is part of the "Only The Dead" project. Contributions welcome!

### Ideas for Extension

- [ ] Blueprint node integration
- [ ] Batch command execution
- [ ] Command macros and scripting
- [ ] Asset generation via AI
- [ ] Landscape painting from descriptions
- [ ] Audio command support ("Play explosion sound at 0,0,0")
- [ ] Sequencer integration ("Create a camera flythrough")

## 📄 License

Part of the "Only The Dead" project. See main repository for license details.

## 🙏 Credits

- **Claude AI** by Anthropic - The brain behind the terminal
- **Unreal Engine 5** by Epic Games - The platform
- **Only The Dead Team** - Plugin development

## 📞 Support

- GitHub Issues: https://github.com/SingSongScreamAlong/OnlyTheDead/issues
- Documentation: See this README
- UE5 Forums: Search for "Claude Terminal Plugin"

---

**"Only the dead have seen the end of war."** - Use AI to build it.
