#!/usr/bin/env hython
"""
qc_render.py - Generate QC thumbnails for tile outputs.

Creates 512px preview renders of geometry and heightfields for visual verification.

Usage:
    hython qc_render.py --tile N13 --input data/staging/meshes/N13_terrain.fbx
    hython qc_render.py --batch data/staging/meshes/*.fbx
"""

import sys
import argparse
import logging
from pathlib import Path
from typing import List

# Add utils to path
sys.path.insert(0, str(Path(__file__).parent))
from utils import load_config, create_thumbnail_stub, VerdunIOError


def setup_logging(log_dir: Path) -> logging.Logger:
    """Configure logging."""
    log_dir.mkdir(parents=True, exist_ok=True)
    log_file = log_dir / "qc_render.log"

    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
        handlers=[
            logging.FileHandler(log_file),
            logging.StreamHandler(sys.stdout)
        ]
    )
    return logging.getLogger(__name__)


def render_geometry_thumbnail(geo_file: Path, output_file: Path,
                                size: int = 512) -> bool:
    """
    Render a thumbnail of a geometry file.

    Args:
        geo_file: Input geometry (FBX, USD, etc).
        output_file: Output image path.
        size: Thumbnail size in pixels.

    Returns:
        True if successful.
    """
    try:
        import hou
    except ImportError:
        raise VerdunIOError("Houdini 'hou' module not available. Run with hython.")

    logger = logging.getLogger(__name__)

    logger.info(f"Rendering thumbnail: {geo_file} -> {output_file}")

    # Create a new scene
    hou.hipFile.clear()

    # Load geometry
    obj = hou.node("/obj")
    geo = obj.createNode("geo", "qc_preview")
    file_node = geo.createNode("file", "load_geo")
    file_node.parm("file").set(str(geo_file))

    # Frame the geometry
    file_node.cook()

    # Create camera and light
    cam = obj.createNode("cam", "preview_cam")
    light = obj.createNode("hlight", "preview_light")

    # Position camera (simple auto-frame)
    # In production, calculate bounds and set camera transform
    cam.parm("tx").set(0)
    cam.parm("ty").set(0)
    cam.parm("tz").set(50)

    # Create a Mantra/Karma ROP for rendering
    out = hou.node("/out")
    rop = out.createNode("ifd", "qc_render")
    rop.parm("camera").set(cam.path())
    rop.parm("vm_picture").set(str(output_file))
    rop.parm("vm_renderengine").set("pbrraytrace")  # Fast preview

    # Set resolution
    rop.parm("vm_resolution1").set(size)
    rop.parm("vm_resolution2").set(size)

    # Render
    logger.info(f"Rendering {size}x{size} preview...")
    rop.render()

    if output_file.exists():
        logger.info(f"Thumbnail saved: {output_file}")
        return True
    else:
        logger.error(f"Thumbnail render failed: {output_file}")
        return False


def render_heightfield_thumbnail(heightfield_file: Path, output_file: Path,
                                   size: int = 512) -> bool:
    """
    Render a thumbnail of a heightfield/DEM.

    Args:
        heightfield_file: Input heightfield (TIFF, PNG, etc).
        output_file: Output image path.
        size: Thumbnail size in pixels.

    Returns:
        True if successful.
    """
    logger = logging.getLogger(__name__)

    logger.info(f"Rendering heightfield thumbnail: {heightfield_file} -> {output_file}")

    # For heightfields, we can:
    # 1. Use Houdini COPs to load and downscale
    # 2. Use external tools (PIL, ImageMagick)
    # 3. Render as 3D displaced surface

    # Stub implementation using create_thumbnail_stub
    return create_thumbnail_stub(heightfield_file, output_file, size)


def batch_render_thumbnails(files: List[Path], output_dir: Path,
                              size: int = 512) -> dict:
    """
    Render thumbnails for multiple files.

    Args:
        files: List of input files.
        output_dir: Where to save thumbnails.
        size: Thumbnail size.

    Returns:
        Results dictionary.
    """
    logger = logging.getLogger(__name__)

    output_dir.mkdir(parents=True, exist_ok=True)

    results = {
        "success": [],
        "failed": [],
        "total": len(files),
    }

    for file_path in files:
        output_file = output_dir / f"{file_path.stem}_thumb.png"

        try:
            # Detect file type
            if file_path.suffix.lower() in [".fbx", ".usd", ".usda", ".usdc", ".obj"]:
                success = render_geometry_thumbnail(file_path, output_file, size)
            elif file_path.suffix.lower() in [".tif", ".tiff", ".png", ".exr"]:
                success = render_heightfield_thumbnail(file_path, output_file, size)
            else:
                logger.warning(f"Unknown file type: {file_path}")
                continue

            if success:
                results["success"].append(str(file_path))
            else:
                results["failed"].append(str(file_path))

        except Exception as e:
            logger.error(f"Failed to render thumbnail for {file_path}: {e}", exc_info=True)
            results["failed"].append(str(file_path))

    logger.info(f"Batch render complete: {len(results['success'])} success, {len(results['failed'])} failed")
    return results


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Generate QC thumbnails for Verdun pipeline outputs"
    )
    parser.add_argument(
        "--input",
        type=Path,
        help="Single input file to render"
    )
    parser.add_argument(
        "--batch",
        nargs="+",
        help="Multiple input files (supports wildcards)"
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Output directory for thumbnails"
    )
    parser.add_argument(
        "--size",
        type=int,
        default=512,
        help="Thumbnail size in pixels (default: 512)"
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

    # Setup logging
    log_dir = Path(config["paths"]["logs"])
    logger = setup_logging(log_dir)

    logger.info("=" * 80)
    logger.info("QC Thumbnail Renderer")
    logger.info("=" * 80)

    # Determine output directory
    if args.output:
        output_dir = args.output
    else:
        output_dir = Path(config["paths"]["staging"]) / "thumbnails"

    # Process files
    if args.input:
        files = [args.input]
    elif args.batch:
        files = []
        for pattern in args.batch:
            files.extend(Path(".").glob(pattern))
    else:
        logger.error("Must specify --input or --batch")
        return 1

    if not files:
        logger.warning("No files to process")
        return 0

    logger.info(f"Rendering {len(files)} thumbnails...")

    try:
        results = batch_render_thumbnails(files, output_dir, args.size)

        if results["failed"]:
            logger.warning(f"{len(results['failed'])} thumbnails failed")
            return 1

        logger.info("All thumbnails rendered successfully")
        return 0

    except Exception as e:
        logger.error(f"Thumbnail rendering failed: {e}", exc_info=True)
        return 1


if __name__ == "__main__":
    sys.exit(main())
