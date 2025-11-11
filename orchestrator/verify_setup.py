#!/usr/bin/env python3
"""
verify_setup.py - Verify pipeline setup and dependencies.

Checks that all required tools, directories, and files are properly configured.

Usage:
    python orchestrator/verify_setup.py
"""

import sys
import os
from pathlib import Path
import subprocess
import json

# Colors for terminal output
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'

def print_success(msg):
    print(f"{Colors.GREEN}✓{Colors.END} {msg}")

def print_error(msg):
    print(f"{Colors.RED}✗{Colors.END} {msg}")

def print_warning(msg):
    print(f"{Colors.YELLOW}⚠{Colors.END} {msg}")

def print_section(msg):
    print(f"\n{Colors.BLUE}{'=' * 60}{Colors.END}")
    print(f"{Colors.BLUE}{msg}{Colors.END}")
    print(f"{Colors.BLUE}{'=' * 60}{Colors.END}")


class SetupVerifier:
    """Verify pipeline setup."""

    def __init__(self):
        self.repo_root = Path(__file__).parent.parent
        self.errors = []
        self.warnings = []

    def check_python_version(self):
        """Check Python version."""
        print_section("Python Environment")

        version = sys.version_info
        if version.major >= 3 and version.minor >= 9:
            print_success(f"Python {version.major}.{version.minor}.{version.micro}")
        else:
            self.errors.append("Python 3.9+ required")
            print_error(f"Python {version.major}.{version.minor}.{version.micro} (need 3.9+)")

    def check_python_packages(self):
        """Check required Python packages."""
        print_section("Python Packages")

        required = {
            "click": "CLI framework",
            "yaml": "Configuration (PyYAML)",
        }

        for module, description in required.items():
            try:
                __import__(module)
                print_success(f"{module} - {description}")
            except ImportError:
                self.errors.append(f"Missing Python package: {module}")
                print_error(f"{module} - {description} (MISSING)")

        print_warning("Run: pip install -r requirements.txt")

    def check_directories(self):
        """Check required directories exist."""
        print_section("Directory Structure")

        required_dirs = [
            "houdini/hdas",
            "houdini/hip",
            "houdini/python",
            "ue/Scripts",
            "orchestrator",
            "data/incoming/vectors",
            "data/incoming/rasters",
            "data/incoming/footprints",
            "data/staging",
            "data/published",
            "docs",
        ]

        for dir_path in required_dirs:
            full_path = self.repo_root / dir_path
            if full_path.exists():
                print_success(f"{dir_path}")
            else:
                self.warnings.append(f"Directory missing: {dir_path}")
                print_warning(f"{dir_path} (missing)")

    def check_config(self):
        """Check configuration file."""
        print_section("Configuration")

        config_file = self.repo_root / "orchestrator" / "config.yaml"

        if config_file.exists():
            print_success(f"config.yaml exists")

            try:
                import yaml
                with open(config_file, 'r') as f:
                    config = yaml.safe_load(f)

                # Check required sections
                required_sections = ["paths", "houdini", "unreal", "generation"]
                for section in required_sections:
                    if section in config:
                        print_success(f"  Section: {section}")
                    else:
                        self.warnings.append(f"Config missing section: {section}")
                        print_warning(f"  Section: {section} (missing)")

            except Exception as e:
                self.errors.append(f"Config parse error: {e}")
                print_error(f"Failed to parse config.yaml: {e}")
        else:
            self.errors.append("config.yaml not found")
            print_error("config.yaml not found")

    def check_houdini(self):
        """Check Houdini installation."""
        print_section("Houdini")

        # Check environment variable
        hfs = os.environ.get("HFS")
        if hfs:
            print_success(f"$HFS = {hfs}")

            # Check for hython
            hython = Path(hfs) / "bin" / "hython"
            if not hython.exists():
                hython = hython.with_suffix(".exe")  # Windows

            if hython.exists():
                print_success(f"hython found: {hython}")

                # Try to get version
                try:
                    result = subprocess.run(
                        [str(hython), "--version"],
                        capture_output=True,
                        text=True,
                        timeout=5
                    )
                    if result.returncode == 0:
                        version = result.stdout.strip() or result.stderr.strip()
                        print_success(f"  Version: {version}")
                except Exception as e:
                    print_warning(f"  Could not get version: {e}")
            else:
                self.warnings.append("hython not found in $HFS/bin")
                print_warning(f"hython not found in {Path(hfs) / 'bin'}")
        else:
            self.warnings.append("$HFS environment variable not set")
            print_warning("$HFS environment variable not set")
            print_warning("  Set with: export HFS=/path/to/houdini")

    def check_unreal(self):
        """Check Unreal Engine installation."""
        print_section("Unreal Engine 5")

        ue_path = os.environ.get("UE5_EDITOR_PATH")
        if ue_path:
            ue_exe = Path(ue_path)
            if ue_exe.exists():
                print_success(f"UE5 Editor: {ue_exe}")
            else:
                self.warnings.append("UE5_EDITOR_PATH points to non-existent file")
                print_warning(f"UE5 Editor not found at: {ue_exe}")
        else:
            self.warnings.append("$UE5_EDITOR_PATH not set")
            print_warning("$UE5_EDITOR_PATH environment variable not set")
            print_warning("  Set with: export UE5_EDITOR_PATH=/path/to/UnrealEditor-Cmd.exe")

        # Check project path
        project_path = os.environ.get("VERDUN_PROJECT_PATH")
        if project_path:
            project_file = Path(project_path)
            if project_file.exists():
                print_success(f"UE5 Project: {project_file}")
            else:
                self.warnings.append("Project file not found")
                print_warning(f"Project file not found: {project_file}")
        else:
            self.warnings.append("$VERDUN_PROJECT_PATH not set")
            print_warning("$VERDUN_PROJECT_PATH not set")

    def check_example_data(self):
        """Check example tile data."""
        print_section("Example Data (Tile N13)")

        data_files = {
            "Trench vectors": "data/incoming/vectors/N13_trenches.geojson",
            "Building footprints": "data/incoming/footprints/N13_buildings.geojson",
            "Debris catalog": "data/incoming/debris_catalog.csv",
            "DEM (required)": "data/incoming/rasters/N13_dem.tif",
            "Shell density (required)": "data/incoming/rasters/N13_shells.tif",
        }

        for name, path in data_files.items():
            full_path = self.repo_root / path
            if full_path.exists():
                size = full_path.stat().st_size
                print_success(f"{name}: {path} ({size} bytes)")
            else:
                if "required" in name:
                    self.warnings.append(f"Example data missing: {path}")
                    print_warning(f"{name}: {path} (MISSING - create for testing)")
                else:
                    print_warning(f"{name}: {path} (missing)")

    def check_hdas(self):
        """Check HDA files."""
        print_section("Houdini Digital Assets")

        required_hdas = [
            "TrenchBuilder.hda",
            "ShellField.hda",
            "DebrisScatter.hda",
            "BuildingRuinizer.hda",
        ]

        hda_dir = self.repo_root / "houdini" / "hdas"

        for hda_name in required_hdas:
            hda_path = hda_dir / hda_name
            if hda_path.exists():
                size = hda_path.stat().st_size
                print_success(f"{hda_name} ({size} bytes)")
            else:
                self.warnings.append(f"HDA not built: {hda_name}")
                print_warning(f"{hda_name} (NOT BUILT - see docs/HDA_SPEC.md)")

    def check_hip_file(self):
        """Check PDG hip file."""
        print_section("Houdini Scene Files")

        hip_file = self.repo_root / "houdini" / "hip" / "verdun_pdg.hip"
        if hip_file.exists():
            size = hip_file.stat().st_size
            print_success(f"verdun_pdg.hip ({size} bytes)")
        else:
            self.warnings.append("PDG scene file not created")
            print_warning("verdun_pdg.hip (NOT CREATED - see docs/RUNBOOK.md)")

    def generate_report(self):
        """Generate summary report."""
        print_section("Verification Summary")

        if not self.errors and not self.warnings:
            print_success("All checks passed! Pipeline is ready.")
            return 0

        if self.errors:
            print(f"\n{Colors.RED}Errors ({len(self.errors)}):{Colors.END}")
            for error in self.errors:
                print(f"  • {error}")

        if self.warnings:
            print(f"\n{Colors.YELLOW}Warnings ({len(self.warnings)}):{Colors.END}")
            for warning in self.warnings:
                print(f"  • {warning}")

        print("\nNext Steps:")
        if self.errors:
            print("  1. Fix errors listed above")
            print("  2. Re-run: python orchestrator/verify_setup.py")
        if self.warnings and not self.errors:
            print("  1. Address warnings for full functionality")
            print("  2. See docs/SETUP.md for detailed instructions")

        return 1 if self.errors else 0

    def run(self):
        """Run all verification checks."""
        print(f"\n{Colors.BLUE}Verdun Pipeline Setup Verification{Colors.END}\n")

        self.check_python_version()
        self.check_python_packages()
        self.check_directories()
        self.check_config()
        self.check_houdini()
        self.check_unreal()
        self.check_hdas()
        self.check_hip_file()
        self.check_example_data()

        return self.generate_report()


def main():
    """Main entry point."""
    verifier = SetupVerifier()
    exit_code = verifier.run()
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
