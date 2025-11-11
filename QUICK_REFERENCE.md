# Verdun Pipeline - Quick Reference

## 🚀 Common Commands

### Setup & Verification
```bash
# Install dependencies
pip install -r requirements.txt

# Verify setup
python orchestrator/verify_setup.py

# Check status
python orchestrator/foreman.py status
```

### Build Pipeline
```bash
# Build single tile
python orchestrator/foreman.py build N13

# Build with variants
python orchestrator/foreman.py build N13 --heavy-shelling
python orchestrator/foreman.py build N13 --season winter

# Build multiple tiles
hython houdini/python/run_pdg.py --tile-list tiles.txt
```

### Quality Control
```bash
# Run QC checks
python orchestrator/foreman.py qc N13

# Generate thumbnails
hython houdini/python/qc_render.py --input data/staging/meshes/N13_terrain.fbx
```

### UE5 Import
```bash
# Import single tile
python orchestrator/foreman.py ue import N13

# Import all staged assets
python orchestrator/foreman.py ue import --all

# Build world instances
python orchestrator/foreman.py ue build-world N13
```

### Maintenance
```bash
# Run UE5 maintenance
python orchestrator/foreman.py ue maintenance

# Rebuild HLODs
python orchestrator/foreman.py ue rebuild-hlod --map Verdun_Persistent

# Publish assets
python orchestrator/foreman.py publish N13
```

---

## 📁 Key File Locations

### Configuration
- `orchestrator/config.yaml` - Main configuration
- `requirements.txt` - Python dependencies

### Houdini Scripts
- `houdini/python/run_pdg.py` - PDG runner
- `houdini/python/cook_trenches.py` - Single HDA cook
- `houdini/python/qc_render.py` - Thumbnail generator

### UE5 Scripts
- `ue/Scripts/auto_ingest.py` - Asset import
- `ue/Scripts/build_world.py` - Instance placement
- `ue/Scripts/maintenance.py` - Housekeeping

### Data
- `data/incoming/` - Source data
- `data/staging/` - Houdini outputs
- `data/published/` - Final assets

### Logs
- `orchestrator/logs/` - All operation logs

---

## 🗺️ Tile Naming

Format: **Letter + Number** (e.g., N13)
- Letter (A-Z): Column (west to east)
- Number (0-99): Row (south to north)
- Grid size: 1km x 1km (configurable)

---

## 📊 Data Formats

### Inputs
| Type | Format | Location |
|------|--------|----------|
| Trench curves | GeoJSON | `incoming/vectors/{TILE}_trenches.geojson` |
| DEM | GeoTIFF | `incoming/rasters/{TILE}_dem.tif` |
| Shell density | GeoTIFF | `incoming/rasters/{TILE}_shells.tif` |
| Buildings | GeoJSON | `incoming/footprints/{TILE}_buildings.geojson` |

### Outputs
| Type | Format | Location |
|------|--------|----------|
| Meshes | FBX | `staging/meshes/{TILE}_*.fbx` |
| Heightmaps | PNG | `staging/heightmaps/{TILE}_height.png` |
| Weight maps | PNG | `staging/weightmaps/{TILE}/*.png` |
| Instances | CSV | `staging/catalogs/{TILE}_instances.csv` |

---

## 🛠️ HDAs

1. **TrenchBuilder.hda** - Generates trench geometry
2. **ShellField.hda** - Creates crater fields
3. **DebrisScatter.hda** - Scatters debris props
4. **BuildingRuinizer.hda** - Damages buildings

Location: `houdini/hdas/`

---

## 🔧 Troubleshooting

### Build Fails
```bash
# Check logs
tail -f orchestrator/logs/pdg_N13.log

# Verify inputs exist
ls data/incoming/vectors/N13_*
ls data/incoming/rasters/N13_*

# Try direct HDA cook
hython houdini/python/cook_trenches.py \
  --curves data/incoming/vectors/N13_trenches.geojson \
  --dem data/incoming/rasters/N13_dem.tif \
  --out data/staging/N13_test
```

### UE5 Import Fails
```bash
# Check Python plugin enabled
# Check UE5 Output Log
# Verify paths in config.yaml

# Test manual import first
# Open UE5 → Content Browser → Import
```

### QC Fails
```bash
# Check output files exist
ls data/staging/meshes/N13_*

# Check file sizes
du -sh data/staging/meshes/N13_*

# Review metadata
cat data/staging/meshes/N13_metadata.json
```

---

## 📚 Documentation

- **[PIPELINE_README.md](PIPELINE_README.md)** - Overview
- **[docs/SETUP.md](docs/SETUP.md)** - Installation
- **[docs/RUNBOOK.md](docs/RUNBOOK.md)** - Operations
- **[docs/HDA_SPEC.md](docs/HDA_SPEC.md)** - HDA specs
- **[docs/UE_IMPORT.md](docs/UE_IMPORT.md)** - UE5 guide
- **[NEXT_STEPS.md](NEXT_STEPS.md)** - Production deployment

---

## 🎯 Typical Workflow

1. **Prepare** → Place source data in `data/incoming/`
2. **Build** → `foreman.py build TILE_ID`
3. **QC** → `foreman.py qc TILE_ID`
4. **Import** → `foreman.py ue import TILE_ID`
5. **Verify** → Check in UE5 editor
6. **Publish** → `foreman.py publish TILE_ID`

---

## ⏱️ Performance Estimates

| Task | Time (Single Tile) |
|------|--------------------|
| Houdini Cook | 5-15 min |
| UE5 Import | 2-5 min |
| QC Checks | 1-2 min |
| **Total** | **10-20 min** |

---

## 🔑 Environment Variables

```bash
# Houdini
export HFS=/path/to/houdini
export PATH=$HFS/bin:$PATH

# UE5
export UE5_EDITOR_PATH=/path/to/UnrealEditor-Cmd.exe
export VERDUN_PROJECT_PATH=/path/to/Project.uproject
```

Or set in `orchestrator/config.yaml`.

---

## 💡 Pro Tips

- **Always run QC** before UE5 import
- **Check logs** first when debugging
- **Test HDAs individually** before PDG
- **Use --help** on any command for options
- **Keep incoming data immutable** - never modify originals

---

## 🆘 Help

```bash
# Get help on any command
python orchestrator/foreman.py --help
python orchestrator/foreman.py build --help

# Check setup
python orchestrator/verify_setup.py

# View logs
tail -f orchestrator/logs/foreman.log
```

---

**Quick start: `python orchestrator/verify_setup.py && python orchestrator/foreman.py build N13`**
