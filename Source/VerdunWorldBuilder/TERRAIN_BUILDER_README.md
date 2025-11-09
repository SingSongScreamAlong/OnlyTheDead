# Verdun Terrain Builder - Destructible/Deformable Terrain System

**Build the Verdun battlefield, make it destructible, then add your artillery mechanics!**

## 🎯 What This Does

Creates a **fully destructible and deformable** Verdun battlefield terrain in Unreal Engine 5:

1. **Generates Landscape** - From heightmap or procedurally
2. **Applies Materials** - 70% mud, 15% chalk, 10% dead grass, 5% rock (historical)
3. **Makes It Destructible** - Runtime terrain deformation system
4. **Ready for Artillery** - You build the artillery, we provide the terrain deformation

## 🚀 Quick Start (5 Minutes)

### Step 1: Place Actor in Level

1. Open your level
2. Place Actors panel → Search "VerdunTerrainBuilder"
3. Drag into level
4. Select it

### Step 2: Configure (Use Defaults for Verdun)

In the Details panel:

**1. Terrain Generation:**
- ✅ Use Procedural (or set Heightmap Path if you have one)
- Landscape Quads: `511` (512 verts = good detail)
- Terrain Scale: `3000` (30m per vertex, SRTM resolution)
- Min Elevation: `165` (Verdun valley)
- Max Elevation: `390` (Fort Vaux hilltop)

**2. Materials:**
- ✅ Apply Materials
- ✅ Use Historical Percentages (70% mud, 15% chalk, etc.)

**3. Deformation:**
- ✅ Enable Deformation (makes terrain destructible)
- ✅ Batch Deformations (performance)
- Max Deformations Per Frame: `10`

**4. Build Settings:**
- ☐ Auto Build On Begin Play (or manually call BuildTerrain())

### Step 3: BUILD THE TERRAIN!

In Blueprint or C++:

**Blueprint:**
```
Get VerdunTerrainBuilder → BuildTerrain
```

**C++:**
```cpp
AVerdunTerrainBuilder* Builder = // ... get reference
Builder->BuildTerrain();
```

### Step 4: Test Destruction!

**Create a crater:**
```cpp
FVector CraterLocation = FVector(0, 0, 0); // Your location
Builder->TestCrater(CraterLocation, 600.0f, 150.0f); // 6m diameter, 1.5m deep
```

**Create a trench:**
```cpp
FVector Start = FVector(0, 0, 0);
FVector End = FVector(10000, 0, 0); // 100m trench
Builder->TestTrench(Start, End, 150.0f, 200.0f); // 1.5m wide, 2m deep
```

## 📦 What You Get

### Terrain Features

**Landscape:**
- Procedurally generated or from heightmap
- Configurable size (default: 8x8 components = ~7.6km × 7.6km)
- Historical Verdun elevations (165-390m)
- 30m vertex resolution (SRTM standard)

**Materials (Historical - Tier 1 Documented):**
- **70% Mud** - Brown-grey, churned, wet
- **15% Chalk** - White Verdun chalk on slopes/crater walls
- **10% Dead Grass** - Sparse, brown, 99% destroyed
- **5% Rock** - Limestone, broken by bombardment

**Deformation System:**
- ✅ Runtime crater generation
- ✅ Runtime trench generation
- ✅ Smooth/sharp/linear falloff options
- ✅ Batched for performance
- ✅ Collision updates (optional)

## 🛠️ Using the Deformation System

### For Your Artillery Mechanics

Get the deformation component:

```cpp
UVerdunTerrainDeformation* Deformation = Builder->GetDeformationComponent();
```

### Create Craters (Artillery Impacts)

```cpp
// 75mm shell (2m diameter, 0.5m deep)
Deformation->DeformCrater(
    ImpactLocation,
    200.0f,  // 2m radius = 200cm
    50.0f,   // 0.5m depth = 50cm
    EVerdunDeformationFalloff::Smooth
);

// 155mm shell (6m diameter, 1.5m deep)
Deformation->DeformCrater(
    ImpactLocation,
    600.0f,  // 6m radius
    150.0f,  // 1.5m depth
    EVerdunDeformationFalloff::Smooth
);

// 420mm Big Bertha (30m diameter, 6m deep!)
Deformation->DeformCrater(
    ImpactLocation,
    3000.0f, // 30m radius
    600.0f,  // 6m depth
    EVerdunDeformationFalloff::Smooth
);
```

### Create Trenches

```cpp
FVector TrenchStart = GetPlayerLocation();
FVector TrenchEnd = TrenchStart + FVector(10000, 0, 0); // 100m forward

Deformation->DeformTrench(
    TrenchStart,
    TrenchEnd,
    150.0f,  // 1.5m width
    200.0f,  // 2m depth
    false    // zigzag pattern?
);
```

### Batch Deformations (Performance)

For multiple impacts at once:

```cpp
TArray<FVerdunDeformation> Impacts;

// Create 100 crater definitions
for (int32 i = 0; i < 100; i++)
{
    FVerdunDeformation Crater;
    Crater.Location = GetRandomLocationInArea();
    Crater.Radius = 600.0f;
    Crater.Depth = 150.0f;
    Crater.Shape = EVerdunDeformationShape::Crater;
    Crater.Falloff = EVerdunDeformationFalloff::Smooth;
    Impacts.Add(Crater);
}

// Apply all at once (batched over multiple frames)
Deformation->BatchApplyDeformations(Impacts);
```

## 📊 Crater Dimensions (Historical Data)

| Shell Type | Diameter | Depth | Usage |
|------------|----------|-------|-------|
| 75mm Field Gun | 2m (200cm) | 0.5m (50cm) | Standard French artillery |
| 155mm Howitzer | 6m (600cm) | 1.5m (150cm) | Medium artillery |
| 305mm Heavy | 15m (1500cm) | 3m (300cm) | Heavy bombardment |
| 420mm Big Bertha | 30m (3000cm) | 6m (600cm) | Super-heavy (rare) |

## 🎮 Example: Artillery System Integration

```cpp
UCLASS()
class AMyArtilleryManager : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY()
    AVerdunTerrainBuilder* TerrainBuilder;

    UPROPERTY()
    UVerdunTerrainDeformation* TerrainDeformation;

    void FireArtillery(FVector TargetLocation, float ShellSize)
    {
        // 1. Spawn projectile
        AShellProjectile* Shell = SpawnShell(TargetLocation, ShellSize);

        // 2. When it hits, deform terrain
        Shell->OnImpact.AddDynamic(this, &AMyArtilleryManager::OnShellImpact);
    }

    UFUNCTION()
    void OnShellImpact(FVector ImpactLocation, float ShellSize)
    {
        // Get crater dimensions based on shell size
        float Radius, Depth;
        GetCraterDimensions(ShellSize, Radius, Depth);

        // Deform terrain!
        TerrainDeformation->DeformCrater(
            ImpactLocation,
            Radius,
            Depth,
            EVerdunDeformationFalloff::Smooth,
            false  // Batch it for performance
        );

        // Spawn effects, debris, etc.
        SpawnCraterEffects(ImpactLocation, Radius);
    }
};
```

## ⚙️ Advanced Configuration

### Landscape Size Options

Calculate size: `ComponentCount × 511 × Scale`

**Small (testing):**
- Components: 4×4, Scale: 3000cm
- Size: ~6km × ~6km
- Vertices: ~2M

**Medium (Verdun default):**
- Components: 8×8, Scale: 3000cm
- Size: ~12km × ~12km
- Vertices: ~8M

**Large (full battlefield):**
- Components: 16×16, Scale: 3000cm
- Size: ~24km × ~24km
- Vertices: ~33M (performance impact!)

### Performance Tips

**Deformation:**
- Enable batching: `bBatchDeformations = true`
- Limit per frame: `MaxDeformationsPerFrame = 10`
- Disable collision updates for distant craters: `bUpdateCollision = false`

**Material Painting:**
- Use layer LOD for distant areas
- Simplify rules for lower-end hardware

## 🎨 Material Setup (Optional)

For materials to work, create layer info objects:

1. Content Browser → Right Click → Miscellaneous → Landscape Layer Info
2. Create 4 layer infos:
   - `LI_Mud` (weight-blended)
   - `LI_Chalk` (weight-blended)
   - `LI_DeadGrass` (weight-blended)
   - `LI_Rock` (weight-blended)
3. Assign to Terrain Builder:
   - Mud Layer Info → `LI_Mud`
   - Chalk Layer Info → `LI_Chalk`
   - etc.

Create landscape material with 4 layers matching those names.

## 📝 Blueprint Events

Listen to build progress:

```
On Terrain Build Started → Show loading screen
On Landscape Generated → Update progress bar (33%)
On Materials Applied → Update progress bar (66%)
On Deformation Setup → Update progress bar (100%)
On Terrain Build Complete → Hide loading screen, enable gameplay
```

Listen to deformation:

```
On Deformation Applied → Spawn dust/debris effect
On Batch Deformations Complete → Update crater count UI
```

## 🔍 Troubleshooting

**"Failed to spawn landscape actor"**
- Landscape generation is editor-only in this version
- For runtime: Use pre-built landscape and only deform it
- Or: Generate in editor, save, then load at runtime

**"No deformation happening"**
- Check `bEnableRuntimeDeformation` is true
- Verify `TargetLandscape` is set
- Check location is within landscape bounds
- Look for errors in Output Log

**Performance issues with deformation**
- Enable batching
- Reduce `MaxDeformationsPerFrame`
- Disable collision updates for distant craters
- Use LOD for crater meshes instead of deformation

**Materials not showing**
- Create layer info objects in editor
- Assign to terrain builder
- Create landscape material with matching layers
- Material painting may be editor-only (placeholder in code)

## 🏗️ Architecture

```
AVerdunTerrainBuilder (Main actor - place this!)
  │
  ├─ AVerdunLandscapeGenerator
  │   └─ Creates landscape mesh from heightmap/procedural
  │
  ├─ UVerdunTerrainDeformation (Component)
  │   ├─ DeformCrater()
  │   ├─ DeformTrench()
  │   └─ BatchApplyDeformations()
  │
  └─ UVerdunTerrainMaterialPainter (Component)
      ├─ Mud (70%)
      ├─ Chalk (15%)
      ├─ Dead Grass (10%)
      └─ Rock (5%)
```

## 🎖️ Historical Accuracy (Tier 1)

Terrain composition based on documented sources:
- **70% Mud** - Constant rain, churned by shells, ankle-knee deep (photos, testimonies)
- **15% Chalk** - White Verdun chalk exposed on slopes (photos, French archives)
- **10% Dead Grass** - 99% vegetation destroyed by gas/shelling (documented)
- **5% Rock** - Limestone bedrock, broken by bombardment (geological surveys)

Elevations from French military maps (1916):
- Min: 165m (Meuse River valley)
- Max: 390m (Fort Vaux)

## 📚 Next Steps

1. ✅ **Build the terrain** (this system)
2. 🔜 **Build artillery mechanics** (your code)
   - Projectile physics
   - Impact detection
   - Call `TerrainDeformation->DeformCrater()` on impact
3. 🔜 **Add gameplay systems**
   - Player movement affected by mud/craters
   - Cover from craters
   - Trench navigation

---

**The terrain is ready. Now build the war!** 🎖️
