# Only The Dead — Proof of Concept
## Single Trench / Artillery Strike Level

---

## What This POC Tests

The core question: **can we build a destructive trench experience that conveys the correct feel?**

The design document describes artillery as _"the defining trauma of Verdun"_ — 70-75% of casualties came from shells, not combat. This POC proves out the three systems that have to work together to create that feel:

1. **Artillery system** — randomised incoming rounds with realistic arc trajectories and whistle audio
2. **Trench destruction** — structural degradation from Intact → Damaged → Collapsed
3. **Morale / shell shock** — the psychological meter that responds to proximity of explosions

A player who plays this level for 10 minutes should feel dread, not excitement. If it feels exciting, something is wrong.

---

## File Structure Created

```
OnlyTheDead/
├── OnlyTheDead.uproject                    ← Open this in UE5
├── Source/
│   ├── OnlyTheDead.Target.cs
│   ├── OnlyTheDeadEditor.Target.cs
│   └── OnlyTheDead/
│       ├── OnlyTheDead.Build.cs
│       ├── Artillery/
│       │   ├── ArtilleryTypes.h            ← Shell data structs + presets
│       │   ├── ArtilleryShell.h/.cpp       ← Projectile actor + detonation
│       │   └── ArtilleryManager.h/.cpp     ← Barrage orchestration
│       ├── Trench/
│       │   └── TrenchSegment.h/.cpp        ← Destructible trench section
│       ├── Player/
│       │   ├── VerdunSoldier.h/.cpp        ← Player character
│       │   └── SurvivalComponent.h/.cpp    ← 7-meter survival system
│       └── GameMode/
│           └── VerdunGameMode.h/.cpp       ← Level setup + barrage timing
├── Config/
│   ├── DefaultEngine.ini                   ← Lumen, Chaos, audio settings
│   ├── DefaultGame.ini                     ← Default map + game mode
│   └── DefaultInput.ini                    ← Keyboard/gamepad bindings
├── Content/
│   └── Data/
│       └── DT_ArtilleryShells.json         ← Shell data table (import to UE5)
└── Scripts/
    └── generate_poc_level.py               ← Run in UE5 Python console
```

---

## Setup Instructions

### Step 1 — Compile the Project

1. Open `OnlyTheDead.uproject` in UE5 (5.3 or later)
2. UE5 will prompt to compile C++ — click **Yes**
3. Wait for compilation (~2-5 minutes first time)
4. Verify no errors in the Output Log

Common compile issues:
- Missing `GeometryCollectionEngine` → ensure Chaos plugin is enabled in `.uproject`
- Missing `NiagaraFunctionLibrary.h` → ensure Niagara plugin is enabled

### Step 2 — Create the Blueprint Layer

The C++ classes are parent classes. You need Blueprint children to assign meshes, sounds, and VFX.

Create these Blueprints in `Content/POC_TrenchArtillery/Blueprints/`:

| Blueprint | Parent Class | Key Assets to Assign |
|---|---|---|
| `BP_TrenchSegment` | `ATrenchSegment` | IntactMesh, DamagedMesh, CollapsedMesh, CollapseSound |
| `BP_ArtilleryShell` | `AArtilleryShell` | WhistleSound, ImpactSound, ExplosionNiagara |
| `BP_ArtilleryManager` | `AArtilleryManager` | ShellClass = BP_ArtilleryShell |
| `BP_VerdunSoldier` | `AVerdunSoldier` | DefaultMappingContext, MoveAction, LookAction, etc. |
| `BP_ShellCrater` | `AStaticMeshActor` | CraterMesh (disc/decal actor) |

### Step 3 — Assign Placeholder Assets

While waiting for art assets, use UE5 starter content or free Fab assets:

**Trench meshes** (placeholder):
- `IntactMesh` → SM_Cube scaled to (3m × 2m × 2m) with a dark grey material
- `DamagedMesh` → Same cube with a cracked material or 50% opacity
- `CollapsedMesh` → Flat SM_Cube (0.5m tall, spread wide) — represents rubble

**Shell FX** (placeholder):
- `ExplosionNiagara` → UE5 Starter Content `NS_Explosion` or Epic's free explosion VFX
- `WhistleSound` → Any high-pitched sweep audio (temp)
- `ImpactSound` → Any explosion audio (temp)

### Step 4 — Generate the Level

1. Open the UE5 Editor Output Log (`Window > Output Log`)
2. Switch to the **Python** tab
3. Run:
   ```python
   import unreal
   exec(open("Scripts/generate_poc_level.py").read())
   ```
   OR use `File > Execute Python Script > Scripts/generate_poc_level.py`

4. The script creates:
   - 12 TrenchSegment actors in an L-shape
   - ArtilleryManager pointed at the trench
   - 8 pre-existing shell craters
   - WWI atmospheric lighting and fog
   - PlayerStart inside the trench

### Step 5 — Configure Enhanced Input

In `BP_VerdunSoldier`:
1. Create an `IMC_VerdunSoldier` (Input Mapping Context)
2. Add actions and bind keys:

| Action | Key | Notes |
|---|---|---|
| `IA_Move` | WASD | 2D Axis Vector |
| `IA_Look` | Mouse XY | 2D Axis Vector |
| `IA_Crouch` | Left Control | Digital |
| `IA_Jump` | Space | Digital |

3. Assign `DefaultMappingContext = IMC_VerdunSoldier` in the Blueprint defaults

### Step 6 — Press Play

Press **Play In Editor**. Wait 5 seconds (BarrageStartDelaySeconds). Shells will begin landing.

---

## What to Expect

### Barrage Behaviour
- Default: 2-9 seconds between shells
- Distribution: ~65% small 75mm, ~25% 155mm howitzers, ~10% heavy
- Pattern: Random scatter across the trench zone
- Each shell's audio cue starts from launch — you hear the whistle before impact

### Trench Destruction Sequence
1. First hits on a TrenchSegment: mesh swaps to Damaged (50% health)
2. Continued hits: Collapsed state (0 HP)
3. Direct hit from 155mm+: Buried outright (one-hit collapse)
4. Adjacent segments take chain damage from heavy shells

### Morale / Shell Shock
- Watch the morale value in logs (`LogTemp, Log`)
- At Morale < 30: screen vignette intensifies, movement slows 40%
- At Morale < 10: heavy desaturation, 60% movement reduction
- Morale recovers during lulls (15-second cooldown after last hit, then 0.033/s)
- Crouching in an intact trench segment reduces morale damage (cover value: 95%)

### Does It Feel Right?

The POC succeeds if after 5 minutes the player's internal state is:
- "Where is the next one going to land?"
- "Should I move? Or stay still?"
- "That was close."

It fails if the player feels powerful, heroic, or bored.

---

## Tuning Knobs

All values are `UPROPERTY(EditAnywhere)` — adjust in the Details panel without recompiling:

**Make it harder (more overwhelming):**
- `ArtilleryManager.MinIntervalSeconds` → 0.5
- `ArtilleryManager.MaxIntervalSeconds` → 3.0
- `ArtilleryManager.Prob210mm` → 0.3

**Make it calmer (test shell shock recovery):**
- `ArtilleryManager.MinIntervalSeconds` → 10.0
- `ArtilleryManager.MaxIntervalSeconds` → 30.0

**Test shell shock immediately:**
- `SurvivalComponent.Morale` → 35 (in BeginPlay)
- Or: `SurvivalComponent.ShellShockThreshold` → 90

**Test trench destruction speed:**
- `TrenchSegment.MaxHealth` → 30 (faster collapse)
- `TrenchSegment.MaxHealth` → 500 (takes many hits)

**Walking barrage (historically authentic):**
- `ArtilleryManager.BarragePattern` → `LinearWalking`
- Shells walk along the trench in sequence

---

## Phase 2 Scope (Not in POC)

Once this POC validates the feel, Phase 2 adds:

- [ ] Remaining 6 survival meters (Hunger, Thirst, Stamina, Warmth, Hygiene, Alertness)
- [ ] First-person camera + proper soldier hands
- [ ] Enemy infantry appearing during lulls in shelling
- [ ] Gas shell variant (different audio, requires gas mask interaction)
- [ ] Burial alive mechanic (trench collapse traps player)
- [ ] Day/night cycle with authentic February 1916 sunrise/sunset times
- [ ] Real terrain from `fetch_elevation_data.py` / `export_for_ue5.py`
- [ ] Full mission structure (7-day rotation)
- [ ] NPC squadmates with morale states

---

## Historical Note

The L-shaped trench layout in this POC reflects actual WWI construction doctrine. Straight trenches amplify blast propagation — an artillery round landing in a straight trench kills everyone in line of sight. The L-shape (and the Z-shape / traverse system) limits this. The game's trench geometry should always reflect this reality.

> *"The artillery conquers, the infantry occupies."*
> — French General Retain, Battle of Verdun, 1916

The 40,000,000 shells fired at Verdun averaged one shell every 45 seconds over 303 days. This POC captures a single hour of that.
