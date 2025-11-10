# Unreal Engine 5 Import Guide - Verdun Positional Data

## Table of Contents
1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Coordinate System Setup](#coordinate-system-setup)
4. [Method 1: Blueprint JSON Import](#method-1-blueprint-json-import)
5. [Method 2: C++ JSON Import](#method-2-c-json-import)
6. [Method 3: CSV Import via Data Table](#method-3-csv-import-via-data-table)
7. [Spawning Actors at Locations](#spawning-actors-at-locations)
8. [Testing and Validation](#testing-and-validation)

## Overview

This guide shows you how to import the Verdun positional data into Unreal Engine 5 and spawn actors at historically accurate locations.

### What You'll Accomplish
- Import GPS coordinate data into UE5
- Convert latitude/longitude to UE5 world coordinates
- Spawn villages, forts, roads, and terrain features at precise locations
- Validate placement accuracy

## Prerequisites

- Unreal Engine 5.0 or later
- Basic knowledge of Blueprints or C++
- The positional data files from this repository
- A Verdun terrain/landscape already created in UE5

## Coordinate System Setup

### Understanding the Coordinate Systems

**Source Data (GPS)**
- System: WGS84
- Format: Decimal Degrees
- Latitude: 49.1667°N to 49.28°N (Verdun area)
- Longitude: 5.16°E to 5.47°E (Verdun area)

**UE5 World Space**
- System: Cartesian (X, Y, Z)
- Units: Centimeters
- X-axis: Forward (North)
- Y-axis: Right (East)
- Z-axis: Up (Elevation)

### Recommended World Origin

Use Verdun city center as your origin (0, 0, 0):
```
Origin Latitude:  49.1667°N
Origin Longitude: 5.3833°E
Origin Elevation: 200m
```

### Coordinate Conversion Formula

```
For each location:
  ΔLat = Location_Latitude - Origin_Latitude
  ΔLon = Location_Longitude - Origin_Longitude

  X (North) = ΔLat × 111,320 meters × 100 (convert to cm)
  Y (East)  = ΔLon × 111,320 × cos(Origin_Latitude) × 100
  Z (Up)    = (Location_Elevation - Origin_Elevation) × 100

Where:
  111,320 = meters per degree latitude (approximately)
  cos(Origin_Latitude) = correction for longitude at this latitude
  cos(49.1667°) ≈ 0.6527
```

**Simplified UE5 Conversion**
```
X = (Lat - 49.1667) × 11,132,000 cm
Y = (Lon - 5.3833) × 7,267,000 cm
Z = (Elev - 200) × 100 cm
```

## Method 1: Blueprint JSON Import

### Step 1: Enable JSON Plugin
1. Edit → Plugins
2. Search for "JSON"
3. Enable "JSON Blueprint Utilities" plugin
4. Restart UE5

### Step 2: Create Import Blueprint

1. Create new Blueprint Class → Actor
2. Name it "BP_VerdunDataImporter"
3. Open the Blueprint

### Step 3: Add JSON Import Logic

**Add these variables:**
- `JSONFilePath` (String) - Path to verdun_master_locations.json
- `OriginLat` (Float) - 49.1667
- `OriginLon` (Float) - 5.3833
- `OriginElev` (Float) - 200.0

**Add this Blueprint logic:**

```
Event BeginPlay
  ├─ Load JSON String from File (JSONFilePath)
  ├─ JSON String to JSON Object
  └─ Branch on Success
      ├─ TRUE:
      │   ├─ Get JSON Field (destroyed_villages)
      │   └─ For Each Loop
      │       ├─ Get Latitude from JSON object
      │       ├─ Get Longitude from JSON object
      │       ├─ Get Elevation from JSON object
      │       ├─ Call ConvertGPSToWorld function
      │       └─ Spawn Actor at converted location
      └─ FALSE:
          └─ Print String "Failed to load JSON"
```

### Step 4: Create Conversion Function

Create a Blueprint Function called "ConvertGPSToWorld":

**Inputs:**
- Latitude (Float)
- Longitude (Float)
- Elevation (Float)

**Output:**
- WorldLocation (Vector)

**Logic:**
```
1. DeltaLat = Latitude - OriginLat
2. DeltaLon = Longitude - OriginLon
3. DeltaElev = Elevation - OriginElev

4. X = DeltaLat × 11132000.0
5. Y = DeltaLon × 7267000.0
6. Z = DeltaElev × 100.0

7. Return Make Vector(X, Y, Z)
```

### Step 5: Spawn Actors

For each location, spawn appropriate actors:

```
Spawn Actor from Class
  ├─ Class: BP_Village (or appropriate actor)
  ├─ Location: Converted World Location
  ├─ Rotation: (0, 0, 0)
  └─ Optional: Set custom properties from JSON
```

## Method 2: C++ JSON Import

### Step 1: Add JSON Module to Build.cs

```cpp
// YourProject.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "Json",           // Add this
    "JsonUtilities"   // Add this
});
```

### Step 2: Create Data Importer Class

Create `VerdunDataImporter.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VerdunDataImporter.generated.h"

USTRUCT(BlueprintType)
struct FVerdunLocation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString ID;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Type;

    UPROPERTY(BlueprintReadWrite)
    float Latitude;

    UPROPERTY(BlueprintReadWrite)
    float Longitude;

    UPROPERTY(BlueprintReadWrite)
    float Elevation;
};

UCLASS()
class YOURPROJECT_API AVerdunDataImporter : public AActor
{
    GENERATED_BODY()

public:
    AVerdunDataImporter();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
    FString JSONFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
    float OriginLatitude = 49.1667f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
    float OriginLongitude = 5.3833f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
    float OriginElevation = 200.0f;

    UFUNCTION(BlueprintCallable, Category = "Import")
    bool LoadVerdunData();

    UFUNCTION(BlueprintCallable, Category = "Conversion")
    FVector ConvertGPSToWorld(float Latitude, float Longitude, float Elevation);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnLocationActors(TArray<FVerdunLocation> Locations);

private:
    TArray<FVerdunLocation> ParsedLocations;
};
```

### Step 3: Implement Data Importer

Create `VerdunDataImporter.cpp`:

```cpp
#include "VerdunDataImporter.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"

AVerdunDataImporter::AVerdunDataImporter()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AVerdunDataImporter::BeginPlay()
{
    Super::BeginPlay();

    if (LoadVerdunData())
    {
        UE_LOG(LogTemp, Log, TEXT("Verdun data loaded successfully!"));
        SpawnLocationActors(ParsedLocations);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Verdun data!"));
    }
}

bool AVerdunDataImporter::LoadVerdunData()
{
    FString JSONString;

    // Load JSON file
    if (!FFileHelper::LoadFileToString(JSONString, *JSONFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Could not load file: %s"), *JSONFilePath);
        return false;
    }

    // Parse JSON
    TSharedPtr<FJsonObject> JSONObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

    if (!FJsonSerializer::Deserialize(Reader, JSONObject) || !JSONObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
        return false;
    }

    // Parse destroyed villages
    const TArray<TSharedPtr<FJsonValue>>* VillagesArray;
    if (JSONObject->TryGetArrayField(TEXT("destroyed_villages"), VillagesArray))
    {
        for (const TSharedPtr<FJsonValue>& VillageValue : *VillagesArray)
        {
            TSharedPtr<FJsonObject> VillageObj = VillageValue->AsObject();

            FVerdunLocation Location;
            Location.ID = VillageObj->GetStringField(TEXT("id"));
            Location.Name = VillageObj->GetStringField(TEXT("name"));
            Location.Type = VillageObj->GetStringField(TEXT("type"));

            // Get coordinates sub-object
            TSharedPtr<FJsonObject> CoordObj = VillageObj->GetObjectField(TEXT("coordinates"));
            Location.Latitude = CoordObj->GetNumberField(TEXT("latitude"));
            Location.Longitude = CoordObj->GetNumberField(TEXT("longitude"));
            Location.Elevation = CoordObj->GetNumberField(TEXT("elevation_meters"));

            ParsedLocations.Add(Location);
        }
    }

    // TODO: Parse forts, roads, terrain features similarly

    return ParsedLocations.Num() > 0;
}

FVector AVerdunDataImporter::ConvertGPSToWorld(float Latitude, float Longitude, float Elevation)
{
    // Calculate deltas
    float DeltaLat = Latitude - OriginLatitude;
    float DeltaLon = Longitude - OriginLongitude;
    float DeltaElev = Elevation - OriginElevation;

    // Convert to UE5 coordinates (centimeters)
    // X = North, Y = East, Z = Up
    float X = DeltaLat * 11132000.0f;  // cm per degree latitude
    float Y = DeltaLon * 7267000.0f;   // cm per degree longitude (at ~49°N)
    float Z = DeltaElev * 100.0f;      // meters to cm

    return FVector(X, Y, Z);
}

void AVerdunDataImporter::SpawnLocationActors(TArray<FVerdunLocation> Locations)
{
    for (const FVerdunLocation& Location : Locations)
    {
        FVector WorldLocation = ConvertGPSToWorld(
            Location.Latitude,
            Location.Longitude,
            Location.Elevation
        );

        // TODO: Determine actor class based on Location.Type
        // Example: Spawn village actor at WorldLocation

        UE_LOG(LogTemp, Log, TEXT("Location %s at World: X=%f, Y=%f, Z=%f"),
            *Location.Name, WorldLocation.X, WorldLocation.Y, WorldLocation.Z);
    }
}
```

## Method 3: CSV Import via Data Table

This is the easiest method for quick prototyping!

### Step 1: Create Data Table Structure

1. Create new Blueprint → Structure
2. Name it "S_VerdunLocation"
3. Add these variables:
   - ID (String)
   - Name (String)
   - Type (String)
   - Latitude (Float)
   - Longitude (Float)
   - Elevation (Float)

### Step 2: Import CSV as Data Table

1. Right-click in Content Browser → Import
2. Select `villages.csv` (or any other CSV file)
3. Choose "Data Table"
4. Select "S_VerdunLocation" as Row Structure
5. Name it "DT_VerdunVillages"

### Step 3: Read Data Table in Blueprint

```
Event BeginPlay
  ├─ Get Data Table Row Names (DT_VerdunVillages)
  └─ For Each Name
      ├─ Get Data Table Row (DT_VerdunVillages, Name)
      ├─ Break S_VerdunLocation structure
      ├─ Call ConvertGPSToWorld
      └─ Spawn Actor at converted location
```

## Spawning Actors at Locations

### Creating Actor Templates

For each location type, create appropriate actors:

1. **BP_Village**
   - Static Mesh: Buildings, church, farms
   - Instance Count: Based on JSON data
   - Layout: Circular or linear village pattern

2. **BP_Fort**
   - Static Mesh: Fort walls, artillery positions
   - Scale: Based on footprint data

3. **BP_Road**
   - Spline Component: Connect waypoints
   - Spline Mesh: Road surface

4. **BP_TerrainFeature**
   - Landscape brush or foliage
   - Modify terrain elevation

### Example: Spawning a Village

```
Function: SpawnVillage
Inputs:
  - Location (Vector)
  - VillageName (String)
  - HouseCount (Integer)
  - DiameterMeters (Float)

Logic:
  1. Spawn BP_Village at Location
  2. Set village name
  3. For each house (HouseCount):
     a. Calculate random position in circle (DiameterMeters radius)
     b. Spawn BP_House at offset position
     c. Random rotation
  4. Spawn BP_Church at village center
  5. Spawn BP_Farm actors around perimeter
```

## Testing and Validation

### Visual Validation

1. **Check Relative Distances**
   - Fort Douaumont to Fort Vaux should be ~6km
   - Verify with UE5 measuring tools

2. **Check Elevations**
   - Fort Douaumont (392m) should be highest
   - Mort-Homme (295m) should be prominent hill

3. **Check Road Alignment**
   - La Voie Sacrée should run roughly north-south
   - Should connect Bar-le-Duc to Verdun

### Coordinate Validation Tool

Create a Blueprint function to verify conversions:

```
Function: ValidateConversion
Input: Location Name (String)

Logic:
  1. Get GPS coordinates for location
  2. Convert to world coordinates
  3. Print: "Location: [Name]"
  4. Print: "GPS: [Lat], [Lon], [Elev]"
  5. Print: "World: [X], [Y], [Z]"
  6. Spawn debug sphere at location
```

### Known Distances for Verification

Use these to validate your coordinate conversion:

| From | To | Distance (km) |
|------|-----|---------------|
| Verdun | Fort Douaumont | ~8 |
| Fort Douaumont | Fort Vaux | ~6 |
| Verdun | Bar-le-Duc | ~56 (via Voie Sacrée) |
| Fleury | Fort Douaumont | ~2 |

## Troubleshooting

### Problem: Actors spawn too close together
**Solution:** Check conversion multipliers - you may be missing a scale factor

### Problem: Actors spawn in wrong direction
**Solution:** Verify X=North, Y=East axis convention matches your project

### Problem: Elevations are wrong
**Solution:** Ensure origin elevation (200m) is subtracted before scaling

### Problem: JSON won't parse
**Solution:** Validate JSON syntax at jsonlint.com

### Problem: Villages are too small/large
**Solution:** Adjust building scale and spacing parameters

## Advanced: Georeferencing

For perfect alignment with real-world satellite imagery:

1. Use UE5 Georeferencing plugin
2. Set Geographic CRS to WGS84 (EPSG:4326)
3. Set origin to Verdun coordinates
4. Import locations directly in lat/lon
5. UE5 handles conversion automatically

## Performance Optimization

- Use Instance Static Meshes for repeated buildings
- LOD (Level of Detail) for distant villages
- Cull actors outside camera frustum
- Stream in villages as player approaches

## Next Steps

1. Import your terrain/landscape
2. Run the importer to spawn location markers
3. Replace markers with detailed 3D assets
4. Add historical details (trenches, bunkers, etc.)
5. Populate forests with trees
6. Add roads as spline actors

---

**Questions?** Check the main README.md or open an issue in the repository.
