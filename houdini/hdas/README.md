# Verdun Pipeline HDAs

This directory contains the Houdini Digital Assets (HDAs) for the Verdun world generation pipeline.

## HDAs Required

### 1. TrenchBuilder.hda
**Purpose:** Generate trench geometry from centerline curves.

**Inputs:**
- Base heightfield (terrain)
- Trench curves (polylines) with width/depth point attributes
- Revetment style parameter

**Outputs:**
- Trench mesh geometry
- Updated heightfield with trench cuts
- Weight maps (mud, wood, water)
- Spline for duckboard placement
- Collision mesh

**Key Parameters:**
- `width`: Default trench width (m)
- `depth`: Default trench depth (m)
- `revetment_style`: Enum (sandbag, wood, gabion, earth)
- `spoil_height`: Height of spoil banks (m)
- `duckboard_spacing`: Spacing between duckboards (m)
- `drainage`: Enable drainage channels
- `side`: french/german/neutral

**Attributes Created:**
- See `docs/HDA_SPEC.md` for full attribute schema

---

### 2. ShellField.hda
**Purpose:** Generate shell craters from density rasters or point clouds.

**Inputs:**
- Base heightfield
- Shell density raster OR point cloud with crater positions
- Crater distribution parameters

**Outputs:**
- Deformed heightfield with craters
- Water mask (for flooded craters)
- Mud mask
- Debris socket points for scatter

**Key Parameters:**
- `density_scale`: Multiplier for crater density
- `min_radius`: Minimum crater radius (m)
- `max_radius`: Maximum crater radius (m)
- `water_threshold`: Terrain depression threshold for water
- `season`: Enum (spring, summer, autumn, winter)
- `seed`: Random seed

**Attributes Created:**
- `crater_radius`, `crater_depth`, `has_water`, `water_depth`

---

### 3. DebrisScatter.hda
**Purpose:** Scatter debris props using masks and catalogs.

**Inputs:**
- Scatter masks (crater lips, trench edges, etc.)
- Debris catalog CSV
- Avoidance areas

**Outputs:**
- Packed primitive instances with transforms
- Instance catalog CSV for UE5

**Key Parameters:**
- `density_scale`: Overall density multiplier
- `catalog_path`: Path to debris CSV
- `min_spacing`: Minimum distance between instances
- `seed`: Random seed
- `avoid_player_paths`: Boolean

**Attributes Created:**
- `asset_path`, `scale_variance`, `rot_variance`, `burial_depth`

---

### 4. BuildingRuinizer.hda
**Purpose:** Procedurally damage buildings based on artillery hits.

**Inputs:**
- Building footprint polygons
- Damage level (0=intact, 1=destroyed)
- Hit direction vector

**Outputs:**
- Damaged building meshes
- Rubble field geometry
- Material override attributes

**Key Parameters:**
- `damage_level`: Float 0-1
- `building_type`: Enum (house, church, barn, factory)
- `hit_direction`: Vector indicating damage direction
- `rubble_scale`: Multiplier for rubble amount
- `pre_fracture`: Use pre-fractured assets

**Attributes Created:**
- `damage_level`, `building_type`, `hit_direction`, `has_rubble`

---

## Building HDAs

These HDAs should be built in Houdini with the following considerations:

1. **Parameterization:** All key controls exposed as parameters
2. **Randomization:** Use seedable random for repeatability
3. **Attributes:** Follow the attribute schema in `docs/HDA_SPEC.md`
4. **Performance:** Optimize for batch processing (compiled blocks, etc.)
5. **Validation:** Include error checking and warnings
6. **Documentation:** Embed help text and parameter tooltips

## Installation

HDAs are auto-loaded by the Python scripts. Ensure they are named exactly as specified:
- `TrenchBuilder.hda`
- `ShellField.hda`
- `DebrisScatter.hda`
- `BuildingRuinizer.hda`

Place them in this directory (`houdini/hdas/`).

## Testing

Test HDAs individually before integrating into PDG:

```bash
hython houdini/python/cook_trenches.py \
    --curves data/incoming/vectors/N13_trenches.geojson \
    --dem data/incoming/rasters/N13.tif \
    --out data/staging/N13_test
```
