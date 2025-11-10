# Verdun Pre-Bombardment Positional Data for UE5

## Overview

This directory contains historically accurate positional data for the Verdun battlefield **BEFORE** the February 1916 bombardment. All data is compiled from historical maps, IGN (Institut Géographique National) sources, battlefield archives, and modern GPS surveys of memorial sites.

## Purpose

This data allows Unreal Engine 5 to precisely place:
- **Villages** (9 destroyed villages that were never rebuilt)
- **Fortifications** (Fort Douaumont, Fort Vaux, etc.)
- **Roads** (La Voie Sacrée, local roads)
- **Terrain Features** (hills, forests, the Meuse River)
- **Trenches** (preliminary defensive positions)
- **Buildings** (estimated counts per village)

## Files Included

### Master Data
- **`verdun_master_locations.json`** - Complete dataset with all metadata, UE5 notes, and historical context

### CSV Files (for easy spreadsheet import)
- **`villages.csv`** - All 9 destroyed villages with building counts
- **`forts.csv`** - Major fortifications
- **`terrain_features.csv`** - Hills, forests, water features
- **`roads.csv`** - Road network including La Voie Sacrée

### Documentation
- **`README.md`** - This file
- **`UE5_IMPORT_GUIDE.md`** - Detailed instructions for importing into Unreal Engine 5
- **`COORDINATE_CONVERSION.md`** - Technical details on coordinate system conversion

## Coordinate System

All coordinates use **WGS84** (World Geodetic System 1984) in **decimal degrees**:
- **Latitude**: Positive values are North
- **Longitude**: Positive values are East
- **Elevation**: Meters above sea level

### Recommended World Origin for UE5
Use Verdun city center as your world origin:
- **Latitude**: 49.1667°N
- **Longitude**: 5.3833°E
- **Elevation**: 200m

## Data Sources

1. **IGN Historical Maps** (1:75,000 and 1:25,000 scale)
2. **Battlefield GPS Surveys** of memorial sites and destroyed villages
3. **Historical Archives** - pre-1916 village population and structure data
4. **Contemporary Military Maps** from French and German sources
5. **Modern Academic Research** on the Battle of Verdun

## Historical Accuracy Notes

### Villages
All 9 villages were:
- Active farming communities in 1914
- Completely destroyed during 1916
- Declared "Mort pour la France" (Died for France)
- Never rebuilt - official population: 0
- Now located in the "Zone Rouge" (Red Zone)

Building counts are estimated based on 1914 population data using typical French village ratios:
- ~6-7 people per household
- 1 church per village
- 1 farm per 25-30 villagers in rural areas
- Municipal buildings scaled to population

### Fortifications
Fort data includes:
- Actual GPS coordinates from site surveys
- Historical elevation data
- Pre-war garrison status
- Estimated footprints from historical plans

### Terrain
- Elevation data from IGN sources
- Forest coverage represents pre-bombardment conditions
- Hills retain their strategic military designations

## Quick Start for UE5

1. **Set World Origin**: Use Verdun city center (49.1667°N, 5.3833°E) as coordinates (0, 0, 0)
2. **Import JSON**: Use the master JSON file with a Blueprint or C++ JSON parser
3. **Convert Coordinates**: See `COORDINATE_CONVERSION.md` for formulas
4. **Place Actors**: Spawn appropriate actors at converted coordinates
5. **Scale Check**: 1 UE5 unit = 1 cm (standard UE5 scale)

## Data Categories

### Forts (2 major)
- Fort Douaumont - Largest fort in the ring
- Fort Vaux - Eastern defense point

### Destroyed Villages (9 total)
1. Beaumont-en-Verdunois
2. Bezonvaux
3. Cumières-le-Mort-Homme
4. Douaumont (village)
5. Fleury-devant-Douaumont
6. Haumont-près-Samogneux
7. Louvemont-Côte-du-Poivre
8. Ornes (largest, 632 people)
9. Vaux-devant-Damloup

### Major Roads
- **La Voie Sacrée** (The Sacred Way) - 56km supply route from Bar-le-Duc to Verdun

### Terrain Features
- Le Mort-Homme (Dead Man's Hill) - 295m elevation
- Côte 304 (Hill 304) - 304m elevation
- Meuse River - Divides battlefield
- Bois d'Haumont (Haumont Woods)
- Bois des Caures (Caures Woods)

### Cities
- Verdun - Fortified city, ~18,000 population (1914)

## Usage Rights

This data is compiled from public historical sources and GPS coordinates of public memorial sites. It is intended for:
- Historical education
- Video game development
- Virtual reality experiences
- Academic research
- Documentary production

## Accuracy Disclaimer

While every effort has been made to ensure historical accuracy:
- GPS coordinates for destroyed villages are approximate (sites are now empty fields)
- Building counts are estimates based on population data
- Trench positions evolved rapidly during the battle
- Some elevation data is interpolated
- Pre-war terrain has been significantly altered by shelling

For maximum accuracy, consult:
- French National Archives (Archives Nationales)
- Service Historique de la Défense
- Mémorial de Verdun museum
- IGN historical map collections

## Contributing

If you have more accurate historical data, please contribute:
- Updated coordinates from archival maps
- Building layouts from pre-war photographs
- Trench system maps from military archives
- Vegetation data from forestry records

## Version History

- **v1.0** (2025-11-10) - Initial compilation
  - 9 destroyed villages
  - 2 major forts
  - La Voie Sacrée road network
  - 5 terrain features
  - Complete metadata and UE5 integration notes

## Contact & Support

For questions about this data or to report errors, please open an issue in the repository.

## Related Files

- See `UE5_IMPORT_GUIDE.md` for step-by-step UE5 integration instructions
- See `COORDINATE_CONVERSION.md` for technical coordinate transformation details
- See `verdun_master_locations.json` for the complete dataset with all metadata

---

*"Ils ne passeront pas!" - They shall not pass!*
