#!/usr/bin/env python3
"""
Verdun GPS to UE5 Coordinate Converter
Converts the JSON positional data to UE5 world coordinates and generates a preprocessed file.
"""

import json
import math
import csv
from pathlib import Path


class VerdunCoordinateConverter:
    """Converts GPS coordinates to UE5 world space."""

    def __init__(self, origin_lat=49.1667, origin_lon=5.3833, origin_elev=200.0):
        """
        Initialize converter with world origin.

        Args:
            origin_lat: Origin latitude in decimal degrees (default: Verdun city center)
            origin_lon: Origin longitude in decimal degrees (default: Verdun city center)
            origin_elev: Origin elevation in meters (default: Verdun city center)
        """
        self.origin_lat = origin_lat
        self.origin_lon = origin_lon
        self.origin_elev = origin_elev

        # Conversion constants
        self.CM_PER_DEGREE_LAT = 11_132_000  # cm per degree latitude
        self.CM_PER_DEGREE_LON = 7_267_500   # cm per degree longitude at ~49°N
        self.CM_PER_METER = 100              # cm per meter

    def convert_to_ue5(self, lat, lon, elev):
        """
        Convert GPS coordinates to UE5 world space.

        Args:
            lat: Latitude in decimal degrees
            lon: Longitude in decimal degrees
            elev: Elevation in meters

        Returns:
            Tuple of (X, Y, Z) in centimeters (UE5 units)
        """
        # Calculate deltas from origin
        delta_lat = lat - self.origin_lat
        delta_lon = lon - self.origin_lon
        delta_elev = elev - self.origin_elev

        # Convert to UE5 coordinates (cm)
        x = delta_lat * self.CM_PER_DEGREE_LAT  # North
        y = delta_lon * self.CM_PER_DEGREE_LON  # East
        z = delta_elev * self.CM_PER_METER      # Up

        return (x, y, z)

    def convert_to_gps(self, x, y, z):
        """
        Convert UE5 world space back to GPS coordinates.

        Args:
            x: UE5 X coordinate in centimeters
            y: UE5 Y coordinate in centimeters
            z: UE5 Z coordinate in centimeters

        Returns:
            Tuple of (latitude, longitude, elevation)
        """
        lat = (x / self.CM_PER_DEGREE_LAT) + self.origin_lat
        lon = (y / self.CM_PER_DEGREE_LON) + self.origin_lon
        elev = (z / self.CM_PER_METER) + self.origin_elev

        return (lat, lon, elev)

    def haversine_distance(self, lat1, lon1, lat2, lon2):
        """
        Calculate distance between two GPS points using Haversine formula.

        Args:
            lat1, lon1: First point in decimal degrees
            lat2, lon2: Second point in decimal degrees

        Returns:
            Distance in meters
        """
        R = 6_371_000  # Earth radius in meters

        phi1 = math.radians(lat1)
        phi2 = math.radians(lat2)
        delta_phi = math.radians(lat2 - lat1)
        delta_lambda = math.radians(lon2 - lon1)

        a = (math.sin(delta_phi / 2) ** 2 +
             math.cos(phi1) * math.cos(phi2) * math.sin(delta_lambda / 2) ** 2)
        c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

        return R * c

    def euclidean_distance(self, x1, y1, z1, x2, y2, z2):
        """
        Calculate Euclidean distance between two UE5 points.

        Args:
            x1, y1, z1: First point in UE5 coordinates (cm)
            x2, y2, z2: Second point in UE5 coordinates (cm)

        Returns:
            Distance in meters
        """
        dx = x2 - x1
        dy = y2 - y1
        dz = z2 - z1

        distance_cm = math.sqrt(dx**2 + dy**2 + dz**2)
        return distance_cm / 100  # Convert to meters


def process_json_file(input_file, output_file):
    """
    Process the master JSON file and add UE5 coordinates.

    Args:
        input_file: Path to input JSON file
        output_file: Path to output JSON file with UE5 coordinates
    """
    converter = VerdunCoordinateConverter()

    # Load JSON
    with open(input_file, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # Add conversion metadata
    data['ue5_conversion'] = {
        'origin_latitude': converter.origin_lat,
        'origin_longitude': converter.origin_lon,
        'origin_elevation': converter.origin_elev,
        'units': 'centimeters',
        'coordinate_system': 'X=North, Y=East, Z=Up'
    }

    # Process each category
    categories = ['forts', 'destroyed_villages', 'towns_cities', 'terrain_features', 'forests']

    for category in categories:
        if category in data:
            for item in data[category]:
                if 'coordinates' in item:
                    coords = item['coordinates']
                    lat = coords['latitude']
                    lon = coords['longitude']
                    elev = coords.get('elevation_meters', 200)

                    # Convert to UE5
                    x, y, z = converter.convert_to_ue5(lat, lon, elev)

                    # Add UE5 coordinates
                    item['ue5_coordinates'] = {
                        'x': round(x, 2),
                        'y': round(y, 2),
                        'z': round(z, 2),
                        'units': 'centimeters'
                    }

    # Process roads (have multiple points)
    if 'roads' in data:
        for road in data['roads']:
            if 'route_points' in road:
                for point in road['route_points']:
                    if 'coordinates' in point:
                        coords = point['coordinates']
                        lat = coords['latitude']
                        lon = coords['longitude']
                        elev = coords.get('elevation_meters', 200)

                        x, y, z = converter.convert_to_ue5(lat, lon, elev)

                        point['ue5_coordinates'] = {
                            'x': round(x, 2),
                            'y': round(y, 2),
                            'z': round(z, 2),
                            'units': 'centimeters'
                        }

    # Process trenches
    if 'pre_1916_trenches' in data and 'systems' in data['pre_1916_trenches']:
        for system in data['pre_1916_trenches']['systems']:
            if 'approximate_line' in system:
                ue5_line = []
                for point in system['approximate_line']:
                    lat = point['latitude']
                    lon = point['longitude']
                    elev = point.get('elevation', 200)

                    x, y, z = converter.convert_to_ue5(lat, lon, elev)

                    ue5_line.append({
                        'x': round(x, 2),
                        'y': round(y, 2),
                        'z': round(z, 2)
                    })

                system['ue5_approximate_line'] = ue5_line

    # Save output
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

    print(f"✓ Converted coordinates written to: {output_file}")


def generate_ue5_csv(input_file, output_dir):
    """
    Generate simplified CSV files for UE5 Data Table import with UE5 coordinates.

    Args:
        input_file: Path to master JSON file
        output_dir: Directory to write CSV files
    """
    converter = VerdunCoordinateConverter()

    with open(input_file, 'r', encoding='utf-8') as f:
        data = json.load(f)

    output_dir = Path(output_dir)
    output_dir.mkdir(exist_ok=True)

    # Villages CSV
    with open(output_dir / 'villages_ue5.csv', 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(['ID', 'Name', 'Type', 'Latitude', 'Longitude', 'Elevation_m',
                        'UE5_X', 'UE5_Y', 'UE5_Z', 'Population_1914', 'Houses', 'Diameter_m'])

        for village in data.get('destroyed_villages', []):
            coords = village['coordinates']
            lat, lon, elev = coords['latitude'], coords['longitude'], coords['elevation_meters']
            x, y, z = converter.convert_to_ue5(lat, lon, elev)

            ue5_notes = village.get('ue5_notes', {})
            buildings = ue5_notes.get('recommended_buildings', {})
            footprint = ue5_notes.get('footprint_meters', {})

            writer.writerow([
                village['id'],
                village['name'],
                village['type'],
                lat,
                lon,
                elev,
                round(x, 2),
                round(y, 2),
                round(z, 2),
                village.get('population_1914', 0),
                buildings.get('houses', 0),
                footprint.get('diameter', 0)
            ])

    print(f"✓ Villages CSV written to: {output_dir / 'villages_ue5.csv'}")

    # Forts CSV
    with open(output_dir / 'forts_ue5.csv', 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(['ID', 'Name', 'Type', 'Latitude', 'Longitude', 'Elevation_m',
                        'UE5_X', 'UE5_Y', 'UE5_Z', 'Length_m', 'Width_m', 'Height_m'])

        for fort in data.get('forts', []):
            coords = fort['coordinates']
            lat, lon, elev = coords['latitude'], coords['longitude'], coords['elevation_meters']
            x, y, z = converter.convert_to_ue5(lat, lon, elev)

            ue5_notes = fort.get('ue5_notes', {})
            footprint = ue5_notes.get('footprint_meters', {})

            writer.writerow([
                fort['id'],
                fort['name'],
                fort['category'],
                lat,
                lon,
                elev,
                round(x, 2),
                round(y, 2),
                round(z, 2),
                footprint.get('length', 0),
                footprint.get('width', 0),
                footprint.get('height', 0)
            ])

    print(f"✓ Forts CSV written to: {output_dir / 'forts_ue5.csv'}")

    # All locations CSV (simplified)
    with open(output_dir / 'all_locations_ue5.csv', 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(['ID', 'Name', 'Category', 'UE5_X', 'UE5_Y', 'UE5_Z', 'Description'])

        # Add all categories
        for category in ['forts', 'destroyed_villages', 'towns_cities', 'terrain_features', 'forests']:
            for item in data.get(category, []):
                if 'coordinates' in item:
                    coords = item['coordinates']
                    x, y, z = converter.convert_to_ue5(
                        coords['latitude'],
                        coords['longitude'],
                        coords.get('elevation_meters', 200)
                    )

                    writer.writerow([
                        item['id'],
                        item['name'],
                        category,
                        round(x, 2),
                        round(y, 2),
                        round(z, 2),
                        item.get('description', '')
                    ])

    print(f"✓ All locations CSV written to: {output_dir / 'all_locations_ue5.csv'}")


def validate_conversions():
    """Validate coordinate conversions with known distances."""
    converter = VerdunCoordinateConverter()

    print("\n=== Validation: Testing Coordinate Conversions ===\n")

    # Fort Douaumont
    fort_d = (49.216868, 5.4383833, 392)
    ue5_d = converter.convert_to_ue5(*fort_d)
    print(f"Fort Douaumont:")
    print(f"  GPS: {fort_d}")
    print(f"  UE5: X={ue5_d[0]:,.0f} Y={ue5_d[1]:,.0f} Z={ue5_d[2]:,.0f} cm")
    print(f"       ({ue5_d[0]/100000:.2f} km N, {ue5_d[1]/100000:.2f} km E, {ue5_d[2]/100:.0f} m Up)")

    # Fort Vaux
    fort_v = (49.20049, 5.47016, 320)
    ue5_v = converter.convert_to_ue5(*fort_v)
    print(f"\nFort Vaux:")
    print(f"  GPS: {fort_v}")
    print(f"  UE5: X={ue5_v[0]:,.0f} Y={ue5_v[1]:,.0f} Z={ue5_v[2]:,.0f} cm")
    print(f"       ({ue5_v[0]/100000:.2f} km N, {ue5_v[1]/100000:.2f} km E, {ue5_v[2]/100:.0f} m Up)")

    # Distance between forts
    gps_dist = converter.haversine_distance(*fort_d[:2], *fort_v[:2])
    ue5_dist = converter.euclidean_distance(*ue5_d, *ue5_v)

    print(f"\nDistance Fort Douaumont → Fort Vaux:")
    print(f"  GPS (Haversine): {gps_dist:,.1f} meters")
    print(f"  UE5 (Euclidean): {ue5_dist:,.1f} meters")
    print(f"  Difference: {abs(gps_dist - ue5_dist):,.1f} meters ({abs(gps_dist - ue5_dist)/gps_dist*100:.2f}%)")

    # Test inverse conversion
    print(f"\n=== Testing Inverse Conversion ===\n")
    lat_back, lon_back, elev_back = converter.convert_to_gps(*ue5_d)
    print(f"Fort Douaumont round-trip:")
    print(f"  Original GPS: {fort_d}")
    print(f"  After UE5 → GPS: ({lat_back:.6f}, {lon_back:.7f}, {elev_back:.1f})")
    print(f"  Error: {abs(lat_back - fort_d[0])*111320:.2f}m lat, {abs(lon_back - fort_d[1])*72675:.2f}m lon")


def main():
    """Main entry point."""
    script_dir = Path(__file__).parent

    input_file = script_dir / 'verdun_master_locations.json'
    output_file = script_dir / 'verdun_master_locations_with_ue5.json'

    print("=== Verdun GPS to UE5 Coordinate Converter ===\n")

    if not input_file.exists():
        print(f"✗ Error: Input file not found: {input_file}")
        return

    # Process JSON and add UE5 coordinates
    print("Converting coordinates...")
    process_json_file(input_file, output_file)

    # Generate UE5-ready CSV files
    print("\nGenerating UE5 CSV files...")
    generate_ue5_csv(input_file, script_dir)

    # Validate conversions
    validate_conversions()

    print("\n✓ All done! Files ready for UE5 import.")
    print("\nGenerated files:")
    print(f"  - {output_file}")
    print(f"  - {script_dir / 'villages_ue5.csv'}")
    print(f"  - {script_dir / 'forts_ue5.csv'}")
    print(f"  - {script_dir / 'all_locations_ue5.csv'}")


if __name__ == '__main__':
    main()
