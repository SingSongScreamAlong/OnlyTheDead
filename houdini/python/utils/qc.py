"""
Quality control utilities for the Verdun pipeline.
Handles validation, metrics, and thumbnail generation.
"""

import logging
import json
from pathlib import Path
from typing import Dict, List, Any, Optional
from datetime import datetime
from dataclasses import dataclass, asdict

logger = logging.getLogger(__name__)


@dataclass
class QCMetrics:
    """Quality control metrics for a tile."""
    tile_id: str
    timestamp: str
    triangle_count: int = 0
    vertex_count: int = 0
    mesh_files: int = 0
    texture_files: int = 0
    total_size_mb: float = 0.0
    layers_present: List[str] = None
    layers_missing: List[str] = None
    errors: List[str] = None
    warnings: List[str] = None
    passed: bool = False

    def __post_init__(self):
        if self.layers_present is None:
            self.layers_present = []
        if self.layers_missing is None:
            self.layers_missing = []
        if self.errors is None:
            self.errors = []
        if self.warnings is None:
            self.warnings = []

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return asdict(self)


class QualityControl:
    """Quality control validator for pipeline outputs."""

    def __init__(self, config: Dict[str, Any]):
        """
        Initialize QC validator.

        Args:
            config: Pipeline configuration dictionary.
        """
        self.config = config
        self.qc_config = config.get("qc", {})

    def validate_tile_output(self, tile_id: str, output_dir: Path) -> QCMetrics:
        """
        Validate outputs for a tile.

        Args:
            tile_id: Tile identifier.
            output_dir: Directory containing tile outputs.

        Returns:
            QCMetrics object with validation results.
        """
        metrics = QCMetrics(
            tile_id=tile_id,
            timestamp=datetime.now().isoformat()
        )

        if not output_dir.exists():
            metrics.errors.append(f"Output directory not found: {output_dir}")
            return metrics

        # Check file counts and sizes
        mesh_files = list(output_dir.glob("**/*.fbx")) + list(output_dir.glob("**/*.usd"))
        texture_files = list(output_dir.glob("**/*.png")) + list(output_dir.glob("**/*.tif"))

        metrics.mesh_files = len(mesh_files)
        metrics.texture_files = len(texture_files)

        total_size = sum(f.stat().st_size for f in output_dir.rglob("*") if f.is_file())
        metrics.total_size_mb = total_size / (1024 * 1024)

        # Check minimum file size
        min_size_mb = self.qc_config.get("min_file_size_mb", 0.1)
        if metrics.total_size_mb < min_size_mb:
            metrics.errors.append(f"Total output size {metrics.total_size_mb:.2f}MB below minimum {min_size_mb}MB")

        # Check for required layers (weight maps)
        required_layers = self.qc_config.get("required_layers", [])
        for layer in required_layers:
            layer_files = list(output_dir.glob(f"**/*{layer}*"))
            if layer_files:
                metrics.layers_present.append(layer)
            else:
                metrics.layers_missing.append(layer)

        if metrics.layers_missing:
            metrics.warnings.append(f"Missing layers: {', '.join(metrics.layers_missing)}")

        # Validate triangle count (requires external tool or metadata)
        # For stub, we'll check if metadata exists
        metadata_file = output_dir / f"{tile_id}_metadata.json"
        if metadata_file.exists():
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
                metrics.triangle_count = metadata.get("triangle_count", 0)
                metrics.vertex_count = metadata.get("vertex_count", 0)

            max_triangles = self.qc_config.get("max_triangles_per_tile", 5000000)
            if metrics.triangle_count > max_triangles:
                metrics.errors.append(
                    f"Triangle count {metrics.triangle_count} exceeds maximum {max_triangles}"
                )

        # Determine pass/fail
        metrics.passed = len(metrics.errors) == 0

        return metrics

    def save_qc_report(self, metrics: QCMetrics, output_path: Path) -> None:
        """
        Save QC report to JSON.

        Args:
            metrics: QC metrics to save.
            output_path: Where to save the report.
        """
        output_path.parent.mkdir(parents=True, exist_ok=True)
        with open(output_path, 'w') as f:
            json.dump(metrics.to_dict(), f, indent=2)
        logger.info(f"Saved QC report: {output_path}")

    def generate_summary_report(self, metrics_list: List[QCMetrics], output_path: Path) -> None:
        """
        Generate a summary CSV report for multiple tiles.

        Args:
            metrics_list: List of QCMetrics objects.
            output_path: Where to save the CSV.
        """
        import csv

        if not metrics_list:
            logger.warning("No metrics to summarize")
            return

        output_path.parent.mkdir(parents=True, exist_ok=True)

        fieldnames = [
            "tile_id", "timestamp", "passed", "triangle_count", "vertex_count",
            "mesh_files", "texture_files", "total_size_mb", "errors", "warnings"
        ]

        with open(output_path, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writeheader()

            for metrics in metrics_list:
                row = {
                    "tile_id": metrics.tile_id,
                    "timestamp": metrics.timestamp,
                    "passed": metrics.passed,
                    "triangle_count": metrics.triangle_count,
                    "vertex_count": metrics.vertex_count,
                    "mesh_files": metrics.mesh_files,
                    "texture_files": metrics.texture_files,
                    "total_size_mb": f"{metrics.total_size_mb:.2f}",
                    "errors": "; ".join(metrics.errors),
                    "warnings": "; ".join(metrics.warnings),
                }
                writer.writerow(row)

        logger.info(f"Saved QC summary: {output_path} ({len(metrics_list)} tiles)")


def create_thumbnail_stub(input_file: Path, output_file: Path, size: int = 512) -> bool:
    """
    Create a thumbnail from a mesh or heightmap.

    Note: This is a stub. In production, you would:
    - Use Houdini's flipbook/render for 3D thumbnails
    - Use PIL/Pillow for image downscaling
    - Call external tools like ImageMagick

    Args:
        input_file: Source file (mesh, heightmap, etc).
        output_file: Output thumbnail path.
        size: Thumbnail size in pixels.

    Returns:
        True if successful.
    """
    logger.info(f"[STUB] Creating {size}px thumbnail: {input_file} -> {output_file}")

    # In production, implement actual thumbnail generation
    # For now, just create a placeholder file
    output_file.parent.mkdir(parents=True, exist_ok=True)
    output_file.write_text(f"Thumbnail placeholder for {input_file.name}")

    return True


def check_file_integrity(file_path: Path) -> bool:
    """
    Check if a file is valid and not corrupted.

    Args:
        file_path: File to check.

    Returns:
        True if file appears valid.
    """
    if not file_path.exists():
        logger.error(f"File does not exist: {file_path}")
        return False

    if file_path.stat().st_size == 0:
        logger.error(f"File is empty: {file_path}")
        return False

    # Add format-specific checks in production
    # e.g., validate FBX header, check PNG CRC, etc.

    return True
