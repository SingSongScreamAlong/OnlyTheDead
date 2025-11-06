# Claude Terminal - Quick Start Guide

## 5-Minute Setup

### Step 1: Get Your API Key (2 minutes)

1. Go to https://console.anthropic.com/
2. Sign up or log in
3. Navigate to "API Keys"
4. Click "Create Key"
5. Copy your key (starts with `sk-ant-api03-...`)

### Step 2: Install Plugin (1 minute)

1. Copy the `ClaudeTerminal` folder to `YourProject/Plugins/`
2. Right-click your `.uproject` file → "Generate Visual Studio project files"
3. Open your project in UE5
4. Go to Edit > Plugins > Search "Claude Terminal" → Enable
5. Restart UE5

### Step 3: Configure (1 minute)

**Option A: Project Settings (Recommended)**
1. Edit > Project Settings
2. Search for "Claude Terminal"
3. Paste your API key
4. Click "Save"

**Option B: Config File**
1. Open `Config/DefaultEditor.ini` in your project
2. Add:
   ```ini
   [ClaudeTerminal]
   APIKey=sk-ant-api03-YOUR_KEY_HERE
   ```
3. Save file

### Step 4: Open Terminal (30 seconds)

Click the "Claude Terminal" button in the toolbar (or Window > Claude AI Terminal)

### Step 5: Try It! (30 seconds)

Type any of these commands:

```
"Hello Claude! Can you help me?"
"Spawn a cube at 0,0,100"
"Create a road from 0,0,0 to 1000,0,0"
"What's the best way to create a trench in UE5?"
```

## 🎮 Using with "Only The Dead"

### Quick Battlefield Setup

```
"Create a trench line from 0,0,0 to 5000,0,0"
"Place shell craters randomly in a 1000 meter radius around 2500,0,0"
"Spawn barbed wire obstacles every 50 meters along the trench"
"Flatten the terrain for the defensive line at coordinates X,Y,Z"
```

### Rapid Asset Placement

```
"Place the destroyed building mesh at 3000,1000,0"
"Add 50 sandbag stacks along this trench line"
"Position artillery pieces on the ridge at elevation 500"
"Create spawn points every 100 meters along the French line"
```

### Environment Design

```
"Raise terrain to create Hill 304 at these coordinates"
"Create a muddy material and apply it to this area"
"Add fog with low visibility for a gas attack scene"
"Smooth the landscape around Fort Douaumont"
```

## 💡 Pro Tips

### Command History
- Press **↑** (up arrow) to cycle through previous commands
- Press **↓** (down arrow) to go forward in history

### Clear Conversation
- Click "New Conversation" to start fresh
- Claude forgets previous context

### Copy Asset Paths
- Right-click any asset in Content Browser
- "Copy Reference"
- Paste into Claude commands

### Ask Questions
```
"How do I make this more performant?"
"What's the best way to create weather effects?"
"Explain how landscape layers work"
```

## 🚨 Common Issues

**"No API Key configured"**
- Set your API key in Project Settings > Claude Terminal

**"Failed to spawn actor"**
- Check the class name (use exact UE5 class names)
- Try with quotes: "StaticMeshActor"

**"Could not load mesh"**
- Right-click mesh in Content Browser → Copy Reference
- Use the exact path in your command

**Plugin not visible**
- Regenerate project files
- Enable plugin in Edit > Plugins
- Restart UE5

## 📖 Next Steps

- Read the full [README.md](README.md) for all features
- Experiment with different commands
- Create custom commands (see Advanced Configuration)
- Join the discussion on GitHub

## 🎯 Example: Complete Trench Scene

Try this sequence to create a basic trench scene:

```
1. "Flatten terrain in a 2000x500 meter rectangle at 0,0,0"
2. "Create a trench line from -1000,0,0 to 1000,0,0"
3. "Add sandbags every 10 meters along the trench"
4. "Place barbed wire 50 meters in front of the trench"
5. "Create 20 shell craters randomly between the wire and trench"
6. "Spawn a player start inside the trench at 0,0,0"
7. "Add a directional light for dawn lighting"
```

---

**Ready to build your Verdun battlefield with AI? Let's go! 🎖️**
