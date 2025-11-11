"""
I/O utilities for the Verdun pipeline.
Handles file operations, data validation, and format conversions.
"""

import json
import csv
import logging
from pathlib import Path
from typing import Dict, List, Optional, Any
import yaml

logger = logging.getLogger(__name__)


class VerdunIOError(Exception):
    """Custom exception for I/O operations."""
    pass


def load_config(config_path: Optional[Path] = None) -> Dict[str, Any]:
    """
    Load pipeline configuration from YAML.

    Args:
        config_path: Path to config.yaml. If None, searches standard locations.

    Returns:
        Configuration dictionary.
    """
    if config_path is None:
        # Search standard locations
        search_paths = [
            Path.cwd() / "orchestrator" / "config.yaml",
            Path(__file__).parent.parent.parent.parent / "orchestrator" / "config.yaml",
        ]
        for path in search_paths:
            if path.exists():
                config_path = path
                break
        else:
            raise VerdunIOError("config.yaml not found in standard locations")

    logger.info(f"Loading config from: {config_path}")
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)

    return config


def save_job_ticket(ticket: Dict[str, Any], output_path: Path) -> None:
    """
    Save a job ticket JSON file.

    Args:
        ticket: Job ticket dictionary.
        output_path: Where to save the ticket.
    """
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, 'w') as f:
        json.dump(ticket, f, indent=2)
    logger.info(f"Saved job ticket: {output_path}")


def load_job_ticket(ticket_path: Path) -> Dict[str, Any]:
    """
    Load a job ticket JSON file.

    Args:
        ticket_path: Path to ticket file.

    Returns:
        Job ticket dictionary.
    """
    with open(ticket_path, 'r') as f:
        ticket = json.load(f)
    logger.info(f"Loaded job ticket: {ticket_path}")
    return ticket


def validate_job_ticket(ticket: Dict[str, Any]) -> bool:
    """
    Validate job ticket has required fields.

    Args:
        ticket: Job ticket to validate.

    Returns:
        True if valid, raises VerdunIOError otherwise.
    """
    required_fields = [
        "tile_id",
        "version",
        "inputs",
        "outputs"
    ]

    for field in required_fields:
        if field not in ticket:
            raise VerdunIOError(f"Job ticket missing required field: {field}")

    # Validate inputs exist
    if "inputs" in ticket:
        for input_type, path in ticket["inputs"].items():
            if path and not Path(path).exists():
                logger.warning(f"Input file not found: {path} ({input_type})")

    return True


def load_catalog_csv(csv_path: Path) -> List[Dict[str, Any]]:
    """
    Load a CSV catalog (e.g., debris props, building types).

    Args:
        csv_path: Path to CSV file.

    Returns:
        List of dictionaries, one per row.
    """
    if not csv_path.exists():
        logger.warning(f"Catalog not found: {csv_path}, returning empty list")
        return []

    items = []
    with open(csv_path, 'r', newline='', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            items.append(row)

    logger.info(f"Loaded {len(items)} items from catalog: {csv_path}")
    return items


def save_catalog_csv(items: List[Dict[str, Any]], csv_path: Path) -> None:
    """
    Save a CSV catalog.

    Args:
        items: List of dictionaries.
        csv_path: Output path.
    """
    if not items:
        logger.warning("No items to save")
        return

    csv_path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = items[0].keys()

    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(items)

    logger.info(f"Saved {len(items)} items to catalog: {csv_path}")


def ensure_path_exists(path: Path, is_file: bool = False) -> Path:
    """
    Ensure a directory or file parent exists.

    Args:
        path: Path to check/create.
        is_file: If True, creates parent directory. If False, creates directory itself.

    Returns:
        The path.
    """
    if is_file:
        path.parent.mkdir(parents=True, exist_ok=True)
    else:
        path.mkdir(parents=True, exist_ok=True)
    return path


def get_tile_paths(tile_id: str, config: Dict[str, Any]) -> Dict[str, Path]:
    """
    Generate standard paths for a tile.

    Args:
        tile_id: Tile identifier (e.g., "N13").
        config: Pipeline configuration.

    Returns:
        Dictionary of named paths.
    """
    root = Path(config["paths"]["data_root"])

    return {
        "incoming_vectors": root / "incoming" / "vectors" / f"{tile_id}_trenches.geojson",
        "incoming_dem": root / "incoming" / "rasters" / f"{tile_id}_dem.tif",
        "incoming_shell_density": root / "incoming" / "rasters" / f"{tile_id}_shells.tif",
        "incoming_buildings": root / "incoming" / "footprints" / f"{tile_id}_buildings.geojson",
        "staging_mesh": root / "staging" / "meshes" / f"{tile_id}_terrain.fbx",
        "staging_heightmap": root / "staging" / "heightmaps" / f"{tile_id}_height.png",
        "staging_weightmaps": root / "staging" / "weightmaps" / tile_id,
        "staging_catalog": root / "staging" / "catalogs" / f"{tile_id}_instances.csv",
        "published": root / "published" / tile_id,
    }
