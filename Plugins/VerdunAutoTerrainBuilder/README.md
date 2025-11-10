# Verdun Auto Terrain Builder

**Version:** 1.0.0
**Game:** Only The Dead - A WWI Survival Horror Game
**Engine:** Unreal Engine 5

## Overview

The **Verdun Auto Terrain Builder** is a comprehensive UE5 plugin that automatically generates the historically accurate Battle of Verdun (1916) battlefield from geographic and historical data. This plugin reads real elevation data, historical location databases, and terrain parameters to create a fully detailed 60 km² battlefield complete with:

- Real-world elevation data from NASA SRTM
- 500,000+ procedurally generated shell craters
- 800km of historically accurate trench systems
- 80+ historical location markers (forts, villages, hills, ravines)
- Authentic material composition (mud, chalk, dead grass, rock)

## Features

### 🗺️ Geographic Data Integration
- **Real Elevation Data**: Imports 16-bit PNG heightmaps generated from NASA SRTM 30m data
- **WGS84 to UE5 Coordinate Conversion**: Automatically converts latitude/longitude to UE5 local coordinates
- **Historical Accuracy**: All data based on French military archives, battlefield archaeology, and verified sources

### 💣 Procedural Crater Generation
- **Intelligent Density Distribution**:
  - High intensity (50-100 craters/hectare) near major forts (Douaumont, Vaux, Souville)
  - Medium intensity (20-50 craters/hectare) across Right Bank combat zone
  - Low intensity (5-20 craters/hectare) in periphery and Left Bank
- **4 Shell Types**: 75mm (60%), 155mm (25%), 305mm (10%), 420mm (5%)
- **Water-Filled Craters**: 60% of craters contain stagnant water (rainy season)
- **500,000 Total Craters**: Distributed across the battlefield based on historical bombardment data

### 🪖 Trench System Generation
- **Zigzag Pattern**: Authentic WWI zigzag trenches with traverses every 10-15 meters
- **Multiple Trench Types**:
  - Front-line fire trenches
  - Support trenches (50-100m behind front)
  - Reserve trenches (200-300m behind front)
  - Communication trenches (connecting front to rear)
- **Fort Defenses**: Concentric trench rings around major fortifications
- **Spline-Based**: Trenches generated as UE5 spline components for easy editing

### 📍 Historical Location Placement
- **80+ Verified Locations**:
  - 6 Major forts (Douaumont, Vaux, Souville, Tavannes, Thiaumont, Froideterre)
  - 9 Destroyed villages ("villages that disappeared")
  - Strategic hills (Mort-Homme, Hill 304, Côte de l'Oie)
  - Woods, ravines, observation posts, artillery positions
- **Automatic Actor Spawning**: Place markers at all historical locations
- **Metadata Rich**: Each location includes significance, battle phase, dates, and notes

### 🎨 Material System
- **4 Surface Materials**:
  - Mud (70%) - Brown-grey, wet, churned
  - Chalk (15%) - White to light grey, exposed in craters
  - Dead Grass (10%) - Brown, sparse, 99% destroyed
  - Rock (5%) - Grey limestone fragments
- **Seasonal Variations**: Winter (frozen), Spring (peak mud), Summer (drying), Fall (rain returns)

## Installation

### Prerequisites
- Unreal Engine 5.0 or higher
- C++ project (plugin requires C++ compilation)
- `verdun_anthology/geographic_data/` folder in your project root

### Step 1: Copy Plugin to Project
```bash
# Copy the plugin to your project's Plugins folder
cp -r Plugins/VerdunAutoTerrainBuilder <YourProject>/Plugins/
```

### Step 2: Regenerate Project Files
```bash
# Right-click your .uproject file and select "Generate Visual Studio project files"
# Or use command line:
<UE5Install>/Engine/Build/BatchFiles/GenerateProjectFiles.bat <YourProject>.uproject
```

### Step 3: Compile Plugin
1. Open your project in your IDE (Visual Studio, Rider, etc.)
2. Build the project (this will compile the plugin)
3. Alternatively, open the project in UE5 Editor and it will prompt you to rebuild

### Step 4: Enable Plugin
1. Open your project in UE5 Editor
2. Go to **Edit → Plugins**
3. Search for "Verdun Auto Terrain Builder"
4. Check the "Enabled" checkbox
5. Restart the editor

## Usage

### Quick Start: One-Click Terrain Building

The fastest way to build the Verdun battlefield:

1. **Open the Terrain Builder**:
   - Go to **Window → Verdun Auto Terrain Builder**
   - Or click the **"Verdun Terrain"** button in the Level Editor toolbar

2. **Click "Yes"** to start the automatic build process

3. The plugin will:
   - Load terrain parameters from `terrain_parameters.json`
   - Load 80+ locations from `locations_database.csv`
   - Generate 500,000 craters
   - Generate trench networks
   - Place location markers

4. **Check the World Outliner** for:
   - `VerdunCraterGenerator` actor
   - `VerdunTrenchGenerator` actor
   - Location marker actors

### Manual Control: Step-by-Step

For more control over the process:

#### 1. Create Landscape from Heightmap

**First, run the Python export script** (if you haven't already):
```bash
cd verdun_anthology/geographic_data
python export_for_ue5.py
```

This creates the `UE5_Import/` folder with:
- `verdun_landscape_heightmap.png` (16-bit heightmap)
- `verdun_landscape_config.json` (import settings)
- `verdun_locations_ue5.json` (location data with UE5 transforms)

**Import Heightmap in UE5**:
1. Go to **Landscape Mode** (Shift+2)
2. Click **Import from File**
3. Select `verdun_landscape_heightmap.png`
4. Set **Scale**:
   - X: 100.0, Y: 100.0, Z: 100.0 (for 30m resolution)
5. Click **Import**

#### 2. Load Terrain Parameters

In Blueprint or C++:
```cpp
// C++ Example
UVerdunTerrainParameters* Params = NewObject<UVerdunTerrainParameters>();
FString JsonPath = FPaths::ProjectDir() / TEXT("verdun_anthology/geographic_data/terrain_parameters.json");
Params->LoadFromJSON(JsonPath);
```

#### 3. Load Locations Database

```cpp
// C++ Example
UVerdunLocationDatabase* Locations = NewObject<UVerdunLocationDatabase>();
FString CsvPath = FPaths::ProjectDir() / TEXT("verdun_anthology/geographic_data/locations_database.csv");
Locations->LoadFromCSV(CsvPath);
Locations->ConvertToUE5Coordinates(Params);
```

#### 4. Generate Craters

```cpp
// Spawn crater generator
AVerdunCraterGenerator* CraterGen = World->SpawnActor<AVerdunCraterGenerator>();
CraterGen->TerrainParameters = Params;
CraterGen->LocationDatabase = Locations;
CraterGen->GenerateCraters();

// Optional: Spawn crater meshes
// CraterGen->SpawnCraterMeshes(YourCraterActorClass);
```

#### 5. Generate Trenches

```cpp
// Spawn trench generator
AVerdunTrenchGenerator* TrenchGen = World->SpawnActor<AVerdunTrenchGenerator>();
TrenchGen->TerrainParameters = Params;
TrenchGen->LocationDatabase = Locations;
TrenchGen->GenerateTrenchNetworks();
TrenchGen->CreateSplineVisualization();
```

#### 6. Place Location Markers

```cpp
// Place markers for all locations
for (const FVerdunLocation& Loc : Locations->Locations)
{
    AActor* Marker = World->SpawnActor<AActor>(ActorClass, Loc.UE5Location, FRotator::ZeroRotator);
    Marker->SetActorLabel(Loc.Name);
}
```

## Data Files

The plugin reads from `verdun_anthology/geographic_data/`:

### Required Files
- **`terrain_parameters.json`** (548 lines)
  - Surface materials, crater parameters, trench specifications
  - Elevation ranges, vegetation survival rates
  - Weather effects, gameplay parameters

- **`locations_database.csv`** (80+ locations)
  - Format: `name,type,latitude,longitude,elevation_m,significance,phase,date_key,notes`
  - All major forts, destroyed villages, hills, ravines, woods

### Optional Files
- **`UE5_Import/verdun_landscape_heightmap.png`** (16-bit heightmap)
- **`UE5_Import/verdun_landscape_config.json`** (landscape import settings)
- **`UE5_Import/verdun_locations_ue5.json`** (pre-converted UE5 coordinates)

Generate these with:
```bash
python verdun_anthology/geographic_data/export_for_ue5.py
```

## Blueprint Integration

All major classes are Blueprint-accessible:

### Terrain Parameters
```
VerdunTerrainParameters (UObject)
├── LoadFromJSON(FilePath)
├── ConvertLatLonToUE5Coords(Lat, Lon, Elevation) → Vector
├── SurfaceMaterials (Mud, Chalk, Grass, Rock percentages)
├── CraterDensity (High, Medium, Low intensity)
└── TrenchParameters (Width, Depth, Pattern)
```

### Location Database
```
VerdunLocationDatabase (UObject)
├── LoadFromCSV(FilePath)
├── LoadFromJSON(FilePath)
├── GetFortifications() → Array<VerdunLocation>
├── GetDestroyedVillages() → Array<VerdunLocation>
├── GetHills() → Array<VerdunLocation>
└── GetLocationByName(Name) → VerdunLocation
```

### Crater Generator
```
VerdunCraterGenerator (Actor)
├── GenerateCraters()
├── ClearCraters()
├── SpawnCraterMeshes(CraterActorClass)
├── GetCraterDensityAtLocation(Location) → Float
└── GeneratedCraters → Array<GeneratedCrater>
```

### Trench Generator
```
VerdunTrenchGenerator (Actor)
├── GenerateTrenchNetworks()
├── GenerateFortTrenches(Location, Radius, NumRings)
├── GenerateFrontLineTrench(Start, End, Type)
├── CreateSplineVisualization()
└── GeneratedTrenches → Array<TrenchSegment>
```

## Coordinate System

The plugin uses a custom coordinate system:

**Origin**: Battlefield center (49.2050°N, 5.4300°E, 365m elevation)

**Conversion Formula**:
```
X (cm) = (Longitude - 5.4300) × 71,370 m/degree × 100 cm/m
Y (cm) = (Latitude - 49.2050) × 111,320 m/degree × 100 cm/m
Z (cm) = Elevation (meters) × 100 cm/m
```

**Coverage**:
- X-axis (Longitude): ~11.4 km (5.31°E to 5.47°E)
- Y-axis (Latitude): ~12.2 km (49.14°N to 49.25°N)
- Z-axis (Elevation): 165m to 390m (225m range)

**Total Area**: ~60 km² (~140 km²)

## Customization

### Adjust Crater Density
```cpp
CraterGenerator->CraterDensity.HighIntensity = 100.0f; // craters per hectare
CraterGenerator->CraterDensity.MediumIntensity = 50.0f;
CraterGenerator->CraterDensity.LowIntensity = 20.0f;
```

### Modify Trench Pattern
```cpp
TrenchGenerator->TraverseSpacingCm = 1000.0f; // 10m zigzag spacing
TrenchGenerator->TraverseAngleDegrees = 90.0f; // 90-degree traverses
TrenchGenerator->TrenchCollapsePercentage = 70.0f; // 70% collapsed
```

### Change Material Composition
```cpp
TerrainParameters->SurfaceMaterials.Mud = 80.0f; // 80% mud
TerrainParameters->SurfaceMaterials.Chalk = 10.0f;
TerrainParameters->SurfaceMaterials.DeadGrass = 5.0f;
TerrainParameters->SurfaceMaterials.Rock = 5.0f;
```

### Adjust Battlefield Size
```cpp
CraterGenerator->BattlefieldSizeMeters = FVector2D(10000.0f, 10000.0f); // 100 km²
```

## Performance Considerations

### Crater Generation
- **500,000 craters** = High memory usage
- **Recommended**: Use LOD meshes for craters
- **Optimization**: Generate craters in chunks/streaming volumes
- **Alternative**: Use landscape layer painting instead of mesh spawning

### Trench Generation
- **800km of trenches** = Thousands of spline segments
- **Recommended**: Use spline mesh components
- **Optimization**: Cull distant trench segments
- **Alternative**: Bake trenches into landscape heightmap

### Location Markers
- **80+ actors** = Minimal performance impact
- Replace empty actors with actual fort/village meshes as needed

## Troubleshooting

### "Failed to load terrain parameters"
- Check that `verdun_anthology/geographic_data/terrain_parameters.json` exists in project root
- Verify JSON is valid (use online JSON validator)
- Check UE5 Output Log for detailed error

### "Failed to load locations database"
- Check that `locations_database.csv` exists
- Verify CSV format (9 columns: name, type, lat, lon, elev, significance, phase, date, notes)
- Watch for duplicate "type" column issue (line 29: "woods,woods,49.2331...")

### "Crater generator spawned 0 craters"
- Ensure `TerrainParameters` and `LocationDatabase` are set
- Check `RandomSeed` is not causing issues
- Verify `BattlefieldSizeMeters` is reasonable (default: 8000m × 8000m)

### "Trenches not visible"
- Call `CreateSplineVisualization()` after generating trenches
- Check World Outliner for `VerdunTrenchGenerator` actor
- Splines may be hidden - toggle visibility in Details panel

### Plugin won't compile
- Verify UE5 version compatibility (requires UE5.0+)
- Check that all module dependencies are available:
  - Landscape, LandscapeEditor, Json, JsonUtilities, ProceduralMeshComponent
- Regenerate project files and rebuild

## Technical Details

### Module Structure
```
VerdunAutoTerrainBuilder/
├── Source/
│   ├── VerdunAutoTerrainBuilder/        (Runtime module)
│   │   ├── Public/
│   │   │   ├── VerdunTerrainParameters.h
│   │   │   ├── VerdunLocationData.h
│   │   │   ├── VerdunCraterGenerator.h
│   │   │   └── VerdunTrenchGenerator.h
│   │   └── Private/
│   │       └── (Implementations)
│   └── VerdunAutoTerrainBuilderEditor/  (Editor module)
│       ├── Public/
│       │   ├── VerdunAutoTerrainBuilderEditor.h
│       │   └── VerdunTerrainBuilderSubsystem.h
│       └── Private/
│           └── (Implementations)
└── VerdunAutoTerrainBuilder.uplugin
```

### Key Classes
- **UVerdunTerrainParameters**: Parses and stores terrain parameters from JSON
- **UVerdunLocationDatabase**: Loads and manages historical locations
- **AVerdunCraterGenerator**: Procedurally generates and places craters
- **AVerdunTrenchGenerator**: Generates spline-based trench networks
- **UVerdunTerrainBuilderSubsystem**: Editor subsystem for orchestrating builds

## Future Enhancements

### Planned Features
- [ ] Direct landscape heightmap modification for craters
- [ ] Automatic trench excavation in landscape
- [ ] Material layer painting (mud, chalk, grass, rock)
- [ ] Destroyed vegetation placement (dead trees, stumps)
- [ ] Barbed wire mesh generation along trenches
- [ ] Rubble pile placement for destroyed villages
- [ ] Dugout entrance placement
- [ ] Shell hole water simulation
- [ ] Seasonal terrain variation (winter snow, spring mud)

## Credits

**Plugin Developer**: Claude (Anthropic)
**Game Project**: Only The Dead - WWI Survival Horror
**Historical Research**: Verdun Anthology (216,000+ words of verified research)
**Elevation Data**: NASA SRTM 30m (Shuttle Radar Topography Mission)
**Historical Sources**:
- Alistair Horne: *The Price of Glory: Verdun 1916*
- Paul Jankowski: *Verdun: The Longest Battle of the Great War*
- Service Historique de la Défense (French military archives)
- Institut Géographique National (French mapping agency)

## License

This plugin is part of the "Only The Dead" game project.
© 2025 - All Rights Reserved

## Support

For issues, questions, or contributions:
- Check the UE5 Output Log for detailed error messages
- Review the `terrain_parameters.json` and `locations_database.csv` for data accuracy
- Consult the Verdun Research Anthology for historical context

---

**Version History**:
- **1.0.0** (2025): Initial release
  - Terrain parameter import
  - Location database import
  - Crater generation (500,000 craters)
  - Trench generation (800km zigzag trenches)
  - Location marker placement
  - One-click terrain building UI
