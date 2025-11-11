# Houdini Import Guide - Verdun Positional Data

## Overview

This guide shows how to import the Verdun dataset into SideFX Houdini for procedural generation of trenches, buildings, trees, roads, and terrain.

## Why Houdini is Perfect for This Data

Houdini excels at:
- ✅ **Point clouds from coordinates** (villages, forts, trees)
- ✅ **Curve generation from GPS points** (trenches, roads)
- ✅ **Procedural scattering** (trees, buildings, fields)
- ✅ **Copy to points** (instancing buildings on scattered points)
- ✅ **Heightfield terrain** (from elevation data)
- ✅ **Rule-based systems** (village layouts, field patterns)

Our data provides EXACT coordinates and procedural rules for everything.

---

## Import Workflows

### 1. TRENCHES - Spline Generation from Coordinates

**Data Available:**
- Exact GPS coordinates for 3 defensive lines
- Left bank line: 4 points
- Right bank line: 3 points
- Specifications: depth, width, zig-zag pattern

**Houdini Workflow:**

```
1. Create Geometry Node
2. Add "File" node → Import trench_coordinates.csv
3. Add "Add" node → Create polyline from points (Connect Adjacent)
4. Add "Resample" node → Even spacing every 1 meter
5. Add "Mountain" node → Add zig-zag (Perlin noise, 5m amplitude, 10-15m frequency)
6. Create trench profile:
   - Top: 1m wide
   - Bottom: 0.8m wide
   - Depth: 1.5m
   - Firing step on one side
7. Add "Sweep" node → Extrude profile along curve
8. Add "PolyExtrude" node → Create trench geometry
9. Boolean subtract from terrain heightfield
```

**VEX for Zig-Zag Pattern:**
```c
// On points after resample
float offset = fit01(noise(@ptnum * 0.1), -2.5, 2.5); // Zig-zag 5m total
vector tangent = normalize(@P - point(0, "P", @ptnum-1));
vector normal = cross(tangent, {0,1,0});
@P += normal * offset;
```

**Add Dugouts Every 30-50m:**
```c
// Create points for dugouts
if (@ptnum % 30 == 0) {  // Every 30 points (30m)
    // Copy dugout geometry here
}
```

---

### 2. VILLAGES - Procedural Layout from Rules

**Data Available:**
- Village center GPS coordinates
- Main street length & orientation
- Building counts per side
- Building spacing (2-4m)
- Building dimensions & types
- Garden depths (10-20m)
- Farm locations (perimeter)

**Houdini Workflow for Fleury-devant-Douaumont:**

```
1. Import village center point (49.198109, 5.427765)
2. Convert to UE5 coords → (349653, 323167, 10500)

3. CREATE MAIN STREET:
   - Line node: 500m long, NW-SE orientation (rotate 135°)
   - Start at village center

4. SCATTER HOUSES ALONG STREET:
   - Resample street: 68 points (34 each side)
   - Spacing: random 2-4m
   - Offset north side: +8m perpendicular
   - Offset south side: -8m perpendicular
   - Orient to face street: @N = normalize(street_center - @P)

5. COPY HOUSE GEOMETRY:
   - Copy to Points node
   - Random selection of house types:
     * maison_lorraine_type1.fbx (40%)
     * maison_lorraine_type2.fbx (35%)
     * ferme_small.fbx (25%)
   - Random rotation: ±5° variance
   - Random scale: 0.9-1.1

6. ADD GARDENS:
   - For each house point
   - Extrude box behind house: 10m wide × random(10-20)m deep
   - Add garden props (vegetables, tools, chicken coops)

7. PLACE CHURCH:
   - At village center (0, 0 offset from origin)
   - Add church square: 40m × 30m polygon
   - Copy church_romanesque.fbx

8. SCATTER FARMS ON PERIMETER:
   - Create circle: radius 175m (half of 350m diameter)
   - Scatter 12 points on circle
   - Copy ferme_lorraine.fbx (U-shaped courtyard farm)
   - Scale: 1.2-1.5 (farms are larger)
```

**VEX for House Placement:**
```c
// After scattering points along street
// Orient houses to face street center
vector to_center = normalize({0,0,0} - @P);
@N = to_center;
@up = {0,1,0};

// Vary rotation slightly
float random_rot = fit01(rand(@ptnum), -5, 5);
@orient = quaternion(maketransform(@N, @up));
@orient = qrotate(@orient, radians(random_rot), {0,1,0});

// Assign building type
float type = fit01(rand(@ptnum * 123), 0, 1);
if (type < 0.4) s@building_type = "maison_type1";
else if (type < 0.75) s@building_type = "maison_type2";
else s@building_type = "ferme_small";
```

---

### 3. TREES - Density-Based Scattering

**Data Available:**
- Forest boundary coordinates
- Area in hectares
- Trees per hectare (180-250)
- Tree spacing (4-8m)
- Tree composition percentages
- Total tree count estimate

**Houdini Workflow for Bois des Caures:**

```
1. CREATE FOREST BOUNDARY:
   - Import forest corner coordinates
   - Create polygon: 1,800m × 1,200m
   - Position at (49.25, 5.41) → convert to UE5

2. SCATTER TREE POINTS:
   - Scatter node inside polygon
   - Total Points: 45,000 (average of 40k-50k estimate)
   - Relax Points: Yes (minimum distance 4m)
   - Add noise for natural variation

3. ASSIGN TREE TYPES:
   - Random float per point
   - 0.0-0.4 → Oak (40%)
   - 0.4-0.75 → Beech (35%)
   - 0.75-0.9 → Hornbeam (15%)
   - 0.9-1.0 → Other (10%)

4. COPY TREE GEOMETRY:
   - Copy to Points node
   - Switch based on tree type attribute
   - Random Y rotation: 0-360°
   - Random scale: 0.8-1.2 (height variation 15-25m)

5. ADD UNDERGROWTH:
   - Second scatter layer
   - Higher density: 500-1000 points/hectare
   - Small bushes, ferns, ground plants
```

**VEX for Tree Type Assignment:**
```c
// Assign tree species based on percentages
float rand_val = fit01(rand(@ptnum), 0, 1);

if (rand_val < 0.40) {
    s@species = "oak";
    s@model_path = "trees/oak_mature.fbx";
    @pscale = fit01(rand(@ptnum * 456), 0.9, 1.2); // 18-25m height
}
else if (rand_val < 0.75) {
    s@species = "beech";
    s@model_path = "trees/beech_mature.fbx";
    @pscale = fit01(rand(@ptnum * 789), 0.85, 1.15);
}
else if (rand_val < 0.90) {
    s@species = "hornbeam";
    s@model_path = "trees/hornbeam.fbx";
    @pscale = fit01(rand(@ptnum * 321), 0.8, 1.0);
}
else {
    s@species = "mixed";
    s@model_path = "trees/deciduous_mixed.fbx";
    @pscale = fit01(rand(@ptnum * 654), 0.85, 1.1);
}

// Random rotation
@orient = quaternion(maketransform({0,0,1}, {0,1,0}));
float rot = fit01(rand(@ptnum * 999), 0, 360);
@orient = qrotate(@orient, radians(rot), {0,1,0});
```

---

### 4. ROADS - Spline Networks from Waypoints

**Data Available:**
- La Voie Sacrée: 6 waypoints with coordinates
- Primary roads: Start/end points
- Secondary roads: Start/end points
- Width specifications
- Surface types

**Houdini Workflow:**

```
1. IMPORT ROAD WAYPOINTS:
   - File node → roads_waypoints.csv
   - Each road has ID attribute

2. CREATE ROAD SPLINES:
   - Group by road ID
   - Add node → Create polyline per group
   - Resample for smooth curves (every 5m)

3. CREATE ROAD PROFILE:
   - La Voie Sacrée: 5m wide, slightly crowned
   - Primary roads: 6m wide, macadam surface
   - Secondary roads: 4m wide, dirt texture
   - Farm tracks: 3m wide, two ruts with grass center

4. SWEEP PROFILE ALONG SPLINE:
   - Sweep node
   - Road cross-section → along curve
   - UV mapping for textures

5. ADD ROAD DETAILS:
   - Ruts for dirt roads (noise displacement)
   - Cobblestones for paved sections
   - Wear patterns at intersections
```

---

### 5. AGRICULTURAL FIELDS - Procedural Placement

**Data Available:**
- 3-6 wheat fields per village (100-200m diameter)
- Field types (wheat, barley, fallow, orchards)
- Kitchen gardens behind each house
- Field rotation patterns

**Houdini Workflow:**

```
1. CREATE FIELD ZONES AROUND VILLAGE:
   - Circle around village center (radius 400m)
   - Exclude village footprint (radius 200m)
   - Voronoi scatter: 5 cells → fields

2. ASSIGN FIELD TYPES:
   - Random per cell
   - 40% wheat fields
   - 20% barley/oats
   - 33% fallow (grass)
   - 7% orchards

3. APPLY FIELD TEXTURES:
   - Wheat: bare earth or green sprouts (February)
   - Fallow: grass texture
   - Orchards: scatter trees 6-8m apart

4. KITCHEN GARDENS:
   - Already created behind each house
   - Size: 10m × 15m
   - Texture: dark tilled soil
   - Optional: add vegetable rows
```

---

### 6. FORTIFICATIONS - Precise Placement

**Data Available:**
- 27 forts with exact GPS coordinates
- Fort dimensions (length × width × height)
- Fort types (masonry, concrete, ouvrages)

**Houdini Workflow:**

```
1. IMPORT FORT COORDINATES:
   - File node → all_forts_complete_ue5.csv
   - Creates 27 points

2. COPY FORT MODELS:
   - Copy to Points node
   - Switch based on fort type:
     * inner_ring → fort_masonry_1870s.fbx
     * outer_ring → fort_masonry_1880s.fbx
     * modern (Vacherauville, La Falouse) → fort_concrete_modern.fbx
     * ouvrage → ouvrage_fortification.fbx

3. SCALE TO DIMENSIONS:
   - Read length/width from CSV attributes
   - @pscale = length from data / model length

4. ORIENT TO TERRAIN:
   - Ray to terrain heightfield
   - Align @N to terrain normal
   - @up = terrain normal
```

---

### 7. TERRAIN HEIGHTFIELD - From Elevation Data

**Data Available:**
- Elevation for all points (188m - 392m)
- Hills: Mort-Homme (295m), Côte 304 (304m)
- Fort elevations
- Village elevations
- River valley (Meuse at 200m)

**Houdini Workflow:**

```
1. CREATE BASE HEIGHTFIELD:
   - HeightField node: 60km × 30km
   - Resolution: 1m per pixel (high detail)

2. IMPORT ELEVATION POINTS:
   - All coordinates with elevation data
   - Scatter into heightfield

3. INTERPOLATE HEIGHT:
   - HeightField Scatter node
   - Interpolate between known elevation points
   - Smooth with HeightField Blur (natural terrain)

4. ADD TERRAIN FEATURES:
   - Mort-Homme: HeightField Draw (295m peak)
   - Côte 304: HeightField Draw (304m peak)
   - Meuse Valley: HeightField Draw (carved channel at 200m)
   - Ravines: HeightField Noise (steep valleys)

5. EROSION (Optional):
   - HeightField Erode
   - Simulates natural water erosion
   - Creates realistic slope patterns
```

---

## Complete Houdini Project Structure

```
verdun_battlefield.hip
├── obj/
│   ├── terrain/
│   │   ├── heightfield (from elevation data)
│   │   ├── meuse_river (carved valley)
│   │   └── ravines (noise-based)
│   │
│   ├── fortifications/
│   │   ├── import_fort_coords (CSV → points)
│   │   ├── copy_fort_models (instance geometry)
│   │   └── 27 forts placed
│   │
│   ├── villages/
│   │   ├── fleury/ (procedural layout)
│   │   ├── ornes/ (procedural layout)
│   │   ├── avocourt/
│   │   └── ... (9 destroyed + 4 left-bank)
│   │
│   ├── forests/
│   │   ├── bois_des_caures/ (45,000 trees scattered)
│   │   ├── bois_haumont/ (60,000 trees)
│   │   ├── bois_bourrus/ (35,000 trees)
│   │   └── undergrowth layer
│   │
│   ├── roads/
│   │   ├── voie_sacree_spline
│   │   ├── primary_roads
│   │   ├── secondary_roads
│   │   └── farm_tracks
│   │
│   ├── trenches/
│   │   ├── first_position_left_bank
│   │   ├── first_position_right_bank
│   │   ├── second_position
│   │   └── communication_boyaux
│   │
│   └── agriculture/
│       ├── field_zones (Voronoi cells)
│       ├── orchards (tree scatter)
│       └── gardens (behind houses)
```

---

## CSV Files for Direct Houdini Import

All CSV files in the dataset can be imported directly:

- `all_forts_complete_ue5.csv` → 27 points with fort data
- `all_villages_complete_ue5.csv` → 13 village centers
- `all_locations_ue5.csv` → Everything
- `all_roads_complete_ue5.csv` → Road waypoints

**File → Import:**
- Import as points
- UE5_X → P.x
- UE5_Y → P.y
- UE5_Z → P.z

---

## Procedural Parameters

All our data translates to Houdini parameters:

| Data | Houdini Parameter |
|------|-------------------|
| Tree density (180-250/hectare) | Scatter: Total Count = area × density |
| Tree spacing (4-8m) | Scatter: Relax Iterations, Min Distance = 4m |
| Building count (68 houses) | Scatter: Total Count = 68 |
| Road width (5m) | Profile: Width = 5m |
| Trench depth (1.5m) | Profile: Depth = 1.5m |
| Forest area (220 hectares) | Polygon: 1,800m × 1,200m |
| Field diameter (100-200m) | Circle: Radius = 50-100m |

---

## Export to UE5

After building in Houdini:

```
1. GEOMETRY EXPORT:
   - ROP FBX Output node
   - Export villages, forts, roads as FBX
   - Maintains UE5 coordinate system

2. HEIGHTFIELD EXPORT:
   - HeightField Output node
   - Export as .tif or .raw
   - Import to UE5 Landscape

3. FOLIAGE POINTS:
   - Export tree scatter points as CSV
   - UE5 can instance foliage at these points
   - Or use Houdini Engine in UE5

4. INSTANCED GEOMETRY:
   - Use Houdini's packed primitives
   - Exports efficiently to UE5
   - Maintains instance optimization
```

---

## Advantages of Houdini Workflow

✅ **Procedural:** Change parameters, regenerate entire battlefield
✅ **Precise:** Uses exact GPS coordinates throughout
✅ **Efficient:** Instance 110,000+ trees with minimal memory
✅ **Flexible:** Easy to adjust village layouts, tree density, etc.
✅ **Accurate:** Maintains historical accuracy through rules
✅ **Scalable:** Can generate entire 60km × 30km battlefield

---

## Example: Complete Village in 10 Minutes

With our data, you can generate Fleury-devant-Douaumont in Houdini:

1. Import village center → 5 seconds
2. Generate main street → 30 seconds
3. Scatter 68 houses → 1 minute
4. Copy building geometry → 2 minutes
5. Add gardens → 1 minute
6. Place church & farms → 2 minutes
7. Tweak & refine → 3 minutes

**Total: ~10 minutes for a complete, historically accurate village!**

Repeat for all 13 villages = full battlefield villages in 2-3 hours.

---

## Summary

**YES - Houdini can use ALL our data precisely!**

- ✅ Trenches: Exact coordinate splines
- ✅ Buildings: Procedural rules + center points
- ✅ Trees: Density scatter within boundaries
- ✅ Roads: Waypoint-based splines
- ✅ Fields: Zone-based placement
- ✅ Forts: Exact GPS positioning
- ✅ Terrain: Elevation-based heightfield

**Everything is coordinate-driven and procedural-ready!**

The data provides both:
1. **Exact positions** (forts, village centers, roads)
2. **Procedural rules** (building layouts, tree scatter, field patterns)

This is the ideal workflow for creating a massive, historically accurate battlefield efficiently.
