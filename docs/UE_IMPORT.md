# Verdun Pipeline - UE5 Import Guide

Complete guide to importing and managing pipeline assets in Unreal Engine 5.

---

## Overview

The pipeline exports assets from Houdini and imports them to UE5 automatically:

**Flow:**
```
Houdini HDAs → FBX/USD + Heightmaps + Catalogs → UE5 Auto-Import → Level Assembly
```

---

## Asset Types

### 1. Static Meshes (FBX/USD)

**Source:** Houdini geometry exports
**Destination:** `/Game/Verdun/Tiles/{TileID}/`
**Format:** FBX or USD

**Settings Applied:**
- Nanite enabled (if supported)
- Collision preset: `BlockAllDynamic`
- LODs: Auto-generated or from source
- Master material assigned

### 2. Heightmaps (PNG/RAW)

**Source:** Houdini heightfield exports
**Destination:** `/Game/Verdun/Heightmaps/{TileID}/`
**Format:** 16-bit PNG or RAW

**Import Process:**
1. Import as Texture2D
2. Compression: `TC_Default` or `TC_HDR`
3. Apply to Landscape via `LandscapeEditor` API

### 3. Weight Maps (PNG)

**Source:** Houdini layer masks
**Destination:** `/Game/Verdun/WeightMaps/{TileID}/`
**Format:** 8-bit PNG

**Layers:**
- `mud` - Mud material
- `water` - Water puddles/craters
- `wood` - Duckboards, wood structures
- `metal` - Debris
- `grass` - Grass coverage
- `blood` - Staining (trenches)

### 4. Instance Catalogs (CSV/JSON)

**Source:** Houdini scatter outputs
**Format:** CSV or JSON
**Contains:** Asset paths, transforms (position, rotation, scale)

**Used by:** `build_world.py` to place actors/HISM

---

## Automated Import Script

### Running auto_ingest.py

**Headless (Commandlet):**
```cmd
UnrealEditor-Cmd.exe VerdunProject.uproject ^
  -run=pythonscript ^
  -script=ue/Scripts/auto_ingest.py
```

**From UE5 Python Console:**
```python
import auto_ingest
auto_ingest.main()
```

### What It Does

1. **Scans** `data/staging/meshes/` for new FBX files
2. **Imports** to `/Game/Verdun/Tiles/{TileID}/`
3. **Enables Nanite** on all static meshes
4. **Sets collision** preset (configurable)
5. **Assigns materials** from master material
6. **Saves packages** automatically
7. **Logs results** to `orchestrator/logs/ue_import_results.json`

### Configuration

Edit `orchestrator/config.yaml`:

```yaml
unreal:
  import:
    base_path: "/Game/Verdun/Tiles"
    enable_nanite: true
    collision_preset: "BlockAllDynamic"
    master_material: "/Game/Verdun/Materials/M_TerrainMaster"
```

---

## Manual Import (Fallback)

If automated import fails, import manually:

### Static Mesh Import

1. Open Content Browser
2. Navigate to `/Game/Verdun/Tiles/N13/`
3. Right-click → Import to `/Game/...`
4. Select FBX file: `data/staging/meshes/N13_terrain.fbx`
5. In FBX Import Options:
   - ✓ Import Meshes
   - ✓ Import Materials
   - ✓ Combine Meshes
   - Skeletal Mesh: Off
6. Import

### Enable Nanite

1. Open static mesh asset
2. Details panel → LOD Settings
3. Enable `Nanite Settings → Enabled`
4. Save

### Set Collision

1. Open static mesh asset
2. Collision menu → Add collision (or import from FBX)
3. Details → Collision Complexity: `Use Complex Collision As Simple`
4. Save

### Assign Material

1. Open static mesh asset
2. Details panel → Material Slots
3. Set Material 0 to: `/Game/Verdun/Materials/M_TerrainMaster`
4. Save

---

## Heightmap & Weight Map Import

### Landscape Creation

1. **New Landscape:**
   - Mode: Landscape
   - Create New → Import from File
   - Heightmap File: `data/staging/heightmaps/N13_height.png`
   - Material: `/Game/Verdun/Materials/M_TerrainMaster`

2. **Update Existing:**
   - Select Landscape
   - Landscape Editor → Sculpt
   - Import heightmap (replaces)

### Weight Map Painting

1. **Setup Layers:**
   - Landscape Material must have Layer Info assets
   - For each layer (mud, water, etc.):
     - Right-click material → Create Layer Info
     - Save to `/Game/Verdun/Layers/`

2. **Import Weight Maps:**
   - Landscape Editor → Paint mode
   - Select layer (e.g., "mud")
   - Import layer from file: `data/staging/weightmaps/N13/mud.png`
   - Repeat for all layers

---

## Instance Placement (build_world.py)

### Running build_world.py

**Headless:**
```cmd
UnrealEditor-Cmd.exe VerdunProject.uproject ^
  -run=pythonscript ^
  -script=ue/Scripts/build_world.py
```

**From UE5 Python:**
```python
import build_world
build_world.main()
```

### Instance Catalog Format

**CSV:**
```csv
asset_path,x,y,z,pitch,yaw,roll,scale_x,scale_y,scale_z
/Game/Props/Debris/WireReel,1234.5,5678.9,100.0,0,45,0,1.0,1.0,1.0
```

**JSON:**
```json
{
  "instances": [
    {
      "asset_path": "/Game/Props/Debris/WireReel",
      "x": 1234.5, "y": 5678.9, "z": 100.0,
      "pitch": 0, "yaw": 45, "roll": 0,
      "scale_x": 1.0, "scale_y": 1.0, "scale_z": 1.0
    }
  ]
}
```

### Placement Methods

**Individual Actors:**
- Used for < 10 instances per asset
- Full per-actor control
- More memory overhead

**Hierarchical Instanced Static Mesh (HISM):**
- Used for ≥ 10 instances per asset
- GPU instancing, very efficient
- LODs and culling supported

Script automatically chooses based on instance count.

---

## Material Setup

### Master Material (M_TerrainMaster)

Create a master material with layer blending:

**Layers:**
1. Base (grass/dirt)
2. Mud (from `layer_mud` weight map)
3. Water (from `layer_water`)
4. Wood (from `layer_wood`)
5. Blood (from `layer_blood`)

**Blend Mode:**
- Use Landscape Layer Blend node
- Import weight maps as layer info
- Adjust blend sharpness

**Example Setup:**
```
Landscape Layer Blend
  ├─ Base (Grass)
  ├─ Mud (from layer_mud)
  ├─ Water (from layer_water)
  └─ Wood (from layer_wood)
```

### Material Instances

For per-tile variation, create Material Instances:

1. Right-click `M_TerrainMaster` → Create Material Instance
2. Name: `MI_N13_Terrain`
3. Override parameters:
   - Mud intensity
   - Water reflectivity
   - Seasonal tint

---

## HLOD (Hierarchical Level of Detail)

### Setup

1. **Enable HLOD System:**
   - World Settings → World Composition → Enable HLOD

2. **Configure HLOD Levels:**
   - HLOD Level 0: Cluster radius = 10000 cm
   - HLOD Level 1: Cluster radius = 50000 cm

3. **Build HLODs:**
   - Window → Hierarchical LOD Outliner
   - Generate Clusters
   - Build All Proxy Meshes

### Automated Rebuild

Use `maintenance.py`:

```bash
python orchestrator/foreman.py ue rebuild-hlod --map Verdun_Persistent
```

---

## Navigation Mesh

### Setup

1. **Add Nav Mesh Bounds Volume:**
   - Place actor → Nav Mesh Bounds Volume
   - Scale to cover tile area

2. **Configure Nav Mesh:**
   - Project Settings → Navigation Mesh
   - Agent Radius: 34 cm (player)
   - Agent Height: 180 cm

3. **Build Nav Mesh:**
   - Press `P` in viewport to visualize
   - Build → Build Paths

### Automated Rebuild

Use `maintenance.py`:

```python
import maintenance
maintenance.rebuild_navmesh()
```

---

## Collision Setup

### Collision Presets

**Terrain:**
- Collision Enabled: Query and Physics
- Object Type: WorldStatic
- Preset: `BlockAllDynamic`

**Debris:**
- Collision Enabled: Query Only
- Object Type: WorldStatic
- Preset: `OverlapAllDynamic` or `NoCollision` (if decorative)

### Complex Collision

For detailed meshes:
- Details → Collision Complexity: `Use Complex Collision As Simple`
- Enables per-poly collision (slower, but accurate for terrain)

---

## Troubleshooting

### Assets Not Importing

**Check:**
1. Python plugin enabled?
2. UE5 Output Log for errors
3. File paths correct in config.yaml
4. FBX files not corrupted (open in Houdini/Blender)

**Fix:**
- Re-export from Houdini
- Try manual import first
- Check file permissions

### Nanite Not Enabling

**Requirements:**
- UE5.1+ (5.0 has limited support)
- Mesh has valid UVs
- Mesh is not skeletal

**Fix:**
- Re-export with UVs
- Check "Generate Lightmap UVs" on import

### Materials Not Assigned

**Check:**
1. Master material exists at configured path
2. Material is compatible (not transparent)
3. Static mesh has material slots

**Fix:**
- Create material if missing
- Manually assign first, then retry auto-import

### HISMs Not Appearing

**Check:**
1. Instances are in level bounds
2. HISM component is visible (not hidden)
3. Transforms are valid (no NaN)

**Debug:**
- Check instance count: `print(hism_component.get_instance_count())`
- Visualize bounds in editor

### Performance Issues

**Optimize:**
1. Use Nanite for terrain (auto LOD)
2. Use HISMs for debris (GPU instancing)
3. Build HLODs for distant clusters
4. Enable occlusion culling

**Profile:**
- Stat FPS
- Stat Unit
- Stat SceneRendering

---

## Best Practices

### Asset Organization

```
/Game/Verdun/
  /Tiles/
    /N13/
      N13_terrain (Static Mesh)
      N13_trenches (Static Mesh)
      ...
  /Materials/
    M_TerrainMaster
    /Instances/
      MI_N13
  /Props/
    /Debris/
    /Buildings/
```

### Naming Conventions

- Tiles: `N13_terrain`, `N13_trenches`
- Materials: `M_` prefix (material), `MI_` (instance)
- Textures: `T_` prefix
- HISMs: `HISM_N13_WireReel`

### Version Control

- Use Perforce or Git LFS for assets
- Track auto_ingest results JSON for audit trail
- Tag published assets with tile ID + date

### Testing

Before publishing:
1. ✓ Static mesh opens without errors
2. ✓ Materials assigned correctly
3. ✓ Collision works (test in PIE)
4. ✓ No missing dependencies
5. ✓ Performance acceptable (>30 FPS in target area)

---

## Maintenance Scripts

### Cleanup Stale Assets

```bash
python orchestrator/foreman.py ue maintenance
```

Runs:
- Fix redirectors
- Clean unused assets (dry run by default)
- Save dirty packages

### Rebuild Everything

```bash
# Rebuild HLODs
python orchestrator/foreman.py ue rebuild-hlod

# Rebuild navmesh
python orchestrator/foreman.py ue maintenance
```

---

## Advanced Topics

### Custom Import Pipeline

Extend `auto_ingest.py` for custom logic:

```python
class CustomImporter(VerdunAssetImporter):
    def post_import_hook(self, asset):
        # Add custom metadata
        asset.set_editor_property("asset_user_data", "Pipeline v1.0")
```

### Batch Re-Import

Re-import all tiles:

```python
import auto_ingest
importer = auto_ingest.VerdunAssetImporter(config)
for tile_id in ["N13", "N14", "N15"]:
    importer.import_tile(tile_id, staging_dir)
```

### Python Commandlet

Create a custom commandlet for advanced workflows:

```cpp
// In C++, create UVerdunImportCommandlet
// Call from command line:
// UnrealEditor-Cmd.exe -run=VerdunImport -tile=N13
```

---

## Resources

- [UE5 Python API](https://docs.unrealengine.com/5.3/en-US/PythonAPI/)
- [Nanite Documentation](https://docs.unrealengine.com/5.3/en-US/nanite-virtualized-geometry-in-unreal-engine/)
- [HISM Documentation](https://docs.unrealengine.com/5.3/en-US/instanced-static-meshes-in-unreal-engine/)

For pipeline-specific questions, check `orchestrator/logs/` or consult HDA_SPEC.md.
