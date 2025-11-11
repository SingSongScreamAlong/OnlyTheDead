# Verdun Pipeline Runbook

Day-to-day operational guide for the Verdun world-generation pipeline.

## Quick Reference

```bash
# Build a single tile
python orchestrator/foreman.py build N13

# Build with variant
python orchestrator/foreman.py build N13 --heavy-shelling --season winter

# Import to UE5
python orchestrator/foreman.py ue import N13

# Run QC checks
python orchestrator/foreman.py qc N13

# Check pipeline status
python orchestrator/foreman.py status

# UE5 maintenance
python orchestrator/foreman.py ue maintenance
```

---

## Daily Workflow

### 1. Receive New Data

New source data arrives in `data/incoming/`:

```
data/incoming/
  vectors/N13_trenches.geojson
  rasters/N13_dem.tif
  rasters/N13_shells.tif
  footprints/N13_buildings.geojson
```

### 2. Validate Inputs

Check that all required files are present:

```bash
# List incoming data
ls data/incoming/vectors/
ls data/incoming/rasters/
ls data/incoming/footprints/

# Verify file formats (basic check)
file data/incoming/vectors/N13_trenches.geojson
file data/incoming/rasters/N13_dem.tif
```

### 3. Build Tile

Run the pipeline for the tile:

```bash
python orchestrator/foreman.py build N13
```

This will:
1. Load the PDG network
2. Process the tile through all HDAs
3. Export meshes, heightmaps, and catalogs to staging
4. Generate thumbnails
5. Run basic validation

**Expected duration:** 5-30 minutes depending on tile complexity.

### 4. Review Outputs

Check the staging directory:

```bash
ls data/staging/meshes/N13*
ls data/staging/heightmaps/N13*
ls data/staging/catalogs/N13*
```

You should see:
- `N13_terrain.fbx` - Main terrain mesh
- `N13_height.png` - Heightmap
- `N13_instances.csv` - Instance catalog
- `N13_metadata.json` - Metadata

### 5. Quality Control

Run automated QC checks:

```bash
python orchestrator/foreman.py qc N13
```

Review the output for:
- ✓ Triangle count within limits
- ✓ All required layers present
- ✓ File sizes reasonable
- ✗ Any errors or warnings

If QC fails, check logs in `orchestrator/logs/`.

### 6. Import to UE5

Import the tile assets:

```bash
python orchestrator/foreman.py ue import N13
```

This will:
1. Import FBX meshes to `/Game/Verdun/Tiles/N13/`
2. Enable Nanite
3. Set collision presets
4. Assign master materials
5. Save packages

### 7. Build World Instances

Place debris and props from the catalog:

```bash
# This runs automatically as part of import
# Or run manually:
python orchestrator/foreman.py ue build-world N13
```

### 8. Verify in UE5

Open UE5 and verify:
1. Navigate to `/Game/Verdun/Tiles/N13/`
2. Open the level or place the tile
3. Check materials, collisions, and LODs
4. Test in PIE (Play In Editor)

### 9. Publish

Once verified, publish the tile:

```bash
python orchestrator/foreman.py publish N13
```

This moves assets from `staging/` to `published/` for version control.

---

## Batch Processing

### Process Multiple Tiles

Create a tile list file:

```bash
# tiles.txt
N13
N14
N15
```

Run batch:

```bash
hython houdini/python/run_pdg.py --tile-list tiles.txt --maxprocs 8
```

### Process All Tiles

Automatically detect and process all tiles in incoming:

```bash
hython houdini/python/run_pdg.py --all --maxprocs 8
```

---

## Variants and Parameters

### Heavy Shelling

Increases crater density and damage:

```bash
python orchestrator/foreman.py build N13 --heavy-shelling
```

### Seasons

Affects water levels, vegetation, mud:

```bash
python orchestrator/foreman.py build N13 --season winter
python orchestrator/foreman.py build N13 --season spring  # Default
```

### Custom PDG Parameters

For advanced control, run PDG directly with custom parameters:

```bash
hython houdini/python/run_pdg.py --tile N13 --param crater_density=2.0
```

---

## Troubleshooting

### Build Fails

1. **Check logs:**
   ```bash
   tail -f orchestrator/logs/pdg_N13.log
   ```

2. **Common issues:**
   - Missing input files → Verify all 4 input files exist
   - Invalid geometry → Check GeoJSON/TIFF formats
   - Out of memory → Reduce tile size or use more RAM

3. **Retry:**
   ```bash
   # Clean staging directory
   rm -rf data/staging/N13*
   # Rebuild
   python orchestrator/foreman.py build N13
   ```

### QC Failures

**Triangle count too high:**
- Reduce geometry detail in HDAs
- Simplify trench meshes
- Reduce crater resolution

**Missing layers:**
- Check HDA outputs for weight maps
- Verify layer names match config

**File size too small:**
- Check if HDAs actually generated geometry
- Look for errors in Houdini logs

### UE5 Import Issues

**Assets not appearing:**
- Check UE5 Output Log for errors
- Verify content path: `/Game/Verdun/Tiles/{tile}/`
- Ensure Python plugin is enabled

**Nanite not enabling:**
- Check static mesh has valid geometry
- Verify UE5 version supports Nanite
- Review auto_ingest.py logs

**Materials not assigned:**
- Verify master material exists: `/Game/Verdun/Materials/M_TerrainMaster`
- Check material path in config.yaml

### Performance Issues

**PDG cooking slowly:**
- Increase `maxprocs` parameter
- Use PDG scheduler: `localscheduler` or `tractor`
- Profile individual HDAs

**UE5 import hangs:**
- Reduce asset complexity
- Import tiles individually, not in batch
- Check UE5 doesn't have modal dialogs blocking

---

## Recovery Procedures

### Recovering Failed Tiles

If a tile fails partway through:

1. **Identify failure point:**
   ```bash
   grep ERROR orchestrator/logs/pdg_N13.log
   ```

2. **Clean partial outputs:**
   ```bash
   rm -rf data/staging/N13*
   ```

3. **Re-run from specific HDA:**
   ```bash
   # Run only TrenchBuilder
   hython houdini/python/cook_trenches.py \
     --curves data/incoming/vectors/N13_trenches.geojson \
     --dem data/incoming/rasters/N13_dem.tif \
     --out data/staging/N13
   ```

### Rolling Back Changes

If a tile was imported to UE5 but needs to be redone:

1. **Delete UE5 assets:**
   - In UE5 Content Browser, right-click `/Game/Verdun/Tiles/N13/`
   - Delete folder

2. **Clean staging:**
   ```bash
   rm -rf data/staging/N13*
   ```

3. **Rebuild from scratch:**
   ```bash
   python orchestrator/foreman.py build N13
   python orchestrator/foreman.py ue import N13
   ```

### Resetting the Pipeline

To completely reset:

```bash
# Clean all generated data
rm -rf data/staging/*
rm -rf data/published/*

# Clean logs
rm -rf orchestrator/logs/*

# Keep incoming data intact!
```

---

## Maintenance Tasks

### Weekly

- **Clean stale assets:**
  ```bash
  python orchestrator/foreman.py ue maintenance
  ```

- **Rebuild HLODs:**
  ```bash
  python orchestrator/foreman.py ue rebuild-hlod --map Verdun_Persistent
  ```

- **Archive logs:**
  ```bash
  tar -czf logs_$(date +%Y%m%d).tar.gz orchestrator/logs/
  rm orchestrator/logs/*.log
  ```

### Monthly

- **Backup published assets:**
  ```bash
  rsync -av data/published/ /backup/verdun_pipeline/
  ```

- **Update HDAs:**
  - Review HDA performance metrics
  - Optimize slow operations
  - Re-save HDAs to `houdini/hdas/`

- **Test full rebuild:**
  - Pick a representative tile
  - Rebuild end-to-end
  - Verify no regressions

---

## Monitoring

### Check Pipeline Status

```bash
python orchestrator/foreman.py status
```

Displays:
- Tool availability (Houdini, UE5)
- Directory status
- Staged asset counts

### View Recent Logs

```bash
# Foreman operations
tail -f orchestrator/logs/foreman.log

# PDG cooking
tail -f orchestrator/logs/pdg_*.log

# UE5 imports
cat orchestrator/logs/ue_import_results.json
```

### Generate Reports

QC summary for all tiles:

```bash
# Run QC on all tiles, generates CSV
python houdini/python/qc_summary.py --all --output orchestrator/qc_report.csv
```

---

## Tips and Best Practices

1. **Always run QC before importing to UE5**
   - Saves time catching issues early

2. **Use descriptive commit messages**
   - When versioning published assets

3. **Test variants on a single tile first**
   - Before batch processing

4. **Keep incoming data organized**
   - Use consistent naming: `{TILE_ID}_{type}.ext`

5. **Monitor disk space**
   - Staging directory can grow large
   - Clean regularly

6. **Use PDG callbacks for notifications**
   - Add email/Slack notifications on completion/failure

---

## Getting Help

- **Logs:** All operations log to `orchestrator/logs/`
- **Documentation:** Full specs in `docs/`
- **Issues:** Check logs first, then consult HDA_SPEC.md

For additional support, review the repository README.md.
