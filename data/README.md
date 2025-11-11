# Verdun Pipeline Data Directory

This directory contains all input, intermediate, and output data for the pipeline.

## Directory Structure

```
data/
├── incoming/          # Source data (inputs)
│   ├── vectors/       # Trench lines, paths (GeoJSON)
│   ├── rasters/       # DEMs, shell density maps (GeoTIFF)
│   └── footprints/    # Building polygons (GeoJSON)
│
├── staging/           # Houdini outputs (intermediate)
│   ├── meshes/        # FBX/USD geometry
│   ├── heightmaps/    # Terrain height (PNG/RAW)
│   ├── weightmaps/    # Material layers (PNG)
│   ├── catalogs/      # Instance placement (CSV/JSON)
│   ├── atlases/       # Texture atlases (optional)
│   └── thumbnails/    # QC preview images
│
└── published/         # Final versioned assets
    └── {TileID}/      # Per-tile published data
```

---

## Input Data Formats

### 1. Trench Vectors (GeoJSON)

**File:** `incoming/vectors/{TileID}_trenches.geojson`

**Format:**
- LineString features representing trench centerlines
- CRS: UTM Zone 31N (EPSG:32631) for Verdun region
- Z-coordinate optional (will be sampled from DEM)

**Required Properties:**
- `trench_id` (string): Unique identifier
- `width` (float): Trench width in meters
- `depth` (float): Trench depth in meters
- `side` (string): "french", "german", or "neutral"

**Optional Properties:**
- `revetment_type` (string): "sandbag", "wood", "gabion", "earth"
- `has_duckboards` (int): 1 or 0

**Example:**
```json
{
  "type": "Feature",
  "geometry": {
    "type": "LineString",
    "coordinates": [[x1, y1, z1], [x2, y2, z2], ...]
  },
  "properties": {
    "trench_id": "FR_001",
    "width": 1.8,
    "depth": 2.5,
    "side": "french"
  }
}
```

---

### 2. Digital Elevation Model (GeoTIFF)

**File:** `incoming/rasters/{TileID}_dem.tif`

**Format:**
- Single-band 32-bit float GeoTIFF
- Values in meters (elevation)
- CRS: Must match vector data (UTM 31N)
- Resolution: 0.5m - 2m per pixel recommended

**Creation:**
- From LiDAR data
- From photogrammetry
- From historical maps + interpolation

**Notes:**
- Ensure proper georeferencing
- Use NoData value for gaps (typically -9999)

---

### 3. Shell Density Raster (GeoTIFF)

**File:** `incoming/rasters/{TileID}_shells.tif`

**Format:**
- Single-band 8-bit or 32-bit GeoTIFF
- Values normalized 0.0 - 1.0 (or 0-255 for 8-bit)
- Higher values = more intense shelling
- CRS and resolution should match DEM

**Creation:**
- Historical artillery maps
- Battle intensity heatmaps
- Procedural generation for scenarios

**Notes:**
- 0.0 = no craters
- 0.5 = moderate shelling
- 1.0 = heavy bombardment (saturated craters)

---

### 4. Building Footprints (GeoJSON)

**File:** `incoming/footprints/{TileID}_buildings.geojson`

**Format:**
- Polygon features representing building outlines
- CRS: UTM Zone 31N

**Required Properties:**
- `building_id` (string): Unique identifier
- `building_type` (string): "house", "church", "barn", "factory"
- `damage_level` (float): 0.0 (intact) to 1.0 (destroyed)

**Optional Properties:**
- `floor_count` (int): Number of stories
- `hit_direction` (array): [x, y, z] vector of primary damage
- `roof_type` (string): "tile", "thatch", "metal"

**Example:**
```json
{
  "type": "Feature",
  "geometry": {
    "type": "Polygon",
    "coordinates": [[[x1,y1], [x2,y2], [x3,y3], [x4,y4], [x1,y1]]]
  },
  "properties": {
    "building_id": "B001",
    "building_type": "house",
    "damage_level": 0.7
  }
}
```

---

## Output Data Formats

### 1. Meshes (FBX/USD)

**Location:** `staging/meshes/`
**Naming:** `{TileID}_{type}.fbx`

**Examples:**
- `N13_terrain.fbx` - Main terrain mesh
- `N13_trenches.fbx` - Trench geometry
- `N13_buildings.fbx` - Damaged buildings

**Contains:**
- Geometry with normals and UVs
- Material assignments (by material slot)
- Collision meshes (optional)

---

### 2. Heightmaps

**Location:** `staging/heightmaps/`
**Naming:** `{TileID}_height.png`

**Format:**
- 16-bit grayscale PNG
- Values normalized to 0-65535
- For UE5 landscape import

**Conversion:**
```python
# Houdini export heightfield as PNG
height_min = 0.0  # meters
height_max = 100.0  # meters
normalized = (height - height_min) / (height_max - height_min)
png_value = normalized * 65535
```

---

### 3. Weight Maps

**Location:** `staging/weightmaps/{TileID}/`
**Files:**
- `mud.png`
- `water.png`
- `wood.png`
- `metal.png`
- `grass.png`
- `blood.png`

**Format:**
- 8-bit grayscale PNG
- Values 0-255 (weight for that layer)
- Resolution should match heightmap

---

### 4. Instance Catalogs

**Location:** `staging/catalogs/{TileID}_instances.csv`

**CSV Format:**
```csv
asset_path,x,y,z,pitch,yaw,roll,scale_x,scale_y,scale_z
/Game/Props/Debris/WireReel,630123.5,5485234.7,102.3,0,45,0,1.0,1.0,1.0
```

**JSON Format:**
```json
{
  "tile_id": "N13",
  "instance_count": 1234,
  "instances": [
    {
      "asset_path": "/Game/Props/Debris/WireReel",
      "x": 630123.5,
      "y": 5485234.7,
      "z": 102.3,
      "pitch": 0,
      "yaw": 45,
      "roll": 0,
      "scale_x": 1.0,
      "scale_y": 1.0,
      "scale_z": 1.0
    }
  ]
}
```

---

## Debris Catalog

**File:** `incoming/debris_catalog.csv`

Defines available debris props for scattering.

**Format:**
```csv
asset_path,weight,min_scale,max_scale,tags,description
/Game/Props/Debris/WireReel,1.5,0.8,1.2,"metal industrial",Wire cable reel
```

**Fields:**
- `asset_path`: UE5 content path to static mesh
- `weight`: Relative spawn probability (higher = more common)
- `min_scale`, `max_scale`: Scale randomization range
- `tags`: Space-separated tags for filtering
- `description`: Human-readable description

---

## Example Tile: N13

A complete example tile is provided with:

- ✓ `incoming/vectors/N13_trenches.geojson` - 3 trench lines
- ✓ `incoming/footprints/N13_buildings.geojson` - 3 buildings
- ✓ `incoming/debris_catalog.csv` - 20 debris types

**Missing (you need to provide):**
- `incoming/rasters/N13_dem.tif` - Digital elevation model
- `incoming/rasters/N13_shells.tif` - Shell density map

These require actual geographic data or can be generated procedurally for testing.

---

## Creating Test Data

### Generate Test DEM

Using GDAL/Python:

```python
import numpy as np
from osgeo import gdal, osr

# Create 1km x 1km tile at 1m resolution
width = height = 1000
data = np.random.randn(height, width) * 5 + 100  # ~100m elevation ± 5m

# Create GeoTIFF
driver = gdal.GetDriverByName('GTiff')
ds = driver.Create('N13_dem.tif', width, height, 1, gdal.GDT_Float32)

# Set geotransform: (top-left X, pixel width, 0, top-left Y, 0, -pixel height)
ds.SetGeoTransform([630000, 1, 0, 5485500, 0, -1])

# Set projection (UTM 31N)
srs = osr.SpatialReference()
srs.ImportFromEPSG(32631)
ds.SetProjection(srs.ExportToWkt())

# Write data
ds.GetRasterBand(1).WriteArray(data)
ds = None
```

### Generate Test Shell Density

```python
# Similar to DEM but with intensity values
data = np.random.rand(height, width)  # 0.0 - 1.0
# Save as GeoTIFF
```

---

## Data Validation

Before processing, validate inputs:

```bash
# Check file exists
ls data/incoming/vectors/N13_trenches.geojson

# Validate GeoJSON
python -m json.tool N13_trenches.geojson > /dev/null && echo "Valid JSON"

# Check GeoTIFF metadata
gdalinfo data/incoming/rasters/N13_dem.tif

# Verify CRS matches
gdalsrsinfo -o proj4 N13_dem.tif
```

---

## Coordinate Systems

### Verdun Historical Region

- **CRS:** EPSG:32631 (WGS84 / UTM Zone 31N)
- **Location:** ~49.16°N, 5.38°E
- **UTM Zone:** 31N
- **Example coords:** (630000, 5485000) = approx. Verdun city center

### Converting from Lat/Lon

```python
from pyproj import Transformer

transformer = Transformer.from_crs("EPSG:4326", "EPSG:32631")
lat, lon = 49.16, 5.38
x, y = transformer.transform(lat, lon)
print(f"UTM: {x:.1f}, {y:.1f}")
```

---

## Tile Naming Convention

Tiles are named with a **letter + number** system:

- Letter (A-Z): Column (west to east)
- Number (0-99): Row (south to north)

**Example:**
- N13 = Column N (14th column), Row 13
- Grid size: 1km x 1km (configurable in config.yaml)

---

## Data Size Estimates

Per 1km² tile:

| Asset Type | Size | Count | Total |
|------------|------|-------|-------|
| DEM (1m res) | 4 MB | 1 | 4 MB |
| Shell density | 1 MB | 1 | 1 MB |
| Vectors | 100 KB | 2 | 200 KB |
| **Subtotal Input** | | | **~5 MB** |
| FBX meshes | 20 MB | 3-5 | 60-100 MB |
| Heightmaps | 2 MB | 1 | 2 MB |
| Weight maps | 1 MB | 6 | 6 MB |
| Instance catalogs | 500 KB | 1 | 500 KB |
| **Subtotal Output** | | | **~70-110 MB** |

For 100 tiles: **~10 GB** total.

---

## Best Practices

1. **Keep incoming data immutable** - Never modify original source data
2. **Version control incoming data** - Tag with date/source
3. **Clean staging regularly** - Regenerate as needed from incoming
4. **Archive published data** - Back up final assets
5. **Document data sources** - Record origin of DEMs, maps, etc.

---

For questions about data formats, consult docs/HDA_SPEC.md or docs/SETUP.md.
