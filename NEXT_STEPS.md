# Next Steps: Stubs → Production

This document outlines the steps to turn the pipeline stubs into a production-ready system.

## ✅ What's Complete

The following has been delivered as **working code stubs**:

### Infrastructure ✓
- [x] Complete directory structure
- [x] Configuration system (YAML)
- [x] Logging framework
- [x] Utility libraries (I/O, tiles, QC, attributes)

### Houdini Python ✓
- [x] PDG runner (`run_pdg.py`)
- [x] HDA cook script (`cook_trenches.py`)
- [x] QC renderer (`qc_render.py`)
- [x] Job ticket system
- [x] Catalog management

### UE5 Python ✓
- [x] Auto-ingest script (`auto_ingest.py`)
- [x] World builder (`build_world.py`)
- [x] Maintenance utilities (`maintenance.py`)
- [x] FBX/USD import automation
- [x] HISM instance placement

### Foreman CLI ✓
- [x] Conversational command interface
- [x] Build, import, QC, publish commands
- [x] Status monitoring
- [x] Log streaming

### Documentation ✓
- [x] SETUP.md - Installation guide
- [x] RUNBOOK.md - Daily operations
- [x] HDA_SPEC.md - HDA specifications
- [x] UE_IMPORT.md - UE5 import guide
- [x] data/README.md - Data format specs
- [x] PIPELINE_README.md - Pipeline overview

### Example Data ✓
- [x] Tile N13 trench curves (GeoJSON)
- [x] Tile N13 building footprints (GeoJSON)
- [x] Debris catalog (CSV, 20 items)
- [x] Job ticket template

---

## 🔨 What Needs to Be Built

### 1. Houdini HDAs (CRITICAL PATH)

Build the 4 HDAs in Houdini:

#### TrenchBuilder.hda
**Status:** Specification complete, HDA not built
**Priority:** HIGH
**Effort:** 2-3 days
**Steps:**
1. Create new HDA in Houdini
2. Implement inputs (heightfield, curves)
3. Create trench profile (width/depth)
4. Sweep profile along curves
5. Boolean subtract from heightfield
6. Generate revetments (based on type)
7. Add spoil banks (displaced mesh)
8. Create duckboard splines
9. Output weight maps (mud, wood, water)
10. Add parameters per spec
11. Test with N13 data
12. Save to `houdini/hdas/TrenchBuilder.hda`

#### ShellField.hda
**Status:** Specification complete, HDA not built
**Priority:** HIGH
**Effort:** 2-3 days
**Steps:**
1. Create new HDA
2. Implement inputs (heightfield, density raster OR points)
3. Scatter crater points from density
4. Assign radii from distribution
5. Create crater SDFs (combine multiple)
6. Displace heightfield with SDFs
7. Generate crater lips (displacement)
8. Fill water based on terrain analysis
9. Place debris sockets on lips
10. Output masks (water, mud)
11. Test with N13 data
12. Save to `houdini/hdas/ShellField.hda`

#### DebrisScatter.hda
**Status:** Specification complete, HDA not built
**Priority:** MEDIUM
**Effort:** 1-2 days
**Steps:**
1. Create new HDA
2. Load catalog CSV
3. Scatter points from mask
4. Apply spacing constraint
5. Assign assets (weighted random)
6. Sample terrain height/normal
7. Apply burial depth
8. Randomize transforms
9. Create packed primitives
10. Export instance CSV for UE5
11. Test with N13 data
12. Save to `houdini/hdas/DebrisScatter.hda`

#### BuildingRuinizer.hda
**Status:** Specification complete, HDA not built
**Priority:** MEDIUM
**Effort:** 2-3 days
**Steps:**
1. Create new HDA
2. Extrude walls from footprints
3. Add floors and roof
4. Generate hit points (based on direction)
5. Boolean damage (subtract spheres/cones)
6. Fracture geometry (optional pre-fracture)
7. Generate rubble (shattered pieces)
8. Scatter rubble around base
9. Apply burnt material mask
10. Test with N13 data
11. Save to `houdini/hdas/BuildingRuinizer.hda`

---

### 2. Houdini PDG Network (CRITICAL PATH)

**Status:** Not created
**Priority:** HIGH
**Effort:** 1-2 days
**Steps:**

1. **Open Houdini** and create new scene
2. **Create TOP network** at `/obj/VERDUN_TOP`
3. **Build PDG graph:**
   ```
   File Pattern TOP (watch incoming/)
     ↓
   Partition by Tile TOP
     ↓
   Python Processor TOP (validate inputs)
     ↓
   [4 parallel HDA Processor TOPs]
     ├─ ShellField HDA
     ├─ TrenchBuilder HDA
     ├─ DebrisScatter HDA
     └─ BuildingRuinizer HDA
     ↓
   ROP Geometry TOP (export FBX/USD)
     ↓
   Python TOP (QC render thumbnails)
     ↓
   Publisher TOP (move to staging)
     ↓
   Email/Slack TOP (notification - optional)
   ```
4. **Configure parameters:**
   - Set HDA paths
   - Set output directories
   - Add error handling
   - Set work item attributes
5. **Add PDG callbacks** for progress tracking
6. **Test with N13** tile
7. **Save** as `houdini/hip/verdun_pdg.hip`

---

### 3. Test Data (REQUIRED FOR TESTING)

**Status:** Partial (vectors provided, rasters missing)
**Priority:** HIGH
**Effort:** 1-2 hours

Create test DEM and shell density for N13:

**Option A: Generate Procedural Data**
```python
# See data/README.md for code examples
import numpy as np
from osgeo import gdal

# Create 1km x 1km DEM at 1m resolution
width = height = 1000
dem = np.random.randn(height, width) * 5 + 100  # ~100m ± 5m
# Save as GeoTIFF with UTM 31N projection

# Create shell density (0-1 values)
shells = np.random.rand(height, width) * 0.5  # Moderate shelling
# Save as GeoTIFF
```

**Option B: Use Real SRTM Data**
- Download from USGS EarthExplorer
- Clip to tile bounds
- Resample to 1m resolution
- Reproject to UTM 31N

**Deliverables:**
- `data/incoming/rasters/N13_dem.tif`
- `data/incoming/rasters/N13_shells.tif`

---

### 4. UE5 Project Setup (REQUIRED FOR IMPORT)

**Status:** Not created
**Priority:** HIGH
**Effort:** 2-4 hours

1. **Create UE5 Project:**
   - New project (Games → First Person or Blank)
   - Name: VerdunProject
   - Enable Nanite, Lumen

2. **Enable Python:**
   - Edit → Plugins
   - Search "Python"
   - Enable "Python Editor Script Plugin"
   - Enable "Editor Scripting Utilities"
   - Restart editor

3. **Create Content Structure:**
   ```
   /Game/Verdun/
     /Tiles/
     /Materials/
       M_TerrainMaster (create this!)
     /Props/
       /Debris/
       /Buildings/
   ```

4. **Create Master Material:**
   - Material → M_TerrainMaster
   - Add Landscape Layer Blend node
   - Layers: mud, water, wood, metal, grass, blood
   - Save

5. **Configure Python Path:**
   - Edit DefaultEngine.ini
   - Add pipeline scripts to Python path

6. **Update config.yaml:**
   ```yaml
   unreal:
     editor_path: "C:/Program Files/.../UnrealEditor-Cmd.exe"
     project_path: "C:/.../VerdunProject.uproject"
   ```

---

### 5. Integration & Testing

**Priority:** HIGH
**Effort:** 1-2 days

#### End-to-End Test (Tile N13)

1. **Verify Setup:**
   ```bash
   python orchestrator/verify_setup.py
   ```
   All checks should pass ✓

2. **Cook Houdini:**
   ```bash
   hython houdini/python/run_pdg.py --tile N13
   ```
   Expected outputs:
   - `data/staging/meshes/N13_terrain.fbx`
   - `data/staging/heightmaps/N13_height.png`
   - `data/staging/weightmaps/N13/*.png`
   - `data/staging/catalogs/N13_instances.csv`

3. **QC Check:**
   ```bash
   python orchestrator/foreman.py qc N13
   ```
   Should pass validation

4. **Import UE5:**
   ```bash
   python orchestrator/foreman.py ue import N13
   ```
   Assets appear in `/Game/Verdun/Tiles/N13/`

5. **Build World:**
   ```bash
   python orchestrator/foreman.py ue build-world N13
   ```
   Instances placed in level

6. **Manual Verification:**
   - Open UE5 project
   - Navigate to `/Game/Verdun/Tiles/N13/`
   - Place assets in level
   - Check materials, collision, LODs
   - Test in PIE

---

### 6. Optimization & Production Features

**Priority:** MEDIUM
**Effort:** Ongoing

#### Performance
- [ ] Profile HDA cook times
- [ ] Add compiled blocks to HDAs
- [ ] Optimize PDG scheduling
- [ ] Implement caching

#### Error Handling
- [ ] Add retry logic to PDG
- [ ] Implement recovery procedures
- [ ] Add validation nodes to HDAs
- [ ] Improve error messages

#### Notifications
- [ ] Add email notifications (success/fail)
- [ ] Add Slack integration
- [ ] Add progress webhooks

#### Monitoring
- [ ] Create dashboard for pipeline status
- [ ] Add metrics collection
- [ ] Implement alerting

---

### 7. Scale-Up

**Priority:** LOW (after N13 works)
**Effort:** Varies

#### Multiple Tiles
- [ ] Process tiles N13-N20 (batch test)
- [ ] Verify tile boundaries align
- [ ] Check performance at scale

#### Real Data
- [ ] Process historical maps to GeoJSON
- [ ] Generate DEM from LiDAR/photogrammetry
- [ ] Create shell density from battle maps
- [ ] Validate building footprints

#### Production Deployment
- [ ] Set up dedicated render farm
- [ ] Configure PDG for Tractor/Deadline
- [ ] Implement versioning system
- [ ] Create CI/CD for HDAs

---

## 📋 Implementation Checklist

Use this checklist to track progress:

### Phase 1: Core Functionality
- [ ] Build TrenchBuilder.hda
- [ ] Build ShellField.hda
- [ ] Build DebrisScatter.hda
- [ ] Build BuildingRuinizer.hda
- [ ] Create PDG network (verdun_pdg.hip)
- [ ] Generate N13 test data (DEM + shells)
- [ ] Set up UE5 project
- [ ] Test N13 end-to-end

### Phase 2: Polish & Optimization
- [ ] Add error handling to HDAs
- [ ] Optimize cook performance
- [ ] Implement retry logic
- [ ] Add notifications
- [ ] Create monitoring dashboard

### Phase 3: Scale & Production
- [ ] Process 10 tiles successfully
- [ ] Set up render farm
- [ ] Implement versioning
- [ ] Deploy to production

---

## 🎯 Recommended Order

**Week 1: Core HDAs**
1. Build TrenchBuilder.hda (Day 1-3)
2. Build ShellField.hda (Day 4-6)
3. Test both with N13 data (Day 7)

**Week 2: Remaining HDAs + PDG**
1. Build DebrisScatter.hda (Day 1-2)
2. Build BuildingRuinizer.hda (Day 3-4)
3. Create PDG network (Day 5-6)
4. Test full pipeline (Day 7)

**Week 3: UE5 Integration**
1. Set up UE5 project (Day 1)
2. Create master material (Day 2)
3. Test import scripts (Day 3-4)
4. End-to-end test (Day 5-7)

**Week 4: Polish & Production-Ready**
1. Optimize performance
2. Add error handling
3. Write production runbooks
4. Train team on usage

---

## 💡 Tips

### Debugging HDAs
- Test each HDA individually before PDG
- Use Houdini Performance Monitor
- Add Python validation nodes
- Check geometry with `geo.attribValue()` calls

### PDG Debugging
- Use PDG Event Log viewer
- Add Python TOP nodes for debugging
- Set environment variables for verbose output
- Test with single work item first

### UE5 Debugging
- Check UE5 Output Log constantly
- Use Python console for testing
- Import assets manually first
- Save packages frequently

---

## 📞 Getting Help

If stuck:

1. **Check logs:** `orchestrator/logs/`
2. **Run verification:** `python orchestrator/verify_setup.py`
3. **Review docs:** All specs in `docs/`
4. **Test individually:** Each HDA, each script
5. **Ask for help:** GitHub issues or team chat

---

## 🎉 Success Criteria

The pipeline is production-ready when:

✅ All 4 HDAs cook without errors
✅ PDG processes N13 successfully
✅ UE5 imports assets with correct settings
✅ Instances place correctly in level
✅ QC checks pass
✅ End-to-end time < 20 minutes for single tile
✅ Documentation is complete and accurate
✅ Team can operate without assistance

---

**Good luck! You have all the infrastructure. Now just build the HDAs and test!**

*Last Updated: 2025-01-15*
