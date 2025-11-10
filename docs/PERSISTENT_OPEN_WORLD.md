# Only The Dead - Persistent Open World Architecture

**The Dying World: Witnessing Verdun's Transformation Over 303 Days**

---

## 🌍 CORE VISION

**ONE continuous 1:1 scale map** of the Verdun battlefield where the player experiences the environmental transformation from picturesque countryside to Zone Rouge wasteland over 303 days.

**This is not 35 separate maps. This is ONE living, dying world.**

The environment itself is a character—perhaps the most important character—in the story. The player doesn't just survive the battle; they witness the death of a world.

---

## 📐 WORLD SPECIFICATIONS

### **Physical Dimensions**
- **Total Battlefield**: ~180 km² (full historical extent)
- **Playable Core Area**: 30-50 km² (main battle zones)
- **Dimensions**: 15km North-South × 12km East-West
- **Scale**: 1:1 (real-world accurate)

### **Coordinate System**
- **Source**: Real GPS coordinates (WGS84)
- **Elevation Data**: NASA SRTM 30m resolution
- **Conversion**: GPS → UE5 World Space (pre-calculated)

### **Major Landmarks** (All in ONE map)
- Fort Douaumont (49.2106°N, 5.4272°E, ~388m elevation)
- Fort Vaux (49.1989°N, 5.4494°E, ~390m)
- Bois des Caures (opening battle site)
- Mort-Homme ("Dead Man" hill)
- Fleury-devant-Douaumont (destroyed village)
- Côte 304 (Hill 304)
- Verdun city center (rear area)
- 80+ additional historically accurate locations

---

## 🏗️ TECHNICAL ARCHITECTURE

### **UE5 World Partition**

```
/Game/Maps/
└── MAP_Verdun_Persistent.umap        # SINGLE persistent map
    ├── WorldPartition/                # Streaming grid
    │   ├── Grid_NorthSector/          # 2km × 2km cells
    │   ├── Grid_CenterSector/
    │   ├── Grid_SouthSector/
    │   └── ...
    ├── Landscape/                     # Real terrain data
    │   ├── Heightmap_Verdun_16bit.raw # 30m resolution SRTM
    │   ├── Materials/
    │   └── Foliage/
    └── PersistentLevel/               # Always-loaded core
        ├── GameMode
        ├── GameState
        └── CoreSystems
```

### **Streaming Strategy**

**Load/Unload Radius**:
- **Always Loaded**: 500m around player
- **High Detail**: 1km radius (full assets)
- **Medium Detail**: 2km radius (LOD1-2)
- **Low Detail**: 5km radius (LOD3-4)
- **Unloaded**: Beyond 5km

**Performance Targets**:
- **PC**: 60 FPS @ 1440p
- **Console**: 60 FPS @ 1440p (Performance) / 30 FPS @ 4K (Quality)

**UE5 Features Leveraged**:
- **Lumen**: Dynamic global illumination (no baked lightmaps needed)
- **Nanite**: Virtualized geometry (billions of triangles)
- **World Partition**: Automatic spatial streaming
- **One File Per Actor**: Parallel team development

---

## 🌱 ENVIRONMENTAL TRANSFORMATION SYSTEM

### **The Core Mechanic**

The world transforms in **real-time** as the player progresses through 303 days:

**Day 1 (Feb 21, 1916)**: Verdun countryside
- Green forests (Bois des Caures: 5,000+ trees)
- Intact villages (Fleury: 85 buildings)
- Farmland, roads, churches
- Birds, wildlife

**Day 100 (May 31, 1916)**: Deteriorating battlefield
- Shattered tree stumps (50% trees gone)
- Ruined buildings (80% damaged)
- Craters everywhere (10,000+ created)
- Mud, smoke, no wildlife

**Day 303 (Dec 18, 1916)**: Zone Rouge wasteland
- No trees remain (100% destroyed)
- No buildings intact (100% rubble)
- Overlapping craters (50,000+ created)
- Poisoned soil, uninhabitable
- The earth itself is dead

### **EnvironmentDegradationSystem**

**Tracks per-region transformation:**

```cpp
struct FEnvironmentRegion {
    EEnvironmentState CurrentState;  // Pristine → Zone Rouge
    float DegradationLevel;          // 0.0 → 1.0
    int32 ShellImpactsReceived;      // Cumulative
    int32 TreesRemaining;            // Decreases over time
    int32 BuildingsRemaining;        // Decreases over time
    int32 CratersCreated;            // Increases over time
    bool bIsActiveWarZone;           // Accelerated degradation
};
```

**Environmental States**:
1. **Pristine** (Tutorial only) - Pre-war beauty
2. **Early War** (Days 1-30) - Damage beginning, still recognizable
3. **Deteriorating** (Days 31-100) - Severe damage, many areas destroyed
4. **Devastated** (Days 101-200) - Moonscape forming, little life remains
5. **Apocalyptic** (Days 201-303) - Zone Rouge, total devastation
6. **Zone Rouge** (Post-war) - Legally uninhabitable, poisoned forever

### **Degradation Drivers**

**Artillery impacts** (primary driver):
- Each shell impact increases regional degradation
- Larger shells = more degradation (420mm >>> 75mm)
- Bombardments cause mass degradation

**Historical events** (key milestones):
- Feb 21, 1916: Bois des Caures forest obliterated (9-hour barrage)
- Feb 25, 1916: Fort Douaumont area devastated
- June 23, 1916: Fleury village completely destroyed
- Oct 24, 1916: Douaumont surroundings cratered beyond recognition

**Cumulative effects**:
- 303 days of constant shelling
- ~60 million shells fired (historical)
- Every square meter hit multiple times
- The transformation is INEVITABLE

---

## 🎮 GAMEPLAY INTEGRATION

### **Missions as Zones, Not Maps**

**OLD Design** (Rejected):
```
Mission 01 → Load MAP_M01_TheGunsBegin.umap
Mission 02 → Load MAP_M02_TheFirstNight.umap
[Loading screen between each mission]
```

**NEW Design** (Persistent World):
```
Mission 01 → Trigger zone at Bois des Caures (real location)
Mission 02 → Player can walk to next mission area (no loading)
Mission 03 → Trigger when player enters Fort Douaumont zone
[NO loading screens - continuous experience]
```

### **Mission System Updates**

```cpp
// OLD: Load separate map
void StartMission(FString MissionID) {
    UGameplayStatics::OpenLevel(World, "MAP_M01");
}

// NEW: Activate zone in persistent world
void StartMission(FString MissionID) {
    FVector MissionLocation = GetMissionLocation(MissionID);
    ActivateMissionTriggerVolume(MissionLocation);
    // Player walks to location
    // Mission starts when they enter zone
}
```

### **Player Freedom**

**During missions**:
- Objectives at specific locations in world
- Player chooses how to navigate there (trenches, no-man's-land, tunnels)
- Can detour to scavenge, help wounded, etc.

**Between missions (Rotation phases)**:
- Front Line rotation: Confined to trench system
- Support rotation: Slightly larger movement area
- Rest period: Can explore rear areas (Verdun city, field hospitals)

**Forbidden zones**:
- Enemy-controlled territory (instant death if detected)
- Zones under heavy bombardment (very high risk)
- Chemically contaminated areas (gas masks required)

---

## 🗺️ WORLD REGIONS

### **20 Major Regions** (Fully detailed)

Each region has its own transformation timeline:

1. **Bois des Caures** (Opening battle, Feb 21-25)
   - Day 1: Dense forest
   - Day 5: Shattered stumps
   - Day 30: Wasteland

2. **Fort Douaumont** (Feb 25 capture, Oct 24 recapture)
   - Day 1: Intact fortification
   - Day 100: Damaged, shell-pocked
   - Day 303: Crater-surrounded ruin

3. **Fleury-devant-Douaumont** (Changed hands 16 times)
   - Day 1: Picturesque village
   - Day 50: Rubble
   - Day 100: Unrecognizable
   - Historical: Never rebuilt, forever destroyed

4. **Mort-Homme** (Dead Man's Hill - prophetic name)
   - Active fighting March-June
   - Named "Dead Man" before the war
   - Becomes literal dead zone

5. **Fort Vaux** (7-day siege, June 1-7)
   - Interior of fort is explorable
   - Water shortage (thirst gameplay)
   - Honorable surrender

[+ 15 more detailed regions]

### **Region-Specific Transformation**

Each region tracks:
- **Shell impacts received** (historical data when available)
- **Key dates** (when major damage occurred)
- **Trees/buildings destroyed** (visual representation)
- **Crater density** (100m² grid tracking)
- **Atmospheric conditions** (smoke, gas, ash)

---

## 📊 PERFORMANCE OPTIMIZATION

### **World Partition Streaming**

**Cell Size**: 2km × 2km (optimal for 30-50 km² world)
- **Total cells**: ~12-25 cells
- **Loaded simultaneously**: 2-4 cells (4-16 km² in memory)
- **Transition time**: < 0.5 seconds (imperceptible)

### **LOD Strategy**

**Static meshes** (buildings, props):
- LOD0: 0-50m (full detail)
- LOD1: 50-200m (75% tris)
- LOD2: 200-500m (40% tris)
- LOD3: 500-1000m (10% tris)
- LOD4: 1000m+ (3% tris, silhouette only)

**Landscape**:
- LOD0: 0-100m (1m resolution)
- LOD1: 100-500m (2m resolution)
- LOD2: 500-2km (10m resolution)
- LOD3: 2km+ (30m resolution, matches SRTM)

**Foliage** (trees, grass):
- Nanite for hero trees (no LODs needed)
- Hierarchical Instanced Static Meshes for grass
- Cull distance: 500m for grass, 2km for trees
- As trees are destroyed, rendering cost decreases (emergent optimization!)

### **Lumen & Nanite Benefits**

**Lumen** (Dynamic GI):
- No baked lightmaps needed (would be 50GB+ for 50 km²)
- Lighting updates in real-time as environment changes
- Smoke, fog, atmospheric effects are dynamic

**Nanite** (Virtualized Geometry):
- Billions of triangles visible simultaneously
- High-detail craters, rubble, destroyed buildings
- No manual LOD creation needed for most assets

### **Memory Budget**

**Target**: 12GB VRAM (RTX 4070 / PS5 / Xbox Series X)
- Landscape: 2GB (streaming)
- Meshes: 4GB (Nanite virtual texture)
- Textures: 4GB (virtual textures)
- Audio: 1GB (streaming)
- Systems: 1GB (gameplay)

---

## 🎨 ASSET REQUIREMENTS

### **Terrain**

**Source**: Real SRTM data (already available in `verdun_anthology/geographic_data/`)

```bash
# Generate UE5 landscape
cd verdun_anthology/geographic_data
python fetch_elevation_data.py    # Download NASA terrain (50MB)
python export_for_ue5.py           # Convert to 16-bit heightmap
# Import into UE5 World Partition landscape
```

**Material Layers**:
- Mud (dominant - 60% of surface by day 100)
- Grass (decreases over time - 80% day 1 → 0% day 200)
- Rock/limestone (increases as soil is destroyed)
- Ash/char (appears in heavily bombarded zones)
- Blood/contamination (subtle, realistic)

### **Foliage Transformation**

**Dynamic foliage density**:
```cpp
// Update foliage as trees are destroyed
void UpdateFoliage(FString RegionID) {
    FEnvironmentRegion Region = Regions[RegionID];
    float TreeDensity = (float)Region.TreesRemaining / Region.InitialTreeCount;

    // Update UE5 foliage system
    SetFoliageDensity(RegionID, TreeDensity);

    // Replace live trees with dead variants
    if (TreeDensity < 0.5f) {
        SwapToDeadTrees(RegionID, (1.0f - TreeDensity) * 100%);
    }
}
```

**Tree states**:
1. Living (days 1-30 in some areas)
2. Damaged (shell-scarred bark, broken branches)
3. Shattered (trunk broken, no canopy)
4. Stump (only base remains)
5. Gone (crater where tree was)

### **Buildings**

**Degradation states** (5 levels per building):
1. Intact (pre-war, tutorial)
2. Damaged (walls cracked, roof damaged)
3. Severely damaged (walls collapsed, no roof)
4. Rubble (foundation + debris pile)
5. Crater (building completely obliterated)

**Swap meshes automatically**:
```cpp
void DamageBuilding(AActor* Building, float Damage) {
    int32 CurrentState = Building->GetDamageState();
    if (Damage > DamageThreshold) {
        Building->SetStaticMesh(DamagedMeshes[CurrentState + 1]);
        CurrentState++;
    }
}
```

### **Craters**

**Procedural crater generation**:
- Store crater data (location, size, depth)
- Use landscape sculpting at runtime
- Or spawn crater meshes (faster, less flexible)

**Crater density visualization**:
- Day 1: 0 craters
- Day 100: 10,000 craters (average 1 per 3,000 m²)
- Day 303: 50,000+ craters (overlapping, merged)

---

## 🚧 DEVELOPMENT PHASING

### **Phase 1: Vertical Slice** (Months 1-6)

**Goal**: Prove the concept with 2km × 2km playable area

**Scope**:
- Bois des Caures sector (opening battle)
- Real terrain imported from SRTM data
- Environmental transformation (Day 1 → Day 30)
- Mission 01 playable in persistent world
- 500 trees → 0 trees transformation visible
- Basic crater accumulation

**Success Criteria**:
- Player can walk 2km without loading
- Visible transformation over 30 in-game days
- 60 FPS maintained

### **Phase 2: Core Battlefield** (Months 7-12)

**Expand to 10km × 10km**:
- Add Fort Douaumont, Fort Vaux, Fleury
- 5 missions playable
- Full 303-day transformation timeline
- 20,000 trees → 0 transformation
- Weather system integrated

### **Phase 3: Full Battlefield** (Months 13-24)

**Complete 30-50 km² implementation**:
- All 20 major regions
- All 35 missions
- Complete historical accuracy
- Full asset polish

### **Phase 4: Optimization** (Months 25-30)

**Performance tuning**:
- Achieve 60 FPS on target hardware
- Reduce load times
- Optimize streaming
- Memory budget compliance

---

## 📈 TECHNICAL SPECIFICATIONS

### **Landscape Settings**

```
Map Size: 50km × 50km (maximum, 30km used)
Components: 64 × 64
Quads per Component: 127 × 127
Total Quads: ~260 million
Heightmap Resolution: 16,257 × 16,257 (16-bit)
Streaming Cell Size: 2048m × 2048m
Total Streaming Cells: 25
```

### **World Partition Settings**

```ini
[/Script/Engine.WorldPartitionRuntimeSpatialHash]
CellSize=204800  // 2048m cells
LoadingRange=512000  // 5120m (5km)
RuntimeHashGridName=WorldPartition2D
HLODLayer=HLOD0
```

### **Nanite Settings**

```ini
[/Script/Engine.RendererSettings]
r.Nanite=1
r.Nanite.MaxPixelsPerEdge=1
r.Nanite.MinPixelsPerEdgeHW=32
r.Shadow.Virtual.Enable=1  // Virtual shadow maps
```

---

## 🎯 PLAYER EXPERIENCE

### **The Journey**

**Day 1**: You arrive at Verdun. The woods are green. Villages stand. It's almost beautiful. You don't yet know what's coming.

**Day 30**: The trees are gone. The villages are rubble. Mud is everywhere. You begin to understand.

**Day 100**: A moonscape. Craters everywhere. Nothing green remains. You wonder if the world will ever heal.

**Day 200**: Zone Rouge. The earth is poisoned. This will never be farmland again. Never be forest. Never be home.

**Day 303**: You've survived. You've won. But the world has lost. Forever.

**Post-game**: You can return. Walk the wasteland. See what you endured. The transformation is permanent in your save file.

### **Emotional Impact**

The persistent world creates emotional connection:
- **Attachment**: You know this land. You've walked every meter.
- **Loss**: You watched it die. Tree by tree. Building by building.
- **Memory**: You remember what it was. The before photos.
- **Witness**: You are the last person who saw Verdun alive.

---

## 🛠️ IMPLEMENTATION GUIDE

### **Step 1: Import Terrain**

```bash
# In verdun_anthology/geographic_data/
python fetch_elevation_data.py
python export_for_ue5.py

# Follow UE5_Import/README.txt instructions:
# 1. Create World Partition map
# 2. Import 16-bit heightmap
# 3. Set landscape scale: X=100, Y=100, Z=50
```

### **Step 2: Set Up World Partition**

```cpp
// In UE5 Editor:
1. Window → World Partition
2. Enable World Partition for map
3. Set cell size: 2048m
4. Set loading range: 5120m
5. Generate HLOD layers (LOD0-3)
```

### **Step 3: Configure Game Mode**

```cpp
// BP_OnlyTheDeadGameMode.cpp
void AOnlyTheDeadGameMode::InitGame(...) {
    // Initialize environment degradation system
    EnvironmentSystem = NewObject<UEnvironmentDegradationSystem>();
    EnvironmentSystem->InitializeRegions();

    // Link to mission system
    MissionSystem->SetPersistentWorld(true);
}
```

### **Step 4: Place Key Locations**

Use GPS coordinates from `verdun_anthology/geographic_data/locations_database.csv`:
- Convert lat/lon to UE5 coordinates
- Place mission trigger volumes
- Place historical markers
- Set up regional boundaries

### **Step 5: Implement Degradation**

```cpp
// Environmental transformation tick
void UEnvironmentDegradationSystem::TickComponent(...) {
    // Update regions based on current day
    UpdateDegradation(MissionSystem->GetCurrentDate());

    // Apply visual changes
    for (auto& Region : Regions) {
        UpdateRegionVisuals(Region);
    }
}
```

---

## ✅ VALIDATION CHECKLIST

**Persistent World Requirements**:
- [ ] Single map file (MAP_Verdun_Persistent.umap)
- [ ] World Partition enabled
- [ ] Real terrain data imported (SRTM 30m)
- [ ] 80+ GPS locations placed accurately
- [ ] 20 regions defined with boundaries
- [ ] Environmental degradation system active
- [ ] No loading screens during gameplay
- [ ] Player can walk entire battlefield
- [ ] Transformation visible over 303 days
- [ ] Performance: 60 FPS maintained

**Historical Accuracy**:
- [ ] All locations match real GPS coordinates
- [ ] Terrain elevation matches SRTM data (±5m)
- [ ] Degradation timeline matches historical records
- [ ] Key events trigger on correct dates
- [ ] Final state = Zone Rouge (uninhabitable)

---

## 📚 REFERENCE MATERIALS

**Geographic Data**: `verdun_anthology/geographic_data/`
- GEOGRAPHIC_DATA_OVERVIEW.md (all coordinates)
- locations_database.csv (80+ locations)
- terrain_parameters.json (materials, weather)

**Python Tools**:
- `fetch_elevation_data.py` (download NASA SRTM)
- `export_for_ue5.py` (convert to heightmap)

**Historical Degradation**:
- verdun_anthology/timeline/ (303-day timeline)
- Photo references (before/after Verdun)

---

**This is the vision: ONE world. ONE journey. 303 days. Watch it die.**

*"Ils ne passeront pas."*
