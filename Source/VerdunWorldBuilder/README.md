# Verdun World Builder Plugin for Unreal Engine 5

**Automated world generation for Battle of Verdun (1916) using historical terrain data**

## Overview

This plugin automates the creation of the Verdun battlefield in Unreal Engine 5 using **Tier 1 documented historical data** from the Verdun Anthology. It loads fort locations, elevations, village positions, and terrain parameters from JSON/CSV files, then provides systems for you to generate your own artillery impacts, craters, and trenches.

## Features

### ✅ Tier 1 Data Loading (Documented Facts)
- **67 historical locations** with precise coordinates
  - 7 major fortifications (Douaumont, Vaux, Souville, etc.)
  - 11 destroyed villages (Fleury, Douaumont village, etc.)
  - 4 strategic hills (Mort-Homme, Hill 304, etc.)
  - Artillery positions, observation posts, ravines, woods
- **Geographic data**: Battlefield bounds, elevation ranges, center point
- **Surface materials**: 70% mud, 15% chalk, 10% dead grass, 5% rock
- **Coordinate conversion**: Automatic lat/lon → UE5 world coordinates

### ✅ Artillery Impact Generation (YOU Control This)
- **Interface-based system** - implement your own crater/trench generator
- **Default generator included** - basic implementation you can extend
- **Shell types**: 75mm, 155mm, 305mm, 420mm with historical crater dimensions
- **Density zones**: Define high/medium/low intensity areas
- **Auto-generation**: Optionally auto-create high-density zones around forts

### ✅ Location Spawning
- Spawn custom actors at historical locations
- Configure different actor classes for each location type
- Automatic world position calculation from geographic coordinates

### ✅ Blueprint & C++ Support
- All systems exposed to Blueprints
- Implement custom artillery generators in Blueprint or C++
- Events for build stages (data loaded, locations spawned, etc.)

## What This Plugin Does NOT Include (You Generate These)

This plugin loads **Tier 1 only** - documented historical facts. You generate:
- ❌ **Tier 2**: Crater positions, trench paths (you generate from your artillery simulation)
- ❌ **Tier 3**: Exact dugout locations, individual tree positions

**Why?** You said you want to generate artillery impacts yourself, so the plugin provides the framework and leaves the actual terrain modification to you.

## Installation

### 1. Copy Plugin to Your Project

```bash
# Copy the entire VerdunWorldBuilder folder to your project's Plugins directory
YourProject/
  Plugins/
    VerdunWorldBuilder/
      Source/
        VerdunWorldBuilder/
          Public/
          Private/
          VerdunWorldBuilder.Build.cs
          VerdunWorldBuilder.uplugin
```

### 2. Copy Data Files to Content

Copy the terrain data files from the repository to your project:

```bash
YourProject/Content/VerdunData/
  terrain_parameters.json
  locations_database.csv
```

From repository location:
```bash
verdun_anthology/geographic_data/terrain_parameters.json
verdun_anthology/geographic_data/locations_database.csv
```

### 3. Enable Plugin in Unreal Engine

1. Open your project
2. Edit → Plugins
3. Search for "Verdun World Builder"
4. Check the "Enabled" box
5. Restart the editor

## Quick Start

### Method 1: Blueprint Usage (Easiest)

1. **Add Manager to Level**
   - Drag `AVerdunWorldBuilderManager` into your level from Place Actors panel
   - Or create a Blueprint derived from `AVerdunWorldBuilderManager`

2. **Configure Data Paths**
   - Select the manager actor
   - In Details panel, set:
     - `Terrain Parameters Path`: `Content/VerdunData/terrain_parameters.json`
     - `Locations Database Path`: `Content/VerdunData/locations_database.csv`

3. **Configure Spawning** (optional)
   - Set `Fortification Actor Class` to your custom fort actor
   - Set `Destroyed Village Actor Class` to your custom village ruins actor
   - Enable/disable spawning for each location type

4. **Configure Artillery Generation**
   - Set `b Generate Artillery Impacts` to true
   - Set `b Auto Generate High Density Zones Around Forts` to true
   - Adjust `Fort Crater Density Per Hectare` (default: 75)
   - Adjust `Fort High Density Radius` (default: 2km)

5. **Run World Building**
   - Check `b Auto Load On Begin Play` to auto-run on level start
   - Or call `Start World Building` manually from Blueprint

### Method 2: Create Custom Artillery Generator (Recommended)

If you want full control over crater/trench generation:

1. **Create Blueprint Class**
   - Parent Class: `VerdunDefaultArtilleryGenerator`
   - Name it `BP_MyArtilleryGenerator`

2. **Override Functions**
   - `Generate Shell Impacts`: Your crater scatter algorithm
   - `Apply Crater To Landscape`: Your landscape sculpting logic
   - `Generate Trench`: Your trench generation logic

3. **Set in Manager**
   - In `AVerdunWorldBuilderManager`, set:
     - `Artillery Generator Class` → `BP_MyArtilleryGenerator`

## Usage Examples

### Example 1: Load Data and Spawn Locations Only

```cpp
// C++ example
AVerdunWorldBuilderManager* Manager = GetWorld()->SpawnActor<AVerdunWorldBuilderManager>();
Manager->TerrainParametersPath = FPaths::ProjectContentDir() + TEXT("VerdunData/terrain_parameters.json");
Manager->LocationsDatabasePath = FPaths::ProjectContentDir() + TEXT("VerdunData/locations_database.csv");
Manager->bGenerateArtilleryImpacts = false; // Don't generate craters yet
Manager->bSpawnFortifications = true;
Manager->FortificationActorClass = MyFortActorClass;
Manager->StartWorldBuilding();
```

### Example 2: Find Specific Location

```cpp
FVerdunLocation FortDouaumont;
if (Manager->FindLocationByName(TEXT("Fort Douaumont"), FortDouaumont))
{
    UE_LOG(LogTemp, Log, TEXT("Found Fort Douaumont at: %s"), *FortDouaumont.WorldPosition.ToString());
    // Spawn something there...
}
```

### Example 3: Get All Forts

```cpp
TArray<FVerdunLocation> Forts = Manager->GetLocationsByType(EVerdunLocationType::Fortification);
for (const FVerdunLocation& Fort : Forts)
{
    UE_LOG(LogTemp, Log, TEXT("Fort: %s at elevation %.0fm"), *Fort.Name, Fort.ElevationMeters);
}
```

### Example 4: Custom Artillery Generator (Blueprint)

In your `BP_MyArtilleryGenerator` Blueprint:

**Generate Shell Impacts Event:**
```
1. Get Zone Area in Hectares
2. Calculate Total Craters = Area × Density
3. FOR each crater:
   - Random position in circle
   - Random shell type (weighted by percentages)
   - Get crater dimensions
   - Add to output array
```

**Apply Crater To Landscape Event:**
```
1. Get landscape heightmap at impact location
2. Calculate crater radius in landscape units
3. FOR each point in radius:
   - Calculate depth using falloff curve (smooth crater edge)
   - Lower heightmap by calculated depth
4. Update landscape
```

## Data Structures

### FVerdunLocation

```cpp
USTRUCT(BlueprintType)
struct FVerdunLocation
{
    FString Name;                      // "Fort Douaumont"
    EVerdunLocationType Type;          // Fortification, Hill, DestroyedVillage, etc.
    double Latitude;                   // 49.2106°N
    double Longitude;                  // 5.4272°E
    float ElevationMeters;             // 388m
    FString Significance;              // Historical importance
    FString Phase;                     // Battle phase
    FString DateKey;                   // Key dates
    FString Notes;                     // Additional info
    FVector WorldPosition;             // Converted UE5 position (cm)
};
```

### FVerdunShellImpact

```cpp
USTRUCT(BlueprintType)
struct FVerdunShellImpact
{
    EVerdunShellType ShellType;        // 75mm, 155mm, 305mm, 420mm
    FVector ImpactLocation;            // World position
    float CraterDiameterMeters;        // 2m - 30m
    float CraterDepthMeters;           // 0.5m - 6m
    float TimeStamp;                   // When it hit
};
```

### Shell Type Dimensions (Historical Data)

| Shell Type | Diameter | Depth | Percentage |
|------------|----------|-------|------------|
| 75mm Field Gun | 2m | 0.5m | 60% |
| 155mm Howitzer | 6m | 1.5m | 25% |
| 305mm Heavy | 15m | 3m | 10% |
| 420mm Super-Heavy | 30m | 6m | 5% |

## Events You Can Listen To

```cpp
// Blueprint events
OnDataLoadComplete(bool bSuccess)
OnLocationsSpawned(int32 NumLocations)
OnArtilleryGenerationComplete(int32 NumImpacts)
OnWorldBuildingComplete()
OnBuildStageChanged(EVerdunBuildStage NewStage)
```

## File Formats

### terrain_parameters.json

Contains:
- `geographic_coverage`: Battlefield center, bounds, area
- `elevation_data`: Min/max/mean elevations
- `surface_materials`: Mud/chalk/grass/rock percentages
- `key_elevations_meters`: Fort and hill elevations

### locations_database.csv

Format:
```csv
name,type,latitude,longitude,elevation_m,significance,phase,date_key,notes
Fort Douaumont,fortification,49.2106,5.4272,388,Primary objective,...
```

## Coordinate System

**Source:** WGS84 (latitude/longitude)
**UE5 Output:** Local coordinates in centimeters from battlefield center

**Conversion:**
- Center: 49.2050°N, 5.4300°E, 365m elevation
- X-axis: East/West (longitude)
- Y-axis: North/South (latitude)
- Z-axis: Elevation (up)

**Example:**
- Fort Douaumont at 49.2106°N, 5.4272°E, 388m
- Converts to: X=-20,365cm, Y=62,339cm, Z=38,800cm

## Architecture

```
VerdunWorldBuilderManager (Actor)
  ├─ VerdunTerrainDataLoader (Static class)
  │   ├─ LoadTerrainParameters()
  │   ├─ LoadLocationsDatabase()
  │   └─ ConvertGeoToWorld()
  │
  ├─ IVerdunArtilleryImpactGenerator (Interface)
  │   ├─ GenerateShellImpacts()
  │   ├─ ApplyCraterToLandscape()
  │   └─ GenerateTrench()
  │
  └─ VerdunDefaultArtilleryGenerator (Default implementation)
      └─ Override in your own class for custom behavior
```

## Build Stages

1. **Loading Data**: Reads JSON/CSV files
2. **Spawning Locations**: Places 67 historical location actors
3. **Generating Artillery**: Your custom generator creates impacts
4. **Applying Materials**: Sets landscape materials (placeholder)
5. **Complete**: World building finished

## Performance Considerations

- **67 locations**: Minimal performance impact
- **Artillery impacts**: Depends on your implementation
  - 500,000 craters = performance impact
  - Start with high-density zones around forts only
  - Use LOD for distant craters
- **Landscape modification**: Can be slow - consider doing it in chunks

## Extending the Plugin

### Add Custom Location Type

1. Edit `EVerdunLocationType` in `VerdunTerrainData.h`
2. Add parsing in `VerdunTerrainDataLoader::ParseLocationType()`
3. Add spawning logic in `AVerdunWorldBuilderManager::SpawnLocationActor()`

### Implement Advanced Artillery

Override `UVerdunDefaultArtilleryGenerator`:

```cpp
UCLASS()
class UMyAdvancedArtillery : public UVerdunDefaultArtilleryGenerator
{
    GENERATED_BODY()

public:
    virtual void ApplyCraterToLandscape_Implementation(const FVerdunShellImpact& Impact, ALandscape* Landscape) override
    {
        // Your custom landscape sculpting
        // Use FLandscapeEditDataInterface
        // Apply noise, erosion, etc.
    }
};
```

## Troubleshooting

### "Failed to load terrain parameters"
- Check file paths are correct
- Ensure files exist in `Content/VerdunData/`
- Verify JSON is valid

### "No locations spawned"
- Check actor classes are assigned
- Enable spawning flags for desired types
- Look for errors in Output Log

### "Artillery generator is null"
- Plugin will auto-use `UVerdunDefaultArtilleryGenerator`
- Set custom class in manager settings

## Historical Accuracy

This plugin uses **Tier 1 documented data only**:
- ✅ Fort locations (French military archives)
- ✅ Village positions (verified maps)
- ✅ Hill elevations (military surveys)
- ✅ Surface materials (photos, testimonies)

You generate (Tier 2/3):
- Artillery crater positions (from your simulation)
- Trench paths (procedural or manual)
- Terrain destruction (dynamic or pre-baked)

## Credits

**Data Source:** Verdun Anthology (Only The Dead Development Book)
**Historical Research:** Based on French military archives, meteorological records, and verified sources
**Plugin Author:** Claude (Verdun World Builder)
**Game:** Only The Dead - Verdun 1916 Survival Game

## License

Part of the "Only The Dead" game development project.

---

## Next Steps

1. **Copy data files** to `Content/VerdunData/`
2. **Enable plugin** in Unreal Engine
3. **Add manager actor** to your level
4. **Configure paths** and settings
5. **Create custom artillery generator** (optional)
6. **Run world building** and iterate!

**Questions?** See the Verdun Anthology documentation in `verdun_anthology/` for complete historical data.
