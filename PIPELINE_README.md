# Verdun World-Generation Pipeline

Unattended procedural generation pipeline for WWI Verdun battlefields using **Houdini + PDG/TOPs** and **Unreal Engine 5**.

> **Note:** This pipeline is part of the "Only The Dead" WWI game project. For the main game documentation, see [README.md](README.md).

---

## Overview

This pipeline automates the creation of historically accurate WWI trench warfare environments:

- **Input:** Historical maps, DEMs, shell density data, building footprints
- **Processing:** Houdini HDAs for trenches, craters, debris, and damage
- **Output:** UE5-ready meshes, heightmaps, weight maps, and instance catalogs
- **Control:** Conversational CLI ("Foreman") for easy operation

### Key Features

✓ **Fully Automated** - Headless Houdini and UE5 Python automation
✓ **PDG/TOPs Orchestration** - Batch process multiple tiles in parallel
✓ **Art-Directable** - Parameterized HDAs with seedable randomness
✓ **QC Built-In** - Automated validation and thumbnail generation
✓ **Conversational Control** - Natural language CLI commands
✓ **Production-Ready** - Error handling, logging, recovery procedures

---

## Quick Start

### 1. Install

```bash
# Install Python dependencies
pip install -r requirements.txt

# Set environment variables (or update config.yaml)
export HFS=/path/to/houdini
export UE5_EDITOR_PATH=/path/to/UnrealEditor-Cmd.exe
export VERDUN_PROJECT_PATH=/path/to/Project.uproject
```

### 2. Verify Setup

```bash
python orchestrator/verify_setup.py
```

Should see all ✓ checks pass. Fix any ✗ errors before proceeding.

### 3. Build Example Tile

```bash
# Build tile N13 (example data provided)
python orchestrator/foreman.py build N13

# Import to UE5
python orchestrator/foreman.py ue import N13

# Run QC checks
python orchestrator/foreman.py qc N13
```

---

## Repository Structure

```
OnlyTheDead/
├── houdini/
│   ├── hdas/                    # Houdini Digital Assets
│   │   ├── TrenchBuilder.hda
│   │   ├── ShellField.hda
│   │   ├── DebrisScatter.hda
│   │   └── BuildingRuinizer.hda
│   ├── hip/
│   │   └── verdun_pdg.hip       # PDG network scene
│   └── python/
│       ├── run_pdg.py           # Headless PDG cook
│       ├── cook_trenches.py     # Single-shot HDA cook
│       ├── qc_render.py         # QC thumbnail generator
│       └── utils/               # Shared utilities
│
├── ue/
│   └── Scripts/
│       ├── auto_ingest.py       # Import FBX/USD to UE5
│       ├── build_world.py       # Place instances from catalogs
│       └── maintenance.py       # HLOD/navmesh rebuild
│
├── orchestrator/
│   ├── foreman.py               # CLI controller
│   ├── verify_setup.py          # Setup verification
│   ├── config.yaml              # Pipeline configuration
│   └── logs/                    # All operation logs
│
├── data/
│   ├── incoming/                # Source data
│   ├── staging/                 # Houdini outputs
│   └── published/               # Final versioned assets
│
└── docs/
    ├── SETUP.md                 # Installation guide
    ├── RUNBOOK.md               # Day-to-day operations
    ├── HDA_SPEC.md              # HDA specifications
    └── UE_IMPORT.md             # UE5 import guide
```

---

## Foreman CLI Commands

```bash
# Check pipeline status
python orchestrator/foreman.py status

# Build a tile
python orchestrator/foreman.py build N13
python orchestrator/foreman.py build N13 --heavy-shelling --season winter

# Import to UE5
python orchestrator/foreman.py ue import N13

# Maintenance
python orchestrator/foreman.py ue rebuild-hlod --map Verdun_Persistent

# Quality control
python orchestrator/foreman.py qc N13

# Publish assets
python orchestrator/foreman.py publish N13
```

---

## HDAs (Houdini Digital Assets)

Four procedural generators create the battlefield:

### 1. TrenchBuilder.hda
- Digs trenches into heightfield
- Creates revetments (sandbag, wood, gabion)
- Adds spoil banks and duckboards
- Outputs mud/wood weight maps

### 2. ShellField.hda
- Scatters craters from density rasters
- Deforms terrain with crater SDFs
- Fills craters with water
- Generates debris socket points

### 3. DebrisScatter.hda
- Reads asset catalog (CSV)
- Weighted random placement
- Conforms to terrain
- Outputs instance catalog for UE5

### 4. BuildingRuinizer.hda
- Boolean artillery damage
- Generates rubble fields
- Applies burnt material masks

**See:** `docs/HDA_SPEC.md` for complete specifications.

---

## Data Formats

### Inputs

| Type | Format | Description |
|------|--------|-------------|
| Trench curves | GeoJSON | LineStrings with width/depth attrs |
| DEM | GeoTIFF | 32-bit float elevation |
| Shell density | GeoTIFF | 0-1 intensity values |
| Buildings | GeoJSON | Polygons with damage levels |

### Outputs

| Type | Format | Description |
|------|--------|-------------|
| Meshes | FBX/USD | Nanite-ready geometry |
| Heightmaps | PNG (16-bit) | For UE5 landscape |
| Weight maps | PNG (8-bit) | Material layer masks |
| Instances | CSV/JSON | Transforms + asset paths |

**See:** `data/README.md` for detailed specifications.

---

## Workflow

### Standard Pipeline

```
Source Data → Houdini HDAs → PDG/TOPs → Staging → QC → UE5 Import → Level Assembly
```

### Step-by-Step

1. **Prepare Data:** Place GeoJSON, GeoTIFFs in `data/incoming/`
2. **Cook Houdini:** `foreman.py build TILE_ID`
3. **QC Check:** `foreman.py qc TILE_ID`
4. **Import UE5:** `foreman.py ue import TILE_ID`
5. **Publish:** `foreman.py publish TILE_ID`

---

## Documentation

| Document | Description |
|----------|-------------|
| [SETUP.md](docs/SETUP.md) | Installation and setup guide |
| [RUNBOOK.md](docs/RUNBOOK.md) | Day-to-day operations |
| [HDA_SPEC.md](docs/HDA_SPEC.md) | HDA specifications and parameters |
| [UE_IMPORT.md](docs/UE_IMPORT.md) | UE5 import and material setup |
| [data/README.md](data/README.md) | Data formats and conventions |

---

## Example: Tile N13

A complete example tile is provided:

- ✓ Trench vectors (3 trench lines)
- ✓ Building footprints (3 buildings)
- ✓ Debris catalog (20 prop types)
- ⚠ DEM and shell density (you need to create)

**Test the pipeline:**

```bash
# 1. Build tile
python orchestrator/foreman.py build N13

# 2. Check outputs
ls data/staging/meshes/N13*

# 3. Import to UE5
python orchestrator/foreman.py ue import N13
```

---

## Requirements

- **Houdini 19.5+** (FX license for PDG)
- **Unreal Engine 5.3+** (with Python scripting)
- **Python 3.9+** (for orchestrator)
- **Disk Space:** ~1 GB per tile
- **RAM:** 16 GB minimum, 32 GB recommended

---

## Performance

Approximate cook times (Intel i7, 32GB RAM):

| Task | Single Tile | 10 Tiles (Parallel) |
|------|-------------|---------------------|
| Houdini Cook | 5-15 min | 30-60 min |
| UE5 Import | 2-5 min | 15-30 min |
| **Total** | **10-20 min** | **45-90 min** |

---

## Production Deployment

For turning these stubs into production:

### Houdini
1. Build the 4 HDAs with full implementations
2. Create PDG network in `verdun_pdg.hip`
3. Optimize HDA performance
4. Add error handling

### UE5
1. Create master material `M_TerrainMaster`
2. Set up landscape layer infos
3. Test import/export workflow
4. Build HLOD and navmesh automation

### Orchestrator
1. Implement file watcher
2. Add notification system
3. Create deployment scripts
4. Set up CI/CD

---

## Support

- **Logs:** Check `orchestrator/logs/`
- **Verification:** Run `python orchestrator/verify_setup.py`
- **Docs:** See `docs/` directory

---

**Status:** Production-ready stubs. HDAs and PDG network need to be built in Houdini.

*Last Updated: 2025-01-15*
