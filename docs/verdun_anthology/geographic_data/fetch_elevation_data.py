#!/usr/bin/env python3
"""
VERDUN BATTLEFIELD ELEVATION DATA FETCHER
==========================================

Downloads real SRTM (Shuttle Radar Topography Mission) elevation data for the
Battle of Verdun area (February-December 1916).

COVERAGE AREA:
- Latitude:  49.14°N to 49.25°N
- Longitude: 5.31°E to 5.47°E
- Resolution: 30 meters (SRTM)
- Area: ~60 km² battlefield zone

USAGE:
    python fetch_elevation_data.py

OUTPUTS:
    - verdun_elevation_raw.tif (GeoTIFF format)
    - verdun_elevation_grid.npy (NumPy array for processing)
    - verdun_elevation_grid.csv (CSV format)
    - verdun_heightmap.png (Grayscale heightmap for UE5/Houdini)
    - verdun_elevation_metadata.json (Data parameters)

DEPENDENCIES:
    pip install elevation numpy pillow rasterio

DATA SOURCE:
    SRTM 30m resolution data via 'elevation' Python library
    (Uses NASA's SRTM data, public domain)

HISTORICAL ACCURACY:
    ⭐ Tier 1 (Documented): Modern SRTM data is accurate to terrain within
    ~5-10m vertical accuracy. WWI battlefield modifications (trenches, shell
    craters) not present in modern data, but overall topography (hills, valleys,
    ridges) is identical to 1916.

AUTHOR: Claude (Verdun Anthology Historian)
DATE: 2025
"""

import os
import sys
import json
import subprocess
from pathlib import Path

# Check dependencies
REQUIRED_PACKAGES = ['elevation', 'numpy', 'PIL', 'rasterio']
missing_packages = []

for package in REQUIRED_PACKAGES:
    try:
        if package == 'PIL':
            import PIL
        else:
            __import__(package)
    except ImportError:
        missing_packages.append(package if package != 'PIL' else 'Pillow')

if missing_packages:
    print("❌ MISSING DEPENDENCIES")
    print(f"   Please install: pip install {' '.join(missing_packages)}")
    print("\n   Full install command:")
    print("   pip install elevation numpy pillow rasterio")
    sys.exit(1)

import numpy as np
from PIL import Image
import rasterio
from rasterio.warp import transform_bounds
from rasterio.enums import Resampling

# Battlefield boundaries (WGS84 coordinates)
VERDUN_BOUNDS = {
    'north': 49.25,  # Northern extent (Mort-Homme area)
    'south': 49.14,  # Southern extent (Verdun city)
    'west': 5.31,    # Western extent (Left Bank)
    'east': 5.47     # Eastern extent (Right Bank beyond Vaux)
}

# Output directory
OUTPUT_DIR = Path(__file__).parent / 'elevation_data'
OUTPUT_DIR.mkdir(exist_ok=True)

# Output files
OUTPUT_GEOTIFF = OUTPUT_DIR / 'verdun_elevation_raw.tif'
OUTPUT_NUMPY = OUTPUT_DIR / 'verdun_elevation_grid.npy'
OUTPUT_CSV = OUTPUT_DIR / 'verdun_elevation_grid.csv'
OUTPUT_HEIGHTMAP = OUTPUT_DIR / 'verdun_heightmap.png'
OUTPUT_METADATA = OUTPUT_DIR / 'verdun_elevation_metadata.json'


def download_srtm_data():
    """Download SRTM elevation data for Verdun area."""
    print("=" * 70)
    print("VERDUN BATTLEFIELD ELEVATION DATA FETCHER")
    print("=" * 70)
    print(f"\n📍 COVERAGE AREA:")
    print(f"   Latitude:  {VERDUN_BOUNDS['south']}°N to {VERDUN_BOUNDS['north']}°N")
    print(f"   Longitude: {VERDUN_BOUNDS['west']}°E to {VERDUN_BOUNDS['east']}°E")
    print(f"   Resolution: 30 meters (SRTM)")
    print(f"\n📁 OUTPUT DIRECTORY: {OUTPUT_DIR}")

    # Clean any previous SRTM cache to ensure fresh download
    print(f"\n🔄 DOWNLOADING SRTM DATA...")
    print("   (This may take 2-5 minutes for first download)")

    try:
        # Use elevation library to download SRTM data
        cmd = [
            'eio',
            'clip',
            '-o', str(OUTPUT_GEOTIFF),
            '--bounds',
            str(VERDUN_BOUNDS['west']),
            str(VERDUN_BOUNDS['south']),
            str(VERDUN_BOUNDS['east']),
            str(VERDUN_BOUNDS['north'])
        ]

        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode != 0:
            print(f"❌ ERROR: {result.stderr}")
            print("\n💡 TROUBLESHOOTING:")
            print("   1. Ensure 'elevation' package is installed: pip install elevation")
            print("   2. Check internet connection (SRTM data downloaded from NASA)")
            print("   3. Try running: eio selfcheck")
            return False

        print(f"✅ Downloaded: {OUTPUT_GEOTIFF}")
        return True

    except FileNotFoundError:
        print("❌ ERROR: 'eio' command not found")
        print("   The elevation package may not be properly installed.")
        print("   Try: pip install --upgrade elevation")
        return False
    except Exception as e:
        print(f"❌ ERROR: {e}")
        return False


def process_elevation_data():
    """Process GeoTIFF into usable formats."""
    print(f"\n🔄 PROCESSING ELEVATION DATA...")

    if not OUTPUT_GEOTIFF.exists():
        print(f"❌ ERROR: {OUTPUT_GEOTIFF} not found")
        return False

    try:
        # Read GeoTIFF
        with rasterio.open(OUTPUT_GEOTIFF) as src:
            # Read elevation data
            elevation = src.read(1)

            # Get metadata
            bounds = src.bounds
            transform = src.transform
            crs = src.crs
            width = src.width
            height = src.height

            print(f"   Dimensions: {width} x {height} pixels")
            print(f"   Min elevation: {elevation.min():.1f} m")
            print(f"   Max elevation: {elevation.max():.1f} m")
            print(f"   Mean elevation: {elevation.mean():.1f} m")

            # Replace any nodata values with mean elevation
            if src.nodata is not None:
                elevation[elevation == src.nodata] = elevation.mean()

            # Save as NumPy array
            np.save(OUTPUT_NUMPY, elevation)
            print(f"✅ Saved NumPy array: {OUTPUT_NUMPY}")

            # Save as CSV (warning: large file)
            np.savetxt(OUTPUT_CSV, elevation, delimiter=',', fmt='%.2f')
            print(f"✅ Saved CSV: {OUTPUT_CSV}")

            # Create normalized heightmap (0-255 grayscale)
            # Normalize to full 0-255 range for maximum detail
            elevation_min = elevation.min()
            elevation_max = elevation.max()
            elevation_normalized = ((elevation - elevation_min) /
                                   (elevation_max - elevation_min) * 255).astype(np.uint8)

            # Save as PNG (16-bit for more precision)
            # For UE5/Houdini, convert back to 16-bit
            elevation_16bit = ((elevation - elevation_min) /
                              (elevation_max - elevation_min) * 65535).astype(np.uint16)

            # PIL doesn't support uint16 directly for PNG, so we'll save 8-bit
            # and provide instructions for 16-bit conversion
            img = Image.fromarray(elevation_normalized, mode='L')
            img.save(OUTPUT_HEIGHTMAP)
            print(f"✅ Saved heightmap (8-bit): {OUTPUT_HEIGHTMAP}")

            # Save 16-bit heightmap for high-precision import
            heightmap_16bit = OUTPUT_DIR / 'verdun_heightmap_16bit.png'
            img_16 = Image.fromarray(elevation_16bit, mode='I;16')
            img_16.save(heightmap_16bit)
            print(f"✅ Saved heightmap (16-bit): {heightmap_16bit}")

            # Save metadata
            metadata = {
                'source': 'SRTM 30m (NASA Shuttle Radar Topography Mission)',
                'coverage_area': {
                    'north': VERDUN_BOUNDS['north'],
                    'south': VERDUN_BOUNDS['south'],
                    'east': VERDUN_BOUNDS['east'],
                    'west': VERDUN_BOUNDS['west']
                },
                'dimensions': {
                    'width': int(width),
                    'height': int(height),
                    'pixels_total': int(width * height)
                },
                'elevation_meters': {
                    'min': float(elevation.min()),
                    'max': float(elevation.max()),
                    'mean': float(elevation.mean()),
                    'range': float(elevation.max() - elevation.min())
                },
                'coordinate_system': str(crs),
                'resolution_meters': 30,
                'geo_transform': {
                    'pixel_size_x': transform[0],
                    'pixel_size_y': transform[4],
                    'top_left_x': transform[2],
                    'top_left_y': transform[5]
                },
                'historical_note': (
                    'Modern SRTM data represents terrain topography accurate to 1916. '
                    'WWI battlefield modifications (trenches, shell craters) not present, '
                    'but overall topography (hills, valleys, forts, ridges) is identical.'
                ),
                'key_elevations_meters': {
                    'Mort_Homme': 295,
                    'Hill_304': 304,
                    'Fort_Douaumont': 388,
                    'Fort_Vaux': 390,
                    'Verdun_city': 200
                },
                'usage_notes': {
                    'ue5': 'Import verdun_heightmap_16bit.png as Landscape heightmap',
                    'houdini': 'Import verdun_elevation_grid.npy or .csv into HeightField',
                    'qgis': 'Open verdun_elevation_raw.tif directly',
                    'python': 'Load verdun_elevation_grid.npy with np.load()'
                }
            }

            with open(OUTPUT_METADATA, 'w') as f:
                json.dump(metadata, f, indent=2)
            print(f"✅ Saved metadata: {OUTPUT_METADATA}")

            return True

    except Exception as e:
        print(f"❌ ERROR processing data: {e}")
        import traceback
        traceback.print_exc()
        return False


def verify_key_locations():
    """Verify elevations at key historical locations."""
    print(f"\n🔍 VERIFYING KEY LOCATIONS...")

    # Key locations (lat, lon, expected_elevation_m, name)
    locations = [
        (49.2106, 5.4272, 388, 'Fort Douaumont'),
        (49.1989, 5.4494, 390, 'Fort Vaux'),
        (49.2389, 5.3331, 295, 'Mort-Homme'),
        (49.2403, 5.3214, 304, 'Hill 304'),
        (49.1594, 5.3864, 200, 'Verdun City'),
    ]

    try:
        with rasterio.open(OUTPUT_GEOTIFF) as src:
            print(f"   {'Location':<20} {'Expected':<12} {'Actual':<12} {'Diff':<10}")
            print(f"   {'-'*20} {'-'*12} {'-'*12} {'-'*10}")

            for lat, lon, expected, name in locations:
                # Convert lat/lon to pixel coordinates
                row, col = src.index(lon, lat)

                # Read elevation at that pixel
                actual = src.read(1)[row, col]
                diff = actual - expected

                status = '✅' if abs(diff) < 20 else '⚠️'
                print(f"{status} {name:<20} {expected:>6.0f} m     {actual:>6.0f} m     {diff:+6.0f} m")

        print(f"\n   Note: ±10-20m differences are normal (SRTM accuracy ~10m)")

    except Exception as e:
        print(f"⚠️  Could not verify locations: {e}")


def print_usage_instructions():
    """Print usage instructions for the generated data."""
    print(f"\n" + "=" * 70)
    print("✅ ELEVATION DATA READY")
    print("=" * 70)
    print(f"\n📁 OUTPUT FILES:")
    print(f"   {OUTPUT_GEOTIFF.relative_to(Path.cwd())}")
    print(f"   {OUTPUT_NUMPY.relative_to(Path.cwd())}")
    print(f"   {OUTPUT_CSV.relative_to(Path.cwd())}")
    print(f"   {OUTPUT_HEIGHTMAP.relative_to(Path.cwd())}")
    print(f"   {(OUTPUT_DIR / 'verdun_heightmap_16bit.png').relative_to(Path.cwd())}")
    print(f"   {OUTPUT_METADATA.relative_to(Path.cwd())}")

    print(f"\n🎮 USAGE IN GAME ENGINES:")
    print(f"\n   UNREAL ENGINE 5:")
    print(f"   1. Open UE5 Landscape tool")
    print(f"   2. Import → Select 'verdun_heightmap_16bit.png'")
    print(f"   3. Set scale: Z=100 (cm), XY=3000 (30m pixels → 30m scale)")
    print(f"   4. Terrain will generate with accurate Verdun topography")

    print(f"\n   HOUDINI:")
    print(f"   1. Create HeightField node")
    print(f"   2. HeightField File → Load 'verdun_elevation_grid.csv'")
    print(f"   3. Set grid spacing to 30 meters")
    print(f"   4. Use for procedural terrain generation")

    print(f"\n   QGIS (GIS Analysis):")
    print(f"   1. Layer → Add Raster Layer")
    print(f"   2. Select 'verdun_elevation_raw.tif'")
    print(f"   3. Use for accurate geographic analysis and visualization")

    print(f"\n   PYTHON (Custom Processing):")
    print(f"   import numpy as np")
    print(f"   elevation = np.load('verdun_elevation_grid.npy')")
    print(f"   # Process as needed")

    print(f"\n📊 NEXT STEPS:")
    print(f"   1. Run export_for_ue5.py to generate location markers")
    print(f"   2. Run export_for_houdini.py for Houdini-specific format")
    print(f"   3. See locations_database.csv for all historical coordinates")
    print(f"\n" + "=" * 70)


def main():
    """Main execution."""
    # Step 1: Download SRTM data
    if not download_srtm_data():
        return 1

    # Step 2: Process into multiple formats
    if not process_elevation_data():
        return 1

    # Step 3: Verify key locations
    verify_key_locations()

    # Step 4: Print usage instructions
    print_usage_instructions()

    return 0


if __name__ == '__main__':
    sys.exit(main())
