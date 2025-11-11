# Verdun Pipeline HDA Specifications

Detailed specifications for the four Houdini Digital Assets.

---

## Overview

All HDAs follow these conventions:

- **Parameterization:** All controls exposed as parameters
- **Attributes:** Follow schema in attribute section below
- **Random Seeding:** All randomization uses a `seed` parameter
- **Performance:** Optimized for batch processing
- **Validation:** Include error checking and warnings

---

## 1. TrenchBuilder.hda

**Purpose:** Generate trench geometry with revetments, spoil banks, and drainage.

### Inputs

| Input | Type | Description |
|-------|------|-------------|
| 0 | Heightfield | Base terrain (height layer) |
| 1 | Curves | Trench centerlines with point attributes |
| 2 | Mask (optional) | Avoidance areas (no trenches) |

### Parameters

#### Basic Settings
- `width` (float, default: 1.8): Default trench width in meters
- `depth` (float, default: 2.5): Trench depth in meters
- `use_curve_attrs` (toggle): Use width/depth from curve point attributes

#### Revetment Style
- `revetment_type` (enum):
  - 0: Earth (no structure)
  - 1: Sandbags
  - 2: Wood planks
  - 3: Gabion/wicker

#### Spoil Banks
- `enable_spoil` (toggle): Create spoil banks
- `spoil_height` (float, default: 0.8): Height of spoil in meters
- `spoil_offset` (float, default: 0.5): Distance from trench edge

#### Duckboards
- `enable_duckboards` (toggle): Generate duckboard splines
- `duckboard_spacing` (float, default: 2.0): Spacing between boards
- `duckboard_width` (float, default: 0.4): Board width

#### Drainage
- `enable_drainage` (toggle): Add drainage channels
- `drainage_depth` (float, default: 0.3): Channel depth

#### Detail
- `profile_resolution` (int, default: 8): Cross-section resolution
- `length_resolution` (float, default: 0.5): Along-curve resolution in meters

#### Randomization
- `seed` (int, default: 1234): Random seed
- `width_variance` (float, default: 0.1): Width randomization ±
- `profile_noise` (float, default: 0.05): Profile irregularity

### Outputs

| Output | Type | Description |
|--------|------|-------------|
| 0 | Geometry | Trench mesh (with collision) |
| 1 | Heightfield | Updated terrain with trench cuts |
| 2 | Curves | Duckboard splines |
| 3 | Volume | Mud mask (for materials) |

### Attributes Created

**Point Attributes:**
- `trench_width` (float): Actual width at this point
- `trench_depth` (float): Actual depth
- `mudiness` (float): 0-1, how muddy (from drainage)

**Primitive Attributes:**
- `revetment_type` (int): Which revetment style used
- `has_duckboards` (int): 1 if duckboards present
- `side` (string): "french", "german", or "neutral"

### Implementation Notes

1. **Resample curves** to ensure even spacing
2. **Create profile** (rectangle with rounded bottom)
3. **Sweep profile** along curve with banking on turns
4. **Boolean subtract** from heightfield
5. **Generate revetment** geometry based on type
6. **Add spoil banks** using displaced mesh
7. **Output layer masks** (mud, wood, water)

---

## 2. ShellField.hda

**Purpose:** Generate shell craters from density data or point clouds.

### Inputs

| Input | Type | Description |
|-------|------|-------------|
| 0 | Heightfield | Base terrain |
| 1 | Raster OR Points | Shell density map or explicit crater points |
| 2 | Mask (optional) | Protected areas (no craters) |

### Parameters

#### Crater Generation
- `input_mode` (enum):
  - 0: Density raster (automatic placement)
  - 1: Point cloud (explicit positions)

- `density_scale` (float, default: 1.0): Overall density multiplier

#### Crater Size Distribution
- `min_radius` (float, default: 2.0): Minimum crater radius in meters
- `max_radius` (float, default: 15.0): Maximum crater radius
- `radius_distribution` (enum):
  - 0: Uniform
  - 1: Gaussian (most craters mid-size)
  - 2: Power law (many small, few large)

#### Crater Shape
- `depth_ratio` (float, default: 0.15): Depth as fraction of radius
- `lip_height_ratio` (float, default: 0.1): Lip height as fraction of depth
- `profile_falloff` (float, default: 2.0): Crater profile sharpness

#### Water Filling
- `enable_water` (toggle): Fill craters with water
- `water_threshold` (float, default: 0.3): Depression threshold (0-1)
- `season` (enum):
  - 0: Spring (high water)
  - 1: Summer (low water)
  - 2: Autumn (moderate)
  - 3: Winter (frozen/snow)

#### Randomization
- `seed` (int, default: 5678): Random seed
- `position_jitter` (float, default: 0.5): Position randomization in meters

### Outputs

| Output | Type | Description |
|--------|------|-------------|
| 0 | Heightfield | Deformed terrain with craters |
| 1 | Volume | Water mask (for shader) |
| 2 | Volume | Mud mask (crater interiors) |
| 3 | Points | Debris socket points (for scatter) |

### Attributes Created

**Point Attributes (on debris sockets):**
- `crater_radius` (float): Crater radius
- `crater_depth` (float): Crater depth
- `has_water` (int): 1 if water-filled
- `water_depth` (float): Depth of water in meters
- `age_days` (float): Simulated age (affects weathering)
- `shell_type` (string): Ordnance type (e.g., "155mm", "77mm")

### Implementation Notes

1. **Scatter points** from density raster or use input points
2. **Assign radii** from distribution
3. **Create crater SDFs** for each point
4. **Merge SDFs** with heightfield
5. **Generate lip geometry** via displacement
6. **Fill water** based on terrain analysis (local minima)
7. **Place debris sockets** on crater lips (60% coverage)

---

## 3. DebrisScatter.hda

**Purpose:** Scatter debris props using masks and asset catalogs.

### Inputs

| Input | Type | Description |
|-------|------|-------------|
| 0 | Heightfield | Terrain for height sampling |
| 1 | Volume | Scatter mask (high = more debris) |
| 2 | Curves (optional) | Avoidance paths |

### Parameters

#### Scatter Settings
- `density_scale` (float, default: 1.0): Overall density multiplier
- `min_spacing` (float, default: 0.5): Minimum distance between instances
- `catalog_path` (file): Path to debris catalog CSV

#### Placement
- `conform_to_terrain` (toggle): Align to terrain normals
- `burial_depth` (float, default: 0.2): How much to sink into ground
- `avoid_paths` (toggle): Avoid placement on curves input

#### Randomization
- `seed` (int, default: 9012): Random seed
- `scale_variance` (float, default: 0.2): Scale randomization ±
- `rotation_variance` (vector, default: [0, 360, 0]): Rotation variance XYZ

### Outputs

| Output | Type | Description |
|--------|------|-------------|
| 0 | Packed Primitives | Instance points with transforms |
| 1 | CSV | Instance catalog for UE5 import |

### Attributes Created

**Point Attributes:**
- `asset_path` (string): Path to debris asset (from catalog)
- `scale_variance` (float): Applied scale factor
- `rot_variance` (vector): Applied rotation
- `burial_depth` (float): How buried in terrain
- `material_override` (string): Optional material name

### Catalog Format (CSV)

```csv
asset_path,weight,min_scale,max_scale,tags
/Game/Props/Debris/WireReel.uasset,1.0,0.8,1.2,"metal industrial"
/Game/Props/Debris/Sandbag_Torn.uasset,2.0,0.9,1.1,"sandbag common"
/Game/Props/Debris/WoodPlank.uasset,3.0,1.0,1.5,"wood common"
```

### Implementation Notes

1. **Load catalog** CSV
2. **Scatter points** from mask using `scatter` SOP
3. **Apply spacing** constraint
4. **Assign assets** from catalog (weighted random)
5. **Conform to terrain** (sample height, compute normal)
6. **Apply burial** (offset down by burial_depth)
7. **Randomize transforms** (scale, rotation)
8. **Export CSV** for UE5 with transforms and asset paths

---

## 4. BuildingRuinizer.hda

**Purpose:** Procedurally damage buildings based on artillery hits.

### Inputs

| Input | Type | Description |
|-------|------|-------------|
| 0 | Polygons | Building footprint polygons |
| 1 | Points (optional) | Hit locations and directions |

### Parameters

#### Damage Settings
- `damage_level` (float, default: 0.5): Overall damage 0=intact to 1=destroyed
- `building_type` (enum):
  - 0: House (2 stories)
  - 1: Church
  - 2: Barn
  - 3: Factory
  - 4: Generic

#### Procedural Damage
- `enable_procedural` (toggle): Auto-generate walls/roof
- `wall_height` (float, default: 2.5): Wall height per story
- `floor_count` (int, default: 2): Number of stories

#### Hit Pattern
- `hit_direction` (vector, default: [1, 0, 0]): Primary damage direction
- `hit_variance` (float, default: 30.0): Cone variance in degrees
- `hit_count` (int, default: 5): Number of simulated hits

#### Rubble
- `enable_rubble` (toggle): Generate rubble field
- `rubble_scale` (float, default: 1.0): Amount of rubble
- `rubble_radius` (float, default: 5.0): Spread radius

#### Randomization
- `seed` (int, default: 3456): Random seed
- `pre_fracture` (toggle): Use pre-fractured assets

### Outputs

| Output | Type | Description |
|--------|------|-------------|
| 0 | Geometry | Damaged building meshes |
| 1 | Geometry | Rubble field |
| 2 | Volume | Material mask (burnt areas) |

### Attributes Created

**Primitive Attributes:**
- `damage_level` (float): Actual damage applied
- `building_type` (string): Type of building
- `hit_direction` (vector): Primary damage vector
- `has_rubble` (int): 1 if rubble present

**Detail Attributes:**
- `floor_count` (int): Number of stories

### Implementation Notes

1. **Extrude walls** from footprint
2. **Add floors and roof**
3. **Generate hit points** based on direction and count
4. **Boolean damage** (subtract spheres/cones at hits)
5. **Fracture remaining** geometry (optional)
6. **Generate rubble** (shattered pieces)
7. **Scatter rubble** around building base
8. **Apply burnt material** mask near hits

---

## Attribute Schema Reference

### Standard Attributes Across All HDAs

All geometry should include:

```
P (vector3) - Position
N (vector3) - Normal
uv (vector3) - UV coordinates (if textured)
```

### Layer Mask Naming

Weight maps output from HDAs use these names:

- `layer_mud` - Mud material weight
- `layer_water` - Water weight
- `layer_wood` - Wood/duckboard weight
- `layer_metal` - Metal debris weight
- `layer_grass` - Grass weight
- `layer_blood` - Blood stain weight (for trenches)

These map to UE5 landscape layers.

---

## Testing HDAs

### Unit Testing

Test each HDA individually before integration:

```bash
hython houdini/python/test_hda.py --hda TrenchBuilder.hda --test-scene test_trenches.hip
```

### Performance Profiling

Profile cook times:

1. Open Houdini Performance Monitor
2. Cook HDA
3. Review bottlenecks
4. Optimize slow operations (use Compiled Blocks, attribute wrangle)

### Validation

Each HDA should validate:
- Input geometry exists
- Parameters in valid ranges
- Output attributes match schema
- No NaN or infinite values
- Geometry is watertight (for collision)

Add validation nodes before output with Python:

```python
geo = hou.pwd().geometry()
if geo.points():
    hou.ui.displayMessage("✓ Geometry generated")
else:
    raise hou.NodeError("No geometry output!")
```

---

## Version Control for HDAs

HDAs are binary files, but track them in Git:

```bash
git add houdini/hdas/TrenchBuilder.hda
git commit -m "TrenchBuilder v1.2: Add drainage channels"
```

Keep a changelog in HDA Extra Files → README:

```
v1.2 - 2025-01-15
- Added drainage channel option
- Fixed spoil bank normals
- Optimized for PDG batch processing

v1.1 - 2025-01-10
- Initial working version
```

---

## Resources

- [Houdini HDA Documentation](https://www.sidefx.com/docs/houdini/assets/index.html)
- [PDG/TOPs Guide](https://www.sidefx.com/docs/houdini/tops/index.html)
- [Heightfield Workflows](https://www.sidefx.com/docs/houdini/nodes/sop/heightfield.html)

For further customization, consult the full Houdini documentation.
