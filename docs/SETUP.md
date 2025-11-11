# Verdun Pipeline Setup Guide

Complete setup instructions for the Verdun world-generation pipeline.

## Prerequisites

### Software Requirements

1. **Houdini 19.5+ or Houdini 20**
   - Standard or FX license (for PDG/TOPs)
   - Download from: https://www.sidefx.com/

2. **Unreal Engine 5.3+**
   - With Python scripting enabled
   - Download from Epic Games Launcher

3. **Python 3.9+** (for orchestrator scripts)
   - Houdini and UE5 have embedded Python, but orchestrator needs standalone Python

### Optional Tools

- Git (for version control)
- Visual Studio Code (recommended editor)
- Houdini Engine for UE5 (optional, for live HDA integration)

---

## Installation Steps

### 1. Clone Repository

```bash
git clone <repository-url>
cd OnlyTheDead
```

### 2. Install Python Dependencies

The orchestrator scripts need a few Python packages:

```bash
# Create virtual environment (recommended)
python -m venv venv

# Activate virtual environment
# On Windows:
venv\Scripts\activate
# On Linux/Mac:
source venv/bin/activate

# Install requirements
pip install -r requirements.txt
```

### 3. Configure Environment Variables

Set these environment variables or update `orchestrator/config.yaml`:

**Windows:**
```cmd
set HFS=C:\Program Files\Side Effects Software\Houdini 20.0.590
set UE5_EDITOR_PATH=C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe
set VERDUN_PROJECT_PATH=C:\Projects\VerdunProject\VerdunProject.uproject
```

**Linux/Mac:**
```bash
export HFS=/opt/hfs20.0
export UE5_EDITOR_PATH=/path/to/UnrealEditor
export VERDUN_PROJECT_PATH=/path/to/project.uproject
```

### 4. Update Configuration

Edit `orchestrator/config.yaml` with your paths:

```yaml
houdini:
  houdini_bin: null  # Leave null to use $HFS/bin

unreal:
  editor_path: "C:/Program Files/Epic Games/UE_5.3/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
  project_path: "C:/Projects/VerdunProject/VerdunProject.uproject"
```

### 5. Verify Installation

Test that all tools are accessible:

```bash
# Test Foreman CLI
python orchestrator/foreman.py status

# Test Houdini
hython --version

# Test UE5 (should print version info)
UnrealEditor-Cmd.exe -help
```

You should see:
- ✓ Houdini path found
- ✓ UE5 path found
- ✓ All directories created

---

## Building HDAs

The pipeline requires 4 Houdini Digital Assets. You need to build these in Houdini:

### 1. Open Houdini

Launch Houdini and create a new scene.

### 2. Build Each HDA

Follow the specifications in `docs/HDA_SPEC.md` to build:

1. **TrenchBuilder.hda** - Trench geometry generator
2. **ShellField.hda** - Crater field generator
3. **DebrisScatter.hda** - Debris instance scatterer
4. **BuildingRuinizer.hda** - Building damage processor

### 3. Save HDAs

Save each HDA to `houdini/hdas/` with exact names:
- `houdini/hdas/TrenchBuilder.hda`
- `houdini/hdas/ShellField.hda`
- `houdini/hdas/DebrisScatter.hda`
- `houdini/hdas/BuildingRuinizer.hda`

### 4. Create PDG Network

1. Create a TOP network at `/obj/VERDUN_TOP`
2. Build the processing graph (see `docs/RUNBOOK.md` for details)
3. Save as `houdini/hip/verdun_pdg.hip`

---

## UE5 Project Setup

### 1. Enable Python Scripting

In your UE5 project:

1. Open Project Settings
2. Navigate to Plugins
3. Enable "Python Editor Script Plugin"
4. Enable "Editor Scripting Utilities"
5. Restart editor

### 2. Create Content Structure

In the Content Browser, create:

```
/Game/Verdun/
  /Tiles/
    /N13/
    /N14/
    ...
  /Materials/
    M_TerrainMaster
  /Props/
  /Blueprints/
```

### 3. Test Python Import

In UE5, open the Output Log and Python console:

```python
import unreal
unreal.log("Python is working!")
```

### 4. Configure Python Path

Ensure UE5 can find the pipeline scripts. Add to your project's `DefaultEngine.ini`:

```ini
[Python]
AdditionalPaths=C:\path\to\OnlyTheDead\ue\Scripts
```

---

## Preparing Sample Data

For testing, you need sample input data for at least one tile.

### Directory Structure

```
data/
  incoming/
    vectors/
      N13_trenches.geojson        # Trench centerlines
    rasters/
      N13_dem.tif                 # Digital elevation model
      N13_shells.tif              # Shell density heatmap
    footprints/
      N13_buildings.geojson       # Building footprint polygons
```

### Sample Data Formats

**Trench Curves (GeoJSON):**
```json
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "geometry": {
        "type": "LineString",
        "coordinates": [[x1, y1], [x2, y2], ...]
      },
      "properties": {
        "width": 1.8,
        "depth": 2.5,
        "side": "french"
      }
    }
  ]
}
```

**DEM (GeoTIFF):**
- Single-band 32-bit float
- Coordinate system: UTM Zone 31N (or as configured)
- Resolution: 0.5m - 2m per pixel

**Shell Density (GeoTIFF):**
- Single-band, normalized 0-1 values
- Higher values = more craters

**Building Footprints (GeoJSON):**
- Polygon features with properties: `building_type`, `damage_level`

---

## Verification

### Test End-to-End Pipeline

Once everything is set up, test with the example tile:

```bash
# 1. Cook Houdini geometry
hython houdini/python/cook_trenches.py \
  --curves data/incoming/vectors/N13_trenches.geojson \
  --dem data/incoming/rasters/N13_dem.tif \
  --out data/staging/N13

# 2. Check outputs
ls data/staging/N13/

# 3. Import to UE5
python orchestrator/foreman.py ue import N13

# 4. Check QC
python orchestrator/foreman.py qc N13
```

If all steps complete without errors, your pipeline is ready!

---

## Troubleshooting

### "hython not found"

- Ensure `$HFS` is set correctly
- Add `$HFS/bin` to your PATH
- Update `config.yaml` with absolute path

### "UE5 Python script failed"

- Verify Python plugin is enabled
- Check UE5 output log for Python errors
- Ensure project path is correct in config

### "HDAs not found"

- Check HDA files exist in `houdini/hdas/`
- Verify exact filenames match config
- Try loading HDAs manually in Houdini first

### "Permission denied" errors

- Run command prompt/terminal as administrator (Windows)
- Check file permissions on data directories

---

## Next Steps

Once setup is complete:

1. Read `docs/RUNBOOK.md` for daily operations
2. Review `docs/HDA_SPEC.md` to customize HDAs
3. Read `docs/UE_IMPORT.md` for UE5 asset management

For questions or issues, check the logs in `orchestrator/logs/`.
