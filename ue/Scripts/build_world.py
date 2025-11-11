"""
build_world.py - Automated world assembly for Unreal Engine 5.

Reads instance catalogs (CSV/JSON) from Houdini and places actors/instances
in the UE5 level with proper transforms, LODs, and HLOD settings.

Usage:
    UnrealEditor-Cmd.exe Project.uproject -run=pythonscript -script=ue/Scripts/build_world.py

Or from within UE5 Python:
    import build_world
    build_world.main()
"""

import unreal
import sys
import csv
import json
from pathlib import Path
from typing import List, Dict, Optional


# Configure paths
REPO_ROOT = Path(__file__).parent.parent.parent
sys.path.insert(0, str(REPO_ROOT / "houdini" / "python"))

try:
    from utils import load_config, load_catalog_csv
    config = load_config()
except Exception as e:
    unreal.log_error(f"Failed to load config: {e}")
    config = None


class WorldBuilder:
    """Handles automated placement of instances in UE5 levels."""

    def __init__(self, config_dict: Optional[Dict] = None):
        """
        Initialize world builder.

        Args:
            config_dict: Configuration dictionary.
        """
        self.config = config_dict or {}
        self.editor_level_lib = unreal.EditorLevelLibrary()
        self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    def load_catalog(self, catalog_path: Path) -> List[Dict]:
        """
        Load an instance catalog.

        Args:
            catalog_path: Path to CSV or JSON catalog.

        Returns:
            List of instance dictionaries.
        """
        if not catalog_path.exists():
            unreal.log_error(f"Catalog not found: {catalog_path}")
            return []

        if catalog_path.suffix.lower() == ".json":
            with open(catalog_path, 'r') as f:
                instances = json.load(f)
                if isinstance(instances, dict) and "instances" in instances:
                    instances = instances["instances"]
        else:
            # CSV format
            instances = load_catalog_csv(catalog_path)

        unreal.log(f"Loaded {len(instances)} instances from: {catalog_path}")
        return instances

    def spawn_static_mesh_actor(self, mesh_path: str, location: unreal.Vector,
                                  rotation: unreal.Rotator, scale: unreal.Vector) -> Optional[unreal.Actor]:
        """
        Spawn a static mesh actor in the level.

        Args:
            mesh_path: UE5 asset path to static mesh.
            location: World location.
            rotation: World rotation.
            scale: Scale vector.

        Returns:
            Spawned actor or None.
        """
        # Load static mesh
        static_mesh = unreal.load_asset(mesh_path)
        if not static_mesh:
            unreal.log_warning(f"Static mesh not found: {mesh_path}")
            return None

        # Spawn actor
        actor = self.editor_actor_subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor,
            location,
            rotation
        )

        if actor:
            # Set static mesh component
            mesh_component = actor.static_mesh_component
            mesh_component.set_static_mesh(static_mesh)
            mesh_component.set_world_scale3d(scale)

            return actor
        else:
            unreal.log_error(f"Failed to spawn actor for: {mesh_path}")
            return None

    def create_hierarchical_instanced_static_mesh(self, mesh_path: str,
                                                   instances: List[Dict],
                                                   actor_name: str = "HISM_Actor") -> Optional[unreal.Actor]:
        """
        Create a Hierarchical Instanced Static Mesh (HISM) actor.

        Args:
            mesh_path: UE5 asset path to static mesh.
            instances: List of instance transforms.
            actor_name: Name for the HISM actor.

        Returns:
            HISM actor or None.
        """
        # Load static mesh
        static_mesh = unreal.load_asset(mesh_path)
        if not static_mesh:
            unreal.log_warning(f"Static mesh not found: {mesh_path}")
            return None

        # Create empty actor
        actor = self.editor_actor_subsystem.spawn_actor_from_class(
            unreal.Actor,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        actor.set_actor_label(actor_name)

        # Add HISM component
        hism_component = actor.add_component_by_class(
            unreal.HierarchicalInstancedStaticMeshComponent,
            manual_attachment=False,
            relative_transform=unreal.Transform()
        )

        hism_component.set_static_mesh(static_mesh)

        # Add instances
        for inst in instances:
            location = unreal.Vector(
                inst.get("x", 0),
                inst.get("y", 0),
                inst.get("z", 0)
            )
            rotation = unreal.Rotator(
                inst.get("pitch", 0),
                inst.get("yaw", 0),
                inst.get("roll", 0)
            )
            scale = unreal.Vector(
                inst.get("scale_x", 1.0),
                inst.get("scale_y", 1.0),
                inst.get("scale_z", 1.0)
            )

            transform = unreal.Transform(location, rotation, scale)
            hism_component.add_instance(transform)

        unreal.log(f"Created HISM actor with {len(instances)} instances: {actor_name}")
        return actor

    def build_tile_from_catalog(self, tile_id: str, catalog_path: Path) -> Dict:
        """
        Build a tile by placing instances from a catalog.

        Args:
            tile_id: Tile identifier.
            catalog_path: Path to instance catalog.

        Returns:
            Results dictionary.
        """
        results = {
            "tile_id": tile_id,
            "actors_spawned": 0,
            "instances_placed": 0,
            "failed": 0,
        }

        # Load catalog
        instances = self.load_catalog(catalog_path)

        if not instances:
            unreal.log_warning(f"No instances to place for tile {tile_id}")
            return results

        # Group instances by asset
        grouped = {}
        for inst in instances:
            asset_path = inst.get("asset_path", "")
            if not asset_path:
                continue

            if asset_path not in grouped:
                grouped[asset_path] = []
            grouped[asset_path].append(inst)

        # Place instances
        for asset_path, asset_instances in grouped.items():
            unreal.log(f"Placing {len(asset_instances)} instances of: {asset_path}")

            # Decide whether to use HISM or individual actors
            if len(asset_instances) > 10:
                # Use HISM for many instances
                actor_name = f"HISM_{tile_id}_{Path(asset_path).stem}"
                actor = self.create_hierarchical_instanced_static_mesh(
                    asset_path,
                    asset_instances,
                    actor_name
                )
                if actor:
                    results["actors_spawned"] += 1
                    results["instances_placed"] += len(asset_instances)
                else:
                    results["failed"] += len(asset_instances)
            else:
                # Individual actors for few instances
                for inst in asset_instances:
                    location = unreal.Vector(
                        inst.get("x", 0),
                        inst.get("y", 0),
                        inst.get("z", 0)
                    )
                    rotation = unreal.Rotator(
                        inst.get("pitch", 0),
                        inst.get("yaw", 0),
                        inst.get("roll", 0)
                    )
                    scale = unreal.Vector(
                        inst.get("scale_x", 1.0),
                        inst.get("scale_y", 1.0),
                        inst.get("scale_z", 1.0)
                    )

                    actor = self.spawn_static_mesh_actor(asset_path, location, rotation, scale)
                    if actor:
                        results["actors_spawned"] += 1
                        results["instances_placed"] += 1
                    else:
                        results["failed"] += 1

        unreal.log(f"Tile {tile_id}: {results['actors_spawned']} actors, {results['instances_placed']} instances, {results['failed']} failed")
        return results

    def build_all_tiles(self, catalog_dir: Path) -> List[Dict]:
        """
        Build all tiles found in catalog directory.

        Args:
            catalog_dir: Directory containing instance catalogs.

        Returns:
            List of results per tile.
        """
        results_list = []

        # Find all catalog files
        catalog_files = list(catalog_dir.glob("*_instances.csv")) + list(catalog_dir.glob("*_instances.json"))

        for catalog_file in catalog_files:
            # Extract tile ID from filename
            tile_id = catalog_file.stem.replace("_instances", "")

            unreal.log(f"Building tile: {tile_id}")
            results = self.build_tile_from_catalog(tile_id, catalog_file)
            results_list.append(results)

        return results_list


def main():
    """Main entry point for world building."""
    unreal.log("=" * 80)
    unreal.log("Verdun World Builder - Starting")
    unreal.log("=" * 80)

    # Initialize builder
    builder = WorldBuilder(config)

    # Determine catalog directory
    if config:
        catalog_dir = Path(config["paths"]["data_root"]) / "staging" / "catalogs"
    else:
        catalog_dir = REPO_ROOT / "data" / "staging" / "catalogs"

    # Build world
    results_list = builder.build_all_tiles(catalog_dir)

    # Summary
    total_actors = sum(r["actors_spawned"] for r in results_list)
    total_instances = sum(r["instances_placed"] for r in results_list)
    total_failed = sum(r["failed"] for r in results_list)

    unreal.log("=" * 80)
    unreal.log(f"Build complete: {total_actors} actors, {total_instances} instances, {total_failed} failed")
    unreal.log("=" * 80)

    # Save results
    results_file = REPO_ROOT / "orchestrator" / "logs" / "ue_build_results.json"
    results_file.parent.mkdir(parents=True, exist_ok=True)
    with open(results_file, 'w') as f:
        json.dump(results_list, f, indent=2)

    unreal.log(f"Results saved: {results_file}")


if __name__ == "__main__":
    main()
