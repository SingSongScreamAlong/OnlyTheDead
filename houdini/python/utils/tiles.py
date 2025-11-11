"""
Tile management utilities for the Verdun pipeline.
Handles tile grids, naming, bounds calculation, and neighbor queries.
"""

import logging
import re
from typing import Tuple, List, Optional
from dataclasses import dataclass

logger = logging.getLogger(__name__)


@dataclass
class TileBounds:
    """Represents the spatial bounds of a tile."""
    min_x: float
    min_y: float
    max_x: float
    max_y: float

    @property
    def width(self) -> float:
        return self.max_x - self.min_x

    @property
    def height(self) -> float:
        return self.max_y - self.min_y

    @property
    def center(self) -> Tuple[float, float]:
        return ((self.min_x + self.max_x) / 2, (self.min_y + self.max_y) / 2)


class TileGrid:
    """
    Manages the Verdun tile grid system.

    Tile naming: Letter (A-Z) + Number (1-99)
    Example: N13, A5, Z42
    """

    def __init__(self, tile_size: float = 1000.0, overlap: float = 50.0,
                 origin_x: float = 0.0, origin_y: float = 0.0):
        """
        Initialize tile grid.

        Args:
            tile_size: Size of each tile in meters.
            overlap: Overlap between tiles in meters.
            origin_x: World origin X coordinate.
            origin_y: World origin Y coordinate.
        """
        self.tile_size = tile_size
        self.overlap = overlap
        self.origin_x = origin_x
        self.origin_y = origin_y

    @staticmethod
    def parse_tile_id(tile_id: str) -> Tuple[str, int]:
        """
        Parse a tile ID into letter and number components.

        Args:
            tile_id: Tile identifier (e.g., "N13").

        Returns:
            Tuple of (letter, number).

        Raises:
            ValueError if tile_id format is invalid.
        """
        match = re.match(r'^([A-Z])(\d+)$', tile_id.upper())
        if not match:
            raise ValueError(f"Invalid tile ID format: {tile_id}. Expected format: A1, N13, etc.")

        letter = match.group(1)
        number = int(match.group(2))
        return letter, number

    def get_tile_bounds(self, tile_id: str) -> TileBounds:
        """
        Calculate world-space bounds for a tile.

        Args:
            tile_id: Tile identifier.

        Returns:
            TileBounds object.
        """
        letter, number = self.parse_tile_id(tile_id)

        # Convert letter to column (A=0, B=1, ...)
        col = ord(letter) - ord('A')
        row = number

        min_x = self.origin_x + col * self.tile_size
        min_y = self.origin_y + row * self.tile_size
        max_x = min_x + self.tile_size
        max_y = min_y + self.tile_size

        return TileBounds(min_x, min_y, max_x, max_y)

    def get_tile_bounds_with_overlap(self, tile_id: str) -> TileBounds:
        """
        Calculate bounds including overlap margin.

        Args:
            tile_id: Tile identifier.

        Returns:
            TileBounds with overlap added.
        """
        bounds = self.get_tile_bounds(tile_id)
        return TileBounds(
            bounds.min_x - self.overlap,
            bounds.min_y - self.overlap,
            bounds.max_x + self.overlap,
            bounds.max_y + self.overlap
        )

    def get_neighbors(self, tile_id: str, include_diagonals: bool = False) -> List[str]:
        """
        Get neighboring tile IDs.

        Args:
            tile_id: Center tile identifier.
            include_diagonals: If True, includes diagonal neighbors (8 total).
                              If False, only cardinal neighbors (4 total).

        Returns:
            List of neighbor tile IDs.
        """
        letter, number = self.parse_tile_id(tile_id)
        col = ord(letter) - ord('A')
        row = number

        offsets = [
            (0, 1),   # North
            (1, 0),   # East
            (0, -1),  # South
            (-1, 0),  # West
        ]

        if include_diagonals:
            offsets.extend([
                (1, 1),   # NE
                (1, -1),  # SE
                (-1, -1), # SW
                (-1, 1),  # NW
            ])

        neighbors = []
        for dc, dr in offsets:
            new_col = col + dc
            new_row = row + dr

            # Validate bounds
            if new_col < 0 or new_col > 25:  # A-Z
                continue
            if new_row < 0 or new_row > 99:
                continue

            new_letter = chr(ord('A') + new_col)
            neighbor_id = f"{new_letter}{new_row}"
            neighbors.append(neighbor_id)

        return neighbors

    def point_to_tile(self, x: float, y: float) -> str:
        """
        Find which tile contains a given point.

        Args:
            x: World X coordinate.
            y: World Y coordinate.

        Returns:
            Tile ID containing the point.
        """
        col = int((x - self.origin_x) / self.tile_size)
        row = int((y - self.origin_y) / self.tile_size)

        if col < 0 or col > 25:
            raise ValueError(f"Point ({x}, {y}) outside valid column range")
        if row < 0:
            raise ValueError(f"Point ({x}, {y}) has negative row")

        letter = chr(ord('A') + col)
        return f"{letter}{row}"


def create_tile_manifest(tile_ids: List[str], description: str = "") -> dict:
    """
    Create a manifest file for a set of tiles.

    Args:
        tile_ids: List of tile identifiers.
        description: Optional description of this tile set.

    Returns:
        Manifest dictionary.
    """
    return {
        "version": "1.0",
        "description": description,
        "tile_count": len(tile_ids),
        "tiles": tile_ids,
    }
