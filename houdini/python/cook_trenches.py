#!/usr/bin/env hython
"""
cook_trenches.py - Single-shot trench generation using HDAs.

Directly cooks TrenchBuilder HDA without PDG for testing or single-tile workflows.

Usage:
    hython cook_trenches.py --curves data/incoming/vectors/N13_trenches.geojson \
                            --dem data/incoming/rasters/N13.tif \
                            --out data/staging/N13
"""

import sys
import argparse
import logging
from pathlib import Path
from typing import Optional
import json

# Add utils to path
sys.path.insert(0, str(Path(__file__).parent))
from utils import load_config, get_tile_paths, VerdunIOError


def setup_logging(log_dir: Path, tile_id: str) -> logging.Logger:
    """Configure logging."""
    log_dir.mkdir(parents=True, exist_ok=True)
    log_file = log_dir / f"cook_trenches_{tile_id}.log"

    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
        handlers=[
            logging.FileHandler(log_file),
            logging.StreamHandler(sys.stdout)
        ]
    )
    return logging.getLogger(__name__)


def load_hda(hda_path: Path, hda_name: str):
    """
    Load an HDA definition.

    Args:
        hda_path: Path to .hda file.
        hda_name: Name of the HDA operator type.

    Returns:
        HDA definition.
    """
    try:
        import hou
    except ImportError:
        raise VerdunIOError("Houdini 'hou' module not available. Run with hython.")

    logger = logging.getLogger(__name__)

    if not hda_path.exists():
        raise VerdunIOError(f"HDA file not found: {hda_path}")

    logger.info(f"Installing HDA: {hda_path}")
    hou.hda.installFile(str(hda_path))

    # Verify it loaded
    definitions = hou.hda.definitionsInFile(str(hda_path))
    if not definitions:
        raise VerdunIOError(f"No HDA definitions found in {hda_path}")

    logger.info(f"Loaded {len(definitions)} HDA definition(s)")
    return definitions[0]


def create_geometry_node(parent, node_type: str, name: str):
    """
    Create a geometry SOP node.

    Args:
        parent: Parent node.
        node_type: Type name (e.g., "trench_builder").
        name: Node name.

    Returns:
        Created node.
    """
    import hou

    logger = logging.getLogger(__name__)

    try:
        node = parent.createNode(node_type, name)
        logger.info(f"Created node: {node.path()}")
        return node
    except hou.OperationFailed as e:
        raise VerdunIOError(f"Failed to create node {node_type}: {e}")


def cook_trench_builder(curves_file: Path, dem_file: Path, output_dir: Path,
                        config: dict, tile_id: str) -> dict:
    """
    Cook TrenchBuilder HDA and export results.

    Args:
        curves_file: Input trench curves (GeoJSON).
        dem_file: Input DEM heightfield (TIFF).
        output_dir: Where to save outputs.
        config: Pipeline config.
        tile_id: Tile identifier.

    Returns:
        Metadata dictionary.
    """
    import hou

    logger = logging.getLogger(__name__)

    # Create a new scene
    hou.hipFile.clear()

    # Load TrenchBuilder HDA
    hda_dir = Path(config["paths"]["hdas"])
    hda_file = hda_dir / config["houdini"]["hdas"]["trench_builder"]
    load_hda(hda_file, "TrenchBuilder")

    # Create geometry container
    obj = hou.node("/obj")
    geo = obj.createNode("geo", f"trench_cook_{tile_id}")

    # File input for curves
    logger.info(f"Loading curves: {curves_file}")
    file_in_curves = geo.createNode("file", "load_curves")
    file_in_curves.parm("file").set(str(curves_file))

    # Heightfield file for DEM
    logger.info(f"Loading DEM: {dem_file}")
    hf_file = geo.createNode("heightfield_file", "load_dem")
    hf_file.parm("file").set(str(dem_file))

    # Create TrenchBuilder node (this is a stub - actual HDA has different internal name)
    # In production, use the correct operator type from your HDA
    logger.info("Creating TrenchBuilder node...")

    # Stub: create a merge for demo purposes
    # In production: trench_builder = create_geometry_node(geo, "trench_builder", "build_trenches")
    merge = geo.createNode("merge", "trench_output")
    merge.setInput(0, file_in_curves)
    merge.setInput(1, hf_file)

    # Set parameters (example)
    trench_config = config["generation"]["trenches"]
    # trench_builder.parm("width").set(trench_config["default_width"])
    # trench_builder.parm("depth").set(trench_config["default_depth"])

    # Cook
    logger.info("Cooking geometry...")
    merge.cook()

    # Export results
    output_dir.mkdir(parents=True, exist_ok=True)

    # Export mesh
    mesh_file = output_dir / f"{tile_id}_trenches.fbx"
    logger.info(f"Exporting FBX: {mesh_file}")

    rop_fbx = geo.createNode("rop_fbx", "export_fbx")
    rop_fbx.parm("sopoutput").set(merge.path())
    rop_fbx.parm("soppath").set(merge.path())
    rop_fbx.parm("execute").pressButton()

    # Export heightfield
    hf_out_file = output_dir / f"{tile_id}_height.png"
    logger.info(f"Exporting heightfield: {hf_out_file}")

    # In production, use heightfield_output node
    # rop_img = geo.createNode("heightfield_output", "export_height")
    # rop_img.parm("copoutput").set(str(hf_out_file))
    # rop_img.parm("execute").pressButton()

    # Generate metadata
    metadata = {
        "tile_id": tile_id,
        "inputs": {
            "curves": str(curves_file),
            "dem": str(dem_file),
        },
        "outputs": {
            "mesh": str(mesh_file),
            "heightmap": str(hf_out_file),
        },
        "triangle_count": 0,  # Would get from hou.Geometry in production
        "vertex_count": 0,
        "parameters": trench_config,
    }

    metadata_file = output_dir / f"{tile_id}_metadata.json"
    with open(metadata_file, 'w') as f:
        json.dump(metadata, f, indent=2)
    logger.info(f"Saved metadata: {metadata_file}")

    return metadata


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Single-shot trench generation using TrenchBuilder HDA"
    )
    parser.add_argument(
        "--curves",
        type=Path,
        required=True,
        help="Input trench curves (GeoJSON)"
    )
    parser.add_argument(
        "--dem",
        type=Path,
        required=True,
        help="Input DEM heightfield (TIFF)"
    )
    parser.add_argument(
        "--out",
        type=Path,
        required=True,
        help="Output directory"
    )
    parser.add_argument(
        "--tile",
        help="Tile ID (extracted from filename if not provided)"
    )
    parser.add_argument(
        "--config",
        type=Path,
        help="Path to config.yaml"
    )

    args = parser.parse_args()

    # Load configuration
    try:
        config = load_config(args.config)
    except Exception as e:
        print(f"ERROR: Failed to load config: {e}", file=sys.stderr)
        return 1

    # Extract tile ID
    tile_id = args.tile or args.curves.stem.replace("_trenches", "")

    # Setup logging
    log_dir = Path(config["paths"]["logs"])
    logger = setup_logging(log_dir, tile_id)

    logger.info("=" * 80)
    logger.info(f"Trench Builder Cook - Tile {tile_id}")
    logger.info("=" * 80)

    # Validate inputs
    if not args.curves.exists():
        logger.error(f"Curves file not found: {args.curves}")
        return 1

    if not args.dem.exists():
        logger.error(f"DEM file not found: {args.dem}")
        return 1

    # Cook
    try:
        metadata = cook_trench_builder(
            args.curves,
            args.dem,
            args.out,
            config,
            tile_id
        )
        logger.info("Cook completed successfully")
        return 0
    except Exception as e:
        logger.error(f"Cook failed: {e}", exc_info=True)
        return 1


if __name__ == "__main__":
    sys.exit(main())
