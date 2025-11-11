# Verdun AI Agent - Quick Usage Guide

## Getting Started in 5 Minutes

### Step 1: Enable the Plugin

1. Open your Unreal Engine 5 project
2. Go to **Edit > Plugins**
3. Search for "Verdun AI Agent"
4. Check the box to enable it
5. Restart the editor

### Step 2: Open the AI Agent Window

- **Method 1:** Click the AI Agent button in the toolbar
- **Method 2:** Go to **Window > Verdun AI Agent**

### Step 3: Configure Your API

In the AI Agent window:

1. **API Key:** Enter your Anthropic or OpenAI API key
   - Get Anthropic key: https://console.anthropic.com/
   - Get OpenAI key: https://platform.openai.com/

2. **Model:** Choose your model
   - Anthropic: `claude-3-5-sonnet-20241022` (recommended)
   - OpenAI: `gpt-4` or `gpt-3.5-turbo`

3. **Project Context:** (Optional) Describe your project
   - Example: "WWI Verdun game with historical accuracy"

### Step 4: Try Your First Command

Enter in the command box:

```
Create a static mesh actor at coordinates (0, 0, 0)
```

Press **Execute Command** and watch the AI work!

---

## Example Commands by Complexity

### Beginner

```
Create a cube at (1000, 0, 100)
```

```
Place 5 trees randomly around (0, 0, 0) within 500 units
```

```
Create a landscape at the origin
```

### Intermediate

```
Create a French trench from (0, 0, 0) to (5000, 0, 0) with 2 meter depth
```

```
Generate a communication trench connecting (1000, 0, 0) and (1000, 5000, 0)
```

```
Place 20 artillery craters randomly in a 10000 unit radius circle
```

### Advanced

```
Build a complete trench system with:
- Main trench from (0, 0, 0) to (20000, 0, 0)
- Zigzag pattern every 10 meters
- 10 dugouts evenly spaced
- Sandbags and duckboards
- Firing steps
```

```
Create a Verdun battlefield section:
- French trenches on the south from (0, 0, 0) to (15000, 0, 0)
- German trenches 300 meters north
- No Man's Land between them with 50 craters
- 3 rows of barbed wire in front of each trench
- 2 communication trenches connecting to rear
```

---

## Understanding the AI's Process

When you give a command, the AI:

1. **Plans:** Breaks your request into steps
   - Status: "Planning - Analyzing request..."
   - Progress bar: 0-30%

2. **Executes:** Runs each step in UE5
   - Status: "Executing step X/Y: [description]"
   - Progress bar: 30-90%

3. **Verifies:** Checks if it worked
   - Status: "Verifying completion..."
   - Progress bar: 90-100%

4. **Completes:** Reports success or failure
   - Status: "Completed" (green) or "Failed" (red)

---

## Tips for Best Results

### ✅ DO:

- **Be specific** about coordinates and measurements
  ```
  Good: "Create a trench 2 meters deep from (0,0,0) to (10000,0,0)"
  Bad: "Make a trench over there"
  ```

- **Use meters** or Unreal units (1 meter = 100 units)
  ```
  "Create a crater 8 meters in diameter" = 800 units
  ```

- **Provide context** in the Project Context field
  ```
  "Trenches should be WWI French style, 2m deep, with dugouts"
  ```

- **Start simple** and build up complexity

### ❌ DON'T:

- **Don't use vague terms**
  ```
  Bad: "Make something cool"
  Bad: "Put stuff around"
  ```

- **Don't mix too many requests**
  ```
  Bad: "Create trenches and also make a fort and add trees and..."
  Better: Break into separate commands
  ```

- **Don't expect perfection first try**
  - AI can make mistakes
  - Be prepared to give follow-up instructions

---

## Verdun-Specific Features

### Trench Types

**French Trenches:**
```
Generate a French trench system from (0,0,0) to (10000,0,0)
```
- 2m deep (200 units)
- 1.5m wide (150 units)
- Zigzag pattern
- Dugouts every 50m

**German Trenches:**
```
Generate a German trench system from (0,0,0) to (10000,0,0)
```
- 2.5m deep (deeper than French)
- More duckboards
- Better drainage

**Communication Trenches:**
```
Create a communication trench from (0,0,0) to (0,5000,0)
```
- 1m wide (narrower)
- No firing steps
- Connects main trenches to rear

### Battlefield Elements

**Artillery Craters:**
```
Create an artillery crater at (5000, 5000, 0) with 10 meter diameter
```

**No Man's Land:**
```
Generate No Man's Land between French trenches at Y=0 and German trenches at Y=30000 with 100 craters
```

**Fortifications:**
```
Create Fort Douaumont at (0, 0, 100) with 50% damage
```

**Barbed Wire:**
```
Add 3 rows of barbed wire from (0,0,0) to (10000,0,0)
```

---

## Troubleshooting

### "Agent is already working on a task"

**Problem:** You tried to execute while another task is running

**Solution:**
- Wait for current task to finish
- Or click **Stop** button
- Then try again

### "No editor world found"

**Problem:** No level is open

**Solution:**
- Open or create a level
- Make sure you're in the viewport

### "Failed to parse JSON plan"

**Problem:** AI returned invalid response

**Solution:**
- Try simplifying your command
- Check your API key is valid
- Try again (sometimes the AI has a bad response)

### "Class not found: XYZ"

**Problem:** AI tried to use a class that doesn't exist

**Solution:**
- Be more specific about what you want
- Use classes you know exist
- Check the log for details

---

## Activity Log

The log at the bottom shows everything the AI is doing:

```
[14:32:15] AI Agent initialized. Ready for commands.
[14:32:45] Received command: Create a trench...
[14:32:46] Planning complete, parsing response...
[14:32:46] Parsed 5 steps from plan
[14:32:47] Executing step 1: Create trench actor [CreateActor]
[14:32:48] Step 1 completed: Created actor: TrenchSystem_1
[14:32:48] Executing step 2: Add dugouts [ModifyActor]
...
[14:33:05] Task completed successfully
```

Use this to:
- Debug what went wrong
- Understand what the AI did
- Learn from its process

---

## Keyboard Shortcuts

- **Ctrl+Enter** in command box: Execute command (coming soon)
- **Esc**: Stop execution (coming soon)

---

## Best Practices for Verdun Development

### 1. Set Up Project Context First

```
Project Context:
"WWI Verdun survival game, February 1916
- French trenches: 2m deep, 1.5m wide, zigzag pattern
- German trenches: 2.5m deep, better fortified
- No Man's Land: 200-400m between trenches
- Artillery craters: 5-10m diameter
- Historical accuracy crucial"
```

### 2. Build in Layers

**Layer 1: Terrain**
```
Create the landscape terrain for the Verdun battlefield
```

**Layer 2: Main Trenches**
```
Generate French front-line trenches
```

**Layer 3: Support Trenches**
```
Add communication trenches and secondary lines
```

**Layer 4: Defenses**
```
Add barbed wire and fortifications
```

**Layer 5: Destruction**
```
Add artillery craters and destroyed buildings
```

### 3. Use Coordinates Systematically

Pick a coordinate system and stick to it:
- French lines: Y = 0 to Y = 5000
- No Man's Land: Y = 5000 to Y = 35000
- German lines: Y = 35000 to Y = 40000

### 4. Save Checkpoints

Before major AI operations:
1. Save your level
2. Create a checkpoint (File > Save As... with version number)
3. Execute AI command
4. Verify results
5. Either keep or revert to checkpoint

---

## Advanced: Using in Blueprints

You can call the AI agent from Blueprints:

1. Add an **AI Agent Core** component to an actor
2. Call **Initialize** with your config
3. Call **Execute Command** with a string
4. Bind to **On Task Completed** for results

Example Blueprint:
```
BeginPlay → Initialize AI Agent → Wait for User Input → Execute Command → On Task Completed → Show Results
```

---

## Getting Help

- Check the **Activity Log** for error messages
- Read the full **README.md** for detailed documentation
- Report issues on GitHub

---

**Happy building! Let the AI do the heavy lifting while you focus on game design.**

---

*For more information, see the full README.md*
