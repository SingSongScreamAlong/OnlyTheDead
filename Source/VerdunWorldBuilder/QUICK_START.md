# Verdun World Builder - Quick Start Guide

## 5-Minute Setup

### Step 1: Install Plugin
```bash
# Plugin is already in your project at:
# Source/VerdunWorldBuilder/

# Enable it in Unreal Engine:
# Edit → Plugins → Search "Verdun World Builder" → Enable → Restart
```

### Step 2: Copy Data Files
```bash
# Copy these files to your project:
Source location: verdun_anthology/geographic_data/
  - terrain_parameters.json
  - locations_database.csv

Destination: YourProject/Content/VerdunData/
  - terrain_parameters.json
  - locations_database.csv
```

### Step 3: Add Manager to Level
1. Open your Verdun level
2. Place Actors panel → Search "VerdunWorldBuilderManager"
3. Drag into level

### Step 4: Configure Manager
Select the manager actor and set:

**Data Loading:**
- Terrain Parameters Path: `Content/VerdunData/terrain_parameters.json`
- Locations Database Path: `Content/VerdunData/locations_database.csv`
- ✅ Auto Load On Begin Play

**Location Spawning:**
- ✅ Spawn Fortifications
- ✅ Spawn Destroyed Villages
- ✅ Spawn Hills
- Set Actor Classes (optional - leave empty to just log positions)

**Artillery Generation:**
- ✅ Generate Artillery Impacts
- ✅ Auto Generate High Density Zones Around Forts
- Fort Crater Density Per Hectare: `75.0` (high intensity)
- Fort High Density Radius: `200000.0` (2km in cm)

**Landscape:**
- ✅ Auto Find Landscape (or drag your landscape actor here)

### Step 5: Run!
- Click Play
- Check Output Log for results:
  ```
  === VERDUN WORLD BUILDING STARTED ===
  Loaded 67 locations from database
  Spawned X location actors
  Generated Y artillery impacts
  === VERDUN WORLD BUILDING COMPLETE ===
  ```

## What Happens

1. **Loads Data** (1 second)
   - Reads terrain_parameters.json
   - Reads locations_database.csv with 67 historical locations
   - Converts lat/lon to UE5 world coordinates

2. **Spawns Locations** (instant)
   - 7 fortifications (Fort Douaumont, Vaux, etc.)
   - 11 destroyed villages (Fleury, etc.)
   - 4 hills (Mort-Homme, Hill 304, etc.)
   - Other locations if actor classes assigned

3. **Generates Artillery** (depends on density)
   - Auto-creates high-density zones around each fort
   - Generates shell impacts using default algorithm
   - Calls ApplyCraterToLandscape() for each (placeholder by default)

4. **Done!**

## Next: Implement Your Own Artillery

The default generator just logs craters - it doesn't actually modify the landscape. To generate real craters:

### Option A: Blueprint Implementation

1. Create Blueprint class: `BP_MyArtilleryGenerator`
   - Parent: `VerdunDefaultArtilleryGenerator`

2. Override `Apply Crater To Landscape`:
   ```
   [Impact] → Get Crater Diameter/Depth
   → Get Landscape Heightmap at Location
   → Modify Height in Crater Radius (with falloff)
   → Update Landscape
   ```

3. In World Builder Manager:
   - Artillery Generator Class → `BP_MyArtilleryGenerator`

### Option B: C++ Implementation

```cpp
UCLASS()
class UMyArtilleryGenerator : public UVerdunDefaultArtilleryGenerator
{
    GENERATED_BODY()

public:
    virtual void ApplyCraterToLandscape_Implementation(
        const FVerdunShellImpact& Impact,
        ALandscape* Landscape) override
    {
        if (!Landscape) return;

        // 1. Get landscape edit interface
        FLandscapeEditDataInterface LandscapeEdit(Landscape->GetLandscapeInfo());

        // 2. Calculate crater area
        float RadiusCm = Impact.CraterDiameterMeters * 50.0f; // Half diameter in cm
        float DepthCm = Impact.CraterDepthMeters * 100.0f;

        // 3. Modify heightmap in crater area
        // (Iterate over landscape quads within radius)
        // (Lower each point by depth with smooth falloff)

        // 4. Flush changes
        LandscapeEdit.Flush();
    }
};
```

## Data You Get

### Fortifications (7)
- Fort Douaumont (388m elevation)
- Fort Vaux (390m)
- Fort Souville (380m)
- Fort Tavannes (350m)
- Fort Thiaumont (385m)
- Fort Froideterre (345m)

### Destroyed Villages (11)
- Fleury-devant-Douaumont (changed hands 16 times)
- Douaumont village
- Vaux village
- Beaumont, Ornes, Louvemont, Haumont, Bezonvaux, Cumières

### Hills (4)
- Mort-Homme (295m) - "The Dead Man"
- Hill 304 (304m)
- Côte de l'Oie (361m)
- Côte du Poivre (365m)

### Surface Materials
- 70% Mud (ankle to knee deep)
- 15% Chalk (white Verdun chalk)
- 10% Dead grass (99% destroyed)
- 5% Rock (limestone)

## Shell Types & Crater Dimensions

| Type | Diameter | Depth | % |
|------|----------|-------|---|
| 75mm | 2m | 0.5m | 60% |
| 155mm | 6m | 1.5m | 25% |
| 305mm | 15m | 3m | 10% |
| 420mm | 30m | 6m | 5% |

## Output Log Example

```
LogTemp: === VERDUN WORLD BUILDING STARTED ===
LogTemp: Loading terrain data...
LogTemp:   Terrain params: .../terrain_parameters.json
LogTemp:   Locations DB: .../locations_database.csv
LogTemp: Successfully loaded terrain parameters: Verdun_Battlefield_1916
LogTemp:   Center: 49.2050°N, 5.4300°E, 365m
LogTemp:   Elevation: 165-390m (mean: 300m)
LogTemp: Successfully loaded 67 locations from database
LogTemp: Terrain data loaded successfully:
LogTemp:   Name: Verdun_Battlefield_1916
LogTemp:   Locations: 67
LogTemp: Spawning historical locations...
LogTemp:   Location: Fort Douaumont (Fortification) at X=-20365.0 Y=62339.0 Z=38800.0
LogTemp:   Location: Fort Vaux (Fortification) at ...
LogTemp: Spawned 0 location actors (no actor classes assigned)
LogTemp: Auto-generating crater density zones around forts...
LogTemp:   Added high-density zone around Fort Douaumont (75 craters/hectare)
LogTemp:   Added high-density zone around Fort Vaux (75 craters/hectare)
LogTemp: Generating shell impacts...
LogTemp: Generating 9432 shell impacts in 2000.0m radius
LogTemp: Generated 9432 shell impacts
LogTemp:   Zone 'high_density_fort_Fort Douaumont': 9432 impacts
LogTemp: Applying crater at X=... (2.1m diameter, 0.5m deep) - [PLACEHOLDER]
LogTemp: Generated total of 66024 artillery impacts
LogTemp: Applying terrain materials...
LogTemp:   Composition: 70% mud, 15% chalk, 10% grass, 5% rock
LogTemp: Material application not yet implemented - set materials manually
LogTemp: === VERDUN WORLD BUILDING COMPLETE ===
```

## Troubleshooting

**No data loaded?**
- Check file paths in manager settings
- Verify files are in Content/VerdunData/

**No actors spawned?**
- This is normal if you haven't assigned actor classes
- Locations are still loaded and available via code

**Craters not appearing?**
- Default generator is placeholder only
- Implement `ApplyCraterToLandscape` to actually modify terrain

**Performance slow?**
- 66,024 craters around 7 forts is a lot!
- Reduce `Fort Crater Density Per Hectare` to ~25
- Or disable auto-generation and create zones manually

## Support

- Full docs: `README.md`
- Data source: `verdun_anthology/geographic_data/`
- Example test: `verdun_anthology/geographic_data/test_terrain_automation.py`

---

**You're ready to build Verdun!** 🎖️
