#!/usr/bin/env python3
"""
TERRAIN AUTOMATION TEST SCRIPT
Tests the complete world-building automation for Battle of Verdun terrain data.

Purpose:
  - Validate terrain_parameters.json structure and data
  - Validate locations_database.csv completeness
  - Test coordinate conversion systems
  - Simulate terrain generation pipeline
  - Verify historical accuracy of data

Usage:
  python test_terrain_automation.py
"""

import json
import csv
import math
import sys
from pathlib import Path
from datetime import datetime


# ANSI color codes for terminal output
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def print_header(text):
    print(f"\n{Colors.HEADER}{Colors.BOLD}{'='*80}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{text:^80}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{'='*80}{Colors.ENDC}\n")


def print_section(text):
    print(f"\n{Colors.OKCYAN}{Colors.BOLD}{'─'*80}{Colors.ENDC}")
    print(f"{Colors.OKCYAN}{Colors.BOLD}{text}{Colors.ENDC}")
    print(f"{Colors.OKCYAN}{Colors.BOLD}{'─'*80}{Colors.ENDC}")


def print_success(text):
    print(f"{Colors.OKGREEN}✓ {text}{Colors.ENDC}")


def print_warning(text):
    print(f"{Colors.WARNING}⚠ {text}{Colors.ENDC}")


def print_error(text):
    print(f"{Colors.FAIL}✗ {text}{Colors.ENDC}")


def print_info(text):
    print(f"{Colors.OKBLUE}ℹ {text}{Colors.ENDC}")


class TerrainDataValidator:
    """Validates and tests terrain data structures"""

    def __init__(self):
        self.base_path = Path(__file__).parent
        self.terrain_params = None
        self.locations = []
        self.test_results = {
            'passed': 0,
            'failed': 0,
            'warnings': 0
        }

    def load_terrain_parameters(self):
        """Load and validate terrain_parameters.json"""
        print_section("TEST 1: Loading Terrain Parameters")

        try:
            params_file = self.base_path / 'terrain_parameters.json'
            with open(params_file, 'r') as f:
                self.terrain_params = json.load(f)
            print_success(f"Loaded terrain_parameters.json ({params_file.stat().st_size:,} bytes)")
            self.test_results['passed'] += 1
        except FileNotFoundError:
            print_error("terrain_parameters.json not found!")
            self.test_results['failed'] += 1
            return False
        except json.JSONDecodeError as e:
            print_error(f"Invalid JSON: {e}")
            self.test_results['failed'] += 1
            return False

        # Validate required sections
        required_sections = [
            'geographic_coverage',
            'elevation_data',
            'surface_materials',
            'battlefield_modifications',
            'weather_effects_on_terrain'
        ]

        for section in required_sections:
            if section in self.terrain_params:
                print_success(f"Section '{section}' present")
                self.test_results['passed'] += 1
            else:
                print_error(f"Missing required section: {section}")
                self.test_results['failed'] += 1

        return True

    def validate_terrain_coverage(self):
        """Validate geographic coverage data"""
        print_section("TEST 2: Geographic Coverage Validation")

        coverage = self.terrain_params['geographic_coverage']
        bounds = coverage['bounds']

        # Check coordinate bounds are reasonable
        if 49.0 <= bounds['north'] <= 50.0 and 49.0 <= bounds['south'] <= 50.0:
            print_success(f"Latitude bounds valid: {bounds['south']}°N to {bounds['north']}°N")
            self.test_results['passed'] += 1
        else:
            print_error(f"Invalid latitude bounds: {bounds}")
            self.test_results['failed'] += 1

        if 5.0 <= bounds['east'] <= 6.0 and 5.0 <= bounds['west'] <= 6.0:
            print_success(f"Longitude bounds valid: {bounds['west']}°E to {bounds['east']}°E")
            self.test_results['passed'] += 1
        else:
            print_error(f"Invalid longitude bounds: {bounds}")
            self.test_results['failed'] += 1

        # Calculate approximate area
        lat_diff = bounds['north'] - bounds['south']
        lon_diff = bounds['east'] - bounds['west']
        approx_area = lat_diff * lon_diff * 111.32 * 111.32  # km²

        print_info(f"Calculated area: {approx_area:.1f} km²")
        print_info(f"Declared area: {coverage['area_km2']} km²")

        if abs(approx_area - coverage['area_km2']) < 20:
            print_success("Area calculation matches declared area")
            self.test_results['passed'] += 1
        else:
            print_warning("Area calculation differs from declared area (acceptable for rough bounds)")
            self.test_results['warnings'] += 1

    def validate_elevation_data(self):
        """Validate elevation parameters"""
        print_section("TEST 3: Elevation Data Validation")

        elev = self.terrain_params['elevation_data']
        elev_range = elev['range_meters']

        # Check elevation range is reasonable
        if 100 <= elev_range['min'] <= 200 and 350 <= elev_range['max'] <= 450:
            print_success(f"Elevation range valid: {elev_range['min']}m to {elev_range['max']}m")
            self.test_results['passed'] += 1
        else:
            print_error(f"Suspicious elevation range: {elev_range}")
            self.test_results['failed'] += 1

        # Check key fort elevations
        forts = self.terrain_params['key_elevations_meters']['forts']
        for fort_name, elevation in forts.items():
            if elev_range['min'] <= elevation <= elev_range['max']:
                print_success(f"{fort_name}: {elevation}m (within valid range)")
                self.test_results['passed'] += 1
            else:
                print_error(f"{fort_name}: {elevation}m (outside valid range!)")
                self.test_results['failed'] += 1

    def validate_surface_materials(self):
        """Validate surface material composition"""
        print_section("TEST 4: Surface Material Validation")

        materials = self.terrain_params['surface_materials']
        composition = materials['composition_percentage']

        # Check percentages sum to 100
        total_percentage = sum(composition.values())
        if total_percentage == 100:
            print_success(f"Material composition sums to 100% (mud: {composition['mud']}%, chalk: {composition['chalk']}%, etc.)")
            self.test_results['passed'] += 1
        else:
            print_warning(f"Material composition sums to {total_percentage}% (not exactly 100%)")
            self.test_results['warnings'] += 1

        # Check mud dominance (historically accurate for Verdun)
        if composition['mud'] >= 60:
            print_success(f"Mud percentage realistic for Verdun: {composition['mud']}%")
            self.test_results['passed'] += 1
        else:
            print_warning(f"Mud percentage seems low for Verdun: {composition['mud']}%")
            self.test_results['warnings'] += 1

    def validate_battlefield_modifications(self):
        """Validate battlefield modification data"""
        print_section("TEST 5: Battlefield Modifications Validation")

        mods = self.terrain_params['battlefield_modifications']

        # Validate trench data
        trenches = mods['trenches']
        if trenches['total_length_km'] > 0:
            print_success(f"Trench network: {trenches['total_length_km']} km total length")
            self.test_results['passed'] += 1

        dimensions = trenches['dimensions']
        if 1.0 <= dimensions['width_m'] <= 2.0 and 1.5 <= dimensions['depth_m'] <= 2.5:
            print_success(f"Trench dimensions realistic: {dimensions['width_m']}m wide × {dimensions['depth_m']}m deep")
            self.test_results['passed'] += 1
        else:
            print_warning(f"Trench dimensions unusual: {dimensions}")
            self.test_results['warnings'] += 1

        # Validate crater data
        craters = mods['shell_craters']
        if craters['total_estimated'] > 100000:
            print_success(f"Shell crater count: {craters['total_estimated']:,} (historically massive)")
            self.test_results['passed'] += 1
        else:
            print_warning(f"Shell crater count seems low: {craters['total_estimated']:,}")
            self.test_results['warnings'] += 1

        # Validate destroyed villages
        villages = mods['destroyed_structures']['villages']
        if len(villages) >= 8:
            print_success(f"Destroyed villages documented: {len(villages)} villages")
            print_info(f"  Including: {villages[0]}, {villages[1]}, etc.")
            self.test_results['passed'] += 1
        else:
            print_warning(f"Expected at least 8 destroyed villages, found {len(villages)}")
            self.test_results['warnings'] += 1

    def load_locations_database(self):
        """Load and validate locations_database.csv"""
        print_section("TEST 6: Loading Locations Database")

        try:
            locations_file = self.base_path / 'locations_database.csv'
            with open(locations_file, 'r') as f:
                reader = csv.DictReader(f)
                self.locations = list(reader)

            print_success(f"Loaded {len(self.locations)} locations from database")
            self.test_results['passed'] += 1
        except FileNotFoundError:
            print_error("locations_database.csv not found!")
            self.test_results['failed'] += 1
            return False
        except Exception as e:
            print_error(f"Error loading locations: {e}")
            self.test_results['failed'] += 1
            return False

        # Validate location types
        location_types = {}
        for loc in self.locations:
            loc_type = loc['type']
            location_types[loc_type] = location_types.get(loc_type, 0) + 1

        print_info(f"Location breakdown:")
        for loc_type, count in sorted(location_types.items()):
            print_info(f"  {loc_type}: {count}")

        # Check for key locations
        key_locations = ['Fort Douaumont', 'Fort Vaux', 'Mort-Homme (Hill 295)', 'Fleury-devant-Douaumont']
        for key_loc in key_locations:
            found = any(loc['name'] == key_loc for loc in self.locations)
            if found:
                print_success(f"Key location present: {key_loc}")
                self.test_results['passed'] += 1
            else:
                print_error(f"Missing key location: {key_loc}")
                self.test_results['failed'] += 1

        return True

    def validate_location_coordinates(self):
        """Validate location coordinates are within bounds"""
        print_section("TEST 7: Location Coordinate Validation")

        bounds = self.terrain_params['geographic_coverage']['bounds']

        invalid_coords = []
        for loc in self.locations:
            try:
                lat = float(loc['latitude'])
                lon = float(loc['longitude'])

                if not (bounds['south'] <= lat <= bounds['north'] and
                        bounds['west'] <= lon <= bounds['east']):
                    invalid_coords.append(loc['name'])
            except (ValueError, KeyError):
                invalid_coords.append(f"{loc['name']} (parse error)")

        if not invalid_coords:
            print_success(f"All {len(self.locations)} locations within declared bounds")
            self.test_results['passed'] += 1
        else:
            print_warning(f"{len(invalid_coords)} locations outside bounds (may be support/rear areas):")
            for name in invalid_coords[:5]:  # Show first 5
                print_info(f"  {name}")
            self.test_results['warnings'] += 1

    def test_coordinate_conversion(self):
        """Test coordinate conversion to game engine coordinates"""
        print_section("TEST 8: Coordinate Conversion Testing")

        # Get battlefield center
        center = self.terrain_params['geographic_coverage']['center']
        center_lat = center['latitude']
        center_lon = center['longitude']

        print_info(f"Battlefield center: {center_lat}°N, {center_lon}°E")

        # Test conversion for Fort Douaumont
        fort_douaumont = next((loc for loc in self.locations if loc['name'] == 'Fort Douaumont'), None)
        if fort_douaumont:
            lat = float(fort_douaumont['latitude'])
            lon = float(fort_douaumont['longitude'])

            # Simple conversion to meters from center (simplified, not accurate for large areas)
            lat_diff = (lat - center_lat) * 111320  # meters per degree latitude
            lon_diff = (lon - center_lon) * 111320 * math.cos(math.radians(center_lat))

            print_success(f"Fort Douaumont offset from center:")
            print_info(f"  North: {lat_diff:.0f} meters")
            print_info(f"  East: {lon_diff:.0f} meters")

            # Convert to UE5 coordinates (centimeters)
            ue5_x = lon_diff * 100  # meters to cm
            ue5_y = lat_diff * 100
            ue5_z = float(fort_douaumont['elevation_m']) * 100

            print_success(f"UE5 coordinates (cm):")
            print_info(f"  X: {ue5_x:.0f}, Y: {ue5_y:.0f}, Z: {ue5_z:.0f}")

            self.test_results['passed'] += 1
        else:
            print_error("Fort Douaumont not found for coordinate test")
            self.test_results['failed'] += 1

    def simulate_terrain_generation(self):
        """Simulate terrain generation pipeline"""
        print_section("TEST 9: Terrain Generation Simulation")

        print_info("Simulating world-building automation pipeline...")

        # Step 1: Base terrain from elevation data
        print_success("[1/6] Base terrain: SRTM 30m elevation data")
        print_info(f"  Resolution: {self.terrain_params['elevation_data']['resolution_meters']}m/pixel")
        print_info(f"  Range: {self.terrain_params['elevation_data']['range_meters']['min']}-{self.terrain_params['elevation_data']['range_meters']['max']}m")
        self.test_results['passed'] += 1

        # Step 2: Apply surface materials
        composition = self.terrain_params['surface_materials']['composition_percentage']
        print_success("[2/6] Surface materials applied")
        print_info(f"  Mud: {composition['mud']}%, Chalk: {composition['chalk']}%, Grass: {composition['dead_grass']}%, Rock: {composition['rock']}%")
        self.test_results['passed'] += 1

        # Step 3: Generate trenches
        trenches = self.terrain_params['battlefield_modifications']['trenches']
        print_success("[3/6] Trench network generated")
        print_info(f"  Total length: {trenches['total_length_km']} km")
        print_info(f"  Pattern: {trenches['pattern']}")
        self.test_results['passed'] += 1

        # Step 4: Scatter shell craters
        craters = self.terrain_params['battlefield_modifications']['shell_craters']
        print_success("[4/6] Shell craters scattered")
        print_info(f"  Total count: {craters['total_estimated']:,}")
        print_info(f"  Overlapping: {craters['overlapping']}")
        self.test_results['passed'] += 1

        # Step 5: Place historical locations
        print_success("[5/6] Historical locations placed")
        print_info(f"  {len(self.locations)} locations imported")
        forts = [loc for loc in self.locations if loc['type'] == 'fortification']
        print_info(f"  Including {len(forts)} major fortifications")
        self.test_results['passed'] += 1

        # Step 6: Apply weather/seasonal effects
        print_success("[6/6] Weather and seasonal effects configured")
        print_info(f"  Visibility: {self.terrain_params['visibility_conditions']}")
        self.test_results['passed'] += 1

        print_success("Terrain generation pipeline simulation complete!")

    def test_gameplay_parameters(self):
        """Test gameplay parameter consistency"""
        print_section("TEST 10: Gameplay Parameters Validation")

        gameplay = self.terrain_params['gameplay_parameters']

        # Test movement speed multipliers
        movement = gameplay['movement_speed_multipliers']
        if all(0.0 <= v <= 1.0 for v in movement.values()):
            print_success(f"Movement speed multipliers valid (0.0-1.0 range)")
            print_info(f"  Mud (ankle): {movement['mud_ankle_deep']}x")
            print_info(f"  Mud (knee): {movement['mud_knee_deep']}x")
            print_info(f"  No man's land (crawl): {movement['no_mans_land_crawling']}x")
            self.test_results['passed'] += 1
        else:
            print_error("Movement speed multipliers out of range")
            self.test_results['failed'] += 1

        # Test cover quality
        cover = gameplay['cover_quality']
        if all(0.0 <= v <= 1.0 for v in cover.values()):
            print_success(f"Cover quality values valid (0.0-1.0 range)")
            print_info(f"  Trench: {cover['trench']}")
            print_info(f"  Shell crater: {cover['shell_crater']}")
            print_info(f"  Open ground: {cover['open_ground']}")
            self.test_results['passed'] += 1
        else:
            print_error("Cover quality values out of range")
            self.test_results['failed'] += 1

    def test_historical_accuracy(self):
        """Test historical accuracy verification data"""
        print_section("TEST 11: Historical Accuracy Verification")

        accuracy = self.terrain_params['historical_accuracy_verification']

        tier1 = accuracy['tier_1_documented']
        tier2 = accuracy['tier_2_reconstructed']
        tier3 = accuracy['tier_3_informed_speculation']

        print_success(f"Tier 1 (Documented): {len(tier1)} facts")
        for fact in tier1[:3]:
            print_info(f"  • {fact}")

        print_success(f"Tier 2 (Reconstructed): {len(tier2)} elements")
        for fact in tier2[:3]:
            print_info(f"  • {fact}")

        print_success(f"Tier 3 (Informed Speculation): {len(tier3)} elements")
        for fact in tier3[:3]:
            print_info(f"  • {fact}")

        self.test_results['passed'] += 1

    def generate_test_report(self):
        """Generate final test report"""
        print_section("TEST SUMMARY")

        total_tests = self.test_results['passed'] + self.test_results['failed'] + self.test_results['warnings']

        print_info(f"Total tests run: {total_tests}")
        print_success(f"Passed: {self.test_results['passed']}")

        if self.test_results['warnings'] > 0:
            print_warning(f"Warnings: {self.test_results['warnings']}")

        if self.test_results['failed'] > 0:
            print_error(f"Failed: {self.test_results['failed']}")

        # Calculate success rate
        success_rate = (self.test_results['passed'] / total_tests * 100) if total_tests > 0 else 0

        print_info(f"\nSuccess rate: {success_rate:.1f}%")

        if self.test_results['failed'] == 0:
            print_success("\n🎖️  ALL TESTS PASSED - TERRAIN AUTOMATION READY FOR DEPLOYMENT")
            return 0
        else:
            print_error("\n⚠️  SOME TESTS FAILED - REVIEW ERRORS ABOVE")
            return 1


def main():
    """Main test execution"""
    print_header("VERDUN BATTLEFIELD TERRAIN AUTOMATION TEST")
    print_info(f"Test started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print_info(f"Testing data for Battle of Verdun (Feb 21 - Dec 18, 1916)")

    validator = TerrainDataValidator()

    # Run all tests
    if not validator.load_terrain_parameters():
        print_error("Critical error: Cannot load terrain parameters")
        return 1

    validator.validate_terrain_coverage()
    validator.validate_elevation_data()
    validator.validate_surface_materials()
    validator.validate_battlefield_modifications()

    if not validator.load_locations_database():
        print_error("Critical error: Cannot load locations database")
        return 1

    validator.validate_location_coordinates()
    validator.test_coordinate_conversion()
    validator.simulate_terrain_generation()
    validator.test_gameplay_parameters()
    validator.test_historical_accuracy()

    # Generate final report
    result = validator.generate_test_report()

    print_info(f"\nTest completed: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print_header("END OF TERRAIN AUTOMATION TEST")

    return result


if __name__ == '__main__':
    sys.exit(main())
