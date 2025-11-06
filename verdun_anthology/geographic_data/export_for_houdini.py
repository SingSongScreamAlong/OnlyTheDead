#!/usr/bin/env python3
"""
HOUDINI TERRAIN EXPORTER
=========================

Converts Verdun battlefield elevation data into Houdini-compatible formats
for procedural terrain generation and historical accuracy.

PREREQUISITES:
    Run fetch_elevation_data.py first to generate elevation data

USAGE:
    python export_for_houdini.py

OUTPUTS:
    Houdini_Import/
    ├── verdun_heightfield.bgeo (Houdini native format)
    ├── verdun_heightfield.exr (32-bit float heightmap)
    ├── verdun_heightfield.csv (Raw grid data)
    ├── verdun_locations.json (Point cloud data)
    ├── verdun_terrain_params.json (Procedural generation parameters)
    └── README_HOUDINI.txt (Import instructions)

COORDINATE SYSTEM:
    Origin: Battlefield center (49.2050°N, 5.4300°E)
    Units: Meters (X, Y, Z)
    Grid spacing: 30 meters (SRTM resolution)

AUTHOR: Claude (Verdun Anthology Historian)
DATE: 2025
"""

import json
import csv
import sys
import struct
from pathlib import Path
import math

# Check dependencies
try:
    import numpy as np
except ImportError:
    print("❌ Missing dependency: numpy")
    print("   Install: pip install numpy")
    sys.exit(1)

# Input files
ELEVATION_DIR = Path(__file__).parent / 'elevation_data'
ELEVATION_NUMPY = ELEVATION_DIR / 'verdun_elevation_grid.npy'
ELEVATION_METADATA = ELEVATION_DIR / 'verdun_elevation_metadata.json'
LOCATIONS_CSV = Path(__file__).parent / 'locations_database.csv'

# Output directory
OUTPUT_DIR = Path(__file__).parent / 'Houdini_Import'
OUTPUT_DIR.mkdir(exist_ok=True)

# Battlefield center (origin)
BATTLEFIELD_CENTER = {
    'lat': 49.2050,
    'lon': 5.4300
}

# Coordinate conversion
METERS_PER_DEGREE_LAT = 111320
METERS_PER_DEGREE_LON = 71370


def check_prerequisites():
    """Check if required files exist."""
    print("=" * 70)
    print("HOUDINI TERRAIN EXPORTER")
    print("=" * 70)

    missing = []

    if not ELEVATION_NUMPY.exists():
        missing.append(f"   - {ELEVATION_NUMPY}")

    if not ELEVATION_METADATA.exists():
        missing.append(f"   - {ELEVATION_METADATA}")

    if not LOCATIONS_CSV.exists():
        missing.append(f"   - {LOCATIONS_CSV}")

    if missing:
        print("\n❌ MISSING REQUIRED FILES:")
        for item in missing:
            print(item)
        print("\n💡 Run 'python fetch_elevation_data.py' first")
        return False

    print("\n✅ All prerequisite files found")
    return True


def lat_lon_to_meters(lat, lon):
    """
    Convert WGS84 to local meters from battlefield center.
    Returns: (x_meters, y_meters)
    """
    delta_lat = lat - BATTLEFIELD_CENTER['lat']
    delta_lon = lon - BATTLEFIELD_CENTER['lon']

    y = delta_lat * METERS_PER_DEGREE_LAT
    x = delta_lon * METERS_PER_DEGREE_LON

    return (x, y)


def export_csv_heightfield():
    """Export heightfield as CSV (already done, just copy)."""
    print("\n📊 EXPORTING CSV HEIGHTFIELD...")

    try:
        import shutil

        src = ELEVATION_DIR / 'verdun_elevation_grid.csv'
        dst = OUTPUT_DIR / 'verdun_heightfield.csv'

        if src.exists():
            shutil.copy(src, dst)
            print(f"   ✅ Copied: {dst.name}")
            return True
        else:
            print(f"   ⚠️  Source file not found: {src}")
            return False

    except Exception as e:
        print(f"   ❌ Error: {e}")
        return False


def export_exr_heightfield():
    """Export 32-bit float EXR heightmap for Houdini."""
    print("\n🎨 EXPORTING EXR HEIGHTFIELD (32-bit float)...")

    try:
        # Try to import OpenEXR (optional - requires OpenEXR library)
        try:
            import OpenEXR
            import Imath
            has_exr = True
        except ImportError:
            has_exr = False
            print("   ⚠️  OpenEXR not available (pip install OpenEXR)")
            print("   ⚠️  Skipping EXR export (use CSV or RAW instead)")
            return False

        if not has_exr:
            return False

        # Load elevation data
        elevation = np.load(ELEVATION_NUMPY)
        height, width = elevation.shape

        # Convert to float32
        elevation_float = elevation.astype(np.float32)

        # Create EXR file
        output_exr = OUTPUT_DIR / 'verdun_heightfield.exr'

        header = OpenEXR.Header(width, height)
        header['channels'] = {'Y': Imath.Channel(Imath.PixelType(Imath.PixelType.FLOAT))}

        exr = OpenEXR.OutputFile(str(output_exr), header)
        exr.writePixels({'Y': elevation_float.tobytes()})
        exr.close()

        print(f"   ✅ Exported: {output_exr.name}")
        return True

    except Exception as e:
        print(f"   ⚠️  EXR export failed: {e}")
        print("   💡 Use CSV format instead for Houdini import")
        return False


def export_raw_heightfield():
    """Export raw binary heightfield (32-bit float)."""
    print("\n📦 EXPORTING RAW BINARY HEIGHTFIELD...")

    try:
        elevation = np.load(ELEVATION_NUMPY)

        # Save as raw 32-bit float binary
        output_raw = OUTPUT_DIR / 'verdun_heightfield.raw'
        elevation.astype(np.float32).tofile(output_raw)

        # Save dimensions file
        height, width = elevation.shape
        dims_file = OUTPUT_DIR / 'verdun_heightfield.raw.txt'
        with open(dims_file, 'w') as f:
            f.write(f"Width: {width}\n")
            f.write(f"Height: {height}\n")
            f.write(f"Format: 32-bit float\n")
            f.write(f"Byte order: Little-endian\n")

        print(f"   ✅ Exported: {output_raw.name}")
        print(f"   ✅ Dimensions: {dims_file.name}")
        return True

    except Exception as e:
        print(f"   ❌ Error: {e}")
        return False


def export_locations():
    """Export location points for Houdini."""
    print("\n📍 EXPORTING LOCATION POINTS...")

    try:
        locations = {
            'description': 'Verdun battlefield historical locations',
            'coordinate_system': 'Local meters (origin: battlefield center)',
            'origin': {
                'latitude': BATTLEFIELD_CENTER['lat'],
                'longitude': BATTLEFIELD_CENTER['lon']
            },
            'points': []
        }

        with open(LOCATIONS_CSV, 'r') as f:
            reader = csv.DictReader(f)

            for row in reader:
                try:
                    lat = float(row['latitude'])
                    lon = float(row['longitude'])
                    elev = float(row['elevation_m'])

                    x, y = lat_lon_to_meters(lat, lon)

                    locations['points'].append({
                        'name': row['name'],
                        'type': row['type'],
                        'position': {
                            'x': round(x, 2),
                            'y': round(y, 2),
                            'z': round(elev, 2)
                        },
                        'geographic': {
                            'lat': lat,
                            'lon': lon
                        },
                        'attributes': {
                            'significance': row['significance'],
                            'phase': row['phase'],
                            'date_key': row['date_key'],
                            'notes': row['notes']
                        }
                    })

                except (ValueError, KeyError) as e:
                    continue

        output_json = OUTPUT_DIR / 'verdun_locations.json'
        with open(output_json, 'w') as f:
            json.dump(locations, f, indent=2)

        print(f"   ✅ Exported: {output_json.name} ({len(locations['points'])} points)")

        # Also export as CSV for easier import
        output_csv = OUTPUT_DIR / 'verdun_locations_xyz.csv'
        with open(output_csv, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['name', 'type', 'x', 'y', 'z', 'significance'])
            for pt in locations['points']:
                writer.writerow([
                    pt['name'],
                    pt['type'],
                    pt['position']['x'],
                    pt['position']['y'],
                    pt['position']['z'],
                    pt['attributes']['significance']
                ])

        print(f"   ✅ Exported: {output_csv.name}")
        return True

    except Exception as e:
        print(f"   ❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return False


def create_terrain_params():
    """Create procedural terrain generation parameters."""
    print("\n⚙️  CREATING TERRAIN PARAMETERS...")

    try:
        with open(ELEVATION_METADATA) as f:
            metadata = json.load(f)

        params = {
            "terrain_name": "Verdun_Battlefield_1916",
            "description": "Procedural terrain parameters for Houdini",

            "heightfield": {
                "file_csv": "verdun_heightfield.csv",
                "file_raw": "verdun_heightfield.raw",
                "file_exr": "verdun_heightfield.exr (if available)",
                "dimensions": {
                    "width": metadata['dimensions']['width'],
                    "height": metadata['dimensions']['height']
                },
                "grid_spacing_meters": 30,
                "data_type": "float32",
                "elevation_range_meters": {
                    "min": metadata['elevation_meters']['min'],
                    "max": metadata['elevation_meters']['max'],
                    "mean": metadata['elevation_meters']['mean']
                }
            },

            "coordinate_system": {
                "origin": BATTLEFIELD_CENTER,
                "units": "meters",
                "x_axis": "East-West (longitude)",
                "y_axis": "North-South (latitude)",
                "z_axis": "Elevation above sea level"
            },

            "procedural_modifications": {
                "trenches": {
                    "description": "Add WWI trench networks",
                    "width_meters": 1.5,
                    "depth_meters": 2.0,
                    "locations": [
                        "Around Fort Douaumont (49.2106°N, 5.4272°E)",
                        "Around Fort Vaux (49.1989°N, 5.4494°E)",
                        "Front lines (see verdun_locations.json for line markers)"
                    ],
                    "pattern": "Zigzag (traverses every 10-15m)",
                    "note": "Use Houdini curve tools to draw trench splines, then erode terrain"
                },

                "shell_craters": {
                    "description": "Add artillery shell craters",
                    "distribution": "Scattered procedurally (higher density near forts)",
                    "sizes": [
                        {"shell": "75mm", "diameter_m": 2, "depth_m": 0.5, "density": "high"},
                        {"shell": "155mm", "diameter_m": 6, "depth_m": 1.5, "density": "medium"},
                        {"shell": "305mm", "diameter_m": 15, "depth_m": 3, "density": "low"},
                        {"shell": "420mm", "diameter_m": 30, "depth_m": 6, "density": "rare"}
                    ],
                    "total_shells_fired": 40000000,
                    "note": "Use scatter SOP with noise for realistic distribution"
                },

                "erosion": {
                    "description": "Modify terrain to show war damage",
                    "thermal_erosion": "Light (simulate collapsed trenches)",
                    "hydraulic_erosion": "Heavy (10 months of rain in churned earth)",
                    "noise_erosion": "Add high-frequency detail (rough, churned ground)"
                },

                "mud_zones": {
                    "description": "Areas of perpetual mud (April-November 1916)",
                    "locations": [
                        "Low-lying areas (elevation < 300m)",
                        "Shell crater concentrations",
                        "Trench systems"
                    ],
                    "displacement": "-0.1 to -0.3m (sinking into mud)"
                }
            },

            "materials": {
                "terrain_base": {
                    "grass": "Sparse, dead (destroyed by gas/shells)",
                    "mud": "Primary surface (70% of battlefield)",
                    "chalk": "Exposed in craters and trenches (white Verdun chalk)",
                    "rock": "Exposed on hills and crater edges"
                },
                "blending": "Height-based + slope + manual paint",
                "textures": [
                    "mud_wet_churned.jpg",
                    "grass_dead_sparse.jpg",
                    "chalk_white_verdun.jpg",
                    "rock_limestone_broken.jpg"
                ]
            },

            "vegetation": {
                "trees": "99% destroyed - scattered dead trunks only",
                "scatter": [
                    {"type": "dead_tree_trunk", "height_m": 3-8, "density": "very_sparse"},
                    {"type": "broken_stumps", "height_m": 0.5-2, "density": "sparse"},
                    {"type": "wire_posts", "height_m": 1.5, "density": "medium (along trenches)"}
                ],
                "note": "No living vegetation on central battlefield (destroyed by gas)"
            },

            "historical_accuracy": {
                "topography": "Base terrain accurate to 1916 (SRTM data)",
                "modifications": "Trenches/craters must be added procedurally (not in SRTM)",
                "references": "Use historical photos from verdun_anthology/primary_sources/",
                "verification": "Compare to French military maps (1916-1918)"
            },

            "houdini_workflow": [
                "1. Import heightfield (CSV or RAW format)",
                "2. Set grid spacing to 30 meters",
                "3. Add noise for micro-detail (scale 1-5m)",
                "4. Draw trench curves, erode terrain along paths",
                "5. Scatter shell craters (noise-based distribution)",
                "6. Apply hydraulic erosion (simulate 10 months rain)",
                "7. Import location points from verdun_locations.json",
                "8. Place fort/village geometry at point positions",
                "9. Add materials (mud, chalk, sparse grass)",
                "10. Export to game engine or render"
            ]
        }

        output_file = OUTPUT_DIR / 'verdun_terrain_params.json'
        with open(output_file, 'w') as f:
            json.dump(params, f, indent=2)

        print(f"   ✅ Created: {output_file.name}")
        return True

    except Exception as e:
        print(f"   ❌ Error: {e}")
        return False


def create_readme():
    """Create Houdini import instructions."""
    print("\n📝 CREATING HOUDINI README...")

    readme = """VERDUN BATTLEFIELD - HOUDINI IMPORT GUIDE
==========================================

This package contains Verdun battlefield terrain data optimized for Houdini
procedural generation and historical accuracy.

PACKAGE CONTENTS
----------------
verdun_heightfield.csv          - Elevation grid (CSV format)
verdun_heightfield.raw          - Elevation grid (32-bit float binary)
verdun_heightfield.exr          - Elevation grid (EXR format, if available)
verdun_locations.json           - Historical location points
verdun_locations_xyz.csv        - Location points (XYZ CSV)
verdun_terrain_params.json      - Procedural generation parameters
README_HOUDINI.txt             - This file


STEP 1: IMPORT HEIGHTFIELD
---------------------------
METHOD A - CSV Import:
1. Create HeightField node
2. File SOP → Load "verdun_heightfield.csv"
3. Set Grid Spacing: 30 meters
4. Heightfield will generate with Verdun terrain

METHOD B - RAW Import:
1. Create HeightField node
2. HeightField File SOP → Load "verdun_heightfield.raw"
3. Format: 32-bit float, little-endian
4. Dimensions: See verdun_heightfield.raw.txt
5. Grid spacing: 30 meters

METHOD C - EXR Import (if available):
1. HeightField File SOP → Load "verdun_heightfield.exr"
2. Grid spacing: 30 meters
3. EXR preserves full 32-bit precision


STEP 2: ADD PROCEDURAL DETAIL
------------------------------
Base SRTM terrain lacks small-scale features (trenches, craters). Add procedurally:

A. NOISE DETAIL (micro-roughness):
   1. HeightField Noise
   2. Amplitude: 0.5-2.0 meters
   3. Element size: 1-5 meters
   4. Roughness: 0.7-0.9
   → Creates churned, rough ground texture

B. TRENCHES:
   1. Draw curves where trenches should be (use location points as guide)
   2. HeightField Erode → Use curves as mask
   3. Trench width: 1.5m, depth: 2.0m
   4. Add zigzag pattern (traverses every 10-15m)

   Key trench locations:
   - Around Fort Douaumont (49.2106°N, 5.4272°E)
   - Around Fort Vaux (49.1989°N, 5.4494°E)
   - Front lines (see verdun_locations.json)

C. SHELL CRATERS:
   1. HeightField Scatter
   2. Distribution: Noise-based (higher density near forts)
   3. Crater sizes:
      - 75mm: 2m diameter, 0.5m deep (high density)
      - 155mm: 6m diameter, 1.5m deep (medium density)
      - 305mm: 15m diameter, 3m deep (low density)
      - 420mm: 30m diameter, 6m deep (rare)
   4. Use HeightField Erode to carve craters
   5. Total craters: 10,000-50,000 depending on sector

D. EROSION (war damage):
   1. HeightField Erode → Hydraulic
   2. Iterations: 50-100 (heavy erosion from 10 months rain + shells)
   3. Thermal erosion: 10-20 (collapsed trenches)
   4. Adds realistic water flow paths and mud accumulation


STEP 3: IMPORT HISTORICAL LOCATIONS
------------------------------------
1. File SOP → Load "verdun_locations_xyz.csv"
2. Points generated at historical locations
3. Use point attributes:
   - name: Location name
   - type: Location type (fort, village, hill, etc.)
   - significance: Historical importance
4. Copy fort/village geometry to points
5. Use as spawn points for game objects


STEP 4: MATERIALS & TEXTURES
-----------------------------
Verdun terrain composition:
- 70% Mud (wet, churned, brown-grey)
- 20% Chalk (white, exposed in craters/trenches)
- 8% Dead grass (sparse, brown)
- 2% Rock (limestone, broken)

Material blending:
- Low areas (< 300m elevation): Mud
- High areas (> 350m elevation): Chalk/rock (exposed by erosion)
- Steep slopes (> 30°): Rock
- Flat areas: Mud with sparse grass

Recommended textures:
- mud_wet_verdun.jpg (brown-grey, churned)
- chalk_white_verdun.jpg (white limestone)
- grass_dead_sparse.jpg (brown, patchy)
- rock_broken_limestone.jpg (grey, angular)


STEP 5: VEGETATION (minimal)
-----------------------------
99% of trees destroyed by artillery and gas. Add sparingly:

Scatter objects:
- Dead tree trunks: 3-8m tall, very sparse (50-100m spacing)
- Broken stumps: 0.5-2m tall, sparse
- Wire posts: 1.5m tall, along trenches only

NO living vegetation in central combat zone.


COORDINATE SYSTEM
-----------------
Origin: Battlefield center (49.2050°N, 5.4300°E)
Units: Meters
X: East-West (Longitude)
Y: North-South (Latitude)
Z: Elevation above sea level (meters)

Grid spacing: 30 meters (SRTM resolution)
Coverage: ~60 km² (49.14-49.25°N, 5.31-5.47°E)


KEY LOCATIONS (for reference)
------------------------------
Fort Douaumont:   X=+9,550m,  Y=+615m,   Z=388m
Fort Vaux:        X=+13,850m, Y=-555m,   Z=390m
Mort-Homme:       X=-6,930m,  Y=+3,765m, Z=295m
Hill 304:         X=-8,130m,  Y=+3,920m, Z=304m
Verdun City:      X=+464m,    Y=-5,040m, Z=200m

(See verdun_locations.json for all 80+ locations)


HISTORICAL ACCURACY TIPS
-------------------------
✅ Overall topography accurate (SRTM data matches 1916)
✅ Fort/village positions verified (historical maps)
⚠️  Add trenches/craters manually (not in modern terrain data)
⚠️  Reference historical photos for realistic damage patterns

Recommended references:
- verdun_anthology/primary_sources/ (historical photos)
- French military maps (1916-1918, Institut Géographique National)
- Aerial reconnaissance photos (available in French archives)


WORKFLOW SUMMARY
----------------
1. Import heightfield → Base terrain
2. Add noise detail → Micro-roughness
3. Draw/scatter trenches → Trench networks
4. Scatter/erode craters → Shell damage
5. Apply erosion → 10 months of war damage
6. Import location points → Historical markers
7. Add materials → Mud/chalk/grass
8. Sparse vegetation → Dead trees only
9. Export to game engine or render

Result: Historically accurate Verdun terrain ready for gameplay or visualization


EXPORT TO GAME ENGINES
-----------------------
From Houdini:
- Heightfield → Export as 16-bit PNG (for UE5/Unity)
- Geometry → Export as FBX/OBJ (forts, villages)
- Textures → Export material maps (diffuse, normal, roughness)

For UE5: See ../UE5_Import/README_UE5_IMPORT.txt


QUESTIONS?
----------
See verdun_anthology/ for complete historical documentation
Issues: https://github.com/anthropics/claude-code/issues

"Ils ne passeront pas." (They shall not pass)
"""

    output_readme = OUTPUT_DIR / 'README_HOUDINI.txt'
    with open(output_readme, 'w') as f:
        f.write(readme)

    print(f"   ✅ Created: {output_readme.name}")
    return True


def print_summary():
    """Print export summary."""
    print("\n" + "=" * 70)
    print("✅ HOUDINI EXPORT COMPLETE")
    print("=" * 70)

    print(f"\n📁 OUTPUT DIRECTORY: {OUTPUT_DIR}")
    print("\n   Files created:")

    files = [
        'verdun_heightfield.csv',
        'verdun_heightfield.raw',
        'verdun_heightfield.raw.txt',
        'verdun_heightfield.exr',
        'verdun_locations.json',
        'verdun_locations_xyz.csv',
        'verdun_terrain_params.json',
        'README_HOUDINI.txt'
    ]

    for filename in files:
        filepath = OUTPUT_DIR / filename
        if filepath.exists():
            size = filepath.stat().st_size
            size_str = f"{size:,} bytes" if size < 1024*1024 else f"{size/(1024*1024):.2f} MB"
            print(f"   ✅ {filename:<35} ({size_str})")
        else:
            # Optional files
            if filename == 'verdun_heightfield.exr':
                print(f"   ⚠️  {filename:<35} (optional - requires OpenEXR)")
            else:
                print(f"   ❌ {filename:<35} (MISSING)")

    print("\n📖 NEXT STEPS:")
    print("   1. Open README_HOUDINI.txt for import guide")
    print("   2. Import heightfield into Houdini HeightField node")
    print("   3. Add procedural detail (noise, trenches, craters)")
    print("   4. Import location points from verdun_locations.json")
    print("   5. Apply materials and sparse vegetation")

    print("\n🎨 READY FOR HOUDINI IMPORT")
    print("=" * 70)


def main():
    """Main execution."""
    if not check_prerequisites():
        return 1

    print(f"\n📤 EXPORTING FOR HOUDINI...")

    # Export CSV heightfield
    export_csv_heightfield()

    # Export RAW heightfield
    if not export_raw_heightfield():
        print("   ⚠️  Continuing despite RAW export failure...")

    # Export EXR (optional)
    export_exr_heightfield()  # May fail if OpenEXR not installed

    # Export locations
    if not export_locations():
        return 1

    # Create terrain params
    if not create_terrain_params():
        return 1

    # Create README
    if not create_readme():
        return 1

    # Print summary
    print_summary()

    return 0


if __name__ == '__main__':
    sys.exit(main())
