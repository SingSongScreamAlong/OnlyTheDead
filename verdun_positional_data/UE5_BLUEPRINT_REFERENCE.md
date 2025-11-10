# UE5 Blueprint Reference - Verdun Location Spawner

## Overview

This document provides step-by-step Blueprint instructions for creating a Verdun location spawner system in Unreal Engine 5.

## Blueprint 1: GPS Coordinate Converter

### Create: BP_GPSConverter (Blueprint Function Library)

This utility provides conversion functions for the entire project.

**Variables:** None (all functions are static/pure)

---

### Function: ConvertGPSToWorld

**Description:** Converts GPS coordinates to UE5 world space

**Category:** Coordinate Conversion

**Pure:** Yes (check the "Pure" box)

**Inputs:**
- `Latitude` (Float) - GPS latitude in decimal degrees
- `Longitude` (Float) - GPS longitude in decimal degrees
- `Elevation` (Float) - Elevation in meters

**Outputs:**
- `WorldLocation` (Vector) - UE5 world coordinates in cm

**Local Variables:**
- `OriginLat` (Float) = 49.1667
- `OriginLon` (Float) = 5.3833
- `OriginElev` (Float) = 200.0
- `DeltaLat` (Float)
- `DeltaLon` (Float)
- `DeltaElev` (Float)
- `X` (Float)
- `Y` (Float)
- `Z` (Float)

**Blueprint Logic:**

```
[Input: Latitude] ─┐
[Float: 49.1667]  ─┤→ [Float - Float] → [DeltaLat]
                    └─ (subtract)

[Input: Longitude] ┐
[Float: 5.3833]   ─┤→ [Float - Float] → [DeltaLon]
                    └─ (subtract)

[Input: Elevation] ┐
[Float: 200.0]    ─┤→ [Float - Float] → [DeltaElev]
                    └─ (subtract)

[DeltaLat] → [Float * Float] → [X]
              ↑
[Float: 11132000.0]

[DeltaLon] → [Float * Float] → [Y]
              ↑
[Float: 7267500.0]

[DeltaElev] → [Float * Float] → [Z]
               ↑
[Float: 100.0]

[X] ┐
[Y] ┤→ [Make Vector] → [Return: WorldLocation]
[Z] ┘
```

**Text Representation:**
```
1. DeltaLat = Latitude - 49.1667
2. DeltaLon = Longitude - 5.3833
3. DeltaElev = Elevation - 200.0

4. X = DeltaLat × 11132000.0
5. Y = DeltaLon × 7267500.0
6. Z = DeltaElev × 100.0

7. Return Make Vector(X, Y, Z)
```

---

### Function: ConvertWorldToGPS

**Description:** Converts UE5 world space back to GPS coordinates

**Pure:** Yes

**Inputs:**
- `WorldLocation` (Vector)

**Outputs:**
- `Latitude` (Float)
- `Longitude` (Float)
- `Elevation` (Float)

**Blueprint Logic:**

```
[Input: WorldLocation] → [Break Vector] → X, Y, Z

X → [Float / Float] → [+ Float] → [Latitude]
     ↑                  ↑
     11132000.0         49.1667

Y → [Float / Float] → [+ Float] → [Longitude]
     ↑                  ↑
     7267500.0          5.3833

Z → [Float / Float] → [+ Float] → [Elevation]
     ↑                  ↑
     100.0              200.0
```

---

## Blueprint 2: Verdun Data Structure

### Create: S_VerdunLocation (Structure)

**Variables:**
- `ID` (String)
- `Name` (String)
- `Type` (String)
- `Category` (String)
- `Latitude` (Float)
- `Longitude` (Float)
- `Elevation` (Float)
- `Population` (Integer)
- `HouseCount` (Integer)
- `Diameter` (Float)
- `Description` (Text)

---

### Create: S_VerdunUE5Location (Structure)

**Variables:**
- `ID` (String)
- `Name` (String)
- `Type` (String)
- `WorldLocation` (Vector)
- `Population` (Integer)
- `HouseCount` (Integer)
- `Diameter` (Float)
- `ActorClass` (Class Reference → Actor)

---

## Blueprint 3: Data Table Setup

### Import CSV as Data Table

1. **Right-click** in Content Browser
2. **Miscellaneous** → **Data Table**
3. Choose `S_VerdunLocation` as Row Structure
4. Name it `DT_VerdunLocations`
5. **Double-click** to open
6. **Import** → Select `all_locations_ue5.csv`

---

## Blueprint 4: Location Spawner Actor

### Create: BP_VerdunLocationSpawner (Actor)

This actor reads location data and spawns appropriate actors.

**Variables:**

| Name | Type | Category | Default | Description |
|------|------|----------|---------|-------------|
| `LocationDataTable` | DataTable | Import | None | Reference to DT_VerdunLocations |
| `VillageActorClass` | Class (Actor) | Spawning | None | Blueprint class for villages |
| `FortActorClass` | Class (Actor) | Spawning | None | Blueprint class for forts |
| `TerrainActorClass` | Class (Actor) | Spawning | None | Blueprint class for terrain |
| `SpawnOnBeginPlay` | Boolean | Settings | True | Auto-spawn on begin play |
| `DebugMode` | Boolean | Debug | False | Show debug spheres |
| `SpawnedActors` | Actor Array | Runtime | [] | Track spawned actors |

**Functions:**

---

### Function: SpawnAllLocations

**Inputs:** None
**Outputs:**
- `SpawnedCount` (Integer)

**Blueprint Logic:**

```
Event BeginPlay (or Custom Event)
  ↓
[LocationDataTable] → [Get Data Table Row Names]
  ↓
[For Each Loop]
  ↓
  Item: RowName (Name)
  ↓
  [Get Data Table Row]
    • Data Table: LocationDataTable
    • Row Name: RowName
    • Out Row: LocationData (S_VerdunLocation)
  ↓
  [Branch: Was Found?]
    ↓
    TRUE:
      ↓
      [SpawnLocationActor]
        • LocationData: LocationData
      ↓
      [Array: Add] → SpawnedActors
    ↓
    FALSE:
      ↓
      [Print String: "Failed to load row"]
  ↓
[Loop Body] completes
  ↓
[Print String]
  • Text: "Spawned {SpawnedActors.Length} locations"
```

---

### Function: SpawnLocationActor

**Inputs:**
- `LocationData` (S_VerdunLocation)

**Outputs:**
- `SpawnedActor` (Actor)

**Blueprint Logic:**

```
[LocationData] → [Break S_VerdunLocation]
  ↓
  Outputs: ID, Name, Type, Latitude, Longitude, Elevation, etc.
  ↓
[ConvertGPSToWorld]
  • Latitude: LocationData.Latitude
  • Longitude: LocationData.Longitude
  • Elevation: LocationData.Elevation
  ↓
  Returns: WorldLocation (Vector)
  ↓
[Switch on String: Type]
  ↓
  ├─ Case "village" → [VillageActorClass]
  ├─ Case "fort" → [FortActorClass]
  ├─ Case "terrain" → [TerrainActorClass]
  └─ Default → [None]
  ↓
  ActorClass
  ↓
[Branch: Is Valid?]
  ↓
  TRUE:
    ↓
    [Spawn Actor from Class]
      • Class: ActorClass
      • Location: WorldLocation
      • Rotation: (0, 0, 0)
      • Collision Handling: Always Spawn
      ↓
      SpawnedActor
      ↓
      [Cast to BP_VerdunLocationBase]
        ↓
        [Set Location Name: LocationData.Name]
        [Set Location ID: LocationData.ID]
        [Set Population: LocationData.Population]
      ↓
      [Branch: DebugMode?]
        ↓
        TRUE:
          ↓
          [Draw Debug Sphere]
            • Location: WorldLocation
            • Radius: 1000 cm
            • Color: Red
            • Duration: 999
          ↓
          [Print String]
            • Text: "Spawned {Name} at {WorldLocation}"
  ↓
  FALSE:
    ↓
    [Print String: "No actor class for type: {Type}"]
  ↓
[Return: SpawnedActor]
```

---

## Blueprint 5: Base Location Actor

### Create: BP_VerdunLocationBase (Actor)

Base class for all location actors (villages, forts, etc.)

**Variables:**

| Name | Type | Category | Default | Editable | Description |
|------|------|----------|---------|----------|-------------|
| `LocationID` | String | Identity | "" | Instance | Unique ID |
| `LocationName` | String | Identity | "" | Instance | Display name |
| `LocationType` | String | Identity | "" | Instance | Type (village/fort/etc) |
| `Population` | Integer | Data | 0 | Instance | Pre-war population |
| `GPS_Latitude` | Float | GPS | 0.0 | Instance | Original latitude |
| `GPS_Longitude` | Float | GPS | 0.0 | Instance | Original longitude |
| `GPS_Elevation` | Float | GPS | 0.0 | Instance | Original elevation |
| `ShowLabel` | Boolean | Display | True | Instance | Show 3D text label |
| `LabelOffset` | Vector | Display | (0,0,500) | Instance | Label offset |

**Components:**
- `SceneRoot` (Scene Component)
- `LocationMesh` (Static Mesh) - Optional placeholder
- `TextRender` (Text Render) - For location name

**Event Graph:**

```
Event BeginPlay
  ↓
[Branch: ShowLabel?]
  ↓
  TRUE:
    ↓
    [TextRender] → [Set Text]
      • Text: LocationName
    ↓
    [TextRender] → [Set World Location]
      • New Location: GetActorLocation + LabelOffset
    ↓
    [TextRender] → [Set World Scale]
      • New Scale: (5, 5, 5)
```

---

## Blueprint 6: Village Actor

### Create: BP_VerdunVillage (inherits from BP_VerdunLocationBase)

Spawns buildings to create a village.

**Variables:**

| Name | Type | Category | Default | Description |
|------|------|----------|---------|-------------|
| `HouseCount` | Integer | Buildings | 0 | Number of houses |
| `VillageDiameter` | Float | Layout | 300 | Village diameter in meters |
| `HouseClass` | Class (Actor) | Buildings | None | House blueprint |
| `ChurchClass` | Class (Actor) | Buildings | None | Church blueprint |
| `FarmClass` | Class (Actor) | Buildings | None | Farm blueprint |
| `LayoutPattern` | Enum | Layout | Circular | Layout type |
| `SpawnedBuildings` | Actor Array | Runtime | [] | Spawned building actors |

**Functions:**

---

### Function: GenerateVillage

```
Event BeginPlay
  ↓
[Delay: 0.1 seconds]  // Let spawner finish
  ↓
[SpawnChurch]
  ↓
[SpawnHouses]
  ↓
[SpawnFarms]
  ↓
[Print String: "Generated village: {LocationName}"]
```

---

### Function: SpawnHouses

```
[For Loop: 0 to HouseCount]
  ↓
  Index: i
  ↓
  [CalculateCircularPosition]
    • Index: i
    • Total: HouseCount
    • Radius: VillageDiameter / 2
    ↓
    Returns: LocalOffset (Vector)
  ↓
  [GetActorLocation] + LocalOffset = SpawnLocation
  ↓
  [Random Float in Range: 0 to 360] → RandomRotation
  ↓
  [Spawn Actor from Class]
    • Class: HouseClass
    • Location: SpawnLocation
    • Rotation: (0, 0, RandomRotation)
    ↓
    SpawnedHouse
  ↓
  [Array: Add] → SpawnedBuildings
```

---

### Function: CalculateCircularPosition

**Inputs:**
- `Index` (Integer)
- `Total` (Integer)
- `Radius` (Float) - in meters

**Outputs:**
- `LocalOffset` (Vector) - in cm

```
[Index] / [Total] = Fraction
  ↓
Fraction × 360.0 = AngleDegrees
  ↓
[DegreesToRadians] → AngleRadians
  ↓
[Cos(AngleRadians)] × Radius × 100 = X
[Sin(AngleRadians)] × Radius × 100 = Y
Z = 0
  ↓
[Make Vector(X, Y, Z)] → LocalOffset
```

---

## Blueprint 7: Debug Visualizer

### Create: BP_VerdunDebugVisualizer (Actor)

Visualizes all locations with colored spheres.

```
Event BeginPlay
  ↓
[Load Data Table: DT_VerdunLocations]
  ↓
[For Each Row]
  ↓
  [Convert GPS to World]
  ↓
  [Switch on Type]
    ├─ "village" → Color: Green
    ├─ "fort" → Color: Red
    ├─ "terrain" → Color: Blue
    └─ Default → Color: White
  ↓
  [Draw Debug Sphere]
    • Center: WorldLocation
    • Radius: 500 cm
    • Color: Color
    • Duration: 999
    • Thickness: 5
  ↓
  [Draw Debug String]
    • Text: LocationName
    • Location: WorldLocation + (0, 0, 1000)
    • Color: White
    • Duration: 999
```

---

## Usage Instructions

### Setup Process

1. **Import Data:**
   - Import `all_locations_ue5.csv` as Data Table
   - Import `villages_ue5.csv`, `forts_ue5.csv` as separate tables

2. **Create Blueprints:**
   - Create `BP_GPSConverter` (Function Library)
   - Create `S_VerdunLocation` (Structure)
   - Create `BP_VerdunLocationBase` (Actor)
   - Create `BP_VerdunVillage` (Actor, inherits Base)
   - Create `BP_VerdunFort` (Actor, inherits Base)
   - Create `BP_VerdunLocationSpawner` (Actor)

3. **Configure Actor Classes:**
   - Assign `BP_VerdunVillage` to spawner's `VillageActorClass`
   - Assign `BP_VerdunFort` to spawner's `FortActorClass`
   - Create building blueprints (houses, churches, etc.)

4. **Place in Level:**
   - Drag `BP_VerdunLocationSpawner` into level
   - Set `LocationDataTable` to `DT_VerdunLocations`
   - Enable `DebugMode` to see visualization
   - Play level

5. **Verify:**
   - Check Output Log for "Spawned X locations"
   - Debug spheres should appear at locations
   - Measure distances to verify accuracy

### Testing Checklist

- [ ] Fort Douaumont spawns ~5.6 km north, 4 km east of origin
- [ ] Fort Vaux spawns ~3.8 km north, 6.3 km east of origin
- [ ] Distance between forts is ~2.9 km
- [ ] Villages spawn at correct relative positions
- [ ] All locations have correct elevations
- [ ] Labels are visible and correct

### Troubleshooting

**Problem:** Locations spawn at origin (0,0,0)
- **Fix:** Check that ConvertGPSToWorld function is called
- Verify DataTable has valid lat/lon values

**Problem:** Locations spawn too far apart
- **Fix:** Check conversion multipliers (should be 11132000 and 7267500)

**Problem:** Villages have no buildings
- **Fix:** Ensure HouseCount > 0 in data
- Verify building classes are assigned

---

## Advanced: JSON Import

For more complex imports, use JSON with the VaRest plugin:

1. Install **VaRest** plugin
2. Use **Construct Json Request**
3. Parse JSON fields
4. Iterate through arrays
5. Call spawning functions

See `UE5_IMPORT_GUIDE.md` for detailed JSON implementation.

---

## Performance Optimization

### For Large Datasets:

1. **Async Loading:**
   ```
   [Delay Node] between spawns
   or
   [Timeline] to spread spawning over frames
   ```

2. **Level Streaming:**
   - Split locations into sub-levels
   - Stream in based on player position

3. **Instanced Static Meshes:**
   - Use Instanced Static Mesh Component
   - For repeated buildings

4. **LOD (Level of Detail):**
   - Auto LOD on all static meshes
   - Cull distant objects

---

**Next Steps:** Implement BP_VerdunLocationSpawner and test with debug visualization!
