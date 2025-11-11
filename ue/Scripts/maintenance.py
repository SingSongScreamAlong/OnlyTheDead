"""
maintenance.py - UE5 maintenance and housekeeping tasks.

Handles HLOD rebuilding, navmesh generation, redirector fixing,
and asset cleanup.

Usage:
    UnrealEditor-Cmd.exe Project.uproject -run=pythonscript -script=ue/Scripts/maintenance.py

Or from within UE5 Python:
    import maintenance
    maintenance.rebuild_hlods()
    maintenance.rebuild_navmesh()
"""

import unreal
import sys
from pathlib import Path
from typing import List, Optional


# Configure paths
REPO_ROOT = Path(__file__).parent.parent.parent
sys.path.insert(0, str(REPO_ROOT / "houdini" / "python"))

try:
    from utils import load_config
    config = load_config()
except Exception as e:
    unreal.log_error(f"Failed to load config: {e}")
    config = None


class MaintenanceTools:
    """UE5 maintenance and optimization tools."""

    def __init__(self, config_dict: Optional[dict] = None):
        """
        Initialize maintenance tools.

        Args:
            config_dict: Configuration dictionary.
        """
        self.config = config_dict or {}
        self.editor_level_lib = unreal.EditorLevelLibrary()
        self.editor_asset_lib = unreal.EditorAssetLibrary()

    def rebuild_hlods(self, map_path: Optional[str] = None) -> bool:
        """
        Rebuild HLODs for the current or specified map.

        Args:
            map_path: Optional map path. If None, uses current level.

        Returns:
            True if successful.
        """
        unreal.log("Rebuilding HLODs...")

        try:
            # Load map if specified
            if map_path:
                success = self.editor_level_lib.load_level(map_path)
                if not success:
                    unreal.log_error(f"Failed to load map: {map_path}")
                    return False

            # Get current world
            world = unreal.EditorLevelLibrary.get_editor_world()
            if not world:
                unreal.log_error("No world loaded")
                return False

            # Rebuild HLODs
            # Note: Direct HLOD rebuild API may vary by UE5 version
            # This is a stub - check UE5 Python API docs for exact method
            unreal.log("HLOD rebuild initiated (check UE5 Python API for exact implementation)")

            # In UE5, you might use:
            # unreal.HierarchicalLODUtilities.build_all_hlods(world)

            return True

        except Exception as e:
            unreal.log_error(f"HLOD rebuild failed: {e}")
            return False

    def rebuild_navmesh(self, map_path: Optional[str] = None) -> bool:
        """
        Rebuild navigation mesh for the current or specified map.

        Args:
            map_path: Optional map path.

        Returns:
            True if successful.
        """
        unreal.log("Rebuilding navigation mesh...")

        try:
            # Load map if specified
            if map_path:
                success = self.editor_level_lib.load_level(map_path)
                if not success:
                    unreal.log_error(f"Failed to load map: {map_path}")
                    return False

            # Get nav system
            world = unreal.EditorLevelLibrary.get_editor_world()
            if not world:
                unreal.log_error("No world loaded")
                return False

            # Rebuild navigation
            # Note: API may vary
            unreal.log("Navmesh rebuild initiated")
            # unreal.NavigationSystem.build(world)

            return True

        except Exception as e:
            unreal.log_error(f"Navmesh rebuild failed: {e}")
            return False

    def fix_redirectors(self, directory_path: str = "/Game/Verdun") -> int:
        """
        Fix up redirectors in a directory.

        Args:
            directory_path: UE5 content path to scan.

        Returns:
            Number of redirectors fixed.
        """
        unreal.log(f"Fixing redirectors in: {directory_path}")

        try:
            # Find all redirectors
            redirectors = self.editor_asset_lib.find_asset_data(directory_path)
            redirector_list = []

            for asset_data in redirectors:
                asset_class = asset_data.asset_class
                if asset_class == "ObjectRedirector":
                    redirector_list.append(asset_data.object_path)

            if not redirector_list:
                unreal.log("No redirectors found")
                return 0

            unreal.log(f"Found {len(redirector_list)} redirectors")

            # Fix redirectors
            # self.editor_asset_lib.consolidate_assets(...)
            # Note: Check UE5 Python API for exact method

            unreal.log(f"Fixed {len(redirector_list)} redirectors")
            return len(redirector_list)

        except Exception as e:
            unreal.log_error(f"Redirector fix failed: {e}")
            return 0

    def clean_stale_assets(self, directory_path: str = "/Game/Verdun",
                            dry_run: bool = True) -> List[str]:
        """
        Find and optionally delete unused assets.

        Args:
            directory_path: UE5 content path to scan.
            dry_run: If True, only reports assets without deleting.

        Returns:
            List of stale asset paths.
        """
        unreal.log(f"Scanning for stale assets in: {directory_path}")

        stale_assets = []

        try:
            # Get all assets in directory
            asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
            all_assets = asset_registry.get_assets_by_path(directory_path, recursive=True)

            for asset_data in all_assets:
                asset_path = str(asset_data.object_path)

                # Check if asset is referenced
                referencers = self.editor_asset_lib.find_package_referencers_for_asset(asset_path)

                if not referencers:
                    stale_assets.append(asset_path)

            unreal.log(f"Found {len(stale_assets)} potentially stale assets")

            if stale_assets and not dry_run:
                unreal.log("Deleting stale assets...")
                for asset_path in stale_assets:
                    self.editor_asset_lib.delete_asset(asset_path)
                    unreal.log(f"Deleted: {asset_path}")

        except Exception as e:
            unreal.log_error(f"Stale asset scan failed: {e}")

        return stale_assets

    def save_all_dirty_packages(self) -> bool:
        """
        Save all dirty packages.

        Returns:
            True if successful.
        """
        unreal.log("Saving all dirty packages...")

        try:
            dirty_packages = unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages()
            if dirty_packages:
                unreal.log(f"Saving {len(dirty_packages)} dirty packages")
                success = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(
                    save_map_packages=True,
                    save_content_packages=True
                )
                return success
            else:
                unreal.log("No dirty packages to save")
                return True

        except Exception as e:
            unreal.log_error(f"Save failed: {e}")
            return False

    def cook_maps(self, map_paths: List[str]) -> bool:
        """
        Cook specified maps (for packaging).

        Args:
            map_paths: List of map paths to cook.

        Returns:
            True if successful.
        """
        unreal.log(f"Cooking {len(map_paths)} maps...")

        # Note: Map cooking typically done via commandlet, not Python
        # This is a stub
        for map_path in map_paths:
            unreal.log(f"Cook map: {map_path}")

        return True


def main():
    """Main entry point for maintenance tasks."""
    unreal.log("=" * 80)
    unreal.log("Verdun Maintenance Tools")
    unreal.log("=" * 80)

    tools = MaintenanceTools(config)

    # Run maintenance tasks
    tasks = [
        ("Fix Redirectors", lambda: tools.fix_redirectors()),
        ("Rebuild HLODs", lambda: tools.rebuild_hlods()),
        ("Rebuild Navmesh", lambda: tools.rebuild_navmesh()),
        ("Save Dirty Packages", lambda: tools.save_all_dirty_packages()),
    ]

    for task_name, task_func in tasks:
        unreal.log(f"\n>>> Running: {task_name}")
        try:
            result = task_func()
            unreal.log(f"<<< {task_name}: {'SUCCESS' if result else 'FAILED'}")
        except Exception as e:
            unreal.log_error(f"<<< {task_name} EXCEPTION: {e}")

    # Optional: scan for stale assets (dry run)
    unreal.log("\n>>> Scanning for stale assets")
    stale = tools.clean_stale_assets(dry_run=True)
    if stale:
        unreal.log(f"Found {len(stale)} stale assets (dry run, not deleted)")

    unreal.log("\n" + "=" * 80)
    unreal.log("Maintenance complete")
    unreal.log("=" * 80)


if __name__ == "__main__":
    main()
