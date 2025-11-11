"""
auto_ingest.py - Automated asset import for Unreal Engine 5.

Scans staging directory for new/updated FBX/USD files, imports them into UE5,
enables Nanite, sets collision presets, and assigns materials.

Usage:
    UnrealEditor-Cmd.exe Project.uproject -run=pythonscript -script=ue/Scripts/auto_ingest.py

Or from within UE5 Python:
    import auto_ingest
    auto_ingest.main()
"""

import unreal
import sys
import json
from pathlib import Path
from typing import List, Dict, Optional


# Configure paths
REPO_ROOT = Path(__file__).parent.parent.parent
sys.path.insert(0, str(REPO_ROOT / "houdini" / "python"))

try:
    from utils import load_config
    config = load_config()
except Exception as e:
    unreal.log_error(f"Failed to load config: {e}")
    config = None


class VerdunAssetImporter:
    """Handles automated import of Verdun pipeline assets into UE5."""

    def __init__(self, config_dict: Optional[Dict] = None):
        """
        Initialize importer.

        Args:
            config_dict: Configuration dictionary (loaded from config.yaml).
        """
        self.config = config_dict or {}
        self.ue_config = self.config.get("unreal", {})
        self.import_config = self.ue_config.get("import", {})

        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_util = unreal.EditorAssetLibrary()

    def import_fbx(self, source_path: Path, destination_path: str,
                   options: Optional[unreal.FbxImportUI] = None) -> Optional[unreal.Object]:
        """
        Import an FBX file.

        Args:
            source_path: Source FBX file path.
            destination_path: UE5 content path (e.g., /Game/Verdun/Tiles/N13).
            options: FBX import options.

        Returns:
            Imported asset or None if failed.
        """
        if not source_path.exists():
            unreal.log_error(f"Source file not found: {source_path}")
            return None

        # Create FBX import options
        if options is None:
            options = unreal.FbxImportUI()
            options.set_editor_property("import_mesh", True)
            options.set_editor_property("import_as_skeletal", False)
            options.set_editor_property("import_materials", True)
            options.set_editor_property("import_textures", True)

            # Static mesh settings
            static_mesh_options = unreal.FbxStaticMeshImportData()
            static_mesh_options.set_editor_property("combine_meshes", True)
            options.set_editor_property("static_mesh_import_data", static_mesh_options)

        # Import
        unreal.log(f"Importing FBX: {source_path} -> {destination_path}")

        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(source_path))
        task.set_editor_property("destination_path", destination_path)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("automated", True)
        task.set_editor_property("save", True)
        task.set_editor_property("options", options)

        self.asset_tools.import_asset_tasks([task])

        if task.get_editor_property("imported_object_paths"):
            asset_path = task.get_editor_property("imported_object_paths")[0]
            unreal.log(f"Successfully imported: {asset_path}")
            return unreal.load_asset(asset_path)
        else:
            unreal.log_error(f"Failed to import: {source_path}")
            return None

    def enable_nanite(self, static_mesh: unreal.StaticMesh) -> bool:
        """
        Enable Nanite on a static mesh.

        Args:
            static_mesh: Static mesh asset.

        Returns:
            True if successful.
        """
        if not static_mesh:
            return False

        try:
            # Enable Nanite
            static_mesh.set_editor_property("nanite_settings",
                                             unreal.NaniteSettings(enabled=True))
            unreal.log(f"Enabled Nanite on: {static_mesh.get_name()}")
            return True
        except Exception as e:
            unreal.log_warning(f"Failed to enable Nanite on {static_mesh.get_name()}: {e}")
            return False

    def set_collision_preset(self, static_mesh: unreal.StaticMesh, preset_name: str) -> bool:
        """
        Set collision preset on a static mesh.

        Args:
            static_mesh: Static mesh asset.
            preset_name: Collision preset name (e.g., "BlockAllDynamic").

        Returns:
            True if successful.
        """
        if not static_mesh:
            return False

        try:
            # Get the body setup
            body_setup = static_mesh.get_editor_property("body_setup")
            if body_setup:
                body_setup.set_editor_property("collision_response", preset_name)
                unreal.log(f"Set collision preset on {static_mesh.get_name()}: {preset_name}")
                return True
        except Exception as e:
            unreal.log_warning(f"Failed to set collision on {static_mesh.get_name()}: {e}")
            return False

    def assign_material(self, static_mesh: unreal.StaticMesh, material_path: str) -> bool:
        """
        Assign a material to a static mesh.

        Args:
            static_mesh: Static mesh asset.
            material_path: UE5 path to material.

        Returns:
            True if successful.
        """
        if not static_mesh:
            return False

        material = unreal.load_asset(material_path)
        if not material:
            unreal.log_warning(f"Material not found: {material_path}")
            return False

        try:
            # Set material on all mesh sections
            static_materials = static_mesh.get_editor_property("static_materials")
            for i in range(len(static_materials)):
                static_materials[i].set_editor_property("material_interface", material)

            static_mesh.set_editor_property("static_materials", static_materials)
            unreal.log(f"Assigned material to {static_mesh.get_name()}: {material_path}")
            return True
        except Exception as e:
            unreal.log_warning(f"Failed to assign material to {static_mesh.get_name()}: {e}")
            return False

    def import_tile(self, tile_id: str, staging_dir: Path) -> Dict[str, any]:
        """
        Import all assets for a tile.

        Args:
            tile_id: Tile identifier (e.g., "N13").
            staging_dir: Directory containing staged assets.

        Returns:
            Results dictionary.
        """
        results = {
            "tile_id": tile_id,
            "imported": [],
            "failed": [],
        }

        # Determine UE5 destination path
        base_path = self.import_config.get("base_path", "/Game/Verdun/Tiles")
        dest_path = f"{base_path}/{tile_id}"

        # Ensure destination exists
        if not self.editor_util.does_directory_exist(dest_path):
            self.editor_util.make_directory(dest_path)
            unreal.log(f"Created directory: {dest_path}")

        # Find FBX files in staging
        fbx_files = list(staging_dir.glob("**/*.fbx"))

        for fbx_file in fbx_files:
            unreal.log(f"Processing: {fbx_file}")

            # Import
            asset = self.import_fbx(fbx_file, dest_path)

            if asset and isinstance(asset, unreal.StaticMesh):
                # Apply settings
                if self.import_config.get("enable_nanite", True):
                    self.enable_nanite(asset)

                collision_preset = self.import_config.get("collision_preset", "BlockAllDynamic")
                self.set_collision_preset(asset, collision_preset)

                # Assign master material if specified
                master_material = self.import_config.get("master_material")
                if master_material:
                    self.assign_material(asset, master_material)

                results["imported"].append(str(fbx_file))

                # Save
                self.editor_util.save_asset(asset.get_path_name())
            else:
                results["failed"].append(str(fbx_file))

        unreal.log(f"Tile {tile_id} import complete: {len(results['imported'])} imported, {len(results['failed'])} failed")
        return results

    def scan_and_import_all(self, staging_root: Path) -> List[Dict]:
        """
        Scan staging directory and import all tiles.

        Args:
            staging_root: Root staging directory.

        Returns:
            List of results per tile.
        """
        results_list = []

        # Look for tile directories
        mesh_dir = staging_root / "meshes"
        if not mesh_dir.exists():
            unreal.log_warning(f"Staging directory not found: {mesh_dir}")
            return results_list

        # Group files by tile
        tile_files = {}
        for fbx_file in mesh_dir.glob("*.fbx"):
            # Extract tile ID from filename (e.g., N13_terrain.fbx -> N13)
            tile_id = fbx_file.stem.split("_")[0]
            if tile_id not in tile_files:
                tile_files[tile_id] = []
            tile_files[tile_id].append(fbx_file)

        # Import each tile
        for tile_id in sorted(tile_files.keys()):
            unreal.log(f"Importing tile: {tile_id}")
            results = self.import_tile(tile_id, mesh_dir)
            results_list.append(results)

        return results_list


def main():
    """Main entry point for automated ingestion."""
    unreal.log("=" * 80)
    unreal.log("Verdun Auto-Ingest - Starting")
    unreal.log("=" * 80)

    # Initialize importer
    importer = VerdunAssetImporter(config)

    # Determine staging directory
    if config:
        staging_root = Path(config["paths"]["data_root"]) / "staging"
    else:
        staging_root = REPO_ROOT / "data" / "staging"

    # Scan and import
    results_list = importer.scan_and_import_all(staging_root)

    # Summary
    total_imported = sum(len(r["imported"]) for r in results_list)
    total_failed = sum(len(r["failed"]) for r in results_list)

    unreal.log("=" * 80)
    unreal.log(f"Import complete: {total_imported} assets imported, {total_failed} failed")
    unreal.log("=" * 80)

    # Save results
    results_file = REPO_ROOT / "orchestrator" / "logs" / "ue_import_results.json"
    results_file.parent.mkdir(parents=True, exist_ok=True)
    with open(results_file, 'w') as f:
        json.dump(results_list, f, indent=2)

    unreal.log(f"Results saved: {results_file}")


if __name__ == "__main__":
    main()
