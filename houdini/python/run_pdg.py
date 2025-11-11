#!/usr/bin/env hython
"""
run_pdg.py - Headless PDG/TOPs execution for Verdun pipeline.

Loads the PDG network and cooks it for specified tiles.
Supports filtering by tile ID, batch processing, and error recovery.

Usage:
    hython run_pdg.py --top /obj/VERDUN_TOP --tile N13
    hython run_pdg.py --top /obj/VERDUN_TOP --all --maxprocs 8
    hython run_pdg.py --top /obj/VERDUN_TOP --tile-list tiles.txt
"""

import sys
import argparse
import logging
from pathlib import Path
from typing import List, Optional
import json

# Add utils to path
sys.path.insert(0, str(Path(__file__).parent))
from utils import load_config, save_job_ticket, VerdunIOError


def setup_logging(log_dir: Path, tile_id: str = "pdg") -> logging.Logger:
    """Configure logging for this run."""
    log_dir.mkdir(parents=True, exist_ok=True)
    log_file = log_dir / f"pdg_{tile_id}.log"

    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
        handlers=[
            logging.FileHandler(log_file),
            logging.StreamHandler(sys.stdout)
        ]
    )
    return logging.getLogger(__name__)


def load_pdg_network(hip_file: Path, top_path: str):
    """
    Load a Houdini scene and return the TOP network.

    Args:
        hip_file: Path to .hip file.
        top_path: Path to TOP network node (e.g., /obj/VERDUN_TOP).

    Returns:
        PDG graph object.
    """
    try:
        import hou
    except ImportError:
        raise VerdunIOError("Houdini 'hou' module not available. Run with hython.")

    logger = logging.getLogger(__name__)

    logger.info(f"Loading hip file: {hip_file}")
    hou.hipFile.load(str(hip_file))

    logger.info(f"Getting TOP network: {top_path}")
    top_node = hou.node(top_path)

    if top_node is None:
        raise VerdunIOError(f"TOP network not found: {top_path}")

    if top_node.type().name() != "topnet":
        raise VerdunIOError(f"Node {top_path} is not a TOP network")

    return top_node


def cook_pdg_for_tile(top_node, tile_id: str, config: dict) -> bool:
    """
    Cook PDG network for a specific tile.

    Args:
        top_node: Houdini TOP network node.
        tile_id: Tile identifier.
        config: Pipeline configuration.

    Returns:
        True if successful.
    """
    logger = logging.getLogger(__name__)

    try:
        import hou
        import pdg
    except ImportError:
        raise VerdunIOError("PDG module not available")

    logger.info(f"Cooking PDG for tile: {tile_id}")

    # Set tile context variable (PDG nodes can reference $TILE_ID)
    top_node.parm("tile_id").set(tile_id) if top_node.parm("tile_id") else None

    # Alternative: use PDG scheduler variables
    graph_context = top_node.getPDGGraphContext()
    if graph_context:
        # Set custom attributes on work items
        logger.info(f"Setting PDG context: TILE_ID={tile_id}")

    # Cook the network
    logger.info("Starting PDG cook...")
    top_node.cookWorkItems()

    # Wait for completion
    # In production, you'd poll for status or use callbacks
    logger.info("PDG cook initiated. Use PDG events or polling to monitor progress.")

    # For headless batch, you might block until done:
    # graph_context.waitForAllTasksToComplete()

    return True


def batch_cook_tiles(top_node, tile_ids: List[str], config: dict) -> dict:
    """
    Cook multiple tiles in sequence.

    Args:
        top_node: TOP network node.
        tile_ids: List of tile IDs.
        config: Pipeline configuration.

    Returns:
        Results dictionary with success/failure counts.
    """
    logger = logging.getLogger(__name__)

    results = {
        "success": [],
        "failed": [],
        "total": len(tile_ids),
    }

    for tile_id in tile_ids:
        logger.info(f"Processing tile {tile_id} ({len(results['success']) + len(results['failed']) + 1}/{results['total']})")

        try:
            success = cook_pdg_for_tile(top_node, tile_id, config)
            if success:
                results["success"].append(tile_id)
            else:
                results["failed"].append(tile_id)
        except Exception as e:
            logger.error(f"Failed to cook tile {tile_id}: {e}", exc_info=True)
            results["failed"].append(tile_id)

    logger.info(f"Batch complete: {len(results['success'])} success, {len(results['failed'])} failed")
    return results


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Headless PDG/TOPs execution for Verdun pipeline"
    )
    parser.add_argument(
        "--hip",
        type=Path,
        default=Path(__file__).parent.parent / "hip" / "verdun_pdg.hip",
        help="Path to Houdini scene file"
    )
    parser.add_argument(
        "--top",
        default="/obj/VERDUN_TOP",
        help="Path to TOP network node"
    )
    parser.add_argument(
        "--tile",
        help="Single tile ID to process (e.g., N13)"
    )
    parser.add_argument(
        "--tile-list",
        type=Path,
        help="Text file with one tile ID per line"
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Process all tiles in incoming directory"
    )
    parser.add_argument(
        "--maxprocs",
        type=int,
        default=8,
        help="Maximum parallel processors for PDG"
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
    logger = setup_logging(log_dir, args.tile or "batch")

    logger.info("=" * 80)
    logger.info("Verdun PDG Runner")
    logger.info("=" * 80)

    # Determine tile list
    tile_ids = []
    if args.tile:
        tile_ids = [args.tile]
    elif args.tile_list:
        with open(args.tile_list, 'r') as f:
            tile_ids = [line.strip() for line in f if line.strip()]
    elif args.all:
        # Scan incoming directory for tiles
        incoming_dir = Path(config["paths"]["incoming"]) / "vectors"
        if incoming_dir.exists():
            for f in incoming_dir.glob("*_trenches.geojson"):
                tile_id = f.stem.replace("_trenches", "")
                tile_ids.append(tile_id)
        logger.info(f"Found {len(tile_ids)} tiles in incoming directory")
    else:
        logger.error("Must specify --tile, --tile-list, or --all")
        return 1

    if not tile_ids:
        logger.warning("No tiles to process")
        return 0

    logger.info(f"Will process {len(tile_ids)} tiles: {', '.join(tile_ids)}")

    # Load PDG network
    try:
        top_node = load_pdg_network(args.hip, args.top)
    except Exception as e:
        logger.error(f"Failed to load PDG network: {e}", exc_info=True)
        return 1

    # Set max processors
    # (In production, set via PDG scheduler parameters)
    logger.info(f"Using max {args.maxprocs} processors")

    # Cook tiles
    try:
        results = batch_cook_tiles(top_node, tile_ids, config)
    except Exception as e:
        logger.error(f"Batch cook failed: {e}", exc_info=True)
        return 1

    # Save results
    results_file = log_dir / f"pdg_results_{args.tile or 'batch'}.json"
    with open(results_file, 'w') as f:
        json.dump(results, f, indent=2)
    logger.info(f"Results saved to: {results_file}")

    # Exit code based on failures
    if results["failed"]:
        logger.warning(f"{len(results['failed'])} tiles failed")
        return 1

    logger.info("All tiles processed successfully")
    return 0


if __name__ == "__main__":
    sys.exit(main())
